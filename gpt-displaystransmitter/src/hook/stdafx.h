#pragma once

#define _CRT_SECURE_NO_WARNINGS 1
#define DIRECTINPUT_VERSION 0x0900
#define _WIN32_WINNT 0x0500

#include <stdint.h>
#include <stdio.h>

void logText(const char * str);
void logNumber(const double d);
void startTexSharer(void * src, const int width, const int height, const int bytesPerPixel);
void killTexSharer();

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

void *detourFunction(uint8_t *orig, uint8_t *hook, int len = 5);

template <typename T>
static int release(T * item) {
	return item ? item->Release() : 0;
}
