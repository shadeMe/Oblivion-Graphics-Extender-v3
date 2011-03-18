
#include "D3D9Identifiers.hpp"
#include "D3D9Identifiers.cpp"
#include "D3D9.hpp"
#include "D3D9Device.hpp"

// Tracker
OBGEDirect3D9 *lastOBGEDirect3D9;
OBGEDirect3DDevice9 *lastOBGEDirect3DDevice9;

// Hook-Tracker
enum OBGEPass currentPass = OBGEPASS_UNKNOWN;
IDirect3DTexture9 *passTexture[OBGEPASS_NUM];
IDirect3DSurface9 *passSurface[OBGEPASS_NUM];
IDirect3DSurface9 *passDepth  [OBGEPASS_NUM];

// Hook structure.
enum
{
	D3DFN_Direct3DCreate9 = 0
};

SDLLHook D3DHook =
{
	"D3D9.DLL", NULL,
	false, NULL,		// Default hook disabled, NULL function pointer.
	{
		{ "Direct3DCreate9", OBGEDirect3DCreate9},
		{ NULL, NULL }
	}
};

// Hook function.
IDirect3D9* FAR WINAPI OBGEDirect3DCreate9(UINT sdk_version)
{
	Direct3DCreate9_t old_func = (Direct3DCreate9_t) D3DHook.Functions[D3DFN_Direct3DCreate9].OrigFn;
	IDirect3D9* d3d = old_func(sdk_version);

	_MESSAGE("OD3D9: D3D queried from 0x%08x", _ReturnAddress());

	return d3d ? new OBGEDirect3D9(d3d) : 0;
}
