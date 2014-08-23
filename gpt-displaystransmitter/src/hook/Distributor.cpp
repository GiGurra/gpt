

#include "common.h"
#include <thread>
#include <memory>
#include <mnet/MNetClient.h>
#include <functional>
#include <QtCore/QCoreApplication>
#include <QEventLoop>
#include <QTimer>
#include "jpeglib.h"
#include "turbojpeg.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include "time.h"

/******************************************************************************
 *
 *
 *							STATICS
 *
 ******************************************************************************/

// These are set by the main thread before starting the slave thread
static void * s_srcPtr = NULL; // RAM ptr to downloaded frame
static volatile bool s_toLive = false; // false if slave thread should commit suicide

/******************************************************************************
 *
 *
 *							Thread stuff
 *
 ******************************************************************************/

static QSharedPointer<QCoreApplication> ensureQtAppOrCreateNew(int argc, char *argv[]) {
	if (QCoreApplication::instance()) {
		return QSharedPointer<QCoreApplication>(QCoreApplication::instance(), [](QCoreApplication * app){});
	}
	else {
		return QSharedPointer<QCoreApplication>(new QCoreApplication(argc, argv));
	}
}

static void threadFunc(const int width, const int height) {
	static const std::string& myNetworkName = "gpt-disp-transmitter";
	static const std::string& tgtNetworkName = "gpt-disp-receiver";

	static unsigned char s_compressBuffer[10 * 1024 * 1024];
	static auto _qtApp = ensureQtAppOrCreateNew(0, 0);
	static tjhandle jpgCompressor = tjInitCompress();
	static se::culvertsoft::mnet::DataMessage msg;

	logText("Enter threadFunc");
		
	std::vector<std::shared_ptr<mnet::MNetClient>> clients;
	for (const auto& tgt : g_settings.getTargets()) {
		const std::string url = std::string("ws://") + tgt.getIp() + std::string(":") + std::to_string(tgt.getPort());
		clients.push_back(std::make_shared<mnet::MNetClient>(url, myNetworkName));
	}

	QTimer timer;
	QEventLoop eventLoop;
	QObject::connect(&timer, &QTimer::timeout, [&] {
		if (s_toLive) {

			unsigned long frameSize = sizeof(s_compressBuffer);
			unsigned char * p = s_compressBuffer;
			const int res = tjCompress2(jpgCompressor, (unsigned char *)s_srcPtr, width, 4 * width, height, TJPF_BGRX, &p, &frameSize, TJSAMP_422, 100.0 * g_settings.getJpegQual(), TJFLAG_NOREALLOC);
			if (res == 0) {
				std::vector<char>& buffer = msg.getBinaryDataMutable();
				buffer.resize(frameSize);
				memcpy(buffer.data(), s_compressBuffer, frameSize);
				for (auto& client : clients) {
					if (client->isConnected()) {
						for (const auto& route : client->getRoutes()) {
							if (route.name() == tgtNetworkName) {
								client->send(msg.setTargetId(route.details().getSenderId()));
							}
						}
					}
				}
			}
			else {
				logText(std::string("tjCompress2 failed with error code ").append(std::to_string(res)));
			}
		} else {
			eventLoop.exit();
		}
	});
	timer.start(1000.0 / g_settings.getMaxFps());
	eventLoop.exec();

	logText("Exit threadFunc");

}

/******************************************************************************
 *
 *
 *								EXPOSED
 *
 ******************************************************************************/
static std::shared_ptr<std::thread> s_thread;

void startTexSharer(void * src, const int width, const int height, const int bytesPerPixel) {
	if (!s_thread) {
		s_toLive = true;
		s_thread = std::make_shared<std::thread>(threadFunc, width, height);
	} else {
		logText("ERROR: startTexSharer called twice in a row!");
	}
}

void killTexSharer() {
	if (s_thread) {
		s_toLive = false;
		s_thread->join();
		s_thread.reset();
	}
}
