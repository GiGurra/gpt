#include <stdio.h>
#include <stdlib.h>
#include "platform.h"

/*****************************************************************************
 * 								TYPES
 *****************************************************************************/

typedef struct {
	HWND hwnd;
	long x1;
	long y2;
	long y1;
	long x2;
	long width;
	long height;
} WindowPos;

/*****************************************************************************
 * 								STATIC VARS
 *****************************************************************************/

static char needToPutMfdsOnTop = TRUE;
static WindowPos mfdPositions[2];

/*****************************************************************************
 * 								STATIC FCNS
 *****************************************************************************/

static void getWndPosAndSize(HWND wnd, WindowPos * trg) {
	RECT rect;
	memset(&rect, 0, sizeof(RECT));
	GetWindowRect(wnd, &rect);
	trg->x1 = rect.left;
	trg->y2 = rect.bottom;
	trg->y1 = rect.top;
	trg->x2 = rect.right;
	trg->width = rect.right - rect.left;
	trg->height = rect.bottom - rect.top;
	trg->hwnd = wnd;
}

static void ensureNotOnTop(const WindowPos * pos) {
	if (WS_EX_TOPMOST & GetWindowLongPtr(pos->hwnd, GWL_EXSTYLE)) {
		SetWindowPos(pos->hwnd, HWND_NOTOPMOST, pos->x1, pos->y1, pos->width, pos->height, SWP_FRAMECHANGED);
		needToPutMfdsOnTop = TRUE;
		printf("\n Set window not on top x1: %ld, x2: %ld, y1: %ld, y2: %ld", pos->x1, pos->x2, pos->y1, pos->y2);
	}
}

static void ensureOnTop(const WindowPos * pos) {
	if (needToPutMfdsOnTop || (!(WS_EX_TOPMOST & GetWindowLongPtr(pos->hwnd, GWL_EXSTYLE)))) {
		SetWindowPos(pos->hwnd, HWND_TOPMOST, pos->x1, pos->y1, pos->width, pos->height, SWP_FRAMECHANGED);
		printf("\nchanged mfds");
		printf("\n Set window on top x1: %ld, x2: %ld, y1: %ld, y2: %ld", pos->x1, pos->x2, pos->y1, pos->y2);
	}
}

static int separate(const WindowPos * A, const WindowPos * B) {
	return A->x2 < B->x1 || A->x1 > B->x2 || A->y2 < B->y1 || A->y1 > B->y2;
}

static CALLBACK BOOL enumTdCb(HWND wnd, LPARAM param) {
	WindowPos curWndPos;
	getWndPosAndSize(wnd, &curWndPos);
	int i;
	for (i = 0; i < 2; i++) {
		if (!separate(&curWndPos, mfdPositions + i)) {
			ensureNotOnTop(&curWndPos);
		}
	}
	return TRUE;
}

/*****************************************************************************
 * 								EXPOSED FCNS
 *****************************************************************************/

int main(void) {

	while (1) {

		const HWND rMfdHwnd = FindWindow(NULL, "Right MFD");
		const HWND lMfdHwnd = FindWindow(NULL, "Left MFD");
		const HWND heliosHwnd = FindWindow(NULL, "Helios Control Center");

		if (rMfdHwnd != NULL && lMfdHwnd != NULL && heliosHwnd != NULL) {
			getWndPosAndSize(lMfdHwnd, mfdPositions + 0);
			getWndPosAndSize(rMfdHwnd, mfdPositions + 1);

			DWORD heliosThread = GetWindowThreadProcessId(heliosHwnd, NULL);
			if (heliosThread != 0) {
				EnumThreadWindows(heliosThread, &enumTdCb, 0);
			}

			// Ensure mfds are always on top
			int i;
			for (i = 0; i < 2; i++) {
				ensureOnTop(mfdPositions + i);
			}
			needToPutMfdsOnTop = FALSE;
		} else {
			needToPutMfdsOnTop = TRUE;
		}
		fflush(stdout);
		Sleep(1000);
	}

	return EXIT_SUCCESS;
}
