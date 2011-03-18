
// Hook structure.
enum
{
	K32FN_LoadLibraryA = 0,
//	K32FN_LoadLibraryA = 1,
	K32FN_GetProcAddress = 1
};

SDLLHook K32Hook =
{
	"kernel32.dll", NULL,
	false, NULL,		// Default hook disabled, NULL function pointer.
	{
		{ "LoadLibraryA", OBGELoadLibraryA},	// ANSI
	//	{ "LoadLibraryW", OBGELoadLibraryW},	// Unicode
		{ "GetProcAddress", OBGEGetProcAddress},
		{ NULL, NULL }
	}
};

// Hook function.
HMODULE WINAPI OBGELoadLibraryA(LPCTSTR dllName)
{
	LoadLibraryA_t old_func = (LoadLibraryA_t) K32Hook.Functions[K32FN_LoadLibraryA].OrigFn;
	HMODULE hModDLL = old_func(dllName);

	// hook dyamically loaded D3D9
	if ( lstrcmpi( "D3D9.DLL", dllName ) == 0 ) {
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
