#include "takpw.h"
#include "log.h"

#include <stdio.h>
#include <string.h>

#include <winuser.h>

HMODULE g_d3d8 = NULL;

typedef struct TAKPIDirect3D8
{
    struct IDirect3D8Vtbl* lpVtbl;
    struct IDirect3D8Vtbl* backup_vtbl;
    IDirect3D8* m_d3d8;
} TAKPIDirect3D8;

typedef struct TAKPIDirect3DDevice8
{
    struct IDirect3DDevice8Vtbl* lpVtbl;
    struct IDirect3DDevice8Vtbl* backup_vtbl;
    IDirect3DDevice8* m_d3d8dev;
    TAKPIDirect3D8* m_d3d8;
} TAKPIDirect3DDevice8;

#define validate_vtable_entry(lhs, rhs, field) \
    if (lhs->field != rhs->field) \
    { \
        log(#field " mismatch %p!=%p\n", lhs->field, rhs->field); \
    }
void validate_IDirect3D8(IDirect3D8Vtbl* lhs, IDirect3D8Vtbl* rhs)
{
    validate_vtable_entry(lhs, rhs, QueryInterface);
    validate_vtable_entry(lhs, rhs, AddRef);
    validate_vtable_entry(lhs, rhs, Release);
    validate_vtable_entry(lhs, rhs, RegisterSoftwareDevice);
    validate_vtable_entry(lhs, rhs, GetAdapterCount);
    validate_vtable_entry(lhs, rhs, GetAdapterIdentifier);
    validate_vtable_entry(lhs, rhs, GetAdapterModeCount);
    validate_vtable_entry(lhs, rhs, EnumAdapterModes);
    validate_vtable_entry(lhs, rhs, GetAdapterDisplayMode);
    validate_vtable_entry(lhs, rhs, CheckDeviceType);
    validate_vtable_entry(lhs, rhs, CheckDeviceFormat);
    validate_vtable_entry(lhs, rhs, CheckDeviceMultiSampleType);
    validate_vtable_entry(lhs, rhs, CheckDepthStencilMatch);
    validate_vtable_entry(lhs, rhs, GetDeviceCaps);
    validate_vtable_entry(lhs, rhs, GetAdapterMonitor);
    validate_vtable_entry(lhs, rhs, CreateDevice);
}
void validate_IDirect3DDevice8(IDirect3DDevice8Vtbl* lhs, IDirect3DDevice8Vtbl* rhs)
{
    validate_vtable_entry(lhs, rhs, QueryInterface);
    validate_vtable_entry(lhs, rhs, AddRef);
    validate_vtable_entry(lhs, rhs, Release);
    validate_vtable_entry(lhs, rhs, TestCooperativeLevel);
    validate_vtable_entry(lhs, rhs, GetAvailableTextureMem);
    validate_vtable_entry(lhs, rhs, ResourceManagerDiscardBytes);
    validate_vtable_entry(lhs, rhs, GetDirect3D);
    validate_vtable_entry(lhs, rhs, GetDeviceCaps);
    validate_vtable_entry(lhs, rhs, GetDisplayMode);
    validate_vtable_entry(lhs, rhs, GetCreationParameters);
    validate_vtable_entry(lhs, rhs, SetCursorProperties);
    validate_vtable_entry(lhs, rhs, SetCursorPosition);
    validate_vtable_entry(lhs, rhs, ShowCursor);
    validate_vtable_entry(lhs, rhs, CreateAdditionalSwapChain);
    validate_vtable_entry(lhs, rhs, Reset);
    validate_vtable_entry(lhs, rhs, Present);
    validate_vtable_entry(lhs, rhs, GetBackBuffer);
    validate_vtable_entry(lhs, rhs, GetRasterStatus);
    validate_vtable_entry(lhs, rhs,  SetGammaRamp);
    validate_vtable_entry(lhs, rhs, GetGammaRamp);
    validate_vtable_entry(lhs, rhs, CreateTexture);
    validate_vtable_entry(lhs, rhs, CreateVolumeTexture);
    validate_vtable_entry(lhs, rhs, CreateCubeTexture);
    validate_vtable_entry(lhs, rhs, CreateVertexBuffer);
    validate_vtable_entry(lhs, rhs, CreateIndexBuffer);
    validate_vtable_entry(lhs, rhs, CreateRenderTarget);
    validate_vtable_entry(lhs, rhs, CreateDepthStencilSurface);
    validate_vtable_entry(lhs, rhs, CreateImageSurface);
    validate_vtable_entry(lhs, rhs, CopyRects);
    validate_vtable_entry(lhs, rhs, UpdateTexture);
    validate_vtable_entry(lhs, rhs, GetFrontBuffer);
    validate_vtable_entry(lhs, rhs, SetRenderTarget);
    validate_vtable_entry(lhs, rhs, GetRenderTarget);
    validate_vtable_entry(lhs, rhs, GetDepthStencilSurface);
    validate_vtable_entry(lhs, rhs, BeginScene);
    validate_vtable_entry(lhs, rhs, EndScene);
    validate_vtable_entry(lhs, rhs, Clear);
    validate_vtable_entry(lhs, rhs, SetTransform);
    validate_vtable_entry(lhs, rhs, GetTransform);
    validate_vtable_entry(lhs, rhs, MultiplyTransform);
    validate_vtable_entry(lhs, rhs, SetViewport);
    validate_vtable_entry(lhs, rhs, GetViewport);
    validate_vtable_entry(lhs, rhs, SetMaterial);
    validate_vtable_entry(lhs, rhs, GetMaterial);
    validate_vtable_entry(lhs, rhs, SetLight);
    validate_vtable_entry(lhs, rhs, GetLight);
    validate_vtable_entry(lhs, rhs, LightEnable);
    validate_vtable_entry(lhs, rhs, GetLightEnable);
    validate_vtable_entry(lhs, rhs, SetClipPlane);
    validate_vtable_entry(lhs, rhs, GetClipPlane);
    validate_vtable_entry(lhs, rhs, SetRenderState);
    validate_vtable_entry(lhs, rhs, GetRenderState);
    validate_vtable_entry(lhs, rhs, BeginStateBlock);
    validate_vtable_entry(lhs, rhs, EndStateBlock);
    validate_vtable_entry(lhs, rhs, ApplyStateBlock);
    validate_vtable_entry(lhs, rhs, CaptureStateBlock);
    validate_vtable_entry(lhs, rhs, DeleteStateBlock);
    validate_vtable_entry(lhs, rhs, CreateStateBlock);
    validate_vtable_entry(lhs, rhs, SetClipStatus);
    validate_vtable_entry(lhs, rhs, GetClipStatus);
    validate_vtable_entry(lhs, rhs, GetTexture);
    validate_vtable_entry(lhs, rhs, SetTexture);
    validate_vtable_entry(lhs, rhs, GetTextureStageState);
    validate_vtable_entry(lhs, rhs, SetTextureStageState);
    validate_vtable_entry(lhs, rhs, ValidateDevice);
    validate_vtable_entry(lhs, rhs, GetInfo);
    validate_vtable_entry(lhs, rhs, SetPaletteEntries);
    validate_vtable_entry(lhs, rhs, GetPaletteEntries);
    validate_vtable_entry(lhs, rhs, SetCurrentTexturePalette);
    validate_vtable_entry(lhs, rhs, GetCurrentTexturePalette);
    validate_vtable_entry(lhs, rhs, DrawPrimitive);
    validate_vtable_entry(lhs, rhs, DrawIndexedPrimitive);
    validate_vtable_entry(lhs, rhs, DrawPrimitiveUP);
    validate_vtable_entry(lhs, rhs, DrawIndexedPrimitiveUP);
    validate_vtable_entry(lhs, rhs, ProcessVertices);
    validate_vtable_entry(lhs, rhs, CreateVertexShader);
    validate_vtable_entry(lhs, rhs, SetVertexShader);
    validate_vtable_entry(lhs, rhs, GetVertexShader);
    validate_vtable_entry(lhs, rhs, DeleteVertexShader);
    validate_vtable_entry(lhs, rhs, SetVertexShaderConstant);
    validate_vtable_entry(lhs, rhs, GetVertexShaderConstant);
    validate_vtable_entry(lhs, rhs, GetVertexShaderDeclaration);
    validate_vtable_entry(lhs, rhs, GetVertexShaderFunction);
    validate_vtable_entry(lhs, rhs, SetStreamSource);
    validate_vtable_entry(lhs, rhs, GetStreamSource);
    validate_vtable_entry(lhs, rhs, SetIndices);
    validate_vtable_entry(lhs, rhs, GetIndices);
    validate_vtable_entry(lhs, rhs, CreatePixelShader);
    validate_vtable_entry(lhs, rhs, SetPixelShader);
    validate_vtable_entry(lhs, rhs, GetPixelShader);
    validate_vtable_entry(lhs, rhs, DeletePixelShader);
    validate_vtable_entry(lhs, rhs, SetPixelShaderConstant);
    validate_vtable_entry(lhs, rhs, GetPixelShaderConstant);
    validate_vtable_entry(lhs, rhs, GetPixelShaderFunction);
    validate_vtable_entry(lhs, rhs, DrawRectPatch);
    validate_vtable_entry(lhs, rhs, DrawTriPatch);
    validate_vtable_entry(lhs, rhs, DeletePatch);
}

HRESULT STDMETHODCALLTYPE d3d8dev_QueryInterface(TAKPIDirect3DDevice8* this, REFIID riid, void** ppvObj)
{
    validate_IDirect3DDevice8(this->m_d3d8dev->lpVtbl, this->backup_vtbl);
    log("%p, %p, %s\n", this, this->m_d3d8dev, __FUNCTION__);
    return this->m_d3d8dev->lpVtbl->QueryInterface(this->m_d3d8dev, riid, ppvObj);
}

ULONG STDMETHODCALLTYPE d3d8dev_AddRef(TAKPIDirect3DDevice8* this)
{
    validate_IDirect3DDevice8(this->m_d3d8dev->lpVtbl, this->backup_vtbl);
    log("%p, %p, %s\n", this, this->m_d3d8dev, __FUNCTION__);
    return this->m_d3d8dev->lpVtbl->AddRef(this->m_d3d8dev);
}

ULONG STDMETHODCALLTYPE d3d8dev_Release(TAKPIDirect3DDevice8* this)
{
    validate_IDirect3DDevice8(this->m_d3d8dev->lpVtbl, this->backup_vtbl);
    log("%p, %p, %s\n", this, this->m_d3d8dev, __FUNCTION__);
    ULONG result = this->m_d3d8dev->lpVtbl->Release(this->m_d3d8dev);
    if (result == 0)
    {
        log("We're done with this! %p %d\n", this, result);
        free(this->backup_vtbl);
        free(this->lpVtbl);
        free(this);
    }
    return result;
}

HRESULT STDMETHODCALLTYPE d3d8dev_TestCooperativeLevel(TAKPIDirect3DDevice8* this)
{
    validate_IDirect3DDevice8(this->m_d3d8dev->lpVtbl, this->backup_vtbl);
    log("%p, %p, %s\n", this, this->m_d3d8dev, __FUNCTION__);
    return this->m_d3d8dev->lpVtbl->TestCooperativeLevel(this->m_d3d8dev);
}

UINT STDMETHODCALLTYPE d3d8dev_GetAvailableTextureMem(TAKPIDirect3DDevice8* this)
{
    validate_IDirect3DDevice8(this->m_d3d8dev->lpVtbl, this->backup_vtbl);
    log("%p, %p, %s ()\n", this, this->m_d3d8dev, __FUNCTION__);
    return this->m_d3d8dev->lpVtbl->GetAvailableTextureMem(this->m_d3d8dev);
}

HRESULT STDMETHODCALLTYPE d3d8dev_ResourceManagerDiscardBytes(TAKPIDirect3DDevice8* this, DWORD Bytes)
{
    validate_IDirect3DDevice8(this->m_d3d8dev->lpVtbl, this->backup_vtbl);
    log("%p, %p, %s\n", this, this->m_d3d8dev, __FUNCTION__);
    return this->m_d3d8dev->lpVtbl->ResourceManagerDiscardBytes(this->m_d3d8dev, Bytes);
}

HRESULT STDMETHODCALLTYPE d3d8dev_GetDirect3D(TAKPIDirect3DDevice8* this, IDirect3D8** ppD3D8)
{
    validate_IDirect3DDevice8(this->m_d3d8dev->lpVtbl, this->backup_vtbl);
    log("%p, %p, %s (%p)\n", this, this->m_d3d8dev, __FUNCTION__);
    HRESULT result = D3D_OK;
    result = this->m_d3d8dev->lpVtbl->GetDirect3D(this->m_d3d8dev, ppD3D8);
    if (result == D3D_OK)
    {
        *ppD3D8 = (IDirect3D8*)this->m_d3d8;
        log("    %p, %p, %s () = %p\n", this, this->m_d3d8dev, __FUNCTION__, *ppD3D8);
    }
    return result;
}

HRESULT STDMETHODCALLTYPE d3d8dev_GetDeviceCaps(TAKPIDirect3DDevice8* this, D3DCAPS8* pCaps)
{
    validate_IDirect3DDevice8(this->m_d3d8dev->lpVtbl, this->backup_vtbl);
    log("%p, %p, %s (%p)\n", this, this->m_d3d8dev, __FUNCTION__, pCaps);
    return this->m_d3d8dev->lpVtbl->GetDeviceCaps(this->m_d3d8dev, pCaps);
}

HRESULT STDMETHODCALLTYPE d3d8dev_GetDisplayMode(TAKPIDirect3DDevice8* this, D3DDISPLAYMODE* pMode)
{
    validate_IDirect3DDevice8(this->m_d3d8dev->lpVtbl, this->backup_vtbl);
    log("%p, %p, %s (%p)\n", this, this->m_d3d8dev, __FUNCTION__, pMode);
    return this->m_d3d8dev->lpVtbl->GetDisplayMode(this->m_d3d8dev, pMode);
}

HRESULT STDMETHODCALLTYPE d3d8dev_GetCreationParameters(TAKPIDirect3DDevice8* this, D3DDEVICE_CREATION_PARAMETERS* pParameters)
{
    validate_IDirect3DDevice8(this->m_d3d8dev->lpVtbl, this->backup_vtbl);
    log("%p, %p, %s\n", this, this->m_d3d8dev, __FUNCTION__);
    return this->m_d3d8dev->lpVtbl->GetCreationParameters(this->m_d3d8dev, pParameters);
}

HRESULT STDMETHODCALLTYPE d3d8dev_SetCursorProperties(TAKPIDirect3DDevice8* this, UINT XHotSpot, UINT YHotSpot, IDirect3DSurface8* pCursorBitmap)
{
    validate_IDirect3DDevice8(this->m_d3d8dev->lpVtbl, this->backup_vtbl);
    log("%p, %p, %s\n", this, this->m_d3d8dev, __FUNCTION__);
    return this->m_d3d8dev->lpVtbl->SetCursorProperties(this->m_d3d8dev, XHotSpot, YHotSpot, pCursorBitmap);
}

void STDMETHODCALLTYPE d3d8dev_SetCursorPosition(TAKPIDirect3DDevice8* this, UINT XScreenSpace, UINT YScreenSpace, DWORD Flags)
{
    validate_IDirect3DDevice8(this->m_d3d8dev->lpVtbl, this->backup_vtbl);
    log("%p, %p, %s\n", this, this->m_d3d8dev, __FUNCTION__);
    return this->m_d3d8dev->lpVtbl->SetCursorPosition(this->m_d3d8dev, XScreenSpace, YScreenSpace, Flags);
}

BOOL STDMETHODCALLTYPE d3d8dev_ShowCursor(TAKPIDirect3DDevice8* this, BOOL bShow)
{
    validate_IDirect3DDevice8(this->m_d3d8dev->lpVtbl, this->backup_vtbl);
    log("%p, %p, %s\n", this, this->m_d3d8dev, __FUNCTION__);
    return this->m_d3d8dev->lpVtbl->ShowCursor(this->m_d3d8dev, bShow);
}

HRESULT STDMETHODCALLTYPE d3d8dev_CreateAdditionalSwapChain(TAKPIDirect3DDevice8* this, D3DPRESENT_PARAMETERS* pPresentationParameters, IDirect3DSwapChain8** pSwapChain)
{
    validate_IDirect3DDevice8(this->m_d3d8dev->lpVtbl, this->backup_vtbl);
    log("%p, %p, %s\n", this, this->m_d3d8dev, __FUNCTION__);
    return this->m_d3d8dev->lpVtbl->CreateAdditionalSwapChain(this->m_d3d8dev, pPresentationParameters, pSwapChain);
}

HRESULT STDMETHODCALLTYPE d3d8dev_Reset(TAKPIDirect3DDevice8* this, D3DPRESENT_PARAMETERS* pPresentationParameters)
{
    validate_IDirect3DDevice8(this->m_d3d8dev->lpVtbl, this->backup_vtbl);
    log("%p, %p, %s (%p)\n", this, this->m_d3d8dev, __FUNCTION__, pPresentationParameters);
    return this->m_d3d8dev->lpVtbl->Reset(this->m_d3d8dev, pPresentationParameters);
}

HRESULT STDMETHODCALLTYPE d3d8dev_Present(TAKPIDirect3DDevice8* this, CONST RECT* pSourceRect, CONST RECT* pDestRect, HWND hDestWindowOverride, CONST RGNDATA* pDirtyRegion)
{
    validate_IDirect3DDevice8(this->m_d3d8dev->lpVtbl, this->backup_vtbl);
    //log("%p, %p, %s (%p, %p, %p, %p)\n", this, this->m_d3d8dev, __FUNCTION__, pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion);
    return this->m_d3d8dev->lpVtbl->Present(this->m_d3d8dev, pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion);
}

HRESULT STDMETHODCALLTYPE d3d8dev_GetBackBuffer(TAKPIDirect3DDevice8* this, UINT BackBuffer, D3DBACKBUFFER_TYPE Type, IDirect3DSurface8** ppBackBuffer)
{
    validate_IDirect3DDevice8(this->m_d3d8dev->lpVtbl, this->backup_vtbl);
    log("%p, %p, %s (%d, %d, %p)\n", this, this->m_d3d8dev, __FUNCTION__, BackBuffer, Type, ppBackBuffer);
    return this->m_d3d8dev->lpVtbl->GetBackBuffer(this->m_d3d8dev, BackBuffer, Type, ppBackBuffer);
}

HRESULT STDMETHODCALLTYPE d3d8dev_GetRasterStatus(TAKPIDirect3DDevice8* this, D3DRASTER_STATUS* pRasterStatus)
{
    validate_IDirect3DDevice8(this->m_d3d8dev->lpVtbl, this->backup_vtbl);
    log("%p, %p, %s\n", this, this->m_d3d8dev, __FUNCTION__);
    return this->m_d3d8dev->lpVtbl->GetRasterStatus(this->m_d3d8dev, pRasterStatus);
}

void STDMETHODCALLTYPE d3d8dev_SetGammaRamp(TAKPIDirect3DDevice8* this, DWORD Flags, CONST D3DGAMMARAMP* pRamp)
{
    validate_IDirect3DDevice8(this->m_d3d8dev->lpVtbl, this->backup_vtbl);
    log("%p, %p, %s (%p)\n", this, this->m_d3d8dev, __FUNCTION__, pRamp);
    return this->m_d3d8dev->lpVtbl->SetGammaRamp(this->m_d3d8dev, Flags, pRamp);
}

void STDMETHODCALLTYPE d3d8dev_GetGammaRamp(TAKPIDirect3DDevice8* this, D3DGAMMARAMP* pRamp)
{
    validate_IDirect3DDevice8(this->m_d3d8dev->lpVtbl, this->backup_vtbl);
    log("%p, %p, %s\n", this, this->m_d3d8dev, __FUNCTION__);
    return this->m_d3d8dev->lpVtbl->GetGammaRamp(this->m_d3d8dev, pRamp);
}

HRESULT STDMETHODCALLTYPE d3d8dev_CreateTexture(TAKPIDirect3DDevice8* this, UINT Width, UINT Height, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DTexture8** ppTexture)
{
    validate_IDirect3DDevice8(this->m_d3d8dev->lpVtbl, this->backup_vtbl);
    //log("%p, %p, %s (%d, %d, %d, %d, %d, %d, %p)\n", this, this->m_d3d8dev, __FUNCTION__, Width, Height, Levels, Usage, Format, Pool, ppTexture);
    return this->m_d3d8dev->lpVtbl->CreateTexture(this->m_d3d8dev, Width, Height, Levels, Usage, Format, Pool, ppTexture);
}

HRESULT STDMETHODCALLTYPE d3d8dev_CreateVolumeTexture(TAKPIDirect3DDevice8* this, UINT Width, UINT Height, UINT Depth, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DVolumeTexture8** ppVolumeTexture)
{
    validate_IDirect3DDevice8(this->m_d3d8dev->lpVtbl, this->backup_vtbl);
    log("%p, %p, %s\n", this, this->m_d3d8dev, __FUNCTION__);
    return this->m_d3d8dev->lpVtbl->CreateVolumeTexture(this->m_d3d8dev, Width, Height, Depth, Levels, Usage, Format, Pool, ppVolumeTexture);
}

HRESULT STDMETHODCALLTYPE d3d8dev_CreateCubeTexture(TAKPIDirect3DDevice8* this, UINT EdgeLength, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DCubeTexture8** ppCubeTexture)
{
    validate_IDirect3DDevice8(this->m_d3d8dev->lpVtbl, this->backup_vtbl);
    log("%p, %p, %s\n", this, this->m_d3d8dev, __FUNCTION__);
    return this->m_d3d8dev->lpVtbl->CreateCubeTexture(this->m_d3d8dev, EdgeLength, Levels, Usage, Format, Pool, ppCubeTexture);
}

HRESULT STDMETHODCALLTYPE d3d8dev_CreateVertexBuffer(TAKPIDirect3DDevice8* this, UINT Length, DWORD Usage, DWORD FVF, D3DPOOL Pool, IDirect3DVertexBuffer8** ppVertexBuffer)
{
    validate_IDirect3DDevice8(this->m_d3d8dev->lpVtbl, this->backup_vtbl);
    log("%p, %p, %s (%d, %d, %d, %d, %p)\n", this, this->m_d3d8dev, __FUNCTION__, Length, Usage, FVF, Pool, ppVertexBuffer);
    return this->m_d3d8dev->lpVtbl->CreateVertexBuffer(this->m_d3d8dev, Length, Usage, FVF, Pool, ppVertexBuffer);
}

HRESULT STDMETHODCALLTYPE d3d8dev_CreateIndexBuffer(TAKPIDirect3DDevice8* this, UINT Length, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DIndexBuffer8** ppIndexBuffer)
{
    validate_IDirect3DDevice8(this->m_d3d8dev->lpVtbl, this->backup_vtbl);
    log("%p, %p, %s (%d, %d, %d, %d, %p)\n", this, this->m_d3d8dev, __FUNCTION__, Length, Usage, Format, Pool, ppIndexBuffer);
    return this->m_d3d8dev->lpVtbl->CreateIndexBuffer(this->m_d3d8dev, Length, Usage, Format, Pool, ppIndexBuffer);
}

HRESULT STDMETHODCALLTYPE d3d8dev_CreateRenderTarget(TAKPIDirect3DDevice8* this, UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE MultiSample, BOOL Lockable, IDirect3DSurface8** ppSurface)
{
    validate_IDirect3DDevice8(this->m_d3d8dev->lpVtbl, this->backup_vtbl);
    log("%p, %p, %s\n", this, this->m_d3d8dev, __FUNCTION__);
    return this->m_d3d8dev->lpVtbl->CreateRenderTarget(this->m_d3d8dev, Width, Height, Format, MultiSample, Lockable, ppSurface);
}

HRESULT STDMETHODCALLTYPE d3d8dev_CreateDepthStencilSurface(TAKPIDirect3DDevice8* this, UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE MultiSample, IDirect3DSurface8** ppSurface)
{
    validate_IDirect3DDevice8(this->m_d3d8dev->lpVtbl, this->backup_vtbl);
    log("%p, %p, %s\n", this, this->m_d3d8dev, __FUNCTION__);
    return this->m_d3d8dev->lpVtbl->CreateDepthStencilSurface(this->m_d3d8dev, Width, Height, Format, MultiSample, ppSurface);
}

HRESULT STDMETHODCALLTYPE d3d8dev_CreateImageSurface(TAKPIDirect3DDevice8* this, UINT Width, UINT Height, D3DFORMAT Format, IDirect3DSurface8** ppSurface)
{
    validate_IDirect3DDevice8(this->m_d3d8dev->lpVtbl, this->backup_vtbl);
    log("%p, %p, %s\n", this, this->m_d3d8dev, __FUNCTION__);
    return this->m_d3d8dev->lpVtbl->CreateImageSurface(this->m_d3d8dev, Width, Height, Format, ppSurface);
}

HRESULT STDMETHODCALLTYPE d3d8dev_CopyRects(TAKPIDirect3DDevice8* this, IDirect3DSurface8* pSourceSurface, CONST RECT* pSourceRectsArray, UINT cRects, IDirect3DSurface8* pDestinationSurface, CONST POINT* pDestPointsArray)
{
    validate_IDirect3DDevice8(this->m_d3d8dev->lpVtbl, this->backup_vtbl);
    log("%p, %p, %s\n", this, this->m_d3d8dev, __FUNCTION__);
    return this->m_d3d8dev->lpVtbl->CopyRects(this->m_d3d8dev, pSourceSurface, pSourceRectsArray, cRects, pDestinationSurface, pDestPointsArray);
}

HRESULT STDMETHODCALLTYPE d3d8dev_UpdateTexture(TAKPIDirect3DDevice8* this, IDirect3DBaseTexture8* pSourceTexture, IDirect3DBaseTexture8* pDestinationTexture)
{
    validate_IDirect3DDevice8(this->m_d3d8dev->lpVtbl, this->backup_vtbl);
    log("%p, %p, %s\n", this, this->m_d3d8dev, __FUNCTION__);
    return this->m_d3d8dev->lpVtbl->UpdateTexture(this->m_d3d8dev, pSourceTexture, pDestinationTexture);
}

HRESULT STDMETHODCALLTYPE d3d8dev_GetFrontBuffer(TAKPIDirect3DDevice8* this, IDirect3DSurface8* pDestSurface)
{
    validate_IDirect3DDevice8(this->m_d3d8dev->lpVtbl, this->backup_vtbl);
    log("%p, %p, %s\n", this, this->m_d3d8dev, __FUNCTION__);
    return this->m_d3d8dev->lpVtbl->GetFrontBuffer(this->m_d3d8dev, pDestSurface);
}

HRESULT STDMETHODCALLTYPE d3d8dev_SetRenderTarget(TAKPIDirect3DDevice8* this, IDirect3DSurface8* pRenderTarget, IDirect3DSurface8* pNewZStencil)
{
    validate_IDirect3DDevice8(this->m_d3d8dev->lpVtbl, this->backup_vtbl);
    log("%p, %p, %s\n", this, this->m_d3d8dev, __FUNCTION__);
    return this->m_d3d8dev->lpVtbl->SetRenderTarget(this->m_d3d8dev, pRenderTarget, pNewZStencil);
}

HRESULT STDMETHODCALLTYPE d3d8dev_GetRenderTarget(TAKPIDirect3DDevice8* this, IDirect3DSurface8** ppRenderTarget)
{
    validate_IDirect3DDevice8(this->m_d3d8dev->lpVtbl, this->backup_vtbl);
    log("%p, %p, %s\n", this, this->m_d3d8dev, __FUNCTION__);
    return this->m_d3d8dev->lpVtbl->GetRenderTarget(this->m_d3d8dev, ppRenderTarget);
}

HRESULT STDMETHODCALLTYPE d3d8dev_GetDepthStencilSurface(TAKPIDirect3DDevice8* this, IDirect3DSurface8** ppZStencilSurface)
{
    validate_IDirect3DDevice8(this->m_d3d8dev->lpVtbl, this->backup_vtbl);
    log("%p, %p, %s\n", this, this->m_d3d8dev, __FUNCTION__);
    return this->m_d3d8dev->lpVtbl->GetDepthStencilSurface(this->m_d3d8dev, ppZStencilSurface);
}

HRESULT STDMETHODCALLTYPE d3d8dev_BeginScene(TAKPIDirect3DDevice8* this)
{
    validate_IDirect3DDevice8(this->m_d3d8dev->lpVtbl, this->backup_vtbl);
    //log("%p, %p, %s ()\n", this, this->m_d3d8dev, __FUNCTION__);
    return this->m_d3d8dev->lpVtbl->BeginScene(this->m_d3d8dev);
}

HRESULT STDMETHODCALLTYPE d3d8dev_EndScene(TAKPIDirect3DDevice8* this)
{
    validate_IDirect3DDevice8(this->m_d3d8dev->lpVtbl, this->backup_vtbl);
    //log("%p, %p, %s ()\n", this, this->m_d3d8dev, __FUNCTION__);
    return this->m_d3d8dev->lpVtbl->EndScene(this->m_d3d8dev);
}

HRESULT STDMETHODCALLTYPE d3d8dev_Clear(TAKPIDirect3DDevice8* this, DWORD Count, CONST D3DRECT* pRects, DWORD Flags, D3DCOLOR Color, float Z, DWORD Stencil)
{
    validate_IDirect3DDevice8(this->m_d3d8dev->lpVtbl, this->backup_vtbl);
    //log("%p, %p, %s (%d, %p, %d, %d, %5.2f, %d)\n", this, this->m_d3d8dev, __FUNCTION__, Count, pRects, Flags, Color, Z, Stencil);
    return this->m_d3d8dev->lpVtbl->Clear(this->m_d3d8dev, Count, pRects, Flags, Color, Z, Stencil);
}

HRESULT STDMETHODCALLTYPE d3d8dev_SetTransform(TAKPIDirect3DDevice8* this, D3DTRANSFORMSTATETYPE State, CONST D3DMATRIX* pMatrix)
{
    validate_IDirect3DDevice8(this->m_d3d8dev->lpVtbl, this->backup_vtbl);
    //log("%p, %p, %s (%d, %p)\n", this, this->m_d3d8dev, __FUNCTION__, State, pMatrix);
    return this->m_d3d8dev->lpVtbl->SetTransform(this->m_d3d8dev, State, pMatrix);
}

HRESULT STDMETHODCALLTYPE d3d8dev_GetTransform(TAKPIDirect3DDevice8* this, D3DTRANSFORMSTATETYPE State, D3DMATRIX* pMatrix)
{
    validate_IDirect3DDevice8(this->m_d3d8dev->lpVtbl, this->backup_vtbl);
    log("%p, %p, %s\n", this, this->m_d3d8dev, __FUNCTION__);
    return this->m_d3d8dev->lpVtbl->GetTransform(this->m_d3d8dev, State, pMatrix);
}

HRESULT STDMETHODCALLTYPE d3d8dev_MultiplyTransform(TAKPIDirect3DDevice8* this, D3DTRANSFORMSTATETYPE State, CONST D3DMATRIX* pMatrix)
{
    validate_IDirect3DDevice8(this->m_d3d8dev->lpVtbl, this->backup_vtbl);
    log("%p, %p, %s\n", this, this->m_d3d8dev, __FUNCTION__);
    return this->m_d3d8dev->lpVtbl->MultiplyTransform(this->m_d3d8dev, State, pMatrix);
}

HRESULT STDMETHODCALLTYPE d3d8dev_SetViewport(TAKPIDirect3DDevice8* this, CONST D3DVIEWPORT8* pViewport)
{
    validate_IDirect3DDevice8(this->m_d3d8dev->lpVtbl, this->backup_vtbl);
    //log("%p, %p, %s (%p)\n", this, this->m_d3d8dev, __FUNCTION__, pViewport);
    return this->m_d3d8dev->lpVtbl->SetViewport(this->m_d3d8dev, pViewport);
}

HRESULT STDMETHODCALLTYPE d3d8dev_GetViewport(TAKPIDirect3DDevice8* this, D3DVIEWPORT8* pViewport)
{
    validate_IDirect3DDevice8(this->m_d3d8dev->lpVtbl, this->backup_vtbl);
    //log("%p, %p, %s (%p)\n", this, this->m_d3d8dev, __FUNCTION__, pViewport);
    return this->m_d3d8dev->lpVtbl->GetViewport(this->m_d3d8dev, pViewport);
}

HRESULT STDMETHODCALLTYPE d3d8dev_SetMaterial(TAKPIDirect3DDevice8* this, CONST D3DMATERIAL8* pMaterial)
{
    validate_IDirect3DDevice8(this->m_d3d8dev->lpVtbl, this->backup_vtbl);
    log("%p, %p, %s\n", this, this->m_d3d8dev, __FUNCTION__);
    return this->m_d3d8dev->lpVtbl->SetMaterial(this->m_d3d8dev, pMaterial);
}

HRESULT STDMETHODCALLTYPE d3d8dev_GetMaterial(TAKPIDirect3DDevice8* this, D3DMATERIAL8* pMaterial)
{
    validate_IDirect3DDevice8(this->m_d3d8dev->lpVtbl, this->backup_vtbl);
    log("%p, %p, %s\n", this, this->m_d3d8dev, __FUNCTION__);
    return this->m_d3d8dev->lpVtbl->GetMaterial(this->m_d3d8dev, pMaterial);
}

HRESULT STDMETHODCALLTYPE d3d8dev_SetLight(TAKPIDirect3DDevice8* this, DWORD Index, CONST D3DLIGHT8* pLight)
{
    validate_IDirect3DDevice8(this->m_d3d8dev->lpVtbl, this->backup_vtbl);
    log("%p, %p, %s\n", this, this->m_d3d8dev, __FUNCTION__);
    return this->m_d3d8dev->lpVtbl->SetLight(this->m_d3d8dev, Index, pLight);
}

HRESULT STDMETHODCALLTYPE d3d8dev_GetLight(TAKPIDirect3DDevice8* this, DWORD Index, D3DLIGHT8* pLight)
{
    validate_IDirect3DDevice8(this->m_d3d8dev->lpVtbl, this->backup_vtbl);
    log("%p, %p, %s\n", this, this->m_d3d8dev, __FUNCTION__);
    return this->m_d3d8dev->lpVtbl->GetLight(this->m_d3d8dev, Index, pLight);
}

HRESULT STDMETHODCALLTYPE d3d8dev_LightEnable(TAKPIDirect3DDevice8* this, DWORD Index, BOOL Enable)
{
    validate_IDirect3DDevice8(this->m_d3d8dev->lpVtbl, this->backup_vtbl);
    log("%p, %p, %s\n", this, this->m_d3d8dev, __FUNCTION__);
    return this->m_d3d8dev->lpVtbl->LightEnable(this->m_d3d8dev, Index, Enable);
}

HRESULT STDMETHODCALLTYPE d3d8dev_GetLightEnable(TAKPIDirect3DDevice8* this, DWORD Index, BOOL* pEnable)
{
    validate_IDirect3DDevice8(this->m_d3d8dev->lpVtbl, this->backup_vtbl);
    log("%p, %p, %s\n", this, this->m_d3d8dev, __FUNCTION__);
    return this->m_d3d8dev->lpVtbl->GetLightEnable(this->m_d3d8dev, Index, pEnable);
}

HRESULT STDMETHODCALLTYPE d3d8dev_SetClipPlane(TAKPIDirect3DDevice8* this, DWORD Index, CONST float* pPlane)
{
    validate_IDirect3DDevice8(this->m_d3d8dev->lpVtbl, this->backup_vtbl);
    log("%p, %p, %s\n", this, this->m_d3d8dev, __FUNCTION__);
    return this->m_d3d8dev->lpVtbl->SetClipPlane(this->m_d3d8dev, Index, pPlane);
}

HRESULT STDMETHODCALLTYPE d3d8dev_GetClipPlane(TAKPIDirect3DDevice8* this, DWORD Index, float* pPlane)
{
    validate_IDirect3DDevice8(this->m_d3d8dev->lpVtbl, this->backup_vtbl);
    log("%p, %p, %s\n", this, this->m_d3d8dev, __FUNCTION__);
    return this->m_d3d8dev->lpVtbl->GetClipPlane(this->m_d3d8dev, Index, pPlane);
}

HRESULT STDMETHODCALLTYPE d3d8dev_SetRenderState(TAKPIDirect3DDevice8* this, D3DRENDERSTATETYPE State, DWORD Value)
{
    validate_IDirect3DDevice8(this->m_d3d8dev->lpVtbl, this->backup_vtbl);
    //log("%p, %p, %s (%d, %d)\n", this, this->m_d3d8dev, __FUNCTION__, State, Value);
    return this->m_d3d8dev->lpVtbl->SetRenderState(this->m_d3d8dev, State, Value);
}

HRESULT STDMETHODCALLTYPE d3d8dev_GetRenderState(TAKPIDirect3DDevice8* this, D3DRENDERSTATETYPE State, DWORD* pValue)
{
    validate_IDirect3DDevice8(this->m_d3d8dev->lpVtbl, this->backup_vtbl);
    //log("%p, %p, %s (%d, %p)\n", this, this->m_d3d8dev, __FUNCTION__, State, pValue);
    return this->m_d3d8dev->lpVtbl->GetRenderState(this->m_d3d8dev, State, pValue);
}

HRESULT STDMETHODCALLTYPE d3d8dev_BeginStateBlock(TAKPIDirect3DDevice8* this)
{
    validate_IDirect3DDevice8(this->m_d3d8dev->lpVtbl, this->backup_vtbl);
    log("%p, %p, %s\n", this, this->m_d3d8dev, __FUNCTION__);
    return this->m_d3d8dev->lpVtbl->BeginStateBlock(this->m_d3d8dev);
}

HRESULT STDMETHODCALLTYPE d3d8dev_EndStateBlock(TAKPIDirect3DDevice8* this, DWORD* pToken)
{
    validate_IDirect3DDevice8(this->m_d3d8dev->lpVtbl, this->backup_vtbl);
    log("%p, %p, %s\n", this, this->m_d3d8dev, __FUNCTION__);
    return this->m_d3d8dev->lpVtbl->EndStateBlock(this->m_d3d8dev, pToken);
}

HRESULT STDMETHODCALLTYPE d3d8dev_ApplyStateBlock(TAKPIDirect3DDevice8* this, DWORD Token)
{
    validate_IDirect3DDevice8(this->m_d3d8dev->lpVtbl, this->backup_vtbl);
    log("%p, %p, %s\n", this, this->m_d3d8dev, __FUNCTION__);
    return this->m_d3d8dev->lpVtbl->ApplyStateBlock(this->m_d3d8dev, Token);
}

HRESULT STDMETHODCALLTYPE d3d8dev_CaptureStateBlock(TAKPIDirect3DDevice8* this, DWORD Token)
{
    validate_IDirect3DDevice8(this->m_d3d8dev->lpVtbl, this->backup_vtbl);
    log("%p, %p, %s\n", this, this->m_d3d8dev, __FUNCTION__);
    return this->m_d3d8dev->lpVtbl->CaptureStateBlock(this->m_d3d8dev, Token);
}

HRESULT STDMETHODCALLTYPE d3d8dev_DeleteStateBlock(TAKPIDirect3DDevice8* this, DWORD Token)
{
    validate_IDirect3DDevice8(this->m_d3d8dev->lpVtbl, this->backup_vtbl);
    log("%p, %p, %s\n", this, this->m_d3d8dev, __FUNCTION__);
    return this->m_d3d8dev->lpVtbl->DeleteStateBlock(this->m_d3d8dev, Token);
}

HRESULT STDMETHODCALLTYPE d3d8dev_CreateStateBlock(TAKPIDirect3DDevice8* this, D3DSTATEBLOCKTYPE Type, DWORD* pToken)
{
    validate_IDirect3DDevice8(this->m_d3d8dev->lpVtbl, this->backup_vtbl);
    log("%p, %p, %s\n", this, this->m_d3d8dev, __FUNCTION__);
    return this->m_d3d8dev->lpVtbl->CreateStateBlock(this->m_d3d8dev, Type, pToken);
}

HRESULT STDMETHODCALLTYPE d3d8dev_SetClipStatus(TAKPIDirect3DDevice8* this, CONST D3DCLIPSTATUS8* pClipStatus)
{
    validate_IDirect3DDevice8(this->m_d3d8dev->lpVtbl, this->backup_vtbl);
    log("%p, %p, %s\n", this, this->m_d3d8dev, __FUNCTION__);
    return this->m_d3d8dev->lpVtbl->SetClipStatus(this->m_d3d8dev, pClipStatus);
}

HRESULT STDMETHODCALLTYPE d3d8dev_GetClipStatus(TAKPIDirect3DDevice8* this, D3DCLIPSTATUS8* pClipStatus)
{
    validate_IDirect3DDevice8(this->m_d3d8dev->lpVtbl, this->backup_vtbl);
    log("%p, %p, %s\n", this, this->m_d3d8dev, __FUNCTION__);
    return this->m_d3d8dev->lpVtbl->GetClipStatus(this->m_d3d8dev, pClipStatus);
}

HRESULT STDMETHODCALLTYPE d3d8dev_GetTexture(TAKPIDirect3DDevice8* this, DWORD Stage, IDirect3DBaseTexture8** ppTexture)
{
    validate_IDirect3DDevice8(this->m_d3d8dev->lpVtbl, this->backup_vtbl);
    log("%p, %p, %s\n", this, this->m_d3d8dev, __FUNCTION__);
    return this->m_d3d8dev->lpVtbl->GetTexture(this->m_d3d8dev, Stage, ppTexture);
}

HRESULT STDMETHODCALLTYPE d3d8dev_SetTexture(TAKPIDirect3DDevice8* this, DWORD Stage, IDirect3DBaseTexture8* pTexture)
{
    validate_IDirect3DDevice8(this->m_d3d8dev->lpVtbl, this->backup_vtbl);
    //log("%p, %p, %s (%d, %p)\n", this, this->m_d3d8dev, __FUNCTION__, Stage, pTexture);
    return this->m_d3d8dev->lpVtbl->SetTexture(this->m_d3d8dev, Stage, pTexture);
}

HRESULT STDMETHODCALLTYPE d3d8dev_GetTextureStageState(TAKPIDirect3DDevice8* this, DWORD Stage, D3DTEXTURESTAGESTATETYPE Type, DWORD* pValue)
{
    validate_IDirect3DDevice8(this->m_d3d8dev->lpVtbl, this->backup_vtbl);
    log("%p, %p, %s\n", this, this->m_d3d8dev, __FUNCTION__);
    return this->m_d3d8dev->lpVtbl->GetTextureStageState(this->m_d3d8dev, Stage, Type, pValue);
}

HRESULT STDMETHODCALLTYPE d3d8dev_SetTextureStageState(TAKPIDirect3DDevice8* this, DWORD Stage, D3DTEXTURESTAGESTATETYPE Type, DWORD Value)
{
    validate_IDirect3DDevice8(this->m_d3d8dev->lpVtbl, this->backup_vtbl);
    //log("%p, %p, %s (%d, %d, %d)\n", this, this->m_d3d8dev, __FUNCTION__, Stage, Type, Value);
    return this->m_d3d8dev->lpVtbl->SetTextureStageState(this->m_d3d8dev, Stage, Type, Value);
}

HRESULT STDMETHODCALLTYPE d3d8dev_ValidateDevice(TAKPIDirect3DDevice8* this, DWORD* pNumPasses)
{
    validate_IDirect3DDevice8(this->m_d3d8dev->lpVtbl, this->backup_vtbl);
    log("%p, %p, %s\n", this, this->m_d3d8dev, __FUNCTION__);
    return this->m_d3d8dev->lpVtbl->ValidateDevice(this->m_d3d8dev, pNumPasses);
}

HRESULT STDMETHODCALLTYPE d3d8dev_GetInfo(TAKPIDirect3DDevice8* this, DWORD DevInfoID, void* pDevInfoStruct, DWORD DevInfoStructSize)
{
    validate_IDirect3DDevice8(this->m_d3d8dev->lpVtbl, this->backup_vtbl);
    log("%p, %p, %s\n", this, this->m_d3d8dev, __FUNCTION__);
    return this->m_d3d8dev->lpVtbl->GetInfo(this->m_d3d8dev, DevInfoID, pDevInfoStruct, DevInfoStructSize);
}

HRESULT STDMETHODCALLTYPE d3d8dev_SetPaletteEntries(TAKPIDirect3DDevice8* this, UINT PaletteNumber, CONST PALETTEENTRY* pEntries)
{
    validate_IDirect3DDevice8(this->m_d3d8dev->lpVtbl, this->backup_vtbl);
    log("%p, %p, %s\n", this, this->m_d3d8dev, __FUNCTION__);
    return this->m_d3d8dev->lpVtbl->SetPaletteEntries(this->m_d3d8dev, PaletteNumber, pEntries);
}

HRESULT STDMETHODCALLTYPE d3d8dev_GetPaletteEntries(TAKPIDirect3DDevice8* this, UINT PaletteNumber, PALETTEENTRY* pEntries)
{
    validate_IDirect3DDevice8(this->m_d3d8dev->lpVtbl, this->backup_vtbl);
    log("%p, %p, %s\n", this, this->m_d3d8dev, __FUNCTION__);
    return this->m_d3d8dev->lpVtbl->GetPaletteEntries(this->m_d3d8dev, PaletteNumber, pEntries);
}

HRESULT STDMETHODCALLTYPE d3d8dev_SetCurrentTexturePalette(TAKPIDirect3DDevice8* this, UINT PaletteNumber)
{
    validate_IDirect3DDevice8(this->m_d3d8dev->lpVtbl, this->backup_vtbl);
    log("%p, %p, %s\n", this, this->m_d3d8dev, __FUNCTION__);
    return this->m_d3d8dev->lpVtbl->SetCurrentTexturePalette(this->m_d3d8dev, PaletteNumber);
}

HRESULT STDMETHODCALLTYPE d3d8dev_GetCurrentTexturePalette(TAKPIDirect3DDevice8* this, UINT *PaletteNumber)
{
    validate_IDirect3DDevice8(this->m_d3d8dev->lpVtbl, this->backup_vtbl);
    log("%p, %p, %s\n", this, this->m_d3d8dev, __FUNCTION__);
    return this->m_d3d8dev->lpVtbl->GetCurrentTexturePalette(this->m_d3d8dev, PaletteNumber);
}

HRESULT STDMETHODCALLTYPE d3d8dev_DrawPrimitive(TAKPIDirect3DDevice8* this, D3DPRIMITIVETYPE PrimitiveType, UINT StartVertex, UINT PrimitiveCount)
{
    validate_IDirect3DDevice8(this->m_d3d8dev->lpVtbl, this->backup_vtbl);
    //log("%p, %p, %s (%d, %d, %d)\n", this, this->m_d3d8dev, __FUNCTION__, PrimitiveType, StartVertex, PrimitiveCount);
    return this->m_d3d8dev->lpVtbl->DrawPrimitive(this->m_d3d8dev, PrimitiveType, StartVertex, PrimitiveCount);
}

HRESULT STDMETHODCALLTYPE d3d8dev_DrawIndexedPrimitive(TAKPIDirect3DDevice8* this, D3DPRIMITIVETYPE PrimitiveType, UINT MinIndex, UINT NumVertices, UINT startIndex, UINT primCount)
{
    validate_IDirect3DDevice8(this->m_d3d8dev->lpVtbl, this->backup_vtbl);
    //log("%p, %p, %s (%d, %d, %d, %d, %d)\n", this, this->m_d3d8dev, __FUNCTION__, PrimitiveType, MinIndex, NumVertices, startIndex, primCount);
    return this->m_d3d8dev->lpVtbl->DrawIndexedPrimitive(this->m_d3d8dev, PrimitiveType, MinIndex, NumVertices, startIndex, primCount);
}

HRESULT STDMETHODCALLTYPE d3d8dev_DrawPrimitiveUP(TAKPIDirect3DDevice8* this, D3DPRIMITIVETYPE PrimitiveType, UINT PrimitiveCount, CONST void* pVertexStreamZeroData, UINT VertexStreamZeroStride)
{
    validate_IDirect3DDevice8(this->m_d3d8dev->lpVtbl, this->backup_vtbl);
    //log("%p, %p, %s (%d, %d, %p, %d)\n", this, this->m_d3d8dev, __FUNCTION__, PrimitiveType, PrimitiveCount, pVertexStreamZeroData, VertexStreamZeroStride);
    return this->m_d3d8dev->lpVtbl->DrawPrimitiveUP(this->m_d3d8dev, PrimitiveType, PrimitiveCount, pVertexStreamZeroData, VertexStreamZeroStride);
}

HRESULT STDMETHODCALLTYPE d3d8dev_DrawIndexedPrimitiveUP(TAKPIDirect3DDevice8* this, D3DPRIMITIVETYPE PrimitiveType, UINT MinVertexIndex, UINT NumVertices, UINT PrimitiveCount, CONST void* pIndexData, D3DFORMAT IndexDataFormat, CONST void* pVertexStreamZeroData, UINT VertexStreamZeroStride)
{
    validate_IDirect3DDevice8(this->m_d3d8dev->lpVtbl, this->backup_vtbl);
    log("%p, %p, %s\n", this, this->m_d3d8dev, __FUNCTION__);
    return this->m_d3d8dev->lpVtbl->DrawIndexedPrimitiveUP(this->m_d3d8dev, PrimitiveType, MinVertexIndex, NumVertices, PrimitiveCount, pIndexData, IndexDataFormat, pVertexStreamZeroData, VertexStreamZeroStride);
}

HRESULT STDMETHODCALLTYPE d3d8dev_ProcessVertices(TAKPIDirect3DDevice8* this, UINT SrcStartIndex, UINT DestIndex, UINT VertexCount, IDirect3DVertexBuffer8* pDestBuffer, DWORD Flags)
{
    validate_IDirect3DDevice8(this->m_d3d8dev->lpVtbl, this->backup_vtbl);
    log("%p, %p, %s\n", this, this->m_d3d8dev, __FUNCTION__);
    return this->m_d3d8dev->lpVtbl->ProcessVertices(this->m_d3d8dev, SrcStartIndex, DestIndex, VertexCount, pDestBuffer, Flags);
}

HRESULT STDMETHODCALLTYPE d3d8dev_CreateVertexShader(TAKPIDirect3DDevice8* this, CONST DWORD* pDeclaration, CONST DWORD* pFunction, DWORD* pHandle, DWORD Usage)
{
    validate_IDirect3DDevice8(this->m_d3d8dev->lpVtbl, this->backup_vtbl);
    log("%p, %p, %s\n", this, this->m_d3d8dev, __FUNCTION__);
    return this->m_d3d8dev->lpVtbl->CreateVertexShader(this->m_d3d8dev, pDeclaration, pFunction, pHandle, Usage);
}

HRESULT STDMETHODCALLTYPE d3d8dev_SetVertexShader(TAKPIDirect3DDevice8* this, DWORD Handle)
{
    validate_IDirect3DDevice8(this->m_d3d8dev->lpVtbl, this->backup_vtbl);
    //log("%p, %p, %s (%d)\n", this, this->m_d3d8dev, __FUNCTION__, Handle);
    return this->m_d3d8dev->lpVtbl->SetVertexShader(this->m_d3d8dev, Handle);
}

HRESULT STDMETHODCALLTYPE d3d8dev_GetVertexShader(TAKPIDirect3DDevice8* this, DWORD* pHandle)
{
    validate_IDirect3DDevice8(this->m_d3d8dev->lpVtbl, this->backup_vtbl);
    log("%p, %p, %s\n", this, this->m_d3d8dev, __FUNCTION__);
    return this->m_d3d8dev->lpVtbl->GetVertexShader(this->m_d3d8dev, pHandle);
}

HRESULT STDMETHODCALLTYPE d3d8dev_DeleteVertexShader(TAKPIDirect3DDevice8* this, DWORD Handle)
{
    validate_IDirect3DDevice8(this->m_d3d8dev->lpVtbl, this->backup_vtbl);
    log("%p, %p, %s\n", this, this->m_d3d8dev, __FUNCTION__);
    return this->m_d3d8dev->lpVtbl->DeleteVertexShader(this->m_d3d8dev, Handle);
}

HRESULT STDMETHODCALLTYPE d3d8dev_SetVertexShaderConstant(TAKPIDirect3DDevice8* this, DWORD Register, CONST void* pConstantData, DWORD ConstantCount)
{
    validate_IDirect3DDevice8(this->m_d3d8dev->lpVtbl, this->backup_vtbl);
    log("%p, %p, %s\n", this, this->m_d3d8dev, __FUNCTION__);
    return this->m_d3d8dev->lpVtbl->SetVertexShaderConstant(this->m_d3d8dev, Register, pConstantData, ConstantCount);
}

HRESULT STDMETHODCALLTYPE d3d8dev_GetVertexShaderConstant(TAKPIDirect3DDevice8* this, DWORD Register, void* pConstantData, DWORD ConstantCount)
{
    validate_IDirect3DDevice8(this->m_d3d8dev->lpVtbl, this->backup_vtbl);
    log("%p, %p, %s\n", this, this->m_d3d8dev, __FUNCTION__);
    return this->m_d3d8dev->lpVtbl->GetVertexShaderConstant(this->m_d3d8dev, Register, pConstantData, ConstantCount);
}

HRESULT STDMETHODCALLTYPE d3d8dev_GetVertexShaderDeclaration(TAKPIDirect3DDevice8* this, DWORD Handle, void* pData, DWORD* pSizeOfData)
{
    validate_IDirect3DDevice8(this->m_d3d8dev->lpVtbl, this->backup_vtbl);
    log("%p, %p, %s\n", this, this->m_d3d8dev, __FUNCTION__);
    return this->m_d3d8dev->lpVtbl->GetVertexShaderDeclaration(this->m_d3d8dev, Handle, pData, pSizeOfData);
}

HRESULT STDMETHODCALLTYPE d3d8dev_GetVertexShaderFunction(TAKPIDirect3DDevice8* this, DWORD Handle, void* pData, DWORD* pSizeOfData)
{
    validate_IDirect3DDevice8(this->m_d3d8dev->lpVtbl, this->backup_vtbl);
    log("%p, %p, %s\n", this, this->m_d3d8dev, __FUNCTION__);
    return this->m_d3d8dev->lpVtbl->GetVertexShaderFunction(this->m_d3d8dev, Handle, pData, pSizeOfData);
}

HRESULT STDMETHODCALLTYPE d3d8dev_SetStreamSource(TAKPIDirect3DDevice8* this, UINT StreamNumber, IDirect3DVertexBuffer8* pStreamData, UINT Stride)
{
    validate_IDirect3DDevice8(this->m_d3d8dev->lpVtbl, this->backup_vtbl);
    //log("%p, %p, %s (%d, %p, %d)\n", this, this->m_d3d8dev, __FUNCTION__, StreamNumber, pStreamData, Stride);
    return this->m_d3d8dev->lpVtbl->SetStreamSource(this->m_d3d8dev, StreamNumber, pStreamData, Stride);
}

HRESULT STDMETHODCALLTYPE d3d8dev_GetStreamSource(TAKPIDirect3DDevice8* this, UINT StreamNumber, IDirect3DVertexBuffer8** ppStreamData, UINT* pStride)
{
    validate_IDirect3DDevice8(this->m_d3d8dev->lpVtbl, this->backup_vtbl);
    log("%p, %p, %s\n", this, this->m_d3d8dev, __FUNCTION__);
    return this->m_d3d8dev->lpVtbl->GetStreamSource(this->m_d3d8dev, StreamNumber, ppStreamData, pStride);
}

HRESULT STDMETHODCALLTYPE d3d8dev_SetIndices(TAKPIDirect3DDevice8* this, IDirect3DIndexBuffer8* pIndexData, UINT BaseVertexIndex)
{
    validate_IDirect3DDevice8(this->m_d3d8dev->lpVtbl, this->backup_vtbl);
    //log("%p, %p, %s (%p, %d)\n", this, this->m_d3d8dev, __FUNCTION__, pIndexData, BaseVertexIndex);
    return this->m_d3d8dev->lpVtbl->SetIndices(this->m_d3d8dev, pIndexData, BaseVertexIndex);
}

HRESULT STDMETHODCALLTYPE d3d8dev_GetIndices(TAKPIDirect3DDevice8* this, IDirect3DIndexBuffer8** ppIndexData, UINT* pBaseVertexIndex)
{
    validate_IDirect3DDevice8(this->m_d3d8dev->lpVtbl, this->backup_vtbl);
    log("%p, %p, %s\n", this, this->m_d3d8dev, __FUNCTION__);
    return this->m_d3d8dev->lpVtbl->GetIndices(this->m_d3d8dev, ppIndexData, pBaseVertexIndex);
}

HRESULT STDMETHODCALLTYPE d3d8dev_CreatePixelShader(TAKPIDirect3DDevice8* this, CONST DWORD* pFunction, DWORD* pHandle)
{
    validate_IDirect3DDevice8(this->m_d3d8dev->lpVtbl, this->backup_vtbl);
    log("%p, %p, %s\n", this, this->m_d3d8dev, __FUNCTION__);
    return this->m_d3d8dev->lpVtbl->CreatePixelShader(this->m_d3d8dev, pFunction, pHandle);
}

HRESULT STDMETHODCALLTYPE d3d8dev_SetPixelShader(TAKPIDirect3DDevice8* this, DWORD Handle)
{
    validate_IDirect3DDevice8(this->m_d3d8dev->lpVtbl, this->backup_vtbl);
    log("%p, %p, %s\n", this, this->m_d3d8dev, __FUNCTION__);
    return this->m_d3d8dev->lpVtbl->SetPixelShader(this->m_d3d8dev, Handle);
}

HRESULT STDMETHODCALLTYPE d3d8dev_GetPixelShader(TAKPIDirect3DDevice8* this, DWORD* pHandle)
{
    validate_IDirect3DDevice8(this->m_d3d8dev->lpVtbl, this->backup_vtbl);
    log("%p, %p, %s\n", this, this->m_d3d8dev, __FUNCTION__);
    return this->m_d3d8dev->lpVtbl->GetPixelShader(this->m_d3d8dev, pHandle);
}

HRESULT STDMETHODCALLTYPE d3d8dev_DeletePixelShader(TAKPIDirect3DDevice8* this, DWORD Handle)
{
    validate_IDirect3DDevice8(this->m_d3d8dev->lpVtbl, this->backup_vtbl);
    log("%p, %p, %s\n", this, this->m_d3d8dev, __FUNCTION__);
    return this->m_d3d8dev->lpVtbl->DeletePixelShader(this->m_d3d8dev, Handle);
}

HRESULT STDMETHODCALLTYPE d3d8dev_SetPixelShaderConstant(TAKPIDirect3DDevice8* this, DWORD Register, CONST void* pConstantData, DWORD ConstantCount)
{
    validate_IDirect3DDevice8(this->m_d3d8dev->lpVtbl, this->backup_vtbl);
    log("%p, %p, %s\n", this, this->m_d3d8dev, __FUNCTION__);
    return this->m_d3d8dev->lpVtbl->SetPixelShaderConstant(this->m_d3d8dev, Register, pConstantData, ConstantCount);
}

HRESULT STDMETHODCALLTYPE d3d8dev_GetPixelShaderConstant(TAKPIDirect3DDevice8* this, DWORD Register, void* pConstantData, DWORD ConstantCount)
{
    validate_IDirect3DDevice8(this->m_d3d8dev->lpVtbl, this->backup_vtbl);
    log("%p, %p, %s\n", this, this->m_d3d8dev, __FUNCTION__);
    return this->m_d3d8dev->lpVtbl->GetPixelShaderConstant(this->m_d3d8dev, Register, pConstantData, ConstantCount);
}

HRESULT STDMETHODCALLTYPE d3d8dev_GetPixelShaderFunction(TAKPIDirect3DDevice8* this, DWORD Handle, void* pData, DWORD* pSizeOfData)
{
    validate_IDirect3DDevice8(this->m_d3d8dev->lpVtbl, this->backup_vtbl);
    log("%p, %p, %s\n", this, this->m_d3d8dev, __FUNCTION__);
    return this->m_d3d8dev->lpVtbl->GetPixelShaderFunction(this->m_d3d8dev, Handle, pData, pSizeOfData);
}

HRESULT STDMETHODCALLTYPE d3d8dev_DrawRectPatch(TAKPIDirect3DDevice8* this, UINT Handle, CONST float* pNumSegs, CONST D3DRECTPATCH_INFO* pRectPatchInfo)
{
    validate_IDirect3DDevice8(this->m_d3d8dev->lpVtbl, this->backup_vtbl);
    log("%p, %p, %s\n", this, this->m_d3d8dev, __FUNCTION__);
    return this->m_d3d8dev->lpVtbl->DrawRectPatch(this->m_d3d8dev, Handle, pNumSegs, pRectPatchInfo);
}

HRESULT STDMETHODCALLTYPE d3d8dev_DrawTriPatch(TAKPIDirect3DDevice8* this, UINT Handle, CONST float* pNumSegs, CONST D3DTRIPATCH_INFO* pTriPatchInfo)
{
    validate_IDirect3DDevice8(this->m_d3d8dev->lpVtbl, this->backup_vtbl);
    log("%p, %p, %s\n", this, this->m_d3d8dev, __FUNCTION__);
    return this->m_d3d8dev->lpVtbl->DrawTriPatch(this->m_d3d8dev, Handle, pNumSegs, pTriPatchInfo);
}

HRESULT STDMETHODCALLTYPE d3d8dev_DeletePatch(TAKPIDirect3DDevice8* this, UINT Handle)
{
    validate_IDirect3DDevice8(this->m_d3d8dev->lpVtbl, this->backup_vtbl);
    log("%p, %p, %s\n", this, this->m_d3d8dev, __FUNCTION__);
    return this->m_d3d8dev->lpVtbl->DeletePatch(this->m_d3d8dev, Handle);
}


#define d3d8dev_entry(vtbl, func) vtbl->func = (__typeof__(vtbl->func))d3d8dev_##func
IDirect3DDevice8* new_TAKPIDirect3DDevice8(TAKPIDirect3D8* d3d8, IDirect3DDevice8* d3d8dev)
{
    TAKPIDirect3DDevice8* takpd3d8dev = (TAKPIDirect3DDevice8*)malloc(sizeof(TAKPIDirect3DDevice8));
    takpd3d8dev->lpVtbl = (IDirect3DDevice8Vtbl*)malloc(sizeof(IDirect3DDevice8Vtbl));
    takpd3d8dev->backup_vtbl = (IDirect3DDevice8Vtbl*)malloc(sizeof(IDirect3DDevice8Vtbl));
    takpd3d8dev->m_d3d8 = d3d8;
    takpd3d8dev->m_d3d8dev = d3d8dev;

    d3d8dev_entry(takpd3d8dev->lpVtbl, QueryInterface);
    d3d8dev_entry(takpd3d8dev->lpVtbl, AddRef);
    d3d8dev_entry(takpd3d8dev->lpVtbl, Release);
    d3d8dev_entry(takpd3d8dev->lpVtbl, TestCooperativeLevel);
    d3d8dev_entry(takpd3d8dev->lpVtbl, GetAvailableTextureMem);
    d3d8dev_entry(takpd3d8dev->lpVtbl, ResourceManagerDiscardBytes);
    d3d8dev_entry(takpd3d8dev->lpVtbl, GetDirect3D);
    d3d8dev_entry(takpd3d8dev->lpVtbl, GetDeviceCaps);
    d3d8dev_entry(takpd3d8dev->lpVtbl, GetDisplayMode);
    d3d8dev_entry(takpd3d8dev->lpVtbl, GetCreationParameters);
    d3d8dev_entry(takpd3d8dev->lpVtbl, SetCursorProperties);
    d3d8dev_entry(takpd3d8dev->lpVtbl, SetCursorPosition);
    d3d8dev_entry(takpd3d8dev->lpVtbl, ShowCursor);
    d3d8dev_entry(takpd3d8dev->lpVtbl, CreateAdditionalSwapChain);
    d3d8dev_entry(takpd3d8dev->lpVtbl, Reset);
    d3d8dev_entry(takpd3d8dev->lpVtbl, Present);
    d3d8dev_entry(takpd3d8dev->lpVtbl, GetBackBuffer);
    d3d8dev_entry(takpd3d8dev->lpVtbl, GetRasterStatus);
    d3d8dev_entry(takpd3d8dev->lpVtbl,  SetGammaRamp);
    d3d8dev_entry(takpd3d8dev->lpVtbl, GetGammaRamp);
    d3d8dev_entry(takpd3d8dev->lpVtbl, CreateTexture);
    d3d8dev_entry(takpd3d8dev->lpVtbl, CreateVolumeTexture);
    d3d8dev_entry(takpd3d8dev->lpVtbl, CreateCubeTexture);
    d3d8dev_entry(takpd3d8dev->lpVtbl, CreateVertexBuffer);
    d3d8dev_entry(takpd3d8dev->lpVtbl, CreateIndexBuffer);
    d3d8dev_entry(takpd3d8dev->lpVtbl, CreateRenderTarget);
    d3d8dev_entry(takpd3d8dev->lpVtbl, CreateDepthStencilSurface);
    d3d8dev_entry(takpd3d8dev->lpVtbl, CreateImageSurface);
    d3d8dev_entry(takpd3d8dev->lpVtbl, CopyRects);
    d3d8dev_entry(takpd3d8dev->lpVtbl, UpdateTexture);
    d3d8dev_entry(takpd3d8dev->lpVtbl, GetFrontBuffer);
    d3d8dev_entry(takpd3d8dev->lpVtbl, SetRenderTarget);
    d3d8dev_entry(takpd3d8dev->lpVtbl, GetRenderTarget);
    d3d8dev_entry(takpd3d8dev->lpVtbl, GetDepthStencilSurface);
    d3d8dev_entry(takpd3d8dev->lpVtbl, BeginScene);
    d3d8dev_entry(takpd3d8dev->lpVtbl, EndScene);
    d3d8dev_entry(takpd3d8dev->lpVtbl, Clear);
    d3d8dev_entry(takpd3d8dev->lpVtbl, SetTransform);
    d3d8dev_entry(takpd3d8dev->lpVtbl, GetTransform);
    d3d8dev_entry(takpd3d8dev->lpVtbl, MultiplyTransform);
    d3d8dev_entry(takpd3d8dev->lpVtbl, SetViewport);
    d3d8dev_entry(takpd3d8dev->lpVtbl, GetViewport);
    d3d8dev_entry(takpd3d8dev->lpVtbl, SetMaterial);
    d3d8dev_entry(takpd3d8dev->lpVtbl, GetMaterial);
    d3d8dev_entry(takpd3d8dev->lpVtbl, SetLight);
    d3d8dev_entry(takpd3d8dev->lpVtbl, GetLight);
    d3d8dev_entry(takpd3d8dev->lpVtbl, LightEnable);
    d3d8dev_entry(takpd3d8dev->lpVtbl, GetLightEnable);
    d3d8dev_entry(takpd3d8dev->lpVtbl, SetClipPlane);
    d3d8dev_entry(takpd3d8dev->lpVtbl, GetClipPlane);
    d3d8dev_entry(takpd3d8dev->lpVtbl, SetRenderState);
    d3d8dev_entry(takpd3d8dev->lpVtbl, GetRenderState);
    d3d8dev_entry(takpd3d8dev->lpVtbl, BeginStateBlock);
    d3d8dev_entry(takpd3d8dev->lpVtbl, EndStateBlock);
    d3d8dev_entry(takpd3d8dev->lpVtbl, ApplyStateBlock);
    d3d8dev_entry(takpd3d8dev->lpVtbl, CaptureStateBlock);
    d3d8dev_entry(takpd3d8dev->lpVtbl, DeleteStateBlock);
    d3d8dev_entry(takpd3d8dev->lpVtbl, CreateStateBlock);
    d3d8dev_entry(takpd3d8dev->lpVtbl, SetClipStatus);
    d3d8dev_entry(takpd3d8dev->lpVtbl, GetClipStatus);
    d3d8dev_entry(takpd3d8dev->lpVtbl, GetTexture);
    d3d8dev_entry(takpd3d8dev->lpVtbl, SetTexture);
    d3d8dev_entry(takpd3d8dev->lpVtbl, GetTextureStageState);
    d3d8dev_entry(takpd3d8dev->lpVtbl, SetTextureStageState);
    d3d8dev_entry(takpd3d8dev->lpVtbl, ValidateDevice);
    d3d8dev_entry(takpd3d8dev->lpVtbl, GetInfo);
    d3d8dev_entry(takpd3d8dev->lpVtbl, SetPaletteEntries);
    d3d8dev_entry(takpd3d8dev->lpVtbl, GetPaletteEntries);
    d3d8dev_entry(takpd3d8dev->lpVtbl, SetCurrentTexturePalette);
    d3d8dev_entry(takpd3d8dev->lpVtbl, GetCurrentTexturePalette);
    d3d8dev_entry(takpd3d8dev->lpVtbl, DrawPrimitive);
    d3d8dev_entry(takpd3d8dev->lpVtbl, DrawIndexedPrimitive);
    d3d8dev_entry(takpd3d8dev->lpVtbl, DrawPrimitiveUP);
    d3d8dev_entry(takpd3d8dev->lpVtbl, DrawIndexedPrimitiveUP);
    d3d8dev_entry(takpd3d8dev->lpVtbl, ProcessVertices);
    d3d8dev_entry(takpd3d8dev->lpVtbl, CreateVertexShader);
    d3d8dev_entry(takpd3d8dev->lpVtbl, SetVertexShader);
    d3d8dev_entry(takpd3d8dev->lpVtbl, GetVertexShader);
    d3d8dev_entry(takpd3d8dev->lpVtbl, DeleteVertexShader);
    d3d8dev_entry(takpd3d8dev->lpVtbl, SetVertexShaderConstant);
    d3d8dev_entry(takpd3d8dev->lpVtbl, GetVertexShaderConstant);
    d3d8dev_entry(takpd3d8dev->lpVtbl, GetVertexShaderDeclaration);
    d3d8dev_entry(takpd3d8dev->lpVtbl, GetVertexShaderFunction);
    d3d8dev_entry(takpd3d8dev->lpVtbl, SetStreamSource);
    d3d8dev_entry(takpd3d8dev->lpVtbl, GetStreamSource);
    d3d8dev_entry(takpd3d8dev->lpVtbl, SetIndices);
    d3d8dev_entry(takpd3d8dev->lpVtbl, GetIndices);
    d3d8dev_entry(takpd3d8dev->lpVtbl, CreatePixelShader);
    d3d8dev_entry(takpd3d8dev->lpVtbl, SetPixelShader);
    d3d8dev_entry(takpd3d8dev->lpVtbl, GetPixelShader);
    d3d8dev_entry(takpd3d8dev->lpVtbl, DeletePixelShader);
    d3d8dev_entry(takpd3d8dev->lpVtbl, SetPixelShaderConstant);
    d3d8dev_entry(takpd3d8dev->lpVtbl, GetPixelShaderConstant);
    d3d8dev_entry(takpd3d8dev->lpVtbl, GetPixelShaderFunction);
    d3d8dev_entry(takpd3d8dev->lpVtbl, DrawRectPatch);
    d3d8dev_entry(takpd3d8dev->lpVtbl, DrawTriPatch);
    d3d8dev_entry(takpd3d8dev->lpVtbl, DeletePatch);

    *(takpd3d8dev->backup_vtbl) = *(takpd3d8dev->m_d3d8dev->lpVtbl);
    return (IDirect3DDevice8*)takpd3d8dev;
}

// IUnknown
HRESULT STDMETHODCALLTYPE d3d8_QueryInterface(TAKPIDirect3D8* this, REFIID riid, void** ppvObj)
{
    validate_IDirect3D8(this->m_d3d8->lpVtbl, this->backup_vtbl);
    log("%p, %p, %s\n", this, this->m_d3d8, __FUNCTION__);
    return this->m_d3d8->lpVtbl->QueryInterface(this->m_d3d8, riid, ppvObj);
}

ULONG STDMETHODCALLTYPE d3d8_AddRef(TAKPIDirect3D8* this)
{
    validate_IDirect3D8(this->m_d3d8->lpVtbl, this->backup_vtbl);
    log("%p, %p, %s\n", this, this->m_d3d8, __FUNCTION__);
    return this->m_d3d8->lpVtbl->AddRef(this->m_d3d8);
}

ULONG STDMETHODCALLTYPE d3d8_Release(TAKPIDirect3D8* this)
{
    validate_IDirect3D8(this->m_d3d8->lpVtbl, this->backup_vtbl);
    log("%p, %p, %s\n", this, this->m_d3d8, __FUNCTION__);
    ULONG result = this->m_d3d8->lpVtbl->Release(this->m_d3d8);
    if (result == 0)
    {
        log("We're done with this! %p %d\n", this, result);
        free(this->lpVtbl);
        free(this->backup_vtbl);
        free(this);
    }
    return result;
}

// IDirect3D8
HRESULT STDMETHODCALLTYPE d3d8_RegisterSoftwareDevice(TAKPIDirect3D8* this, void* pInitializeFunction)
{
    validate_IDirect3D8(this->m_d3d8->lpVtbl, this->backup_vtbl);
    log("%p, %p, %s\n", this, this->m_d3d8, __FUNCTION__);
    return this->m_d3d8->lpVtbl->RegisterSoftwareDevice(this->m_d3d8, pInitializeFunction);
}

UINT STDMETHODCALLTYPE d3d8_GetAdapterCount(TAKPIDirect3D8* this)
{
    validate_IDirect3D8(this->m_d3d8->lpVtbl, this->backup_vtbl);
    log("%p, %p, %s ()\n", this, this->m_d3d8, __FUNCTION__);
    return this->m_d3d8->lpVtbl->GetAdapterCount(this->m_d3d8);
}

HRESULT STDMETHODCALLTYPE d3d8_GetAdapterIdentifier(TAKPIDirect3D8* this, UINT Adapter, DWORD Flags, D3DADAPTER_IDENTIFIER8* pIdentifier)
{
    validate_IDirect3D8(this->m_d3d8->lpVtbl, this->backup_vtbl);
    log("%p, %p, %s (%d, %d, %p)\n", this, this->m_d3d8, __FUNCTION__, Adapter, Flags, pIdentifier);
    return this->m_d3d8->lpVtbl->GetAdapterIdentifier(this->m_d3d8, Adapter, Flags, pIdentifier);
}

UINT STDMETHODCALLTYPE d3d8_GetAdapterModeCount(TAKPIDirect3D8* this, UINT Adapter)
{
    validate_IDirect3D8(this->m_d3d8->lpVtbl, this->backup_vtbl);
    log("%p, %p, %s (%d)\n", this, this->m_d3d8, __FUNCTION__, Adapter);
    return this->m_d3d8->lpVtbl->GetAdapterModeCount(this->m_d3d8, Adapter);
}

HRESULT STDMETHODCALLTYPE d3d8_EnumAdapterModes(TAKPIDirect3D8* this, UINT Adapter, UINT Mode, D3DDISPLAYMODE* pMode)
{
    validate_IDirect3D8(this->m_d3d8->lpVtbl, this->backup_vtbl);
    //log("%p, %p, %s (%d, %d, %p)\n", this, this->m_d3d8, __FUNCTION__);
    return this->m_d3d8->lpVtbl->EnumAdapterModes(this->m_d3d8, Adapter, Mode, pMode);
}

HRESULT STDMETHODCALLTYPE d3d8_GetAdapterDisplayMode(TAKPIDirect3D8* this, UINT Adapter, D3DDISPLAYMODE* pMode)
{
    validate_IDirect3D8(this->m_d3d8->lpVtbl, this->backup_vtbl);
    log("%p, %p, %s (%d, %p)\n", this, this->m_d3d8, __FUNCTION__, Adapter, pMode);
    return this->m_d3d8->lpVtbl->GetAdapterDisplayMode(this->m_d3d8, Adapter, pMode);
}

HRESULT STDMETHODCALLTYPE d3d8_CheckDeviceType(TAKPIDirect3D8* this, UINT Adapter, D3DDEVTYPE CheckType, D3DFORMAT DisplayFormat, D3DFORMAT BackBufferFormat, BOOL Windowed)
{
    validate_IDirect3D8(this->m_d3d8->lpVtbl, this->backup_vtbl);
    log("%p, %p, %s (%d, %d, %d, %d, %d)\n", this, this->m_d3d8, __FUNCTION__, Adapter, CheckType, DisplayFormat, BackBufferFormat, Windowed);
    return this->m_d3d8->lpVtbl->CheckDeviceType(this->m_d3d8, Adapter, CheckType, DisplayFormat, BackBufferFormat, Windowed);
}

HRESULT STDMETHODCALLTYPE d3d8_CheckDeviceFormat(TAKPIDirect3D8* this, UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT AdapterFormat, DWORD Usage, D3DRESOURCETYPE RType, D3DFORMAT CheckFormat)
{
    validate_IDirect3D8(this->m_d3d8->lpVtbl, this->backup_vtbl);
    //log("%p, %p, %s (%u, %d, %d, %d, %d, %d)\n", this, this->m_d3d8, __FUNCTION__, Adapter, DeviceType, AdapterFormat, Usage, RType, CheckFormat);
    return this->m_d3d8->lpVtbl->CheckDeviceFormat(this->m_d3d8, Adapter, DeviceType, AdapterFormat, Usage, RType, CheckFormat);
}

HRESULT STDMETHODCALLTYPE d3d8_CheckDeviceMultiSampleType(TAKPIDirect3D8* this, UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT SurfaceFormat, BOOL Windowed, D3DMULTISAMPLE_TYPE MultiSampleType)
{
    validate_IDirect3D8(this->m_d3d8->lpVtbl, this->backup_vtbl);
    log("%p, %p, %s\n", this, this->m_d3d8, __FUNCTION__);
    return this->m_d3d8->lpVtbl->CheckDeviceMultiSampleType(this->m_d3d8, Adapter, DeviceType, SurfaceFormat, Windowed, MultiSampleType);
}

HRESULT STDMETHODCALLTYPE d3d8_CheckDepthStencilMatch(TAKPIDirect3D8* this, UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT AdapterFormat, D3DFORMAT RenderTargetFormat, D3DFORMAT DepthStencilFormat)
{
    validate_IDirect3D8(this->m_d3d8->lpVtbl, this->backup_vtbl);
    log("%p, %p, %s\n", this, this->m_d3d8, __FUNCTION__);
    return this->m_d3d8->lpVtbl->CheckDepthStencilMatch(this->m_d3d8, Adapter, DeviceType, AdapterFormat, RenderTargetFormat, DepthStencilFormat);
}

HRESULT STDMETHODCALLTYPE d3d8_GetDeviceCaps(TAKPIDirect3D8* this, UINT Adapter, D3DDEVTYPE DeviceType, D3DCAPS8* pCaps)
{
    validate_IDirect3D8(this->m_d3d8->lpVtbl, this->backup_vtbl);
    log("%p, %p, %s (%d, %d, %p)\n", this, this->m_d3d8, __FUNCTION__, Adapter, DeviceType, pCaps);
    return this->m_d3d8->lpVtbl->GetDeviceCaps(this->m_d3d8, Adapter, DeviceType, pCaps);
}

HMONITOR STDMETHODCALLTYPE d3d8_GetAdapterMonitor(TAKPIDirect3D8* this, UINT Adapter)
{
    validate_IDirect3D8(this->m_d3d8->lpVtbl, this->backup_vtbl);
    log("%p, %p, %s\n", this, this->m_d3d8, __FUNCTION__);
    return this->m_d3d8->lpVtbl->GetAdapterMonitor(this->m_d3d8, Adapter);
}

HRESULT STDMETHODCALLTYPE d3d8_CreateDevice(TAKPIDirect3D8* this, UINT Adapter, D3DDEVTYPE DeviceType, HWND hFocusWindow, DWORD BehaviorFlags, D3DPRESENT_PARAMETERS* pPresentationParameters, IDirect3DDevice8** ppReturnedDeviceInterface)
{
    validate_IDirect3D8(this->m_d3d8->lpVtbl, this->backup_vtbl);
    log("%p, %p, %s (%d, %d, %p, %d, %p, %p)\n", this, this->m_d3d8, __FUNCTION__, Adapter, DeviceType, hFocusWindow, BehaviorFlags, pPresentationParameters, ppReturnedDeviceInterface);
    log("    PresentationParameters: %u %u %d %u %d %d %p (%d) %d %d %d (%d %d)\n",
        pPresentationParameters->BackBufferWidth, pPresentationParameters->BackBufferHeight, pPresentationParameters->BackBufferFormat, pPresentationParameters->BackBufferCount,
        pPresentationParameters->MultiSampleType, pPresentationParameters->SwapEffect, pPresentationParameters->hDeviceWindow, pPresentationParameters->Windowed,
        pPresentationParameters->EnableAutoDepthStencil, pPresentationParameters->AutoDepthStencilFormat, pPresentationParameters->Flags,
        pPresentationParameters->FullScreen_RefreshRateInHz, pPresentationParameters->FullScreen_PresentationInterval);
    IDirect3DDevice8* newDevice;
    HRESULT result = this->m_d3d8->lpVtbl->CreateDevice(this->m_d3d8, Adapter, DeviceType, hFocusWindow, BehaviorFlags, pPresentationParameters, &newDevice);
#if 1
    *ppReturnedDeviceInterface = new_TAKPIDirect3DDevice8(this, newDevice);
#else
    *ppReturnedDeviceInterface = newDevice;
#endif
    log("    %p, %p, %s (...) = %p\n", this, this->m_d3d8, __FUNCTION__, *ppReturnedDeviceInterface);

    return result;
}

#define d3d8_entry(vtbl, func) vtbl->func = (__typeof__(vtbl->func))d3d8_##func
IDirect3D8* new_TAKPIDirect3D8(IDirect3D8* d3d8)
{
    TAKPIDirect3D8* takpd3d8 = (TAKPIDirect3D8*)malloc(sizeof(TAKPIDirect3D8));
    takpd3d8->lpVtbl = (IDirect3D8Vtbl*)malloc(sizeof(IDirect3D8Vtbl));
    takpd3d8->backup_vtbl = (IDirect3D8Vtbl*)malloc(sizeof(IDirect3D8Vtbl));
    takpd3d8->m_d3d8 = d3d8;

    d3d8_entry(takpd3d8->lpVtbl, QueryInterface);
    d3d8_entry(takpd3d8->lpVtbl, AddRef);
    d3d8_entry(takpd3d8->lpVtbl, Release);
    d3d8_entry(takpd3d8->lpVtbl, RegisterSoftwareDevice);
    d3d8_entry(takpd3d8->lpVtbl, GetAdapterCount             );
    d3d8_entry(takpd3d8->lpVtbl, GetAdapterIdentifier);
    d3d8_entry(takpd3d8->lpVtbl, GetAdapterModeCount);
    d3d8_entry(takpd3d8->lpVtbl, EnumAdapterModes);
    d3d8_entry(takpd3d8->lpVtbl, GetAdapterDisplayMode);
    d3d8_entry(takpd3d8->lpVtbl, CheckDeviceType);
    d3d8_entry(takpd3d8->lpVtbl, CheckDeviceFormat);
    d3d8_entry(takpd3d8->lpVtbl, CheckDeviceMultiSampleType);
    d3d8_entry(takpd3d8->lpVtbl, CheckDepthStencilMatch);
    d3d8_entry(takpd3d8->lpVtbl, GetDeviceCaps);
    d3d8_entry(takpd3d8->lpVtbl, GetAdapterMonitor);
    d3d8_entry(takpd3d8->lpVtbl, CreateDevice);

    *(takpd3d8->backup_vtbl) = *(takpd3d8->m_d3d8->lpVtbl);

    return (IDirect3D8*)takpd3d8;
}

// Intercept the one function call
typedef IDirect3D8* (*Direct3DCreate_fn)(UINT);

IDirect3D8* WINAPI Direct3DCreate8(UINT SDKVersion)
{
    log("Direct3DCreate8 %d\n", SDKVersion);
    Direct3DCreate_fn orig = (Direct3DCreate_fn)GetProcAddress(g_d3d8, "Direct3DCreate8");
    return new_TAKPIDirect3D8(orig(SDKVersion));
}