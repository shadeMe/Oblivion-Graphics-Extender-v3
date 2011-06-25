#ifndef	OBGE_NOSHADER

#include "D3D9.hpp"
#include "D3D9Device.hpp"

// Tracker
OBGEDirect3D9 *lastOBGEDirect3D9 = NULL;
static std::vector<OBGEDirect3D9 *> OBGEDrivers;

/* ----------------------------------------------------------------------------- */

#include "D3D9Device.hpp"


OBGEDirect3D9::OBGEDirect3D9(IDirect3D9 *d3d) : m_d3d(d3d) {
  _MESSAGE("OD3D9: Driver 0x%08x constructed from 0x%08x (%d drivers available)", this, _ReturnAddress(), OBGEDrivers.size() + 1);

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

HRESULT STDMETHODCALLTYPE OBGEDirect3D9::CreateDevice( UINT Adapter, D3DDEVTYPE DeviceType, HWND hFocusWindow, DWORD BehaviorFlags, D3DPRESENT_PARAMETERS *pPresentationParameters, IDirect3DDevice9 **ppReturnedDeviceInterface) {
  HRESULT hr = m_d3d->CreateDevice(Adapter, DeviceType, hFocusWindow, BehaviorFlags,
                                   pPresentationParameters, ppReturnedDeviceInterface);

  _MESSAGE("OD3D9: Device queried from 0x%08x", _ReturnAddress());

  if (SUCCEEDED(hr)) {
    // Return our device
    *ppReturnedDeviceInterface = new OBGEDirect3DDevice9(this, *ppReturnedDeviceInterface);
  }

  return hr;
}

#endif
