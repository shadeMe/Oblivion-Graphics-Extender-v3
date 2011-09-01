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

#include "RenderStateManagerHooks.h"
#include "obse_common/SafeWrite.h"

bool v1_2_416::NiDX9RenderStateEx::DisableFogOverride=false;

#pragma optimize ("",off)
void _declspec(naked) v1_2_416::NiDX9RenderStateEx::SetRenderStateOld(D3DRENDERSTATETYPE state, UInt32 value, BOOL BackUp)
{
	_asm
	{
		mov eax,0x0077B000
		jmp eax
	}
}
#pragma optimize ("",on)

void v1_2_416::NiDX9RenderStateEx::SetRenderStateNew(D3DRENDERSTATETYPE state, UInt32 value, BOOL BackUp)
{
	if(state==D3DRS_FILLMODE)
	{
		//_MESSAGE("Fog override.");
		if(DisableFogOverride)
		{
			value=D3DFILL_WIREFRAME;
		}
	}
	SetRenderStateOld(state,value,BackUp);
}

void v1_2_416::NiDX9RenderStateEx::HookRenderStateManager()
{
// Hook SetFog
	void (v1_2_416::NiDX9RenderStateEx::*temp)(D3DRENDERSTATETYPE, UInt32, BOOL)=&v1_2_416::NiDX9RenderStateEx::SetRenderStateNew;
	UInt32	*temp2=(UInt32 *)&temp;
	_MESSAGE("Hooking set render state.");

	SafeWrite32(0x00A8AA58,*temp2);
}