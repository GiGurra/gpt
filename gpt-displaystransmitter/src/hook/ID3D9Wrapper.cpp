#include "stdafx.h"
#include "ID3D9Wrapper.h"
#include "ID3D9Wrapper_Device.h"


/******************************************************************************
*
* 
*							MODIFIED DX FCNS
*
******************************************************************************/

HRESULT Direct3D9Wrapper::CreateDevice(UINT Adapter,D3DDEVTYPE DeviceType,HWND hFocusWindow,DWORD BehaviorFlags,D3DPRESENT_PARAMETERS* pPresentationParameters,IDirect3DDevice9** ppReturnedDeviceInterface) {
	logToTestFile("Direct3D9Wrapper::CreateDevice");
	IDirect3DDevice9* pDirect3DDevice9;
	HRESULT hRes = Direct3D9->CreateDevice( Adapter, DeviceType, hFocusWindow, BehaviorFlags, pPresentationParameters, &pDirect3DDevice9 );
	*ppReturnedDeviceInterface = new Direct3DDevice9Wrapper( pDirect3DDevice9, this, pPresentationParameters );
	return hRes;
}

Direct3D9Wrapper::Direct3D9Wrapper(LPDIRECT3D9 pDirect3D) {
	logToTestFile("Direct3D9Wrapper::Direct3D9Wrapper");
	Direct3D9 = pDirect3D;
}


/******************************************************************************
*
* 
*						UNMODIFIED DX FCNS
*
******************************************************************************/

Direct3D9Wrapper::~Direct3D9Wrapper() {
	logToTestFile("Direct3D9Wrapper::~Direct3D9Wrapper");
}

HRESULT Direct3D9Wrapper::QueryInterface(const IID &riid, void **ppvObj) {
	logToTestFile("Direct3D9Wrapper::QueryInterface");
	return Direct3D9->QueryInterface(riid, ppvObj);
}

ULONG Direct3D9Wrapper::AddRef() {
	logToTestFile("Direct3D9Wrapper::AddRef");
	return Direct3D9->AddRef();
}

ULONG Direct3D9Wrapper::Release() {
	logToTestFile("Direct3D9Wrapper::Release");
	return Direct3D9->Release();
}

HRESULT Direct3D9Wrapper::RegisterSoftwareDevice(void* pInitializeFunction) {
	logToTestFile("Direct3D9Wrapper::RegisterSoftwareDevice");
	return Direct3D9->RegisterSoftwareDevice(pInitializeFunction);
}

UINT Direct3D9Wrapper::GetAdapterCount() {
	logToTestFile("Direct3D9Wrapper::GetAdapterCount");
	return Direct3D9->GetAdapterCount();
}

UINT Direct3D9Wrapper::GetAdapterModeCount(UINT Adapter, D3DFORMAT Format) {
	logToTestFile("Direct3D9Wrapper::GetAdapterModeCount");
	return Direct3D9->GetAdapterModeCount(Adapter, Format);
}

HRESULT Direct3D9Wrapper::GetAdapterIdentifier(UINT Adapter,DWORD Flags,D3DADAPTER_IDENTIFIER9* pIdentifier) {
	logToTestFile("Direct3D9Wrapper::GetAdapterIdentifier");
	return Direct3D9->GetAdapterIdentifier(Adapter, Flags, pIdentifier);
}

HRESULT Direct3D9Wrapper::EnumAdapterModes(UINT Adapter,D3DFORMAT Format,UINT Mode,D3DDISPLAYMODE* pMode) {
	logToTestFile("Direct3D9Wrapper::EnumAdapterModes");
	return Direct3D9->EnumAdapterModes(Adapter, Format, Mode, pMode);
}

HRESULT Direct3D9Wrapper::GetAdapterDisplayMode(UINT Adapter,D3DDISPLAYMODE* pMode) {
	logToTestFile("Direct3D9Wrapper::GetAdapterDisplayMode");
	return Direct3D9->GetAdapterDisplayMode(Adapter, pMode);
}

HRESULT Direct3D9Wrapper::CheckDeviceType(UINT Adapter,D3DDEVTYPE DevType,D3DFORMAT AdapterFormat,D3DFORMAT BackBufferFormat,BOOL bWindowed) {
	logToTestFile("Direct3D9Wrapper::CheckDeviceType");
	return Direct3D9->CheckDeviceType(Adapter, DevType, AdapterFormat, BackBufferFormat, bWindowed);
}

HRESULT Direct3D9Wrapper::CheckDeviceFormat(UINT Adapter,D3DDEVTYPE DeviceType,D3DFORMAT AdapterFormat,DWORD Usage,D3DRESOURCETYPE RType,D3DFORMAT CheckFormat) {
	logToTestFile("Direct3D9Wrapper::CheckDeviceFormat");
	return Direct3D9->CheckDeviceFormat(Adapter, DeviceType, AdapterFormat, Usage, RType, CheckFormat);
}

HRESULT Direct3D9Wrapper::CheckDeviceMultiSampleType(UINT Adapter,D3DDEVTYPE DeviceType,D3DFORMAT SurfaceFormat,BOOL Windowed,D3DMULTISAMPLE_TYPE MultiSampleType,DWORD* pQualityLevels) {
	logToTestFile("Direct3D9Wrapper::CheckDeviceMultiSampleType");
	return Direct3D9->CheckDeviceMultiSampleType(Adapter, DeviceType, SurfaceFormat, Windowed, MultiSampleType, pQualityLevels);
}

HRESULT Direct3D9Wrapper::CheckDepthStencilMatch(UINT Adapter,D3DDEVTYPE DeviceType,D3DFORMAT AdapterFormat,D3DFORMAT RenderTargetFormat,D3DFORMAT DepthStencilFormat) {
	logToTestFile("Direct3D9Wrapper::CheckDepthStencilMatch");
	return Direct3D9->CheckDepthStencilMatch(Adapter, DeviceType, AdapterFormat, RenderTargetFormat, DepthStencilFormat);
}

HRESULT Direct3D9Wrapper::CheckDeviceFormatConversion(UINT Adapter,D3DDEVTYPE DeviceType,D3DFORMAT SourceFormat,D3DFORMAT TargetFormat) {
	logToTestFile("Direct3D9Wrapper::CheckDeviceFormatConversion");
	return Direct3D9->CheckDeviceFormatConversion(Adapter, DeviceType, SourceFormat, TargetFormat);
}

HRESULT Direct3D9Wrapper::GetDeviceCaps(UINT Adapter,D3DDEVTYPE DeviceType,D3DCAPS9* pCaps) {
	logToTestFile("Direct3D9Wrapper::GetDeviceCaps");
	return Direct3D9->GetDeviceCaps(Adapter, DeviceType, pCaps);
}

HMONITOR Direct3D9Wrapper::GetAdapterMonitor(UINT Adapter) {
	logToTestFile("Direct3D9Wrapper::GetAdapterMonitor");
	return Direct3D9->GetAdapterMonitor(Adapter);
}
