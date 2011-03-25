#ifndef	HOOKING_D3D9_HPP
#define	HOOKING_D3D9_HPP

#include <d3d9.h>

// Function pointer types.
typedef IDirect3D9* (FAR WINAPI *Direct3DCreate9_t)(UINT sdk_version);

// Function prototypes.
IDirect3D9* FAR WINAPI OBGEDirect3DCreate9(UINT sdk_version);

#endif
