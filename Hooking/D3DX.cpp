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

#if	defined(OBGE_LOGGING) || defined(OBGE_DEVLING) || defined(OBGE_GAMMACORRECTION)
#ifndef	HOOKING_D3DX_CPP
#define	HOOKING_D3DX_CPP

#include <sys/stat.h>
#include <assert.h>
#include "D3DX.hpp"
#include "../D3DX.hpp"

// *********************************************************************************************************

static const DWORD DDSD_CAPS = 0x00000001U;
static const DWORD DDSD_PIXELFORMAT = 0x00001000U;
static const DWORD DDSD_WIDTH = 0x00000004U;
static const DWORD DDSD_HEIGHT = 0x00000002U;
static const DWORD DDSD_PITCH = 0x00000008U;
static const DWORD DDSD_MIPMAPCOUNT = 0x00020000U;
static const DWORD DDSD_LINEARSIZE = 0x00080000U;
static const DWORD DDSD_DEPTH = 0x00800000U;

static const DWORD DDSCAPS_COMPLEX = 0x00000008U;
static const DWORD DDSCAPS_TEXTURE = 0x00001000U;
static const DWORD DDSCAPS_MIPMAP = 0x00400000U;
static const DWORD DDSCAPS2_VOLUME = 0x00200000U;
static const DWORD DDSCAPS2_CUBEMAP = 0x00000200U;

static const DWORD DDSCAPS2_CUBEMAP_POSITIVEX = 0x00000400U;
static const DWORD DDSCAPS2_CUBEMAP_NEGATIVEX = 0x00000800U;
static const DWORD DDSCAPS2_CUBEMAP_POSITIVEY = 0x00001000U;
static const DWORD DDSCAPS2_CUBEMAP_NEGATIVEY = 0x00002000U;
static const DWORD DDSCAPS2_CUBEMAP_POSITIVEZ = 0x00004000U;
static const DWORD DDSCAPS2_CUBEMAP_NEGATIVEZ = 0x00008000U;
static const DWORD DDSCAPS2_CUBEMAP_ALL_FACES = 0x0000FC00U;

static const DWORD DDPF_ALPHAPIXELS = 0x00000001U;
static const DWORD DDPF_ALPHA = 0x00000002U;
static const DWORD DDPF_FOURCC = 0x00000004U;
static const DWORD DDPF_RGB = 0x00000040U;
static const DWORD DDPF_PALETTEINDEXED1 = 0x00000800U;
static const DWORD DDPF_PALETTEINDEXED2 = 0x00001000U;
static const DWORD DDPF_PALETTEINDEXED4 = 0x00000008U;
static const DWORD DDPF_PALETTEINDEXED8 = 0x00000020U;
static const DWORD DDPF_LUMINANCE = 0x00020000U;
static const DWORD DDPF_ALPHAPREMULT = 0x00008000U;
static const DWORD DDPF_NORMAL = 0x80000000U;	// @@ Custom nv flag.

struct DDS_PIXELFORMAT {
  DWORD dwSize;
  DWORD dwFlags;
  DWORD dwFourCC;
  DWORD dwRGBBitCount;
  DWORD dwRBitMask;
  DWORD dwGBitMask;
  DWORD dwBBitMask;
  DWORD dwABitMask;
};

typedef struct {
  DWORD dwMagicNumber;

  DWORD dwSize;
  DWORD dwHeaderFlags;
  DWORD dwHeight;
  DWORD dwWidth;
  DWORD dwPitchOrLinearSize;
  DWORD dwDepth; // only if DDS_HEADER_FLAGS_VOLUME is set in dwHeaderFlags
  DWORD dwMipMapCount;
  DWORD dwReserved1[11];
  DDS_PIXELFORMAT ddspf;
  DWORD dwSurfaceFlags;
  DWORD dwCubemapFlags;
  DWORD dwReserved2[3];
} DDS_HEADER;

#define FOURCC_ATI1N ((D3DFORMAT)MAKEFOURCC('A', 'T', 'I', '1'))
#define FOURCC_ATI2N ((D3DFORMAT)MAKEFOURCC('A', 'T', 'I', '2'))

static UINT BitsPerPixel(DWORD FourCC) {
  switch(FourCC) {
    case FOURCC_ATI1N:
    case FOURCC_ATI2N:
      return 8;

    default:
      assert( FALSE ); // unhandled format
      return 0;
  }
}

static void GetSurfaceInfo(UINT width, UINT height, DWORD FourCC, UINT *pNumBytes, UINT *pRowBytes, UINT *pNumRows) {
  UINT numBytes = 0;
  UINT rowBytes = 0;
  UINT numRows = 0;

  bool bc = true;
  int bcnumBytesPerBlock = 16;
  switch (FourCC) {
    case FOURCC_ATI1N:
      bcnumBytesPerBlock = 8;
      break;
    case FOURCC_ATI2N:
      bcnumBytesPerBlock = 16;
      break;
    default:
      bc = false;
      break;
  }

  if (bc) {
    int numBlocksWide = 0;
    if (width > 0)
      numBlocksWide = max(1, width / 4);
    int numBlocksHigh = 0;
    if (height > 0)
      numBlocksHigh = max(1, height / 4);

    rowBytes = numBlocksWide * bcnumBytesPerBlock;
    numRows = numBlocksHigh;
  }
  else {
    UINT bpp = BitsPerPixel(FourCC);
    rowBytes = (width * bpp + 7) / 8; // round up to nearest byte
    numRows = height;
  }

  numBytes = rowBytes * numRows;
  if (pNumBytes != NULL)
    *pNumBytes = numBytes;
  if (pRowBytes != NULL)
    *pRowBytes = rowBytes;
  if (pNumRows != NULL)
    *pNumRows = numRows;
}

// *********************************************************************************************************

static LPDIRECT3DTEXTURE9 LoadBC45TextureFromFileInMemory(LPDIRECT3DDEVICE9 pDevice, const void *data, int size, bool NONPOW2) {
  if ((((DDS_HEADER *)data)->ddspf.dwFourCC != FOURCC_ATI1N) &&
      (((DDS_HEADER *)data)->ddspf.dwFourCC != FOURCC_ATI2N))
    return NULL;

#if 0
  // Check if ATI1N is supported
  hr = lastOBGEDirect3D9->CheckDeviceFormat(AdapterOrdinal, DeviceType, AdapterFormat,
    0, D3DRTYPE_TEXTURE, FOURCC_ATI1N);
  BOOL bATI1NSupported = (hr == D3D_OK);
  • To check support for ATI2N:
  // Check if ATI2N is supported
  HRESULT hr;
  hr = lastOBGEDirect3D9->CheckDeviceFormat(AdapterOrdinal, DeviceType, AdapterFormat,
    0, D3DRTYPE_TEXTURE, FOURCC_ATI2N);
  BOOL bATI2NSupported = (hr == D3D_OK);
#endif

  LPDIRECT3DTEXTURE9 GPUTexture = NULL;
  HRESULT res;

  if (FAILED(res = pDevice->CreateTexture(
	((DDS_HEADER *)data)->dwWidth,
	((DDS_HEADER *)data)->dwHeight,
	((DDS_HEADER *)data)->dwMipMapCount,
	0, (D3DFORMAT)
	((DDS_HEADER *)data)->ddspf.dwFourCC,
	D3DPOOL_MANAGED,
	&GPUTexture,
	NULL)))
    return NULL;

  // Lock, fill, unlock
  D3DLOCKED_RECT LockedRect;
  UINT RowBytes, NumRows;
  BYTE *pSrcBits = (BYTE *)data + sizeof(DDS_HEADER);

  UINT iWidth = ((DDS_HEADER *)data)->dwWidth;
  UINT iHeight = ((DDS_HEADER *)data)->dwHeight;
  UINT iMipCount = ((DDS_HEADER *)data)->dwMipMapCount;
  DWORD iFourCC = ((DDS_HEADER *)data)->ddspf.dwFourCC;
  if (0 == iMipCount)
    iMipCount = 1;

  for (UINT i = 0; i < iMipCount; i++) {
    GetSurfaceInfo(iWidth, iHeight, iFourCC, NULL, &RowBytes, &NumRows);

    if (SUCCEEDED(GPUTexture->LockRect(i, &LockedRect, NULL, 0))) {
      BYTE *pDestBits = (BYTE *)LockedRect.pBits;

#if 1
      int size = (iWidth * iHeight * (iFourCC == FOURCC_ATI2N ? 2 : 1)) >> 1;
      CopyMemory(pDestBits, pSrcBits, size);
      pDestBits += size;
      pSrcBits += size;
#else
      // Copy stride line by line
      for (UINT h = 0; h < NumRows; h++) {
	CopyMemory(pDestBits, pSrcBits, RowBytes);

	pDestBits += LockedRect.Pitch;
	pSrcBits += RowBytes;
      }
#endif

      GPUTexture->UnlockRect(i);
    }

    iWidth  = iWidth  >> 1;
    iHeight = iHeight >> 1;

    if (iWidth == 0)
      iWidth = 1;
    if (iHeight == 0)
      iHeight = 1;
  }

  return GPUTexture;
}

static LPDIRECT3DTEXTURE9 LoadBC45TextureFromFile(LPDIRECT3DDEVICE9 pDevice, const char *fp, bool NONPOW2) {
  struct stat vf; FILE *f; void *data;
  if (stat((const char *)fp, &vf))
    return NULL;
  if (!(data = malloc(vf.st_size)))
    return NULL;
  if (fopen_s(&f, fp, "rb")) {
    free(data); return NULL; }

  fread(data, 1, vf.st_size, f);
  fclose(f);

  LPDIRECT3DTEXTURE9 tex = LoadBC45TextureFromFileInMemory(pDevice, data, vf.st_size, NONPOW2);

  free(data);

  return tex;
}

HRESULT FAR WINAPI OBGECreateBC45TextureFromFile(LPDIRECT3DDEVICE9 pDevice,
					      LPCTSTR pSrcFile,
					      LPDIRECT3DTEXTURE9 *ppTexture)
{
  if (1) {
      LPDIRECT3DTEXTURE9 tex = LoadBC45TextureFromFile(pDevice, pSrcFile, true);

      if (tex) {
	*ppTexture = tex;
	return D3D_OK;
      }
  }

  return S_FALSE;
}

HRESULT FAR WINAPI OBGECreateBC45TextureFromFileEx(LPDIRECT3DDEVICE9 pDevice,
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
  if ((Width  == D3DX_DEFAULT_NONPOW2) &&
      (Height == D3DX_DEFAULT_NONPOW2) &&
      (MipLevels == 0) &&
      (Pool == D3DPOOL_MANAGED) &&
      (Filter == D3DX_DEFAULT) &&
      (MipFilter == D3DX_DEFAULT)) {
    LPDIRECT3DTEXTURE9 tex = LoadBC45TextureFromFile(pDevice, pSrcFile, true);

    if (tex) {
      *ppTexture = tex;
      return D3D_OK;
    }
  }

  return S_FALSE;
}

HRESULT FAR WINAPI OBGECreateBC45TextureFromFileInMemory(LPDIRECT3DDEVICE9 pDevice,
							 LPCVOID pSrcData,
							 UINT SrcDataSize,
							 LPDIRECT3DTEXTURE9 *ppTexture)
{
  if (1) {
    LPDIRECT3DTEXTURE9 tex = LoadBC45TextureFromFileInMemory(pDevice, pSrcData, SrcDataSize, true);

    if (tex) {
      *ppTexture = tex;
      return D3D_OK;
    }
  }

  return S_FALSE;
}

HRESULT FAR WINAPI OBGECreateBC45TextureFromFileInMemoryEx(LPDIRECT3DDEVICE9 pDevice,
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
  if ((Width  == D3DX_DEFAULT_NONPOW2) &&
      (Height == D3DX_DEFAULT_NONPOW2) &&
      (MipLevels == 0) &&
      (Pool == D3DPOOL_MANAGED) &&
      (Filter == D3DX_DEFAULT) &&
      (MipFilter == D3DX_DEFAULT)) {
    LPDIRECT3DTEXTURE9 tex = LoadBC45TextureFromFileInMemory(pDevice, pSrcData, SrcDataSize, true);

    if (tex) {
      *ppTexture = tex;
      return D3D_OK;
    }
  }

  return S_FALSE;
}

// *********************************************************************************************************

// Hook function.
static bool SaveBC45TextureToFile(const char *fp, LPDIRECT3DBASETEXTURE9 pSrcTexture) {
  FILE *f;
  if (fopen_s(&f, fp, "wb"))
    return NULL;

  D3DSURFACE_DESC desc; int levels =

  ((IDirect3DTexture9 *)pSrcTexture)->GetLevelCount();
  ((IDirect3DTexture9 *)pSrcTexture)->GetLevelDesc(0, &desc);

  DDS_HEADER head; memset(&head, 0, sizeof(head));

  head.dwMagicNumber = MAKEFOURCC('D', 'D', 'S', ' ');
  head.dwSize = 0x7C;
  head.dwHeaderFlags =
	DDSD_CAPS |
	DDSD_WIDTH |
	DDSD_HEIGHT |
	DDSD_MIPMAPCOUNT |
	DDSD_LINEARSIZE |
	DDSD_PIXELFORMAT;
  head.dwWidth = desc.Width;
  head.dwHeight = desc.Height;
  head.dwMipMapCount = levels;
  head.dwPitchOrLinearSize = (desc.Width * desc.Height * (desc.Format == FOURCC_ATI2N ? 2 : 1)) >> 1;
  head.ddspf.dwFourCC = desc.Format;
  head.ddspf.dwFlags = DDPF_FOURCC;
  head.dwSurfaceFlags = (levels > 1 ? DDSCAPS_MIPMAP : 0) | DDSCAPS_TEXTURE;
  // (desc.Format == FOURCC_ATI2N ? DDPF_NORMAL : 0);

  fwrite(&head, 1, sizeof(head), f);

  // Lock, fill, unlock
  D3DLOCKED_RECT LockedRect;
  UINT RowBytes, NumRows;

  UINT iWidth = head.dwWidth;
  UINT iHeight = head.dwHeight;
  UINT iMipCount = head.dwMipMapCount;
  DWORD iFourCC = head.ddspf.dwFourCC;
  if (0 == iMipCount)
    iMipCount = 1;

  for (UINT i = 0; i < iMipCount; i++) {
    GetSurfaceInfo(iWidth, iHeight, iFourCC, NULL, &RowBytes, &NumRows);

    if (SUCCEEDED(((IDirect3DTexture9 *)pSrcTexture)->LockRect(i, &LockedRect, NULL, 0))) {
      BYTE *pSrcBits = (BYTE *)LockedRect.pBits;

#if 1
      int size = (iWidth * iHeight * (iFourCC == FOURCC_ATI2N ? 2 : 1)) >> 1;
      fwrite(pSrcBits, 1, size, f);
      pSrcBits += size;
#else
      // Write stride line by line
      for (UINT h = 0; h < NumRows; h++) {
        fwrite(pSrcBits, 1, RowBytes, f);

	pSrcBits += RowBytes;
      }
#endif

      ((IDirect3DTexture9 *)pSrcTexture)->UnlockRect(i);
    }

    iWidth  = iWidth  >> 1;
    iHeight = iHeight >> 1;

    if (iWidth == 0)
      iWidth = 1;
    if (iHeight == 0)
      iHeight = 1;
  }

  fclose(f);

  return true;
}

// Hook function.
HRESULT FAR WINAPI OBGESaveBC45TextureToFile(LPCTSTR pDestFile,
					  D3DXIMAGE_FILEFORMAT DestFormat,
					  LPDIRECT3DBASETEXTURE9 pSrcTexture,
					  const PALETTEENTRY *pSrcPalette)
{
  D3DSURFACE_DESC desc; ((IDirect3DTexture9 *)pSrcTexture)->GetLevelDesc(0, &desc);

  if ((DestFormat == D3DXIFF_DDS) &&
      (pSrcTexture->GetType() == D3DRTYPE_TEXTURE) &&
      ((desc.Format == FOURCC_ATI1N) ||
       (desc.Format == FOURCC_ATI2N))) {
    return SaveBC45TextureToFile(pDestFile, pSrcTexture) ? D3D_OK : S_FALSE;
  }

  return S_FALSE;
}

/* ----------------------------------------------------------------------------- */
// Tracker
IDirect3DBaseTexture9 *lastOBGEDirect3DBaseTexture9;

// Hook structure.
enum
{
	D3XFN_CreateTextureFromFileA = 0,
//	D3XFN_CreateTextureFromFileW = 0,
	D3XFN_CreateTextureFromFileExA = 1,
//	D3XFN_CreateTextureFromFileExW = 1,
	D3XFN_CreateTextureFromFileInMemory = 2,
	D3XFN_CreateTextureFromFileInMemoryEx = 3,
	D3XFN_CreateCubeTextureFromFileA = 4,
//	D3XFN_CreateCubeTextureFromFileW = 4,
	D3XFN_CreateCubeTextureFromFileExA = 5,
//	D3XFN_CreateCubeTextureFromFileExW = 5,
	D3XFN_CreateCubeTextureFromFileInMemory = 6,
	D3XFN_CreateCubeTextureFromFileInMemoryEx = 7,
	D3XFN_CreateVolumeTextureFromFileA = 8,
//	D3XFN_CreateVolumeTextureFromFileW = 8,
	D3XFN_CreateVolumeTextureFromFileExA = 9,
//	D3XFN_CreateVolumeTextureFromFileExW = 9,
	D3XFN_CreateVolumeTextureFromFileInMemory = 10,
	D3XFN_CreateVolumeTextureFromFileInMemoryEx = 11,
	D3XFN_SaveTextureToFileA = 12,
//	D3XFN_SaveTextureToFileW = 12,
};

/* linked functions:
 *
 * D3DXCreateTextureFromFileA
 * D3DXSaveTextureToFileA
 * D3DXLoadSurfaceFromSurface
 * D3DXMatrixMultiply
 * D3DXPlaneTransform
 * D3DXPlaneNormalize
 * D3DXMatrixTranspose
 * D3DXMatrixInverse
 * D3DXCreateTexture
 * D3DXCreateVolumeTextureFromFileInMemory
 * D3DXCreateCubeTextureFromFileInMemory
 * D3DXCreateTextureFromFileInMemory
 * D3DXGetImageInfoFromFileInMemory
 * D3DXGetShaderConstantTable
 * D3DXCompileShaderFromFileA
 * D3DXCompileShader
 * D3DXGetVertexShaderProfile
 * D3DXGetPixelShaderProfile
 * D3DXAssembleShaderFromFileA
 * D3DXAssembleShader
 * D3DXVec3Normalize
 * D3DXVec3TransformNormal
 * D3DXVec3TransformCoord
 * D3DXMatrixRotationYawPitchRoll
 *
 * D3DXVec4Transform
 */

/* ----------------------------------------------------------------------------- */
#define	D3XHook		D3XHook27
#define	D3XHookDLL	"d3dx9_27.dll"

#define	OBGESaveTextureToFileA				OBGE27SaveTextureToFileA
#define	OBGESaveTextureToFileW				OBGE27SaveTextureToFileW
#define	OBGECreateTextureFromFileA			OBGE27CreateTextureFromFileA
#define	OBGECreateTextureFromFileW                      OBGE27CreateTextureFromFileW
#define	OBGECreateTextureFromFileExA                    OBGE27CreateTextureFromFileExA
#define	OBGECreateTextureFromFileExW                    OBGE27CreateTextureFromFileExW
#define	OBGECreateTextureFromFileInMemory               OBGE27CreateTextureFromFileInMemory
#define	OBGECreateTextureFromFileInMemoryEx             OBGE27CreateTextureFromFileInMemoryEx
#define	OBGECreateCubeTextureFromFileA                  OBGE27CreateCubeTextureFromFileA
#define	OBGECreateCubeTextureFromFileW                  OBGE27CreateCubeTextureFromFileW
#define	OBGECreateCubeTextureFromFileExA                OBGE27CreateCubeTextureFromFileExA
#define	OBGECreateCubeTextureFromFileExW                OBGE27CreateCubeTextureFromFileExW
#define	OBGECreateCubeTextureFromFileInMemory           OBGE27CreateCubeTextureFromFileInMemory
#define	OBGECreateCubeTextureFromFileInMemoryEx         OBGE27CreateCubeTextureFromFileInMemoryEx
#define	OBGECreateVolumeTextureFromFileA                OBGE27CreateVolumeTextureFromFileA
#define	OBGECreateVolumeTextureFromFileW                OBGE27CreateVolumeTextureFromFileW
#define	OBGECreateVolumeTextureFromFileExA              OBGE27CreateVolumeTextureFromFileExA
#define	OBGECreateVolumeTextureFromFileExW              OBGE27CreateVolumeTextureFromFileExW
#define	OBGECreateVolumeTextureFromFileInMemory         OBGE27CreateVolumeTextureFromFileInMemory
#define	OBGECreateVolumeTextureFromFileInMemoryEx       OBGE27CreateVolumeTextureFromFileInMemoryEx

#include "D3DX_XX.cpp"

/* ----------------------------------------------------------------------------- */
#define	D3XHook		D3XHook31
#define	D3XHookDLL	"d3dx9_31.dll"

#define	OBGESaveTextureToFileA				OBGE31SaveTextureToFileA
#define	OBGESaveTextureToFileW				OBGE31SaveTextureToFileW
#define	OBGECreateTextureFromFileA			OBGE31CreateTextureFromFileA
#define	OBGECreateTextureFromFileW                      OBGE31CreateTextureFromFileW
#define	OBGECreateTextureFromFileExA                    OBGE31CreateTextureFromFileExA
#define	OBGECreateTextureFromFileExW                    OBGE31CreateTextureFromFileExW
#define	OBGECreateTextureFromFileInMemory               OBGE31CreateTextureFromFileInMemory
#define	OBGECreateTextureFromFileInMemoryEx             OBGE31CreateTextureFromFileInMemoryEx
#define	OBGECreateCubeTextureFromFileA                  OBGE31CreateCubeTextureFromFileA
#define	OBGECreateCubeTextureFromFileW                  OBGE31CreateCubeTextureFromFileW
#define	OBGECreateCubeTextureFromFileExA                OBGE31CreateCubeTextureFromFileExA
#define	OBGECreateCubeTextureFromFileExW                OBGE31CreateCubeTextureFromFileExW
#define	OBGECreateCubeTextureFromFileInMemory           OBGE31CreateCubeTextureFromFileInMemory
#define	OBGECreateCubeTextureFromFileInMemoryEx         OBGE31CreateCubeTextureFromFileInMemoryEx
#define	OBGECreateVolumeTextureFromFileA                OBGE31CreateVolumeTextureFromFileA
#define	OBGECreateVolumeTextureFromFileW                OBGE31CreateVolumeTextureFromFileW
#define	OBGECreateVolumeTextureFromFileExA              OBGE31CreateVolumeTextureFromFileExA
#define	OBGECreateVolumeTextureFromFileExW              OBGE31CreateVolumeTextureFromFileExW
#define	OBGECreateVolumeTextureFromFileInMemory         OBGE31CreateVolumeTextureFromFileInMemory
#define	OBGECreateVolumeTextureFromFileInMemoryEx       OBGE31CreateVolumeTextureFromFileInMemoryEx

#include "D3DX_XX.cpp"

/* ----------------------------------------------------------------------------- */
#define	D3XHook		D3XHook41
#define	D3XHookDLL	"d3dx9_41.dll"

#define	OBGESaveTextureToFileA				OBGE41SaveTextureToFileA
#define	OBGESaveTextureToFileW				OBGE41SaveTextureToFileW
#define	OBGECreateTextureFromFileA			OBGE41CreateTextureFromFileA
#define	OBGECreateTextureFromFileW                      OBGE41CreateTextureFromFileW
#define	OBGECreateTextureFromFileExA                    OBGE41CreateTextureFromFileExA
#define	OBGECreateTextureFromFileExW                    OBGE41CreateTextureFromFileExW
#define	OBGECreateTextureFromFileInMemory               OBGE41CreateTextureFromFileInMemory
#define	OBGECreateTextureFromFileInMemoryEx             OBGE41CreateTextureFromFileInMemoryEx
#define	OBGECreateCubeTextureFromFileA                  OBGE41CreateCubeTextureFromFileA
#define	OBGECreateCubeTextureFromFileW                  OBGE41CreateCubeTextureFromFileW
#define	OBGECreateCubeTextureFromFileExA                OBGE41CreateCubeTextureFromFileExA
#define	OBGECreateCubeTextureFromFileExW                OBGE41CreateCubeTextureFromFileExW
#define	OBGECreateCubeTextureFromFileInMemory           OBGE41CreateCubeTextureFromFileInMemory
#define	OBGECreateCubeTextureFromFileInMemoryEx         OBGE41CreateCubeTextureFromFileInMemoryEx
#define	OBGECreateVolumeTextureFromFileA                OBGE41CreateVolumeTextureFromFileA
#define	OBGECreateVolumeTextureFromFileW                OBGE41CreateVolumeTextureFromFileW
#define	OBGECreateVolumeTextureFromFileExA              OBGE41CreateVolumeTextureFromFileExA
#define	OBGECreateVolumeTextureFromFileExW              OBGE41CreateVolumeTextureFromFileExW
#define	OBGECreateVolumeTextureFromFileInMemory         OBGE41CreateVolumeTextureFromFileInMemory
#define	OBGECreateVolumeTextureFromFileInMemoryEx       OBGE41CreateVolumeTextureFromFileInMemoryEx

#include "D3DX_XX.cpp"

/* ----------------------------------------------------------------------------- */
#define	D3XHook		D3XHook43
#define	D3XHookDLL	"d3dx9_43.dll"

#define	OBGESaveTextureToFileA				OBGE43SaveTextureToFileA
#define	OBGESaveTextureToFileW				OBGE43SaveTextureToFileW
#define	OBGECreateTextureFromFileA			OBGE43CreateTextureFromFileA
#define	OBGECreateTextureFromFileW                      OBGE43CreateTextureFromFileW
#define	OBGECreateTextureFromFileExA                    OBGE43CreateTextureFromFileExA
#define	OBGECreateTextureFromFileExW                    OBGE43CreateTextureFromFileExW
#define	OBGECreateTextureFromFileInMemory               OBGE43CreateTextureFromFileInMemory
#define	OBGECreateTextureFromFileInMemoryEx             OBGE43CreateTextureFromFileInMemoryEx
#define	OBGECreateCubeTextureFromFileA                  OBGE43CreateCubeTextureFromFileA
#define	OBGECreateCubeTextureFromFileW                  OBGE43CreateCubeTextureFromFileW
#define	OBGECreateCubeTextureFromFileExA                OBGE43CreateCubeTextureFromFileExA
#define	OBGECreateCubeTextureFromFileExW                OBGE43CreateCubeTextureFromFileExW
#define	OBGECreateCubeTextureFromFileInMemory           OBGE43CreateCubeTextureFromFileInMemory
#define	OBGECreateCubeTextureFromFileInMemoryEx         OBGE43CreateCubeTextureFromFileInMemoryEx
#define	OBGECreateVolumeTextureFromFileA                OBGE43CreateVolumeTextureFromFileA
#define	OBGECreateVolumeTextureFromFileW                OBGE43CreateVolumeTextureFromFileW
#define	OBGECreateVolumeTextureFromFileExA              OBGE43CreateVolumeTextureFromFileExA
#define	OBGECreateVolumeTextureFromFileExW              OBGE43CreateVolumeTextureFromFileExW
#define	OBGECreateVolumeTextureFromFileInMemory         OBGE43CreateVolumeTextureFromFileInMemory
#define	OBGECreateVolumeTextureFromFileInMemoryEx       OBGE43CreateVolumeTextureFromFileInMemoryEx

#include "D3DX_XX.cpp"

#endif
#endif
