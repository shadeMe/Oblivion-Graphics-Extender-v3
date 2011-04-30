
#include "D3D9.hpp"
#include "D3D9Device.hpp"

// Tracker
OBGEDirect3DDevice9 *lastOBGEDirect3DDevice9;

// Hook-Tracker
enum OBGEPass currentPass = OBGEPASS_UNKNOWN;
IDirect3DTexture9 *passTexture[OBGEPASS_NUM];
IDirect3DSurface9 *passSurface[OBGEPASS_NUM];
IDirect3DSurface9 *passDepth  [OBGEPASS_NUM];
int                passFrames [OBGEPASS_NUM] = {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1};
int                passPasses [OBGEPASS_NUM] = {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1};
const char        *passNames  [OBGEPASS_NUM] = {
  "0. No particular pass",
  "1. Reflection pass (off-screen)",
  "2. Water aux pass (off-screen)",
  "3. Water heightmap pass (off-screen)",
  "4. Water displacement pass (off-screen)",
  "5. Shadow pass (off-screen)",
  "6. Main pass (screen-space)",
  "7. HDR pass (screen-space)",
  "8. Effects pass (screen-space)",
  "9. Post pass (screen-space)",

  "-. Video pass",
  "-. Unknown pass",
};

#include "D3D9Identifiers.hpp"

#if	defined(OBGE_LOGGING)
int frame_dmp = 0;
int frame_num = 0;
int frame_bge = 0;
IDebugLog *frame_log = NULL;
#elif	defined(OBGE_DEVLING)
int frame_num = 0;
int frame_bge = 0;
#endif

bool frame_trk = true;

/* these have to be tracked globally, when the device is recreated they may stay alive, but the map wouldn't */
std::map <void *, struct renderSurface  *> surfaceRender;
std::map <void *, struct depthSurface   *> surfaceDepth;
std::map <void *, struct textureSurface *> surfaceTexture;
std::map <void *, struct textureMap     *> textureMaps;
