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

#ifndef	OBGE_NOSHADER

#include <assert.h>
#include <windows.h>

#include "ShaderManager.h"
#include "ShaderIOHook.hpp"
#include "obse_common/SafeWrite.h"
#include "GlobalSettings.h"

#include "D3D9.hpp"

#include "Hooking/detours/detours.h"

static ShaderManager *sm = NULL;
static ShaderRecord *sb = NULL;

/* ------------------------------------------------------------------------------------------------- */

class Anonymous {
public:
//	void ReplaceShaderBinary(char *unk1, void **unk2);
	void *ReplaceShaderBinary(char *unk1);
};

void (__thiscall Anonymous::* FindShaderBinary)(char *, void **)/* =
	(void (__stdcall *)(char *, void **))0x0055E000*/;
void * (__thiscall Anonymous::* GetShaderBinary)(char *)/* =
	(void * (__stdcall *)(char *))0x007DAC70*/;
void * (__thiscall Anonymous::* ReplaceShaderBinary)(char *)/* =
	(void * (__stdcall *)(char *))0x007DAC70*/;

void *Anonymous::ReplaceShaderBinary(char *name) {
	unsigned char *binO, *bin;

	binO = bin = (unsigned char *)(this->*GetShaderBinary)(name);
//	(this->*FindShaderBinary)(name, (void *)&bin);

	if (sm && (sb = sm->GetBuiltInShader(name))) {
		if (binO) {
			/* internal directory-entry offsets */
#define	DIR_SIZE	0x100
#define	DIR_DATA	0x104
			int size = *((int *)(binO + DIR_SIZE));
			void *data = ((void *)(binO + DIR_DATA));

			sb->SetBinary(size, (const DWORD *)data);
		}

		if ((bin = (unsigned char *)sb->GetBinary())) {
			_DMESSAGE("Hooked built-in shader %s.", name);
			bin = bin - DIR_DATA;
		}
		else if (binO) {
			_DMESSAGE("Unchanged built-in shader %s.", name);
			bin = binO;
		}
		else {
			_DMESSAGE("No built-in shader %s exist, but may be provided", name);
			bin = NULL;
		}
	}

//	_MESSAGE("Shader load: %s", name);

	return bin;
}

/* ------------------------------------------------------------------------------------------------- */

void CreateShaderIOHook(void) {
	if (!(sm = ShaderManager::GetSingleton()))
		return;

	/* GetShaderBinary */
	*((int *)&FindShaderBinary) = 0x0055E000;
	*((int *)&GetShaderBinary) = 0x007DAC70;
	ReplaceShaderBinary = &Anonymous::ReplaceShaderBinary;

        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());
	DetourAttach(&(PVOID&)GetShaderBinary, *((PVOID *)&ReplaceShaderBinary));
        LONG error = DetourTransactionCommit();

        if (error == NO_ERROR)
		_MESSAGE("Detoured GetShaderBinary(); succeeded");
        else
		_MESSAGE("Detoured GetShaderBinary(); failed");

	return;
}

#endif
