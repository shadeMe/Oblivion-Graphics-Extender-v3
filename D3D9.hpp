#ifndef	D3D9_HPP
#define	D3D9_HPP

#include <intrin.h>
#include <d3d9.h>
#include <d3dx9.h>

#undef	OBGE_LOGGING
#undef	OBGE_HOOKING
#define	OBGE_DEVLING
#undef	OBGE_PROFILE
#define	OBGE_TRACKER		0	// replace by OBGE-implementation, 0 = only rendertargets, 1 = all
#undef	OBGE_TRACKER_SURFACES		// replace by OBGE-implementation, 0 = only rendertargets, 1 = all
#undef	OBGE_TRACKER_TEXTURES		// replace by OBGE-implementation, 0 = only rendertargets, 1 = all

// Tracker
class OBGEDirect3D9; extern OBGEDirect3D9 *lastOBGEDirect3D9;

#include "D3D9Device.hpp"

class OBGEDirect3D9 : public IDirect3D9
{
public:
	OBGEDirect3D9(IDirect3D9* d3d) : m_d3d(d3d)
	{
		lastOBGEDirect3D9 = this;

		_MESSAGE("OD3D9: D3D constructed from 0x%08x", _ReturnAddress());
	}

	/*** IUnknown methods ***/
    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObj)
	{
		return m_d3d->QueryInterface(riid, ppvObj);
	}

    ULONG STDMETHODCALLTYPE AddRef()
	{
		return m_d3d->AddRef();
	}

    ULONG STDMETHODCALLTYPE Release()
	{
		ULONG count = m_d3d->Release();
		if(0 == count)
			delete this;

		return count;
	}

    /*** IDirect3D9 methods ***/
    HRESULT STDMETHODCALLTYPE RegisterSoftwareDevice(void* pInitializeFunction)
	{
		return m_d3d->RegisterSoftwareDevice(pInitializeFunction);
	}

    UINT STDMETHODCALLTYPE GetAdapterCount()
	{
		return m_d3d->GetAdapterCount();
	}

    HRESULT STDMETHODCALLTYPE GetAdapterIdentifier( UINT Adapter,DWORD Flags,D3DADAPTER_IDENTIFIER9* pIdentifier)
	{
		return m_d3d->GetAdapterIdentifier(Adapter, Flags, pIdentifier);
	}

    UINT STDMETHODCALLTYPE GetAdapterModeCount( UINT Adapter,D3DFORMAT Format)
	{
		return m_d3d->GetAdapterModeCount(Adapter, Format);
	}

    HRESULT STDMETHODCALLTYPE EnumAdapterModes( UINT Adapter,D3DFORMAT Format,UINT Mode,D3DDISPLAYMODE* pMode)
	{
		return m_d3d->EnumAdapterModes(Adapter, Format, Mode, pMode);
	}

    HRESULT STDMETHODCALLTYPE GetAdapterDisplayMode( UINT Adapter,D3DDISPLAYMODE* pMode)
	{
		return m_d3d->GetAdapterDisplayMode(Adapter, pMode);
	}

    HRESULT STDMETHODCALLTYPE CheckDeviceType( UINT Adapter,D3DDEVTYPE DevType,D3DFORMAT AdapterFormat,D3DFORMAT BackBufferFormat,BOOL bWindowed)
	{
		return m_d3d->CheckDeviceType(Adapter, DevType, AdapterFormat, BackBufferFormat, bWindowed);
	}

    HRESULT STDMETHODCALLTYPE CheckDeviceFormat( UINT Adapter,D3DDEVTYPE DeviceType,D3DFORMAT AdapterFormat,DWORD Usage,D3DRESOURCETYPE RType,D3DFORMAT CheckFormat)
	{
		return m_d3d->CheckDeviceFormat(Adapter, DeviceType, AdapterFormat, Usage, RType, CheckFormat);
	}

    HRESULT STDMETHODCALLTYPE CheckDeviceMultiSampleType( UINT Adapter,D3DDEVTYPE DeviceType,D3DFORMAT SurfaceFormat,BOOL Windowed,D3DMULTISAMPLE_TYPE MultiSampleType,DWORD* pQualityLevels)
	{
		return m_d3d->CheckDeviceMultiSampleType(Adapter, DeviceType, SurfaceFormat, Windowed, MultiSampleType, pQualityLevels);
	}

    HRESULT STDMETHODCALLTYPE CheckDepthStencilMatch( UINT Adapter,D3DDEVTYPE DeviceType,D3DFORMAT AdapterFormat,D3DFORMAT RenderTargetFormat,D3DFORMAT DepthStencilFormat)
	{
		return m_d3d->CheckDepthStencilMatch(Adapter, DeviceType, AdapterFormat, RenderTargetFormat, DepthStencilFormat);
	}

    HRESULT STDMETHODCALLTYPE CheckDeviceFormatConversion( UINT Adapter,D3DDEVTYPE DeviceType,D3DFORMAT SourceFormat,D3DFORMAT TargetFormat)
	{
		return m_d3d->CheckDeviceFormatConversion(Adapter, DeviceType, SourceFormat, TargetFormat);
	}

    HRESULT STDMETHODCALLTYPE GetDeviceCaps( UINT Adapter,D3DDEVTYPE DeviceType,D3DCAPS9* pCaps)
	{
		return m_d3d->GetDeviceCaps(Adapter, DeviceType, pCaps);
	}

    HMONITOR STDMETHODCALLTYPE GetAdapterMonitor( UINT Adapter)
	{
		return m_d3d->GetAdapterMonitor(Adapter);
	}

    HRESULT STDMETHODCALLTYPE CreateDevice( UINT Adapter,D3DDEVTYPE DeviceType,HWND hFocusWindow,DWORD BehaviorFlags,D3DPRESENT_PARAMETERS* pPresentationParameters,IDirect3DDevice9** ppReturnedDeviceInterface)
	{
		HRESULT hr = m_d3d->CreateDevice(Adapter, DeviceType, hFocusWindow, BehaviorFlags,
			pPresentationParameters, ppReturnedDeviceInterface);

		_MESSAGE("OD3D9: Device queried from 0x%08x", _ReturnAddress());

		if(SUCCEEDED(hr))
		{
			// Return our device
			*ppReturnedDeviceInterface = new OBGEDirect3DDevice9(this, *ppReturnedDeviceInterface);
		}

		return hr;
	}

private:
	IDirect3D9* m_d3d;
};

#endif
