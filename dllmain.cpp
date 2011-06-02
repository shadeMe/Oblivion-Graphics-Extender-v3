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

	if (EffectManager::Singleton) delete EffectManager::Singleton;
	if (TextureManager::Singleton) delete TextureManager::Singleton;
	if (ShaderManager::Singleton) delete ShaderManager::Singleton;

	if (lastOBGEDirect3DDevice9) delete lastOBGEDirect3DDevice9;
	if (lastOBGEDirect3D9) delete lastOBGEDirect3D9;
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
