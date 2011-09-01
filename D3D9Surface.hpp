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

#ifndef	D3D9SURFACE_HPP
#define	D3D9SURFACE_HPP

#include <intrin.h>
#include <d3d9.h>
#include <d3dx9.h>

#include "D3D9Identifiers.hpp"

class OBGEDirect3DSurface9 : public IDirect3DSurface9
{
public:
	// We need d3d so that we'd use a pointer to OBGEDirect3D9 instead of the original IDirect3D9 implementor
	// in functions like GetDirect3D9
	OBGEDirect3DSurface9(IDirect3D9* d3d, IDirect3DDevice9* device, IDirect3DSurface9* surface) : m_d3d(d3d), m_device(device), m_surface(surface)
	{
	//	_MESSAGE("OD3D9: Surface constructed from 0x%08x", _ReturnAddress());
	}

	/*** IUnknown methods ***/
	STDMETHOD(QueryInterface)(THIS_ REFIID riid, void** ppvObj)
	{
		return m_surface->QueryInterface(riid, ppvObj);
	}

	STDMETHOD_(ULONG,AddRef)(THIS)
	{
		return m_surface->AddRef();
	}

	STDMETHOD_(ULONG,Release)(THIS)
	{
		ULONG count = m_surface->Release();
		if(0 == count)
			delete this;

		return count;
	}

	/*** IDirect3DResource9 methods ***/
	STDMETHOD(GetDevice)(THIS_ IDirect3DDevice9** ppDevice)
	{
		// Let the device validate the incoming pointer for us
		HRESULT hr = m_surface->GetDevice(ppDevice);
		if(SUCCEEDED(hr))
			*ppDevice = m_device;

		return hr;
	}

	STDMETHOD(SetPrivateData)(THIS_ REFGUID refguid,CONST void* pData,DWORD SizeOfData,DWORD Flags)
	{
		return m_surface->SetPrivateData(refguid, pData, SizeOfData, Flags);
	}

	STDMETHOD(GetPrivateData)(THIS_ REFGUID refguid,void* pData,DWORD* pSizeOfData)
	{
		return m_surface->GetPrivateData(refguid, pData, pSizeOfData);
	}

	STDMETHOD(FreePrivateData)(THIS_ REFGUID refguid)
	{
		return m_surface->FreePrivateData(refguid);
	}

	STDMETHOD_(DWORD, SetPriority)(THIS_ DWORD PriorityNew)
	{
		return m_surface->SetPriority(PriorityNew);
	}

	STDMETHOD_(DWORD, GetPriority)(THIS)
	{
		return m_surface->GetPriority();
	}

	STDMETHOD_(void, PreLoad)(THIS)
	{
		m_surface->PreLoad();
	}

	STDMETHOD_(D3DRESOURCETYPE, GetType)(THIS)
	{
		return m_surface->GetType();
	}

	STDMETHOD(GetContainer)(THIS_ REFIID riid,void** ppContainer)
	{
		return m_surface->GetContainer(riid, ppContainer);
	}

	STDMETHOD(GetDesc)(THIS_ D3DSURFACE_DESC *pDesc)
	{
		return m_surface->GetDesc(pDesc);
	}

	STDMETHOD(LockRect)(THIS_ D3DLOCKED_RECT* pLockedRect,CONST RECT* pRect,DWORD Flags)
	{
		return m_surface->LockRect(pLockedRect, pRect, Flags);
	}

	STDMETHOD(UnlockRect)(THIS)
	{
		return m_surface->UnlockRect();
	}

	STDMETHOD(GetDC)(THIS_ HDC *phdc)
	{
		return m_surface->GetDC(phdc);
	}

	STDMETHOD(ReleaseDC)(THIS_ HDC hdc)
	{
		return m_surface->ReleaseDC(hdc);
	}

public:
	IDirect3DSurface9* getSurface() {
		return m_surface;
	}

private:
	IDirect3DSurface9* m_surface;
	IDirect3DDevice9* m_device;
	IDirect3D9* m_d3d;
};

static inline IDirect3DSurface9 *translateSurface(IDirect3DSurface9 *org) {
  if (surfaceRender[org])
    return ((OBGEDirect3DSurface9 *)org)->getSurface();
  else if (surfaceDepth[org])
    return ((OBGEDirect3DSurface9 *)org)->getSurface();
  else if (surfaceTexture[org])
    return ((OBGEDirect3DSurface9 *)org)->getSurface();

  return org;
}

#endif
