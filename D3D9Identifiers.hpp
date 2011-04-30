#ifndef	D3D9IDENTIFIERS_HPP
#define	D3D9IDENTIFIERS_HPP

#include <d3d9.h>
#include <d3dx9.h>

#ifndef	NDEBUG
#define	_DMESSAGE	_MESSAGE
#else
#define	_DMESSAGE(...)	0
#endif

#undef	OBGE_LOGGING
#undef	OBGE_HOOKING
#define	OBGE_DEVLING
#undef	OBGE_PROFILE
#define	OBGE_TRACKER		0	// replace by OBGE-implementation, 0 = only rendertargets, 1 = all
#undef	OBGE_TRACKER_SURFACES		// replace by OBGE-implementation, 0 = only rendertargets, 1 = all
#undef	OBGE_TRACKER_TEXTURES		// replace by OBGE-implementation, 0 = only rendertargets, 1 = all

enum OBGEPass {
  OBGEPASS_ANY			= 0x0,

  OBGEPASS_REFLECTION		= 0x1,	// off screen-space
  OBGEPASS_WATER	 	= 0x2,	// off screen-space
  OBGEPASS_WATERHEIGHTMAP	= 0x3,	// off screen-space
  OBGEPASS_WATERDISPLACEMENT 	= 0x4,	// off screen-space
  OBGEPASS_SHADOW		= 0x5,	// off screen-space
  OBGEPASS_MAIN			= 0x6,	// missing
  OBGEPASS_HDR			= 0x7,	// on screen-space, hdr
  OBGEPASS_EFFECTS		= 0x8,	// on screen-space, blur, hit, menu, etc.
  OBGEPASS_POST			= 0x9,	// unknown

  OBGEPASS_VIDEO		= 0xA,	// bink
  OBGEPASS_UNKNOWN		= 0xB,

  OBGEPASS_MAX			= OBGEPASS_UNKNOWN,
  OBGEPASS_NUM			= OBGEPASS_UNKNOWN + 1,
};

/* ------------------------------------------------------------------------------- */

const char *findShader(void *iface, UINT len, const DWORD* buf);
const char *findShader(void *iface);
const char *findFormat(D3DFORMAT fmt);
const char *findUsage(DWORD use);
const char *findSamplerState(D3DSAMPLERSTATETYPE sstate);
const char *findTextureState(D3DTEXTURESTAGESTATETYPE tstate);
const char *findRenderState(D3DRENDERSTATETYPE rstate);

#endif
