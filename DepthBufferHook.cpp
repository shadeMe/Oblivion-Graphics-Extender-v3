/* Version: MPL 1.1/LGPL 3.0
 *
 * "The contents of this file are subject to the Mozilla Public License
 * Version 1.1 (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS"
 * basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
 * License for the specific language governing rights and limitations
 * under the License.
 *
 * The Original Code is the Oblivion Graphics Extender, short OBGE.
 *
 * The Initial Developer of the Original Code is
 * Ethatron <niels@paradice-insight.us>. Portions created by The Initial
 * Developer are Copyright (C) 2011 The Initial Developer.
 * All Rights Reserved.
 *
 * Contributor(s):
 *  Timeslip (Version 1)
 *  scanti (Version 2)
 *  IlmrynAkios (Version 3)
 *
 * Alternatively, the contents of this file may be used under the terms
 * of the GNU Library General Public License Version 3 license (the
 * "LGPL License"), in which case the provisions of LGPL License are
 * applicable instead of those above. If you wish to allow use of your
 * version of this file only under the terms of the LGPL License and not
 * to allow others to use your version of this file under the MPL,
 * indicate your decision by deleting the provisions above and replace
 * them with the notice and other provisions required by the LGPL License.
 * If you do not delete the provisions above, a recipient may use your
 * version of this file under either the MPL or the LGPL License."
 */

#include <windows.h>
#include <assert.h>

#include "DepthBufferHook.h"
#include "obse_common/SafeWrite.h"
#include "nodes\NiDX9Renderer.h"
#include "Rendering.h"
#include "EffectManager.h"
#include "ShaderManager.h"

#include "D3D9.hpp"
#include "D3D9Device.hpp"

static global<bool> UseDepthBuffer(true, NULL, "DepthBuffer", "bUseDepthBuffer");
static global<bool> UseRAWZfix(true, NULL, "DepthBuffer", "bUseRAWZfix");

static IDirect3DTexture9 *pDepthTexture = NULL;
static IDirect3DSurface9 *pDepthSurface = NULL;
static IDirect3DSurface9 *pMultiSurface = NULL;
static IDirect3DSurface9 *pOldSurface = NULL;
static D3DSURFACE_DESC DepthInfo;
static bool HasDepthVar = false;
static bool DoResolve = false;
static bool IsRAWZflag = false;

struct DFLIST {
  D3DFORMAT	FourCC;
  char		*Name;
}

/* these are all available ones */
ReplacementList[8] = {
  CODE_INTZ, "INTZ",
  D3DFMT_D24S8, "D24S8",
  D3DFMT_D24X4S4, "D3DFMT_D24X4S4",
  D3DFMT_D24FS8, "D24FS8",
  CODE_DF24, "DF24",
  CODE_DF16, "DF16",
  D3DFMT_D15S1, "D15S1",
  CODE_RAWZ, "RAWZ"
/*D3DFMT_D32, "DF32",
  D3DFMT_D16, "DF16",*/
},

/* these ones can be read (and bound at the same time) */
#define RAWZINDEX 3
DepthList[4] = {
  CODE_INTZ, "INTZ",
  CODE_DF24, "DF24",
  CODE_DF16, "DF16",
  CODE_RAWZ, "RAWZ"
};

D3DFORMAT GetDepthBufferFormat(IDirect3D9 *pD3D, D3DFORMAT def, D3DMULTISAMPLE_TYPE MS) {
  D3DDISPLAYMODE d3ddm;
  if (pD3D->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &d3ddm) == D3D_OK) {
    int DepthCount;
    for (DepthCount = 0; DepthCount < 8; DepthCount++) {
      if (pD3D->CheckDeviceFormat(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, d3ddm.Format, D3DUSAGE_DEPTHSTENCIL, D3DRTYPE_SURFACE, ReplacementList[DepthCount].FourCC) != D3D_OK)
	continue;

      if (MS != D3DMULTISAMPLE_NONE)
	if (pD3D->CheckDeviceMultiSampleType(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, ReplacementList[DepthCount].FourCC, FALSE, MS, NULL) != D3D_OK)
	  continue;

      _MESSAGE("Replaced depth-stencil by %s", ReplacementList[DepthCount].Name);
      return ReplacementList[DepthCount].FourCC;
    }
  }

  return def;
}

#ifdef	OBGE_NOSHADER
static bool DoesRESZflag = false;
static bool DoesNULLflag = false;
static bool DoesFCH4flag = false;

bool DoesRESZ(void) { return DoesRESZflag; };
bool DoesNULL(void) { return DoesNULLflag; };
bool DoesFCH4(void) { return DoesFCH4flag; };
#endif

bool v1_2_416::NiDX9ImplicitDepthStencilBufferDataEx::GetBufferDataHook(IDirect3DDevice9 *D3DDevice) {
  HRESULT hr;
  UInt32 Width, Height;

  _MESSAGE("Re-attaching depth buffer texture.");
  assert(NULL);

  IsRAWZflag = false;

  Width = v1_2_416::GetRenderer()->SizeWidth;
  Height = v1_2_416::GetRenderer()->SizeHeight;

  IDirect3D9	*pD3D;
  D3DDISPLAYMODE d3ddm;
  D3DDevice->GetDirect3D(&pD3D);
  pD3D->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &d3ddm);

#ifdef	OBGE_NOSHADER
  DoesRESZflag = (D3D_OK == pD3D->CheckDeviceFormat(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, d3ddm.Format, D3DUSAGE_RENDERTARGET, D3DRTYPE_SURFACE, CODE_RESZ));
  DoesNULLflag = (D3D_OK == pD3D->CheckDeviceFormat(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, d3ddm.Format, D3DUSAGE_RENDERTARGET, D3DRTYPE_SURFACE, CODE_NULL));
  DoesFCH4flag = (D3D_OK == pD3D->CheckDeviceFormat(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, d3ddm.Format, D3DUSAGE_RENDERTARGET, D3DRTYPE_SURFACE, CODE_DF24));
#endif

  if (UseDepthBuffer.Get()) {
    /* even if the old format was INTZ we need to allocate a
     * seperate surface to allow it to be bound as a texture
     */
    D3DDevice->GetDepthStencilSurface(&pOldSurface);
    pOldSurface->GetDesc(&DepthInfo);
    HasDepthVar = true;

    int DepthCount;
    for (DepthCount = 0; DepthCount < 4; DepthCount++) {
      hr = pD3D->CheckDeviceFormat(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, d3ddm.Format, D3DUSAGE_DEPTHSTENCIL, D3DRTYPE_SURFACE, DepthList[DepthCount].FourCC);

      if (hr == D3D_OK) {
        D3DDevice->CreateTexture(Width, Height , 1, D3DUSAGE_DEPTHSTENCIL, DepthList[DepthCount].FourCC, D3DPOOL_DEFAULT, &pDepthTexture, NULL);
        _MESSAGE("Depth buffer texture (%s) (%i,%i) created OK.", DepthList[DepthCount].Name, Width, Height);

        // Retrieve depth buffer surface from texture interface
        pDepthTexture->GetSurfaceLevel(0, &pDepthSurface);

        // Check if it's multi-sampled
	if (DoesRESZ() && (IsMultiSampled() || (DepthInfo.MultiSampleType != D3DMULTISAMPLE_NONE))) {
	  assert (DoesNULL());
	  D3DDevice->CreateRenderTarget(DepthInfo.Width, DepthInfo.Height, CODE_NULL, DepthInfo.MultiSampleType, DepthInfo.MultiSampleQuality, FALSE, &pMultiSurface, NULL);
	  assert(pMultiSurface);

	  _MESSAGE("Multi-sampled depth buffer resolve established. %i", DepthInfo.MultiSampleType);
	  DoResolve = true; break;
	}

	// Bind depth buffer if not multi-sampled
        hr = D3DDevice->SetDepthStencilSurface(pDepthSurface);
        if (hr == D3D_OK) {
          _MESSAGE("Depth buffer reattached OK. %i", DepthCount);

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
	  if (!ShaderManager::GetSingleton()->SetTransferZ(IsRAWZflag ? 1 : 0) ||
	      !EffectManager::GetSingleton()->SetTransferZ(IsRAWZflag ? 1 : 0))
	    UseRAWZfix.Set(false);

          break;
        }
        else {
          _MESSAGE("Failed to reattach depth buffer.");

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

  IsRAWZflag = false;

  Width = v1_2_416::GetRenderer()->SizeWidth;
  Height = v1_2_416::GetRenderer()->SizeHeight;

  IDirect3D9 *pD3D;
  D3DDISPLAYMODE d3ddm;
  Device->GetDirect3D(&pD3D);
  pD3D->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &d3ddm);

#ifdef	OBGE_NOSHADER
  DoesRESZflag = (D3D_OK == pD3D->CheckDeviceFormat(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, d3ddm.Format, D3DUSAGE_RENDERTARGET, D3DRTYPE_SURFACE, CODE_RESZ));
  DoesNULLflag = (D3D_OK == pD3D->CheckDeviceFormat(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, d3ddm.Format, D3DUSAGE_RENDERTARGET, D3DRTYPE_SURFACE, CODE_NULL));
  DoesFCH4flag = (D3D_OK == pD3D->CheckDeviceFormat(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, d3ddm.Format, D3DUSAGE_RENDERTARGET, D3DRTYPE_SURFACE, CODE_DF24));
#endif

  if (UseDepthBuffer.Get()) {
    /* even if the old format was INTZ we need to allocate a
     * seperate surface to allow it to be bound as a texture
     */
    Device->GetDepthStencilSurface(&pOldSurface);
    pOldSurface->GetDesc(&DepthInfo);
    HasDepthVar = true;

    int DepthCount;
    for (DepthCount = 0; DepthCount < 4; DepthCount++) {
      hr = pD3D->CheckDeviceFormat(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, d3ddm.Format, D3DUSAGE_DEPTHSTENCIL, D3DRTYPE_SURFACE, DepthList[DepthCount].FourCC);

      if (hr == D3D_OK) {
        Device->CreateTexture(Width, Height, 1, D3DUSAGE_DEPTHSTENCIL, DepthList[DepthCount].FourCC, D3DPOOL_DEFAULT, &pDepthTexture, NULL);
        _MESSAGE("Depth buffer texture (%s) (%i,%i) created OK.", DepthList[DepthCount].Name, Width, Height);

        // Retrieve depth buffer surface from texture interface
        pDepthTexture->GetSurfaceLevel(0, &pDepthSurface);

	// Check if it's multi-sampled
	if (DoesRESZ() && (IsMultiSampled() || (DepthInfo.MultiSampleType != D3DMULTISAMPLE_NONE))) {
	  assert (DoesNULL());
	  Device->CreateRenderTarget(DepthInfo.Width, DepthInfo.Height, CODE_NULL, DepthInfo.MultiSampleType, DepthInfo.MultiSampleQuality, FALSE, &pMultiSurface, NULL);
	  assert(pMultiSurface);

	  _MESSAGE("Multi-sampled depth buffer resolve established. %i", DepthInfo.MultiSampleType);
	  DoResolve = true; break;
	}

	// Bind depth buffer if not multi-sampled
        hr = Device->SetDepthStencilSurface(pDepthSurface);
        if (hr == D3D_OK) {
          _MESSAGE("Depth buffer reattached OK. %i", DepthCount);

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
	  if (!ShaderManager::GetSingleton()->SetTransferZ(IsRAWZflag ? 1 : 0) ||
	      !EffectManager::GetSingleton()->SetTransferZ(IsRAWZflag ? 1 : 0))
	    UseRAWZfix.Set(false);

          break;
        }
        else {
          _MESSAGE("Failed to reattach depth buffer.");
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

  /* corrections only if the own the depth-stencil surface */
  if (!DoesRESZ() || (DepthInfo.MultiSampleType == D3DMULTISAMPLE_NONE)) {
    /**/ if ((TextureFormat == CODE_INTZ) ||
	     (TextureFormat == CODE_RAWZ))
      TextureFormat = D3DFMT_D24S8;
    else if ((TextureFormat == CODE_DF24))
      TextureFormat = D3DFMT_D24X8;									// ATi's DF formats don't have stencil.
    else if ((TextureFormat == CODE_DF16))
      TextureFormat = D3DFMT_D16;
  }

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

IDirect3DTexture9 *ResolvableDepthBuffer(IDirect3DSurface9 *DepthS, IDirect3DTexture9 *DepthT) {
  if (DoResolve && (!DepthS || (DepthS == pOldSurface)))
    return pDepthTexture;

  return DepthT;
}

bool ResolveDepthBuffer(IDirect3DDevice9 *Device) {
  if (DoResolve) {
    Device->SetRenderTarget(0, pMultiSurface);
    IDirect3DSurface9 *pSurface = NULL;
    Device->GetDepthStencilSurface(&pSurface);
    if (pSurface != pOldSurface)
      Device->SetDepthStencilSurface(pOldSurface);

//  Device->EndScene();
//  Device->BeginScene();

    // Bind depth stencil texture to texture sampler 0
    Device->SetTexture(0, pDepthTexture);

    // Perform a dummy draw call to ensure texture sampler 0 is set before the
    // resolve is triggered
    // Vertex declaration and shaders may need to be adjusted to ensure no debug
    // error message is produced
    D3DXVECTOR3 vDummyPoint(0.0f, 0.0f, 0.0f);

    DWORD dCurrZE;
    DWORD dCurrZW;
    DWORD dCurrCW;

    Device->GetRenderState(D3DRS_ZENABLE, &dCurrZE);
    Device->GetRenderState(D3DRS_ZWRITEENABLE, &dCurrZW);
    Device->GetRenderState(D3DRS_COLORWRITEENABLE, &dCurrCW);

    Device->SetRenderState(D3DRS_ZENABLE, D3DZB_FALSE);
    Device->SetRenderState(D3DRS_ZWRITEENABLE, D3DZB_FALSE);
    Device->SetRenderState(D3DRS_COLORWRITEENABLE, 0);

    Device->SetVertexShader(NULL);
    Device->SetPixelShader(NULL);

    Device->SetFVF(D3DFVF_XYZ);
    Device->DrawPrimitiveUP(D3DPT_POINTLIST, 1, vDummyPoint, sizeof(D3DXVECTOR3));

#define RESZ_MAGIC 0x7fa05000
    // Trigger the depth buffer resolve; after this call texture sampler 0
    // will contain the contents of the resolve operation
    Device->SetRenderState(D3DRS_POINTSIZE, RESZ_MAGIC);

    Device->SetRenderState(D3DRS_ZENABLE, dCurrZE);
    Device->SetRenderState(D3DRS_ZWRITEENABLE, dCurrZW);
    Device->SetRenderState(D3DRS_COLORWRITEENABLE, dCurrCW);

//  Device->EndScene();
//  Device->BeginScene();

    Device->SetDepthStencilSurface(pSurface);

    return true;
  }

  return false;
};

IDirect3DSurface9 *GetStencilSurface() {
  return pOldSurface;
};

IDirect3DSurface9 *GetDepthBufferSurface() {
  return pDepthSurface;
};

IDirect3DTexture9 *GetDepthBufferTexture() {
  return pDepthTexture;
};

bool HasDepth(void) {
  return HasDepthVar;
};

bool IsRAWZ(void) {
  return IsRAWZflag;
};
