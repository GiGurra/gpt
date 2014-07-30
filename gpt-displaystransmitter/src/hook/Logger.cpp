#include "stdafx.h"
#include "windows.h"

static HANDLE s_mutex = CreateMutex(NULL, FALSE, NULL);
static FILE * pFile = NULL;

void logToTestFileX(const char * str) {
	if (str != NULL && strlen(str) > 0) {
		WaitForSingleObject(s_mutex, INFINITE);
		pFile = fopen("hook.log", "a");
		if (pFile) {
			fprintf(pFile, "%s\n", str);
			fclose(pFile);
		}
		ReleaseMutex(s_mutex);
	}
}

void logToTestFileNX(const double d) {
	char buf[32] = { };
	sprintf(buf, "%f", d);
	logToTestFileX(buf);
}
