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

#ifndef	HOOKING_K32_CPP
#define	HOOKING_K32_CPP

#include "K32.hpp"

// Hook structure.
enum
{
	K32FN_LoadLibraryA = 0,
	K32FN_LoadLibraryW = 1,
	K32FN_GetProcAddress = 2
};

SDLLHook K32Hook =
{
	"kernel32.dll", NULL,
	false, NULL,		// Default hook disabled, NULL function pointer.
	{
		{ "LoadLibraryA", OBGELoadLibraryA},	// ANSI
		{ "LoadLibraryW", OBGELoadLibraryW},	// Unicode
		{ "GetProcAddress", OBGEGetProcAddress},
		{ NULL, NULL }
	}
};

// Hook function.
HMODULE WINAPI OBGELoadLibraryA(LPCTSTR dllName)
{
	LoadLibraryA_t old_func = (LoadLibraryA_t) K32Hook.Functions[K32FN_LoadLibraryA].OrigFn;
	HMODULE hModDLL = old_func(dllName);

	_MESSAGE("Init: %s queried.", dllName);

	// hook dyamically loaded D3D9
	if ( lstrcmpiA( "D3D9.DLL", dllName ) == 0 ) {
		D3DHook.hMod = hModDLL;

		_MESSAGE("Init: %s loaded.", dllName);

//		HookAPICalls(&D3DHook);
	}
	else if ((strstr("d3d9.dll", dllName) ||
		  strstr("D3D9.dll", dllName) ||
		  strstr("d3d9.DLL", dllName) ||
		  strstr("D3D9.DLL", dllName)) && !D3DHook.hMod) {
		D3DHook.hMod = hModDLL;

		_MESSAGE("Init: %s loaded.", dllName);

//		HookAPICalls(&D3DHook);
	}

	return hModDLL;
}

// Hook function.
HMODULE WINAPI OBGELoadLibraryW(LPCWSTR dllName)
{
	LoadLibraryW_t old_func = (LoadLibraryW_t) K32Hook.Functions[K32FN_LoadLibraryW].OrigFn;
	HMODULE hModDLL = old_func(dllName);

	_MESSAGE("Init: %s queried.", dllName);

	// hook dyamically loaded D3D9
	if ( lstrcmpiW( L"D3D9.DLL", dllName ) == 0 ) {
		D3DHook.hMod = hModDLL;

		_MESSAGE("Init: %s loaded.", dllName);

//		HookAPICalls(&D3DHook);
	}

	return hModDLL;
}

// Hook function.
FARPROC WINAPI OBGEGetProcAddress(HMODULE hModule, LPCSTR lpProcName)
{
	GetProcAddress_t old_func = (GetProcAddress_t) K32Hook.Functions[K32FN_GetProcAddress].OrigFn;
	FARPROC jmp = old_func(hModule, lpProcName);

	// hook dyamically loaded D3D9
	if ( D3DHook.hMod == hModule ) {
		FARPROC red;

		if ( ( red = (FARPROC)RedirectPA( &D3DHook, lpProcName, jmp ) ) ) {
			_MESSAGE("Init: Took over %s.", lpProcName);

			return red;
		}

//		HookAPICalls(&D3DHook);
	}

	return jmp;
}

#endif
