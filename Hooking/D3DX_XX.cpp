
extern SDLLHook D3XHook;

/* ----------------------------------------------------------------------------- */

// Hook function.
HRESULT FAR WINAPI OBGESaveTextureToFileA(LPCTSTR pDestFile,
					  D3DXIMAGE_FILEFORMAT DestFormat,
					  LPDIRECT3DBASETEXTURE9 pSrcTexture,
					  const PALETTEENTRY *pSrcPalette)
{
  D3DXSaveTextureToFileA_t old_func = (D3DXSaveTextureToFileA_t) D3XHook.Functions[D3XFN_SaveTextureToFileA].OrigFn;
//_MESSAGE("OD3DX: SaveTextureToFile(\"%s\") queried from 0x%08x", pDestFile, _ReturnAddress());
  HRESULT res = old_func(pDestFile, DestFormat, pSrcTexture, pSrcPalette);

  if (res != D3D_OK)
    res = OBGESaveBC45TextureToFile(pDestFile, DestFormat, pSrcTexture, pSrcPalette);

  return res;
}

/* ----------------------------------------------------------------------------- */

// Hook function.
HRESULT FAR WINAPI OBGECreateTextureFromFileA(LPDIRECT3DDEVICE9 pDevice,
					      LPCTSTR pSrcFile,
					      LPDIRECT3DTEXTURE9 *ppTexture)
{
  D3DXCreateTextureFromFileA_t old_func = (D3DXCreateTextureFromFileA_t) D3XHook.Functions[D3XFN_CreateTextureFromFileA].OrigFn;
//_MESSAGE("OD3DX: CreateTextureFromFile(\"%s\") queried from 0x%08x", pSrcFile, _ReturnAddress());
  HRESULT res = old_func(pDevice, pSrcFile, ppTexture);

  if (res != D3D_OK)
    res = OBGECreateBC45TextureFromFile(pDevice, pSrcFile, ppTexture);

  lastOBGEDirect3DBaseTexture9 = *ppTexture;

  return res;
}

// Hook function.
HRESULT FAR WINAPI OBGECreateTextureFromFileExA(LPDIRECT3DDEVICE9 pDevice,
						LPCTSTR pSrcFile,
						UINT Width,
						UINT Height,
						UINT MipLevels,
						DWORD Usage,
						D3DFORMAT Format,
						D3DPOOL Pool,
						DWORD Filter,
						DWORD MipFilter,
						D3DCOLOR ColorKey,
						D3DXIMAGE_INFO *pSrcInfo,
						PALETTEENTRY *pPalette,
						LPDIRECT3DTEXTURE9 *ppTexture)
{
  D3DXCreateTextureFromFileExA_t old_func = (D3DXCreateTextureFromFileExA_t) D3XHook.Functions[D3XFN_CreateTextureFromFileExA].OrigFn;
//_MESSAGE("OD3DX: CreateTextureFromFileEx(\"%s\") queried from 0x%08x", pSrcFile, _ReturnAddress());
  HRESULT res = old_func(pDevice, pSrcFile, Width, Height, MipLevels, Usage, Format, Pool, Filter, MipFilter, ColorKey, pSrcInfo, pPalette, ppTexture);

  if (res != D3D_OK)
    res = OBGECreateBC45TextureFromFileEx(pDevice, pSrcFile, Width, Height, MipLevels, Usage, Format, Pool, Filter, MipFilter, ColorKey, pSrcInfo, pPalette, ppTexture);

  lastOBGEDirect3DBaseTexture9 = *ppTexture;

  return res;
}

// Hook function.
HRESULT FAR WINAPI OBGECreateTextureFromFileInMemory(LPDIRECT3DDEVICE9 pDevice,
						      LPCVOID pSrcData,
						      UINT SrcDataSize,
						      LPDIRECT3DTEXTURE9 *ppTexture)
{
  D3DXCreateTextureFromFileInMemory_t old_func = (D3DXCreateTextureFromFileInMemory_t) D3XHook.Functions[D3XFN_CreateTextureFromFileInMemory].OrigFn;
//_MESSAGE("OD3DX: CreateTextureFromFileInMemory(\"%d\") queried from 0x%08x", SrcDataSize, _ReturnAddress());
  HRESULT res = old_func(pDevice, pSrcData, SrcDataSize, ppTexture);

  if (res != D3D_OK)
    res = OBGECreateBC45TextureFromFileInMemory(pDevice, pSrcData, SrcDataSize, ppTexture);

  lastOBGEDirect3DBaseTexture9 = *ppTexture;

  return res;
}

// Hook function.
HRESULT FAR WINAPI OBGECreateTextureFromFileInMemoryEx(LPDIRECT3DDEVICE9 pDevice,
							LPCVOID pSrcData,
							UINT SrcDataSize,
							UINT Width,
							UINT Height,
							UINT MipLevels,
							DWORD Usage,
							D3DFORMAT Format,
							D3DPOOL Pool,
							DWORD Filter,
							DWORD MipFilter,
							D3DCOLOR ColorKey,
							D3DXIMAGE_INFO *pSrcInfo,
							PALETTEENTRY *pPalette,
							LPDIRECT3DTEXTURE9 *ppTexture)
{
  D3DXCreateTextureFromFileInMemoryEx_t old_func = (D3DXCreateTextureFromFileInMemoryEx_t) D3XHook.Functions[D3XFN_CreateTextureFromFileInMemoryEx].OrigFn;
//_MESSAGE("OD3DX: CreateTextureFromFileInMemoryEx(\"%d\") queried from 0x%08x", SrcDataSize, _ReturnAddress());
  HRESULT res = old_func(pDevice, pSrcData, SrcDataSize, Width, Height, MipLevels, Usage, Format, Pool, Filter, MipFilter, ColorKey, pSrcInfo, pPalette, ppTexture);

  if (res != D3D_OK)
    res = OBGECreateBC45TextureFromFileInMemoryEx(pDevice, pSrcData, SrcDataSize, Width, Height, MipLevels, Usage, Format, Pool, Filter, MipFilter, ColorKey, pSrcInfo, pPalette, ppTexture);

  lastOBGEDirect3DBaseTexture9 = *ppTexture;

  return res;
}

/* ----------------------------------------------------------------------------- */

// Hook function.
HRESULT FAR WINAPI OBGECreateCubeTextureFromFileA(LPDIRECT3DDEVICE9 pDevice,
						  LPCTSTR pSrcFile,
						  LPDIRECT3DTEXTURE9 *ppCubeTexture)
{
  D3DXCreateCubeTextureFromFileA_t old_func = (D3DXCreateCubeTextureFromFileA_t) D3XHook.Functions[D3XFN_CreateCubeTextureFromFileA].OrigFn;
  HRESULT res = old_func(pDevice, pSrcFile, ppCubeTexture);

//_MESSAGE("OD3DX: CreateCubeTextureFromFile(\"%s\") queried from 0x%08x", pSrcFile, _ReturnAddress());
  lastOBGEDirect3DBaseTexture9 = *ppCubeTexture;

  return res;
}

// Hook function.
HRESULT FAR WINAPI OBGECreateCubeTextureFromFileExA(LPDIRECT3DDEVICE9 pDevice,
						    LPCTSTR pSrcFile,
						    UINT Size,
						    UINT MipLevels,
						    DWORD Usage,
						    D3DFORMAT Format,
						    D3DPOOL Pool,
						    DWORD Filter,
						    DWORD MipFilter,
						    D3DCOLOR ColorKey,
						    D3DXIMAGE_INFO *pSrcInfo,
						    PALETTEENTRY *pPalette,
						    LPDIRECT3DTEXTURE9 *ppCubeTexture)
{
  D3DXCreateCubeTextureFromFileExA_t old_func = (D3DXCreateCubeTextureFromFileExA_t) D3XHook.Functions[D3XFN_CreateCubeTextureFromFileExA].OrigFn;
  HRESULT res = old_func(pDevice, pSrcFile, Size, MipLevels, Usage, Format, Pool, Filter, MipFilter, ColorKey, pSrcInfo, pPalette, ppCubeTexture);

//_MESSAGE("OD3DX: CreateCubeTextureFromFileEx(\"%s\") queried from 0x%08x", pSrcFile, _ReturnAddress());
  lastOBGEDirect3DBaseTexture9 = *ppCubeTexture;

  return res;
}

// Hook function.
HRESULT FAR WINAPI OBGECreateCubeTextureFromFileInMemory(LPDIRECT3DDEVICE9 pDevice,
							 LPCVOID pSrcData,
							 UINT SrcDataSize,
							 LPDIRECT3DTEXTURE9 *ppCubeTexture)
{
  D3DXCreateCubeTextureFromFileInMemory_t old_func = (D3DXCreateCubeTextureFromFileInMemory_t) D3XHook.Functions[D3XFN_CreateCubeTextureFromFileInMemory].OrigFn;
  HRESULT res = old_func(pDevice, pSrcData, SrcDataSize, ppCubeTexture);

//_MESSAGE("OD3DX: CreateCubeTextureFromFileInMemory(\"%d\") queried from 0x%08x", SrcDataSize, _ReturnAddress());
  lastOBGEDirect3DBaseTexture9 = *ppCubeTexture;

  return res;
}

// Hook function.
HRESULT FAR WINAPI OBGECreateCubeTextureFromFileInMemoryEx(LPDIRECT3DDEVICE9 pDevice,
							LPCVOID pSrcData,
							UINT SrcDataSize,
							UINT Size,
							UINT MipLevels,
							DWORD Usage,
							D3DFORMAT Format,
							D3DPOOL Pool,
							DWORD Filter,
							DWORD MipFilter,
							D3DCOLOR ColorKey,
							D3DXIMAGE_INFO *pSrcInfo,
							PALETTEENTRY *pPalette,
							LPDIRECT3DTEXTURE9 *ppCubeTexture)
{
  D3DXCreateCubeTextureFromFileInMemoryEx_t old_func = (D3DXCreateCubeTextureFromFileInMemoryEx_t) D3XHook.Functions[D3XFN_CreateCubeTextureFromFileInMemoryEx].OrigFn;
  HRESULT res = old_func(pDevice, pSrcData, SrcDataSize, Size, MipLevels, Usage, Format, Pool, Filter, MipFilter, ColorKey, pSrcInfo, pPalette, ppCubeTexture);

//_MESSAGE("OD3DX: CreateCubeTextureFromFileInMemoryEx(\"%d\") queried from 0x%08x", SrcDataSize, _ReturnAddress());
  lastOBGEDirect3DBaseTexture9 = *ppCubeTexture;

  return res;
}

/* ----------------------------------------------------------------------------- */

// Hook function.
HRESULT FAR WINAPI OBGECreateVolumeTextureFromFileA(LPDIRECT3DDEVICE9 pDevice,
						    LPCTSTR pSrcFile,
						    LPDIRECT3DTEXTURE9 *ppVolumeTexture)
{
  D3DXCreateVolumeTextureFromFileA_t old_func = (D3DXCreateVolumeTextureFromFileA_t) D3XHook.Functions[D3XFN_CreateVolumeTextureFromFileA].OrigFn;
  HRESULT res = old_func(pDevice, pSrcFile, ppVolumeTexture);

//_MESSAGE("OD3DX: CreateVolumeTextureFromFile(\"%s\") queried from 0x%08x", pSrcFile, _ReturnAddress());
  lastOBGEDirect3DBaseTexture9 = *ppVolumeTexture;

  return res;
}

// Hook function.
HRESULT FAR WINAPI OBGECreateVolumeTextureFromFileExA(LPDIRECT3DDEVICE9 pDevice,
						LPCTSTR pSrcFile,
						UINT Width,
						UINT Height,
						UINT Depth,
						UINT MipLevels,
						DWORD Usage,
						D3DFORMAT Format,
						D3DPOOL Pool,
						DWORD Filter,
						DWORD MipFilter,
						D3DCOLOR ColorKey,
						D3DXIMAGE_INFO *pSrcInfo,
						PALETTEENTRY *pPalette,
						LPDIRECT3DTEXTURE9 *ppVolumeTexture)
{
  D3DXCreateVolumeTextureFromFileExA_t old_func = (D3DXCreateVolumeTextureFromFileExA_t) D3XHook.Functions[D3XFN_CreateVolumeTextureFromFileExA].OrigFn;
  HRESULT res = old_func(pDevice, pSrcFile, Width, Height, Depth, MipLevels, Usage, Format, Pool, Filter, MipFilter, ColorKey, pSrcInfo, pPalette, ppVolumeTexture);

//_MESSAGE("OD3DX: CreateVolumeTextureFromFileEx(\"%s\") queried from 0x%08x", pSrcFile, _ReturnAddress());
  lastOBGEDirect3DBaseTexture9 = *ppVolumeTexture;

  return res;
}

// Hook function.
HRESULT FAR WINAPI OBGECreateVolumeTextureFromFileInMemory(LPDIRECT3DDEVICE9 pDevice,
						      LPCVOID pSrcData,
						      UINT SrcDataSize,
						      LPDIRECT3DTEXTURE9 *ppVolumeTexture)
{
  D3DXCreateVolumeTextureFromFileInMemory_t old_func = (D3DXCreateVolumeTextureFromFileInMemory_t) D3XHook.Functions[D3XFN_CreateVolumeTextureFromFileInMemory].OrigFn;
  HRESULT res = old_func(pDevice, pSrcData, SrcDataSize, ppVolumeTexture);

//_MESSAGE("OD3DX: CreateVolumeTextureFromFileInMemory(\"%d\") queried from 0x%08x", SrcDataSize, _ReturnAddress());
  lastOBGEDirect3DBaseTexture9 = *ppVolumeTexture;

  return res;
}

// Hook function.
HRESULT FAR WINAPI OBGECreateVolumeTextureFromFileInMemoryEx(LPDIRECT3DDEVICE9 pDevice,
							LPCVOID pSrcData,
							UINT SrcDataSize,
							UINT Width,
							UINT Height,
							UINT Depth,
							UINT MipLevels,
							DWORD Usage,
							D3DFORMAT Format,
							D3DPOOL Pool,
							DWORD Filter,
							DWORD MipFilter,
							D3DCOLOR ColorKey,
							D3DXIMAGE_INFO *pSrcInfo,
							PALETTEENTRY *pPalette,
							LPDIRECT3DTEXTURE9 *ppVolumeTexture)
{
  D3DXCreateVolumeTextureFromFileInMemoryEx_t old_func = (D3DXCreateVolumeTextureFromFileInMemoryEx_t) D3XHook.Functions[D3XFN_CreateVolumeTextureFromFileInMemoryEx].OrigFn;
  HRESULT res = old_func(pDevice, pSrcData, SrcDataSize, Width, Height, Depth, MipLevels, Usage, Format, Pool, Filter, MipFilter, ColorKey, pSrcInfo, pPalette, ppVolumeTexture);

//_MESSAGE("OD3DX: CreateVolumeTextureFromFileInMemoryEx(\"%d\") queried from 0x%08x", SrcDataSize, _ReturnAddress());
  lastOBGEDirect3DBaseTexture9 = *ppVolumeTexture;

  return res;
}

/* ----------------------------------------------------------------------------- */

SDLLHook D3XHook =
{
  D3XHookDLL, NULL,
  false, NULL,		// Default hook disabled, NULL function pointer.
  {
    { "D3DXCreateTextureFromFileA", OBGECreateTextureFromFileA},
//  { "D3DXCreateTextureFromFileW", OBGECreateTextureFromFileW},
    { "D3DXCreateTextureFromFileExA", OBGECreateTextureFromFileExA},
//  { "D3DXCreateTextureFromFileExW", OBGECreateTextureFromFileExW},
    { "D3DXCreateTextureFromFileInMemory", OBGECreateTextureFromFileInMemory},
    { "D3DXCreateTextureFromFileInMemoryEx", OBGECreateTextureFromFileInMemoryEx},
    { "D3DXCreateCubeTextureFromFileA", OBGECreateCubeTextureFromFileA},
//  { "D3DXCreateCubeTextureFromFileW", OBGECreateCubeTextureFromFileW},
    { "D3DXCreateCubeTextureFromFileExA", OBGECreateCubeTextureFromFileExA},
//  { "D3DXCreateCubeTextureFromFileExW", OBGECreateCubeTextureFromFileExW},
    { "D3DXCreateCubeTextureFromFileInMemory", OBGECreateCubeTextureFromFileInMemory},
    { "D3DXCreateCubeTextureFromFileInMemoryEx", OBGECreateCubeTextureFromFileInMemoryEx},
    { "D3DXCreateVolumeTextureFromFileA", OBGECreateVolumeTextureFromFileA},
//  { "D3DXCreateVolumeTextureFromFileW", OBGECreateVolumeTextureFromFileW},
    { "D3DXCreateVolumeTextureFromFileExA", OBGECreateVolumeTextureFromFileExA},
//  { "D3DXCreateVolumeTextureFromFileExW", OBGECreateVolumeTextureFromFileExW},
    { "D3DXCreateVolumeTextureFromFileInMemory", OBGECreateVolumeTextureFromFileInMemory},
    { "D3DXCreateVolumeTextureFromFileInMemoryEx", OBGECreateVolumeTextureFromFileInMemoryEx},
    { "D3DXSaveTextureToFileA", OBGESaveTextureToFileA},
//  { "D3DXSaveTextureToFileW", OBGESaveTextureToFileW},
    { NULL, NULL }
  }
};

/* ----------------------------------------------------------------------------- */

#undef	D3XHook
#undef	D3XHookDLL

#undef	OBGESaveTextureToFileA
#undef	OBGESaveTextureToFileW
#undef	OBGECreateTextureFromFileA
#undef	OBGECreateTextureFromFileW
#undef	OBGECreateTextureFromFileExA
#undef	OBGECreateTextureFromFileExW
#undef	OBGECreateTextureFromFileInMemory
#undef	OBGECreateTextureFromFileInMemoryEx
#undef	OBGECreateCubeTextureFromFileA
#undef	OBGECreateCubeTextureFromFileW
#undef	OBGECreateCubeTextureFromFileExA
#undef	OBGECreateCubeTextureFromFileExW
#undef	OBGECreateCubeTextureFromFileInMemory
#undef	OBGECreateCubeTextureFromFileInMemoryEx
#undef	OBGECreateVolumeTextureFromFileA
#undef	OBGECreateVolumeTextureFromFileW
#undef	OBGECreateVolumeTextureFromFileExA
#undef	OBGECreateVolumeTextureFromFileExW
#undef	OBGECreateVolumeTextureFromFileInMemory
#undef	OBGECreateVolumeTextureFromFileInMemoryEx
