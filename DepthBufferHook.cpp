#include <windows.h>

#include "DepthBufferHook.h"
#include "obse_common/SafeWrite.h"
#include "nodes\NiDX9Renderer.h"
#include "Rendering.h"
#include "EffectManager.h"

#include "D3D9.hpp"
#include "D3D9Device.hpp"

static global<bool> UseDepthBuffer(true, NULL, "DepthBuffer", "bUseDepthBuffer");
static global<bool> UseRAWZfix(true, NULL, "DepthBuffer", "bUseRAWZfix");

static IDirect3DTexture9 *pDepthTexture = NULL;
static IDirect3DSurface9 *pDepthSurface = NULL;
static IDirect3DSurface9 *pOldSurface = NULL;
static bool HasDepthVar;
static bool DoesRESZflag = false;
static bool IsRAWZflag = false;

struct DFLIST {
  D3DFORMAT	FourCC;
  char		*Name;
} DepthList[4] = {
  (D3DFORMAT)MAKEFOURCC('I','N','T','Z'), "INTZ",
  (D3DFORMAT)MAKEFOURCC('D','F','2','4'), "DF24",
  (D3DFORMAT)MAKEFOURCC('D','F','1','6'), "DF16",
  (D3DFORMAT)MAKEFOURCC('R','A','W','Z'), "RAWZ"
};

#define RAWZINDEX 3

bool v1_2_416::NiDX9ImplicitDepthStencilBufferDataEx::GetBufferDataHook(IDirect3DDevice9 *D3DDevice) {
  HRESULT hr;
  UInt32 Width, Height;

  _MESSAGE("Re-attaching depth buffer texture.");

  DoesRESZflag = false;
  IsRAWZflag = false;

  Width = v1_2_416::GetRenderer()->SizeWidth;
  Height = v1_2_416::GetRenderer()->SizeHeight;

  if (UseDepthBuffer.data) {
    D3DDevice->GetDepthStencilSurface(&pOldSurface);
    HasDepthVar = true;

    IDirect3D9	*pD3D;
    D3DDISPLAYMODE d3ddm;
    D3DDevice->GetDirect3D(&pD3D);
    pD3D->GetAdapterDisplayMode( D3DADAPTER_DEFAULT, &d3ddm );

    hr = pD3D->CheckDeviceFormat(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, d3ddm.Format, D3DUSAGE_RENDERTARGET, D3DRTYPE_SURFACE, (D3DFORMAT)MAKEFOURCC('R', 'E', 'S', 'Z'));

    if (hr == D3D_OK)
      _MESSAGE("RESZ format supported.");
    else
      _MESSAGE("RESZ not supported.");

    DoesRESZflag = (hr == D3D_OK);

    int DepthCount;
    for (DepthCount = 0; DepthCount < 4; DepthCount++) {
      hr = pD3D->CheckDeviceFormat(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, d3ddm.Format, D3DUSAGE_DEPTHSTENCIL, D3DRTYPE_SURFACE, DepthList[DepthCount].FourCC);

      if (hr == D3D_OK) {
        D3DDevice->CreateTexture(Width, Height , 1, D3DUSAGE_DEPTHSTENCIL, DepthList[DepthCount].FourCC, D3DPOOL_DEFAULT, &pDepthTexture, NULL);
        _MESSAGE("Depth buffer texture (%s) (%i,%i) created OK.", DepthList[DepthCount].Name, Width, Height);

        // Retrieve depth buffer surface from texture interface
        pDepthTexture->GetSurfaceLevel(0, &pDepthSurface);

        // Bind depth buffer
        hr = D3DDevice->SetDepthStencilSurface(pDepthSurface);

        if (hr == D3D_OK) {
          _MESSAGE("Depth buffer attached OK. %i", DepthCount);

#ifndef	OBGE_NOSHADER
	  /* register in the device-tracker because apparently
	   * here the surface-pointer is NOT constant
	   */
	  if (!surfaceTexture[pDepthSurface]) {
	    struct textureSurface *track = new struct textureSurface;

	    track->Level = 0;
	    track->map = textureMaps[pDepthTexture];
	    track->tex = pDepthTexture;

	    surfaceTexture[pDepthSurface] = track;
	  }
#endif

          if (UseRAWZfix.Get() && (DepthCount == RAWZINDEX)) {
            _MESSAGE("Setting IsRAWZflag.");
            IsRAWZflag = true;
          }

	  /* failed, turn it off */
	  if (!EffectManager::GetSingleton()->SetRAWZ(IsRAWZflag))
	    UseRAWZfix.Set(false);

          break;
        }
        else {
          _MESSAGE("Failed to attach depth buffer.");
          pDepthSurface->Release();
          pDepthTexture->Release();
        }
      }
      else {
        _MESSAGE("Failed to create buffer texture (%s).", DepthList[DepthCount].Name);
      }
    }

    if (DepthCount == 4) {
      _MESSAGE("Failed in creating a readable depth buffer. Will use Oblivion's default buffer.");
      D3DDevice->SetDepthStencilSurface(pOldSurface);
      HasDepthVar = false;
    }

    pD3D->Release();
    pOldSurface->Release();
  }
  else {
    _MESSAGE("Readable depth buffer disabled in INI file.");
    HasDepthVar = false;
  }

  return jGetBufferData(D3DDevice);
}

#pragma optimize ("",off)
bool _declspec(naked) v1_2_416::NiDX9ImplicitDepthStencilBufferDataEx::jGetBufferData(IDirect3DDevice9 *D3DDevice) {
  _asm {
    mov eax, 0x0076D530
    jmp eax
  }
}
#pragma optimize ("",on)

#pragma optimize ("",off)
void static _cdecl DepthBufferHook(IDirect3DDevice9 *Device, UInt32 u2) {
  /* NiDX9ImplicitDepthStencilBufferData_Constructor */
  void (_cdecl * Hook)(IDirect3DDevice9 *, UInt32) = (void( *)(IDirect3DDevice9 *, UInt32))0x0076E0B0;

  // Give me enough time to attach to process for debugging. Vista seems to block JIT debugging.
  //Sleep(10000);

  _MESSAGE("Pre Hook");

  HRESULT hr;
  UInt32 Width, Height;

  Width = v1_2_416::GetRenderer()->SizeWidth;
  Height = v1_2_416::GetRenderer()->SizeHeight;

  if (UseDepthBuffer.data) {
    Device->GetDepthStencilSurface(&pOldSurface);
    HasDepthVar = true;

    IDirect3D9 *pD3D;
    D3DDISPLAYMODE d3ddm;
    Device->GetDirect3D(&pD3D);
    pD3D->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &d3ddm);

    hr = pD3D->CheckDeviceFormat(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, d3ddm.Format, D3DUSAGE_RENDERTARGET, D3DRTYPE_SURFACE, (D3DFORMAT)MAKEFOURCC('R', 'E', 'S', 'Z'));

    if (hr == D3D_OK)
      _MESSAGE("RESZ format supported.");
    else
      _MESSAGE("RESZ not supported.");

    int DepthCount;

    for (DepthCount = 0; DepthCount < 4; DepthCount++) {
      hr = pD3D->CheckDeviceFormat(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, d3ddm.Format, D3DUSAGE_DEPTHSTENCIL, D3DRTYPE_SURFACE, DepthList[DepthCount].FourCC);

      if (hr == D3D_OK) {
        Device->CreateTexture(Width, Height, 1, D3DUSAGE_DEPTHSTENCIL, DepthList[DepthCount].FourCC, D3DPOOL_DEFAULT, &pDepthTexture, NULL);
        _MESSAGE("Depth buffer texture (%s) (%i,%i) created OK.", DepthList[DepthCount].Name, Width, Height);

        // Retrieve depth buffer surface from texture interface
        pDepthTexture->GetSurfaceLevel(0, &pDepthSurface);

        // Bind depth buffer
        hr = Device->SetDepthStencilSurface(pDepthSurface);

        if (hr == D3D_OK) {
          _MESSAGE("Depth buffer attached OK. %i", DepthCount);

#ifndef	OBGE_NOSHADER
	  /* register in the device-tracker because apparently
	   * here the surface-pointer is NOT constant
	   */
	  if (!surfaceTexture[pDepthSurface]) {
	    struct textureSurface *track = new struct textureSurface;

	    track->Level = 0;
	    track->map = textureMaps[pDepthTexture];
	    track->tex = pDepthTexture;

	    surfaceTexture[pDepthSurface] = track;
	  }
#endif

          if (UseRAWZfix.data && (DepthCount == RAWZINDEX)) {
            _MESSAGE("Setting IsRAWZflag.");
	    IsRAWZflag = true;
          }

	  /* failed, turn it off */
	  if (!EffectManager::GetSingleton()->SetRAWZ(IsRAWZflag))
	    UseRAWZfix.Set(false);

          break;
        }
        else {
          _MESSAGE("Failed to attach depth buffer.");
          pDepthSurface->Release();
          pDepthTexture->Release();
        }
      }
      else {
        _MESSAGE("Failed to create buffer texture (%s).", DepthList[DepthCount].Name);
      }
    }

    if (DepthCount == 4) {
      _MESSAGE("Failed in creating a readable depth buffer. Will use Oblivion's default buffer.");
      Device->SetDepthStencilSurface(pOldSurface);
      HasDepthVar = false;
    }

    pD3D->Release();
    pOldSurface->Release();
  }
  else {
    _MESSAGE("Readable depth buffer disabled in INI file.");
    HasDepthVar = false;
  }

  Hook(Device, u2);

  v1_2_416::GetRenderer()->RegisterLostDeviceCallback(LostDepthBuffer, NULL);

  //_MESSAGE("Post Hook");
  return;
};

// The game does a sanity check on the depth buffer. We need to bypass this otherwise the game will mark the depth buffer
// as incompatible and attempt to reset it.

UInt32 static _cdecl TextureSanityCheckHook(D3DFORMAT TextureFormat, UInt32 u2) {
  UInt32 (_cdecl * Hook)(D3DFORMAT , UInt32) = (UInt32 ( *)(D3DFORMAT, UInt32))0x0076C3B0;

  if (TextureFormat == (D3DFORMAT)MAKEFOURCC('I', 'N', 'T', 'Z') ||
      TextureFormat == (D3DFORMAT)MAKEFOURCC('R', 'A', 'W', 'Z'))
    TextureFormat = D3DFMT_D24S8;
  else if (TextureFormat == (D3DFORMAT)MAKEFOURCC('D', 'F', '2', '4'))
    TextureFormat = D3DFMT_D24X8;									// ATi's DF formats don't have stencil.
  else if (TextureFormat == (D3DFORMAT)MAKEFOURCC('D', 'F', '1', '6'))
    TextureFormat = D3DFMT_D16;

  UInt32 temp = Hook(TextureFormat, u2);
  return temp;
};

#pragma optimize("",on)

bool LostDepthBuffer(bool stage, void *parameters) {
  _MESSAGE("Depth buffer : Lost device callback.");

  if (stage) {
    if (pOldSurface)
      GetD3DDevice()->SetDepthStencilSurface(pOldSurface);

    if (pDepthSurface) {
      _MESSAGE("Releasing the depth buffer surface.");
      pDepthSurface->Release();
      pDepthSurface = NULL;
    }

    if (pDepthTexture) {
      _MESSAGE("Releasing the depth buffer texture.");

      while (pDepthTexture->Release()) {};

      pDepthTexture = NULL;
    }
  }
  else {
//  GetD3DDevice()->GetDepthStencilSurface(&pDepthSurface);
    HasDepthVar = false;
  }

  return true;
}

void CreateDepthBufferHook(void) {
  bool (v1_2_416::NiDX9ImplicitDepthStencilBufferDataEx::*temp)(IDirect3DDevice9 *) = &v1_2_416::NiDX9ImplicitDepthStencilBufferDataEx::GetBufferDataHook;
  UInt32 *temp2 = (UInt32 *)&temp;

  WriteRelCall(0x0076A6C4, (UInt32)&DepthBufferHook);
  WriteRelCall(0x0076C6EE, (UInt32)&TextureSanityCheckHook);
  SafeWrite32(0x00A89A24, *temp2);

  // Stops the game from clearing the z buffer before rending the player in 1st person.
  SafeWrite8(0x0040CE11, 0);

  return;
};

IDirect3DTexture9 *GetDepthBufferTexture(void) {
  return pDepthTexture;
};

bool HasDepth(void) {
  return HasDepthVar;
};

bool IsRAWZ(void) {
  return IsRAWZflag;
};

bool DoesRESZ(void) {
  return DoesRESZflag;
};
