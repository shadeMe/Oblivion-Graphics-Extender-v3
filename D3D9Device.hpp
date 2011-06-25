#ifndef	OBGE_NOSHADER
#ifndef	D3D9DEVICE_HPP
#define	D3D9DEVICE_HPP

#include <intrin.h>
#include <d3d9.h>
#include <d3dx9.h>

#include "D3D9.hpp"
#include "D3D9Identifiers.hpp"

#if	defined(OBGE_TRACKER_SURFACES) || defined(OBGE_TRACKER_TEXTURES)
#include "D3D9Texture.hpp"
#include "D3D9Surface.hpp"
#endif

#include "TextureIOHook.hpp"
#include "ShaderManager.h"

/* ----------------------------------------------------------------------------- */

// Hook-Tracker
extern enum OBGEPass currentPass, previousPass;
extern IDirect3DTexture9 *passTexture[OBGEPASS_NUM];
extern IDirect3DTexture9 *passDepthT [OBGEPASS_NUM];
extern IDirect3DSurface9 *passSurface[OBGEPASS_NUM];
extern IDirect3DSurface9 *passDepth  [OBGEPASS_NUM];
extern int                passFrames [OBGEPASS_NUM];
extern int                passPasses [OBGEPASS_NUM];
extern const char        *passNames  [OBGEPASS_NUM];
extern const char        *passScene; // named scene

/* ----------------------------------------------------------------------------- */

/* hacking CreateTexure passed via the RenderSurfaceParameters-hook */
extern D3DTEXTUREFILTERTYPE AMFilter;
extern int Anisotropy;
extern float LODBias;

/* ------------------------------------------------------------------------------- */

#if	defined(OBGE_LOGGING)
extern int frame_dmp;
extern IDebugLog *frame_log;
#else
#define	frame_log ((IDebugLog *)NULL)
#endif

#if	defined(OBGE_DEVLING) || defined(OBGE_LOGGING)
extern int frame_num;
extern int frame_bge;
#else
#define frame_num 0
#define frame_bge 0
#endif

#if	defined(OBGE_DEVLING) && defined(OBGE_PROFILE)
extern LARGE_INTEGER frame_bgn;
extern LARGE_INTEGER frame_end;

extern bool frame_prf;
extern bool frame_ntx;
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
class OBGEDirect3DDevice9 : public IDirect3DDevice9
{

public:
	OBGEDirect3DDevice9(IDirect3D9* d3d, IDirect3DDevice9* device);
	~OBGEDirect3DDevice9();

public:
	/*** IUnknown methods ***/
	STDMETHOD(QueryInterface)(THIS_ REFIID riid, void** ppvObj);
	STDMETHOD_(ULONG,AddRef)(THIS);
	STDMETHOD_(ULONG,Release)(THIS);

	/*** IDirect3DDevice9 methods ***/
	STDMETHOD(TestCooperativeLevel)(THIS);
	STDMETHOD_(UINT, GetAvailableTextureMem)(THIS);
	STDMETHOD(EvictManagedResources)(THIS);
	STDMETHOD(GetDirect3D)(THIS_ IDirect3D9** ppD3D9);
	STDMETHOD(GetDeviceCaps)(THIS_ D3DCAPS9* pCaps);
	STDMETHOD(GetDisplayMode)(THIS_ UINT iSwapChain,D3DDISPLAYMODE* pMode);
	STDMETHOD(GetCreationParameters)(THIS_ D3DDEVICE_CREATION_PARAMETERS *pParameters);
	STDMETHOD(SetCursorProperties)(THIS_ UINT XHotSpot,UINT YHotSpot,IDirect3DSurface9* pCursorBitmap);
	STDMETHOD_(void, SetCursorPosition)(THIS_ int X,int Y,DWORD Flags);
	STDMETHOD_(BOOL, ShowCursor)(THIS_ BOOL bShow);
	STDMETHOD(CreateAdditionalSwapChain)(THIS_ D3DPRESENT_PARAMETERS* pPresentationParameters,IDirect3DSwapChain9** pSwapChain);
	STDMETHOD(GetSwapChain)(THIS_ UINT iSwapChain,IDirect3DSwapChain9** pSwapChain);
	STDMETHOD_(UINT, GetNumberOfSwapChains)(THIS);
	STDMETHOD(Reset)(THIS_ D3DPRESENT_PARAMETERS* pPresentationParameters);
	STDMETHOD(Present)(THIS_ CONST RECT* pSourceRect,CONST RECT* pDestRect,HWND hDestWindowOverride,CONST RGNDATA* pDirtyRegion);
	STDMETHOD(GetBackBuffer)(THIS_ UINT iSwapChain,UINT iBackBuffer,D3DBACKBUFFER_TYPE Type,IDirect3DSurface9** ppBackBuffer);
	STDMETHOD(GetRasterStatus)(THIS_ UINT iSwapChain,D3DRASTER_STATUS* pRasterStatus);
	STDMETHOD(SetDialogBoxMode)(THIS_ BOOL bEnableDialogs);
	STDMETHOD_(void, SetGammaRamp)(THIS_ UINT iSwapChain,DWORD Flags,CONST D3DGAMMARAMP* pRamp);
	STDMETHOD_(void, GetGammaRamp)(THIS_ UINT iSwapChain,D3DGAMMARAMP* pRamp);
	STDMETHOD(CreateTexture)(THIS_ UINT Width,UINT Height,UINT Levels,DWORD Usage,D3DFORMAT Format,D3DPOOL Pool,IDirect3DTexture9** ppTexture,HANDLE* pSharedHandle);
	STDMETHOD(CreateVolumeTexture)(THIS_ UINT Width,UINT Height,UINT Depth,UINT Levels,DWORD Usage,D3DFORMAT Format,D3DPOOL Pool,IDirect3DVolumeTexture9** ppVolumeTexture,HANDLE* pSharedHandle);
	STDMETHOD(CreateCubeTexture)(THIS_ UINT EdgeLength,UINT Levels,DWORD Usage,D3DFORMAT Format,D3DPOOL Pool,IDirect3DCubeTexture9** ppCubeTexture,HANDLE* pSharedHandle);
	STDMETHOD(CreateVertexBuffer)(THIS_ UINT Length,DWORD Usage,DWORD FVF,D3DPOOL Pool,IDirect3DVertexBuffer9** ppVertexBuffer,HANDLE* pSharedHandle);
	STDMETHOD(CreateIndexBuffer)(THIS_ UINT Length,DWORD Usage,D3DFORMAT Format,D3DPOOL Pool,IDirect3DIndexBuffer9** ppIndexBuffer,HANDLE* pSharedHandle);
	STDMETHOD(CreateRenderTarget)(THIS_ UINT Width,UINT Height,D3DFORMAT Format,D3DMULTISAMPLE_TYPE MultiSample,DWORD MultisampleQuality,BOOL Lockable,IDirect3DSurface9** ppSurface,HANDLE* pSharedHandle);
	STDMETHOD(CreateDepthStencilSurface)(THIS_ UINT Width,UINT Height,D3DFORMAT Format,D3DMULTISAMPLE_TYPE MultiSample,DWORD MultisampleQuality,BOOL Discard,IDirect3DSurface9** ppSurface,HANDLE* pSharedHandle);
	STDMETHOD(UpdateSurface)(THIS_ IDirect3DSurface9* pSourceSurface,CONST RECT* pSourceRect,IDirect3DSurface9* pDestinationSurface,CONST POINT* pDestPoint);
	STDMETHOD(UpdateTexture)(THIS_ IDirect3DBaseTexture9* pSourceTexture,IDirect3DBaseTexture9* pDestinationTexture);
	STDMETHOD(GetRenderTargetData)(THIS_ IDirect3DSurface9* pRenderTarget,IDirect3DSurface9* pDestSurface);
	STDMETHOD(GetFrontBufferData)(THIS_ UINT iSwapChain,IDirect3DSurface9* pDestSurface);
	STDMETHOD(StretchRect)(THIS_ IDirect3DSurface9* pSourceSurface,CONST RECT* pSourceRect,IDirect3DSurface9* pDestSurface,CONST RECT* pDestRect,D3DTEXTUREFILTERTYPE Filter);
	STDMETHOD(ColorFill)(THIS_ IDirect3DSurface9* pSurface,CONST RECT* pRect,D3DCOLOR color);
	STDMETHOD(CreateOffscreenPlainSurface)(THIS_ UINT Width,UINT Height,D3DFORMAT Format,D3DPOOL Pool,IDirect3DSurface9** ppSurface,HANDLE* pSharedHandle);
	STDMETHOD(SetRenderTarget)(THIS_ DWORD RenderTargetIndex,IDirect3DSurface9* pRenderTarget);
	STDMETHOD(GetRenderTarget)(THIS_ DWORD RenderTargetIndex,IDirect3DSurface9** ppRenderTarget);
	STDMETHOD(SetDepthStencilSurface)(THIS_ IDirect3DSurface9* pNewZStencil);
	STDMETHOD(GetDepthStencilSurface)(THIS_ IDirect3DSurface9** ppZStencilSurface);
	STDMETHOD(BeginScene)(THIS);
	STDMETHOD(EndScene)(THIS);
	STDMETHOD(Clear)(THIS_ DWORD Count,CONST D3DRECT* pRects,DWORD Flags,D3DCOLOR Color,float Z,DWORD Stencil);
	STDMETHOD(SetTransform)(THIS_ D3DTRANSFORMSTATETYPE State,CONST D3DMATRIX* pMatrix);
	STDMETHOD(GetTransform)(THIS_ D3DTRANSFORMSTATETYPE State,D3DMATRIX* pMatrix);
	STDMETHOD(MultiplyTransform)(THIS_ D3DTRANSFORMSTATETYPE,CONST D3DMATRIX*);
	STDMETHOD(SetViewport)(THIS_ CONST D3DVIEWPORT9* pViewport);
	STDMETHOD(GetViewport)(THIS_ D3DVIEWPORT9* pViewport);
	STDMETHOD(SetMaterial)(THIS_ CONST D3DMATERIAL9* pMaterial);
	STDMETHOD(GetMaterial)(THIS_ D3DMATERIAL9* pMaterial);
	STDMETHOD(SetLight)(THIS_ DWORD Index,CONST D3DLIGHT9*);
	STDMETHOD(GetLight)(THIS_ DWORD Index,D3DLIGHT9*);
	STDMETHOD(LightEnable)(THIS_ DWORD Index,BOOL Enable);
	STDMETHOD(GetLightEnable)(THIS_ DWORD Index,BOOL* pEnable);
	STDMETHOD(SetClipPlane)(THIS_ DWORD Index,CONST float* pPlane);
	STDMETHOD(GetClipPlane)(THIS_ DWORD Index,float* pPlane);
	STDMETHOD(SetRenderState)(THIS_ D3DRENDERSTATETYPE State,DWORD Value);
	STDMETHOD(GetRenderState)(THIS_ D3DRENDERSTATETYPE State,DWORD* pValue);
	STDMETHOD(CreateStateBlock)(THIS_ D3DSTATEBLOCKTYPE Type,IDirect3DStateBlock9** ppSB);
	STDMETHOD(BeginStateBlock)(THIS);
	STDMETHOD(EndStateBlock)(THIS_ IDirect3DStateBlock9** ppSB);
	STDMETHOD(SetClipStatus)(THIS_ CONST D3DCLIPSTATUS9* pClipStatus);
	STDMETHOD(GetClipStatus)(THIS_ D3DCLIPSTATUS9* pClipStatus);
	STDMETHOD(GetTexture)(THIS_ DWORD Stage,IDirect3DBaseTexture9** ppTexture);
	STDMETHOD(SetTexture)(THIS_ DWORD Stage,IDirect3DBaseTexture9* pTexture);
	STDMETHOD(GetTextureStageState)(THIS_ DWORD Stage,D3DTEXTURESTAGESTATETYPE Type,DWORD* pValue);
	STDMETHOD(SetTextureStageState)(THIS_ DWORD Stage,D3DTEXTURESTAGESTATETYPE Type,DWORD Value);
	STDMETHOD(GetSamplerState)(THIS_ DWORD Sampler,D3DSAMPLERSTATETYPE Type,DWORD* pValue);
	STDMETHOD(SetSamplerState)(THIS_ DWORD Sampler,D3DSAMPLERSTATETYPE Type,DWORD Value);
	STDMETHOD(ValidateDevice)(THIS_ DWORD* pNumPasses);
	STDMETHOD(SetPaletteEntries)(THIS_ UINT PaletteNumber,CONST PALETTEENTRY* pEntries);
	STDMETHOD(GetPaletteEntries)(THIS_ UINT PaletteNumber,PALETTEENTRY* pEntries);
	STDMETHOD(SetCurrentTexturePalette)(THIS_ UINT PaletteNumber);
	STDMETHOD(GetCurrentTexturePalette)(THIS_ UINT *PaletteNumber);
	STDMETHOD(SetScissorRect)(THIS_ CONST RECT* pRect);
	STDMETHOD(GetScissorRect)(THIS_ RECT* pRect);
	STDMETHOD(SetSoftwareVertexProcessing)(THIS_ BOOL bSoftware);
	STDMETHOD_(BOOL, GetSoftwareVertexProcessing)(THIS);
	STDMETHOD(SetNPatchMode)(THIS_ float nSegments);
	STDMETHOD_(float, GetNPatchMode)(THIS);
	STDMETHOD(DrawPrimitive)(THIS_ D3DPRIMITIVETYPE PrimitiveType,UINT StartVertex,UINT PrimitiveCount);
	STDMETHOD(DrawIndexedPrimitive)(THIS_ D3DPRIMITIVETYPE,INT BaseVertexIndex,UINT MinVertexIndex,UINT NumVertices,UINT startIndex,UINT primCount);
	STDMETHOD(DrawPrimitiveUP)(THIS_ D3DPRIMITIVETYPE PrimitiveType,UINT PrimitiveCount,CONST void* pVertexStreamZeroData,UINT VertexStreamZeroStride);
	STDMETHOD(DrawIndexedPrimitiveUP)(THIS_ D3DPRIMITIVETYPE PrimitiveType,UINT MinVertexIndex,UINT NumVertices,UINT PrimitiveCount,CONST void* pIndexData,D3DFORMAT IndexDataFormat,CONST void* pVertexStreamZeroData,UINT VertexStreamZeroStride);
	STDMETHOD(ProcessVertices)(THIS_ UINT SrcStartIndex,UINT DestIndex,UINT VertexCount,IDirect3DVertexBuffer9* pDestBuffer,IDirect3DVertexDeclaration9* pVertexDecl,DWORD Flags);
	STDMETHOD(CreateVertexDeclaration)(THIS_ CONST D3DVERTEXELEMENT9* pVertexElements,IDirect3DVertexDeclaration9** ppDecl);
	STDMETHOD(SetVertexDeclaration)(THIS_ IDirect3DVertexDeclaration9* pDecl);
	STDMETHOD(GetVertexDeclaration)(THIS_ IDirect3DVertexDeclaration9** ppDecl);
	STDMETHOD(SetFVF)(THIS_ DWORD FVF);
	STDMETHOD(GetFVF)(THIS_ DWORD* pFVF);
	STDMETHOD(CreateVertexShader)(THIS_ CONST DWORD* pFunction,IDirect3DVertexShader9** ppShader);
	STDMETHOD(SetVertexShader)(THIS_ IDirect3DVertexShader9* pShader);
	STDMETHOD(GetVertexShader)(THIS_ IDirect3DVertexShader9** ppShader);
	STDMETHOD(SetVertexShaderConstantF)(THIS_ UINT StartRegister,CONST float* pConstantData,UINT Vector4fCount);
	STDMETHOD(GetVertexShaderConstantF)(THIS_ UINT StartRegister,float* pConstantData,UINT Vector4fCount);
	STDMETHOD(SetVertexShaderConstantI)(THIS_ UINT StartRegister,CONST int* pConstantData,UINT Vector4iCount);
	STDMETHOD(GetVertexShaderConstantI)(THIS_ UINT StartRegister,int* pConstantData,UINT Vector4iCount);
	STDMETHOD(SetVertexShaderConstantB)(THIS_ UINT StartRegister,CONST BOOL* pConstantData,UINT  BoolCount);
	STDMETHOD(GetVertexShaderConstantB)(THIS_ UINT StartRegister,BOOL* pConstantData,UINT BoolCount);
	STDMETHOD(SetStreamSource)(THIS_ UINT StreamNumber,IDirect3DVertexBuffer9* pStreamData,UINT OffsetInBytes,UINT Stride);
	STDMETHOD(GetStreamSource)(THIS_ UINT StreamNumber,IDirect3DVertexBuffer9** ppStreamData,UINT* pOffsetInBytes,UINT* pStride);
	STDMETHOD(SetStreamSourceFreq)(THIS_ UINT StreamNumber,UINT Setting);
	STDMETHOD(GetStreamSourceFreq)(THIS_ UINT StreamNumber,UINT* pSetting);
	STDMETHOD(SetIndices)(THIS_ IDirect3DIndexBuffer9* pIndexData);
	STDMETHOD(GetIndices)(THIS_ IDirect3DIndexBuffer9** ppIndexData);
	STDMETHOD(CreatePixelShader)(THIS_ CONST DWORD* pFunction,IDirect3DPixelShader9** ppShader);
	STDMETHOD(SetPixelShader)(THIS_ IDirect3DPixelShader9* pShader);
	STDMETHOD(GetPixelShader)(THIS_ IDirect3DPixelShader9** ppShader);
	STDMETHOD(SetPixelShaderConstantF)(THIS_ UINT StartRegister,CONST float* pConstantData,UINT Vector4fCount);
	STDMETHOD(GetPixelShaderConstantF)(THIS_ UINT StartRegister,float* pConstantData,UINT Vector4fCount);
	STDMETHOD(SetPixelShaderConstantI)(THIS_ UINT StartRegister,CONST int* pConstantData,UINT Vector4iCount);
	STDMETHOD(GetPixelShaderConstantI)(THIS_ UINT StartRegister,int* pConstantData,UINT Vector4iCount);
	STDMETHOD(SetPixelShaderConstantB)(THIS_ UINT StartRegister,CONST BOOL* pConstantData,UINT  BoolCount);
	STDMETHOD(GetPixelShaderConstantB)(THIS_ UINT StartRegister,BOOL* pConstantData,UINT BoolCount);
	STDMETHOD(DrawRectPatch)(THIS_ UINT Handle,CONST float* pNumSegs,CONST D3DRECTPATCH_INFO* pRectPatchInfo);
	STDMETHOD(DrawTriPatch)(THIS_ UINT Handle,CONST float* pNumSegs,CONST D3DTRIPATCH_INFO* pTriPatchInfo);
	STDMETHOD(DeletePatch)(THIS_ UINT Handle);
	STDMETHOD(CreateQuery)(THIS_ D3DQUERYTYPE Type,IDirect3DQuery9** ppQuery);

public:
	STDMETHOD(DumpFrameScript)(THIS);
	STDMETHOD(DumpFrameSurfaces)(THIS);

private:
	IDirect3DDevice9 *m_device;
	IDirect3D9 *m_d3d;

	ShaderManager *m_shaders;
	RuntimeShaderRecord *m_shadercv;
	RuntimeShaderRecord *m_shadercp;

#if	defined(OBGE_DEVLING) && defined(OBGE_PROFILE)
	IDirect3DQuery9* pEvent;
#endif
};

#endif
#else
#define	frame_log ((IDebugLog *)NULL)
#define lastOBGEDirect3DDevice9	GetD3DDevice()
#endif
