#pragma once
#include <d3d8.h>

#ifdef __cplusplus
extern "C" {
#endif

IDirect3D8* WINAPI Direct3DCreate8(UINT SDKVersion);

#ifdef __cplusplus
}
#endif