

#include "common.h"
#include <thread>
#include <memory>
#include <mnet/MNetClient.h>
#include <functional>
#include <QtCore/QCoreApplication>
#include <QEventLoop>
#include <QTimer>
#include <QTime>
#include "jpeglib.h"
#include "turbojpeg.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include "time.h"

using se::gigurra::gpt::model::displays::common::StreamMsg;
using se::gigurra::gpt::model::ClassRegistry;

/******************************************************************************
 *
 *
 *							STATICS
 *
 ******************************************************************************/

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

static void threadFunc(volatile void * src, const int width, const int height) {
	static const std::string& myNetworkName = "gpt-disp-transmitter";
	static const std::string& tgtNetworkName = "gpt-disp-receiver";
	static unsigned char s_compressBuffer[10 * 1024 * 1024];
	static mnet::MNetSerializer<ClassRegistry> serializer;
	static tjhandle jpgCompressor = tjInitCompress();

	auto _qtApp = ensureQtAppOrCreateNew(0, 0);
	logText("Enter threadFunc");
		
	std::vector<std::shared_ptr<mnet::MNetClient>> clients;
	for (const auto& tgt : g_settings.getTargets()) {
		const std::string url = std::string("ws://") + tgt.getIp() + std::string(":") + std::to_string(tgt.getPort());
		clients.push_back(std::make_shared<mnet::MNetClient>(url, myNetworkName));
	}

	int frameNbr = 0;
	QTime elapsedTimer;
	QTimer stepTimer;
	QEventLoop eventLoop;
	QObject::connect(&stepTimer, &QTimer::timeout, [&] {
		if (s_toLive) {
			if (elapsedTimer.elapsed() >= 1000.0 / g_settings.getMaxFps()) {
				elapsedTimer.restart();

				unsigned long frameSize = sizeof(s_compressBuffer);
				unsigned char * p = s_compressBuffer;
				const int res = tjCompress2(jpgCompressor, (unsigned char *)src, width, 4 * width, height, TJPF_BGRX, &p, &frameSize, TJSAMP_422, 100.0 * g_settings.getJpegQual(), TJFLAG_NOREALLOC);
				if (res == 0) {

					const std::vector<char>& msgData = 
						serializer.writeBinary(StreamMsg()
						.setFrameNbr(frameNbr++)
						.setWidth(width)
						.setHeight(height)
						.setData(std::vector<char>(p, p + frameSize)));

					for (auto& client : clients) {
						for (const auto& route : client->getRoutes()) {
							if (client->isConnected() && route.name() == tgtNetworkName) {
								client->sendBinary(msgData, route.id());
							}
						}
					}
				}
				else {
					logText(std::string("tjCompress2 failed with error code ").append(std::to_string(res)));
				}
			}
		} else {
			eventLoop.exit();
		}
	});
	elapsedTimer.start();
	stepTimer.start(1); // We can't set teh refresh rate here, because the timers are too imprecise
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
		s_thread = std::make_shared<std::thread>(threadFunc, src, width, height);
	} else {
		logText("ERROR: startTexSharer: called twice in a row!");
	}
}

void killTexSharer() {
	if (s_thread) {
		s_toLive = false;
		s_thread->join();
		s_thread.reset();
	}
}
