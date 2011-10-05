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

#include <Windows.h>
//#include <vld.h>
#include "Hooking/detours/detoured.h"

#include "D3D9.hpp"
#include "D3D9Device.hpp"
#include "GUIs_DebugWindow.hpp"

#include "ShaderManager.h"
#include "TextureManager.h"
#include "EffectManager.h"

void GlobalCleanup() {
	/* this is strange, none of the destructors has been called
	 * but we can't free any resources ...
	 */
	return;

#ifdef	OBGE_DEVLING
	DebugWindow::Exit();
#endif

	if ( EffectManager::Singleton) delete  EffectManager::Singleton;
	if (TextureManager::Singleton) delete TextureManager::Singleton;
	if ( ShaderManager::Singleton) delete  ShaderManager::Singleton;

#ifndef	OBGE_NOSHADER
	if (lastOBGEDirect3DDevice9) delete lastOBGEDirect3DDevice9;
	if (lastOBGEDirect3D9      ) delete lastOBGEDirect3D9;
#endif
}

static HMODULE s_hDll;

HMODULE WINAPI Detoured()
{
    return s_hDll;
}

extern "C" {

BOOL WINAPI DllMain(
        HMODULE hDllHandle,
        DWORD   dwReason,
        LPVOID  lpreserved
        )
{
	if (dwReason == DLL_PROCESS_ATTACH) {
		s_hDll = hDllHandle;
		DisableThreadLibraryCalls(hDllHandle);
	}
	else if (dwReason == DLL_PROCESS_DETACH) {
		GlobalCleanup();
	}

	return TRUE;
}

};
