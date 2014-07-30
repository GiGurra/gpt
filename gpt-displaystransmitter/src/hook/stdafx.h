#pragma once

#define _CRT_SECURE_NO_WARNINGS 1
#define DLLEXPORT __declspec(dllexport)
#define DIRECTINPUT_VERSION 0x0900
#define _WIN32_WINNT 0x0500

#include "stdlib.h"
#include "stdio.h"
#include "string.h"

void logToTestFileX(const char * str);
void logToTestFileNX(const double d);
void startTexSharer(void * src, const int width, const int height, const int bytesPerPixel);
void killTexSharer();

#if 0
#define logToTestFile(a) logToTestFileX(a)
#define logToTestFileN(a) logToTestFileNX(a)
#else
#define logToTestFile(a) 1
#define logToTestFileN(a) 1
#endif

typedef struct {
	bool active;
	char name[128];
	int pad;
} SmhSettings;

typedef struct {
	bool active;
	char addrStr[128];
	int pad;
	int port;
	unsigned long max_kbps;
	unsigned long max_hz;
	int jpegQual;
} SocketSettings;

extern SmhSettings g_shmSettings;
extern SocketSettings g_socketSettings;
