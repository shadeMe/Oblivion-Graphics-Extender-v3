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

#ifndef	D3D9TEXTURE_HPP
#define	D3D9TEXTURE_HPP

#include <intrin.h>
#include <d3d9.h>
#include <d3dx9.h>

#include "D3D9Identifiers.hpp"
#include "D3D9Surface.hpp"

class OBGEDirect3DTexture9 : public IDirect3DTexture9
{
	friend class OBGEDirect3DDevice9;

public:
	// We need d3d so that we'd use a pointer to OBGEDirect3D9 instead of the original IDirect3D9 implementor
	// in functions like GetDirect3D9
	OBGEDirect3DTexture9(IDirect3D9* d3d, IDirect3DDevice9* device, IDirect3DTexture9* texture) : m_d3d(d3d), m_device(device), m_texture(texture)
	{
		_MESSAGE("OD3D9: Texture constructed from 0x%08x", _ReturnAddress());
	}

	/*** IUnknown methods ***/
	STDMETHOD(QueryInterface)(THIS_ REFIID riid, void** ppvObj)
	{
		return m_texture->QueryInterface(riid, ppvObj);
	}

	STDMETHOD_(ULONG,AddRef)(THIS)
	{
		return m_texture->AddRef();
	}

	STDMETHOD_(ULONG,Release)(THIS)
	{
		ULONG count = m_texture->Release();
		if(0 == count)
			delete this;

		return count;
	}

	/*** IDirect3DBaseTexture9 methods ***/
	STDMETHOD(GetDevice)(THIS_ IDirect3DDevice9** ppDevice)
	{
		// Let the device validate the incoming pointer for us
		HRESULT hr = m_texture->GetDevice(ppDevice);
		if(SUCCEEDED(hr))
			*ppDevice = m_device;

		return hr;
	}

	STDMETHOD(SetPrivateData)(THIS_ REFGUID refguid,CONST void* pData,DWORD SizeOfData,DWORD Flags)
	{
		return m_texture->SetPrivateData(refguid, pData, SizeOfData, Flags);
	}

	STDMETHOD(GetPrivateData)(THIS_ REFGUID refguid,void* pData,DWORD* pSizeOfData)
	{
		return m_texture->GetPrivateData(refguid, pData, pSizeOfData);
	}

	STDMETHOD(FreePrivateData)(THIS_ REFGUID refguid)
	{
		return m_texture->FreePrivateData(refguid);
	}

	STDMETHOD_(DWORD, SetPriority)(THIS_ DWORD PriorityNew)
	{
		return m_texture->SetPriority(PriorityNew);
	}

	STDMETHOD_(DWORD, GetPriority)(THIS)
	{
		return m_texture->GetPriority();
	}

	STDMETHOD_(void, PreLoad)(THIS)
	{
		m_texture->PreLoad();
	}

	STDMETHOD_(D3DRESOURCETYPE, GetType)(THIS)
	{
		return m_texture->GetType();
	}

	STDMETHOD_(DWORD, SetLOD)(THIS_ DWORD LODNew)
	{
		return m_texture->SetLOD(LODNew);
	}

	STDMETHOD_(DWORD, GetLOD)(THIS)
	{
		return m_texture->GetLOD();
	}

	STDMETHOD_(DWORD, GetLevelCount)(THIS)
	{
		return m_texture->GetLevelCount();
	}

	STDMETHOD(SetAutoGenFilterType)(THIS_ D3DTEXTUREFILTERTYPE FilterType)
	{
		return m_texture->SetAutoGenFilterType(FilterType);
	}

	STDMETHOD_(D3DTEXTUREFILTERTYPE, GetAutoGenFilterType)(THIS)
	{
		return m_texture->GetAutoGenFilterType();
	}

	STDMETHOD_(void, GenerateMipSubLevels)(THIS)
	{
		m_texture->GenerateMipSubLevels();
	}

	STDMETHOD(GetLevelDesc)(THIS_ UINT Level,D3DSURFACE_DESC *pDesc)
	{
		return m_texture->GetLevelDesc(Level, pDesc);
	}

	STDMETHOD(GetSurfaceLevel)(THIS_ UINT Level,IDirect3DSurface9** ppSurfaceLevel)
	{
#ifdef	OBGE_LOGGING
		if (frame_log) {
			frame_log->FormattedMessage("GetSurfaceLevel from 0x%08x", _ReturnAddress());
		}
#endif

		HRESULT hr = m_texture->GetSurfaceLevel(Level, ppSurfaceLevel);

		if(SUCCEEDED(hr))
		{
#ifdef	OBGE_TRACKER_SURFACES
#if	OBGE_TRACKER_SURFACES > 0
			// Return our surface
			*ppSurfaceLevel = new OBGEDirect3DSurface9(m_d3d, m_device, *ppSurfaceLevel);
#endif
#endif

			if (frame_log || frame_trk) {
				struct textureSurface *track = new struct textureSurface;

				track->Level = Level;
				track->map = textureMaps[this];

				surfaceTexture[*ppSurfaceLevel] = track;
			}
		}

#ifdef	OBGE_LOGGING
		if (frame_log) {
			struct textureMap *track = textureMaps[this];
			if (track) {
				frame_log->Indent();
				frame_log->FormattedMessage("{W,H}: {%d,%d}", track->Width, track->Height);
				frame_log->FormattedMessage("Format: %s", findFormat(track->Format));
				frame_log->FormattedMessage("Levels: %d", track->Levels);
				frame_log->FormattedMessage("Usage: %s", findUsage(track->Usage));
				frame_log->Outdent();
			}
			else {
				frame_log->FormattedMessage("Untracked texture!");
			}
		}
		else
			_MESSAGE("OD3D9: GetSurfaceLevel from 0x%08x: 0x%08x", _ReturnAddress(), *ppSurfaceLevel);
#endif

		return hr;
	}

	STDMETHOD(LockRect)(THIS_ UINT Level,D3DLOCKED_RECT* pLockedRect,CONST RECT* pRect,DWORD Flags)
	{
		return m_texture->LockRect(Level, pLockedRect, pRect, Flags);
	}

	STDMETHOD(UnlockRect)(THIS_ UINT Level)
	{
		return m_texture->UnlockRect(Level);
	}

	STDMETHOD(AddDirtyRect)(THIS_ CONST RECT* pDirtyRect)
	{
		return m_texture->AddDirtyRect(pDirtyRect);
	}

public:
	IDirect3DTexture9* getTexture() {
		return m_texture;
	}

private:
	IDirect3DTexture9* m_texture;
	IDirect3DDevice9* m_device;
	IDirect3D9* m_d3d;
};

static inline IDirect3DTexture9 *translateTexture(IDirect3DTexture9 *org) {
  if (textureMaps[org])
    return ((OBGEDirect3DTexture9 *)org)->getTexture();

  return org;
}

#endif
