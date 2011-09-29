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

#include "D3D9.hpp"
#include "D3D9Device.hpp"

#include "GlobalSettings.h"
#include "OBSEShaderInterface.h"

static global<int> MultiSample(0, "Oblivion.ini", "Display", "iMultiSample");

#ifndef	OBGE_NOSHADER

// Tracker
OBGEDirect3D9 *lastOBGEDirect3D9 = NULL;
D3DCAPS9 lastOBGEDirect3D9CAPS = {D3DDEVTYPE_HAL};
static std::vector<OBGEDirect3D9 *> OBGEDrivers;

/* ----------------------------------------------------------------------------- */

#include "D3D9Device.hpp"
#include "D3D9Identifiers.hpp"

const D3DFORMAT testingFmts[] = {
  D3DFMT_R8G8B8             ,
  D3DFMT_A8R8G8B8           ,
  D3DFMT_X8R8G8B8           ,
  D3DFMT_R5G6B5             ,
  D3DFMT_X1R5G5B5           ,
  D3DFMT_A1R5G5B5           ,
  D3DFMT_A4R4G4B4           ,
  D3DFMT_R3G3B2             ,
  D3DFMT_A8                 ,
  D3DFMT_A8R3G3B2           ,
  D3DFMT_X4R4G4B4           ,
  D3DFMT_A2B10G10R10        ,
  D3DFMT_A8B8G8R8           ,
  D3DFMT_X8B8G8R8           ,
  D3DFMT_G16R16             ,
  D3DFMT_A2R10G10B10        ,
  D3DFMT_A16B16G16R16       ,
  D3DFMT_A8P8               ,
  D3DFMT_P8                 ,
  D3DFMT_L8                 ,
  D3DFMT_A8L8               ,
  D3DFMT_A4L4               ,
  D3DFMT_V8U8               ,
  D3DFMT_L6V5U5             ,
  D3DFMT_X8L8V8U8           ,
  D3DFMT_Q8W8V8U8           ,
  D3DFMT_V16U16             ,
  D3DFMT_A2W10V10U10        ,
  D3DFMT_D16_LOCKABLE       ,
  D3DFMT_D32                ,
  D3DFMT_D15S1              ,
  D3DFMT_D24S8              ,
  D3DFMT_D24X8              ,
  D3DFMT_D24X4S4            ,
  D3DFMT_D16                ,
  D3DFMT_D32F_LOCKABLE      ,
  D3DFMT_D24FS8             ,
  D3DFMT_D32_LOCKABLE       ,
  D3DFMT_S8_LOCKABLE        ,
  D3DFMT_L16                ,
  D3DFMT_VERTEXDATA         ,
  D3DFMT_INDEX16            ,
  D3DFMT_INDEX32            ,
  D3DFMT_Q16W16V16U16       ,
  D3DFMT_R16F               ,
  D3DFMT_G16R16F            ,
  D3DFMT_A16B16G16R16F      ,
  D3DFMT_R32F               ,
  D3DFMT_G32R32F            ,
  D3DFMT_A32B32G32R32F      ,
  D3DFMT_CxV8U8             ,
  D3DFMT_A1                 ,
  D3DFMT_A2B10G10R10_XR_BIAS,
  D3DFMT_BINARYBUFFER       ,

  D3DFMT_MULTI2_ARGB8       ,
  D3DFMT_UYVY               ,
  D3DFMT_R8G8_B8G8          ,
  D3DFMT_YUY2               ,
  D3DFMT_G8R8_G8B8          ,
  D3DFMT_DXT1               ,
  D3DFMT_DXT2               ,
  D3DFMT_DXT3               ,
  D3DFMT_DXT4               ,
  D3DFMT_DXT5               ,

  CODE_INTZ,
  CODE_DF24,
  CODE_DF16,
  CODE_RAWZ,
  CODE_RESZ,
  CODE_NULL,

  (D3DFORMAT)MAKEFOURCC('A','T','I','1'),
  (D3DFORMAT)MAKEFOURCC('A','T','I','2'),

  (D3DFORMAT)MAKEFOURCC('A','T','O','C'),
  (D3DFORMAT)MAKEFOURCC('S','S','A','A'),
  (D3DFORMAT)MAKEFOURCC('N','V','D','B'),
  (D3DFORMAT)MAKEFOURCC('R','2','V','B'),
  (D3DFORMAT)MAKEFOURCC('I','N','S','T'),
};

static bool DoesRESZflag = false;
static bool DoesNULLflag = false;
static bool DoesFCH4flag = false;

bool DoesRESZ(void) { return DoesRESZflag; };
bool DoesNULL(void) { return DoesNULLflag; };
bool DoesFCH4(void) { return DoesFCH4flag; };

OBGEDirect3D9::OBGEDirect3D9(IDirect3D9 *d3d) : m_d3d(d3d) {
  _MESSAGE("OD3D9: Driver 0x%08x constructed from 0x%08x (%d drivers available)", this, _ReturnAddress(), OBGEDrivers.size() + 1);

  DoesRESZflag = false;
  DoesNULLflag = false;
  DoesFCH4flag = false;

  /* --------------------------------------------------- */
  D3DDISPLAYMODE d3ddm;
  if (m_d3d->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &d3ddm) == D3D_OK) {
    m_d3d->GetDeviceCaps(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, &lastOBGEDirect3D9CAPS);

    HRESULT hrs, hrt;
    char rts[2048]; rts[0] = '\0';
    char dss[2048]; dss[0] = '\0';
    for (int fmtn = 0; fmtn < (sizeof(testingFmts) / sizeof(D3DFORMAT)); fmtn++) {
      D3DFORMAT fmt = testingFmts[fmtn];
      const char *FMT = findFormat(fmt);
      if (strcmp(FMT, "unknown")) {
	  hrs = m_d3d->CheckDeviceFormat(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, d3ddm.Format, D3DUSAGE_RENDERTARGET, D3DRTYPE_SURFACE, fmt);
	  hrt = m_d3d->CheckDeviceFormat(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, d3ddm.Format, D3DUSAGE_RENDERTARGET, D3DRTYPE_TEXTURE, fmt);
	  if ((hrs == D3D_OK) || (hrt == D3D_OK)) {
	    if (rts[0])
	      strcat(rts, ", ");
	    strcat(rts, FMT);

	    if (hrs != hrt) {
	      if (hrs == D3D_OK)
		strcat(rts, " (srf)");
	      if (hrt == D3D_OK)
		strcat(rts, " (tex)");
	    }
	  }

	  if ((fmt == CODE_NULL) && ((hrs == D3D_OK) || (hrt == D3D_OK))) DoesNULLflag = true;

	  hrs = m_d3d->CheckDeviceFormat(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, d3ddm.Format, D3DUSAGE_DEPTHSTENCIL, D3DRTYPE_SURFACE, fmt);
	  hrt = m_d3d->CheckDeviceFormat(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, d3ddm.Format, D3DUSAGE_DEPTHSTENCIL, D3DRTYPE_TEXTURE, fmt);
	  if ((hrs == D3D_OK) || (hrt == D3D_OK)) {
	    if (dss[0])
	      strcat(dss, ", ");
	    strcat(dss, FMT);

	    if (hrs != hrt) {
	      if (hrs == D3D_OK)
		strcat(dss, " (srf)");
	      if (hrt == D3D_OK)
		strcat(dss, " (tex)");
	    }
	  }

	  if ((fmt == CODE_DF24) && ((hrs == D3D_OK) || (hrt == D3D_OK))) DoesFCH4flag = true;
	  if ((fmt == CODE_RESZ) && ((hrs == D3D_OK) || (hrt == D3D_OK))) DoesRESZflag = true;
      }
    }

    _MESSAGE("OD3D9: Supported render-targets: %s", rts);
    _MESSAGE("OD3D9: Supported depth-stencils: %s", dss);

    if (DoesNULLflag) {
      hrs = m_d3d->CheckDeviceMultiSampleType(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, (D3DFORMAT)MAKEFOURCC('N','U','L','L'), FALSE, D3DMULTISAMPLE_2_SAMPLES, NULL);

      _MESSAGE("OD3D9: NULL has multi-sample support: %s", hrs == D3D_OK ? "yes" : "no");
    }

    if (DoesFCH4flag)
      _MESSAGE("OD3D9: Fetch4 supported.");
    else
      _MESSAGE("OD3D9: Fetch4 not supported.");
  }

  /* add to vector and replace by the new version */
  lastOBGEDirect3D9 = this;
  OBGEDrivers.push_back(this);
}

OBGEDirect3D9::~OBGEDirect3D9() {
  /* remove from vector and replace by the other version */
  OBGEDrivers.erase(std::find(OBGEDrivers.begin(), OBGEDrivers.end(), this));
  lastOBGEDirect3D9 = (OBGEDrivers.size() ? OBGEDrivers.back() : NULL);

  _MESSAGE("OD3D9: Driver 0x%08x destructed from 0x%08x (%d drivers left)", this, _ReturnAddress(), OBGEDrivers.size());
}

/*** IUnknown methods ***/
HRESULT STDMETHODCALLTYPE OBGEDirect3D9::QueryInterface(REFIID riid, void **ppvObj) {
  return m_d3d->QueryInterface(riid, ppvObj);
}

ULONG STDMETHODCALLTYPE OBGEDirect3D9::AddRef() {
  return m_d3d->AddRef();
}

ULONG STDMETHODCALLTYPE OBGEDirect3D9::Release() {
  ULONG count = m_d3d->Release();

  if (0 == count)
    delete this;

  return count;
}

/*** IDirect3D9 methods ***/
HRESULT STDMETHODCALLTYPE OBGEDirect3D9::RegisterSoftwareDevice(void *pInitializeFunction) {
  return m_d3d->RegisterSoftwareDevice(pInitializeFunction);
}

UINT STDMETHODCALLTYPE OBGEDirect3D9::GetAdapterCount() {
  return m_d3d->GetAdapterCount();
}

HRESULT STDMETHODCALLTYPE OBGEDirect3D9::GetAdapterIdentifier( UINT Adapter, DWORD Flags, D3DADAPTER_IDENTIFIER9 *pIdentifier) {
  return m_d3d->GetAdapterIdentifier(Adapter, Flags, pIdentifier);
}

UINT STDMETHODCALLTYPE OBGEDirect3D9::GetAdapterModeCount( UINT Adapter, D3DFORMAT Format) {
  return m_d3d->GetAdapterModeCount(Adapter, Format);
}

HRESULT STDMETHODCALLTYPE OBGEDirect3D9::EnumAdapterModes( UINT Adapter, D3DFORMAT Format, UINT Mode, D3DDISPLAYMODE *pMode) {
  return m_d3d->EnumAdapterModes(Adapter, Format, Mode, pMode);
}

HRESULT STDMETHODCALLTYPE OBGEDirect3D9::GetAdapterDisplayMode( UINT Adapter, D3DDISPLAYMODE *pMode) {
  return m_d3d->GetAdapterDisplayMode(Adapter, pMode);
}

HRESULT STDMETHODCALLTYPE OBGEDirect3D9::CheckDeviceType( UINT Adapter, D3DDEVTYPE DevType, D3DFORMAT AdapterFormat, D3DFORMAT BackBufferFormat, BOOL bWindowed) {
  return m_d3d->CheckDeviceType(Adapter, DevType, AdapterFormat, BackBufferFormat, bWindowed);
}

HRESULT STDMETHODCALLTYPE OBGEDirect3D9::CheckDeviceFormat( UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT AdapterFormat, DWORD Usage, D3DRESOURCETYPE RType, D3DFORMAT CheckFormat) {
  return m_d3d->CheckDeviceFormat(Adapter, DeviceType, AdapterFormat, Usage, RType, CheckFormat);
}

HRESULT STDMETHODCALLTYPE OBGEDirect3D9::CheckDeviceMultiSampleType( UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT SurfaceFormat, BOOL Windowed, D3DMULTISAMPLE_TYPE MultiSampleType, DWORD *pQualityLevels) {
  return m_d3d->CheckDeviceMultiSampleType(Adapter, DeviceType, SurfaceFormat, Windowed, MultiSampleType, pQualityLevels);
}

HRESULT STDMETHODCALLTYPE OBGEDirect3D9::CheckDepthStencilMatch( UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT AdapterFormat, D3DFORMAT RenderTargetFormat, D3DFORMAT DepthStencilFormat) {
  return m_d3d->CheckDepthStencilMatch(Adapter, DeviceType, AdapterFormat, RenderTargetFormat, DepthStencilFormat);
}

HRESULT STDMETHODCALLTYPE OBGEDirect3D9::CheckDeviceFormatConversion( UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT SourceFormat, D3DFORMAT TargetFormat) {
  return m_d3d->CheckDeviceFormatConversion(Adapter, DeviceType, SourceFormat, TargetFormat);
}

HRESULT STDMETHODCALLTYPE OBGEDirect3D9::GetDeviceCaps( UINT Adapter, D3DDEVTYPE DeviceType, D3DCAPS9 *pCaps) {
  return m_d3d->GetDeviceCaps(Adapter, DeviceType, pCaps);
}

HMONITOR STDMETHODCALLTYPE OBGEDirect3D9::GetAdapterMonitor( UINT Adapter) {
  return m_d3d->GetAdapterMonitor(Adapter);
}

HRESULT STDMETHODCALLTYPE OBGEDirect3D9::CreateDevice(
	UINT Adapter,
	D3DDEVTYPE DeviceType,
	HWND hFocusWindow,
	DWORD BehaviorFlags,
	D3DPRESENT_PARAMETERS *pPresentationParameters,
	IDirect3DDevice9 **ppReturnedDeviceInterface) {

  HRESULT hr;
  D3DFORMAT asbak;

#if 0
  /* check if the format isn't possibly really available */
  if (MultiSample.Get() && (pPresentationParameters->MultiSampleType == D3DMULTISAMPLE_NONE)) {
    D3DFORMAT frmt = D3DFMT_UNKNOWN;
    if (IsHDR())
      frmt = D3DFMT_A16B16G16R16F;
    else
      frmt = D3DFMT_A8R8G8B8;

    hr = m_d3d->CheckDeviceMultiSampleType(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, frmt, FALSE, (D3DMULTISAMPLE_TYPE)MultiSample.Get(), NULL);
    if (SUCCEEDED(hr)) {
      pPresentationParameters->MultiSampleType = (D3DMULTISAMPLE_TYPE)MultiSample.Get();
      pPresentationParameters->MultiSampleQuality = 0;
      pPresentationParameters->Flags &= ~D3DPRESENTFLAG_LOCKABLE_BACKBUFFER;
    }
  }
#endif

  /* replace whatever by INTZ for example if possible */
  pPresentationParameters->AutoDepthStencilFormat =
    GetDepthBufferFormat(m_d3d, asbak =
      pPresentationParameters->AutoDepthStencilFormat,
	pPresentationParameters->MultiSampleType);

  /* try -> fail -> restore */
  hr = m_d3d->CreateDevice(Adapter, DeviceType, hFocusWindow, BehaviorFlags,
                           pPresentationParameters, ppReturnedDeviceInterface);
  if (!SUCCEEDED(hr)) {
    pPresentationParameters->AutoDepthStencilFormat = asbak;
    hr = m_d3d->CreateDevice(Adapter, DeviceType, hFocusWindow, BehaviorFlags,
			     pPresentationParameters, ppReturnedDeviceInterface);
  }

  _MESSAGE("OD3D9: Device queried from 0x%08x", _ReturnAddress());
  _MESSAGE("OD3D9: Queue %d, MS-type %d, MS-quality %d",
    pPresentationParameters->BackBufferCount,
    pPresentationParameters->MultiSampleType,
    pPresentationParameters->MultiSampleQuality);

  // Return our device
  if (SUCCEEDED(hr))
    *ppReturnedDeviceInterface = new OBGEDirect3DDevice9(this, *ppReturnedDeviceInterface);

  return hr;
}

#endif

bool IsMultiSampled() {
  return !!MultiSample.Get();
}
