
#include "D3D9.hpp"
#include "D3D9Device.hpp"

// Tracker
OBGEDirect3DDevice9 *lastOBGEDirect3DDevice9;

// Hook-Tracker
enum OBGEPass currentPass = OBGEPASS_UNKNOWN;
IDirect3DTexture9 *passTexture[OBGEPASS_NUM];
IDirect3DSurface9 *passSurface[OBGEPASS_NUM];
IDirect3DSurface9 *passDepth  [OBGEPASS_NUM];

#include "D3D9Identifiers.hpp"

#ifdef	OBGE_LOGGING
int frame_dmp = 0;
int frame_num = 0;
int frame_bge = 0;
IDebugLog *frame_log = NULL;
#endif

bool frame_trk = true;

/* these have to be tracked globally, when the device is recreated they may stay alive, but the map wouldn't */
std::map <void *, struct renderSurface  *> surfaceRender;
std::map <void *, struct depthSurface   *> surfaceDepth;
std::map <void *, struct textureSurface *> surfaceTexture;
std::map <void *, struct textureMap     *> textureMaps;
