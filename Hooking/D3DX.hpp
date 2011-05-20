#ifndef	HOOKING_D3DX_HPP
#define	HOOKING_D3DX_HPP

#include <d3dx9.h>

/* ----------------------------------------------------------------------------- */
// Tracker
extern IDirect3DBaseTexture9 *lastOBGEDirect3DBaseTexture9;

// Function pointer types.
typedef HRESULT (FAR WINAPI *D3DXCreateTextureFromFileA_t)(LPDIRECT3DDEVICE9 pDevice,
							   LPCTSTR pSrcFile,
							   LPDIRECT3DTEXTURE9 *ppTexture);
typedef HRESULT (FAR WINAPI *D3DXCreateTextureFromFileExA_t)(LPDIRECT3DDEVICE9 pDevice,
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
							     LPDIRECT3DTEXTURE9 *ppTexture);
typedef HRESULT (FAR WINAPI *D3DXCreateTextureFromFileInMemory_t)(LPDIRECT3DDEVICE9 pDevice,
								   LPCVOID pSrcData,
								   UINT SrcDataSize,
								   LPDIRECT3DTEXTURE9 *ppTexture);
typedef HRESULT (FAR WINAPI *D3DXCreateTextureFromFileInMemoryEx_t)(LPDIRECT3DDEVICE9 pDevice,
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
								     LPDIRECT3DTEXTURE9 *ppTexture);

// Function prototypes.
HRESULT FAR WINAPI OBGECreateTextureFromFileA(LPDIRECT3DDEVICE9 pDevice,
					      LPCTSTR pSrcFile,
					      LPDIRECT3DTEXTURE9 *ppTexture);
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
						LPDIRECT3DTEXTURE9 *ppTexture);
HRESULT FAR WINAPI OBGECreateTextureFromFileInMemory(LPDIRECT3DDEVICE9 pDevice,
						      LPCVOID pSrcData,
						      UINT SrcDataSize,
						      LPDIRECT3DTEXTURE9 *ppTexture);
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
							LPDIRECT3DTEXTURE9 *ppTexture);

/* ----------------------------------------------------------------------------- */

// Function pointer types.
typedef HRESULT (FAR WINAPI *D3DXCreateCubeTextureFromFileA_t)(LPDIRECT3DDEVICE9 pDevice,
							   LPCTSTR pSrcFile,
							   LPDIRECT3DTEXTURE9 *ppCubeTexture);
typedef HRESULT (FAR WINAPI *D3DXCreateCubeTextureFromFileExA_t)(LPDIRECT3DDEVICE9 pDevice,
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
							     LPDIRECT3DTEXTURE9 *ppCubeTexture);
typedef HRESULT (FAR WINAPI *D3DXCreateCubeTextureFromFileInMemory_t)(LPDIRECT3DDEVICE9 pDevice,
								   LPCVOID pSrcData,
								   UINT SrcDataSize,
								   LPDIRECT3DTEXTURE9 *ppCubeTexture);
typedef HRESULT (FAR WINAPI *D3DXCreateCubeTextureFromFileInMemoryEx_t)(LPDIRECT3DDEVICE9 pDevice,
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
								     LPDIRECT3DTEXTURE9 *ppCubeTexture);

// Function prototypes.
HRESULT FAR WINAPI OBGECreateCubeTextureFromFileA(LPDIRECT3DDEVICE9 pDevice,
					      LPCTSTR pSrcFile,
					      LPDIRECT3DTEXTURE9 *ppCubeTexture);
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
						LPDIRECT3DTEXTURE9 *ppCubeTexture);
HRESULT FAR WINAPI OBGECreateCubeTextureFromFileInMemory(LPDIRECT3DDEVICE9 pDevice,
						      LPCVOID pSrcData,
						      UINT SrcDataSize,
						      LPDIRECT3DTEXTURE9 *ppCubeTexture);
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
							LPDIRECT3DTEXTURE9 *ppCubeTexture);

/* ----------------------------------------------------------------------------- */

// Function pointer types.
typedef HRESULT (FAR WINAPI *D3DXCreateVolumeTextureFromFileA_t)(LPDIRECT3DDEVICE9 pDevice,
							   LPCTSTR pSrcFile,
							   LPDIRECT3DTEXTURE9 *ppVolumeTexture);
typedef HRESULT (FAR WINAPI *D3DXCreateVolumeTextureFromFileExA_t)(LPDIRECT3DDEVICE9 pDevice,
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
							     LPDIRECT3DTEXTURE9 *ppVolumeTexture);
typedef HRESULT (FAR WINAPI *D3DXCreateVolumeTextureFromFileInMemory_t)(LPDIRECT3DDEVICE9 pDevice,
								   LPCVOID pSrcData,
								   UINT SrcDataSize,
								   LPDIRECT3DTEXTURE9 *ppVolumeTexture);
typedef HRESULT (FAR WINAPI *D3DXCreateVolumeTextureFromFileInMemoryEx_t)(LPDIRECT3DDEVICE9 pDevice,
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
								     LPDIRECT3DTEXTURE9 *ppVolumeTexture);

// Function prototypes.
HRESULT FAR WINAPI OBGECreateVolumeTextureFromFileA(LPDIRECT3DDEVICE9 pDevice,
					      LPCTSTR pSrcFile,
					      LPDIRECT3DTEXTURE9 *ppVolumeTexture);
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
						LPDIRECT3DTEXTURE9 *ppVolumeTexture);
HRESULT FAR WINAPI OBGECreateVolumeTextureFromFileInMemory(LPDIRECT3DDEVICE9 pDevice,
						      LPCVOID pSrcData,
						      UINT SrcDataSize,
						      LPDIRECT3DTEXTURE9 *ppVolumeTexture);
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
							LPDIRECT3DTEXTURE9 *ppVolumeTexture);

#endif
