
/***********************************************
*
*				INCLUDES & DEFS
*
************************************************/

#include <se/gigurra/gpt/model/ClassRegistry.cpp> // Unity build for generated data model
#include <se/culvertsoft/mnet/ClassRegistry.cpp> // Unity build for generated data model
#include "common.h"
#include "ID3D9Wrapper.h"

#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "jpeg.lib")
#pragma comment(lib, "turbojpeg.lib")

typedef IDirect3D9 *(WINAPI*CreateD3D9DevFcn)(UINT SDKVersion);

/***********************************************
*
*				GLOBALS & STATICS
*
************************************************/
static StreamTransmitterCfg readConfig();
static IDirect3D9* WINAPI myCreateD3D9IfFcn(UINT SDKVersion);

static const std::string s_cfgFileName = "gpt_displaystransmitter_cfg.json";
static CreateD3D9DevFcn s_realCreateD3D9IfFcn = NULL;
const StreamTransmitterCfg g_settings = readConfig();

/***********************************************
*
*				DLL ENTRY POINT
*
************************************************/

extern "C" {
__declspec(dllexport) BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
	if (ul_reason_for_call == DLL_PROCESS_ATTACH) {
		//DisableThreadLibraryCalls(hModule);
		s_realCreateD3D9IfFcn = (CreateD3D9DevFcn)detourFunction((PBYTE)Direct3DCreate9, (PBYTE)myCreateD3D9IfFcn);
	}
	return true;
}
}

/***********************************************
*
*				HELPERS IMPL
*
************************************************/

static StreamTransmitterCfg readConfig() {

	mnet::MNetSerializer<ClassRegistry> serializer;
	const std::vector<char> cfgFileData = file2Vector(s_cfgFileName);

	StreamTransmitterCfg cfg = !cfgFileData.empty() ?
		*serializer.readJson<StreamTransmitterCfg>(cfgFileData.data(), cfgFileData.size()) : StreamTransmitterCfg();

	vector2File(s_cfgFileName, serializer.writeJson(cfg));

	return cfg;
	
}


static IDirect3D9* WINAPI myCreateD3D9IfFcn(UINT SDKVersion) {
	return new Direct3D9Wrapper(s_realCreateD3D9IfFcn(SDKVersion));
}
