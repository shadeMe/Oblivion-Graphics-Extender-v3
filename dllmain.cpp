#include <Windows.h>
#include "detours/detoured.h"

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

	return TRUE;
}

};
