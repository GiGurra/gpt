#pragma once

/***********************************************
*
*				GLOBAL DEFINES
*
************************************************/

#define _CRT_SECURE_NO_WARNINGS 1
#define DIRECTINPUT_VERSION 0x0900
#define _WIN32_WINNT 0x0500

/***********************************************
*
*				GLOBAL INCLUDES
*
************************************************/

#include <vector>
#include <string>
#include <fstream>
#include <stdint.h>
#include <stdio.h>

#include <se/gigurra/gpt/model/ClassRegistry.h>
#include <mnet/MNetSerializer.h>

using namespace se::gigurra::gpt::model;
using namespace se::gigurra::gpt::model::common;
using namespace se::gigurra::gpt::model::displays;
using namespace se::gigurra::gpt::model::displays::transmitter;


/***********************************************
*
*				GLOBAL VARS
*
************************************************/

/**
* Global settings for this application
*/
extern const StreamTransmitterCfg g_settings;

/***********************************************
*
*				GLOBAL UTILS
*
************************************************/

/**
* Logs text.
*/
void logText(const char * str);

/**
* Logs text.
*/
void logText(const std::string& str);

/**
* Logs a number.
*/
void logNumber(const double d);

/**
* Starts the slave thread that does the texture sharing/compression/transmission
*/
void startTexSharer(void * src, const int width, const int height, const int bytesPerPixel);

/**
* Stops the slave thread that does the texture sharing/compression/transmission
*/
void killTexSharer();

/**
* utility for dx release with null checking
*/
template <typename Releasable>
inline int release(Releasable * item) {
	return item ? item->Release() : 0;
}

/**
* Reads a file completely to memory
*/
inline std::vector<char> file2Vector(const std::string& fileName) {
	std::ifstream f(fileName, std::ios::binary);
	return std::vector<char>(std::istreambuf_iterator<char>(f), std::istreambuf_iterator<char>());
}

/**
* Reads a file completely to memory
*/
inline std::string file2String(const std::string& fileName) {
	std::ifstream f(fileName, std::ios::binary);
	return std::string(std::istreambuf_iterator<char>(f), std::istreambuf_iterator<char>());
}

/**
* Writes a string to a file
*/
inline void string2File(const std::string& fileName, const std::string& data) {
	std::ofstream file(fileName, std::ios::binary);
	file.write(data.data(), data.size());
}

/**
* Writes a std::vector<char> to a file
*/
inline void vector2File(const std::string& fileName, const std::vector<char>& data) {
	std::ofstream file(fileName, std::ios::binary);
	file.write(data.data(), data.size());
}