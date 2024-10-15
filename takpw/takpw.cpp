#include "takpw.h"

#include <stdio.h>
#include <string.h>

HMODULE g_d3d8 = NULL;

#if 1
#define log(fmt, ...) printf("KAIKAIKAI " fmt, __VA_ARGS__)
#else
#define log(fmt, ...)
#endif

BOOL WINAPI DllMain(
    HINSTANCE hinstDLL,  // handle to DLL module
    DWORD fdwReason,     // reason for calling function
    LPVOID lpvReserved )  // reserved
{
    // Perform actions based on the reason for calling.
    const char* libname;
    switch( fdwReason )
    {
        case DLL_PROCESS_ATTACH:
            // Initialize once for each new process.
            // Return FALSE to fail DLL load.
            libname = "d3d8-dxvk.dll";
            g_d3d8 = LoadLibrary(libname);
            if (!g_d3d8)
            {
                libname = "C:\\Windows\\System32\\d3d8.dll";
                g_d3d8 = LoadLibrary(libname);
            }
            log("Loaded %s\n", libname);

            break;

        case DLL_THREAD_ATTACH:
            // Do thread-specific initialization.
            break;

        case DLL_THREAD_DETACH:
            // Do thread-specific cleanup.
            break;

        case DLL_PROCESS_DETACH:

            if (lpvReserved != NULL)
            {
                break; // do not do cleanup if process termination scenario
            }

            FreeLibrary(g_d3d8);

            // Perform any necessary cleanup.
            break;
    }

    return TRUE;  // Successful DLL_PROCESS_ATTACH.
}

class TAKPIDirect3D8;

class TAKPIDirect3DDevice8 : public IDirect3DDevice8
{
public:
    TAKPIDirect3DDevice8(IDirect3D8* d3d8, IDirect3DDevice8* d3d8dev)
        : m_d3d8(d3d8)
        , m_d3d8dev(d3d8dev)
    {
        log("%p, %p, %s\n", this, m_d3d8dev, __FUNCTION__);
    }

    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObj) override
    {
        log("%p, %p, %s\n", this, m_d3d8dev, __FUNCTION__);
        return m_d3d8dev->QueryInterface(riid, ppvObj);
    }

    ULONG STDMETHODCALLTYPE AddRef() override
    {
        log("%p, %p, %s\n", this, m_d3d8dev, __FUNCTION__);
        return m_d3d8dev->AddRef();
    }

    ULONG STDMETHODCALLTYPE Release() override
    {
        log("%p, %p, %s\n", this, m_d3d8dev, __FUNCTION__);
        auto result = m_d3d8dev->Release();
        if (result == 0)
        {
            log("We're done with this! %p %d\n", this, result);
            delete this;
        }
        return result;
    }

    HRESULT STDMETHODCALLTYPE TestCooperativeLevel() override
    {
        log("%p, %p, %s\n", this, m_d3d8dev, __FUNCTION__);
        return m_d3d8dev->TestCooperativeLevel();
    }

    UINT STDMETHODCALLTYPE GetAvailableTextureMem() override
    {
        log("%p, %p, %s ()\n", this, m_d3d8dev, __FUNCTION__);
        return m_d3d8dev->GetAvailableTextureMem();
    }

    HRESULT STDMETHODCALLTYPE ResourceManagerDiscardBytes(DWORD Bytes) override
    {
        log("%p, %p, %s\n", this, m_d3d8dev, __FUNCTION__);
        return m_d3d8dev->ResourceManagerDiscardBytes(Bytes);
    }

    HRESULT STDMETHODCALLTYPE GetDirect3D(IDirect3D8** ppD3D8) override
    {
        log("%p, %p, %s (%p)\n", this, m_d3d8dev, __FUNCTION__);
        HRESULT result = D3D_OK;
        result = m_d3d8dev->GetDirect3D(ppD3D8);
        *ppD3D8 = m_d3d8;
        log("    %p, %p, %s () = %p\n", this, m_d3d8dev, __FUNCTION__, *ppD3D8);
        return result;
    }

    HRESULT STDMETHODCALLTYPE GetDeviceCaps(D3DCAPS8* pCaps) override
    {
        log("%p, %p, %s (%p)\n", this, m_d3d8dev, __FUNCTION__, pCaps);
        return m_d3d8dev->GetDeviceCaps(pCaps);
    }

    HRESULT STDMETHODCALLTYPE GetDisplayMode(D3DDISPLAYMODE* pMode) override
    {
        log("%p, %p, %s (%p)\n", this, m_d3d8dev, __FUNCTION__, pMode);
        return m_d3d8dev->GetDisplayMode(pMode);
    }

    HRESULT STDMETHODCALLTYPE GetCreationParameters(D3DDEVICE_CREATION_PARAMETERS* pParameters) override
    {
        log("%p, %p, %s\n", this, m_d3d8dev, __FUNCTION__);
        return m_d3d8dev->GetCreationParameters(pParameters);
    }

    HRESULT STDMETHODCALLTYPE SetCursorProperties(UINT XHotSpot, UINT YHotSpot, IDirect3DSurface8* pCursorBitmap) override
    {
        log("%p, %p, %s\n", this, m_d3d8dev, __FUNCTION__);
        return m_d3d8dev->SetCursorProperties(XHotSpot, YHotSpot, pCursorBitmap);
    }

    void STDMETHODCALLTYPE SetCursorPosition(UINT XScreenSpace, UINT YScreenSpace, DWORD Flags) override
    {
        log("%p, %p, %s\n", this, m_d3d8dev, __FUNCTION__);
        return m_d3d8dev->SetCursorPosition(XScreenSpace, YScreenSpace, Flags);
    }

    BOOL STDMETHODCALLTYPE ShowCursor(BOOL bShow) override
    {
        log("%p, %p, %s\n", this, m_d3d8dev, __FUNCTION__);
        return m_d3d8dev->ShowCursor(bShow);
    }

    HRESULT STDMETHODCALLTYPE CreateAdditionalSwapChain(D3DPRESENT_PARAMETERS* pPresentationParameters, IDirect3DSwapChain8** pSwapChain) override
    {
        log("%p, %p, %s\n", this, m_d3d8dev, __FUNCTION__);
        return m_d3d8dev->CreateAdditionalSwapChain(pPresentationParameters, pSwapChain);
    }

    HRESULT STDMETHODCALLTYPE Reset(D3DPRESENT_PARAMETERS* pPresentationParameters) override
    {
        log("%p, %p, %s (%p)\n", this, m_d3d8dev, __FUNCTION__, pPresentationParameters);
        return m_d3d8dev->Reset(pPresentationParameters);
    }

    HRESULT STDMETHODCALLTYPE Present(CONST RECT* pSourceRect, CONST RECT* pDestRect, HWND hDestWindowOverride, CONST RGNDATA* pDirtyRegion) override
    {
        log("%p, %p, %s (%p, %p, %p, %p)\n", this, m_d3d8dev, __FUNCTION__, pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion);
        return m_d3d8dev->Present(pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion);
    }

    HRESULT STDMETHODCALLTYPE GetBackBuffer(UINT BackBuffer, D3DBACKBUFFER_TYPE Type, IDirect3DSurface8** ppBackBuffer) override
    {
        log("%p, %p, %s (%d, %d, %p)\n", this, m_d3d8dev, __FUNCTION__, BackBuffer, Type, ppBackBuffer);
        return m_d3d8dev->GetBackBuffer(BackBuffer, Type, ppBackBuffer);
    }

    HRESULT STDMETHODCALLTYPE GetRasterStatus(D3DRASTER_STATUS* pRasterStatus) override
    {
        log("%p, %p, %s\n", this, m_d3d8dev, __FUNCTION__);
        return m_d3d8dev->GetRasterStatus(pRasterStatus);
    }

    void STDMETHODCALLTYPE SetGammaRamp(DWORD Flags, CONST D3DGAMMARAMP* pRamp) override
    {
        log("%p, %p, %s\n", this, m_d3d8dev, __FUNCTION__);
        return m_d3d8dev->SetGammaRamp(Flags, pRamp);
    }

    void STDMETHODCALLTYPE GetGammaRamp(D3DGAMMARAMP* pRamp) override
    {
        log("%p, %p, %s\n", this, m_d3d8dev, __FUNCTION__);
        return m_d3d8dev->GetGammaRamp(pRamp);
    }

    HRESULT STDMETHODCALLTYPE CreateTexture(UINT Width, UINT Height, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DTexture8** ppTexture) override
    {
        log("%p, %p, %s (%d, %d, %d, %d, %d, %d, %p)\n", this, m_d3d8dev, __FUNCTION__, Width, Height, Levels, Usage, Format, Pool, ppTexture);
        return m_d3d8dev->CreateTexture(Width, Height, Levels, Usage, Format, Pool, ppTexture);
    }

    HRESULT STDMETHODCALLTYPE CreateVolumeTexture(UINT Width, UINT Height, UINT Depth, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DVolumeTexture8** ppVolumeTexture) override
    {
        log("%p, %p, %s\n", this, m_d3d8dev, __FUNCTION__);
        return m_d3d8dev->CreateVolumeTexture(Width, Height, Depth, Levels, Usage, Format, Pool, ppVolumeTexture);
    }

    HRESULT STDMETHODCALLTYPE CreateCubeTexture(UINT EdgeLength, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DCubeTexture8** ppCubeTexture) override
    {
        log("%p, %p, %s\n", this, m_d3d8dev, __FUNCTION__);
        return m_d3d8dev->CreateCubeTexture(EdgeLength, Levels, Usage, Format, Pool, ppCubeTexture);
    }

    HRESULT STDMETHODCALLTYPE CreateVertexBuffer(UINT Length, DWORD Usage, DWORD FVF, D3DPOOL Pool, IDirect3DVertexBuffer8** ppVertexBuffer) override
    {
        log("%p, %p, %s (%d, %d, %d, %d, %p)\n", this, m_d3d8dev, __FUNCTION__, Length, Usage, FVF, Pool, ppVertexBuffer);
        return m_d3d8dev->CreateVertexBuffer(Length, Usage, FVF, Pool, ppVertexBuffer);
    }

    HRESULT STDMETHODCALLTYPE CreateIndexBuffer(UINT Length, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DIndexBuffer8** ppIndexBuffer) override
    {
        log("%p, %p, %s (%d, %d, %d, %d, %p)\n", this, m_d3d8dev, __FUNCTION__, Length, Usage, Format, Pool, ppIndexBuffer);
        return m_d3d8dev->CreateIndexBuffer(Length, Usage, Format, Pool, ppIndexBuffer);
    }

    HRESULT STDMETHODCALLTYPE CreateRenderTarget(UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE MultiSample, BOOL Lockable, IDirect3DSurface8** ppSurface) override
    {
        log("%p, %p, %s\n", this, m_d3d8dev, __FUNCTION__);
        return m_d3d8dev->CreateRenderTarget(Width, Height, Format, MultiSample, Lockable, ppSurface);
    }

    HRESULT STDMETHODCALLTYPE CreateDepthStencilSurface(UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE MultiSample, IDirect3DSurface8** ppSurface) override
    {
        log("%p, %p, %s\n", this, m_d3d8dev, __FUNCTION__);
        return m_d3d8dev->CreateDepthStencilSurface(Width, Height, Format, MultiSample, ppSurface);
    }

    HRESULT STDMETHODCALLTYPE CreateImageSurface(UINT Width, UINT Height, D3DFORMAT Format, IDirect3DSurface8** ppSurface) override
    {
        log("%p, %p, %s\n", this, m_d3d8dev, __FUNCTION__);
        return m_d3d8dev->CreateImageSurface(Width, Height, Format, ppSurface);
    }

    HRESULT STDMETHODCALLTYPE CopyRects(IDirect3DSurface8* pSourceSurface, CONST RECT* pSourceRectsArray, UINT cRects, IDirect3DSurface8* pDestinationSurface, CONST POINT* pDestPointsArray) override
    {
        log("%p, %p, %s\n", this, m_d3d8dev, __FUNCTION__);
        return m_d3d8dev->CopyRects(pSourceSurface, pSourceRectsArray, cRects, pDestinationSurface, pDestPointsArray);
    }

    HRESULT STDMETHODCALLTYPE UpdateTexture(IDirect3DBaseTexture8* pSourceTexture, IDirect3DBaseTexture8* pDestinationTexture) override
    {
        log("%p, %p, %s\n", this, m_d3d8dev, __FUNCTION__);
        return m_d3d8dev->UpdateTexture(pSourceTexture, pDestinationTexture);
    }

    HRESULT STDMETHODCALLTYPE GetFrontBuffer(IDirect3DSurface8* pDestSurface) override
    {
        log("%p, %p, %s\n", this, m_d3d8dev, __FUNCTION__);
        return m_d3d8dev->GetFrontBuffer(pDestSurface);
    }

    HRESULT STDMETHODCALLTYPE SetRenderTarget(IDirect3DSurface8* pRenderTarget, IDirect3DSurface8* pNewZStencil) override
    {
        log("%p, %p, %s\n", this, m_d3d8dev, __FUNCTION__);
        return m_d3d8dev->SetRenderTarget(pRenderTarget, pNewZStencil);
    }

    HRESULT STDMETHODCALLTYPE GetRenderTarget(IDirect3DSurface8** ppRenderTarget) override
    {
        log("%p, %p, %s\n", this, m_d3d8dev, __FUNCTION__);
        return m_d3d8dev->GetRenderTarget(ppRenderTarget);
    }

    HRESULT STDMETHODCALLTYPE GetDepthStencilSurface(IDirect3DSurface8** ppZStencilSurface) override
    {
        log("%p, %p, %s\n", this, m_d3d8dev, __FUNCTION__);
        return m_d3d8dev->GetDepthStencilSurface(ppZStencilSurface);
    }

    HRESULT STDMETHODCALLTYPE BeginScene() override
    {
        log("%p, %p, %s ()\n", this, m_d3d8dev, __FUNCTION__);
        return m_d3d8dev->BeginScene();
    }

    HRESULT STDMETHODCALLTYPE EndScene() override
    {
        log("%p, %p, %s ()\n", this, m_d3d8dev, __FUNCTION__);
        return m_d3d8dev->EndScene();
    }

    HRESULT STDMETHODCALLTYPE Clear(DWORD Count, CONST D3DRECT* pRects, DWORD Flags, D3DCOLOR Color, float Z, DWORD Stencil) override
    {
        log("%p, %p, %s (%d, %p, %d, %d, %5.2f, %d)\n", this, m_d3d8dev, __FUNCTION__, Count, pRects, Flags, Color, Z, Stencil);
        return m_d3d8dev->Clear(Count, pRects, Flags, Color, Z, Stencil);
    }

    HRESULT STDMETHODCALLTYPE SetTransform(D3DTRANSFORMSTATETYPE State, CONST D3DMATRIX* pMatrix) override
    {
        log("%p, %p, %s (%d, %p)\n", this, m_d3d8dev, __FUNCTION__, State, pMatrix);
        return m_d3d8dev->SetTransform(State, pMatrix);
    }

    HRESULT STDMETHODCALLTYPE GetTransform(D3DTRANSFORMSTATETYPE State, D3DMATRIX* pMatrix) override
    {
        log("%p, %p, %s\n", this, m_d3d8dev, __FUNCTION__);
        return m_d3d8dev->GetTransform(State, pMatrix);
    }

    HRESULT STDMETHODCALLTYPE MultiplyTransform(D3DTRANSFORMSTATETYPE State, CONST D3DMATRIX* pMatrix) override
    {
        log("%p, %p, %s\n", this, m_d3d8dev, __FUNCTION__);
        return m_d3d8dev->MultiplyTransform(State, pMatrix);
    }

    HRESULT STDMETHODCALLTYPE SetViewport(CONST D3DVIEWPORT8* pViewport) override
    {
        log("%p, %p, %s (%p)\n", this, m_d3d8dev, __FUNCTION__, pViewport);
        return m_d3d8dev->SetViewport(pViewport);
    }

    HRESULT STDMETHODCALLTYPE GetViewport(D3DVIEWPORT8* pViewport) override
    {
        log("%p, %p, %s (%p)\n", this, m_d3d8dev, __FUNCTION__, pViewport);
        return m_d3d8dev->GetViewport(pViewport);
    }

    HRESULT STDMETHODCALLTYPE SetMaterial(CONST D3DMATERIAL8* pMaterial) override
    {
        log("%p, %p, %s\n", this, m_d3d8dev, __FUNCTION__);
        return m_d3d8dev->SetMaterial(pMaterial);
    }

    HRESULT STDMETHODCALLTYPE GetMaterial(D3DMATERIAL8* pMaterial) override
    {
        log("%p, %p, %s\n", this, m_d3d8dev, __FUNCTION__);
        return m_d3d8dev->GetMaterial(pMaterial);
    }

    HRESULT STDMETHODCALLTYPE SetLight(DWORD Index, CONST D3DLIGHT8* pLight) override
    {
        log("%p, %p, %s\n", this, m_d3d8dev, __FUNCTION__);
        return m_d3d8dev->SetLight(Index, pLight);
    }

    HRESULT STDMETHODCALLTYPE GetLight(DWORD Index, D3DLIGHT8* pLight) override
    {
        log("%p, %p, %s\n", this, m_d3d8dev, __FUNCTION__);
        return m_d3d8dev->GetLight(Index, pLight);
    }

    HRESULT STDMETHODCALLTYPE LightEnable(DWORD Index, BOOL Enable) override
    {
        log("%p, %p, %s\n", this, m_d3d8dev, __FUNCTION__);
        return m_d3d8dev->LightEnable(Index, Enable);
    }

    HRESULT STDMETHODCALLTYPE GetLightEnable(DWORD Index, BOOL* pEnable) override
    {
        log("%p, %p, %s\n", this, m_d3d8dev, __FUNCTION__);
        return m_d3d8dev->GetLightEnable(Index, pEnable);
    }

    HRESULT STDMETHODCALLTYPE SetClipPlane(DWORD Index, CONST float* pPlane) override
    {
        log("%p, %p, %s\n", this, m_d3d8dev, __FUNCTION__);
        return m_d3d8dev->SetClipPlane(Index, pPlane);
    }

    HRESULT STDMETHODCALLTYPE GetClipPlane(DWORD Index, float* pPlane) override
    {
        log("%p, %p, %s\n", this, m_d3d8dev, __FUNCTION__);
        return m_d3d8dev->GetClipPlane(Index, pPlane);
    }

    HRESULT STDMETHODCALLTYPE SetRenderState(D3DRENDERSTATETYPE State, DWORD Value) override
    {
        log("%p, %p, %s (%d, %d)\n", this, m_d3d8dev, __FUNCTION__, State, Value);
        return m_d3d8dev->SetRenderState(State, Value);
    }

    HRESULT STDMETHODCALLTYPE GetRenderState(D3DRENDERSTATETYPE State, DWORD* pValue) override
    {
        log("%p, %p, %s (%d, %p)\n", this, m_d3d8dev, __FUNCTION__, State, pValue);
        return m_d3d8dev->GetRenderState(State, pValue);
    }

    HRESULT STDMETHODCALLTYPE BeginStateBlock() override
    {
        log("%p, %p, %s\n", this, m_d3d8dev, __FUNCTION__);
        return m_d3d8dev->BeginStateBlock();
    }

    HRESULT STDMETHODCALLTYPE EndStateBlock(DWORD* pToken) override
    {
        log("%p, %p, %s\n", this, m_d3d8dev, __FUNCTION__);
        return m_d3d8dev->EndStateBlock(pToken);
    }

    HRESULT STDMETHODCALLTYPE ApplyStateBlock(DWORD Token) override
    {
        log("%p, %p, %s\n", this, m_d3d8dev, __FUNCTION__);
        return m_d3d8dev->ApplyStateBlock(Token);
    }

    HRESULT STDMETHODCALLTYPE CaptureStateBlock(DWORD Token) override
    {
        log("%p, %p, %s\n", this, m_d3d8dev, __FUNCTION__);
        return m_d3d8dev->CaptureStateBlock(Token);
    }

    HRESULT STDMETHODCALLTYPE DeleteStateBlock(DWORD Token) override
    {
        log("%p, %p, %s\n", this, m_d3d8dev, __FUNCTION__);
        return m_d3d8dev->DeleteStateBlock(Token);
    }

    HRESULT STDMETHODCALLTYPE CreateStateBlock(D3DSTATEBLOCKTYPE Type, DWORD* pToken) override
    {
        log("%p, %p, %s\n", this, m_d3d8dev, __FUNCTION__);
        return m_d3d8dev->CreateStateBlock(Type, pToken);
    }

    HRESULT STDMETHODCALLTYPE SetClipStatus(CONST D3DCLIPSTATUS8* pClipStatus) override
    {
        log("%p, %p, %s\n", this, m_d3d8dev, __FUNCTION__);
        return m_d3d8dev->SetClipStatus(pClipStatus);
    }

    HRESULT STDMETHODCALLTYPE GetClipStatus(D3DCLIPSTATUS8* pClipStatus) override
    {
        log("%p, %p, %s\n", this, m_d3d8dev, __FUNCTION__);
        return m_d3d8dev->GetClipStatus(pClipStatus);
    }

    HRESULT STDMETHODCALLTYPE GetTexture(DWORD Stage, IDirect3DBaseTexture8** ppTexture) override
    {
        log("%p, %p, %s\n", this, m_d3d8dev, __FUNCTION__);
        return m_d3d8dev->GetTexture(Stage, ppTexture);
    }

    HRESULT STDMETHODCALLTYPE SetTexture(DWORD Stage, IDirect3DBaseTexture8* pTexture) override
    {
        log("%p, %p, %s (%d, %p)\n", this, m_d3d8dev, __FUNCTION__, Stage, pTexture);
        return m_d3d8dev->SetTexture(Stage, pTexture);
    }

    HRESULT STDMETHODCALLTYPE GetTextureStageState(DWORD Stage, D3DTEXTURESTAGESTATETYPE Type, DWORD* pValue) override
    {
        log("%p, %p, %s\n", this, m_d3d8dev, __FUNCTION__);
        return m_d3d8dev->GetTextureStageState(Stage, Type, pValue);
    }

    HRESULT STDMETHODCALLTYPE SetTextureStageState(DWORD Stage, D3DTEXTURESTAGESTATETYPE Type, DWORD Value) override
    {
        log("%p, %p, %s (%d, %d, %d)\n", this, m_d3d8dev, __FUNCTION__, Stage, Type, Value);
        return m_d3d8dev->SetTextureStageState(Stage, Type, Value);
    }

    HRESULT STDMETHODCALLTYPE ValidateDevice(DWORD* pNumPasses) override
    {
        log("%p, %p, %s\n", this, m_d3d8dev, __FUNCTION__);
        return m_d3d8dev->ValidateDevice(pNumPasses);
    }

    HRESULT STDMETHODCALLTYPE GetInfo(DWORD DevInfoID, void* pDevInfoStruct, DWORD DevInfoStructSize) override
    {
        log("%p, %p, %s\n", this, m_d3d8dev, __FUNCTION__);
        return m_d3d8dev->GetInfo(DevInfoID, pDevInfoStruct, DevInfoStructSize);
    }

    HRESULT STDMETHODCALLTYPE SetPaletteEntries(UINT PaletteNumber, CONST PALETTEENTRY* pEntries) override
    {
        log("%p, %p, %s\n", this, m_d3d8dev, __FUNCTION__);
        return m_d3d8dev->SetPaletteEntries(PaletteNumber, pEntries);
    }

    HRESULT STDMETHODCALLTYPE GetPaletteEntries(UINT PaletteNumber, PALETTEENTRY* pEntries) override
    {
        log("%p, %p, %s\n", this, m_d3d8dev, __FUNCTION__);
        return m_d3d8dev->GetPaletteEntries(PaletteNumber, pEntries);
    }

    HRESULT STDMETHODCALLTYPE SetCurrentTexturePalette(UINT PaletteNumber) override
    {
        log("%p, %p, %s\n", this, m_d3d8dev, __FUNCTION__);
        return m_d3d8dev->SetCurrentTexturePalette(PaletteNumber);
    }

    HRESULT STDMETHODCALLTYPE GetCurrentTexturePalette(UINT *PaletteNumber) override
    {
        log("%p, %p, %s\n", this, m_d3d8dev, __FUNCTION__);
        return m_d3d8dev->GetCurrentTexturePalette(PaletteNumber);
    }

    HRESULT STDMETHODCALLTYPE DrawPrimitive(D3DPRIMITIVETYPE PrimitiveType, UINT StartVertex, UINT PrimitiveCount) override
    {
        log("%p, %p, %s (%d, %d, %d)\n", this, m_d3d8dev, __FUNCTION__, PrimitiveType, StartVertex, PrimitiveCount);
        return m_d3d8dev->DrawPrimitive(PrimitiveType, StartVertex, PrimitiveCount);
    }

    HRESULT STDMETHODCALLTYPE DrawIndexedPrimitive(D3DPRIMITIVETYPE PrimitiveType, UINT MinIndex, UINT NumVertices, UINT startIndex, UINT primCount) override
    {
        log("%p, %p, %s (%d, %d, %d, %d, %d)\n", this, m_d3d8dev, __FUNCTION__, PrimitiveType, MinIndex, NumVertices, startIndex, primCount);
        return m_d3d8dev->DrawIndexedPrimitive(PrimitiveType, MinIndex, NumVertices, startIndex, primCount);
    }

    HRESULT STDMETHODCALLTYPE DrawPrimitiveUP(D3DPRIMITIVETYPE PrimitiveType, UINT PrimitiveCount, CONST void* pVertexStreamZeroData, UINT VertexStreamZeroStride) override
    {
        log("%p, %p, %s (%d, %d, %p, %d)\n", this, m_d3d8dev, __FUNCTION__, PrimitiveType, PrimitiveCount, pVertexStreamZeroData, VertexStreamZeroStride);
        return m_d3d8dev->DrawPrimitiveUP(PrimitiveType, PrimitiveCount, pVertexStreamZeroData, VertexStreamZeroStride);
    }

    HRESULT STDMETHODCALLTYPE DrawIndexedPrimitiveUP(D3DPRIMITIVETYPE PrimitiveType, UINT MinVertexIndex, UINT NumVertices, UINT PrimitiveCount, CONST void* pIndexData, D3DFORMAT IndexDataFormat, CONST void* pVertexStreamZeroData, UINT VertexStreamZeroStride) override
    {
        log("%p, %p, %s\n", this, m_d3d8dev, __FUNCTION__);
        return m_d3d8dev->DrawIndexedPrimitiveUP(PrimitiveType, MinVertexIndex, NumVertices, PrimitiveCount, pIndexData, IndexDataFormat, pVertexStreamZeroData, VertexStreamZeroStride);
    }

    HRESULT STDMETHODCALLTYPE ProcessVertices(UINT SrcStartIndex, UINT DestIndex, UINT VertexCount, IDirect3DVertexBuffer8* pDestBuffer, DWORD Flags) override
    {
        log("%p, %p, %s\n", this, m_d3d8dev, __FUNCTION__);
        return m_d3d8dev->ProcessVertices(SrcStartIndex, DestIndex, VertexCount, pDestBuffer, Flags);
    }

    HRESULT STDMETHODCALLTYPE CreateVertexShader(CONST DWORD* pDeclaration, CONST DWORD* pFunction, DWORD* pHandle, DWORD Usage) override
    {
        log("%p, %p, %s\n", this, m_d3d8dev, __FUNCTION__);
        return m_d3d8dev->CreateVertexShader(pDeclaration, pFunction, pHandle, Usage);
    }

    HRESULT STDMETHODCALLTYPE SetVertexShader(DWORD Handle) override
    {
        log("%p, %p, %s (%d)\n", this, m_d3d8dev, __FUNCTION__, Handle);
        return m_d3d8dev->SetVertexShader(Handle);
    }

    HRESULT STDMETHODCALLTYPE GetVertexShader(DWORD* pHandle) override
    {
        log("%p, %p, %s\n", this, m_d3d8dev, __FUNCTION__);
        return m_d3d8dev->GetVertexShader(pHandle);
    }

    HRESULT STDMETHODCALLTYPE DeleteVertexShader(DWORD Handle) override
    {
        log("%p, %p, %s\n", this, m_d3d8dev, __FUNCTION__);
        return m_d3d8dev->DeleteVertexShader(Handle);
    }

    HRESULT STDMETHODCALLTYPE SetVertexShaderConstant(DWORD Register, CONST void* pConstantData, DWORD ConstantCount) override
    {
        log("%p, %p, %s\n", this, m_d3d8dev, __FUNCTION__);
        return m_d3d8dev->SetVertexShaderConstant(Register, pConstantData, ConstantCount);
    }

    HRESULT STDMETHODCALLTYPE GetVertexShaderConstant(DWORD Register, void* pConstantData, DWORD ConstantCount) override
    {
        log("%p, %p, %s\n", this, m_d3d8dev, __FUNCTION__);
        return m_d3d8dev->GetVertexShaderConstant(Register, pConstantData, ConstantCount);
    }

    HRESULT STDMETHODCALLTYPE GetVertexShaderDeclaration(DWORD Handle, void* pData, DWORD* pSizeOfData) override
    {
        log("%p, %p, %s\n", this, m_d3d8dev, __FUNCTION__);
        return m_d3d8dev->GetVertexShaderDeclaration(Handle, pData, pSizeOfData);
    }

    HRESULT STDMETHODCALLTYPE GetVertexShaderFunction(DWORD Handle, void* pData, DWORD* pSizeOfData) override
    {
        log("%p, %p, %s\n", this, m_d3d8dev, __FUNCTION__);
        return m_d3d8dev->GetVertexShaderFunction(Handle, pData, pSizeOfData);
    }

    HRESULT STDMETHODCALLTYPE SetStreamSource(UINT StreamNumber, IDirect3DVertexBuffer8* pStreamData, UINT Stride) override
    {
        log("%p, %p, %s (%d, %p, %d)\n", this, m_d3d8dev, __FUNCTION__, StreamNumber, pStreamData, Stride);
        return m_d3d8dev->SetStreamSource(StreamNumber, pStreamData, Stride);
    }

    HRESULT STDMETHODCALLTYPE GetStreamSource(UINT StreamNumber, IDirect3DVertexBuffer8** ppStreamData, UINT* pStride) override
    {
        log("%p, %p, %s\n", this, m_d3d8dev, __FUNCTION__);
        return m_d3d8dev->GetStreamSource(StreamNumber, ppStreamData, pStride);
    }

    HRESULT STDMETHODCALLTYPE SetIndices(IDirect3DIndexBuffer8* pIndexData, UINT BaseVertexIndex) override
    {
        log("%p, %p, %s (%p, %d)\n", this, m_d3d8dev, __FUNCTION__, pIndexData, BaseVertexIndex);
        return m_d3d8dev->SetIndices(pIndexData, BaseVertexIndex);
    }

    HRESULT STDMETHODCALLTYPE GetIndices(IDirect3DIndexBuffer8** ppIndexData, UINT* pBaseVertexIndex) override
    {
        log("%p, %p, %s\n", this, m_d3d8dev, __FUNCTION__);
        return m_d3d8dev->GetIndices(ppIndexData, pBaseVertexIndex);
    }

    HRESULT STDMETHODCALLTYPE CreatePixelShader(CONST DWORD* pFunction, DWORD* pHandle) override
    {
        log("%p, %p, %s\n", this, m_d3d8dev, __FUNCTION__);
        return m_d3d8dev->CreatePixelShader(pFunction, pHandle);
    }

    HRESULT STDMETHODCALLTYPE SetPixelShader(DWORD Handle) override
    {
        log("%p, %p, %s\n", this, m_d3d8dev, __FUNCTION__);
        return m_d3d8dev->SetPixelShader(Handle);
    }

    HRESULT STDMETHODCALLTYPE GetPixelShader(DWORD* pHandle) override
    {
        log("%p, %p, %s\n", this, m_d3d8dev, __FUNCTION__);
        return m_d3d8dev->GetPixelShader(pHandle);
    }

    HRESULT STDMETHODCALLTYPE DeletePixelShader(DWORD Handle) override
    {
        log("%p, %p, %s\n", this, m_d3d8dev, __FUNCTION__);
        return m_d3d8dev->DeletePixelShader(Handle);
    }

    HRESULT STDMETHODCALLTYPE SetPixelShaderConstant(DWORD Register, CONST void* pConstantData, DWORD ConstantCount) override
    {
        log("%p, %p, %s\n", this, m_d3d8dev, __FUNCTION__);
        return m_d3d8dev->SetPixelShaderConstant(Register, pConstantData, ConstantCount);
    }

    HRESULT STDMETHODCALLTYPE GetPixelShaderConstant(DWORD Register, void* pConstantData, DWORD ConstantCount) override
    {
        log("%p, %p, %s\n", this, m_d3d8dev, __FUNCTION__);
        return m_d3d8dev->GetPixelShaderConstant(Register, pConstantData, ConstantCount);
    }

    HRESULT STDMETHODCALLTYPE GetPixelShaderFunction(DWORD Handle, void* pData, DWORD* pSizeOfData) override
    {
        log("%p, %p, %s\n", this, m_d3d8dev, __FUNCTION__);
        return m_d3d8dev->GetPixelShaderFunction(Handle, pData, pSizeOfData);
    }

    HRESULT STDMETHODCALLTYPE DrawRectPatch(UINT Handle, CONST float* pNumSegs, CONST D3DRECTPATCH_INFO* pRectPatchInfo) override
    {
        log("%p, %p, %s\n", this, m_d3d8dev, __FUNCTION__);
        return m_d3d8dev->DrawRectPatch(Handle, pNumSegs, pRectPatchInfo);
    }

    HRESULT STDMETHODCALLTYPE DrawTriPatch(UINT Handle, CONST float* pNumSegs, CONST D3DTRIPATCH_INFO* pTriPatchInfo) override
    {
        log("%p, %p, %s\n", this, m_d3d8dev, __FUNCTION__);
        return m_d3d8dev->DrawTriPatch(Handle, pNumSegs, pTriPatchInfo);
    }

    HRESULT STDMETHODCALLTYPE DeletePatch(UINT Handle) override
    {
        log("%p, %p, %s\n", this, m_d3d8dev, __FUNCTION__);
        return m_d3d8dev->DeletePatch(Handle);
    }

    virtual ~TAKPIDirect3DDevice8() {}
private:
    IDirect3D8* m_d3d8;
    IDirect3DDevice8* m_d3d8dev;
};

// Intercept the IDirect3D8 interface
class TAKPIDirect3D8 : public IDirect3D8
{
public:
    TAKPIDirect3D8(IDirect3D8* d3d8) : m_d3d8(d3d8)
    {
        log("%p, %p, %s\n", this, m_d3d8, __FUNCTION__);
    }

    virtual ~TAKPIDirect3D8() {}

    // IUnknown
    virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObj) override
    {
        log("%p, %p, %s\n", this, m_d3d8, __FUNCTION__);
        return m_d3d8->QueryInterface(riid, ppvObj);
    }

    virtual ULONG STDMETHODCALLTYPE AddRef() override
    {
        log("%p, %p, %s\n", this, m_d3d8, __FUNCTION__);
        return m_d3d8->AddRef();
    }

    virtual ULONG STDMETHODCALLTYPE Release() override
    {
        log("%p, %p, %s\n", this, m_d3d8, __FUNCTION__);
        auto result = m_d3d8->Release();
        if (result == 0)
        {
            log("We're done with this! %p %d\n", this, result);
            delete this;
        }
        return result;
    }

    // IDirect3D8
    virtual HRESULT STDMETHODCALLTYPE RegisterSoftwareDevice(void* pInitializeFunction) override
    {
        log("%p, %p, %s\n", this, m_d3d8, __FUNCTION__);
        return m_d3d8->RegisterSoftwareDevice(pInitializeFunction);
    }

    virtual UINT STDMETHODCALLTYPE GetAdapterCount() override
    {
        log("%p, %p, %s ()\n", this, m_d3d8, __FUNCTION__);
        return m_d3d8->GetAdapterCount();
    }

    virtual HRESULT STDMETHODCALLTYPE GetAdapterIdentifier(UINT Adapter, DWORD Flags, D3DADAPTER_IDENTIFIER8* pIdentifier) override
    {
        log("%p, %p, %s (%d, %d, %p)\n", this, m_d3d8, __FUNCTION__, Adapter, Flags, pIdentifier);
        return m_d3d8->GetAdapterIdentifier(Adapter, Flags, pIdentifier);
    }

    virtual UINT STDMETHODCALLTYPE GetAdapterModeCount(UINT Adapter) override
    {
        log("%p, %p, %s (%d)\n", this, m_d3d8, __FUNCTION__, Adapter);
        return m_d3d8->GetAdapterModeCount(Adapter);
    }

    virtual HRESULT STDMETHODCALLTYPE EnumAdapterModes(UINT Adapter, UINT Mode, D3DDISPLAYMODE* pMode) override
    {
        log("%p, %p, %s (%d, %d, %p)\n", this, m_d3d8, __FUNCTION__);
        return m_d3d8->EnumAdapterModes(Adapter, Mode, pMode);
    }

    virtual HRESULT STDMETHODCALLTYPE GetAdapterDisplayMode(UINT Adapter, D3DDISPLAYMODE* pMode) override
    {
        log("%p, %p, %s (%d, %p)\n", this, m_d3d8, __FUNCTION__, Adapter, pMode);
        return m_d3d8->GetAdapterDisplayMode(Adapter, pMode);
    }

    virtual HRESULT STDMETHODCALLTYPE CheckDeviceType(UINT Adapter, D3DDEVTYPE CheckType, D3DFORMAT DisplayFormat, D3DFORMAT BackBufferFormat, BOOL Windowed) override
    {
        log("%p, %p, %s (%d, %d, %d, %d, %d)\n", this, m_d3d8, __FUNCTION__, Adapter, CheckType, DisplayFormat, BackBufferFormat, Windowed);
        return m_d3d8->CheckDeviceType(Adapter, CheckType, DisplayFormat, BackBufferFormat, Windowed);
    }

    virtual HRESULT STDMETHODCALLTYPE CheckDeviceFormat(UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT AdapterFormat, DWORD Usage, D3DRESOURCETYPE RType, D3DFORMAT CheckFormat) override
    {
        log("%p, %p, %s (%u, %d, %d, %d, %d, %d)\n", this, m_d3d8, __FUNCTION__, Adapter, DeviceType, AdapterFormat, Usage, RType, CheckFormat);
        return m_d3d8->CheckDeviceFormat(Adapter, DeviceType, AdapterFormat, Usage, RType, CheckFormat);
    }

    virtual HRESULT STDMETHODCALLTYPE CheckDeviceMultiSampleType(UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT SurfaceFormat, BOOL Windowed, D3DMULTISAMPLE_TYPE MultiSampleType) override
    {
        log("%p, %p, %s\n", this, m_d3d8, __FUNCTION__);
        return m_d3d8->CheckDeviceMultiSampleType(Adapter, DeviceType, SurfaceFormat, Windowed, MultiSampleType);
    }

    virtual HRESULT STDMETHODCALLTYPE CheckDepthStencilMatch(UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT AdapterFormat, D3DFORMAT RenderTargetFormat, D3DFORMAT DepthStencilFormat) override
    {
        log("%p, %p, %s\n", this, m_d3d8, __FUNCTION__);
        return m_d3d8->CheckDepthStencilMatch(Adapter, DeviceType, AdapterFormat, RenderTargetFormat, DepthStencilFormat);
    }

    virtual HRESULT STDMETHODCALLTYPE GetDeviceCaps(UINT Adapter, D3DDEVTYPE DeviceType, D3DCAPS8* pCaps) override
    {
        log("%p, %p, %s (%d, %d, %p)\n", this, m_d3d8, __FUNCTION__, Adapter, DeviceType, pCaps);
        return m_d3d8->GetDeviceCaps(Adapter, DeviceType, pCaps);
    }

    virtual HMONITOR STDMETHODCALLTYPE GetAdapterMonitor(UINT Adapter) override
    {
        log("%p, %p, %s\n", this, m_d3d8, __FUNCTION__);
        return m_d3d8->GetAdapterMonitor(Adapter);
    }

    virtual HRESULT STDMETHODCALLTYPE CreateDevice(UINT Adapter, D3DDEVTYPE DeviceType, HWND hFocusWindow, DWORD BehaviorFlags, D3DPRESENT_PARAMETERS* pPresentationParameters, IDirect3DDevice8** ppReturnedDeviceInterface) override
    {
        log("%p, %p, %s (%d, %d, %p, %d, %p, %p)\n", this, m_d3d8, __FUNCTION__, Adapter, DeviceType, hFocusWindow, BehaviorFlags, pPresentationParameters, ppReturnedDeviceInterface);
        IDirect3DDevice8* newDevice;
        auto result = m_d3d8->CreateDevice(Adapter, DeviceType, hFocusWindow, BehaviorFlags, pPresentationParameters, &newDevice);
        *ppReturnedDeviceInterface = new TAKPIDirect3DDevice8(this, newDevice);
        log("    %p, %p, %s (...) = %p\n", this, m_d3d8, __FUNCTION__, *ppReturnedDeviceInterface);

        return result;
    }

private:
    IDirect3D8* m_d3d8;
};

// Intercept the one function call
typedef IDirect3D8* (*orig_Direct3DCreate8)(UINT);

IDirect3D8* WINAPI Direct3DCreate8(UINT SDKVersion)
{
    printf("Direct3DCreate8 %d\n", SDKVersion);
    orig_Direct3DCreate8 orig = (orig_Direct3DCreate8)GetProcAddress(g_d3d8, "Direct3DCreate8");
    return new TAKPIDirect3D8(orig(SDKVersion));
}
