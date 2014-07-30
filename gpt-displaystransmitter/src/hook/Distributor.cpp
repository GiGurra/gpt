#include "stdafx.h"
#include "time.h"

/******************************************************************************
 *
 *
 *							GENERAL AND SHM INTERNALS
 *
 ******************************************************************************/

// These are set by the main thread before starting the slave thread
static void * s_trgPtr = NULL; // SHM ptr
static void * s_srcPtr = NULL; // RAM ptr to downloaded frame
static int s_nBytes = 0; // bytes per frame (set once)
static int s_width = 0; // width in pixels of a frame (set once)
static int s_height = 0; // height in pixels of a frame (set once)

// Spin variables
static volatile bool s_alive = false; // true if slave thread is alive
static volatile bool s_toLive = false; // false if slave thread should commit suicide

/******************************************************************************
 *
 *
 *							JPG EXPORT INTERNALS
 *
 ******************************************************************************/

#include "jpeglib.h"
#include "turbojpeg.h"
#define WIN32_LEAN_AND_MEAN
#include "WinSock2.h"
#pragma comment(lib, "jpeg.lib")
#pragma comment(lib, "turbojpeg.lib")
#pragma comment(lib, "Ws2_32")

#define PRE_ALLOC_SZ (10 * 1024 * 1024)
#define DATA_BLOCK_SIZE (1024)
#define KEY (0x01234567)
#define PAD (0)

typedef struct {
	unsigned int key;
	unsigned int pad;
	unsigned int size;
	unsigned int frameId;
	unsigned int framePartid;
	unsigned int totalFrameParts;
	char data[DATA_BLOCK_SIZE];
	unsigned int nSignBytes;
	unsigned int pad2;
} JpgMsg;

static void displaysSockTransmit(const char * pSrcData, const int nBytes) {

	static bool wsaUp = false;
	static bool tcpUp = false;
	static bool tcpConnected = false;
	static SOCKET s = NULL;

	if (!wsaUp) {
		WSAData wsaData;
		wsaUp = WSAStartup(MAKEWORD(2,2), &wsaData) == NO_ERROR;
		logToTestFileX("WSAStartup");
	}

	if (wsaUp && !tcpUp) {
		s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP); // create the socket
		tcpUp = s != INVALID_SOCKET;
		if (tcpUp) {
			logToTestFileX("socket created");
		}
	}

	if (tcpUp && !tcpConnected) {
		sockaddr_in dest = { };
		dest.sin_family = AF_INET;
		dest.sin_addr.s_addr = inet_addr(g_socketSettings.addrStr);
		dest.sin_port = htons(g_socketSettings.port);
		tcpConnected = connect(s, (SOCKADDR*) &dest, sizeof(dest)) != SOCKET_ERROR;
		if (tcpConnected) {
			const int noDelaySetting = TRUE;
			setsockopt(s, IPPROTO_TCP, TCP_NODELAY, (const char*) &noDelaySetting, sizeof(int));
			logToTestFileX("socket connect()");
		}
	}

	if (tcpConnected) {
		const int sendResult = send(s, pSrcData, nBytes, 0);
		if (sendResult == SOCKET_ERROR) {
			closesocket(s);
			tcpConnected = false;
			tcpUp = false;
		} else if (sendResult < nBytes && sendResult > 0) {
			displaysSockTransmit(pSrcData + sendResult, nBytes - sendResult);
		}
	} else {
		logToTestFileX("Unable to set up tcp talker");
	}

}

static void transmitUdMsg(const unsigned char * srcData, const unsigned int imageDataSz) {

	// Calculated
	static unsigned int frameId = 0;
	const unsigned int nTotalFrameParts = imageDataSz / DATA_BLOCK_SIZE + (imageDataSz % DATA_BLOCK_SIZE != 0 ? 1 : 0);
	int nBytesLeft = imageDataSz;
	int nextByte = 0;
	int curFramePartId = 0;

	JpgMsg out;
	while (nBytesLeft > 0) {

		int nBytesThisPart = min(nBytesLeft, DATA_BLOCK_SIZE);

		out.key = htonl(KEY);
		out.pad = htonl(PAD);
		out.size = htonl(sizeof(JpgMsg));
		out.frameId = htonl(frameId);
		out.framePartid = htonl(curFramePartId);
		out.totalFrameParts = htonl(nTotalFrameParts);
		memcpy(out.data, srcData + nextByte, nBytesThisPart);
		out.nSignBytes = htonl(nBytesThisPart);

		displaysSockTransmit((const char *) &out, sizeof(JpgMsg));

		nBytesLeft -= nBytesThisPart;
		nextByte += nBytesThisPart;
		curFramePartId++;

	}

	frameId++;
}

/******************************************************************************
 *
 *
 *						Internal thread main loop
 *
 ******************************************************************************/

static DWORD WINAPI thrdFnc(LPVOID lpThreadParameter) {

	static tjhandle jpgCompressor = NULL;

	if (jpgCompressor == NULL) {
		jpgCompressor = tjInitCompress();
	}

	const unsigned long minDt = 1000 / g_socketSettings.max_hz;

	logToTestFileX("slave thread started");
	while (s_toLive) {

		// Write to SHM
		if (g_shmSettings.active) {
			memcpy(s_trgPtr, s_srcPtr, s_nBytes);
			Sleep(10);
		}

		// Send over tcp
		else if (g_socketSettings.active) {

			static unsigned char jpgData[PRE_ALLOC_SZ];
			static unsigned long tLastTransmission = 0;
			static unsigned long lastPayLoad = 0;
			unsigned long curTime = clock();

			// Consider maximum rate
			if (curTime - tLastTransmission > minDt) {

				// Consider maximum bandwidth
				while ((g_socketSettings.max_kbps * (clock() - tLastTransmission)) < (lastPayLoad * 8)) {
					Sleep(1);
				}

				// Transmit!
				unsigned long sz = PRE_ALLOC_SZ;
				unsigned char * p = jpgData;
				int res = tjCompress2(jpgCompressor, (unsigned char *) s_srcPtr, s_width, 4 * s_width, s_height, TJPF_BGRX,
						&p, &sz, TJSAMP_422, g_socketSettings.jpegQual, TJFLAG_NOREALLOC);
				if (res == 0) {
					transmitUdMsg(jpgData, sz);
					tLastTransmission = curTime;
					lastPayLoad = sz;
				}

			} else {
				Sleep(2);
			}
		}

		// If neither is active, just sleep...
		else {
			Sleep(10);
		}

	}
	MemoryBarrier();
	s_alive = false;
	logToTestFileX("slave thread quit");
	return TRUE;
}

/******************************************************************************
 *
 *
 *								EXPOSED
 *
 ******************************************************************************/

static HANDLE s_threadId = 0;
void startTexSharer(void * src, const int width, const int height, const int bytesPerPixel) {

	static HANDLE shmHandle = NULL;
	int newSize = width * height * bytesPerPixel;

	if (s_trgPtr == NULL) {
		if (shmHandle == NULL) {
			shmHandle = OpenFileMappingA(FILE_MAP_WRITE, false, g_shmSettings.name);
		}
		if (shmHandle == NULL) {
			shmHandle = CreateFileMappingA(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, newSize, g_shmSettings.name);
		}
		if (shmHandle != NULL) {
			s_trgPtr = MapViewOfFile(shmHandle, FILE_MAP_WRITE, 0, 0, 0);
		}
	}

	if (s_trgPtr != NULL) {
		if (!s_alive) {
			s_srcPtr = src;
			s_toLive = true;
			s_alive = true;
			s_nBytes = newSize;
			s_width = width;
			s_height = height;
			MemoryBarrier();
			s_threadId = CreateThread(NULL, 0, &thrdFnc, NULL, 0, 0);
		}
	} else {
		logToTestFileX("Unable to create shared memory for writing");
	}
}

void killTexSharer() {
	TerminateThread(s_threadId, 0);
	MemoryBarrier();
	s_alive = false;
	/*
	 int waits = 0;
	 while (s_alive) {
	 s_toLive = false;
	 Sleep(10);
	 }*/
}
