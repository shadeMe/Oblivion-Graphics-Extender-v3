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

#pragma once

#include "D3D9.h"
#include "Nodes\NiDX9ImplicitDepthStencilBufferData.h"
#include "GlobalSettings.h"

namespace v1_2_416
{
	class NiDX9ImplicitDepthStencilBufferDataEx : public NiDX9ImplicitDepthStencilBufferData
	{
	public:
		NiDX9ImplicitDepthStencilBufferDataEx();
		virtual ~NiDX9ImplicitDepthStencilBufferDataEx();

		bool GetBufferDataHook(IDirect3DDevice9 *D3DDevice);
		bool jGetBufferData(IDirect3DDevice9 *D3DDevice);
	};
}

D3DFORMAT GetDepthBufferFormat(IDirect3D9 *pD3D, D3DFORMAT def, D3DMULTISAMPLE_TYPE MS);
void static _cdecl DepthBufferHook(IDirect3DDevice9 *Device,UInt32 u2);
UInt32 static _cdecl TextureSanityCheckHook(D3DFORMAT TextureFormat, UInt32 u2);
void CreateDepthBufferHook(void);

IDirect3DTexture9 *ResolvableDepthBuffer(IDirect3DSurface9 *DepthS = NULL, IDirect3DTexture9 *DepthT = NULL);
bool ResolveDepthBuffer(IDirect3DDevice9 *Device);
IDirect3DSurface9 *GetStencilSurface();
IDirect3DSurface9 *GetDepthBufferSurface();
IDirect3DTexture9 *GetDepthBufferTexture();
bool LostDepthBuffer(bool stage, void *parameters);

bool HasDepth(void);
bool IsRAWZ(void);
