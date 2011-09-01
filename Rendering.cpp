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

#include "Rendering.h"
#include "nodes\NiDX9Renderer.h"

#pragma optimize ("",off)
_declspec(naked) IDirect3DDevice9 *GetD3DDevice(void)
{
	_asm
	{
		mov eax,0x00B3F928
		mov eax,[eax]
		mov eax,[eax+0x280]
		retn
	}
}
#pragma optimize("",on)

IDirect3DDevice9 *GetD3DDeviceNew(void)
{
	v1_2_416::NiDX9Renderer *pRenderer = NULL;

	pRenderer = v1_2_416::GetRenderer();
	//_MESSAGE("Renderer = %x",pRenderer);
	//_MESSAGE("Renderer Debug Info = %s",pRenderer->GetDeviceDebugInfo());

	return(pRenderer->pDirect3DDevice);
}

void D3DClearTest()
{
	GetD3DDevice()->Clear(0,NULL,D3DCLEAR_TARGET,0x00FF0000,0.0,0);
}

void CopyScreen(IDirect3DSurface9 *SourceSurface)
{
	IDirect3DSurface9 *RenderTarget;

	GetD3DDevice()->GetRenderTarget(0,&RenderTarget);	// Adds ref need to release when finished with.
	GetD3DDevice()->StretchRect(SourceSurface,NULL,RenderTarget,NULL,D3DTEXF_POINT);
	RenderTarget->Release();
}