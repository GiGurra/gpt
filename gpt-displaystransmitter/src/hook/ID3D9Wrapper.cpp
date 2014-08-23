#include "common.h"
#include "ID3D9Wrapper.h"
#include "ID3D9Wrapper_Device.h"


/******************************************************************************
*
* 
*							MODIFIED DX FCNS
*
******************************************************************************/

HRESULT Direct3D9Wrapper::CreateDevice(UINT Adapter,D3DDEVTYPE DeviceType,HWND hFocusWindow,DWORD BehaviorFlags,D3DPRESENT_PARAMETERS* pPresentationParameters,IDirect3DDevice9** ppReturnedDeviceInterface) {
	IDirect3DDevice9* pDirect3DDevice9;
	HRESULT hRes = m_d3d->CreateDevice( Adapter, DeviceType, hFocusWindow, BehaviorFlags, pPresentationParameters, &pDirect3DDevice9 );
	*ppReturnedDeviceInterface = new Direct3DDevice9Wrapper( pDirect3DDevice9, this, pPresentationParameters );
	return hRes;
}

Direct3D9Wrapper::Direct3D9Wrapper(LPDIRECT3D9 pDirect3D) {
	m_d3d = pDirect3D;
}


/******************************************************************************
*
* 
*						UNMODIFIED DX FCNS
*
******************************************************************************/

Direct3D9Wrapper::~Direct3D9Wrapper() {
}

HRESULT Direct3D9Wrapper::QueryInterface(const IID &riid, void **ppvObj) {
	return m_d3d->QueryInterface(riid, ppvObj);
}

ULONG Direct3D9Wrapper::AddRef() {
	return m_d3d->AddRef();
}

ULONG Direct3D9Wrapper::Release() {
	return m_d3d->Release();
}

HRESULT Direct3D9Wrapper::RegisterSoftwareDevice(void* pInitializeFunction) {
	return m_d3d->RegisterSoftwareDevice(pInitializeFunction);
}

UINT Direct3D9Wrapper::GetAdapterCount() {
	return m_d3d->GetAdapterCount();
}

UINT Direct3D9Wrapper::GetAdapterModeCount(UINT Adapter, D3DFORMAT Format) {
	return m_d3d->GetAdapterModeCount(Adapter, Format);
}

HRESULT Direct3D9Wrapper::GetAdapterIdentifier(UINT Adapter,DWORD Flags,D3DADAPTER_IDENTIFIER9* pIdentifier) {
	return m_d3d->GetAdapterIdentifier(Adapter, Flags, pIdentifier);
}

HRESULT Direct3D9Wrapper::EnumAdapterModes(UINT Adapter,D3DFORMAT Format,UINT Mode,D3DDISPLAYMODE* pMode) {
	return m_d3d->EnumAdapterModes(Adapter, Format, Mode, pMode);
}

HRESULT Direct3D9Wrapper::GetAdapterDisplayMode(UINT Adapter,D3DDISPLAYMODE* pMode) {
	return m_d3d->GetAdapterDisplayMode(Adapter, pMode);
}

HRESULT Direct3D9Wrapper::CheckDeviceType(UINT Adapter,D3DDEVTYPE DevType,D3DFORMAT AdapterFormat,D3DFORMAT BackBufferFormat,BOOL bWindowed) {
	return m_d3d->CheckDeviceType(Adapter, DevType, AdapterFormat, BackBufferFormat, bWindowed);
}

HRESULT Direct3D9Wrapper::CheckDeviceFormat(UINT Adapter,D3DDEVTYPE DeviceType,D3DFORMAT AdapterFormat,DWORD Usage,D3DRESOURCETYPE RType,D3DFORMAT CheckFormat) {
	return m_d3d->CheckDeviceFormat(Adapter, DeviceType, AdapterFormat, Usage, RType, CheckFormat);
}

HRESULT Direct3D9Wrapper::CheckDeviceMultiSampleType(UINT Adapter,D3DDEVTYPE DeviceType,D3DFORMAT SurfaceFormat,BOOL Windowed,D3DMULTISAMPLE_TYPE MultiSampleType,DWORD* pQualityLevels) {
	return m_d3d->CheckDeviceMultiSampleType(Adapter, DeviceType, SurfaceFormat, Windowed, MultiSampleType, pQualityLevels);
}

HRESULT Direct3D9Wrapper::CheckDepthStencilMatch(UINT Adapter,D3DDEVTYPE DeviceType,D3DFORMAT AdapterFormat,D3DFORMAT RenderTargetFormat,D3DFORMAT DepthStencilFormat) {
	return m_d3d->CheckDepthStencilMatch(Adapter, DeviceType, AdapterFormat, RenderTargetFormat, DepthStencilFormat);
}

HRESULT Direct3D9Wrapper::CheckDeviceFormatConversion(UINT Adapter,D3DDEVTYPE DeviceType,D3DFORMAT SourceFormat,D3DFORMAT TargetFormat) {
	return m_d3d->CheckDeviceFormatConversion(Adapter, DeviceType, SourceFormat, TargetFormat);
}

HRESULT Direct3D9Wrapper::GetDeviceCaps(UINT Adapter,D3DDEVTYPE DeviceType,D3DCAPS9* pCaps) {
	return m_d3d->GetDeviceCaps(Adapter, DeviceType, pCaps);
}

HMONITOR Direct3D9Wrapper::GetAdapterMonitor(UINT Adapter) {
	return m_d3d->GetAdapterMonitor(Adapter);
}
