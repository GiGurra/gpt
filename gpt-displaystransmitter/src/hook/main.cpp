#include <se/gigurra/gpt/model/ClassRegistry.cpp>

#include "stdafx.h"
#include "windows.h"
#include "detours.h"
#include "ID3D9Wrapper.h"
#pragma comment(lib, "detours.lib")

typedef IDirect3D9 *(WINAPI*CreateD3D9DevFcn)(UINT SDKVersion);
static CreateD3D9DevFcn s_realCreateD3D9IfFcn = NULL;

static IDirect3D9* WINAPI s_myCreateD3D9IfFcn(UINT SDKVersion) {
	return new Direct3D9Wrapper(s_realCreateD3D9IfFcn(SDKVersion));
}

SmhSettings g_shmSettings = {};
SocketSettings g_socketSettings = {};

static void readSettingsFromFile() {

	char fullPathName[1025] = { };
	GetCurrentDirectoryA(900, fullPathName);
	strcat(fullPathName, "\\DisplaysTransmitter.ini");
	// Read SHM settings
	g_shmSettings.active = GetPrivateProfileInt("falconhook_shm", "active", 0, fullPathName) != 0;
	GetPrivateProfileStringA("falconhook_shm", "name", "GiGurraTexturesSharedMemoryArea", g_shmSettings.name, 128, fullPathName);
	g_shmSettings.pad = 0;

	// Read socket settings
	g_socketSettings.active = GetPrivateProfileInt("falconhook_socket", "active", 0, fullPathName) != 0;
	GetPrivateProfileStringA("falconhook_socket", "addr", "127.0.0.1", g_socketSettings.addrStr, 128, fullPathName);
	g_socketSettings.pad = 0;
	g_socketSettings.port = GetPrivateProfileInt("falconhook_socket", "port", 0, fullPathName);
	g_socketSettings.max_kbps = GetPrivateProfileInt("falconhook_socket", "max_kbps", 0, fullPathName);
	g_socketSettings.max_hz = GetPrivateProfileInt("falconhook_socket", "max_hz", 0, fullPathName);
	g_socketSettings.jpegQual = GetPrivateProfileInt("falconhook_socket", "jpegQual", 0, fullPathName);

	logToTestFileX("\n  Read from config file:");
	logToTestFileX(fullPathName);
	logToTestFileX("\n  falconhook_shm");
	logToTestFileNX(g_shmSettings.active);
	logToTestFileX(g_shmSettings.name);
	logToTestFileX("\n  falconhook_socket:");
	logToTestFileNX(g_socketSettings.active);
	logToTestFileX(g_socketSettings.addrStr);
	logToTestFileNX(g_socketSettings.port);
	logToTestFileNX(g_socketSettings.max_kbps);
	logToTestFileNX(g_socketSettings.max_hz);
	logToTestFileNX(g_socketSettings.jpegQual);

	// These must at least be something...else we stall the distributor thread..which means we will hang upon exiting a mission
	g_socketSettings.max_hz = g_socketSettings.max_hz <= 0 ? 50 : g_socketSettings.max_hz;
	g_socketSettings.max_kbps = g_socketSettings.max_kbps <= 0 ? 100 * 1024 : g_socketSettings.max_kbps;
}

extern "C" {
__declspec(dllexport) BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
	if (ul_reason_for_call == DLL_PROCESS_ATTACH) {
		//DisableThreadLibraryCalls(hModule);
		readSettingsFromFile();
		//s_realCreateD3D9IfFcn = (CreateD3D9DevFcn)DetourFunction((PBYTE)Direct3DCreate9, (PBYTE)s_myCreateD3D9IfFcn);
		s_realCreateD3D9IfFcn = (CreateD3D9DevFcn)detourFunction((PBYTE)Direct3DCreate9, (PBYTE)s_myCreateD3D9IfFcn);
	}
	return true;
}
}
