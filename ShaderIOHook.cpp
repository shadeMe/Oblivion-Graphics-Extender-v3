#include "D3D9.hpp"
#include "ShaderManager.h"
#include "ShaderIOHook.hpp"
#include "windows.h"
#include "obse_common/SafeWrite.h"
#include "GlobalSettings.h"
#include <assert.h>

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
		/* SLS1000.vso doesn't exist for example */
		if (binO) {
			/* internal directory-entry offsets */
#define	DIR_SIZE	0x100
#define	DIR_DATA	0x104
			int size = *((int *)(binO + DIR_SIZE));
			void *data = ((void *)(binO + DIR_DATA));

			sb->SetBinary(size, data);
		}

		if ((bin = (unsigned char *)sb->GetBinary())) {
			_MESSAGE("Replaced built-in shader %s.", name);
			bin = bin - DIR_DATA;
		}
		else if (binO) {
			_MESSAGE("Unchanged built-in shader %s.", name);
			bin = binO;
		}
		else {
			_MESSAGE("No built-in shader %s exist, but may be provided", name);
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

        if (error == NO_ERROR) {
		_MESSAGE("Detoured GetShaderBinary(); succeeded");
        }
        else {
		_MESSAGE("Detoured GetShaderBinary(); failed");
        }

	return;
}
