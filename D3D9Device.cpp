#ifndef	OBGE_NOSHADER

#include "D3D9.hpp"
#include "D3D9Device.hpp"
#include "Constants.h"

// Tracker
OBGEDirect3DDevice9 *lastOBGEDirect3DDevice9 = NULL;
static std::vector<OBGEDirect3DDevice9 *> OBGEDevices;

/* ----------------------------------------------------------------------------- */

// Hook-Tracker
enum OBGEPass currentPass  = OBGEPASS_UNKNOWN,
              previousPass = OBGEPASS_UNKNOWN;
IDirect3DTexture9 *passTexture[OBGEPASS_NUM];
IDirect3DTexture9 *passDepthT [OBGEPASS_NUM];
IDirect3DSurface9 *passSurface[OBGEPASS_NUM];
IDirect3DSurface9 *passDepth  [OBGEPASS_NUM];
int                passFrames [OBGEPASS_NUM] = { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1};
int                passPasses [OBGEPASS_NUM] = { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1};
const char        *passScene;

/* ----------------------------------------------------------------------------- */

/* hacking CreateTexure passed via the RenderSurfaceParameters-hook */
D3DTEXTUREFILTERTYPE AMFilter = D3DTEXF_NONE;
unsigned long AFilters = 0;
unsigned long ALODs = 0;
int Anisotropy = 1;
float LODBias = 0.0;

GUID GammaGUID = {0};
bool DeGamma = false, DeGammaState = false;
bool ReGamma = false, ReGammaState = false;

/* ----------------------------------------------------------------------------- */

#include "D3D9Identifiers.hpp"

bool frame_trk = true;

/* . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . */
#if	defined(OBGE_LOGGING)
int frame_dmp = 0;
IDebugLog *frame_log = NULL;
#endif

/* . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . */
#if	defined(OBGE_DEVLING) || defined(OBGE_LOGGING)
int frame_num = 0;
int frame_bge = 0;
#endif

/* . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . */
#if	defined(OBGE_DEVLING) && defined(OBGE_PROFILE)
LARGE_INTEGER frame_bgn;
LARGE_INTEGER frame_end;

bool frame_prf = false;
bool frame_ntx = false;
#endif

/* . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . */
#if	defined(OBGE_DEVLING) && defined(OBGE_TESSELATION)
bool frame_wre = false;
bool frame_tes = false;
bool shadr_tes = false;	// next vertex shader has tesselation support

#ifndef	NDEBUG
#pragma comment(lib,"ATITessellation/Lib/x86/ATITessellationD3D9_MT_d.lib")
#else
#pragma comment(lib,"ATITessellation/Lib/x86/ATITessellationD3D9_MT.lib")
#endif
#endif

/* . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . */

/* these have to be tracked globally, when the device is recreated they may stay alive, but the map wouldn't */
std::map <void *, struct renderSurface  *> surfaceRender;
std::map <void *, struct depthSurface   *> surfaceDepth;
std::map <void *, struct textureSurface *> surfaceTexture;
std::map <void *, struct textureMap     *> textureMaps;

/* -----------------------------------------------------------------------------*/

/* map [0-15,256,257-260] to [0-15,16-19,20] */
int dx2obgeSampler[512] = {
  /* pixel shader sampler */
  0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,	/* 000-031 */
            0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,	/* 032-063 */
            0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,	/* 064-095 */
            0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,	/* 096-127 */

            0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,	/* 128-159 */
            0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,	/* 160-191 */
            0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,	/* 192-223 */
            0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,	/* 224-255 */
  /* tesselator sampler */
  20,
  /* vertex shader sampler */
  16, 17, 18, 19											/* rest 0s */
};

/* ----------------------------------------------------------------------------- */

#include "GUIs_DebugWindow.hpp"

/* We need d3d so that we'd use a pointer to OBGEDirect3D9 instead of the original
 * IDirect3D9 implementor in functions like GetDirect3D9
 */
OBGEDirect3DDevice9::OBGEDirect3DDevice9(IDirect3D9 *d3d, IDirect3DDevice9 *device) : m_d3d(d3d), m_device(device) {
  _MESSAGE("OD3D9: Device 0x%08x constructed from 0x%08x (%d devices available)", this, _ReturnAddress(), OBGEDevices.size() + 1);

  /* add to vector and replace by the new version */
  lastOBGEDirect3DDevice9 = this;
  OBGEDevices.push_back(this);

    ALODs    |= 1 << D3DSAMP_MIPFILTER;
  if (lastOBGEDirect3D9CAPS.TextureCaps & D3DPTFILTERCAPS_MAGFANISOTROPIC)
    AFilters |= 1 << D3DSAMP_MAGFILTER;
  if (lastOBGEDirect3D9CAPS.TextureCaps & D3DPTFILTERCAPS_MINFANISOTROPIC)
    AFilters |= 1 << D3DSAMP_MINFILTER;

  /* must be on in any case, otherwise we can not get
   * access to any of the rendertargets/depthstencils
   */
  frame_trk = true;

#if	defined(OBGE_LOGGING)
  /* setup logging */
  frame_dmp = 0;
  frame_log = NULL;
#endif

#if	defined(OBGE_DEVLING) || defined(OBGE_LOGGING)
  frame_num = 0;
  frame_bge = 0;

  frame_prf = false;
  frame_ntx = false;

  pEvent = NULL;
#endif

#if	defined(OBGE_DEVLING) || defined(OBGE_TESSELATION)
  frame_wre = false;
  frame_tes = false;

  pATITessInterface = IATITessellationD3D9::Create(d3d, device);
#endif

#define HasShaderManager	1	// m_shaders
  m_shaders = ShaderManager::GetSingleton();
  m_shaders->OnCreateDevice();
  m_shadercv = NULL;
  m_shadercp = NULL;

#if	defined(OBGE_DEVLING)
  /* just for now */
  DebugWindow::Expunge();

//assert(NULL);
#endif
}

OBGEDirect3DDevice9::~OBGEDirect3DDevice9() {
  lastOBGEDirect3DDevice9 = NULL;

  std::map<void *, struct renderSurface *>::iterator sR = surfaceRender.begin();
  while (sR != surfaceRender.end()) { if (sR->second) delete sR->second; sR++; }

  surfaceRender.clear();

  std::map<void *, struct depthSurface *>::iterator sD = surfaceDepth.begin();
  while (sD != surfaceDepth.end()) { if (sD->second) delete sD->second; sD++; }

  surfaceDepth.clear();

  std::map<void *, struct textureSurface *>::iterator sT = surfaceTexture.begin();
  while (sT != surfaceTexture.end()) { if (sT->second) delete sT->second; sT++; }

  surfaceTexture.clear();

  std::map<void *, struct textureMap *>::iterator tM = textureMaps.begin();
  while (tM != textureMaps.end()) { if (tM->second) delete tM->second; tM++; }

  textureMaps.clear();

#if	defined(OBGE_DEVLING) || defined(OBGE_TESSELATION)
  if (pATITessInterface)
    delete pATITessInterface;
#endif

#if	defined(OBGE_DEVLING)
  /* just for now */
  DebugWindow::Destroy();
#endif

  /* remove from vector and replace by the other version */
  OBGEDevices.erase(std::find(OBGEDevices.begin(), OBGEDevices.end(), this));
  lastOBGEDirect3DDevice9 = (OBGEDevices.size() ? OBGEDevices.back() : NULL);

  _MESSAGE("OD3D9: Device 0x%08x destructed from 0x%08x (%d devices left)", this, _ReturnAddress(), OBGEDevices.size());
}

/*** IUnknown methods ***/
COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE OBGEDirect3DDevice9::QueryInterface(REFIID riid, void **ppvObj) {
  return m_device->QueryInterface(riid, ppvObj);
}

COM_DECLSPEC_NOTHROW ULONG STDMETHODCALLTYPE OBGEDirect3DDevice9::AddRef(void) {
  return m_device->AddRef();
}

COM_DECLSPEC_NOTHROW ULONG STDMETHODCALLTYPE OBGEDirect3DDevice9::Release(void) {
  ULONG count = m_device->Release();

  if (0 == count)
    delete this;

  return count;
}

/*** IDirect3DDevice9 methods ***/
COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE OBGEDirect3DDevice9::TestCooperativeLevel(void) {
  return m_device->TestCooperativeLevel();
}

COM_DECLSPEC_NOTHROW UINT STDMETHODCALLTYPE OBGEDirect3DDevice9::GetAvailableTextureMem(void) {
  return m_device->GetAvailableTextureMem();
}

COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE OBGEDirect3DDevice9::EvictManagedResources(void) {
  return m_device->EvictManagedResources();
}

COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE OBGEDirect3DDevice9::GetDirect3D(IDirect3D9 **ppD3D9) {
  // Let the device validate the incoming pointer for us
  HRESULT hr = m_device->GetDirect3D(ppD3D9);

  if (SUCCEEDED(hr))
    *ppD3D9 = m_d3d;

  return hr;
}

COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE OBGEDirect3DDevice9::GetDeviceCaps(D3DCAPS9 *pCaps) {
  return m_device->GetDeviceCaps(pCaps);
}

COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE OBGEDirect3DDevice9::GetDisplayMode(UINT iSwapChain, D3DDISPLAYMODE *pMode) {
  return m_device->GetDisplayMode(iSwapChain, pMode);
}

COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE OBGEDirect3DDevice9::GetCreationParameters(D3DDEVICE_CREATION_PARAMETERS *pParameters) {
  return m_device->GetCreationParameters(pParameters);
}

COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE OBGEDirect3DDevice9::SetCursorProperties(UINT XHotSpot, UINT YHotSpot, IDirect3DSurface9 *pCursorBitmap) {
  return m_device->SetCursorProperties(XHotSpot, YHotSpot, pCursorBitmap);
}

COM_DECLSPEC_NOTHROW void STDMETHODCALLTYPE OBGEDirect3DDevice9::SetCursorPosition(int X, int Y, DWORD Flags) {
  m_device->SetCursorPosition(X, Y, Flags);
}

COM_DECLSPEC_NOTHROW BOOL STDMETHODCALLTYPE OBGEDirect3DDevice9::ShowCursor(BOOL bShow) {
  return m_device->ShowCursor(bShow);
}

COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE OBGEDirect3DDevice9::CreateAdditionalSwapChain(D3DPRESENT_PARAMETERS *pPresentationParameters, IDirect3DSwapChain9 **pSwapChain) {
  return m_device->CreateAdditionalSwapChain(pPresentationParameters, pSwapChain);
}

COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE OBGEDirect3DDevice9::GetSwapChain(UINT iSwapChain, IDirect3DSwapChain9 **pSwapChain) {
  return m_device->GetSwapChain(iSwapChain, pSwapChain);
}

COM_DECLSPEC_NOTHROW UINT STDMETHODCALLTYPE OBGEDirect3DDevice9::GetNumberOfSwapChains(void) {
  return m_device->GetNumberOfSwapChains();
}

COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE OBGEDirect3DDevice9::Reset(D3DPRESENT_PARAMETERS *pPresentationParameters) {
  return m_device->Reset(pPresentationParameters);
}

COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE OBGEDirect3DDevice9::Present(CONST RECT *pSourceRect, CONST RECT *pDestRect, HWND hDestWindowOverride, CONST RGNDATA *pDirtyRegion) {
  memset(passTexture, 0, sizeof(passTexture));
  memset(passSurface, 0, sizeof(passSurface));
  memset(passDepthT , 0, sizeof(passDepthT ));
  memset(passDepth  , 0, sizeof(passDepth  ));
  memset(passPasses , 0, sizeof(passPasses ));

#if	defined(OBGE_DEVLING) && defined(OBGE_PROFILE)
  if (HasShaderManager) {
    int frame_wrp = m_shaders->frame_capt % OBGEFRAME_NUM;

    LARGE_INTEGER moment;
    QueryPerformanceCounter(&moment);

    m_shaders->trackh[frame_wrp].frame_totl.QuadPart =
    moment.QuadPart - m_shaders->frame_time.QuadPart;
    m_shaders->frame_time.QuadPart = moment.QuadPart;

    for (int p = 0; p < OBGEPASS_NUM; p++) {
      m_shaders->trackh[frame_wrp].trackd[p].frame_cntr =
      m_shaders->                  trackd[p].frame_cntr;

      memcpy(
	m_shaders->trackh[frame_wrp].trackd[p].frame_hist,
	m_shaders->                  trackd[p].frame_time,
	sizeof(m_shaders->trackd[p].frame_time[0]) *
	       m_shaders->trackd[p].frame_cntr
      );
    }

    m_shaders->frame_capt++;
  }
#endif

#if	defined(OBGE_DEVLING) || defined(OBGE_LOGGING)
  /* log just a single frame, otherwise it's too much data */
  frame_num++;
  frame_bge = 0;
#endif

#if	defined(OBGE_LOGGING)
  /* count down */
  if (frame_dmp > 0)
    frame_dmp = frame_dmp - 1;

  /* start logging */
  if ((frame_dmp > 0) && !frame_log) {
    char name[256];
    sprintf_s(name, "OBGEv2-frame%04d+.log", frame_num);

    frame_log = new IDebugLog(name);
    frame_dmp = frame_dmp;
  }
  /* interrupt logging */
  else if ((frame_dmp <= 0) && frame_log) {
    delete frame_log;

    frame_log = NULL;
    frame_dmp = 0;
  }

  if (frame_log)
    frame_log->FormattedMessage("-- Frame %d --", frame_num);
#endif

  return m_device->Present(pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion);
}

COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE OBGEDirect3DDevice9::GetBackBuffer(UINT iSwapChain, UINT iBackBuffer, D3DBACKBUFFER_TYPE Type, IDirect3DSurface9 **ppBackBuffer) {
  /* TODO: grab */
  return m_device->GetBackBuffer(iSwapChain, iBackBuffer, Type, ppBackBuffer);
}

COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE OBGEDirect3DDevice9::GetRasterStatus(UINT iSwapChain, D3DRASTER_STATUS *pRasterStatus) {
  return m_device->GetRasterStatus(iSwapChain, pRasterStatus);
}

COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE OBGEDirect3DDevice9::SetDialogBoxMode(BOOL bEnableDialogs) {
  return m_device->SetDialogBoxMode(bEnableDialogs);
}

COM_DECLSPEC_NOTHROW void STDMETHODCALLTYPE OBGEDirect3DDevice9::SetGammaRamp(UINT iSwapChain, DWORD Flags, CONST D3DGAMMARAMP *pRamp) {
  return m_device->SetGammaRamp(iSwapChain, Flags, pRamp);
}

COM_DECLSPEC_NOTHROW void STDMETHODCALLTYPE OBGEDirect3DDevice9::GetGammaRamp(UINT iSwapChain, D3DGAMMARAMP *pRamp) {
  return m_device->GetGammaRamp(iSwapChain, pRamp);
}

COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE OBGEDirect3DDevice9::CreateTexture(UINT Width, UINT Height, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DTexture9 **ppTexture, HANDLE *pSharedHandle) {
  if (frame_log)
    frame_log->FormattedMessage("CreateTexture from 0x%08x", _ReturnAddress());

#if	defined(OBGE_AUTOMIPMAP)
  /* hack a mipmap generating rendertarget (just once!) */
  if ((Usage & D3DUSAGE_RENDERTARGET) && (AMFilter != D3DTEXF_NONE)) {
    Usage |= D3DUSAGE_AUTOGENMIPMAP;
    Levels = 0;
  }
#endif

  HRESULT hr = m_device->CreateTexture(Width, Height, Levels, Usage, Format, Pool, ppTexture, pSharedHandle);

#if	defined(OBGE_AUTOMIPMAP)
  if ((*ppTexture) && (AMFilter != D3DTEXF_NONE))
    (*ppTexture)->SetAutoGenFilterType(AMFilter);
#endif

  if (SUCCEEDED(hr)) {
    // Return our texture
#ifdef	OBGE_TRACKER_TEXTURES
#if	OBGE_TRACKER_TEXTURES < 1
    if ((Usage & D3DUSAGE_RENDERTARGET) || (Usage & D3DUSAGE_DEPTHSTENCIL))
#endif
      *ppTexture = new OBGEDirect3DTexture9(m_d3d, m_device, *ppTexture);
#endif

    if (frame_log || frame_trk) {
      if (OBGE_TRACKER || (Usage & (D3DUSAGE_RENDERTARGET | D3DUSAGE_DEPTHSTENCIL))) {
        struct textureMap *track = new struct textureMap;

        track->Width = Width;
        track->Height = Height;
        track->Levels = Levels;
        track->Usage = Usage;
        track->Format = Format;

        textureMaps[*ppTexture] = track;

#ifndef	OBGE_TRACKER_TEXTURES
        /* apparently the level-address stays constant, so we can track this already from here */
        IDirect3DSurface9 *ppSurfaceLevel = NULL;

        if (SUCCEEDED((*ppTexture)->GetSurfaceLevel(0, &ppSurfaceLevel))) {
          struct textureSurface *track = new struct textureSurface;

          track->Level = 0;
          track->map = textureMaps[*ppTexture];
          track->tex = *ppTexture;

          surfaceTexture[ppSurfaceLevel] = track;

          if (Usage & D3DUSAGE_RENDERTARGET)
            _DMESSAGE("OD3D9: RT GetSurfaceLevel[0]: 0x%08x", ppSurfaceLevel);
          else if (Usage & D3DUSAGE_DEPTHSTENCIL)
            _DMESSAGE("OD3D9: DS GetSurfaceLevel[0]: 0x%08x", ppSurfaceLevel);
        }
#endif
      }
    }
  }

  if (frame_log) {
    frame_log->Indent();
    frame_log->FormattedMessage("{W,H}: {%d,%d}", Width, Height);
    frame_log->FormattedMessage("Format: %s", findFormat(Format));
    frame_log->FormattedMessage("Levels: %d", Levels);
    frame_log->FormattedMessage("Usage: %s", findUsage(Usage));
    frame_log->Outdent();
  }
  else if (Usage & D3DUSAGE_RENDERTARGET) {
    _DMESSAGE("OD3D9: CreateRenderTarget via CreateTexture from 0x%08x: 0x%08x", _ReturnAddress(), *ppTexture);
    _DMESSAGE("{W,H}: {%d,%d}", Width, Height);
    _DMESSAGE("Format: %s", findFormat(Format));
    _DMESSAGE("Levels: %d", Levels);
    _DMESSAGE("Usage: %s", findUsage(Usage));
  }
  else if (Usage & D3DUSAGE_DEPTHSTENCIL) {
    _DMESSAGE("OD3D9: CreateDepthStencilSurface via CreateTexture from 0x%08x: 0x%08x", _ReturnAddress(), *ppTexture);
    _DMESSAGE("{W,H}: {%d,%d}", Width, Height);
    _DMESSAGE("Format: %s", findFormat(Format));
    _DMESSAGE("Levels: %d", Levels);
    _DMESSAGE("Usage: %s", findUsage(Usage));
  }

  return hr;
}

COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE OBGEDirect3DDevice9::CreateVolumeTexture(UINT Width, UINT Height, UINT Depth, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DVolumeTexture9 **ppVolumeTexture, HANDLE *pSharedHandle) {
  return m_device->CreateVolumeTexture(Width, Height, Depth, Levels, Usage, Format, Pool, ppVolumeTexture, pSharedHandle);
}

COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE OBGEDirect3DDevice9::CreateCubeTexture(UINT EdgeLength, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DCubeTexture9 **ppCubeTexture, HANDLE *pSharedHandle) {
  return m_device->CreateCubeTexture(EdgeLength, Levels, Usage, Format, Pool, ppCubeTexture, pSharedHandle);
}

COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE OBGEDirect3DDevice9::CreateVertexBuffer(UINT Length, DWORD Usage, DWORD FVF, D3DPOOL Pool, IDirect3DVertexBuffer9 **ppVertexBuffer, HANDLE *pSharedHandle) {
  return m_device->CreateVertexBuffer(Length, Usage, FVF, Pool, ppVertexBuffer, pSharedHandle);
}

COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE OBGEDirect3DDevice9::CreateIndexBuffer(UINT Length, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DIndexBuffer9 **ppIndexBuffer, HANDLE *pSharedHandle) {
  return m_device->CreateIndexBuffer(Length, Usage, Format, Pool, ppIndexBuffer, pSharedHandle);
}

COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE OBGEDirect3DDevice9::CreateRenderTarget(UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE MultiSample, DWORD MultisampleQuality, BOOL Lockable, IDirect3DSurface9 **ppSurface, HANDLE *pSharedHandle) {
  if (frame_log)
    frame_log->FormattedMessage("CreateRenderTarget from 0x%08x", _ReturnAddress());

  HRESULT hr = m_device->CreateRenderTarget(Width, Height, Format, MultiSample, MultisampleQuality, Lockable, ppSurface, pSharedHandle);

  if (SUCCEEDED(hr)) {
    // Return our surface
#if	defined(OBGE_TRACKER_SURFACES)
    *ppSurface = new OBGEDirect3DSurface9(m_d3d, m_device, *ppSurface);
#endif

    if (frame_log || frame_trk) {
      struct renderSurface *track = new struct renderSurface;

      track->Width = Width;
      track->Height = Height;
      track->Format = Format;
      track->MultiSample = MultiSample;
      track->MultisampleQuality = MultisampleQuality;
      track->Lockable = Lockable;

      surfaceRender[*ppSurface] = track;
    }
  }

  if (frame_log) {
    frame_log->Indent();
    frame_log->FormattedMessage("Address: 0x%08x", *ppSurface);
    frame_log->FormattedMessage("{W,H}: {%d,%d}", Width, Height);
    frame_log->FormattedMessage("Format: %s", findFormat(Format));
    frame_log->FormattedMessage("MultiSample: %d", MultiSample);
    frame_log->FormattedMessage("MultisampleQuality: %d", MultisampleQuality);
    frame_log->FormattedMessage("Lockable: %d", Lockable);
    frame_log->Outdent();
  }
  else {
    _DMESSAGE("OD3D9: CreateRenderTarget from 0x%08x: 0x%08x", _ReturnAddress(), *ppSurface);
    _DMESSAGE("Address: 0x%08x", *ppSurface);
    _DMESSAGE("{W,H}: {%d,%d}", Width, Height);
    _DMESSAGE("Format: %s", findFormat(Format));
    _DMESSAGE("MultiSample: %d", MultiSample);
    _DMESSAGE("MultisampleQuality: %d", MultisampleQuality);
    _DMESSAGE("Lockable: %d", Lockable);
  }

  return hr;
}

COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE OBGEDirect3DDevice9::CreateDepthStencilSurface(UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE MultiSample, DWORD MultisampleQuality, BOOL Discard, IDirect3DSurface9 **ppSurface, HANDLE *pSharedHandle) {
  if (frame_log)
    frame_log->FormattedMessage("CreateDepthStencilSurface from 0x%08x", _ReturnAddress());

  HRESULT hr = m_device->CreateDepthStencilSurface(Width, Height, Format, MultiSample, MultisampleQuality, Discard, ppSurface, pSharedHandle);

  if (SUCCEEDED(hr)) {
    // Return our surface
#if	defined(OBGE_TRACKER_SURFACES)
    *ppSurface = new OBGEDirect3DSurface9(m_d3d, m_device, *ppSurface);
#endif

    if (frame_log || frame_trk) {
      struct depthSurface *track = new struct depthSurface;

      track->Width = Width;
      track->Height = Height;
      track->Format = Format;
      track->MultiSample = MultiSample;
      track->MultisampleQuality = MultisampleQuality;
      track->Discard = Discard;

      surfaceDepth[*ppSurface] = track;
    }
  }

  if (frame_log) {
    frame_log->Indent();
    frame_log->FormattedMessage("Address: 0x%08x", *ppSurface);
    frame_log->FormattedMessage("{W,H}: {%d,%d}", Width, Height);
    frame_log->FormattedMessage("Format: %s", findFormat(Format));
    frame_log->FormattedMessage("MultiSample: %d", MultiSample);
    frame_log->FormattedMessage("MultisampleQuality: %d", MultisampleQuality);
    frame_log->FormattedMessage("Discard: %d", Discard);
    frame_log->Outdent();
  }
  else {
    _DMESSAGE("OD3D9: CreateDepthStencilSurface from 0x%08x: 0x%08x", _ReturnAddress(), *ppSurface);
    _DMESSAGE("Address: 0x%08x", *ppSurface);
    _DMESSAGE("{W,H}: {%d,%d}", Width, Height);
    _DMESSAGE("Format: %s", findFormat(Format));
    _DMESSAGE("MultiSample: %d", MultiSample);
    _DMESSAGE("MultisampleQuality: %d", MultisampleQuality);
    _DMESSAGE("Discard: %d", Discard);
  }

  return hr;
}

COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE OBGEDirect3DDevice9::UpdateSurface(IDirect3DSurface9 *pSourceSurface, CONST RECT *pSourceRect, IDirect3DSurface9 *pDestinationSurface, CONST POINT *pDestPoint) {
#if	defined(OBGE_TRACKER_SURFACES) || defined(OBGE_TRACKER_TEXTURES)
  pSourceSurface      = translateSurface(pSourceSurface     );
  pDestinationSurface = translateSurface(pDestinationSurface);
#endif

  return m_device->UpdateSurface(pSourceSurface, pSourceRect, pDestinationSurface, pDestPoint);
}

COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE OBGEDirect3DDevice9::UpdateTexture(IDirect3DBaseTexture9 *pSourceTexture, IDirect3DBaseTexture9 *pDestinationTexture) {
  return m_device->UpdateTexture(pSourceTexture, pDestinationTexture);
}

COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE OBGEDirect3DDevice9::GetRenderTargetData(IDirect3DSurface9 *pRenderTarget, IDirect3DSurface9 *pDestSurface) {
#if	defined(OBGE_TRACKER_SURFACES) || defined(OBGE_TRACKER_TEXTURES)
  pRenderTarget = translateSurface(pRenderTarget);
  pDestSurface  = translateSurface(pDestSurface );
#endif

  return m_device->GetRenderTargetData(pRenderTarget, pDestSurface);
}

COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE OBGEDirect3DDevice9::GetFrontBufferData(UINT iSwapChain, IDirect3DSurface9 *pDestSurface) {
#if	defined(OBGE_TRACKER_SURFACES) || defined(OBGE_TRACKER_TEXTURES)
  pDestSurface  = translateSurface(pDestSurface );
#endif

  return m_device->GetFrontBufferData(iSwapChain, pDestSurface);
}

COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE OBGEDirect3DDevice9::StretchRect(IDirect3DSurface9 *pSourceSurface, CONST RECT *pSourceRect, IDirect3DSurface9 *pDestSurface, CONST RECT *pDestRect, D3DTEXTUREFILTERTYPE Filter) {
#if	defined(OBGE_TRACKER_SURFACES) || defined(OBGE_TRACKER_TEXTURES)
  pSourceSurface = translateSurface(pSourceSurface);
  pDestSurface   = translateSurface(pDestSurface  );
#endif

  return m_device->StretchRect(pSourceSurface, pSourceRect, pDestSurface, pDestRect, Filter);
}

COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE OBGEDirect3DDevice9::ColorFill(IDirect3DSurface9 *pSurface, CONST RECT *pRect, D3DCOLOR color) {
#if	defined(OBGE_TRACKER_SURFACES) || defined(OBGE_TRACKER_TEXTURES)
  pSurface = translateSurface(pSurface);
#endif

  return m_device->ColorFill(pSurface, pRect, color);
}

COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE OBGEDirect3DDevice9::CreateOffscreenPlainSurface(UINT Width, UINT Height, D3DFORMAT Format, D3DPOOL Pool, IDirect3DSurface9 **ppSurface, HANDLE *pSharedHandle) {
  /* TODO: grab */
  return m_device->CreateOffscreenPlainSurface(Width, Height, Format, Pool, ppSurface, pSharedHandle);
}

COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE OBGEDirect3DDevice9::SetRenderTarget(DWORD RenderTargetIndex, IDirect3DSurface9 *pRenderTarget) {
  /* they are a textures anyway, no need to check dedicated targets */
  if (currentPass != OBGEPASS_UNKNOWN) {
    /* dedicated rendertarget, possibly with multi-sampling */
    passTexture[OBGEPASS_ANY] = passTexture[currentPass] = NULL;
    if ((passSurface[OBGEPASS_ANY] = passSurface[currentPass] = pRenderTarget)) {
      struct textureSurface *texs;

      /* texture-based rendertarget surface, no multisampling */
      if ((texs = surfaceTexture[pRenderTarget]))
        passTexture[OBGEPASS_ANY] = passTexture[currentPass] = texs->tex;
    }

//  _MESSAGE("OD3D9: Grabbed pass %d texture", currentPass);
  }

  if (frame_log) {
    frame_log->FormattedMessage("SetRenderTarget[%d] from 0x%08x", RenderTargetIndex, _ReturnAddress());
    frame_log->Indent();
    frame_log->FormattedMessage("Address: 0x%08x", pRenderTarget);

    bool ok = false;
    if (!pRenderTarget)
      ok = true;

    if (!ok) {
      struct renderSurface *track = surfaceRender[pRenderTarget];

      if (track) {
        frame_log->Message("Type: Dedicated Rendertarget");
        frame_log->FormattedMessage("{W,H}: {%d,%d}", track->Width, track->Height);
        frame_log->FormattedMessage("Format: %s", findFormat(track->Format));
        frame_log->FormattedMessage("MultiSample: %d", track->MultiSample);
        frame_log->FormattedMessage("MultisampleQuality: %d", track->MultisampleQuality);
        frame_log->FormattedMessage("Lockable: %d", track->Lockable);

        ok = true;
      }
    }

    if (!ok) {
      struct textureSurface *track = surfaceTexture[pRenderTarget];

      if (track) {
        frame_log->Message("Type: Rendertarget Texture");
        frame_log->FormattedMessage("{W,H}: {%d,%d}", track->map->Width, track->map->Height);
        frame_log->FormattedMessage("Format: %s", findFormat(track->map->Format));
        frame_log->FormattedMessage("Level: %d of %d", track->Level, track->map->Levels);
        frame_log->FormattedMessage("Usage: %s", findUsage(track->map->Usage));

        ok = true;
      }
    }

    if (!ok) {
      D3DSURFACE_DESC track;

      if (pRenderTarget->GetDesc(&track) == D3D_OK) {
        const char *Type = "Untracked";

        switch (track.Type) {
          case D3DRTYPE_SURFACE:       Type = "Untracked Surface"; break;
          case D3DRTYPE_VOLUME:        Type = "Untracked Volume"; break;
          case D3DRTYPE_TEXTURE:       Type = "Untracked Texture"; break;
          case D3DRTYPE_VOLUMETEXTURE: Type = "Untracked Volume Texture"; break;
          case D3DRTYPE_CUBETEXTURE:   Type = "Untracked Cube Texture"; break;
          case D3DRTYPE_VERTEXBUFFER:  Type = "Untracked Vertex Buffer"; break;
          case D3DRTYPE_INDEXBUFFER:   Type = "Untracked Index Buffer"; break;
        }

        frame_log->FormattedMessage("Type: %s", Type);
        frame_log->FormattedMessage("{W,H}: {%d,%d}", track.Width, track.Height);
        frame_log->FormattedMessage("Format: %s", findFormat(track.Format));
        frame_log->FormattedMessage("MultiSampleType: %d", track.MultiSampleType);
        frame_log->FormattedMessage("MultisampleQuality: %d", track.MultiSampleQuality);
        frame_log->FormattedMessage("Usage: %s", findUsage(track.Usage));

        ok = true;
      }
    }

    frame_log->Outdent();
  }

#if	defined(OBGE_TRACKER_SURFACES) || defined(OBGE_TRACKER_TEXTURES)
  pRenderTarget = translateSurface(pRenderTarget);
#endif

  return m_device->SetRenderTarget(RenderTargetIndex, pRenderTarget);
}

COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE OBGEDirect3DDevice9::GetRenderTarget(DWORD RenderTargetIndex, IDirect3DSurface9 **ppRenderTarget) {
  /* TODO: grab */
  return m_device->GetRenderTarget(RenderTargetIndex, ppRenderTarget);
}

COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE OBGEDirect3DDevice9::SetDepthStencilSurface(IDirect3DSurface9 *pNewZStencil) {
  /* they are a dedicated anyway, no need to check textures */
  if (currentPass != OBGEPASS_UNKNOWN) {
    /* dedicated depthstencil, possibly with multi-sampling */
    passDepthT[OBGEPASS_ANY] = passDepthT[currentPass] = NULL;
    if ((passDepth[OBGEPASS_ANY] = passDepth[currentPass] = pNewZStencil)) {
      struct textureSurface *texs;

      /* texture-based depthstencil surface, no multisampling */
      if ((texs = surfaceTexture[pNewZStencil]))
        passDepthT[OBGEPASS_ANY] = passDepthT[currentPass] = texs->tex;
    }

//  _MESSAGE("OD3D9: Grabbed pass %d depth", currentPass);
  }

  if (frame_log) {
    frame_log->FormattedMessage("SetDepthStencilSurface from 0x%08x", _ReturnAddress());
    frame_log->Indent();
    frame_log->FormattedMessage("Address: 0x%08x", pNewZStencil);

    bool ok = false;
    if (!pNewZStencil)
      ok = true;

    if (!ok) {
      struct depthSurface *track = surfaceDepth[pNewZStencil];

      if (track) {
        frame_log->Message("Type: Dedicated DepthStencilSurface");
        frame_log->FormattedMessage("{W,H}: {%d,%d}", track->Width, track->Height);
        frame_log->FormattedMessage("Format: %s", findFormat(track->Format));
        frame_log->FormattedMessage("MultiSample: %d", track->MultiSample);
        frame_log->FormattedMessage("MultisampleQuality: %d", track->MultisampleQuality);
        frame_log->FormattedMessage("Discard: %d", track->Discard);

        ok = true;
      }
    }

    if (!ok) {
      struct textureSurface *track = surfaceTexture[pNewZStencil];

      if (track) {
        frame_log->Message("Type: Rendertarget Texture");
        frame_log->FormattedMessage("{W,H}: {%d,%d}", track->map->Width, track->map->Height);
        frame_log->FormattedMessage("Format: %s", findFormat(track->map->Format));
        frame_log->FormattedMessage("Level: %d of %d", track->Level, track->map->Levels);
        frame_log->FormattedMessage("Usage: %s", findUsage(track->map->Usage));

        ok = true;
      }
    }

    if (!ok) {
      D3DSURFACE_DESC track;

      if (pNewZStencil->GetDesc(&track) == D3D_OK) {
        const char *Type = "Untracked";

        switch (track.Type) {
          case D3DRTYPE_SURFACE:       Type = "Untracked Surface"; break;
          case D3DRTYPE_VOLUME:        Type = "Untracked Volume"; break;
          case D3DRTYPE_TEXTURE:       Type = "Untracked Texture"; break;
          case D3DRTYPE_VOLUMETEXTURE: Type = "Untracked Volume Texture"; break;
          case D3DRTYPE_CUBETEXTURE:   Type = "Untracked Cube Texture"; break;
          case D3DRTYPE_VERTEXBUFFER:  Type = "Untracked Vertex Buffer"; break;
          case D3DRTYPE_INDEXBUFFER:   Type = "Untracked Index Buffer"; break;
        }

        frame_log->FormattedMessage("Type: %s", Type);
        frame_log->FormattedMessage("{W,H}: {%d,%d}", track.Width, track.Height);
        frame_log->FormattedMessage("Format: %s", findFormat(track.Format));
        frame_log->FormattedMessage("MultiSampleType: %d", track.MultiSampleType);
        frame_log->FormattedMessage("MultisampleQuality: %d", track.MultiSampleQuality);
        frame_log->FormattedMessage("Usage: %s", findUsage(track.Usage));

        ok = true;
      }
    }

    frame_log->Outdent();
  }

#if	defined(OBGE_TRACKER_SURFACES) || defined(OBGE_TRACKER_TEXTURES)
  pNewZStencil = translateSurface(pNewZStencil);
#endif

  return m_device->SetDepthStencilSurface(pNewZStencil);
}

COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE OBGEDirect3DDevice9::GetDepthStencilSurface(IDirect3DSurface9 **ppZStencilSurface) {
  /* TODO: grab */
  return m_device->GetDepthStencilSurface(ppZStencilSurface);
}

COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE OBGEDirect3DDevice9::BeginScene(void) {
//assert(frame_num != 8000);
//assert(currentPass != OBGEPASS_UNKNOWN);
//assert(frame_bge == 26);

  /* the D3D-statemachine doesn't kill these either ... */
//m_shadercv = NULL;
//m_shadercp = NULL;

  if (HasShaderManager) {
    m_shaders->Begin();

#if	defined(OBGE_DEVLING)
    /* clean all on first scene, and partial on successive scenes */
    int frame_numr = m_shaders->trackd[currentPass].frame_numr;

    m_shaders->Clear(currentPass, frame_numr != frame_num);
    m_shaders->trackd[currentPass].frame_numr = frame_num;
#endif
  }

#if	defined(OBGE_DEVLING) || defined(OBGE_LOGGING)
  passFrames[currentPass] = frame_num;
#endif

  if (frame_log) {
    frame_log->FormattedMessage("BeginScene %d from 0x%08x", frame_bge, _ReturnAddress());
    frame_log->Indent();
  }

#if	defined(OBGE_DEVLING) && defined(OBGE_PROFILE)
  if (frame_prf) {
    // 1. Create an event query from the current device
    CreateQuery(D3DQUERYTYPE_EVENT, &pEvent);

    // 2. Add an end marker to the command buffer queue.
    pEvent->Issue(D3DISSUE_END);

    // 3. Empty the command buffer and wait until the GPU is idle.
    while(S_FALSE == pEvent->GetData(NULL, 0, D3DGETDATA_FLUSH))
      ;

    // 4. Start profiling
  }

  QueryPerformanceCounter(&frame_bgn);
#endif

#if	defined(OBGE_DEVLING) && defined(OBGE_TESSELATION)
  // Set max tessellation level
  if (pATITessInterface)
    pATITessInterface->SetMaxLevel(1);
#endif

#if	defined(OBGE_GAMMACORRECTION) && (OBGE_GAMMACORRECTION > 0)
  /* we have to experiment with this */
  if ((currentPass == OBGEPASS_REFLECTION) ||
      (currentPass == OBGEPASS_MAIN)) {
    /* gamma-correction on read */
    for (int s = 0; s < 16; s++)
      m_device->SetSamplerState(s, D3DSAMP_SRGBTEXTURE, DeGamma);

    /* gamma-correction on write */
    m_device->SetRenderState(D3DRS_SRGBWRITEENABLE, ReGamma);
  }
  else {
    /* gamma-correction on read */
    for (int s = 0; s < 16; s++)
      m_device->SetSamplerState(s, D3DSAMP_SRGBTEXTURE, false);

    /* gamma-correction on write */
    m_device->SetRenderState(D3DRS_SRGBWRITEENABLE, false);
  }
#endif

  return m_device->BeginScene();
}

COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE OBGEDirect3DDevice9::EndScene(void) {
  HRESULT res = m_device->EndScene();

#if	defined(OBGE_DEVLING) && defined(OBGE_PROFILE)
  if (frame_prf || pEvent) {
    // 6. Add an end marker to the command buffer queue.
    pEvent->Issue(D3DISSUE_END);

    // 7. Force the driver to execute the commands from the command buffer.
    // Empty the command buffer and wait until the GPU is idle.
    while(S_FALSE == pEvent->GetData(NULL, 0, D3DGETDATA_FLUSH))
      ;

    // 8. End profiling
    pEvent->Release();
    pEvent = NULL;
  }

  QueryPerformanceCounter(&frame_end);
#endif

  if (frame_log) {
    frame_log->Outdent();
    frame_log->FormattedMessage("EndScene from 0x%08x", _ReturnAddress());
  }

#ifdef	OBGE_DEVLING
  /* apparenty there is one BeginScene missing in the HDR-pipeline (2nd scene! */
  if (HasShaderManager) {
    /* record exact position of occurance */
    int frame_cntr = m_shaders->trackd[currentPass].frame_cntr;

    /* huh, going into menu provokes huge numbers here */
    if (frame_cntr < OBGESCENE_NUM) {
      m_shaders->trackd[currentPass].frame_used[frame_cntr] = frame_num;
      m_shaders->trackd[currentPass].frame_pass[frame_cntr] = frame_bge;
      m_shaders->trackd[currentPass].frame_name[frame_cntr] = passScene; passScene = NULL;
#ifdef	OBGE_PROFILE
      m_shaders->trackd[currentPass].frame_time[frame_cntr].QuadPart = frame_end.QuadPart - frame_bgn.QuadPart;
#endif

      m_shaders->trackd[currentPass].rt[frame_cntr] = passSurface[currentPass];
      m_shaders->trackd[currentPass].ds[frame_cntr] = passDepth  [currentPass];

      m_shaders->trackd[currentPass].frame_cntr = frame_cntr + 1;
    }
  }
#endif

#if	defined(OBGE_DEVLING) || defined(OBGE_LOGGING)
  /* track the number of sub-passes for a particular pass */
  passPasses[currentPass]++; frame_bge++;
#endif

#ifdef	OBGE_LOGGING
  if (frame_log && 0) {
    IDirect3DSurface9 *pRenderTarget;
    IDirect3DSurface9 *pBuf;
    D3DSURFACE_DESC VDesc;
    HRESULT LstErr;

    LstErr = GetRenderTarget(0, &pRenderTarget);
    LstErr = pRenderTarget->GetDesc(&VDesc);
    LstErr = CreateOffscreenPlainSurface(VDesc.Width, VDesc.Height, VDesc.Format, D3DPOOL_SYSTEMMEM, &pBuf, NULL);
    LstErr = GetRenderTargetData(pRenderTarget, pBuf);
    char str[256];
    sprintf_s(str, "OBGEv2-frame%04d-scene%02d.dds", frame_num, frame_bge);
    LstErr = D3DXSaveSurfaceToFile(str, D3DXIFF_DDS, pBuf, NULL, NULL);
    pBuf->Release();
  }
#endif

  return res;
}

COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE OBGEDirect3DDevice9::Clear(DWORD Count, CONST D3DRECT *pRects, DWORD Flags, D3DCOLOR Color, float Z, DWORD Stencil) {
  if (frame_log) {
    frame_log->Message("Clear:");

    frame_log->Indent();
    frame_log->FormattedMessage("Flags:%s%s%s",
                                Flags & D3DCLEAR_TARGET  ? " Rendertarget" : "",
                                Flags & D3DCLEAR_STENCIL ? " Stencil" : "",
                                Flags & D3DCLEAR_ZBUFFER ? " Depth" : "");
    frame_log->FormattedMessage("Values: {%d,%d,%d,%d} %f %d",
                                (Color >>  0) & 0xFF,
                                (Color >>  8) & 0xFF,
                                (Color >> 16) & 0xFF,
                                (Color >> 24) & 0xFF,
                                Z,
                                Stencil);

    frame_log->FormattedMessage("Count: %d", Count);

    if (Count) {
      frame_log->FormattedMessage("Rects:");
      frame_log->Indent();

      for (int c = 0; c < Count; c++)
        frame_log->FormattedMessage("[%d,%d] [%d,%d]", pRects[c].x1, pRects[c].x2, pRects[c].y1, pRects[c].y2);

      frame_log->Outdent();
    }

    frame_log->Outdent();
  }

  return m_device->Clear(Count, pRects, Flags, Color, Z, Stencil);
}

COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE OBGEDirect3DDevice9::SetTransform(D3DTRANSFORMSTATETYPE State, CONST D3DMATRIX *pMatrix) {
  if (HasShaderManager) {
    /**/ if (State == D3DTS_VIEW)
      Constants.UpdateView((D3DXMATRIX)*pMatrix);
    else if (State == D3DTS_PROJECTION)
      Constants.UpdateProjection((D3DXMATRIX)*pMatrix);
    else if (State == D3DTS_WORLD)
      Constants.UpdateWorld((D3DXMATRIX)*pMatrix);

#ifdef	OBGE_DEVLING
    /* record exact position of occurance */
    int frame_cntr = m_shaders->trackd[currentPass].frame_cntr;

    /* huh, going into menu provokes huge numbers here */
    if (frame_cntr < OBGESCENE_NUM) {
      if (State == D3DTS_VIEW)
        m_shaders->trackd[currentPass].transf[frame_cntr][0] = *pMatrix;
      else if (State == D3DTS_PROJECTION)
        m_shaders->trackd[currentPass].transf[frame_cntr][1] = *pMatrix;
      else if (State == D3DTS_WORLD)
        m_shaders->trackd[currentPass].transf[frame_cntr][2] = *pMatrix;
    }
#endif
  }

  if (frame_log) {
    frame_log->Message("SetTransform:");

    const char *Type = NULL;

    switch (State) {
      case D3DTS_VIEW:       Type = "View"; break;
      case D3DTS_PROJECTION: Type = "Projection"; break;
      case D3DTS_WORLD:      Type = "World"; break;
      case D3DTS_TEXTURE0:   Type = "Texture0"; break;
      case D3DTS_TEXTURE1:   Type = "Texture1"; break;
      case D3DTS_TEXTURE2:   Type = "Texture2"; break;
      case D3DTS_TEXTURE3:   Type = "Texture3"; break;
      case D3DTS_TEXTURE4:   Type = "Texture4"; break;
      case D3DTS_TEXTURE5:   Type = "Texture5"; break;
      case D3DTS_TEXTURE6:   Type = "Texture6"; break;
      case D3DTS_TEXTURE7:   Type = "Texture7"; break;
    }

    frame_log->Indent();

    if (Type)
      frame_log->FormattedMessage("Type: %s", Type);
    else if ((State >= 256) && (State <= 511))
      frame_log->FormattedMessage("Type: World[%d]", State - 256);
    else
      frame_log->FormattedMessage("Type: unknown %d", State);

    frame_log->FormattedMessage("|%f|%f|%f|%f|", pMatrix->_11, pMatrix->_12, pMatrix->_13, pMatrix->_14);
    frame_log->FormattedMessage("|%f|%f|%f|%f|", pMatrix->_21, pMatrix->_22, pMatrix->_23, pMatrix->_24);
    frame_log->FormattedMessage("|%f|%f|%f|%f|", pMatrix->_31, pMatrix->_32, pMatrix->_33, pMatrix->_34);
    frame_log->FormattedMessage("|%f|%f|%f|%f|", pMatrix->_41, pMatrix->_42, pMatrix->_43, pMatrix->_44);
    frame_log->Outdent();
  }

  return m_device->SetTransform(State, pMatrix);
}

COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE OBGEDirect3DDevice9::GetTransform(D3DTRANSFORMSTATETYPE State, D3DMATRIX *pMatrix) {
  return m_device->GetTransform(State, pMatrix);
}

COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE OBGEDirect3DDevice9::MultiplyTransform(D3DTRANSFORMSTATETYPE State, CONST D3DMATRIX *pMatrix) {
  return m_device->MultiplyTransform(State, pMatrix);
}

COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE OBGEDirect3DDevice9::SetViewport(CONST D3DVIEWPORT9 *pViewport) {
  if (HasShaderManager) {
    const float W = (float)pViewport->Width;
    const float H = (float)pViewport->Height;

    /* record constants */
    Constants.rcpres.x = 1.0f / W;
    Constants.rcpres.y = 1.0f / H;
    Constants.rcpres.z = W / H;
    Constants.rcpres.w = W * H;
  }

  if (frame_log) {
    frame_log->Message("SetViewport:");

    frame_log->Indent();
    frame_log->FormattedMessage("{X,Y}: {%d,%d}", pViewport->X, pViewport->Y);
    frame_log->FormattedMessage("{Width,Height}: {%d,%d}", pViewport->Width, pViewport->Height);
    frame_log->FormattedMessage("{MinZ,MaxZ}: {%f,%f}", pViewport->MinZ, pViewport->MaxZ);
    frame_log->Outdent();
  }

  return m_device->SetViewport(pViewport);
}

COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE OBGEDirect3DDevice9::GetViewport(D3DVIEWPORT9 *pViewport) {
  return m_device->GetViewport(pViewport);
}

COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE OBGEDirect3DDevice9::SetMaterial(CONST D3DMATERIAL9 *pMaterial) {
  if (frame_log) {
    frame_log->Message("SetMaterial:");

    frame_log->Indent();
    frame_log->FormattedMessage("Diffuse: {%f,%f,%f,%f}", pMaterial->Diffuse.r, pMaterial->Diffuse.g, pMaterial->Diffuse.b, pMaterial->Diffuse.a);
    frame_log->FormattedMessage("Ambient: {%f,%f,%f,%f}", pMaterial->Ambient.r, pMaterial->Ambient.g, pMaterial->Ambient.b, pMaterial->Ambient.a);
    frame_log->FormattedMessage("Specular: {%f,%f,%f,%f}", pMaterial->Specular.r, pMaterial->Specular.g, pMaterial->Specular.b, pMaterial->Specular.a);
    frame_log->FormattedMessage("Emissive: {%f,%f,%f,%f}", pMaterial->Emissive.r, pMaterial->Emissive.g, pMaterial->Emissive.b, pMaterial->Emissive.a);
    frame_log->FormattedMessage("Power: %f", pMaterial->Power);
    frame_log->Outdent();
  }

  return m_device->SetMaterial(pMaterial);
}

COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE OBGEDirect3DDevice9::GetMaterial(D3DMATERIAL9 *pMaterial) {
  return m_device->GetMaterial(pMaterial);
}

COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE OBGEDirect3DDevice9::SetLight(DWORD Index, CONST D3DLIGHT9 *pLight) {
  if (frame_log) {
    frame_log->FormattedMessage("Light[%d]:", Index);

    frame_log->Indent();
    frame_log->FormattedMessage("Type: %d", pLight->Type);
    frame_log->FormattedMessage("Diffuse: {%f,%f,%f,%f}", pLight->Diffuse.r, pLight->Diffuse.g, pLight->Diffuse.b, pLight->Diffuse.a);
    frame_log->FormattedMessage("Specular: {%f,%f,%f,%f}", pLight->Specular.r, pLight->Specular.g, pLight->Specular.b, pLight->Specular.a);
    frame_log->FormattedMessage("Ambient: {%f,%f,%f,%f}", pLight->Ambient.r, pLight->Ambient.g, pLight->Ambient.b, pLight->Ambient.a);
    frame_log->FormattedMessage("Position: {%f,%f,%f}", pLight->Position.x, pLight->Position.y, pLight->Position.z);
    frame_log->FormattedMessage("Direction: {%f,%f,%f}", pLight->Direction.x, pLight->Direction.y, pLight->Direction.z);
    frame_log->FormattedMessage("Range: %g", pLight->Range);
    frame_log->FormattedMessage("Falloff: %g", pLight->Falloff);
    frame_log->FormattedMessage("Attenuation0: %g", pLight->Attenuation0);
    frame_log->FormattedMessage("Attenuation1: %g", pLight->Attenuation1);
    frame_log->FormattedMessage("Attenuation2: %g", pLight->Attenuation2);
    frame_log->FormattedMessage("Theta: %g", pLight->Theta);
    frame_log->FormattedMessage("Phi: %g", pLight->Phi);
    frame_log->Outdent();
  }

  return m_device->SetLight(Index, pLight);
}

COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE OBGEDirect3DDevice9::GetLight(DWORD Index, D3DLIGHT9 *pLight) {
  return m_device->GetLight(Index, pLight);
}

COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE OBGEDirect3DDevice9::LightEnable(DWORD Index, BOOL Enable) {
  if (frame_log)
    frame_log->FormattedMessage("Light[%d]: %s", Index, Enable ? "enabled" : "disabled");

  return m_device->LightEnable(Index, Enable);
}

COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE OBGEDirect3DDevice9::GetLightEnable(DWORD Index, BOOL *pEnable) {
  return m_device->GetLightEnable(Index, pEnable);
}

COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE OBGEDirect3DDevice9::SetClipPlane(DWORD Index, CONST float *pPlane) {
  return m_device->SetClipPlane(Index, pPlane);
}

COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE OBGEDirect3DDevice9::GetClipPlane(DWORD Index, float *pPlane) {
  return m_device->GetClipPlane(Index, pPlane);
}

COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE OBGEDirect3DDevice9::SetRenderState(D3DRENDERSTATETYPE State, DWORD Value) {
  if (HasShaderManager) {
    /* blow the fuse, if manually set */
    if (State == D3DRS_ZWRITEENABLE)
      RuntimeShaderRecord::rsb[currentPass].bZLoaded = 0;

#ifdef	OBGE_DEVLING
    /* record exact position of occurance */
    int frame_cntr = m_shaders->trackd[currentPass].frame_cntr;

    /* huh, going into menu provokes huge numbers here */
    if (frame_cntr < OBGESCENE_NUM)
      m_shaders->trackd[currentPass].states[frame_cntr][State] = Value;
#endif
  }

  if (frame_log) {
    frame_log->Message("SetRenderState");
    frame_log->Indent();
    frame_log->FormattedMessage("%s: %s", findRenderState(State), findRenderStateValue(State, Value));
    frame_log->Outdent();
  }

#if	defined(OBGE_DEVLING) && defined(OBGE_TESSELATION)
  if (frame_wre /*&& (State == D3DRS_FILLMODE)*/) {
    if (currentPass == OBGEPASS_MAIN)
      m_device->SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME);
    else
      m_device->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
  }
#endif

  return m_device->SetRenderState(State, Value);
}

COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE OBGEDirect3DDevice9::GetRenderState(D3DRENDERSTATETYPE State, DWORD *pValue) {
  return m_device->GetRenderState(State, pValue);
}

COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE OBGEDirect3DDevice9::CreateStateBlock(D3DSTATEBLOCKTYPE Type, IDirect3DStateBlock9 **ppSB) {
  return m_device->CreateStateBlock(Type, ppSB);
}

COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE OBGEDirect3DDevice9::BeginStateBlock(void) {
  return m_device->BeginStateBlock();
}

COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE OBGEDirect3DDevice9::EndStateBlock(IDirect3DStateBlock9 **ppSB) {
  return m_device->EndStateBlock(ppSB);
}

COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE OBGEDirect3DDevice9::SetClipStatus(CONST D3DCLIPSTATUS9 *pClipStatus) {
  return m_device->SetClipStatus(pClipStatus);
}

COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE OBGEDirect3DDevice9::GetClipStatus(D3DCLIPSTATUS9 *pClipStatus) {
  return m_device->GetClipStatus(pClipStatus);
}

COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE OBGEDirect3DDevice9::GetTexture(DWORD Sampler, IDirect3DBaseTexture9 **ppTexture) {
  return m_device->GetTexture(Sampler, ppTexture);
}

COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE OBGEDirect3DDevice9::SetTexture(DWORD Sampler, IDirect3DBaseTexture9 *pTexture) {
#if	defined(OBGE_DEVLING) && defined(OBGE_PROFILE)
  if (frame_ntx)
    pTexture = NULL;
#endif

  if (pTexture) {
#if	defined(OBGE_ANISOTROPY)
    if (currentPass == OBGEPASS_MAIN) {
      // stablelize AF override, SetTexture is called before any SetSamplerState
      if (AFilters) {
	m_device->SetSamplerState(Sampler, D3DSAMP_MINFILTER, D3DTEXF_ANISOTROPIC);
	m_device->SetSamplerState(Sampler, D3DSAMP_MAXANISOTROPY, *((DWORD *)&Anisotropy));
      }
    }
#endif

#if	defined(OBGE_GAMMACORRECTION)
    /* we have to experiment with this */
    if ((currentPass == OBGEPASS_REFLECTION) ||
	(currentPass == OBGEPASS_MAIN)) {
      /* gamma-correction on read */
      bool _DeGamma; DWORD _DGs;
      if (DeGamma && (pTexture->GetPrivateData(GammaGUID, &_DeGamma, &_DGs) == D3D_OK))
        m_device->SetSamplerState(Sampler, D3DSAMP_SRGBTEXTURE, DeGamma);
      else
        m_device->SetSamplerState(Sampler, D3DSAMP_SRGBTEXTURE, false);
    }
#endif
  }

  if (HasShaderManager) {
    assert(Sampler < 512);
    DWORD tSampler = dx2obgeSampler[Sampler];
    assert(tSampler < OBGESAMPLER_NUM);

#if 1
    /* shaders may like to get some absolute texture information
     * I hope this is quick ...
     */
    if ((m_shaders->GlobalConst.pTexture[tSampler].vals.texture.texture = pTexture)) {
      D3DSURFACE_DESC desc; int levels =

      ((IDirect3DTexture9 *)pTexture)->GetLevelCount();
      ((IDirect3DTexture9 *)pTexture)->GetLevelDesc(0, &desc);

      m_shaders->GlobalConst.pTexture[tSampler].vals.texture.data[0] = desc.Width;
      m_shaders->GlobalConst.pTexture[tSampler].vals.texture.data[1] = desc.Height;
      m_shaders->GlobalConst.pTexture[tSampler].vals.texture.data[2] = levels;
      m_shaders->GlobalConst.pTexture[tSampler].vals.texture.data[3] = 0;
    }
#endif

#ifdef	OBGE_DEVLING
    m_shaders->traced[currentPass].values_s[tSampler] = pTexture;
#endif
  }

  if (frame_log) {
    frame_log->FormattedMessage("SetTexture[%d]", Sampler);
    frame_log->Indent();
    frame_log->FormattedMessage("Address: 0x%08x", pTexture);

#if	defined(OBGE_LOGGING)
    if (pTexture)
      frame_log->FormattedMessage("Path: %s", findTexture(pTexture));
#endif

    bool ok = false;

    if (!pTexture) {
      ok = true;
    }

    if (!ok) {
      struct textureSurface *track = surfaceTexture[pTexture];

      if (track) {
        frame_log->Message("Type: Dedicated Texture");
        frame_log->FormattedMessage("{W,H}: {%d,%d}", track->map->Width, track->map->Height);
        frame_log->FormattedMessage("Format: %s", findFormat(track->map->Format));
        frame_log->FormattedMessage("Levels: %d", track->map->Levels);
        frame_log->FormattedMessage("Usage: %s", findUsage(track->map->Usage));

        ok = true;
      }
    }

    frame_log->Outdent();
  }

#if	defined(OBGE_AUTOMIPMAP) && (OBGE_AUTOMIPMAP > 0)
  struct textureMap *track;
  bool setfilter = false;

  if (pTexture && (track = textureMaps[pTexture])) {
    if ((track->Usage & D3DUSAGE_AUTOGENMIPMAP) &&
        (track->Usage & D3DUSAGE_RENDERTARGET)) {
      pTexture->SetAutoGenFilterType(AMFilter);
      pTexture->GenerateMipSubLevels();
    }
  }
#endif

//assert(!pTexture || (pTexture != passTexture[OBGEPASS_ANY]));
  HRESULT res = m_device->SetTexture(Sampler, pTexture);
  return res;
}

COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE OBGEDirect3DDevice9::GetTextureStageState(DWORD Stage, D3DTEXTURESTAGESTATETYPE Type, DWORD *pValue) {
  return m_device->GetTextureStageState(Stage, Type, pValue);
}

COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE OBGEDirect3DDevice9::SetTextureStageState(DWORD Stage, D3DTEXTURESTAGESTATETYPE Type, DWORD Value) {
  return m_device->SetTextureStageState(Stage, Type, Value);
}

COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE OBGEDirect3DDevice9::GetSamplerState(DWORD Sampler, D3DSAMPLERSTATETYPE Type, DWORD *pValue) {
  return m_device->GetSamplerState(Sampler, Type, pValue);
}

COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE OBGEDirect3DDevice9::SetSamplerState(DWORD Sampler, D3DSAMPLERSTATETYPE Type, DWORD Value) {
#if	defined(OBGE_ANISOTROPY)
  if (currentPass == OBGEPASS_MAIN) {
    // MIP
    if (((1 << Type) & ALODs   ) && (Value >= D3DTEXF_LINEAR))
      m_device->SetSamplerState(Sampler, D3DSAMP_MIPMAPLODBIAS, *((DWORD *)&LODBias));
    // MIN & MAG
    if (((1 << Type) & AFilters) && (Value == D3DTEXF_LINEAR)) {
      m_device->SetSamplerState(Sampler, D3DSAMP_MAXANISOTROPY, *((DWORD *)&Anisotropy));

      Value = D3DTEXF_ANISOTROPIC;
    }
  }
#endif

  if (frame_log) {
    frame_log->FormattedMessage("SetSamplerState[%d]", Sampler);
    frame_log->Indent();
    frame_log->FormattedMessage("%s: %s", findSamplerState(Type), findSamplerStateValue(Type, Value));
    frame_log->Outdent();
  }

  if (HasShaderManager) {
    assert(Sampler < 512);
    DWORD tSampler = dx2obgeSampler[Sampler];
    assert(tSampler < OBGESAMPLER_NUM);

#ifdef	OBGE_DEVLING
    m_shaders->traced[currentPass].states_s[tSampler][Type] = Value;
#endif
  }

  HRESULT res = m_device->SetSamplerState(Sampler, Type, Value);
  assert(res == D3D_OK);
  return res;
}

COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE OBGEDirect3DDevice9::ValidateDevice(DWORD *pNumPasses) {
  return m_device->ValidateDevice(pNumPasses);
}

COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE OBGEDirect3DDevice9::SetPaletteEntries(UINT PaletteNumber, CONST PALETTEENTRY *pEntries) {
  return m_device->SetPaletteEntries(PaletteNumber, pEntries);
}

COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE OBGEDirect3DDevice9::GetPaletteEntries(UINT PaletteNumber, PALETTEENTRY *pEntries) {
  return m_device->GetPaletteEntries(PaletteNumber, pEntries);
}

COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE OBGEDirect3DDevice9::SetCurrentTexturePalette(UINT PaletteNumber) {
  return m_device->SetCurrentTexturePalette(PaletteNumber);
}

COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE OBGEDirect3DDevice9::GetCurrentTexturePalette(UINT *PaletteNumber) {
  return m_device->GetCurrentTexturePalette(PaletteNumber);
}

COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE OBGEDirect3DDevice9::SetScissorRect(CONST RECT *pRect) {
  return m_device->SetScissorRect(pRect);
}

COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE OBGEDirect3DDevice9::GetScissorRect(RECT *pRect) {
  return m_device->GetScissorRect(pRect);
}

COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE OBGEDirect3DDevice9::SetSoftwareVertexProcessing(BOOL bSoftware) {
  return m_device->SetSoftwareVertexProcessing(bSoftware);
}

COM_DECLSPEC_NOTHROW BOOL STDMETHODCALLTYPE OBGEDirect3DDevice9::GetSoftwareVertexProcessing(void) {
  return m_device->GetSoftwareVertexProcessing();
}

COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE OBGEDirect3DDevice9::SetNPatchMode(float nSegments) {
  return m_device->SetNPatchMode(nSegments);
}

COM_DECLSPEC_NOTHROW float STDMETHODCALLTYPE OBGEDirect3DDevice9::GetNPatchMode(void) {
  return m_device->GetNPatchMode();
}

COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE OBGEDirect3DDevice9::DrawPrimitive(D3DPRIMITIVETYPE PrimitiveType, UINT StartVertex, UINT PrimitiveCount) {
#ifdef	OBGE_DEVLING
  if (m_shadercv && m_shadercp) {
    m_shadercv->Paired.insert(m_shadercp);
    m_shadercp->Paired.insert(m_shadercv);
  }
#endif

  if (frame_log) {
    frame_log->Message("DrawPrimitive");

    const char *Type = "Unknown";

    switch (PrimitiveType) {
      case D3DPT_POINTLIST:     Type = "PointList"; break;
      case D3DPT_LINELIST:      Type = "LineList"; break;
      case D3DPT_LINESTRIP:     Type = "LineStrip"; break;
      case D3DPT_TRIANGLELIST:  Type = "TriangleList"; break;
      case D3DPT_TRIANGLESTRIP: Type = "TriangleStrip"; break;
      case D3DPT_TRIANGLEFAN:   Type = "TriangleFan"; break;
    }

    frame_log->Indent();
    frame_log->FormattedMessage("Type: %s", Type);
    frame_log->FormattedMessage("Count: %d", PrimitiveCount);
    frame_log->Outdent();
  }

  return m_device->DrawPrimitive(PrimitiveType, StartVertex, PrimitiveCount);
}

COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE OBGEDirect3DDevice9::DrawIndexedPrimitive(D3DPRIMITIVETYPE PrimitiveType, INT BaseVertexIndex, UINT MinVertexIndex, UINT NumVertices, UINT startIndex, UINT primCount) {
#ifdef	OBGE_DEVLING
  if (m_shadercv && m_shadercp) {
    m_shadercv->Paired.insert(m_shadercp);
    m_shadercp->Paired.insert(m_shadercv);
  }
#endif

  if (frame_log) {
    frame_log->Message("DrawIndexedPrimitive");

    const char *Type = "Unknown";

    switch (PrimitiveType) {
      case D3DPT_POINTLIST:     Type = "PointList"; break;
      case D3DPT_LINELIST:      Type = "LineList"; break;
      case D3DPT_LINESTRIP:     Type = "LineStrip"; break;
      case D3DPT_TRIANGLELIST:  Type = "TriangleList"; break;
      case D3DPT_TRIANGLESTRIP: Type = "TriangleStrip"; break;
      case D3DPT_TRIANGLEFAN:   Type = "TriangleFan"; break;
    }

    frame_log->Indent();
    frame_log->FormattedMessage("Type: %s", Type);
    frame_log->FormattedMessage("Count: %d", primCount);
    frame_log->FormattedMessage("Vertices: %d", NumVertices);
    frame_log->Outdent();
  }

  HRESULT res;

#if	defined(OBGE_DEVLING) && defined(OBGE_TESSELATION)
  if (shadr_tes) {
    TSPrimitiveType TSPrimType = TSPT_TRIANGLELIST;

    switch (PrimitiveType) {
       case D3DPT_TRIANGLELIST: TSPrimType = TSPT_TRIANGLELIST; break;
       case D3DPT_TRIANGLESTRIP: TSPrimType = TSPT_TRIANGLESTRIP; break;
    }

    // Enable tessellation
    pATITessInterface->SetMode(TSMD_ENABLE_CONTINUOUS);
    res = pATITessInterface->DrawIndexed(TSPrimType, BaseVertexIndex, MinVertexIndex, NumVertices, startIndex, primCount);
    pATITessInterface->SetMode(TSMD_DISABLE);
  }
  else
#endif

  res = m_device->DrawIndexedPrimitive(PrimitiveType, BaseVertexIndex, MinVertexIndex, NumVertices, startIndex, primCount);

  return res;
}

COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE OBGEDirect3DDevice9::DrawPrimitiveUP(D3DPRIMITIVETYPE PrimitiveType, UINT PrimitiveCount, CONST void *pVertexStreamZeroData, UINT VertexStreamZeroStride) {
#ifdef	OBGE_DEVLING
  if (m_shadercv && m_shadercp) {
    m_shadercv->Paired.insert(m_shadercp);
    m_shadercp->Paired.insert(m_shadercv);
  }
#endif

  if (frame_log) {
    frame_log->Message("DrawPrimitiveUP");

    const char *Type = "Unknown";

    switch (PrimitiveType) {
      case D3DPT_POINTLIST:     Type = "PointList"; break;
      case D3DPT_LINELIST:      Type = "LineList"; break;
      case D3DPT_LINESTRIP:     Type = "LineStrip"; break;
      case D3DPT_TRIANGLELIST:  Type = "TriangleList"; break;
      case D3DPT_TRIANGLESTRIP: Type = "TriangleStrip"; break;
      case D3DPT_TRIANGLEFAN:   Type = "TriangleFan"; break;
    }

    frame_log->Indent();
    frame_log->FormattedMessage("Type: %s", Type);
    frame_log->FormattedMessage("Count: %d", PrimitiveCount);
    frame_log->FormattedMessage("Stride: %d", VertexStreamZeroStride);
    frame_log->Outdent();
  }

  HRESULT res = m_device->DrawPrimitiveUP(PrimitiveType, PrimitiveCount, pVertexStreamZeroData, VertexStreamZeroStride);
  return res;
}

COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE OBGEDirect3DDevice9::DrawIndexedPrimitiveUP(D3DPRIMITIVETYPE PrimitiveType, UINT MinVertexIndex, UINT NumVertices, UINT PrimitiveCount, CONST void *pIndexData, D3DFORMAT IndexDataFormat, CONST void *pVertexStreamZeroData, UINT VertexStreamZeroStride) {
#ifdef	OBGE_DEVLING
  if (m_shadercv && m_shadercp) {
    m_shadercv->Paired.insert(m_shadercp);
    m_shadercp->Paired.insert(m_shadercv);
  }
#endif

  if (frame_log) {
    frame_log->Message("DrawIndexedPrimitiveUP");

    const char *Type = "Unknown";

    switch (PrimitiveType) {
      case D3DPT_POINTLIST:     Type = "PointList"; break;
      case D3DPT_LINELIST:      Type = "LineList"; break;
      case D3DPT_LINESTRIP:     Type = "LineStrip"; break;
      case D3DPT_TRIANGLELIST:  Type = "TriangleList"; break;
      case D3DPT_TRIANGLESTRIP: Type = "TriangleStrip"; break;
      case D3DPT_TRIANGLEFAN:   Type = "TriangleFan"; break;
    }

    frame_log->Indent();
    frame_log->FormattedMessage("Type: %s", Type);
    frame_log->FormattedMessage("Format: %s", findFormat(IndexDataFormat));
    frame_log->FormattedMessage("Count: %d", PrimitiveCount);
    frame_log->FormattedMessage("Vertices: %d", NumVertices);
    frame_log->FormattedMessage("Stride: %d", VertexStreamZeroStride);
    frame_log->Outdent();
  }

  return m_device->DrawIndexedPrimitiveUP(PrimitiveType, MinVertexIndex, NumVertices, PrimitiveCount, pIndexData, IndexDataFormat, pVertexStreamZeroData, VertexStreamZeroStride);
}

COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE OBGEDirect3DDevice9::ProcessVertices(UINT SrcStartIndex, UINT DestIndex, UINT VertexCount, IDirect3DVertexBuffer9 *pDestBuffer, IDirect3DVertexDeclaration9 *pVertexDecl, DWORD Flags) {
  return m_device->ProcessVertices(SrcStartIndex, DestIndex, VertexCount, pDestBuffer, pVertexDecl, Flags);
}

COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE OBGEDirect3DDevice9::CreateVertexDeclaration(CONST D3DVERTEXELEMENT9 *pVertexElements, IDirect3DVertexDeclaration9 **ppDecl) {
  return m_device->CreateVertexDeclaration(pVertexElements, ppDecl);
}

COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE OBGEDirect3DDevice9::SetVertexDeclaration(IDirect3DVertexDeclaration9 *pDecl) {
  return m_device->SetVertexDeclaration(pDecl);
}

COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE OBGEDirect3DDevice9::GetVertexDeclaration(IDirect3DVertexDeclaration9 **ppDecl) {
  return m_device->GetVertexDeclaration(ppDecl);
}

COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE OBGEDirect3DDevice9::SetFVF(DWORD FVF) {
  if (frame_log)
    frame_log->FormattedMessage("SetFVF:%s", findFVF(FVF));

#if	defined(OBGE_DEVLING)
//m_shadercv->traced[currentPass].vertex_f = FVF;
//m_shadercp->traced[currentPass].vertex_f = FVF;
#endif

  return m_device->SetFVF(FVF);
}

COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE OBGEDirect3DDevice9::GetFVF(DWORD *pFVF) {
  return m_device->GetFVF(pFVF);
}

COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE OBGEDirect3DDevice9::CreateVertexShader(CONST DWORD *pFunction, IDirect3DVertexShader9 **ppShader) {
  if (frame_log)
    frame_log->FormattedMessage("CreateVertexShader 0x%08x", _ReturnAddress());

  HRESULT res = m_device->CreateVertexShader(pFunction, ppShader);

  if (frame_log || frame_trk) {
    /* just register, don't return any manipulated class */
    if (HasShaderManager)
      m_shaders->SetRuntimeShader(pFunction, *ppShader);

    if (frame_log) {
      UINT len = 0;

      if (*ppShader) (*ppShader)->GetFunction(NULL, &len);

      const char *nam = findShader(*ppShader, len, pFunction);

      frame_log->Indent();
      frame_log->FormattedMessage("Length: %d", len);
      frame_log->FormattedMessage("Name: %s", nam);
      frame_log->Outdent();
    }
  }

  return res;
}

COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE OBGEDirect3DDevice9::SetVertexShader(IDirect3DVertexShader9 *pShader) {
  HRESULT res = 0;
  m_shadercv = NULL;

  if (HasShaderManager && pShader && (m_shadercv = m_shaders->GetRuntimeShader(pShader))) {
#if	!defined(OBGE_DEVLING)
    /* set shader first before constants */
    res = m_device->SetVertexShader(pShader);
    /* apply dynamic runtime parameters */
    m_shadercv->SetRuntimeParams(m_device, m_device);
#else
    m_shadercv->frame_used[OBGEPASS_ANY] = frame_num;
    m_shadercv->frame_pass[OBGEPASS_ANY] = frame_bge;

    m_shadercv->frame_used[currentPass] = frame_num;
    m_shadercv->frame_pass[currentPass] = frame_bge;

    m_shadercv->Clear(currentPass);

    /* it's impossible for that function to return NULL */
    res = m_device->SetVertexShader(m_shadercv->GetShader(pShader));
    /* apply dynamic runtime parameters */
    m_shadercv->SetRuntimeParams(this, m_device);

    /* these have been set before, takeover */
    memcpy(m_shadercv->traced[currentPass].states_s, m_shaders->traced[currentPass].states_s, sizeof(m_shaders->traced[currentPass].states_s));
    memcpy(m_shadercv->traced[currentPass].values_s, m_shaders->traced[currentPass].values_s, sizeof(m_shaders->traced[currentPass].values_s));

    if (frame_log) {
      char buf[256];
      sprintf(buf, "Name: %s", m_shadercv->pAssociate ? m_shadercv->pAssociate->Name : "unknown");

      frame_log->Message("SetVertexShader");
      frame_log->Indent();
      frame_log->Message(buf);
      frame_log->Outdent();
    }
#endif
  }
  else if (frame_log) {
    char buf[256];
    sprintf(buf, "Name: %s", pShader ? "untracked" : "cleared");

    frame_log->Message("SetVertexShader");
    frame_log->Indent();
    frame_log->Message(buf);
    frame_log->Outdent();

    res = m_device->SetVertexShader(pShader);
  }
  else
    res = m_device->SetVertexShader(pShader);

  return res;
}

COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE OBGEDirect3DDevice9::GetVertexShader(IDirect3DVertexShader9 **ppShader) {
  return m_device->GetVertexShader(ppShader);
}

COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE OBGEDirect3DDevice9::SetVertexShaderConstantF(UINT StartRegister, CONST float *pConstantData, UINT Vector4fCount) {
#ifdef	OBGE_DEVLING
  if (m_shadercv) {
    memcpy(
      m_shadercv->traced[currentPass].values_c[StartRegister],
      pConstantData,
      Vector4fCount * 4 * sizeof(float)
    );
  }
#endif

  if (frame_log) {
    frame_log->FormattedMessage("SetVertexShaderConstantF[%d+]", StartRegister);
    frame_log->Indent();

    for (int v = 0; v < Vector4fCount; v++)
      frame_log->FormattedMessage("|%f|%f|%f|%f|", pConstantData[4 * v + 0], pConstantData[4 * v + 1], pConstantData[4 * v + 2], pConstantData[4 * v + 3]);

    frame_log->Outdent();
  }

  return m_device->SetVertexShaderConstantF(StartRegister, pConstantData, Vector4fCount);
}

COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE OBGEDirect3DDevice9::GetVertexShaderConstantF(UINT StartRegister, float *pConstantData, UINT Vector4fCount) {
  return m_device->GetVertexShaderConstantF(StartRegister, pConstantData, Vector4fCount);
}

COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE OBGEDirect3DDevice9::SetVertexShaderConstantI(UINT StartRegister, CONST int *pConstantData, UINT Vector4iCount) {
#ifdef	OBGE_DEVLING
  if (m_shadercv) {
    memcpy(
      m_shadercv->traced[currentPass].values_i[StartRegister],
      pConstantData,
      Vector4iCount * 4 * sizeof(int)
    );
  }
#endif

  if (frame_log) {
    frame_log->FormattedMessage("SetVertexShaderConstantI[%d+]", StartRegister);
    frame_log->Indent();

    for (int v = 0; v < Vector4iCount; v++)
      frame_log->FormattedMessage("|%d|%d|%d|%d|", pConstantData[4 * v + 0], pConstantData[4 * v + 1], pConstantData[4 * v + 2], pConstantData[4 * v + 3]);

    frame_log->Outdent();
  }

  return m_device->SetVertexShaderConstantI(StartRegister, pConstantData, Vector4iCount);
}

COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE OBGEDirect3DDevice9::GetVertexShaderConstantI(UINT StartRegister, int *pConstantData, UINT Vector4iCount) {
  return m_device->GetVertexShaderConstantI(StartRegister, pConstantData, Vector4iCount);
}

COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE OBGEDirect3DDevice9::SetVertexShaderConstantB(UINT StartRegister, CONST BOOL *pConstantData, UINT  BoolCount) {
  if (frame_log) {
    frame_log->FormattedMessage("SetVertexShaderConstantB[%d+]", StartRegister);
    frame_log->Indent();

    for (int v = 0; v < BoolCount; v++)
      frame_log->FormattedMessage("|%d|", pConstantData[v]);

    frame_log->Outdent();
  }

  return m_device->SetVertexShaderConstantB(StartRegister, pConstantData, BoolCount);
}

COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE OBGEDirect3DDevice9::GetVertexShaderConstantB(UINT StartRegister, BOOL *pConstantData, UINT BoolCount) {
  return m_device->GetVertexShaderConstantB(StartRegister, pConstantData, BoolCount);
}

COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE OBGEDirect3DDevice9::SetStreamSource(UINT StreamNumber, IDirect3DVertexBuffer9 *pStreamData, UINT OffsetInBytes, UINT Stride) {
  return m_device->SetStreamSource(StreamNumber, pStreamData, OffsetInBytes, Stride);
}

COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE OBGEDirect3DDevice9::GetStreamSource(UINT StreamNumber, IDirect3DVertexBuffer9 **ppStreamData, UINT *pOffsetInBytes, UINT *pStride) {
  return m_device->GetStreamSource(StreamNumber, ppStreamData, pOffsetInBytes, pStride);
}

COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE OBGEDirect3DDevice9::SetStreamSourceFreq(UINT StreamNumber, UINT Setting) {
  return m_device->SetStreamSourceFreq(StreamNumber, Setting);
}

COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE OBGEDirect3DDevice9::GetStreamSourceFreq(UINT StreamNumber, UINT *pSetting) {
  return m_device->GetStreamSourceFreq(StreamNumber, pSetting);
}

COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE OBGEDirect3DDevice9::SetIndices(IDirect3DIndexBuffer9 *pIndexData) {
  return m_device->SetIndices(pIndexData);
}

COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE OBGEDirect3DDevice9::GetIndices(IDirect3DIndexBuffer9 **ppIndexData) {
  return m_device->GetIndices(ppIndexData);
}

COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE OBGEDirect3DDevice9::CreatePixelShader(CONST DWORD *pFunction, IDirect3DPixelShader9 **ppShader) {
  if (frame_log)
    frame_log->FormattedMessage("CreatePixelShader 0x%08x", _ReturnAddress());

  HRESULT res = m_device->CreatePixelShader(pFunction, ppShader);

  if (frame_log || frame_trk) {
    /* just register, don't return any manipulated class */
    if (HasShaderManager)
      m_shaders->SetRuntimeShader(pFunction, *ppShader);

    if (frame_log) {
      UINT len = 0;

      if (*ppShader) (*ppShader)->GetFunction(NULL, &len);

      const char *nam = findShader(*ppShader, len, pFunction);

      frame_log->Indent();
      frame_log->FormattedMessage("Length: %d", len);
      frame_log->FormattedMessage("Name: %s", nam);
      frame_log->Outdent();
    }
  }

  return res;
}

COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE OBGEDirect3DDevice9::SetPixelShader(IDirect3DPixelShader9 *pShader) {
  HRESULT res = 0;
  m_shadercp = NULL;

  if (HasShaderManager && pShader && (m_shadercp = m_shaders->GetRuntimeShader(pShader))) {
#if	!defined(OBGE_DEVLING)
    /* set shader first before constants */
    res = m_device->SetPixelShader(pShader);
    /* apply dynamic runtime parameters */
    m_shadercp->SetRuntimeParams(m_device, m_device);
#else
    m_shadercp->frame_used[OBGEPASS_ANY] = frame_num;
    m_shadercp->frame_pass[OBGEPASS_ANY] = frame_bge;

    m_shadercp->frame_used[currentPass] = frame_num;
    m_shadercp->frame_pass[currentPass] = frame_bge;

    m_shadercp->Clear(currentPass);

    /* it's impossible for that function to return NULL */
    res = m_device->SetPixelShader(m_shadercp->GetShader(pShader));
    /* apply dynamic runtime parameters */
    m_shadercp->SetRuntimeParams(this, m_device);

    /* these have been set before, takeover */
    memcpy(m_shadercp->traced[currentPass].states_s, m_shaders->traced[currentPass].states_s, sizeof(m_shaders->traced[currentPass].states_s));
    memcpy(m_shadercp->traced[currentPass].values_s, m_shaders->traced[currentPass].values_s, sizeof(m_shaders->traced[currentPass].values_s));

    if (frame_log) {
      char buf[256];
      sprintf(buf, "Name: %s", m_shadercp->pAssociate ? m_shadercp->pAssociate->Name : "unknown");

      frame_log->Message("SetPixelShader");
      frame_log->Indent();
      frame_log->Message(buf);
      frame_log->Outdent();
    }
#endif
  }
  else if (frame_log) {
    char buf[256];
    sprintf(buf, "Name: %s", pShader ? "untracked" : "cleared");

    frame_log->Message("SetPixelShader");
    frame_log->Indent();
    frame_log->Message(buf);
    frame_log->Outdent();

    res = m_device->SetPixelShader(pShader);
  }
  else
    res = m_device->SetPixelShader(pShader);

  return res;
}

COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE OBGEDirect3DDevice9::GetPixelShader(IDirect3DPixelShader9 **ppShader) {
  return m_device->GetPixelShader(ppShader);
}

COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE OBGEDirect3DDevice9::SetPixelShaderConstantF(UINT StartRegister, CONST float *pConstantData, UINT Vector4fCount) {
#ifdef	OBGE_DEVLING
  if (m_shadercp) {
    memcpy(
      m_shadercp->traced[currentPass].values_c[StartRegister],
      pConstantData,
      Vector4fCount * 4 * sizeof(float)
    );
  }
#endif

  if (frame_log) {
    frame_log->FormattedMessage("SetPixelShaderConstantF[%d+]", StartRegister);
    frame_log->Indent();

    for (int v = 0; v < Vector4fCount; v++)
      frame_log->FormattedMessage("|%f|%f|%f|%f|", pConstantData[4 * v + 0], pConstantData[4 * v + 1], pConstantData[4 * v + 2], pConstantData[4 * v + 3]);

    frame_log->Outdent();
  }

  return m_device->SetPixelShaderConstantF(StartRegister, pConstantData, Vector4fCount);
}

COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE OBGEDirect3DDevice9::GetPixelShaderConstantF(UINT StartRegister, float *pConstantData, UINT Vector4fCount) {
  return m_device->GetPixelShaderConstantF(StartRegister, pConstantData, Vector4fCount);
}

COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE OBGEDirect3DDevice9::SetPixelShaderConstantI(UINT StartRegister, CONST int *pConstantData, UINT Vector4iCount) {
#ifdef	OBGE_DEVLING
  if (m_shadercp) {
    memcpy(
      m_shadercp->traced[currentPass].values_i[StartRegister],
      pConstantData,
      Vector4iCount * 4 * sizeof(int)
    );
  }
#endif

  if (frame_log) {
    frame_log->FormattedMessage("SetPixelShaderConstantI[%d+]", StartRegister);
    frame_log->Indent();

    for (int v = 0; v < Vector4iCount; v++)
      frame_log->FormattedMessage("|%d|%d|%d|%d|", pConstantData[4 * v + 0], pConstantData[4 * v + 1], pConstantData[4 * v + 2], pConstantData[4 * v + 3]);

    frame_log->Outdent();
  }

  return m_device->SetPixelShaderConstantI(StartRegister, pConstantData, Vector4iCount);
}

COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE OBGEDirect3DDevice9::GetPixelShaderConstantI(UINT StartRegister, int *pConstantData, UINT Vector4iCount) {
  return m_device->GetPixelShaderConstantI(StartRegister, pConstantData, Vector4iCount);
}

COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE OBGEDirect3DDevice9::SetPixelShaderConstantB(UINT StartRegister, CONST BOOL *pConstantData, UINT  BoolCount) {
#ifdef	OBGE_DEVLING
  if (m_shadercp) {
    memcpy(
      m_shadercp->traced[currentPass].values_b + StartRegister,
      pConstantData,
      BoolCount * sizeof(bool)
    );
  }
#endif

  if (frame_log) {
    frame_log->FormattedMessage("SetPixelShaderConstantB[%d+]", StartRegister);
    frame_log->Indent();

    for (int v = 0; v < BoolCount; v++)
      frame_log->FormattedMessage("|%d|", pConstantData[v]);

    frame_log->Outdent();
  }

  return m_device->SetPixelShaderConstantB(StartRegister, pConstantData, BoolCount);
}

COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE OBGEDirect3DDevice9::GetPixelShaderConstantB(UINT StartRegister, BOOL *pConstantData, UINT BoolCount) {
  return m_device->GetPixelShaderConstantB(StartRegister, pConstantData, BoolCount);
}

COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE OBGEDirect3DDevice9::DrawRectPatch(UINT Handle, CONST float *pNumSegs, CONST D3DRECTPATCH_INFO *pRectPatchInfo) {
  return m_device->DrawRectPatch(Handle, pNumSegs, pRectPatchInfo);
}

COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE OBGEDirect3DDevice9::DrawTriPatch(UINT Handle, CONST float *pNumSegs, CONST D3DTRIPATCH_INFO *pTriPatchInfo) {
  return m_device->DrawTriPatch(Handle, pNumSegs, pTriPatchInfo);
}

COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE OBGEDirect3DDevice9::DeletePatch(UINT Handle) {
  return m_device->DeletePatch(Handle);
}

COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE OBGEDirect3DDevice9::CreateQuery(D3DQUERYTYPE Type, IDirect3DQuery9 **ppQuery) {
  return m_device->CreateQuery(Type, ppQuery);
}

COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE OBGEDirect3DDevice9::DumpFrameScript(void) {
#ifdef	OBGE_LOGGING
  /* log the next 2 frames */
  frame_dmp = 15;
#endif

  return D3D_OK;
}

COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE OBGEDirect3DDevice9::DumpFrameSurfaces(void) {
  return D3D_OK;
}

#endif
