#if	defined(OBGE_LOGGING) || defined(OBGE_DEVLING) || defined(OBGE_GAMMACORRECTION)
#ifndef	HOOKING_D3DX_CPP
#define	HOOKING_D3DX_CPP

#include "D3DX.hpp"
#include "../D3DX.hpp"

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
