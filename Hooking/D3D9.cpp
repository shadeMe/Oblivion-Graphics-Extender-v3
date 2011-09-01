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

#ifndef	HOOKING_D3D9_CPP
#define	HOOKING_D3D9_CPP

#include "D3D9.hpp"
#include "../D3D9.hpp"

// Hook structure.
enum
{
	D3DFN_Direct3DCreate9 = 0
};

SDLLHook D3DHook =
{
	"D3D9.DLL", NULL,
	false, NULL,		// Default hook disabled, NULL function pointer.
	{
		{ "Direct3DCreate9", OBGEDirect3DCreate9},
		{ NULL, NULL }
	}
};

// Hook function.
IDirect3D9* FAR WINAPI OBGEDirect3DCreate9(UINT sdk_version)
{
	Direct3DCreate9_t old_func = (Direct3DCreate9_t) D3DHook.Functions[D3DFN_Direct3DCreate9].OrigFn;
	IDirect3D9* d3d = old_func(sdk_version);

	_MESSAGE("OD3D9: Driver queried from 0x%08x", _ReturnAddress());

	return d3d ? new OBGEDirect3D9(d3d) : 0;
}

// Hook function.
//.text:00763E1F	push    esi
//.text:00763E20	push    20h
//.text:00763E22	call    eax ; hLibModule_D3D9_Direct3DCreate9
//.text:00763E24	mov     esi, eax
//.text:00763E26	test    esi, esi
//.text:00763E28        jz      short loc_763E6B

UInt32 backOD3Da = 0x00763E26;

#pragma optimize ("",off)
void _declspec(naked) OD3Da() {
  _asm {
    push    20h

    cmp	    DWORD PTR [D3DHook.hMod], 0UL
    jnz	    _skip
    mov	    D3DHook.Functions[D3DFN_Direct3DCreate9].OrigFn, eax
    mov	    eax, OBGEDirect3DCreate9
_skip:

    call    eax ; hLibModule_D3D9_Direct3DCreate9
    mov     esi, eax
    jmp	    [backOD3Da]
  }
}
#pragma optimize ("",on)

UInt32 forwOD3Da = (UInt32)&OD3Da;

//.text:00761E2D	push    20h
//.text:00761E2F	call    eax ; hLibModule_D3D9_Direct3DCreate9
//.text:00761E31	test    eax, eax
//.text:00761E33	mov     IDirect3D9_Instance, eax
//.text:00761E38	jnz     short loc_761E4C

UInt32 backOD3Db = 0x00761E33;

#pragma optimize ("",off)
void _declspec(naked) OD3Db() {
  _asm {
    push    20h

    cmp	    DWORD PTR [D3DHook.hMod], 0UL
    jnz	    _skip
    mov	    D3DHook.Functions[D3DFN_Direct3DCreate9].OrigFn, eax
    mov	    eax, OBGEDirect3DCreate9
_skip:

    call    eax ; hLibModule_D3D9_Direct3DCreate9
    test    eax, eax
    jmp	    [backOD3Db]
  }
}
#pragma optimize ("",on)

UInt32 forwOD3Db = (UInt32)&OD3Db;

void OBGEDirect3DCreate9Hook()
{
  SafeWrite16(0x00763E20, 0x25FF);  // jmp
  SafeWrite32(0x00763E22, (UInt32)&forwOD3Da);

  SafeWrite16(0x00761E2D, 0x25FF);  // jmp
  SafeWrite32(0x00761E2F, (UInt32)&forwOD3Db);
}

#endif
