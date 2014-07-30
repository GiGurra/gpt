#include "ID3D9Wrapper_Device.h"

/******************************************************************************
 *
 *
 *							MODIFIED DX FCNS
 *
 ******************************************************************************/

Direct3DDevice9Wrapper::Direct3DDevice9Wrapper(IDirect3DDevice9 *pDirect3DDevice9, IDirect3D9 *pDirect3D9,
		D3DPRESENT_PARAMETERS *pPresentationParameters) {
	logToTestFile("Direct3DDevice9Wrapper::Direct3DDevice9Wrapper");
	Direct3DDevice9 = pDirect3DDevice9;
	Direct3D9 = pDirect3D9;
	pSrcVramcSurface = NULL;
	pBufferVramSurface = NULL;
	pSysRamSurface = NULL;
	comRefCount = 1;
	downloadedThisFrame = false;
}

Direct3DDevice9Wrapper::~Direct3DDevice9Wrapper() {
	logToTestFile("Direct3DDevice9Wrapper::~Direct3DDevice9Wrapper");
}

HRESULT Direct3DDevice9Wrapper::QueryInterface(REFIID riid, void** ppvObj) {
	logToTestFile("Direct3DDevice9Wrapper::QueryInterface");
	HRESULT hRes = Direct3DDevice9->QueryInterface(riid, ppvObj);
	if (hRes == S_OK) {
		*ppvObj = this;
	} else {
		*ppvObj = NULL;
	}
	return hRes;
}

ULONG Direct3DDevice9Wrapper::AddRef() {
	logToTestFile("Direct3DDevice9Wrapper::AddRef");
	comRefCount++;
	return Direct3DDevice9->AddRef();
}

ULONG Direct3DDevice9Wrapper::Release() {
	logToTestFile("Direct3DDevice9Wrapper::Release");
	HRESULT out = Direct3DDevice9->Release();
	comRefCount--;
	if (comRefCount == 0) {
		killTexSharer();
		logToTestFile("Direct3DDevice9Wrapper::, delete this;");
		pSysRamSurface != NULL ? pSysRamSurface->Release() : 0;
		pBufferVramSurface != NULL ? pBufferVramSurface->Release() : 0;
		delete this;
	}
	return out;
}

HRESULT Direct3DDevice9Wrapper::GetDirect3D(IDirect3D9** ppD3D9) {
	logToTestFile("Direct3DDevice9Wrapper::GetDirect3D");
	*ppD3D9 = Direct3D9;
	Direct3D9->AddRef();
	return D3D_OK;
}

HRESULT Direct3DDevice9Wrapper::SetRenderTarget(DWORD RenderTargetIndex, IDirect3DSurface9* pRenderTarget) {
	logToTestFile("Direct3DDevice9Wrapper::SetRenderTarget");
	if (pSrcVramcSurface == NULL) {
		D3DSURFACE_DESC srcVramSurfDescr;
		pRenderTarget->GetDesc(&srcVramSurfDescr);
		if ((srcVramSurfDescr.Width == 600 && srcVramSurfDescr.Height == 600)
				|| (srcVramSurfDescr.Width == 1200 && srcVramSurfDescr.Height == 1200)) {
			pSrcVramcSurface = pRenderTarget;
			CreateRenderTarget(srcVramSurfDescr.Width, srcVramSurfDescr.Height, srcVramSurfDescr.Format,
					srcVramSurfDescr.MultiSampleType, srcVramSurfDescr.MultiSampleQuality, false, &pBufferVramSurface, NULL);
			if (pSysRamSurface == NULL) {
				CreateOffscreenPlainSurface(srcVramSurfDescr.Width, srcVramSurfDescr.Height, srcVramSurfDescr.Format,
						D3DPOOL_SYSTEMMEM, &pSysRamSurface, NULL);
				D3DLOCKED_RECT rect;
				pSysRamSurface->LockRect(&rect, NULL, D3DLOCK_NO_DIRTY_UPDATE | D3DLOCK_READONLY);
				startTexSharer(rect.pBits, srcVramSurfDescr.Width, srcVramSurfDescr.Height, 4);
				pSysRamSurface->UnlockRect();
			}
		}
	} else if (!downloadedThisFrame && pRenderTarget == pSrcVramcSurface) {
		downloadedThisFrame = true;
		GetRenderTargetData(pBufferVramSurface, pSysRamSurface);
	}
	return Direct3DDevice9->SetRenderTarget(RenderTargetIndex, pRenderTarget);
}

HRESULT Direct3DDevice9Wrapper::Reset(D3DPRESENT_PARAMETERS* pPresentationParameters) {
	if (pSrcVramcSurface != NULL) {
		pBufferVramSurface != NULL ? pBufferVramSurface->Release() : 0;
		pBufferVramSurface = NULL;
		pSrcVramcSurface = NULL;
	}
	logToTestFile("Direct3DDevice9Wrapper::Reset");
	return Direct3DDevice9->Reset(pPresentationParameters);
}

HRESULT Direct3DDevice9Wrapper::Present(CONST RECT* pSourceRect, CONST RECT* pDestRect, HWND hDestWindowOverride,
		CONST RGNDATA* pDirtyRegion) {
	logToTestFile("Direct3DDevice9Wrapper::Present");
	HRESULT out = Direct3DDevice9->Present(pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion);
	if (pSrcVramcSurface != NULL) {
		StretchRect(pSrcVramcSurface, NULL, pBufferVramSurface, NULL, D3DTEXF_NONE);
		downloadedThisFrame = false;
	}
	return out;
}

/******************************************************************************
 *
 *
 *						UNMODIFIED DX FCNS
 *
 ******************************************************************************/

HRESULT Direct3DDevice9Wrapper::BeginScene() {
	logToTestFile("Direct3DDevice9Wrapper::BeginScene");
	return Direct3DDevice9->BeginScene();
}

HRESULT Direct3DDevice9Wrapper::TestCooperativeLevel() {
	logToTestFile("Direct3DDevice9Wrapper::TestCooperativeLevel");
	return Direct3DDevice9->TestCooperativeLevel();
}

UINT Direct3DDevice9Wrapper::GetAvailableTextureMem() {
	logToTestFile("Direct3DDevice9Wrapper::GetAvailableTextureMem");
	return Direct3DDevice9->GetAvailableTextureMem();
}

HRESULT Direct3DDevice9Wrapper::EvictManagedResources() {
	logToTestFile("Direct3DDevice9Wrapper::EvictManagedResources");
	return Direct3DDevice9->EvictManagedResources();
}

HRESULT Direct3DDevice9Wrapper::GetDeviceCaps(D3DCAPS9* pCaps) {
	logToTestFile("Direct3DDevice9Wrapper::GetDeviceCaps");
	return Direct3DDevice9->GetDeviceCaps(pCaps);
}

HRESULT Direct3DDevice9Wrapper::GetDisplayMode(UINT iSwapChain, D3DDISPLAYMODE* pMode) {
	logToTestFile("Direct3DDevice9Wrapper::GetDisplayMode");
	return Direct3DDevice9->GetDisplayMode(iSwapChain, pMode);
}

HRESULT Direct3DDevice9Wrapper::GetCreationParameters(D3DDEVICE_CREATION_PARAMETERS *pParameters) {
	logToTestFile("Direct3DDevice9Wrapper::GetCreationParameters");
	return Direct3DDevice9->GetCreationParameters(pParameters);
}

HRESULT Direct3DDevice9Wrapper::SetCursorProperties(UINT XHotSpot, UINT YHotSpot, IDirect3DSurface9* pCursorBitmap) {
	logToTestFile("Direct3DDevice9Wrapper::SetCursorProperties");
	return Direct3DDevice9->SetCursorProperties(XHotSpot, YHotSpot, pCursorBitmap);
}

void Direct3DDevice9Wrapper::SetCursorPosition(int X, int Y, DWORD Flags) {
	logToTestFile("Direct3DDevice9Wrapper::SetCursorPosition");
	Direct3DDevice9->SetCursorPosition(X, Y, Flags);
}

BOOL Direct3DDevice9Wrapper::ShowCursor(BOOL bShow) {
	logToTestFile("Direct3DDevice9Wrapper::ShowCursor");
	return Direct3DDevice9->ShowCursor(bShow);
}

HRESULT Direct3DDevice9Wrapper::CreateAdditionalSwapChain(D3DPRESENT_PARAMETERS* pPresentationParameters,
		IDirect3DSwapChain9** pSwapChain) {
	logToTestFile("Direct3DDevice9Wrapper::CreateAdditionalSwapChain");
	return Direct3DDevice9->CreateAdditionalSwapChain(pPresentationParameters, pSwapChain);
}

HRESULT Direct3DDevice9Wrapper::GetSwapChain(UINT iSwapChain, IDirect3DSwapChain9** pSwapChain) {
	logToTestFile("Direct3DDevice9Wrapper::GetSwapChain");
	return Direct3DDevice9->GetSwapChain(iSwapChain, pSwapChain);
}

UINT Direct3DDevice9Wrapper::GetNumberOfSwapChains() {
	logToTestFile("Direct3DDevice9Wrapper::GetNumberOfSwapChains");
	return Direct3DDevice9->GetNumberOfSwapChains();
}

HRESULT Direct3DDevice9Wrapper::GetBackBuffer(UINT iSwapChain, UINT iBackBuffer, D3DBACKBUFFER_TYPE Type,
		IDirect3DSurface9** ppBackBuffer) {
	logToTestFile("Direct3DDevice9Wrapper::GetBackBuffer");
	return Direct3DDevice9->GetBackBuffer(iSwapChain, iBackBuffer, Type, ppBackBuffer);
}

HRESULT Direct3DDevice9Wrapper::GetRasterStatus(UINT iSwapChain, D3DRASTER_STATUS* pRasterStatus) {
	logToTestFile("Direct3DDevice9Wrapper::GetRasterStatus");
	return Direct3DDevice9->GetRasterStatus(iSwapChain, pRasterStatus);
}

HRESULT Direct3DDevice9Wrapper::SetDialogBoxMode(BOOL bEnableDialogs) {
	logToTestFile("Direct3DDevice9Wrapper::SetDialogBoxMode");
	return Direct3DDevice9->SetDialogBoxMode(bEnableDialogs);
}

void Direct3DDevice9Wrapper::SetGammaRamp(UINT iSwapChain, DWORD Flags, CONST D3DGAMMARAMP* pRamp) {
	logToTestFile("Direct3DDevice9Wrapper::SetGammaRamp");
	Direct3DDevice9->SetGammaRamp(iSwapChain, Flags, pRamp);
}

void Direct3DDevice9Wrapper::GetGammaRamp(UINT iSwapChain, D3DGAMMARAMP* pRamp) {
	logToTestFile("Direct3DDevice9Wrapper::GetGammaRamp");
	Direct3DDevice9->GetGammaRamp(iSwapChain, pRamp);
}

HRESULT Direct3DDevice9Wrapper::CreateTexture(UINT Width, UINT Height, UINT Levels, DWORD Usage, D3DFORMAT Format,
		D3DPOOL Pool, IDirect3DTexture9** ppTexture, HANDLE* pSharedHandle) {
	logToTestFile("Direct3DDevice9Wrapper::CreateTexture");
	return Direct3DDevice9->CreateTexture(Width, Height, Levels, Usage, Format, Pool, ppTexture, pSharedHandle);
}

HRESULT Direct3DDevice9Wrapper::CreateVolumeTexture(UINT Width, UINT Height, UINT Depth, UINT Levels, DWORD Usage,
		D3DFORMAT Format, D3DPOOL Pool, IDirect3DVolumeTexture9** ppVolumeTexture, HANDLE* pSharedHandle) {
	logToTestFile("Direct3DDevice9Wrapper::CreateVolumeTexture");
	return Direct3DDevice9->CreateVolumeTexture(Width, Height, Depth, Levels, Usage, Format, Pool, ppVolumeTexture,
			pSharedHandle);
}

HRESULT Direct3DDevice9Wrapper::CreateCubeTexture(UINT EdgeLength, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool,
		IDirect3DCubeTexture9** ppCubeTexture, HANDLE* pSharedHandle) {
	logToTestFile("Direct3DDevice9Wrapper::CreateCubeTexture");
	return Direct3DDevice9->CreateCubeTexture(EdgeLength, Levels, Usage, Format, Pool, ppCubeTexture, pSharedHandle);
}

HRESULT Direct3DDevice9Wrapper::CreateVertexBuffer(UINT Length, DWORD Usage, DWORD FVF, D3DPOOL Pool,
		IDirect3DVertexBuffer9** ppVertexBuffer, HANDLE* pSharedHandle) {
	logToTestFile("Direct3DDevice9Wrapper::CreateVertexBuffer");
	return Direct3DDevice9->CreateVertexBuffer(Length, Usage, FVF, Pool, ppVertexBuffer, pSharedHandle);
}

HRESULT Direct3DDevice9Wrapper::CreateIndexBuffer(UINT Length, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool,
		IDirect3DIndexBuffer9** ppIndexBuffer, HANDLE* pSharedHandle) {
	logToTestFile("Direct3DDevice9Wrapper::CreateIndexBuffer");
	return Direct3DDevice9->CreateIndexBuffer(Length, Usage, Format, Pool, ppIndexBuffer, pSharedHandle);
}

HRESULT Direct3DDevice9Wrapper::CreateRenderTarget(UINT Width, UINT Height, D3DFORMAT Format,
		D3DMULTISAMPLE_TYPE MultiSample, DWORD MultisampleQuality, BOOL Lockable, IDirect3DSurface9** ppSurface,
		HANDLE* pSharedHandle) {
	logToTestFile("Direct3DDevice9Wrapper::CreateRenderTarget");
	return Direct3DDevice9->CreateRenderTarget(Width, Height, Format, MultiSample, MultisampleQuality, Lockable, ppSurface,
			pSharedHandle);
}

HRESULT Direct3DDevice9Wrapper::CreateDepthStencilSurface(UINT Width, UINT Height, D3DFORMAT Format,
		D3DMULTISAMPLE_TYPE MultiSample, DWORD MultisampleQuality, BOOL Discard, IDirect3DSurface9** ppSurface,
		HANDLE* pSharedHandle) {
	logToTestFile("Direct3DDevice9Wrapper::CreateDepthStencilSurface");
	return Direct3DDevice9->CreateDepthStencilSurface(Width, Height, Format, MultiSample, MultisampleQuality, Discard,
			ppSurface, pSharedHandle);
}

HRESULT Direct3DDevice9Wrapper::UpdateSurface(IDirect3DSurface9* pSourceSurface, CONST RECT* pSourceRect,
		IDirect3DSurface9* pDestinationSurface, CONST POINT* pDestPoint) {
	logToTestFile("Direct3DDevice9Wrapper::UpdateSurface");
	return Direct3DDevice9->UpdateSurface(pSourceSurface, pSourceRect, pDestinationSurface, pDestPoint);
}

HRESULT Direct3DDevice9Wrapper::UpdateTexture(IDirect3DBaseTexture9* pSourceTexture,
		IDirect3DBaseTexture9* pDestinationTexture) {
	logToTestFile("Direct3DDevice9Wrapper::UpdateTexture");
	return Direct3DDevice9->UpdateTexture(pSourceTexture, pDestinationTexture);
}

HRESULT Direct3DDevice9Wrapper::GetRenderTargetData(IDirect3DSurface9* pRenderTarget, IDirect3DSurface9* pDestSurface) {
	logToTestFile("Direct3DDevice9Wrapper::GetRenderTargetData");
	return Direct3DDevice9->GetRenderTargetData(pRenderTarget, pDestSurface);
}

HRESULT Direct3DDevice9Wrapper::GetFrontBufferData(UINT iSwapChain, IDirect3DSurface9* pDestSurface) {
	logToTestFile("Direct3DDevice9Wrapper::GetFrontBufferData");
	return Direct3DDevice9->GetFrontBufferData(iSwapChain, pDestSurface);
}

HRESULT Direct3DDevice9Wrapper::StretchRect(IDirect3DSurface9* pSourceSurface, CONST RECT* pSourceRect,
		IDirect3DSurface9* pDestSurface, CONST RECT* pDestRect, D3DTEXTUREFILTERTYPE Filter) {
	logToTestFile("Direct3DDevice9Wrapper::StretchRect");
	return Direct3DDevice9->StretchRect(pSourceSurface, pSourceRect, pDestSurface, pDestRect, Filter);
}

HRESULT Direct3DDevice9Wrapper::ColorFill(IDirect3DSurface9* pSurface, CONST RECT* pRect, D3DCOLOR color) {
	logToTestFile("Direct3DDevice9Wrapper::ColorFill");
	return Direct3DDevice9->ColorFill(pSurface, pRect, color);
}

HRESULT Direct3DDevice9Wrapper::CreateOffscreenPlainSurface(UINT Width, UINT Height, D3DFORMAT Format, D3DPOOL Pool,
		IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle) {
	logToTestFile("\nDirect3DDevice9Wrapper::CreateOffscreenPlainSurface:");
	return Direct3DDevice9->CreateOffscreenPlainSurface(Width, Height, Format, Pool, ppSurface, pSharedHandle);
}

HRESULT Direct3DDevice9Wrapper::GetRenderTarget(DWORD RenderTargetIndex, IDirect3DSurface9** ppRenderTarget) {
	logToTestFile("Direct3DDevice9Wrapper::GetRenderTarget");
	return Direct3DDevice9->GetRenderTarget(RenderTargetIndex, ppRenderTarget);
}

HRESULT Direct3DDevice9Wrapper::SetDepthStencilSurface(IDirect3DSurface9* pNewZStencil) {
	logToTestFile("Direct3DDevice9Wrapper::SetDepthStencilSurface");
	return Direct3DDevice9->SetDepthStencilSurface(pNewZStencil);
}

HRESULT Direct3DDevice9Wrapper::GetDepthStencilSurface(IDirect3DSurface9** ppZStencilSurface) {
	logToTestFile("Direct3DDevice9Wrapper::GetDepthStencilSurface");
	return Direct3DDevice9->GetDepthStencilSurface(ppZStencilSurface);
}

HRESULT Direct3DDevice9Wrapper::EndScene() {
	logToTestFile("Direct3DDevice9Wrapper::EndScene");
	return Direct3DDevice9->EndScene();
}

HRESULT Direct3DDevice9Wrapper::Clear(DWORD Count, CONST D3DRECT* pRects, DWORD Flags, D3DCOLOR Color, float Z,
		DWORD Stencil) {
	logToTestFile("Direct3DDevice9Wrapper::Clear");
	return Direct3DDevice9->Clear(Count, pRects, Flags, Color, Z, Stencil);
}

HRESULT Direct3DDevice9Wrapper::SetTransform(D3DTRANSFORMSTATETYPE State, CONST D3DMATRIX* pMatrix) {
	logToTestFile("Direct3DDevice9Wrapper::SetTransform");
	return Direct3DDevice9->SetTransform(State, pMatrix);
}

HRESULT Direct3DDevice9Wrapper::GetTransform(D3DTRANSFORMSTATETYPE State, D3DMATRIX* pMatrix) {
	logToTestFile("Direct3DDevice9Wrapper::GetTransform");
	return Direct3DDevice9->GetTransform(State, pMatrix);
}

HRESULT Direct3DDevice9Wrapper::MultiplyTransform(D3DTRANSFORMSTATETYPE param0, CONST D3DMATRIX * param1) {
	logToTestFile("Direct3DDevice9Wrapper::MultiplyTransform");
	return Direct3DDevice9->MultiplyTransform(param0, param1);
}

HRESULT Direct3DDevice9Wrapper::SetViewport(CONST D3DVIEWPORT9* pViewport) {
	logToTestFile("Direct3DDevice9Wrapper::SetViewport");
	return Direct3DDevice9->SetViewport(pViewport);
}

HRESULT Direct3DDevice9Wrapper::GetViewport(D3DVIEWPORT9* pViewport) {
	logToTestFile("Direct3DDevice9Wrapper::GetViewport");
	return Direct3DDevice9->GetViewport(pViewport);
}

HRESULT Direct3DDevice9Wrapper::SetMaterial(CONST D3DMATERIAL9* pMaterial) {
	logToTestFile("Direct3DDevice9Wrapper::SetMaterial");
	return Direct3DDevice9->SetMaterial(pMaterial);
}

HRESULT Direct3DDevice9Wrapper::GetMaterial(D3DMATERIAL9* pMaterial) {
	logToTestFile("Direct3DDevice9Wrapper::GetMaterial");
	return Direct3DDevice9->GetMaterial(pMaterial);
}

HRESULT Direct3DDevice9Wrapper::SetLight(DWORD Index, CONST D3DLIGHT9* param1) {
	logToTestFile("Direct3DDevice9Wrapper::SetLight");
	return Direct3DDevice9->SetLight(Index, param1);
}

HRESULT Direct3DDevice9Wrapper::GetLight(DWORD Index, D3DLIGHT9* param1) {
	logToTestFile("Direct3DDevice9Wrapper::GetLight");
	return Direct3DDevice9->GetLight(Index, param1);
}

HRESULT Direct3DDevice9Wrapper::LightEnable(DWORD Index, BOOL Enable) {
	logToTestFile("Direct3DDevice9Wrapper::LightEnable");
	return Direct3DDevice9->LightEnable(Index, Enable);
}

HRESULT Direct3DDevice9Wrapper::GetLightEnable(DWORD Index, BOOL* pEnable) {
	logToTestFile("Direct3DDevice9Wrapper::GetLightEnable");
	return Direct3DDevice9->GetLightEnable(Index, pEnable);
}

HRESULT Direct3DDevice9Wrapper::SetClipPlane(DWORD Index, CONST float* pPlane) {
	logToTestFile("Direct3DDevice9Wrapper::SetClipPlane");
	return Direct3DDevice9->SetClipPlane(Index, pPlane);
}

HRESULT Direct3DDevice9Wrapper::GetClipPlane(DWORD Index, float* pPlane) {
	logToTestFile("Direct3DDevice9Wrapper::GetClipPlane");
	return Direct3DDevice9->GetClipPlane(Index, pPlane);
}

HRESULT Direct3DDevice9Wrapper::SetRenderState(D3DRENDERSTATETYPE State, DWORD Value) {
	logToTestFile("Direct3DDevice9Wrapper::SetRenderState");
	return Direct3DDevice9->SetRenderState(State, Value);
}

HRESULT Direct3DDevice9Wrapper::GetRenderState(D3DRENDERSTATETYPE State, DWORD* pValue) {
	logToTestFile("Direct3DDevice9Wrapper::GetRenderState");
	return Direct3DDevice9->GetRenderState(State, pValue);
}

HRESULT Direct3DDevice9Wrapper::CreateStateBlock(D3DSTATEBLOCKTYPE Type, IDirect3DStateBlock9** ppSB) {
	logToTestFile("Direct3DDevice9Wrapper::CreateStateBlock");
	return Direct3DDevice9->CreateStateBlock(Type, ppSB);
}

HRESULT Direct3DDevice9Wrapper::BeginStateBlock() {
	logToTestFile("Direct3DDevice9Wrapper::BeginStateBlock");
	return Direct3DDevice9->BeginStateBlock();
}

HRESULT Direct3DDevice9Wrapper::EndStateBlock(IDirect3DStateBlock9** ppSB) {
	logToTestFile("Direct3DDevice9Wrapper::EndStateBlock");
	return Direct3DDevice9->EndStateBlock(ppSB);
}

HRESULT Direct3DDevice9Wrapper::SetClipStatus(CONST D3DCLIPSTATUS9* pClipStatus) {
	logToTestFile("Direct3DDevice9Wrapper::SetClipStatus");
	return Direct3DDevice9->SetClipStatus(pClipStatus);
}

HRESULT Direct3DDevice9Wrapper::GetClipStatus(D3DCLIPSTATUS9* pClipStatus) {
	logToTestFile("Direct3DDevice9Wrapper::GetClipStatus");
	return Direct3DDevice9->GetClipStatus(pClipStatus);
}

HRESULT Direct3DDevice9Wrapper::GetTexture(DWORD Stage, IDirect3DBaseTexture9** ppTexture) {
	logToTestFile("Direct3DDevice9Wrapper::GetTexture");
	return Direct3DDevice9->GetTexture(Stage, ppTexture);
}

HRESULT Direct3DDevice9Wrapper::SetTexture(DWORD Stage, IDirect3DBaseTexture9* pTexture) {
	logToTestFile("Direct3DDevice9Wrapper::SetTexture");
	return Direct3DDevice9->SetTexture(Stage, pTexture);
}

HRESULT Direct3DDevice9Wrapper::GetTextureStageState(DWORD Stage, D3DTEXTURESTAGESTATETYPE Type, DWORD* pValue) {
	logToTestFile("Direct3DDevice9Wrapper::GetTextureStageState");
	return Direct3DDevice9->GetTextureStageState(Stage, Type, pValue);
}

HRESULT Direct3DDevice9Wrapper::SetTextureStageState(DWORD Stage, D3DTEXTURESTAGESTATETYPE Type, DWORD Value) {
	logToTestFile("Direct3DDevice9Wrapper::SetTextureStageState");
	return Direct3DDevice9->SetTextureStageState(Stage, Type, Value);
}

HRESULT Direct3DDevice9Wrapper::GetSamplerState(DWORD Sampler, D3DSAMPLERSTATETYPE Type, DWORD* pValue) {
	logToTestFile("Direct3DDevice9Wrapper::GetSamplerState");
	return Direct3DDevice9->GetSamplerState(Sampler, Type, pValue);
}

HRESULT Direct3DDevice9Wrapper::SetSamplerState(DWORD Sampler, D3DSAMPLERSTATETYPE Type, DWORD Value) {
	logToTestFile("Direct3DDevice9Wrapper::SetSamplerState");
	return Direct3DDevice9->SetSamplerState(Sampler, Type, Value);
}

HRESULT Direct3DDevice9Wrapper::ValidateDevice(DWORD* pNumPasses) {
	logToTestFile("Direct3DDevice9Wrapper::ValidateDevice");
	return Direct3DDevice9->ValidateDevice(pNumPasses);
}

HRESULT Direct3DDevice9Wrapper::SetPaletteEntries(UINT PaletteNumber, CONST PALETTEENTRY* pEntries) {
	logToTestFile("Direct3DDevice9Wrapper::SetPaletteEntries");
	return Direct3DDevice9->SetPaletteEntries(PaletteNumber, pEntries);
}

HRESULT Direct3DDevice9Wrapper::GetPaletteEntries(UINT PaletteNumber, PALETTEENTRY* pEntries) {
	logToTestFile("Direct3DDevice9Wrapper::GetPaletteEntries");
	return Direct3DDevice9->GetPaletteEntries(PaletteNumber, pEntries);
}

HRESULT Direct3DDevice9Wrapper::SetCurrentTexturePalette(UINT PaletteNumber) {
	logToTestFile("Direct3DDevice9Wrapper::SetCurrentTexturePalette");
	return Direct3DDevice9->SetCurrentTexturePalette(PaletteNumber);
}

HRESULT Direct3DDevice9Wrapper::GetCurrentTexturePalette(UINT *PaletteNumber) {
	logToTestFile("Direct3DDevice9Wrapper::GetCurrentTexturePalette");
	return Direct3DDevice9->GetCurrentTexturePalette(PaletteNumber);
}

HRESULT Direct3DDevice9Wrapper::SetScissorRect(CONST RECT* pRect) {
	logToTestFile("Direct3DDevice9Wrapper::SetScissorRect");
	return Direct3DDevice9->SetScissorRect(pRect);
}

HRESULT Direct3DDevice9Wrapper::GetScissorRect(RECT* pRect) {
	logToTestFile("Direct3DDevice9Wrapper::GetScissorRect");
	return Direct3DDevice9->GetScissorRect(pRect);
}

HRESULT Direct3DDevice9Wrapper::SetSoftwareVertexProcessing(BOOL bSoftware) {
	logToTestFile("Direct3DDevice9Wrapper::SetSoftwareVertexProcessing");
	return Direct3DDevice9->SetSoftwareVertexProcessing(bSoftware);
}

BOOL Direct3DDevice9Wrapper::GetSoftwareVertexProcessing() {
	logToTestFile("Direct3DDevice9Wrapper::GetSoftwareVertexProcessing");
	return Direct3DDevice9->GetSoftwareVertexProcessing();
}

HRESULT Direct3DDevice9Wrapper::SetNPatchMode(float nSegments) {
	logToTestFile("Direct3DDevice9Wrapper::SetNPatchMode");
	return Direct3DDevice9->SetNPatchMode(nSegments);
}

float Direct3DDevice9Wrapper::GetNPatchMode() {
	logToTestFile("Direct3DDevice9Wrapper::GetNPatchMode");
	return Direct3DDevice9->GetNPatchMode();
}

HRESULT Direct3DDevice9Wrapper::DrawPrimitive(D3DPRIMITIVETYPE PrimitiveType, UINT StartVertex, UINT PrimitiveCount) {
	logToTestFile("Direct3DDevice9Wrapper::DrawPrimitive");
	return Direct3DDevice9->DrawPrimitive(PrimitiveType, StartVertex, PrimitiveCount);
}

HRESULT Direct3DDevice9Wrapper::DrawIndexedPrimitive(D3DPRIMITIVETYPE param0, INT BaseVertexIndex, UINT MinVertexIndex,
		UINT NumVertices, UINT startIndex, UINT primCount) {
	logToTestFile("Direct3DDevice9Wrapper::DrawIndexedPrimitive");
	return Direct3DDevice9->DrawIndexedPrimitive(param0, BaseVertexIndex, MinVertexIndex, NumVertices, startIndex, primCount);
}

HRESULT Direct3DDevice9Wrapper::DrawPrimitiveUP(D3DPRIMITIVETYPE PrimitiveType, UINT PrimitiveCount,
		CONST void* pVertexStreamZeroData, UINT VertexStreamZeroStride) {
	logToTestFile("Direct3DDevice9Wrapper::DrawPrimitiveUP");
	return Direct3DDevice9->DrawPrimitiveUP(PrimitiveType, PrimitiveCount, pVertexStreamZeroData, VertexStreamZeroStride);
}

HRESULT Direct3DDevice9Wrapper::DrawIndexedPrimitiveUP(D3DPRIMITIVETYPE PrimitiveType, UINT MinVertexIndex, UINT NumVertices,
		UINT PrimitiveCount, CONST void* pIndexData, D3DFORMAT IndexDataFormat, CONST void* pVertexStreamZeroData,
		UINT VertexStreamZeroStride) {
	logToTestFile("Direct3DDevice9Wrapper::DrawIndexedPrimitiveUP");
	return Direct3DDevice9->DrawIndexedPrimitiveUP(PrimitiveType, MinVertexIndex, NumVertices, PrimitiveCount, pIndexData,
			IndexDataFormat, pVertexStreamZeroData, VertexStreamZeroStride);
}

HRESULT Direct3DDevice9Wrapper::ProcessVertices(UINT SrcStartIndex, UINT DestIndex, UINT VertexCount,
		IDirect3DVertexBuffer9* pDestBuffer, IDirect3DVertexDeclaration9* pVertexDecl, DWORD Flags) {
	logToTestFile("Direct3DDevice9Wrapper::ProcessVertices");
	return Direct3DDevice9->ProcessVertices(SrcStartIndex, DestIndex, VertexCount, pDestBuffer, pVertexDecl, Flags);
}

HRESULT Direct3DDevice9Wrapper::CreateVertexDeclaration(CONST D3DVERTEXELEMENT9* pVertexElements,
		IDirect3DVertexDeclaration9** ppDecl) {
	logToTestFile("Direct3DDevice9Wrapper::CreateVertexDeclaration");
	return Direct3DDevice9->CreateVertexDeclaration(pVertexElements, ppDecl);
}

HRESULT Direct3DDevice9Wrapper::SetVertexDeclaration(IDirect3DVertexDeclaration9* pDecl) {
	logToTestFile("Direct3DDevice9Wrapper::SetVertexDeclaration");
	return Direct3DDevice9->SetVertexDeclaration(pDecl);
}

HRESULT Direct3DDevice9Wrapper::GetVertexDeclaration(IDirect3DVertexDeclaration9** ppDecl) {
	logToTestFile("Direct3DDevice9Wrapper::GetVertexDeclaration");
	return Direct3DDevice9->GetVertexDeclaration(ppDecl);
}

HRESULT Direct3DDevice9Wrapper::SetFVF(DWORD FVF) {
	logToTestFile("Direct3DDevice9Wrapper::SetFVF");
	return Direct3DDevice9->SetFVF(FVF);
}

HRESULT Direct3DDevice9Wrapper::GetFVF(DWORD* pFVF) {
	logToTestFile("Direct3DDevice9Wrapper::GetFVF");
	return Direct3DDevice9->GetFVF(pFVF);
}

HRESULT Direct3DDevice9Wrapper::CreateVertexShader(CONST DWORD* pFunction, IDirect3DVertexShader9** ppShader) {
	logToTestFile("Direct3DDevice9Wrapper::CreateVertexShader");
	return Direct3DDevice9->CreateVertexShader(pFunction, ppShader);
}

HRESULT Direct3DDevice9Wrapper::SetVertexShader(IDirect3DVertexShader9* pShader) {
	logToTestFile("Direct3DDevice9Wrapper::SetVertexShader");
	return Direct3DDevice9->SetVertexShader(pShader);
}

HRESULT Direct3DDevice9Wrapper::GetVertexShader(IDirect3DVertexShader9** ppShader) {
	logToTestFile("Direct3DDevice9Wrapper::GetVertexShader");
	return Direct3DDevice9->GetVertexShader(ppShader);
}

HRESULT Direct3DDevice9Wrapper::SetVertexShaderConstantF(UINT StartRegister, CONST float* pConstantData,
		UINT Vector4fCount) {
	logToTestFile("Direct3DDevice9Wrapper::SetVertexShaderConstantF");
	return Direct3DDevice9->SetVertexShaderConstantF(StartRegister, pConstantData, Vector4fCount);
}

HRESULT Direct3DDevice9Wrapper::GetVertexShaderConstantF(UINT StartRegister, float* pConstantData, UINT Vector4fCount) {
	logToTestFile("Direct3DDevice9Wrapper::GetVertexShaderConstantF");
	return Direct3DDevice9->GetVertexShaderConstantF(StartRegister, pConstantData, Vector4fCount);
}

HRESULT Direct3DDevice9Wrapper::SetVertexShaderConstantI(UINT StartRegister, CONST int* pConstantData, UINT Vector4iCount) {
	logToTestFile("Direct3DDevice9Wrapper::SetVertexShaderConstantI");
	return Direct3DDevice9->SetVertexShaderConstantI(StartRegister, pConstantData, Vector4iCount);
}

HRESULT Direct3DDevice9Wrapper::GetVertexShaderConstantI(UINT StartRegister, int* pConstantData, UINT Vector4iCount) {
	logToTestFile("Direct3DDevice9Wrapper::GetVertexShaderConstantI");
	return Direct3DDevice9->GetVertexShaderConstantI(StartRegister, pConstantData, Vector4iCount);
}

HRESULT Direct3DDevice9Wrapper::SetVertexShaderConstantB(UINT StartRegister, CONST BOOL* pConstantData, UINT BoolCount) {
	logToTestFile("Direct3DDevice9Wrapper::SetVertexShaderConstantB");
	return Direct3DDevice9->SetVertexShaderConstantB(StartRegister, pConstantData, BoolCount);
}

HRESULT Direct3DDevice9Wrapper::GetVertexShaderConstantB(UINT StartRegister, BOOL* pConstantData, UINT BoolCount) {
	logToTestFile("Direct3DDevice9Wrapper::GetVertexShaderConstantB");
	return Direct3DDevice9->GetVertexShaderConstantB(StartRegister, pConstantData, BoolCount);
}

HRESULT Direct3DDevice9Wrapper::SetStreamSource(UINT StreamNumber, IDirect3DVertexBuffer9* pStreamData, UINT OffsetInBytes,
		UINT Stride) {
	logToTestFile("Direct3DDevice9Wrapper::SetStreamSource");
	if (StreamNumber == 0)
		m_Stride = Stride;
	return Direct3DDevice9->SetStreamSource(StreamNumber, pStreamData, OffsetInBytes, Stride);
}

HRESULT Direct3DDevice9Wrapper::GetStreamSource(UINT StreamNumber, IDirect3DVertexBuffer9** ppStreamData,
		UINT* pOffsetInBytes, UINT* pStride) {
	logToTestFile("Direct3DDevice9Wrapper::GetStreamSource");
	return Direct3DDevice9->GetStreamSource(StreamNumber, ppStreamData, pOffsetInBytes, pStride);
}

HRESULT Direct3DDevice9Wrapper::SetStreamSourceFreq(UINT StreamNumber, UINT Setting) {
	logToTestFile("Direct3DDevice9Wrapper::SetStreamSourceFreq");
	return Direct3DDevice9->SetStreamSourceFreq(StreamNumber, Setting);
}

HRESULT Direct3DDevice9Wrapper::GetStreamSourceFreq(UINT StreamNumber, UINT* pSetting) {
	logToTestFile("Direct3DDevice9Wrapper::GetStreamSourceFreq");
	return Direct3DDevice9->GetStreamSourceFreq(StreamNumber, pSetting);
}

HRESULT Direct3DDevice9Wrapper::SetIndices(IDirect3DIndexBuffer9* pIndexData) {
	logToTestFile("Direct3DDevice9Wrapper::SetIndices");
	return Direct3DDevice9->SetIndices(pIndexData);
}

HRESULT Direct3DDevice9Wrapper::GetIndices(IDirect3DIndexBuffer9** ppIndexData) {
	logToTestFile("Direct3DDevice9Wrapper::GetIndices");
	return Direct3DDevice9->GetIndices(ppIndexData);
}

HRESULT Direct3DDevice9Wrapper::CreatePixelShader(CONST DWORD* pFunction, IDirect3DPixelShader9** ppShader) {
	logToTestFile("Direct3DDevice9Wrapper::CreatePixelShader");
	return Direct3DDevice9->CreatePixelShader(pFunction, ppShader);
}

HRESULT Direct3DDevice9Wrapper::SetPixelShader(IDirect3DPixelShader9* pShader) {
	logToTestFile("Direct3DDevice9Wrapper::SetPixelShader");
	return Direct3DDevice9->SetPixelShader(pShader);
}

HRESULT Direct3DDevice9Wrapper::GetPixelShader(IDirect3DPixelShader9** ppShader) {
	logToTestFile("Direct3DDevice9Wrapper::GetPixelShader");
	return Direct3DDevice9->GetPixelShader(ppShader);
}

HRESULT Direct3DDevice9Wrapper::SetPixelShaderConstantF(UINT StartRegister, CONST float* pConstantData, UINT Vector4fCount) {
	logToTestFile("Direct3DDevice9Wrapper::SetPixelShaderConstantF");
	return Direct3DDevice9->SetPixelShaderConstantF(StartRegister, pConstantData, Vector4fCount);
}

HRESULT Direct3DDevice9Wrapper::GetPixelShaderConstantF(UINT StartRegister, float* pConstantData, UINT Vector4fCount) {
	logToTestFile("Direct3DDevice9Wrapper::GetPixelShaderConstantF");
	return Direct3DDevice9->GetPixelShaderConstantF(StartRegister, pConstantData, Vector4fCount);
}

HRESULT Direct3DDevice9Wrapper::SetPixelShaderConstantI(UINT StartRegister, CONST int* pConstantData, UINT Vector4iCount) {
	logToTestFile("Direct3DDevice9Wrapper::SetPixelShaderConstantI");
	return Direct3DDevice9->SetPixelShaderConstantI(StartRegister, pConstantData, Vector4iCount);
}

HRESULT Direct3DDevice9Wrapper::GetPixelShaderConstantI(UINT StartRegister, int* pConstantData, UINT Vector4iCount) {
	logToTestFile("Direct3DDevice9Wrapper::GetPixelShaderConstantI");
	return Direct3DDevice9->GetPixelShaderConstantI(StartRegister, pConstantData, Vector4iCount);
}

HRESULT Direct3DDevice9Wrapper::SetPixelShaderConstantB(UINT StartRegister, CONST BOOL* pConstantData, UINT BoolCount) {
	logToTestFile("Direct3DDevice9Wrapper::SetPixelShaderConstantB");
	return Direct3DDevice9->SetPixelShaderConstantB(StartRegister, pConstantData, BoolCount);
}

HRESULT Direct3DDevice9Wrapper::GetPixelShaderConstantB(UINT StartRegister, BOOL* pConstantData, UINT BoolCount) {
	logToTestFile("Direct3DDevice9Wrapper::GetPixelShaderConstantB");
	return Direct3DDevice9->GetPixelShaderConstantB(StartRegister, pConstantData, BoolCount);
}

HRESULT Direct3DDevice9Wrapper::DrawRectPatch(UINT Handle, CONST float* pNumSegs, CONST D3DRECTPATCH_INFO* pRectPatchInfo) {
	logToTestFile("Direct3DDevice9Wrapper::DrawRectPatch");
	return Direct3DDevice9->DrawRectPatch(Handle, pNumSegs, pRectPatchInfo);
}

HRESULT Direct3DDevice9Wrapper::DrawTriPatch(UINT Handle, CONST float* pNumSegs, CONST D3DTRIPATCH_INFO* pTriPatchInfo) {
	logToTestFile("Direct3DDevice9Wrapper::DrawTriPatch");
	return Direct3DDevice9->DrawTriPatch(Handle, pNumSegs, pTriPatchInfo);
}

HRESULT Direct3DDevice9Wrapper::DeletePatch(UINT Handle) {
	logToTestFile("Direct3DDevice9Wrapper::DeletePatch");
	return Direct3DDevice9->DeletePatch(Handle);
}

HRESULT Direct3DDevice9Wrapper::CreateQuery(D3DQUERYTYPE Type, IDirect3DQuery9** ppQuery) {
	logToTestFile("Direct3DDevice9Wrapper::CreateQuery");
	return Direct3DDevice9->CreateQuery(Type, ppQuery);
}
