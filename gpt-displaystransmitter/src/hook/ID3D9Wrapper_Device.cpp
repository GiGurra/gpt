#include "ID3D9Wrapper_Device.h"

/******************************************************************************
 *
 *
 *							MODIFIED DX FCNS
 *
 ******************************************************************************/

Direct3DDevice9Wrapper::Direct3DDevice9Wrapper(
	IDirect3DDevice9 *pDirect3DDevice9, 
	IDirect3D9 *pDirect3D9, 
	D3DPRESENT_PARAMETERS *pPresentationParameters) :
		m_device(pDirect3DDevice9), 
		m_d3d(pDirect3D9),
		m_srcVramcSurface(0),
		m_middleBufferSurface(0),
		m_comRefCount(1),
		m_downloadedThisFrame(false) {
}

Direct3DDevice9Wrapper::~Direct3DDevice9Wrapper() {
}

HRESULT Direct3DDevice9Wrapper::QueryInterface(REFIID riid, void** ppvObj) {
	HRESULT hRes = m_device->QueryInterface(riid, ppvObj);
	*ppvObj = (hRes == S_OK) ? this : 0;
	return hRes;
}

ULONG Direct3DDevice9Wrapper::AddRef() {
	m_comRefCount++;
	return m_device->AddRef();
}

ULONG Direct3DDevice9Wrapper::Release() {
	HRESULT out = release(m_device);
	m_comRefCount--;
	if (m_comRefCount == 0) {
		killTexSharer();
		release(pSysRamSurface);
		release(m_middleBufferSurface);
		delete this;
	}
	return out;
}

HRESULT Direct3DDevice9Wrapper::GetDirect3D(IDirect3D9** ppD3D9) {
	*ppD3D9 = m_d3d;
	m_d3d->AddRef();
	return D3D_OK;
}

HRESULT Direct3DDevice9Wrapper::SetRenderTarget(DWORD RenderTargetIndex, IDirect3DSurface9* pRenderTarget) {
	if (m_srcVramcSurface == NULL) {
		D3DSURFACE_DESC srcVramSurfDescr;
		pRenderTarget->GetDesc(&srcVramSurfDescr);
		if ((srcVramSurfDescr.Width == 600 && srcVramSurfDescr.Height == 600)
				|| (srcVramSurfDescr.Width == 1200 && srcVramSurfDescr.Height == 1200)) {
			m_srcVramcSurface = pRenderTarget;
			CreateRenderTarget(srcVramSurfDescr.Width, srcVramSurfDescr.Height, srcVramSurfDescr.Format,
					srcVramSurfDescr.MultiSampleType, srcVramSurfDescr.MultiSampleQuality, false, &m_middleBufferSurface, NULL);
			if (pSysRamSurface == NULL) {
				CreateOffscreenPlainSurface(srcVramSurfDescr.Width, srcVramSurfDescr.Height, srcVramSurfDescr.Format,
						D3DPOOL_SYSTEMMEM, &pSysRamSurface, NULL);
				D3DLOCKED_RECT rect;
				pSysRamSurface->LockRect(&rect, NULL, D3DLOCK_NO_DIRTY_UPDATE | D3DLOCK_READONLY);
				startTexSharer(rect.pBits, srcVramSurfDescr.Width, srcVramSurfDescr.Height, 4);
				pSysRamSurface->UnlockRect();
			}
		}
	} else if (!m_downloadedThisFrame && pRenderTarget == m_srcVramcSurface) {
		m_downloadedThisFrame = true;
		GetRenderTargetData(m_middleBufferSurface, pSysRamSurface);
	}
	return m_device->SetRenderTarget(RenderTargetIndex, pRenderTarget);
}

HRESULT Direct3DDevice9Wrapper::Reset(D3DPRESENT_PARAMETERS* pPresentationParameters) {
	if (m_srcVramcSurface != NULL) {
		release(m_middleBufferSurface);
		m_middleBufferSurface = NULL;
		m_srcVramcSurface = NULL;
	}
	return m_device->Reset(pPresentationParameters);
}

HRESULT Direct3DDevice9Wrapper::Present(CONST RECT* pSourceRect, CONST RECT* pDestRect, HWND hDestWindowOverride,
		CONST RGNDATA* pDirtyRegion) {
	HRESULT out = m_device->Present(pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion);
	if (m_srcVramcSurface != NULL) {
		StretchRect(m_srcVramcSurface, NULL, m_middleBufferSurface, NULL, D3DTEXF_NONE);
		m_downloadedThisFrame = false;
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
	return m_device->BeginScene();
}

HRESULT Direct3DDevice9Wrapper::TestCooperativeLevel() {
	return m_device->TestCooperativeLevel();
}

UINT Direct3DDevice9Wrapper::GetAvailableTextureMem() {
	return m_device->GetAvailableTextureMem();
}

HRESULT Direct3DDevice9Wrapper::EvictManagedResources() {
	return m_device->EvictManagedResources();
}

HRESULT Direct3DDevice9Wrapper::GetDeviceCaps(D3DCAPS9* pCaps) {
	return m_device->GetDeviceCaps(pCaps);
}

HRESULT Direct3DDevice9Wrapper::GetDisplayMode(UINT iSwapChain, D3DDISPLAYMODE* pMode) {
	return m_device->GetDisplayMode(iSwapChain, pMode);
}

HRESULT Direct3DDevice9Wrapper::GetCreationParameters(D3DDEVICE_CREATION_PARAMETERS *pParameters) {
	return m_device->GetCreationParameters(pParameters);
}

HRESULT Direct3DDevice9Wrapper::SetCursorProperties(UINT XHotSpot, UINT YHotSpot, IDirect3DSurface9* pCursorBitmap) {
	return m_device->SetCursorProperties(XHotSpot, YHotSpot, pCursorBitmap);
}

void Direct3DDevice9Wrapper::SetCursorPosition(int X, int Y, DWORD Flags) {
	m_device->SetCursorPosition(X, Y, Flags);
}

BOOL Direct3DDevice9Wrapper::ShowCursor(BOOL bShow) {
	return m_device->ShowCursor(bShow);
}

HRESULT Direct3DDevice9Wrapper::CreateAdditionalSwapChain(D3DPRESENT_PARAMETERS* pPresentationParameters,
		IDirect3DSwapChain9** pSwapChain) {
	return m_device->CreateAdditionalSwapChain(pPresentationParameters, pSwapChain);
}

HRESULT Direct3DDevice9Wrapper::GetSwapChain(UINT iSwapChain, IDirect3DSwapChain9** pSwapChain) {
	return m_device->GetSwapChain(iSwapChain, pSwapChain);
}

UINT Direct3DDevice9Wrapper::GetNumberOfSwapChains() {
	return m_device->GetNumberOfSwapChains();
}

HRESULT Direct3DDevice9Wrapper::GetBackBuffer(UINT iSwapChain, UINT iBackBuffer, D3DBACKBUFFER_TYPE Type,
		IDirect3DSurface9** ppBackBuffer) {
	return m_device->GetBackBuffer(iSwapChain, iBackBuffer, Type, ppBackBuffer);
}

HRESULT Direct3DDevice9Wrapper::GetRasterStatus(UINT iSwapChain, D3DRASTER_STATUS* pRasterStatus) {
	return m_device->GetRasterStatus(iSwapChain, pRasterStatus);
}

HRESULT Direct3DDevice9Wrapper::SetDialogBoxMode(BOOL bEnableDialogs) {
	return m_device->SetDialogBoxMode(bEnableDialogs);
}

void Direct3DDevice9Wrapper::SetGammaRamp(UINT iSwapChain, DWORD Flags, CONST D3DGAMMARAMP* pRamp) {
	m_device->SetGammaRamp(iSwapChain, Flags, pRamp);
}

void Direct3DDevice9Wrapper::GetGammaRamp(UINT iSwapChain, D3DGAMMARAMP* pRamp) {
	m_device->GetGammaRamp(iSwapChain, pRamp);
}

HRESULT Direct3DDevice9Wrapper::CreateTexture(UINT Width, UINT Height, UINT Levels, DWORD Usage, D3DFORMAT Format,
		D3DPOOL Pool, IDirect3DTexture9** ppTexture, HANDLE* pSharedHandle) {
	return m_device->CreateTexture(Width, Height, Levels, Usage, Format, Pool, ppTexture, pSharedHandle);
}

HRESULT Direct3DDevice9Wrapper::CreateVolumeTexture(UINT Width, UINT Height, UINT Depth, UINT Levels, DWORD Usage,
		D3DFORMAT Format, D3DPOOL Pool, IDirect3DVolumeTexture9** ppVolumeTexture, HANDLE* pSharedHandle) {
	return m_device->CreateVolumeTexture(Width, Height, Depth, Levels, Usage, Format, Pool, ppVolumeTexture,
			pSharedHandle);
}

HRESULT Direct3DDevice9Wrapper::CreateCubeTexture(UINT EdgeLength, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool,
		IDirect3DCubeTexture9** ppCubeTexture, HANDLE* pSharedHandle) {
	return m_device->CreateCubeTexture(EdgeLength, Levels, Usage, Format, Pool, ppCubeTexture, pSharedHandle);
}

HRESULT Direct3DDevice9Wrapper::CreateVertexBuffer(UINT Length, DWORD Usage, DWORD FVF, D3DPOOL Pool,
		IDirect3DVertexBuffer9** ppVertexBuffer, HANDLE* pSharedHandle) {
	return m_device->CreateVertexBuffer(Length, Usage, FVF, Pool, ppVertexBuffer, pSharedHandle);
}

HRESULT Direct3DDevice9Wrapper::CreateIndexBuffer(UINT Length, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool,
		IDirect3DIndexBuffer9** ppIndexBuffer, HANDLE* pSharedHandle) {
	return m_device->CreateIndexBuffer(Length, Usage, Format, Pool, ppIndexBuffer, pSharedHandle);
}

HRESULT Direct3DDevice9Wrapper::CreateRenderTarget(UINT Width, UINT Height, D3DFORMAT Format,
		D3DMULTISAMPLE_TYPE MultiSample, DWORD MultisampleQuality, BOOL Lockable, IDirect3DSurface9** ppSurface,
		HANDLE* pSharedHandle) {
	return m_device->CreateRenderTarget(Width, Height, Format, MultiSample, MultisampleQuality, Lockable, ppSurface,
			pSharedHandle);
}

HRESULT Direct3DDevice9Wrapper::CreateDepthStencilSurface(UINT Width, UINT Height, D3DFORMAT Format,
		D3DMULTISAMPLE_TYPE MultiSample, DWORD MultisampleQuality, BOOL Discard, IDirect3DSurface9** ppSurface,
		HANDLE* pSharedHandle) {
	return m_device->CreateDepthStencilSurface(Width, Height, Format, MultiSample, MultisampleQuality, Discard,
			ppSurface, pSharedHandle);
}

HRESULT Direct3DDevice9Wrapper::UpdateSurface(IDirect3DSurface9* pSourceSurface, CONST RECT* pSourceRect,
		IDirect3DSurface9* pDestinationSurface, CONST POINT* pDestPoint) {
	return m_device->UpdateSurface(pSourceSurface, pSourceRect, pDestinationSurface, pDestPoint);
}

HRESULT Direct3DDevice9Wrapper::UpdateTexture(IDirect3DBaseTexture9* pSourceTexture,
		IDirect3DBaseTexture9* pDestinationTexture) {
	return m_device->UpdateTexture(pSourceTexture, pDestinationTexture);
}

HRESULT Direct3DDevice9Wrapper::GetRenderTargetData(IDirect3DSurface9* pRenderTarget, IDirect3DSurface9* pDestSurface) {
	return m_device->GetRenderTargetData(pRenderTarget, pDestSurface);
}

HRESULT Direct3DDevice9Wrapper::GetFrontBufferData(UINT iSwapChain, IDirect3DSurface9* pDestSurface) {
	return m_device->GetFrontBufferData(iSwapChain, pDestSurface);
}

HRESULT Direct3DDevice9Wrapper::StretchRect(IDirect3DSurface9* pSourceSurface, CONST RECT* pSourceRect,
		IDirect3DSurface9* pDestSurface, CONST RECT* pDestRect, D3DTEXTUREFILTERTYPE Filter) {
	return m_device->StretchRect(pSourceSurface, pSourceRect, pDestSurface, pDestRect, Filter);
}

HRESULT Direct3DDevice9Wrapper::ColorFill(IDirect3DSurface9* pSurface, CONST RECT* pRect, D3DCOLOR color) {
	return m_device->ColorFill(pSurface, pRect, color);
}

HRESULT Direct3DDevice9Wrapper::CreateOffscreenPlainSurface(UINT Width, UINT Height, D3DFORMAT Format, D3DPOOL Pool,
		IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle) {
	return m_device->CreateOffscreenPlainSurface(Width, Height, Format, Pool, ppSurface, pSharedHandle);
}

HRESULT Direct3DDevice9Wrapper::GetRenderTarget(DWORD RenderTargetIndex, IDirect3DSurface9** ppRenderTarget) {
	return m_device->GetRenderTarget(RenderTargetIndex, ppRenderTarget);
}

HRESULT Direct3DDevice9Wrapper::SetDepthStencilSurface(IDirect3DSurface9* pNewZStencil) {
	return m_device->SetDepthStencilSurface(pNewZStencil);
}

HRESULT Direct3DDevice9Wrapper::GetDepthStencilSurface(IDirect3DSurface9** ppZStencilSurface) {
	return m_device->GetDepthStencilSurface(ppZStencilSurface);
}

HRESULT Direct3DDevice9Wrapper::EndScene() {
	return m_device->EndScene();
}

HRESULT Direct3DDevice9Wrapper::Clear(DWORD Count, CONST D3DRECT* pRects, DWORD Flags, D3DCOLOR Color, float Z,
		DWORD Stencil) {
	return m_device->Clear(Count, pRects, Flags, Color, Z, Stencil);
}

HRESULT Direct3DDevice9Wrapper::SetTransform(D3DTRANSFORMSTATETYPE State, CONST D3DMATRIX* pMatrix) {
	return m_device->SetTransform(State, pMatrix);
}

HRESULT Direct3DDevice9Wrapper::GetTransform(D3DTRANSFORMSTATETYPE State, D3DMATRIX* pMatrix) {
	return m_device->GetTransform(State, pMatrix);
}

HRESULT Direct3DDevice9Wrapper::MultiplyTransform(D3DTRANSFORMSTATETYPE param0, CONST D3DMATRIX * param1) {
	return m_device->MultiplyTransform(param0, param1);
}

HRESULT Direct3DDevice9Wrapper::SetViewport(CONST D3DVIEWPORT9* pViewport) {
	return m_device->SetViewport(pViewport);
}

HRESULT Direct3DDevice9Wrapper::GetViewport(D3DVIEWPORT9* pViewport) {
	return m_device->GetViewport(pViewport);
}

HRESULT Direct3DDevice9Wrapper::SetMaterial(CONST D3DMATERIAL9* pMaterial) {
	return m_device->SetMaterial(pMaterial);
}

HRESULT Direct3DDevice9Wrapper::GetMaterial(D3DMATERIAL9* pMaterial) {
	return m_device->GetMaterial(pMaterial);
}

HRESULT Direct3DDevice9Wrapper::SetLight(DWORD Index, CONST D3DLIGHT9* param1) {
	return m_device->SetLight(Index, param1);
}

HRESULT Direct3DDevice9Wrapper::GetLight(DWORD Index, D3DLIGHT9* param1) {
	return m_device->GetLight(Index, param1);
}

HRESULT Direct3DDevice9Wrapper::LightEnable(DWORD Index, BOOL Enable) {
	return m_device->LightEnable(Index, Enable);
}

HRESULT Direct3DDevice9Wrapper::GetLightEnable(DWORD Index, BOOL* pEnable) {
	return m_device->GetLightEnable(Index, pEnable);
}

HRESULT Direct3DDevice9Wrapper::SetClipPlane(DWORD Index, CONST float* pPlane) {
	return m_device->SetClipPlane(Index, pPlane);
}

HRESULT Direct3DDevice9Wrapper::GetClipPlane(DWORD Index, float* pPlane) {
	return m_device->GetClipPlane(Index, pPlane);
}

HRESULT Direct3DDevice9Wrapper::SetRenderState(D3DRENDERSTATETYPE State, DWORD Value) {
	return m_device->SetRenderState(State, Value);
}

HRESULT Direct3DDevice9Wrapper::GetRenderState(D3DRENDERSTATETYPE State, DWORD* pValue) {
	return m_device->GetRenderState(State, pValue);
}

HRESULT Direct3DDevice9Wrapper::CreateStateBlock(D3DSTATEBLOCKTYPE Type, IDirect3DStateBlock9** ppSB) {
	return m_device->CreateStateBlock(Type, ppSB);
}

HRESULT Direct3DDevice9Wrapper::BeginStateBlock() {
	return m_device->BeginStateBlock();
}

HRESULT Direct3DDevice9Wrapper::EndStateBlock(IDirect3DStateBlock9** ppSB) {
	return m_device->EndStateBlock(ppSB);
}

HRESULT Direct3DDevice9Wrapper::SetClipStatus(CONST D3DCLIPSTATUS9* pClipStatus) {
	return m_device->SetClipStatus(pClipStatus);
}

HRESULT Direct3DDevice9Wrapper::GetClipStatus(D3DCLIPSTATUS9* pClipStatus) {
	return m_device->GetClipStatus(pClipStatus);
}

HRESULT Direct3DDevice9Wrapper::GetTexture(DWORD Stage, IDirect3DBaseTexture9** ppTexture) {
	return m_device->GetTexture(Stage, ppTexture);
}

HRESULT Direct3DDevice9Wrapper::SetTexture(DWORD Stage, IDirect3DBaseTexture9* pTexture) {
	return m_device->SetTexture(Stage, pTexture);
}

HRESULT Direct3DDevice9Wrapper::GetTextureStageState(DWORD Stage, D3DTEXTURESTAGESTATETYPE Type, DWORD* pValue) {
	return m_device->GetTextureStageState(Stage, Type, pValue);
}

HRESULT Direct3DDevice9Wrapper::SetTextureStageState(DWORD Stage, D3DTEXTURESTAGESTATETYPE Type, DWORD Value) {
	return m_device->SetTextureStageState(Stage, Type, Value);
}

HRESULT Direct3DDevice9Wrapper::GetSamplerState(DWORD Sampler, D3DSAMPLERSTATETYPE Type, DWORD* pValue) {
	return m_device->GetSamplerState(Sampler, Type, pValue);
}

HRESULT Direct3DDevice9Wrapper::SetSamplerState(DWORD Sampler, D3DSAMPLERSTATETYPE Type, DWORD Value) {
	return m_device->SetSamplerState(Sampler, Type, Value);
}

HRESULT Direct3DDevice9Wrapper::ValidateDevice(DWORD* pNumPasses) {
	return m_device->ValidateDevice(pNumPasses);
}

HRESULT Direct3DDevice9Wrapper::SetPaletteEntries(UINT PaletteNumber, CONST PALETTEENTRY* pEntries) {
	return m_device->SetPaletteEntries(PaletteNumber, pEntries);
}

HRESULT Direct3DDevice9Wrapper::GetPaletteEntries(UINT PaletteNumber, PALETTEENTRY* pEntries) {
	return m_device->GetPaletteEntries(PaletteNumber, pEntries);
}

HRESULT Direct3DDevice9Wrapper::SetCurrentTexturePalette(UINT PaletteNumber) {
	return m_device->SetCurrentTexturePalette(PaletteNumber);
}

HRESULT Direct3DDevice9Wrapper::GetCurrentTexturePalette(UINT *PaletteNumber) {
	return m_device->GetCurrentTexturePalette(PaletteNumber);
}

HRESULT Direct3DDevice9Wrapper::SetScissorRect(CONST RECT* pRect) {
	return m_device->SetScissorRect(pRect);
}

HRESULT Direct3DDevice9Wrapper::GetScissorRect(RECT* pRect) {
	return m_device->GetScissorRect(pRect);
}

HRESULT Direct3DDevice9Wrapper::SetSoftwareVertexProcessing(BOOL bSoftware) {
	return m_device->SetSoftwareVertexProcessing(bSoftware);
}

BOOL Direct3DDevice9Wrapper::GetSoftwareVertexProcessing() {
	return m_device->GetSoftwareVertexProcessing();
}

HRESULT Direct3DDevice9Wrapper::SetNPatchMode(float nSegments) {
	return m_device->SetNPatchMode(nSegments);
}

float Direct3DDevice9Wrapper::GetNPatchMode() {
	return m_device->GetNPatchMode();
}

HRESULT Direct3DDevice9Wrapper::DrawPrimitive(D3DPRIMITIVETYPE PrimitiveType, UINT StartVertex, UINT PrimitiveCount) {
	return m_device->DrawPrimitive(PrimitiveType, StartVertex, PrimitiveCount);
}

HRESULT Direct3DDevice9Wrapper::DrawIndexedPrimitive(D3DPRIMITIVETYPE param0, INT BaseVertexIndex, UINT MinVertexIndex,
		UINT NumVertices, UINT startIndex, UINT primCount) {
	return m_device->DrawIndexedPrimitive(param0, BaseVertexIndex, MinVertexIndex, NumVertices, startIndex, primCount);
}

HRESULT Direct3DDevice9Wrapper::DrawPrimitiveUP(D3DPRIMITIVETYPE PrimitiveType, UINT PrimitiveCount,
		CONST void* pVertexStreamZeroData, UINT VertexStreamZeroStride) {
	return m_device->DrawPrimitiveUP(PrimitiveType, PrimitiveCount, pVertexStreamZeroData, VertexStreamZeroStride);
}

HRESULT Direct3DDevice9Wrapper::DrawIndexedPrimitiveUP(D3DPRIMITIVETYPE PrimitiveType, UINT MinVertexIndex, UINT NumVertices,
		UINT PrimitiveCount, CONST void* pIndexData, D3DFORMAT IndexDataFormat, CONST void* pVertexStreamZeroData,
		UINT VertexStreamZeroStride) {
	return m_device->DrawIndexedPrimitiveUP(PrimitiveType, MinVertexIndex, NumVertices, PrimitiveCount, pIndexData,
			IndexDataFormat, pVertexStreamZeroData, VertexStreamZeroStride);
}

HRESULT Direct3DDevice9Wrapper::ProcessVertices(UINT SrcStartIndex, UINT DestIndex, UINT VertexCount,
		IDirect3DVertexBuffer9* pDestBuffer, IDirect3DVertexDeclaration9* pVertexDecl, DWORD Flags) {
	return m_device->ProcessVertices(SrcStartIndex, DestIndex, VertexCount, pDestBuffer, pVertexDecl, Flags);
}

HRESULT Direct3DDevice9Wrapper::CreateVertexDeclaration(CONST D3DVERTEXELEMENT9* pVertexElements,
		IDirect3DVertexDeclaration9** ppDecl) {
	return m_device->CreateVertexDeclaration(pVertexElements, ppDecl);
}

HRESULT Direct3DDevice9Wrapper::SetVertexDeclaration(IDirect3DVertexDeclaration9* pDecl) {
	return m_device->SetVertexDeclaration(pDecl);
}

HRESULT Direct3DDevice9Wrapper::GetVertexDeclaration(IDirect3DVertexDeclaration9** ppDecl) {
	return m_device->GetVertexDeclaration(ppDecl);
}

HRESULT Direct3DDevice9Wrapper::SetFVF(DWORD FVF) {
	return m_device->SetFVF(FVF);
}

HRESULT Direct3DDevice9Wrapper::GetFVF(DWORD* pFVF) {
	return m_device->GetFVF(pFVF);
}

HRESULT Direct3DDevice9Wrapper::CreateVertexShader(CONST DWORD* pFunction, IDirect3DVertexShader9** ppShader) {
	return m_device->CreateVertexShader(pFunction, ppShader);
}

HRESULT Direct3DDevice9Wrapper::SetVertexShader(IDirect3DVertexShader9* pShader) {
	return m_device->SetVertexShader(pShader);
}

HRESULT Direct3DDevice9Wrapper::GetVertexShader(IDirect3DVertexShader9** ppShader) {
	return m_device->GetVertexShader(ppShader);
}

HRESULT Direct3DDevice9Wrapper::SetVertexShaderConstantF(UINT StartRegister, CONST float* pConstantData,
		UINT Vector4fCount) {
	return m_device->SetVertexShaderConstantF(StartRegister, pConstantData, Vector4fCount);
}

HRESULT Direct3DDevice9Wrapper::GetVertexShaderConstantF(UINT StartRegister, float* pConstantData, UINT Vector4fCount) {
	return m_device->GetVertexShaderConstantF(StartRegister, pConstantData, Vector4fCount);
}

HRESULT Direct3DDevice9Wrapper::SetVertexShaderConstantI(UINT StartRegister, CONST int* pConstantData, UINT Vector4iCount) {
	return m_device->SetVertexShaderConstantI(StartRegister, pConstantData, Vector4iCount);
}

HRESULT Direct3DDevice9Wrapper::GetVertexShaderConstantI(UINT StartRegister, int* pConstantData, UINT Vector4iCount) {
	return m_device->GetVertexShaderConstantI(StartRegister, pConstantData, Vector4iCount);
}

HRESULT Direct3DDevice9Wrapper::SetVertexShaderConstantB(UINT StartRegister, CONST BOOL* pConstantData, UINT BoolCount) {
	return m_device->SetVertexShaderConstantB(StartRegister, pConstantData, BoolCount);
}

HRESULT Direct3DDevice9Wrapper::GetVertexShaderConstantB(UINT StartRegister, BOOL* pConstantData, UINT BoolCount) {
	return m_device->GetVertexShaderConstantB(StartRegister, pConstantData, BoolCount);
}

HRESULT Direct3DDevice9Wrapper::SetStreamSource(UINT StreamNumber, IDirect3DVertexBuffer9* pStreamData, UINT OffsetInBytes,
		UINT Stride) {
	if (StreamNumber == 0)
		m_Stride = Stride;
	return m_device->SetStreamSource(StreamNumber, pStreamData, OffsetInBytes, Stride);
}

HRESULT Direct3DDevice9Wrapper::GetStreamSource(UINT StreamNumber, IDirect3DVertexBuffer9** ppStreamData,
		UINT* pOffsetInBytes, UINT* pStride) {
	return m_device->GetStreamSource(StreamNumber, ppStreamData, pOffsetInBytes, pStride);
}

HRESULT Direct3DDevice9Wrapper::SetStreamSourceFreq(UINT StreamNumber, UINT Setting) {
	return m_device->SetStreamSourceFreq(StreamNumber, Setting);
}

HRESULT Direct3DDevice9Wrapper::GetStreamSourceFreq(UINT StreamNumber, UINT* pSetting) {
	return m_device->GetStreamSourceFreq(StreamNumber, pSetting);
}

HRESULT Direct3DDevice9Wrapper::SetIndices(IDirect3DIndexBuffer9* pIndexData) {
	return m_device->SetIndices(pIndexData);
}

HRESULT Direct3DDevice9Wrapper::GetIndices(IDirect3DIndexBuffer9** ppIndexData) {
	return m_device->GetIndices(ppIndexData);
}

HRESULT Direct3DDevice9Wrapper::CreatePixelShader(CONST DWORD* pFunction, IDirect3DPixelShader9** ppShader) {
	return m_device->CreatePixelShader(pFunction, ppShader);
}

HRESULT Direct3DDevice9Wrapper::SetPixelShader(IDirect3DPixelShader9* pShader) {
	return m_device->SetPixelShader(pShader);
}

HRESULT Direct3DDevice9Wrapper::GetPixelShader(IDirect3DPixelShader9** ppShader) {
	return m_device->GetPixelShader(ppShader);
}

HRESULT Direct3DDevice9Wrapper::SetPixelShaderConstantF(UINT StartRegister, CONST float* pConstantData, UINT Vector4fCount) {
	return m_device->SetPixelShaderConstantF(StartRegister, pConstantData, Vector4fCount);
}

HRESULT Direct3DDevice9Wrapper::GetPixelShaderConstantF(UINT StartRegister, float* pConstantData, UINT Vector4fCount) {
	return m_device->GetPixelShaderConstantF(StartRegister, pConstantData, Vector4fCount);
}

HRESULT Direct3DDevice9Wrapper::SetPixelShaderConstantI(UINT StartRegister, CONST int* pConstantData, UINT Vector4iCount) {
	return m_device->SetPixelShaderConstantI(StartRegister, pConstantData, Vector4iCount);
}

HRESULT Direct3DDevice9Wrapper::GetPixelShaderConstantI(UINT StartRegister, int* pConstantData, UINT Vector4iCount) {
	return m_device->GetPixelShaderConstantI(StartRegister, pConstantData, Vector4iCount);
}

HRESULT Direct3DDevice9Wrapper::SetPixelShaderConstantB(UINT StartRegister, CONST BOOL* pConstantData, UINT BoolCount) {
	return m_device->SetPixelShaderConstantB(StartRegister, pConstantData, BoolCount);
}

HRESULT Direct3DDevice9Wrapper::GetPixelShaderConstantB(UINT StartRegister, BOOL* pConstantData, UINT BoolCount) {
	return m_device->GetPixelShaderConstantB(StartRegister, pConstantData, BoolCount);
}

HRESULT Direct3DDevice9Wrapper::DrawRectPatch(UINT Handle, CONST float* pNumSegs, CONST D3DRECTPATCH_INFO* pRectPatchInfo) {
	return m_device->DrawRectPatch(Handle, pNumSegs, pRectPatchInfo);
}

HRESULT Direct3DDevice9Wrapper::DrawTriPatch(UINT Handle, CONST float* pNumSegs, CONST D3DTRIPATCH_INFO* pTriPatchInfo) {
	return m_device->DrawTriPatch(Handle, pNumSegs, pTriPatchInfo);
}

HRESULT Direct3DDevice9Wrapper::DeletePatch(UINT Handle) {
	return m_device->DeletePatch(Handle);
}

HRESULT Direct3DDevice9Wrapper::CreateQuery(D3DQUERYTYPE Type, IDirect3DQuery9** ppQuery) {
	return m_device->CreateQuery(Type, ppQuery);
}
