#pragma comment(lib,"detours.lib")
#include "windows.h"
#include "detours.h"

int main() {
	STARTUPINFO s = { sizeof(STARTUPINFO) };
	PROCESS_INFORMATION p = { 0 };
	return !DetourCreateProcessWithDll("Falcon BMS.exe", NULL, NULL, NULL, TRUE, 0, NULL, NULL, &s, &p, "FalconDisplaysExposer.dll", 0);
}
