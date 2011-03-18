#ifndef	D3D9IDENTIFIERS_HPP
#define	D3D9IDENTIFIERS_HPP

#include <map>
#include <d3d9.h>
#include <d3dx9.h>

#define	OBGE_LOGGING
#define	OBGE_HOOKING
#define	OBGE_PROFILE
#define	OBGE_TRACKER
#undef	OBGE_TRACKER_SURFACES		// replace by OBGE-implementation, 0 = only rendertargets, 1 = all
#undef	OBGE_TRACKER_TEXTURES		// replace by OBGE-implementation, 0 = only rendertargets, 1 = all

/* ------------------------------------------------------------------------------- */

#ifdef	OBGE_LOGGING
extern int frame_dmp;
extern bool frame_trk;
extern int frame_num;
extern int frame_bge;
extern IDebugLog *frame_log;
#endif

/* ------------------------------------------------------------------------------- */

const char *findShader(void *iface, UINT len, const DWORD* buf);
const char *findShader(void *iface);
const char *findFormat(D3DFORMAT fmt);
const char *findUsage(DWORD use);

/* ------------------------------------------------------------------------------- */

struct textureMap {
	UINT Width;
	UINT Height;
	UINT Levels;
	DWORD Usage;
	D3DFORMAT Format;
};

struct renderSurface {
	UINT Width;
	UINT Height;
	D3DFORMAT Format;
	D3DMULTISAMPLE_TYPE MultiSample;
	DWORD MultisampleQuality;
	BOOL Lockable;
};

struct depthSurface {
	UINT Width;
	UINT Height;
	D3DFORMAT Format;
	D3DMULTISAMPLE_TYPE MultiSample;
	DWORD MultisampleQuality;
	BOOL Discard;
};

struct textureSurface {
	UINT Level;

	struct textureMap *map;
	IDirect3DTexture9 *tex;
};

/* these have to be tracked globally, when the device is recreated they may stay alive, but the map wouldn't */
extern std::map <void *, struct renderSurface  *> surfaceRender;
extern std::map <void *, struct depthSurface   *> surfaceDepth;
extern std::map <void *, struct textureSurface *> surfaceTexture;
extern std::map <void *, struct textureMap     *> textureMaps;

#endif
