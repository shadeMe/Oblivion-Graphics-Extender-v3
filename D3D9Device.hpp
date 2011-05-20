#ifndef	D3D9DEVICE_HPP
#define	D3D9DEVICE_HPP

#include <intrin.h>
#include <d3d9.h>
#include <d3dx9.h>

#include "D3D9.hpp"
#include "D3D9Identifiers.hpp"
#include "TextureIOHook.hpp"

/* ----------------------------------------------------------------------------- */

// Hook-Tracker
extern enum OBGEPass currentPass, previousPass;
extern IDirect3DTexture9 *passTexture[OBGEPASS_NUM];
extern IDirect3DSurface9 *passSurface[OBGEPASS_NUM];
extern IDirect3DSurface9 *passDepth  [OBGEPASS_NUM];
extern int                passFrames [OBGEPASS_NUM];
extern int                passPasses [OBGEPASS_NUM];
extern const char        *passNames  [OBGEPASS_NUM];

/* ----------------------------------------------------------------------------- */

/* hacking CreateTexure passed via the RenderSurfaceParameters-hook */
extern DWORD specialUsage;

/* ------------------------------------------------------------------------------- */

#if	defined(OBGE_LOGGING)
extern int frame_num;
extern int frame_bge;
extern int frame_dmp;
extern IDebugLog *frame_log;
#elif	defined(OBGE_DEVLING)
extern int frame_num;
extern int frame_bge;
#define	frame_log ((IDebugLog *)NULL)
#else
#define	frame_log ((IDebugLog *)NULL)
#endif

#if	defined(OBGE_PROFILE)
extern LARGE_INTEGER frame_bgn;
extern LARGE_INTEGER frame_end;
#endif

extern bool frame_trk;

/* ------------------------------------------------------------------------------- */

#include <map>

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

/* ----------------------------------------------------------------------------- */
// Tracker
class OBGEDirect3DDevice9; extern OBGEDirect3DDevice9 *lastOBGEDirect3DDevice9;

#if	defined(OBGE_TRACKER_SURFACES) || defined(OBGE_TRACKER_TEXTURES)
#include "D3D9Texture.hpp"
#include "D3D9Surface.hpp"
#endif

#include "ShaderManager.h"
#include "GUIs_DebugWindow.hpp"

class OBGEDirect3DDevice9 : public IDirect3DDevice9
{
public:
	// We need d3d so that we'd use a pointer to OBGEDirect3D9 instead of the original IDirect3D9 implementor
	// in functions like GetDirect3D9
	OBGEDirect3DDevice9(IDirect3D9* d3d, IDirect3DDevice9* device) : m_d3d(d3d), m_device(device)
	{
		lastOBGEDirect3DDevice9 = this;

		_MESSAGE("OD3D9: Device constructed from 0x%08x", _ReturnAddress());

#if	defined(OBGE_LOGGING)
		Sleep(20000);		// Give me enough time to attach to process for debugging. Vista seems to block JIT debugging.

		/* setup logging */
		frame_dmp = 0;
		frame_trk = true;
		frame_log = NULL;

		frame_num = 0;
		frame_bge = 0;
#elif	defined(OBGE_DEVLING)
		frame_num = 0;
		frame_bge = 0;
#endif

#define HasShaderManager	1	// m_shaders
		m_shaders = ShaderManager::GetSingleton();
		m_shadercv = NULL;
		m_shadercp = NULL;

#if	defined(OBGE_DEVLING)
		/* just for now */
		DebugWindow::Expunge();

	//	assert(NULL);
#endif
	}

	~OBGEDirect3DDevice9()
	{
		lastOBGEDirect3DDevice9 = NULL;

      		std::map<void *, struct renderSurface *>::iterator sR = surfaceRender.begin();
      		while (sR != surfaceRender.end()) { if (sR->second) delete sR->second; sR++; } surfaceRender.clear();

      		std::map<void *, struct depthSurface *>::iterator sD = surfaceDepth.begin();
      		while (sD != surfaceDepth.end()) { if (sD->second) delete sD->second; sD++; } surfaceDepth.clear();

      		std::map<void *, struct textureSurface *>::iterator sT = surfaceTexture.begin();
      		while (sT != surfaceTexture.end()) { if (sT->second) delete sT->second; sT++;
      		/*if (sT->first) ((IDirect3DSurface9 *)sT->first)->Release();*/ } surfaceTexture.clear();

      		std::map<void *, struct textureMap *>::iterator tM = textureMaps.begin();
      		while (tM != textureMaps.end()) { if (tM->second) delete tM->second; tM++; } textureMaps.clear();

#ifdef	OBGE_DEVLING
		/* just for now */
		DebugWindow::Destroy();
#endif
	}

	/*** IUnknown methods ***/
	STDMETHOD(QueryInterface)(THIS_ REFIID riid, void** ppvObj)
	{
		return m_device->QueryInterface(riid, ppvObj);
	}

	STDMETHOD_(ULONG,AddRef)(THIS)
	{
		return m_device->AddRef();
	}

	STDMETHOD_(ULONG,Release)(THIS)
	{
		ULONG count = m_device->Release();
		if (0 == count)
			delete this;

		return count;
	}

	/*** IDirect3DDevice9 methods ***/
	STDMETHOD(TestCooperativeLevel)(THIS)
	{
		return m_device->TestCooperativeLevel();
	}

	STDMETHOD_(UINT, GetAvailableTextureMem)(THIS)
	{
		return m_device->GetAvailableTextureMem();
	}

	STDMETHOD(EvictManagedResources)(THIS)
	{
		return m_device->EvictManagedResources();
	}

	STDMETHOD(GetDirect3D)(THIS_ IDirect3D9** ppD3D9)
	{
		// Let the device validate the incoming pointer for us
		HRESULT hr = m_device->GetDirect3D(ppD3D9);
		if(SUCCEEDED(hr))
			*ppD3D9 = m_d3d;

		return hr;
	}

	STDMETHOD(GetDeviceCaps)(THIS_ D3DCAPS9* pCaps)
	{
		return m_device->GetDeviceCaps(pCaps);
	}

	STDMETHOD(GetDisplayMode)(THIS_ UINT iSwapChain,D3DDISPLAYMODE* pMode)
	{
		return m_device->GetDisplayMode(iSwapChain, pMode);
	}

	STDMETHOD(GetCreationParameters)(THIS_ D3DDEVICE_CREATION_PARAMETERS *pParameters)
	{
		return m_device->GetCreationParameters(pParameters);
	}

	STDMETHOD(SetCursorProperties)(THIS_ UINT XHotSpot,UINT YHotSpot,IDirect3DSurface9* pCursorBitmap)
	{
		return m_device->SetCursorProperties(XHotSpot, YHotSpot, pCursorBitmap);
	}

	STDMETHOD_(void, SetCursorPosition)(THIS_ int X,int Y,DWORD Flags)
	{
		m_device->SetCursorPosition(X, Y, Flags);
	}

	STDMETHOD_(BOOL, ShowCursor)(THIS_ BOOL bShow)
	{
		return m_device->ShowCursor(bShow);
	}

	STDMETHOD(CreateAdditionalSwapChain)(THIS_ D3DPRESENT_PARAMETERS* pPresentationParameters,IDirect3DSwapChain9** pSwapChain)
	{
		return m_device->CreateAdditionalSwapChain(pPresentationParameters, pSwapChain);
	}

	STDMETHOD(GetSwapChain)(THIS_ UINT iSwapChain,IDirect3DSwapChain9** pSwapChain)
	{
		return m_device->GetSwapChain(iSwapChain, pSwapChain);
	}

	STDMETHOD_(UINT, GetNumberOfSwapChains)(THIS)
	{
		return m_device->GetNumberOfSwapChains();
	}

	STDMETHOD(Reset)(THIS_ D3DPRESENT_PARAMETERS* pPresentationParameters)
	{
		return m_device->Reset(pPresentationParameters);
	}

	STDMETHOD(Present)(THIS_ CONST RECT* pSourceRect,CONST RECT* pDestRect,HWND hDestWindowOverride,CONST RGNDATA* pDirtyRegion)
	{
		memset(passTexture, 0, sizeof(passTexture));
		memset(passSurface, 0, sizeof(passSurface));
		memset(passDepth  , 0, sizeof(passDepth  ));
		memset(passPasses , 0, sizeof(passPasses ));

#if	defined(OBGE_LOGGING)
		/* log just a single frame, otherwise it's too much data */
		frame_num++;
		frame_bge = 0;

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

		if (frame_log) {
			frame_log->FormattedMessage("-- Frame %d --", frame_num);
		}
#elif	defined(OBGE_DEVLING)
		frame_num++;
		frame_bge = 0;
#endif

		return m_device->Present(pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion);
	}

	STDMETHOD(GetBackBuffer)(THIS_ UINT iSwapChain,UINT iBackBuffer,D3DBACKBUFFER_TYPE Type,IDirect3DSurface9** ppBackBuffer)
	{
		/* TODO: grab */
		return m_device->GetBackBuffer(iSwapChain, iBackBuffer, Type, ppBackBuffer);
	}

	STDMETHOD(GetRasterStatus)(THIS_ UINT iSwapChain,D3DRASTER_STATUS* pRasterStatus)
	{
		return m_device->GetRasterStatus(iSwapChain, pRasterStatus);
	}

	STDMETHOD(SetDialogBoxMode)(THIS_ BOOL bEnableDialogs)
	{
		return m_device->SetDialogBoxMode(bEnableDialogs);
	}

	STDMETHOD_(void, SetGammaRamp)(THIS_ UINT iSwapChain,DWORD Flags,CONST D3DGAMMARAMP* pRamp)
	{
		return m_device->SetGammaRamp(iSwapChain, Flags, pRamp);
	}

	STDMETHOD_(void, GetGammaRamp)(THIS_ UINT iSwapChain,D3DGAMMARAMP* pRamp)
	{
		return m_device->GetGammaRamp(iSwapChain, pRamp);
	}

	STDMETHOD(CreateTexture)(THIS_ UINT Width,UINT Height,UINT Levels,DWORD Usage,D3DFORMAT Format,D3DPOOL Pool,IDirect3DTexture9** ppTexture,HANDLE* pSharedHandle)
	{
#ifdef	OBGE_LOGGING
		if (frame_log)
			frame_log->FormattedMessage("CreateTexture from 0x%08x", _ReturnAddress());
#endif

#if	defined(OBGE_AUTOMIPMAP)
		/* hack a mipmap generating rendertarget (just once!) */
		if (Usage & D3DUSAGE_RENDERTARGET) {
			/* we get two reflection allocations, brrr */
			Usage |= specialUsage; //specialUsage = 0;
			if (Usage & D3DUSAGE_AUTOGENMIPMAP)
				Levels = 0;
		}
#endif

		HRESULT hr = m_device->CreateTexture(Width, Height, Levels, Usage, Format, Pool, ppTexture, pSharedHandle);

#if	defined(OBGE_AUTOMIPMAP)
		if ((*ppTexture) && (Usage & D3DUSAGE_AUTOGENMIPMAP))
			(*ppTexture)->SetAutoGenFilterType(D3DTEXF_LINEAR);
#endif

		if(SUCCEEDED(hr))
		{
			// Return our texture
#ifdef	OBGE_TRACKER_TEXTURES
#if	OBGE_TRACKER_TEXTURES < 1
			if ((Usage & D3DUSAGE_RENDERTARGET) || (Usage & D3DUSAGE_DEPTHSTENCIL))
#endif
				*ppTexture = new OBGEDirect3DTexture9(m_d3d, m_device, *ppTexture);
#endif

			if (frame_log || frame_trk) {
				if (OBGE_TRACKER || (Usage & D3DUSAGE_RENDERTARGET) || (Usage & D3DUSAGE_DEPTHSTENCIL)) {
					struct textureMap *track = new struct textureMap;

					track->Width = Width;
					track->Height = Height;
					track->Levels = Levels;
					track->Usage = Usage;
					track->Format = Format;

					textureMaps[*ppTexture] = track;

#ifndef	OBGE_TRACKER_TEXTURES
					/* apparently the level-address stays constant, so we can track this already from here */
					IDirect3DSurface9* ppSurfaceLevel = NULL;
					if (SUCCEEDED((*ppTexture)->GetSurfaceLevel(0, &ppSurfaceLevel))) {
						struct textureSurface *track = new struct textureSurface;

						track->Level = 0;
						track->map = textureMaps[*ppTexture];
						track->tex = *ppTexture;

						surfaceTexture[ppSurfaceLevel] = track;

						if (Usage & D3DUSAGE_RENDERTARGET)
						  _MESSAGE("OD3D9: RT GetSurfaceLevel[0]: 0x%08x", ppSurfaceLevel);
						else if (Usage & D3DUSAGE_DEPTHSTENCIL)
						  _MESSAGE("OD3D9: DS GetSurfaceLevel[0]: 0x%08x", ppSurfaceLevel);
					}
#endif
				}
			}
		}

#ifdef	OBGE_LOGGING
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
#endif

		return hr;
	}

	STDMETHOD(CreateVolumeTexture)(THIS_ UINT Width,UINT Height,UINT Depth,UINT Levels,DWORD Usage,D3DFORMAT Format,D3DPOOL Pool,IDirect3DVolumeTexture9** ppVolumeTexture,HANDLE* pSharedHandle)
	{
		return m_device->CreateVolumeTexture(Width, Height, Depth, Levels, Usage, Format, Pool, ppVolumeTexture, pSharedHandle);
	}

	STDMETHOD(CreateCubeTexture)(THIS_ UINT EdgeLength,UINT Levels,DWORD Usage,D3DFORMAT Format,D3DPOOL Pool,IDirect3DCubeTexture9** ppCubeTexture,HANDLE* pSharedHandle)
	{
		return m_device->CreateCubeTexture(EdgeLength, Levels, Usage, Format, Pool, ppCubeTexture, pSharedHandle);
	}

	STDMETHOD(CreateVertexBuffer)(THIS_ UINT Length,DWORD Usage,DWORD FVF,D3DPOOL Pool,IDirect3DVertexBuffer9** ppVertexBuffer,HANDLE* pSharedHandle)
	{
		return m_device->CreateVertexBuffer(Length, Usage, FVF, Pool, ppVertexBuffer, pSharedHandle);
	}

	STDMETHOD(CreateIndexBuffer)(THIS_ UINT Length,DWORD Usage,D3DFORMAT Format,D3DPOOL Pool,IDirect3DIndexBuffer9** ppIndexBuffer,HANDLE* pSharedHandle)
	{
		return m_device->CreateIndexBuffer(Length, Usage, Format, Pool, ppIndexBuffer, pSharedHandle);
	}

	STDMETHOD(CreateRenderTarget)(THIS_ UINT Width,UINT Height,D3DFORMAT Format,D3DMULTISAMPLE_TYPE MultiSample,DWORD MultisampleQuality,BOOL Lockable,IDirect3DSurface9** ppSurface,HANDLE* pSharedHandle)
	{
#if	defined(OBGE_LOGGING)
		if (frame_log)
			frame_log->FormattedMessage("CreateRenderTarget from 0x%08x", _ReturnAddress());
#endif

		HRESULT hr = m_device->CreateRenderTarget(Width, Height, Format, MultiSample, MultisampleQuality, Lockable, ppSurface, pSharedHandle);

		if(SUCCEEDED(hr))
		{
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

#if	defined(OBGE_LOGGING)
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
#endif

		return hr;
	}

	STDMETHOD(CreateDepthStencilSurface)(THIS_ UINT Width,UINT Height,D3DFORMAT Format,D3DMULTISAMPLE_TYPE MultiSample,DWORD MultisampleQuality,BOOL Discard,IDirect3DSurface9** ppSurface,HANDLE* pSharedHandle)
	{
#if	defined(OBGE_LOGGING)
		if (frame_log)
			frame_log->FormattedMessage("CreateDepthStencilSurface from 0x%08x", _ReturnAddress());
#endif

		HRESULT hr = m_device->CreateDepthStencilSurface(Width, Height, Format, MultiSample, MultisampleQuality, Discard, ppSurface, pSharedHandle);

		if(SUCCEEDED(hr))
		{
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

#if	defined(OBGE_LOGGING)
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
#endif

		return hr;
	}

	STDMETHOD(UpdateSurface)(THIS_ IDirect3DSurface9* pSourceSurface,CONST RECT* pSourceRect,IDirect3DSurface9* pDestinationSurface,CONST POINT* pDestPoint)
	{
#if	defined(OBGE_TRACKER_SURFACES) || defined(OBGE_TRACKER_TEXTURES)
		pSourceSurface      = translateSurface(pSourceSurface     );
		pDestinationSurface = translateSurface(pDestinationSurface);
#endif

		return m_device->UpdateSurface(pSourceSurface, pSourceRect, pDestinationSurface, pDestPoint);
	}

	STDMETHOD(UpdateTexture)(THIS_ IDirect3DBaseTexture9* pSourceTexture,IDirect3DBaseTexture9* pDestinationTexture)
	{
		return m_device->UpdateTexture(pSourceTexture, pDestinationTexture);
	}

	STDMETHOD(GetRenderTargetData)(THIS_ IDirect3DSurface9* pRenderTarget,IDirect3DSurface9* pDestSurface)
	{
#if	defined(OBGE_TRACKER_SURFACES) || defined(OBGE_TRACKER_TEXTURES)
		pRenderTarget = translateSurface(pRenderTarget);
		pDestSurface  = translateSurface(pDestSurface );
#endif

		return m_device->GetRenderTargetData(pRenderTarget, pDestSurface);
	}

	STDMETHOD(GetFrontBufferData)(THIS_ UINT iSwapChain,IDirect3DSurface9* pDestSurface)
	{
#if	defined(OBGE_TRACKER_SURFACES) || defined(OBGE_TRACKER_TEXTURES)
		pDestSurface  = translateSurface(pDestSurface );
#endif

		return m_device->GetFrontBufferData(iSwapChain, pDestSurface);
	}

	STDMETHOD(StretchRect)(THIS_ IDirect3DSurface9* pSourceSurface,CONST RECT* pSourceRect,IDirect3DSurface9* pDestSurface,CONST RECT* pDestRect,D3DTEXTUREFILTERTYPE Filter)
	{
#if	defined(OBGE_TRACKER_SURFACES) || defined(OBGE_TRACKER_TEXTURES)
		pSourceSurface = translateSurface(pSourceSurface);
		pDestSurface   = translateSurface(pDestSurface  );
#endif

		return m_device->StretchRect(pSourceSurface, pSourceRect, pDestSurface, pDestRect, Filter);
	}

	STDMETHOD(ColorFill)(THIS_ IDirect3DSurface9* pSurface,CONST RECT* pRect,D3DCOLOR color)
	{
#if	defined(OBGE_TRACKER_SURFACES) || defined(OBGE_TRACKER_TEXTURES)
		pSurface = translateSurface(pSurface);
#endif

		return m_device->ColorFill(pSurface, pRect, color);
	}

	STDMETHOD(CreateOffscreenPlainSurface)(THIS_ UINT Width,UINT Height,D3DFORMAT Format,D3DPOOL Pool,IDirect3DSurface9** ppSurface,HANDLE* pSharedHandle)
	{
		/* TODO: grab */
		return m_device->CreateOffscreenPlainSurface(Width, Height, Format, Pool, ppSurface, pSharedHandle);
	}

	STDMETHOD(SetRenderTarget)(THIS_ DWORD RenderTargetIndex,IDirect3DSurface9* pRenderTarget)
	{
		/* they are a textures anyway, no need to check dedicated targets */
		if (currentPass != OBGEPASS_UNKNOWN) {
		  /* dedicated rendertarget, possibly with multi-sampling */
		  if (surfaceRender[pRenderTarget])
		    passSurface[currentPass] = pRenderTarget,
		      passTexture[currentPass] = NULL;

		  /* texture-based rendertarget surface, no multisampling */
		  if (surfaceTexture[pRenderTarget])
		    if ((passSurface[currentPass] = pRenderTarget))
		      passTexture[currentPass] = surfaceTexture[pRenderTarget]->tex;

	//	  _MESSAGE("OD3D9: Grabbed pass %d texture", currentPass);
		}

#ifdef	OBGE_LOGGING
		if (frame_log) {
			frame_log->FormattedMessage("SetRenderTarget[%d] from 0x%08x", RenderTargetIndex, _ReturnAddress());
			frame_log->Indent();
			frame_log->FormattedMessage("Address: 0x%08x", pRenderTarget);

			bool ok = false;

			if (!pRenderTarget) {
				ok = true;
			}

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
						case D3DRTYPE_SURFACE: Type = "Untracked Surface"; break;
						case D3DRTYPE_VOLUME: Type = "Untracked Volume"; break;
						case D3DRTYPE_TEXTURE: Type = "Untracked Texture"; break;
						case D3DRTYPE_VOLUMETEXTURE: Type = "Untracked Volume Texture"; break;
						case D3DRTYPE_CUBETEXTURE: Type = "Untracked Cube Texture"; break;
						case D3DRTYPE_VERTEXBUFFER: Type = "Untracked Vertex Buffer"; break;
						case D3DRTYPE_INDEXBUFFER: Type = "Untracked Index Buffer"; break;
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
#endif

#if	defined(OBGE_TRACKER_SURFACES) || defined(OBGE_TRACKER_TEXTURES)
		pRenderTarget = translateSurface(pRenderTarget);
#endif

		return m_device->SetRenderTarget(RenderTargetIndex, pRenderTarget);
	}

	STDMETHOD(GetRenderTarget)(THIS_ DWORD RenderTargetIndex,IDirect3DSurface9** ppRenderTarget)
	{
		/* TODO: grab */
		return m_device->GetRenderTarget(RenderTargetIndex, ppRenderTarget);
	}

	STDMETHOD(SetDepthStencilSurface)(THIS_ IDirect3DSurface9* pNewZStencil)
	{
		/* they are a dedicated anyway, no need to check textures */
		if (currentPass != OBGEPASS_UNKNOWN) {
		/* dedicated depthstencil, possibly with multi-sampling */
		  if (surfaceDepth[pNewZStencil])
		    passDepth[currentPass] = pNewZStencil;

		  /* texture-based depthstencil surface, no multisampling */
		  if (surfaceTexture[pNewZStencil])
		    passDepth[currentPass] = pNewZStencil;

	//	  _MESSAGE("OD3D9: Grabbed pass %d depth", currentPass);
		}

#ifdef	OBGE_LOGGING
		if (frame_log) {
			frame_log->FormattedMessage("SetDepthStencilSurface from 0x%08x", _ReturnAddress());
			frame_log->Indent();
			frame_log->FormattedMessage("Address: 0x%08x", pNewZStencil);

			bool ok = false;

			if (!pNewZStencil) {
				ok = true;
			}

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
						case D3DRTYPE_SURFACE: Type = "Untracked Surface"; break;
						case D3DRTYPE_VOLUME: Type = "Untracked Volume"; break;
						case D3DRTYPE_TEXTURE: Type = "Untracked Texture"; break;
						case D3DRTYPE_VOLUMETEXTURE: Type = "Untracked Volume Texture"; break;
						case D3DRTYPE_CUBETEXTURE: Type = "Untracked Cube Texture"; break;
						case D3DRTYPE_VERTEXBUFFER: Type = "Untracked Vertex Buffer"; break;
						case D3DRTYPE_INDEXBUFFER: Type = "Untracked Index Buffer"; break;
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
#endif

#if	defined(OBGE_TRACKER_SURFACES) || defined(OBGE_TRACKER_TEXTURES)
		pNewZStencil = translateSurface(pNewZStencil);
#endif

		return m_device->SetDepthStencilSurface(pNewZStencil);
	}

	STDMETHOD(GetDepthStencilSurface)(THIS_ IDirect3DSurface9** ppZStencilSurface)
	{
		/* TODO: grab */
		return m_device->GetDepthStencilSurface(ppZStencilSurface);
	}

	STDMETHOD(BeginScene)(THIS)
	{
	//	assert(frame_num != 8000);
	//	assert(currentPass != OBGEPASS_UNKNOWN);
	//	assert(frame_bge == 26);

		/* the D3D-statemachine doesn't kill these ... */
	//	m_shadercv = NULL;
	//	m_shadercp = NULL;

#if	defined(OBGE_DEVLING)
		if (HasShaderManager) {
			/* clean all on first scene, and partial on successive scenes */
			int frame_numr = m_shaders->trackd[currentPass].frame_numr;

			m_shaders->Clear(currentPass, frame_numr != frame_num);
			m_shaders->trackd[currentPass].frame_numr = frame_num;
		}

		passFrames[currentPass] = frame_num;
#elif	defined(OBGE_LOGGING)
		passFrames[currentPass] = frame_num;
#endif

#ifdef	OBGE_LOGGING
		if (frame_log) {
			frame_log->FormattedMessage("BeginScene %d from 0x%08x", frame_bge, _ReturnAddress());
			frame_log->Indent();
		}
#endif

#if	defined(OBGE_PROFILE)
		QueryPerformanceCounter(&frame_bgn);
#endif

		return m_device->BeginScene();
	}

	STDMETHOD(EndScene)(THIS)
	{
#if	defined(OBGE_PROFILE)
		QueryPerformanceCounter(&frame_end);
#endif

		HRESULT res = m_device->EndScene();

#ifdef	OBGE_LOGGING
		if (frame_log) {
			frame_log->Outdent();
			frame_log->FormattedMessage("EndScene from 0x%08x", _ReturnAddress());
		}
#endif

#ifdef	OBGE_DEVLING
		/* apparenty there is one BeginScene missing in the HDR-pipeline (2nd scene! */
		if (HasShaderManager) {
			/* record exact position of occurance */
			int frame_cntr = m_shaders->trackd[currentPass].frame_cntr;

			/* huh, going into menu provokes huge numbers here */
			if (frame_cntr < OBGESCENE_NUM) {
				m_shaders->trackd[currentPass].frame_used[frame_cntr] = frame_num;
				m_shaders->trackd[currentPass].frame_pass[frame_cntr] = frame_bge;
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
		passPasses[currentPass]++;

		frame_bge++;
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
			char str[256]; sprintf_s(str, "OBGEv2-frame%04d-scene%02d.dds", frame_num, frame_bge);
			LstErr = D3DXSaveSurfaceToFile(str, D3DXIFF_DDS, pBuf, NULL, NULL);
			pBuf->Release();
		}
#endif

		return res;
	}

	STDMETHOD(Clear)(THIS_ DWORD Count,CONST D3DRECT* pRects,DWORD Flags,D3DCOLOR Color,float Z,DWORD Stencil)
	{
#ifdef	OBGE_LOGGING
		if (frame_log) {
			frame_log->Message("Clear:");

			frame_log->Indent();
			frame_log->FormattedMessage("Flags:%s%s%s",
			  Flags & D3DCLEAR_TARGET  ? " Rendertarget" : "",
			  Flags & D3DCLEAR_STENCIL ? " Stencil" : "",
			  Flags & D3DCLEAR_ZBUFFER ? " Depth" : "");
			frame_log->FormattedMessage("Values: {%d,%d,%d,%d} %f %d",
			  (Color >> 0) & 0xFF, (Color >> 8) & 0xFF, (Color >> 16) & 0xFF, (Color >> 24) & 0xFF,
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
#endif

		return m_device->Clear(Count, pRects, Flags, Color, Z, Stencil);
	}

	STDMETHOD(SetTransform)(THIS_ D3DTRANSFORMSTATETYPE State,CONST D3DMATRIX* pMatrix)
	{
		if (HasShaderManager) {
			/**/ if (State == D3DTS_VIEW)
				m_shaders->ShaderConst.view = *pMatrix;
			else if (State == D3DTS_PROJECTION)
				m_shaders->ShaderConst.proj = *pMatrix;
			else if (State == D3DTS_WORLD)
				m_shaders->ShaderConst.wrld = *pMatrix;

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

#ifdef	OBGE_LOGGING
		if (frame_log) {
			frame_log->Message("SetTransform:");

			const char *Type = NULL;
			switch (State) {
			    case D3DTS_VIEW: Type = "View"; break;
			    case D3DTS_PROJECTION: Type = "Projection"; break;
			    case D3DTS_WORLD: Type = "World"; break;
			    case D3DTS_TEXTURE0: Type = "Texture0"; break;
			    case D3DTS_TEXTURE1: Type = "Texture1"; break;
			    case D3DTS_TEXTURE2: Type = "Texture2"; break;
			    case D3DTS_TEXTURE3: Type = "Texture3"; break;
			    case D3DTS_TEXTURE4: Type = "Texture4"; break;
			    case D3DTS_TEXTURE5: Type = "Texture5"; break;
			    case D3DTS_TEXTURE6: Type = "Texture6"; break;
			    case D3DTS_TEXTURE7: Type = "Texture7"; break;
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
#endif

		return m_device->SetTransform(State, pMatrix);
	}

	STDMETHOD(GetTransform)(THIS_ D3DTRANSFORMSTATETYPE State,D3DMATRIX* pMatrix)
	{
		return m_device->GetTransform(State, pMatrix);
	}

	STDMETHOD(MultiplyTransform)(THIS_ D3DTRANSFORMSTATETYPE State,CONST D3DMATRIX* pMatrix)
	{
		return m_device->MultiplyTransform(State, pMatrix);
	}

	STDMETHOD(SetViewport)(THIS_ CONST D3DVIEWPORT9* pViewport)
	{
		if (HasShaderManager) {
			const float W = (float)pViewport->Width;
			const float H = (float)pViewport->Height;

			/* record constants */
			m_shaders->ShaderConst.rcpres[0] = 1.0f / W;
			m_shaders->ShaderConst.rcpres[1] = 1.0f / H;
			m_shaders->ShaderConst.rcpres[2] = W / H;
			m_shaders->ShaderConst.rcpres[3] = W * H;
		}

#ifdef	OBGE_LOGGING
		if (frame_log) {
			frame_log->Message("SetViewport:");

			frame_log->Indent();
			frame_log->FormattedMessage("{X,Y}: {%d,%d}", pViewport->X, pViewport->Y);
			frame_log->FormattedMessage("{Width,Height}: {%d,%d}", pViewport->Width, pViewport->Height);
			frame_log->FormattedMessage("{MinZ,MaxZ}: {%f,%f}", pViewport->MinZ, pViewport->MaxZ);
			frame_log->Outdent();
		}
#endif

		return m_device->SetViewport(pViewport);
	}

	STDMETHOD(GetViewport)(THIS_ D3DVIEWPORT9* pViewport)
	{
		return m_device->GetViewport(pViewport);
	}

	STDMETHOD(SetMaterial)(THIS_ CONST D3DMATERIAL9* pMaterial)
	{
#ifdef	OBGE_LOGGING
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
#endif

		return m_device->SetMaterial(pMaterial);
	}

	STDMETHOD(GetMaterial)(THIS_ D3DMATERIAL9* pMaterial)
	{
		return m_device->GetMaterial(pMaterial);
	}

	STDMETHOD(SetLight)(THIS_ DWORD Index,CONST D3DLIGHT9* pLight)
	{
#ifdef	OBGE_LOGGING
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
#endif

		return m_device->SetLight(Index, pLight);
	}

	STDMETHOD(GetLight)(THIS_ DWORD Index,D3DLIGHT9* pLight)
	{
		return m_device->GetLight(Index, pLight);
	}

	STDMETHOD(LightEnable)(THIS_ DWORD Index,BOOL Enable)
	{
#ifdef	OBGE_LOGGING
		if (frame_log)
			frame_log->FormattedMessage("Light[%d]: %s", Index, Enable ? "enabled" : "disabled");
#endif

		return m_device->LightEnable(Index, Enable);
	}

	STDMETHOD(GetLightEnable)(THIS_ DWORD Index,BOOL* pEnable)
	{
		return m_device->GetLightEnable(Index, pEnable);
	}

	STDMETHOD(SetClipPlane)(THIS_ DWORD Index,CONST float* pPlane)
	{
		return m_device->SetClipPlane(Index, pPlane);
	}

	STDMETHOD(GetClipPlane)(THIS_ DWORD Index,float* pPlane)
	{
		return m_device->GetClipPlane(Index, pPlane);
	}

	STDMETHOD(SetRenderState)(THIS_ D3DRENDERSTATETYPE State,DWORD Value)
	{
#ifdef	OBGE_DEVLING
		if (HasShaderManager) {
			/* record exact position of occurance */
			int frame_cntr = m_shaders->trackd[currentPass].frame_cntr;

			/* huh, going into menu provokes huge numbers here */
			if (frame_cntr < OBGESCENE_NUM)
				m_shaders->trackd[currentPass].states[frame_cntr][State] = Value;
		}
#endif

#ifdef	OBGE_LOGGING
		if (frame_log) {
			frame_log->Message("SetRenderState");
			frame_log->Indent();
			frame_log->FormattedMessage("%s: %s", findRenderState(State), findRenderStateValue(State, Value));
			frame_log->Outdent();
		}
#endif

		return m_device->SetRenderState(State, Value);
	}

	STDMETHOD(GetRenderState)(THIS_ D3DRENDERSTATETYPE State,DWORD* pValue)
	{
		return m_device->GetRenderState(State, pValue);
	}

	STDMETHOD(CreateStateBlock)(THIS_ D3DSTATEBLOCKTYPE Type,IDirect3DStateBlock9** ppSB)
	{
		return m_device->CreateStateBlock(Type, ppSB);
	}

	STDMETHOD(BeginStateBlock)(THIS)
	{
		return m_device->BeginStateBlock();
	}

	STDMETHOD(EndStateBlock)(THIS_ IDirect3DStateBlock9** ppSB)
	{
		return m_device->EndStateBlock(ppSB);
	}

	STDMETHOD(SetClipStatus)(THIS_ CONST D3DCLIPSTATUS9* pClipStatus)
	{
		return m_device->SetClipStatus(pClipStatus);
	}

	STDMETHOD(GetClipStatus)(THIS_ D3DCLIPSTATUS9* pClipStatus)
	{
		return m_device->GetClipStatus(pClipStatus);
	}

	STDMETHOD(GetTexture)(THIS_ DWORD Sampler,IDirect3DBaseTexture9** ppTexture)
	{
		return m_device->GetTexture(Sampler, ppTexture);
	}

	STDMETHOD(SetTexture)(THIS_ DWORD Sampler,IDirect3DBaseTexture9* pTexture)
	{
#ifdef	OBGE_DEVLING
		if (HasShaderManager)
			m_shaders->traced[currentPass].values_s[Sampler] = pTexture;
#endif

#ifdef	OBGE_LOGGING
		if (frame_log) {
			frame_log->FormattedMessage("SetTexture[%d]", Sampler);
			frame_log->Indent();
			frame_log->FormattedMessage("Address: 0x%08x", pTexture); if (pTexture)
			frame_log->FormattedMessage("Path: %s", findTexture(pTexture));

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
#endif

#if	defined(OBGE_AUTOMIPMAP)
		struct textureMap *track;
		bool setfilter = false;
		if (pTexture && (track = textureMaps[pTexture])) {
			D3DTEXTUREFILTERTYPE ft = pTexture->GetAutoGenFilterType();

			if (track->Usage & D3DUSAGE_RENDERTARGET) {
			  int i = 0;
			}

			if ((track->Usage & D3DUSAGE_AUTOGENMIPMAP) &&
			    (track->Usage & D3DUSAGE_RENDERTARGET)) {
#if	defined(OBGE_AUTOMIPMAP) && (OBGE_AUTOMIPMAP > 0)
      				pTexture->SetAutoGenFilterType(D3DTEXF_LINEAR);
      				pTexture->GenerateMipSubLevels();
#endif
      				setfilter = true;
			}
		}
#endif

		HRESULT res = m_device->SetTexture(Sampler, pTexture);

#if	defined(OBGE_AUTOMIPMAP)
		if (setfilter) {
			SetSamplerState(Sampler, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
			SetSamplerState(Sampler, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
			SetSamplerState(Sampler, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);
		}
#endif

   		return res;
	}

	STDMETHOD(GetTextureStageState)(THIS_ DWORD Stage,D3DTEXTURESTAGESTATETYPE Type,DWORD* pValue)
	{
		return m_device->GetTextureStageState(Stage, Type, pValue);
	}

	STDMETHOD(SetTextureStageState)(THIS_ DWORD Stage,D3DTEXTURESTAGESTATETYPE Type,DWORD Value)
	{
		return m_device->SetTextureStageState(Stage, Type, Value);
	}

	STDMETHOD(GetSamplerState)(THIS_ DWORD Sampler,D3DSAMPLERSTATETYPE Type,DWORD* pValue)
	{
		return m_device->GetSamplerState(Sampler, Type, pValue);
	}

	STDMETHOD(SetSamplerState)(THIS_ DWORD Sampler,D3DSAMPLERSTATETYPE Type,DWORD Value)
	{
#ifdef	OBGE_DEVLING
		if (HasShaderManager)
			m_shaders->traced[currentPass].states_s[Sampler][Type] = Value;
#endif

		return m_device->SetSamplerState(Sampler, Type, Value);
	}

	STDMETHOD(ValidateDevice)(THIS_ DWORD* pNumPasses)
	{
		return m_device->ValidateDevice(pNumPasses);
	}

	STDMETHOD(SetPaletteEntries)(THIS_ UINT PaletteNumber,CONST PALETTEENTRY* pEntries)
	{
		return m_device->SetPaletteEntries(PaletteNumber, pEntries);
	}

	STDMETHOD(GetPaletteEntries)(THIS_ UINT PaletteNumber,PALETTEENTRY* pEntries)
	{
		return m_device->GetPaletteEntries(PaletteNumber, pEntries);
	}

	STDMETHOD(SetCurrentTexturePalette)(THIS_ UINT PaletteNumber)
	{
		return m_device->SetCurrentTexturePalette(PaletteNumber);
	}

	STDMETHOD(GetCurrentTexturePalette)(THIS_ UINT *PaletteNumber)
	{
		return m_device->GetCurrentTexturePalette(PaletteNumber);
	}

	STDMETHOD(SetScissorRect)(THIS_ CONST RECT* pRect)
	{
		return m_device->SetScissorRect(pRect);
	}

	STDMETHOD(GetScissorRect)(THIS_ RECT* pRect)
	{
		return m_device->GetScissorRect(pRect);
	}

	STDMETHOD(SetSoftwareVertexProcessing)(THIS_ BOOL bSoftware)
	{
		return m_device->SetSoftwareVertexProcessing(bSoftware);
	}

	STDMETHOD_(BOOL, GetSoftwareVertexProcessing)(THIS)
	{
		return m_device->GetSoftwareVertexProcessing();
	}

	STDMETHOD(SetNPatchMode)(THIS_ float nSegments)
	{
		return m_device->SetNPatchMode(nSegments);
	}

	STDMETHOD_(float, GetNPatchMode)(THIS)
	{
		return m_device->GetNPatchMode();
	}

	STDMETHOD(DrawPrimitive)(THIS_ D3DPRIMITIVETYPE PrimitiveType,UINT StartVertex,UINT PrimitiveCount)
	{
#ifdef	OBGE_LOGGING
		if (frame_log) {
			frame_log->Message("DrawPrimitive");

			const char *Type = "Unknown";
			switch (PrimitiveType) {
				case D3DPT_POINTLIST: Type = "PointList"; break;
				case D3DPT_LINELIST: Type = "LineList"; break;
				case D3DPT_LINESTRIP: Type = "LineStrip"; break;
				case D3DPT_TRIANGLELIST: Type = "TriangleList"; break;
				case D3DPT_TRIANGLESTRIP: Type = "TriangleStrip"; break;
				case D3DPT_TRIANGLEFAN: Type = "TriangleFan"; break;
			}

			frame_log->Indent();
			frame_log->FormattedMessage("Type: %s", Type);
			frame_log->FormattedMessage("Count: %d", PrimitiveCount);
			frame_log->Outdent();
		}
#endif

		return m_device->DrawPrimitive(PrimitiveType, StartVertex, PrimitiveCount);
	}

	STDMETHOD(DrawIndexedPrimitive)(THIS_ D3DPRIMITIVETYPE PrimitiveType,INT BaseVertexIndex,UINT MinVertexIndex,UINT NumVertices,UINT startIndex,UINT primCount)
	{
#ifdef	OBGE_LOGGING
		if (frame_log) {
			frame_log->Message("DrawIndexedPrimitive");

			const char *Type = "Unknown";
			switch (PrimitiveType) {
				case D3DPT_POINTLIST: Type = "PointList"; break;
				case D3DPT_LINELIST: Type = "LineList"; break;
				case D3DPT_LINESTRIP: Type = "LineStrip"; break;
				case D3DPT_TRIANGLELIST: Type = "TriangleList"; break;
				case D3DPT_TRIANGLESTRIP: Type = "TriangleStrip"; break;
				case D3DPT_TRIANGLEFAN: Type = "TriangleFan"; break;
			}

			frame_log->Indent();
			frame_log->FormattedMessage("Type: %s", Type);
			frame_log->FormattedMessage("Count: %d", primCount);
			frame_log->FormattedMessage("Vertices: %d", NumVertices);
			frame_log->Outdent();
		}
#endif

		return m_device->DrawIndexedPrimitive(PrimitiveType, BaseVertexIndex, MinVertexIndex, NumVertices, startIndex, primCount);
	}

	STDMETHOD(DrawPrimitiveUP)(THIS_ D3DPRIMITIVETYPE PrimitiveType,UINT PrimitiveCount,CONST void* pVertexStreamZeroData,UINT VertexStreamZeroStride)
	{
#ifdef	OBGE_LOGGING
		if (frame_log) {
			frame_log->Message("DrawPrimitiveUP");

			const char *Type = "Unknown";
			switch (PrimitiveType) {
				case D3DPT_POINTLIST: Type = "PointList"; break;
				case D3DPT_LINELIST: Type = "LineList"; break;
				case D3DPT_LINESTRIP: Type = "LineStrip"; break;
				case D3DPT_TRIANGLELIST: Type = "TriangleList"; break;
				case D3DPT_TRIANGLESTRIP: Type = "TriangleStrip"; break;
				case D3DPT_TRIANGLEFAN: Type = "TriangleFan"; break;
			}

			frame_log->Indent();
			frame_log->FormattedMessage("Type: %s", Type);
			frame_log->FormattedMessage("Count: %d", PrimitiveCount);
			frame_log->FormattedMessage("Stride: %d", VertexStreamZeroStride);
			frame_log->Outdent();
		}
#endif

		return m_device->DrawPrimitiveUP(PrimitiveType, PrimitiveCount, pVertexStreamZeroData, VertexStreamZeroStride);
	}

	STDMETHOD(DrawIndexedPrimitiveUP)(THIS_ D3DPRIMITIVETYPE PrimitiveType,UINT MinVertexIndex,UINT NumVertices,UINT PrimitiveCount,CONST void* pIndexData,D3DFORMAT IndexDataFormat,CONST void* pVertexStreamZeroData,UINT VertexStreamZeroStride)
	{
#ifdef	OBGE_LOGGING
		if (frame_log) {
			frame_log->Message("DrawIndexedPrimitiveUP");

			const char *Type = "Unknown";
			switch (PrimitiveType) {
				case D3DPT_POINTLIST: Type = "PointList"; break;
				case D3DPT_LINELIST: Type = "LineList"; break;
				case D3DPT_LINESTRIP: Type = "LineStrip"; break;
				case D3DPT_TRIANGLELIST: Type = "TriangleList"; break;
				case D3DPT_TRIANGLESTRIP: Type = "TriangleStrip"; break;
				case D3DPT_TRIANGLEFAN: Type = "TriangleFan"; break;
			}

			frame_log->Indent();
			frame_log->FormattedMessage("Type: %s", Type);
			frame_log->FormattedMessage("Format: %s", findFormat(IndexDataFormat));
			frame_log->FormattedMessage("Count: %d", PrimitiveCount);
			frame_log->FormattedMessage("Vertices: %d", NumVertices);
			frame_log->FormattedMessage("Stride: %d", VertexStreamZeroStride);
			frame_log->Outdent();
		}
#endif

		return m_device->DrawIndexedPrimitiveUP(PrimitiveType, MinVertexIndex, NumVertices, PrimitiveCount, pIndexData, IndexDataFormat, pVertexStreamZeroData, VertexStreamZeroStride);
	}

	STDMETHOD(ProcessVertices)(THIS_ UINT SrcStartIndex,UINT DestIndex,UINT VertexCount,IDirect3DVertexBuffer9* pDestBuffer,IDirect3DVertexDeclaration9* pVertexDecl,DWORD Flags)
	{
		return m_device->ProcessVertices(SrcStartIndex, DestIndex, VertexCount, pDestBuffer, pVertexDecl, Flags);
	}

	STDMETHOD(CreateVertexDeclaration)(THIS_ CONST D3DVERTEXELEMENT9* pVertexElements,IDirect3DVertexDeclaration9** ppDecl)
	{
		return m_device->CreateVertexDeclaration(pVertexElements, ppDecl);
	}

	STDMETHOD(SetVertexDeclaration)(THIS_ IDirect3DVertexDeclaration9* pDecl)
	{
		return m_device->SetVertexDeclaration(pDecl);
	}

	STDMETHOD(GetVertexDeclaration)(THIS_ IDirect3DVertexDeclaration9** ppDecl)
	{
		return m_device->GetVertexDeclaration(ppDecl);
	}

	STDMETHOD(SetFVF)(THIS_ DWORD FVF)
	{
		return m_device->SetFVF(FVF);
	}

	STDMETHOD(GetFVF)(THIS_ DWORD* pFVF)
	{
		return m_device->GetFVF(pFVF);
	}

	STDMETHOD(CreateVertexShader)(THIS_ CONST DWORD* pFunction,IDirect3DVertexShader9** ppShader)
	{
#ifdef	OBGE_LOGGING
		if (frame_log)
			frame_log->FormattedMessage("CreateVertexShader 0x%08x", _ReturnAddress());
#endif

		HRESULT res = m_device->CreateVertexShader(pFunction, ppShader);

		if (frame_log || frame_trk) {
			/* just register, don't return any manipulated class */
			if (HasShaderManager)
				m_shaders->SetRuntimeShader(pFunction, *ppShader);

#ifdef	OBGE_LOGGING
			if (frame_log) {
				UINT len = 0; if (*ppShader) (*ppShader)->GetFunction(NULL, &len);
				const char *nam = findShader(*ppShader, len, pFunction);

				frame_log->Indent();
				frame_log->FormattedMessage("Length: %d", len);
				frame_log->FormattedMessage("Name: %s", nam);
				frame_log->Outdent();
			}
#endif
		}

		return res;
	}

	STDMETHOD(SetVertexShader)(THIS_ IDirect3DVertexShader9* pShader)
	{
		HRESULT res = 0;
		m_shadercv = NULL;

		if (HasShaderManager && pShader && (m_shadercv = m_shaders->GetRuntimeShader(pShader))) {
#if	!defined(OBGE_DEVLING)
			/* set shader first before constants */
			res = m_device->SetPixelShader(pShader);
			/* apply dynamic runtime parameters */
			m_shadercv->SetRuntimeParams(m_device, m_device);
#else
			m_shadercv->frame_used[OBGEPASS_UNKNOWN] = frame_num;
			m_shadercv->frame_pass[OBGEPASS_UNKNOWN] = frame_bge;

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

#if	defined(OBGE_LOGGING)
			if (frame_log) {
				char buf[256]; sprintf(buf, "Name: %s", m_shadercv->pAssociate ? m_shadercv->pAssociate->Name : "unknown");

				frame_log->Message("SetVertexShader");
				frame_log->Indent();
				frame_log->Message(buf);
				frame_log->Outdent();
			}
#endif
#endif
		}
#if	defined(OBGE_LOGGING) && defined(OBGE_DEVLING)
		else if (frame_log) {
			char buf[256]; sprintf(buf, "Name: %s", pShader ? "untracked" : "cleared");

			frame_log->Message("SetVertexShader");
			frame_log->Indent();
			frame_log->Message(buf);
			frame_log->Outdent();

			res = m_device->SetVertexShader(pShader);
		}
		else
			res = m_device->SetVertexShader(pShader);
#endif
#if	defined(OBGE_LOGGING) && !defined(OBGE_DEVLING)
		if (frame_log) {
			frame_log->Message("SetVertexShader");

			frame_log->Indent();
			frame_log->FormattedMessage("Name: %s", findShader(pShader));
			frame_log->Outdent();
		}

		res = m_device->SetVertexShader(pShader);
#endif

		return res;
	}

	STDMETHOD(GetVertexShader)(THIS_ IDirect3DVertexShader9** ppShader)
	{
		return m_device->GetVertexShader(ppShader);
	}

	STDMETHOD(SetVertexShaderConstantF)(THIS_ UINT StartRegister,CONST float* pConstantData,UINT Vector4fCount)
	{
#ifdef	OBGE_DEVLING
		if (m_shadercv) {
			memcpy(
				m_shadercv->traced[currentPass].values_c[StartRegister],
				pConstantData,
				Vector4fCount * 4 * sizeof(float)
			);
		}
#endif

#ifdef	OBGE_LOGGING
		if (frame_log) {
			frame_log->FormattedMessage("SetVertexShaderConstantF[%d+]", StartRegister);
			frame_log->Indent();
			for (int v = 0; v < Vector4fCount; v++)
				frame_log->FormattedMessage("|%f|%f|%f|%f|", pConstantData[4*v+0], pConstantData[4*v+1], pConstantData[4*v+2], pConstantData[4*v+3]);
			frame_log->Outdent();
		}
#endif

		return m_device->SetVertexShaderConstantF(StartRegister, pConstantData, Vector4fCount);
	}

	STDMETHOD(GetVertexShaderConstantF)(THIS_ UINT StartRegister,float* pConstantData,UINT Vector4fCount)
	{
		return m_device->GetVertexShaderConstantF(StartRegister, pConstantData, Vector4fCount);
	}

	STDMETHOD(SetVertexShaderConstantI)(THIS_ UINT StartRegister,CONST int* pConstantData,UINT Vector4iCount)
	{
#ifdef	OBGE_DEVLING
		if (m_shadercv) {
			memcpy(
				m_shadercv->traced[currentPass].values_i[StartRegister],
				pConstantData,
				Vector4iCount * 4 * sizeof(int)
			);
		}
#endif

#ifdef	OBGE_LOGGING
		if (frame_log) {
			frame_log->FormattedMessage("SetVertexShaderConstantI[%d+]", StartRegister);
			frame_log->Indent();
			for (int v = 0; v < Vector4iCount; v++)
				frame_log->FormattedMessage("|%d|%d|%d|%d|", pConstantData[4*v+0], pConstantData[4*v+1], pConstantData[4*v+2], pConstantData[4*v+3]);
			frame_log->Outdent();
		}
#endif

		return m_device->SetVertexShaderConstantI(StartRegister, pConstantData, Vector4iCount);
	}

	STDMETHOD(GetVertexShaderConstantI)(THIS_ UINT StartRegister,int* pConstantData,UINT Vector4iCount)
	{
		return m_device->GetVertexShaderConstantI(StartRegister, pConstantData, Vector4iCount);
	}

	STDMETHOD(SetVertexShaderConstantB)(THIS_ UINT StartRegister,CONST BOOL* pConstantData,UINT  BoolCount)
	{
#ifdef	OBGE_LOGGING
		if (frame_log) {
			frame_log->FormattedMessage("SetVertexShaderConstantB[%d+]", StartRegister);
			frame_log->Indent();
			for (int v = 0; v < BoolCount; v++)
				frame_log->FormattedMessage("|%d|", pConstantData[v]);
			frame_log->Outdent();
		}
#endif

		return m_device->SetVertexShaderConstantB(StartRegister, pConstantData, BoolCount);
	}

	STDMETHOD(GetVertexShaderConstantB)(THIS_ UINT StartRegister,BOOL* pConstantData,UINT BoolCount)
	{
		return m_device->GetVertexShaderConstantB(StartRegister, pConstantData, BoolCount);
	}

	STDMETHOD(SetStreamSource)(THIS_ UINT StreamNumber,IDirect3DVertexBuffer9* pStreamData,UINT OffsetInBytes,UINT Stride)
	{
		return m_device->SetStreamSource(StreamNumber, pStreamData, OffsetInBytes, Stride);
	}

	STDMETHOD(GetStreamSource)(THIS_ UINT StreamNumber,IDirect3DVertexBuffer9** ppStreamData,UINT* pOffsetInBytes,UINT* pStride)
	{
		return m_device->GetStreamSource(StreamNumber, ppStreamData, pOffsetInBytes, pStride);
	}

	STDMETHOD(SetStreamSourceFreq)(THIS_ UINT StreamNumber,UINT Setting)
	{
		return m_device->SetStreamSourceFreq(StreamNumber, Setting);
	}

	STDMETHOD(GetStreamSourceFreq)(THIS_ UINT StreamNumber,UINT* pSetting)
	{
		return m_device->GetStreamSourceFreq(StreamNumber, pSetting);
	}

	STDMETHOD(SetIndices)(THIS_ IDirect3DIndexBuffer9* pIndexData)
	{
		return m_device->SetIndices(pIndexData);
	}

	STDMETHOD(GetIndices)(THIS_ IDirect3DIndexBuffer9** ppIndexData)
	{
		return m_device->GetIndices(ppIndexData);
	}

	STDMETHOD(CreatePixelShader)(THIS_ CONST DWORD* pFunction,IDirect3DPixelShader9** ppShader)
	{
#ifdef	OBGE_LOGGING
		if (frame_log)
			frame_log->FormattedMessage("CreatePixelShader 0x%08x", _ReturnAddress());
#endif

		HRESULT res = m_device->CreatePixelShader(pFunction, ppShader);

		if (frame_log || frame_trk) {
			/* just register, don't return any manipulated class */
			if (HasShaderManager)
				m_shaders->SetRuntimeShader(pFunction, *ppShader);

#ifdef	OBGE_LOGGING
			if (frame_log) {
				UINT len = 0; if (*ppShader) (*ppShader)->GetFunction(NULL, &len);
				const char *nam = findShader(*ppShader, len, pFunction);

				frame_log->Indent();
				frame_log->FormattedMessage("Length: %d", len);
				frame_log->FormattedMessage("Name: %s", nam);
				frame_log->Outdent();
			}
#endif
		}

		return res;
	}

	STDMETHOD(SetPixelShader)(THIS_ IDirect3DPixelShader9* pShader)
	{
		HRESULT res = 0;
		m_shadercp = NULL;

		if (HasShaderManager && pShader && (m_shadercp = m_shaders->GetRuntimeShader(pShader))) {
#if	!defined(OBGE_DEVLING)
			/* set shader first before constants */
			res = m_device->SetPixelShader(pShader);
		        /* apply dynamic runtime parameters */
			m_shadercp->SetRuntimeParams(m_device, m_device);
#else

			m_shadercp->frame_used[OBGEPASS_UNKNOWN] = frame_num;
			m_shadercp->frame_pass[OBGEPASS_UNKNOWN] = frame_bge;

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

#if	defined(OBGE_LOGGING)
			if (frame_log) {
				char buf[256]; sprintf(buf, "Name: %s", m_shadercp->pAssociate ? m_shadercp->pAssociate->Name : "unknown");

				frame_log->Message("SetPixelShader");
				frame_log->Indent();
				frame_log->Message(buf);
				frame_log->Outdent();
			}
#endif
#endif
		}
#if	defined(OBGE_LOGGING) && defined(OBGE_DEVLING)
		else if (frame_log) {
			char buf[256]; sprintf(buf, "Name: %s", pShader ? "untracked" : "cleared");

			frame_log->Message("SetPixelShader");
			frame_log->Indent();
			frame_log->Message(buf);
			frame_log->Outdent();

			res = m_device->SetPixelShader(pShader);
		}
		else
			res = m_device->SetPixelShader(pShader);
#endif
#if	defined(OBGE_LOGGING) && !defined(OBGE_DEVLING)
		/* set shader first before constants */
		res = m_device->SetPixelShader(pShader);

		if (frame_log) {
			frame_log->Message("SetPixelShader");

			frame_log->Indent();
			frame_log->FormattedMessage("Name: %s", findShader(pShader));
			frame_log->Outdent();
		}
#endif

		return res;
	}

	STDMETHOD(GetPixelShader)(THIS_ IDirect3DPixelShader9** ppShader)
	{
		return m_device->GetPixelShader(ppShader);
	}

	STDMETHOD(SetPixelShaderConstantF)(THIS_ UINT StartRegister,CONST float* pConstantData,UINT Vector4fCount)
	{
#ifdef	OBGE_DEVLING
		if (m_shadercp) {
			memcpy(
				m_shadercp->traced[currentPass].values_c[StartRegister],
				pConstantData,
				Vector4fCount * 4 * sizeof(float)
			);
		}
#endif

#ifdef	OBGE_LOGGING
		if (frame_log) {
			frame_log->FormattedMessage("SetPixelShaderConstantF[%d+]", StartRegister);
			frame_log->Indent();
			for (int v = 0; v < Vector4fCount; v++)
				frame_log->FormattedMessage("|%f|%f|%f|%f|", pConstantData[4*v+0], pConstantData[4*v+1], pConstantData[4*v+2], pConstantData[4*v+3]);
			frame_log->Outdent();
		}
#endif

		return m_device->SetPixelShaderConstantF(StartRegister, pConstantData, Vector4fCount);
	}

	STDMETHOD(GetPixelShaderConstantF)(THIS_ UINT StartRegister,float* pConstantData,UINT Vector4fCount)
	{
		return m_device->GetPixelShaderConstantF(StartRegister, pConstantData, Vector4fCount);
	}

	STDMETHOD(SetPixelShaderConstantI)(THIS_ UINT StartRegister,CONST int* pConstantData,UINT Vector4iCount)
	{
#ifdef	OBGE_DEVLING
		if (m_shadercp) {
			memcpy(
				m_shadercp->traced[currentPass].values_i[StartRegister],
				pConstantData,
				Vector4iCount * 4 * sizeof(int)
			);
		}
#endif

#ifdef	OBGE_LOGGING
		if (frame_log) {
			frame_log->FormattedMessage("SetPixelShaderConstantI[%d+]", StartRegister);
			frame_log->Indent();
			for (int v = 0; v < Vector4iCount; v++)
				frame_log->FormattedMessage("|%d|%d|%d|%d|", pConstantData[4*v+0], pConstantData[4*v+1], pConstantData[4*v+2], pConstantData[4*v+3]);
			frame_log->Outdent();
		}
#endif

		return m_device->SetPixelShaderConstantI(StartRegister, pConstantData, Vector4iCount);
	}

	STDMETHOD(GetPixelShaderConstantI)(THIS_ UINT StartRegister,int* pConstantData,UINT Vector4iCount)
	{
		return m_device->GetPixelShaderConstantI(StartRegister, pConstantData, Vector4iCount);
	}

	STDMETHOD(SetPixelShaderConstantB)(THIS_ UINT StartRegister,CONST BOOL* pConstantData,UINT  BoolCount)
	{
#ifdef	OBGE_LOGGING
		if (frame_log) {
			frame_log->FormattedMessage("SetPixelShaderConstantB[%d+]", StartRegister);
			frame_log->Indent();
			for (int v = 0; v < BoolCount; v++)
				frame_log->FormattedMessage("|%d|", pConstantData[v]);
			frame_log->Outdent();
		}
#endif

		return m_device->SetPixelShaderConstantB(StartRegister, pConstantData, BoolCount);
	}

	STDMETHOD(GetPixelShaderConstantB)(THIS_ UINT StartRegister,BOOL* pConstantData,UINT BoolCount)
	{
		return m_device->GetPixelShaderConstantB(StartRegister, pConstantData, BoolCount);
	}

	STDMETHOD(DrawRectPatch)(THIS_ UINT Handle,CONST float* pNumSegs,CONST D3DRECTPATCH_INFO* pRectPatchInfo)
	{
		return m_device->DrawRectPatch(Handle, pNumSegs, pRectPatchInfo);
	}

	STDMETHOD(DrawTriPatch)(THIS_ UINT Handle,CONST float* pNumSegs,CONST D3DTRIPATCH_INFO* pTriPatchInfo)
	{
		return m_device->DrawTriPatch(Handle, pNumSegs, pTriPatchInfo);
	}

	STDMETHOD(DeletePatch)(THIS_ UINT Handle)
	{
		return m_device->DeletePatch(Handle);
	}

	STDMETHOD(CreateQuery)(THIS_ D3DQUERYTYPE Type,IDirect3DQuery9** ppQuery)
	{
		return m_device->CreateQuery(Type, ppQuery);
	}

public:
	STDMETHOD(DumpFrameScript)(THIS)
	{
#ifdef	OBGE_LOGGING
		/* log the next 2 frames */
		frame_dmp = 15;
#endif

		return D3D_OK;
	}

	STDMETHOD(DumpFrameSurfaces)(THIS)
	{
		return D3D_OK;
	}

private:
	IDirect3DDevice9 *m_device;
	IDirect3D9 *m_d3d;

	ShaderManager *m_shaders;
	RuntimeShaderRecord *m_shadercv;
	RuntimeShaderRecord *m_shadercp;
};

#endif
