#include <sys/stat.h>

#include <algorithm>

#include "EffectManager.h"
#include "TextureManager.h"
#include "GlobalSettings.h"
#include "OBSEShaderInterface.h"

#include "D3D9.hpp"
#include "D3D9Device.hpp"

static global<bool> UseEffectList(true, NULL, "Effects", "bUseEffectList");
static global<char *> EffectDirectory("data\\shaders\\", NULL, "Effects", "sEffectDirectory");
static global<char *> EffectListFile("data\\shaders\\shaderlist.txt", NULL, "Effects", "sEffectListFile");
static global<bool> UseLegacyCompiler(false, NULL, "Effects", "bUseLegacyCompiler");
static global<bool> Optimize(false, NULL, "Effects", "bOptimize");
static global<bool> FreezeTweaks(false, NULL, "Effects", "bFreezeTweaks");
static global<bool> SplitScreen(false, NULL, "Effects", "bRenderHalfScreen");
static global<bool> PurgeOnNewGame(false, NULL, "Effects", "bPurgeOnNewGame");
static global<int> BufferTexturesNumBits(0, NULL, "ScreenBuffers", "iBufferTexturesNumBits");
static global<int> BufferRawZDepthNumBits(0, NULL, "ScreenBuffers", "iBufferRawZDepthNumBits");

/* #################################################################################################
 */

class FXIncludeManager : public ID3DXInclude
{
public:
    STDMETHOD(Open)(D3DXINCLUDE_TYPE IncludeType, LPCSTR pFileName, LPCVOID pParentData, LPCVOID *ppData, UINT *pBytes);
    STDMETHOD(Close)(LPCVOID pData);
};

HRESULT FXIncludeManager::Open(D3DXINCLUDE_TYPE IncludeType, LPCSTR pName, LPCVOID pParentData, LPCVOID *ppData, UINT *pBytes)
{
    struct stat s;
    FILE *f;
    char strFileFull[MAX_PATH];

    switch (IncludeType) {
      case D3DXINC_LOCAL: strcpy(strFileFull, ::EffectDirectory.Get()); strcat(strFileFull, pName); break;
      case D3DXINC_SYSTEM: strcpy(strFileFull, ""); strcat(strFileFull, pName); break;
    }

    if (stat((const char *)strFileFull, &s))
      return E_FAIL;

    UINT size = s.st_size;

    BYTE* pData = new BYTE[size];
    if (pData == NULL)
        return E_OUTOFMEMORY;

    if (fopen_s(&f, strFileFull, "rb"/*"rt"*/))
      return E_FAIL;

    fread(pData, 1, size, f);
    fclose(f);

    *ppData = pData;
    *pBytes = size;

    return S_OK;
}

HRESULT FXIncludeManager::Close(LPCVOID pData)
{
    BYTE* pData2 = (BYTE*)pData;
    if (pData2)
      delete[] pData2;
    return S_OK;
}

static FXIncludeManager incl;

/* #################################################################################################
 */

#define	strprete(i)	#i
#define	stringify(i)	strprete(i)

#define	EFFECTGROUP_FIRST	0x00
#define	EFFECTGROUP_PRE		0x10
#define	EFFECTGROUP_MAIN	0x40
#define	EFFECTGROUP_POST	0xC0
#define	EFFECTGROUP_LAST	0xFF

#define	EFFECTCLASS_NEUTRAL	0x00
#define	EFFECTCLASS_AO	        0x01
#define	EFFECTCLASS_LIGHT	0x02
#define	EFFECTCLASS_WATER	0x03
#define	EFFECTCLASS_DOF		0x04
#define	EFFECTCLASS_COLOR	0x05
#define	EFFECTCLASS_FILTER	0x06

#define	EFFECTCOND_HASSUN	(1 << 0)
#define	EFFECTCOND_HASWATER	(1 << 1)
#define	EFFECTCOND_INTERIOUR	(1 << 8)
#define	EFFECTCOND_EXTERIOUR	(1 << 9)
#define	EFFECTCOND_UNDERWATER	(1 << 10)
#define	EFFECTCOND_ISDAY	(1 << 16)
#define	EFFECTCOND_ISNIGHT	(1 << 17)
#define	EFFECTCOND_HASREFL	(1 << 18)
#define	EFFECTCOND_HASACHN	(1 << 19)	// special one
#define	EFFECTCOND_HASMIPS	(1 << 20)	// special one
#define	EFFECTCOND_HASWNRM	(1 << 26)	// special one
#define	EFFECTCOND_HASWPOS	(1 << 27)	// special one
#define	EFFECTCOND_HASENRM	(1 << 28)	// special one
#define	EFFECTCOND_HASEPOS	(1 << 29)	// special one
#define	EFFECTCOND_HASLBUF	(1 << 30)	// special one
#define	EFFECTCOND_HASZBUF	(1 << 31)	// special one

#define	EFFECTBUF_RAWZ		(1 <<  0)	// special one
#define	EFFECTBUF_COPY		(1 <<  1)	// need frame "copy" because the effect surfaces have other format
#define	EFFECTBUF_PREV		(1 <<  2)	// need previous effect "copy"
#define	EFFECTBUF_PAST		(1 <<  3)	// need previous frame "copy"
#define	EFFECTBUF_ACHN		EFFECTCOND_HASACHN
#define	EFFECTBUF_WNRM		EFFECTCOND_HASWNRM
#define	EFFECTBUF_WPOS		EFFECTCOND_HASWPOS
#define	EFFECTBUF_ENRM		EFFECTCOND_HASENRM
#define	EFFECTBUF_EPOS		EFFECTCOND_HASEPOS
#define	EFFECTBUF_LBUF		EFFECTCOND_HASLBUF
#define	EFFECTBUF_ZBUF		EFFECTCOND_HASZBUF
#define	EFFECTBUF_ZMASK		(EFFECTCOND_HASWNRM | EFFECTCOND_HASWPOS | EFFECTCOND_HASENRM | EFFECTCOND_HASEPOS | EFFECTCOND_HASLBUF | EFFECTCOND_HASZBUF | EFFECTBUF_RAWZ)
#define	EFFECTBUF_TRANSFERZMASK	(EFFECTCOND_HASWNRM | EFFECTCOND_HASWPOS | EFFECTCOND_HASENRM | EFFECTCOND_HASEPOS | EFFECTCOND_HASLBUF |                      EFFECTBUF_RAWZ)

/* deprecated */
#ifndef	NO_DEPRECATED
#undef	EFFECTBUF_ACHN
#define	EFFECTBUF_ACHN		-1
#endif

typedef struct _myD3DXMACRO
{
  const char *Name;
  char *Definition;
} myD3DXMACRO;

static char IN_RAWZ[] = "0";
static char IN_LINZ[] = "0";
static char IN_PRJZ[] = "0";
static char IN_NRMZ[] = "0";
static char *TWEAK_DYNAMIC = "extern";
static char *TWEAK_FROZEN  = "static const";

#define	DEFS_INRAWZ	0
#define	DEFS_INLINZ	1
#define	DEFS_INPRJZ	2
#define	DEFS_INNRMZ	3
#define	DEFS_iface	4

static myD3DXMACRO defs[] = {
  {"IN_RAWZ"	        	, IN_RAWZ},
  {"IN_LINZ"	        	, IN_LINZ},
  {"IN_PRJZ"	        	, IN_PRJZ},
  {"IN_NRMZ"	        	, IN_NRMZ},
  {"iface"	        	, TWEAK_DYNAMIC},

#ifndef	OBGE_CONSTANTPOOLS
  {"shared"	        	, ""},
#endif

  {"EFFECTGROUP_FIRST"		, stringify(EFFECTGROUP_FIRST		)},
  {"EFFECTGROUP_PRE"		, stringify(EFFECTGROUP_PRE		)},
  {"EFFECTGROUP_MAIN"		, stringify(EFFECTGROUP_MAIN		)},
  {"EFFECTGROUP_POST"		, stringify(EFFECTGROUP_POST		)},
  {"EFFECTGROUP_LAST"		, stringify(EFFECTGROUP_LAST		)},

  {"EFFECTCLASS_NEUTRAL"	, stringify(EFFECTCLASS_NEUTRAL		)},
  {"EFFECTCLASS_LIGHT"		, stringify(EFFECTCLASS_LIGHT		)},
  {"EFFECTCLASS_AO"		, stringify(EFFECTCLASS_AO		)},
  {"EFFECTCLASS_WATER"		, stringify(EFFECTCLASS_WATER		)},
  {"EFFECTCLASS_DOF"		, stringify(EFFECTCLASS_DOF		)},
  {"EFFECTCLASS_COLOR"		, stringify(EFFECTCLASS_COLOR		)},
  {"EFFECTCLASS_FILTER"		, stringify(EFFECTCLASS_FILTER		)},

  {"EFFECTCOND_HASSUN"	        , stringify(EFFECTCOND_HASSUN	        )},
  {"EFFECTCOND_HASWATER"	, stringify(EFFECTCOND_HASWATER	        )},
  {"EFFECTCOND_HASREFLECTIONS"	, stringify(EFFECTCOND_HASREFL	        )},
  {"EFFECTCOND_HASZBUFFER"	, stringify(EFFECTCOND_HASZBUF		)},
  {"EFFECTCOND_HASMIPMAPS"	, stringify(EFFECTCOND_HASMIPS		)},
  {"EFFECTCOND_INTERIOUR"	, stringify(EFFECTCOND_INTERIOUR	)},
  {"EFFECTCOND_EXTERIOUR"	, stringify(EFFECTCOND_EXTERIOUR	)},
  {"EFFECTCOND_UNDERWATER"	, stringify(EFFECTCOND_UNDERWATER	)},
  {"EFFECTCOND_ISDAY"	        , stringify(EFFECTCOND_ISDAY	        )},
  {"EFFECTCOND_ISNIGHT"	        , stringify(EFFECTCOND_ISNIGHT	        )},

  {"EFFECTCOND_ZBUFFER"		, stringify(EFFECTCOND_HASZBUF		)}, // hm, error
  {"EFFECTCOND_LBUFFER"		, stringify(EFFECTCOND_HASLBUF		)}, // hm, error
  {"EFFECTCOND_PBUFFER"		, stringify(EFFECTCOND_HASEPOS		)}, // hm, error
  {"EFFECTCOND_NBUFFER"		, stringify(EFFECTCOND_HASENRM		)}, // hm, error
  {"EFFECTCOND_MIPMAPS"		, stringify(EFFECTCOND_HASMIPS		)}, // hm, error
  {"EFFECTCOND_ACHANNEL"	, stringify(EFFECTCOND_HASACHN		)}, // hm, error

  {"D3DFMT_DEFAULT"	        , "0"},		// same format as main-pass surface

/* deprecated */
#ifndef	NO_DEPRECATED
  {"lastframe"			, "obge_PastRendertarget0_MAINPASS"	 },
  {"lastpass"			, "obge_LastRendertarget0_EFFECTPASS"	 },
  {"thisframe"			, "obge_PrevRendertarget0_EFFECTPASS"	 },

  {"reflection"			, "oblv_Rendertarget0_REFLECTIONPASS"	 },
  {"Depth"			, "oblv_CurrDepthStencilZ_MAINPASS"	 },
//{"rcpres"			, "oblv_ReciprocalResolution_MAINPASS"	 },

  {"m44world"			, "oblv_WorldTransform_MAINPASS"	 },
  {"m44view"			, "oblv_ViewTransform_MAINPASS"		 },
  {"m44proj"			, "oblv_ProjectionTransform_MAINPASS"	 },
  {"f3EyeForward"		, "oblv_CameraForward_MAINPASS"		 },
//{"f4Time"			, "oblv_GameTime"			 },
  {"f4SunDir"			, "oblv_SunDirection"			 },
#endif

  {NULL, NULL}
};

/* #################################################################################################
 */

EffectBuffer::EffectBuffer() {
  for (int rt = 0; rt < EBUFRT_NUM; rt++) {
    Tex[rt] = NULL;
    Srf[rt] = NULL;
  }

  mine = false;
}

EffectBuffer::~EffectBuffer() {
  Release();
}

inline HRESULT EffectBuffer::Initialise(IDirect3DTexture9 *text) {
  Release();

  Tex[0] = text;
//Tex[0]->GetSurfaceLevel(0, &Srf[0]);

  return (Srf[0] ? D3D_OK : S_FALSE);
}

inline HRESULT EffectBuffer::Initialise(IDirect3DSurface9 *surf) {
  IDirect3DTexture9 *text = NULL;

#ifndef	OBGE_NOSHADER
  text = surfaceTexture[surf] ? surfaceTexture[surf]->tex : NULL;
#endif
  surf =                surf                                    ;

  if (!text) {
    /* no alternative replacement target available, so we
     * preserve the surface anyway
     */
    if (!Tex[0]) {
      Tex[0] = text;
      Srf[0] = surf;
    }

    return S_FALSE;
  }

  /* this is now the texture/surface-pair to use */
  Release();

  Tex[0] = text;
  Srf[0] = surf;

  return D3D_OK;
}

inline HRESULT EffectBuffer::Initialise(const D3DFORMAT fmt[EBUFRT_NUM]) {
  UInt32 Width  = v1_2_416::GetRenderer()->SizeWidth;
  UInt32 Height = v1_2_416::GetRenderer()->SizeHeight;
  HRESULT hr;

#if	defined(OBGE_AUTOMIPMAP)
#define EFFECT_USAGE  D3DUSAGE_RENDERTARGET | D3DUSAGE_AUTOGENMIPMAP
#else
#define EFFECT_USAGE  D3DUSAGE_RENDERTARGET
#endif

  for (int rt = 0; rt < EBUFRT_NUM; rt++) {
    if (!Tex[rt] && (fmt[rt] != D3DFMT_UNKNOWN)) {
      if ((hr = lastOBGEDirect3DDevice9->CreateTexture(Width, Height, 1, EFFECT_USAGE, fmt[rt], D3DPOOL_DEFAULT, &Tex[0], 0)) == D3D_OK)
	Tex[rt]->GetSurfaceLevel(0, &Srf[rt]);
#if	defined(OBGE_AUTOMIPMAP)
      if (Tex[rt] && (AMFilter != D3DTEXF_NONE))
	Tex[rt]->SetAutoGenFilterType(AMFilter);
#endif
    }
  }

  mine = true;
  return D3D_OK;
}

inline void EffectBuffer::Release() {
  if (mine)
    for (int rt = 0; rt < EBUFRT_NUM; rt++) {
      if (Tex[rt]) Tex[rt]->Release(); Tex[rt] = NULL;
      if (Srf[rt]) Srf[rt]->Release(); Srf[rt] = NULL;
    }
  else
    for (int rt = 0; rt < EBUFRT_NUM; rt++) {
      Tex[rt] = NULL;
      Srf[rt] = NULL;
    }

  mine = false;
}

inline bool EffectBuffer::IsValid() {
  for (int rt = 0; rt < EBUFRT_NUM; rt++)
    if (Tex[rt]) return true;

  return false;
}

bool EffectBuffer::IsTexture(IDirect3DBaseTexture9 *text) {
  for (int rt = 0; rt < EBUFRT_NUM; rt++)
    if (Tex[rt] == text) return true;

  return false;
}

inline void EffectBuffer::SetTexture(const char *fmt, ID3DXEffect *pEffect) {
  char buf[256];
  for (int rt = 0; rt < EBUFRT_NUM; rt++) {
    if (Tex[rt]) {
      sprintf(buf, fmt, rt);
      pEffect->SetTexture(buf, Tex[rt]);
    }
  }
}

inline void EffectBuffer::SetRenderTarget(IDirect3DDevice9 *Device) {
  for (int rt = 0; rt < EBUFRT_NUM; rt++) {
    if (Srf[rt])
      Device->SetRenderTarget(rt, Srf[rt]);
  }
}

inline void EffectBuffer::Copy(IDirect3DDevice9 *Device, EffectBuffer *from) {
  for (int rt = 0; rt < EBUFRT_NUM; rt++) {
    if (from->Srf[rt] && Srf[rt] && (from->Srf[rt] != Srf[rt]))
      minStretchRect(Device, from->Srf[rt], 0, Srf[rt], 0, D3DTEXF_NONE);
  }
}

inline void EffectBuffer::Copy(IDirect3DDevice9 *Device, IDirect3DSurface9 *from) {
  for (int rt = 0; rt < EBUFRT_NUM; rt++) {
    if (from          && Srf[rt] && (from          != Srf[rt]))
      minStretchRect(Device, from, 0, Srf[rt], 0, D3DTEXF_NONE);
  }
}

/* #################################################################################################
 */

#define EQLAST	0
#define EQPREV	1

inline void EffectQueue::Init(EffectBuffer *past,
			      EffectBuffer *prev,
			      EffectBuffer *alt) {
  this->past = past;
  this->prev = prev;

  queue[1] = alt;
}

inline void EffectQueue::Begin(EffectBuffer *orig,
			       EffectBuffer *target,
			       EffectBuffer *alt) {

  this->orig = orig;

  /* both buffers in the queue are of distinct type from target (the original surface) */
  if (!alt)
    (queue[0] = target);
  else
    (queue[0] = alt)->Copy(device, target);

  /* 1)  begin() alterning = 1
   *  a) begin() alterning = 0, 0 == target, setrendertarget(target)
   *     step()  alterning = 1, 1 == alt,    setrendertarget(alt)
   *     end()
   *  b) begin() alterning = 0, 0 == target, setrendertarget(target)
   *     step()  alterning = 1, 1 == alt,    setrendertarget(alt)
   *     step()  alterning = 0, 0 == target, setrendertarget(target)
   *     end()
   *  c) begin() alterning = 1, 1 == alt,    setrendertarget(alt)
   *     step()  alterning = 0, 0 == target, setrendertarget(target)
   *     step()  alterning = 1, 1 == alt,    setrendertarget(alt)
   *     end()
   *     end()   alterning = 1, 1 == alt, copy alt to target
   */
  alterning = 1;

  /* initially prev is orig */
  rotate[EQPREV] = orig;
  rotate[EQLAST] = orig;
}

inline void EffectQueue::Begin(ID3DXEffect *pEffect) {
  /* set this effects initial constant parameters */
  if (past) past->SetTexture("obge_PastRendertarget%d_MAINPASS", pEffect);
  /*     */ orig->SetTexture("oblv_CurrRendertarget%d_MAINPASS", pEffect);

  rotate[EQPREV]->SetTexture("obge_PrevRendertarget%d_EFFECTPASS", pEffect);
  rotate[EQLAST]->SetTexture("obge_LastRendertarget%d_EFFECTPASS", pEffect);

  alterning ^= 1; (rotate[EQLAST] = queue[alterning])->SetRenderTarget(device);
  device->Clear(0, NULL, D3DCLEAR_TARGET, 0, 0, 0);
}

inline void EffectQueue::Step(ID3DXEffect *pEffect) {
  rotate[EQLAST]->SetTexture("obge_LastRendertarget%d_EFFECTPASS", pEffect);

  alterning ^= 1; (rotate[EQLAST] = queue[alterning])->SetRenderTarget(device);
  device->Clear(0, NULL, D3DCLEAR_TARGET, 0, 0, 0);
}

inline void EffectQueue::End(ID3DXEffect *pEffect) {
  /* not allocated if not needed! */
  if (prev) {
    /* TODO: I believe this can be done, it's only complex */
  /*if (alterning) {
      IDirect3DSurface9 *swap[4];
      swap[0] = prev[0];
      prev[0] = queue[1][0];
      queue[1][0] = swap[0];
    }
    else*/
      /* once we have an updated prev we move to that rendertarget */
      (rotate[EQPREV] = prev)->Copy(device, rotate[EQLAST]);
  }
}

inline void EffectQueue::End(EffectBuffer *target) {
  /* this occurs when "target" never entered the queue,
   * or when we have an odd number of accumulated passes
   */
  target->Copy(device, rotate[EQLAST]);

  /* not allocated if not needed! */
  if (past)
    past->Copy(device, orig);

  /* restore original rendertarget (Oblivion expects this!) */
  target->SetRenderTarget(device);
}

#undef EQLAST
#undef EQPREV

/* #################################################################################################
 */

EffectRecord::EffectRecord() {
  Filepath[0] = '\0';
  Name[0] = '\0';
  ParentRefID = 0;

  Enabled = false;
  Private = false;

  /* shader source, binaries and constant tables */
  pDefine = NULL;
  pBinary = NULL;
  pEffect = NULL;
  pSource = NULL; sourceLen = 0;
  pErrorMsgs = NULL;
  pDisasmbly = NULL;

  Parameters = 0;
  Priority = 0;
  Flags = 0;
  Class = 0;
}

EffectRecord::~EffectRecord() {
  if (pBinary) pBinary->Release();
  if (pEffect) pEffect->Release();
  if (pSource) delete[] pSource;
  if (pErrorMsgs) pErrorMsgs->Release();
  if (pDisasmbly) pDisasmbly->Release();

  /* release previous texture */
  TextureManager *TexMan = TextureManager::GetSingleton();
  std::vector<int>::iterator PTexture = Textures.begin();

  while (PTexture != Textures.end()) {
    TexMan->ReleaseTexture(*PTexture);
    PTexture++;
  }
}

void EffectRecord::Kill() {
  if (pBinary) while (pBinary->Release()) {};
  if (pEffect) while (pEffect->Release()) {};
  if (pSource) delete[] pSource;
  if (pErrorMsgs) pErrorMsgs->Release();
  if (pDisasmbly) pDisasmbly->Release();

  this->pDefine = NULL;
  this->pBinary = NULL;
  this->pEffect = NULL;
  this->pSource = NULL;
  this->pErrorMsgs = NULL;
  this->pDisasmbly = NULL;

  this->Name[0] = '\0';
  this->Filepath[0] = '\0';
}

bool EffectRecord::LoadEffect(const char *Filename, UINT32 refID, bool Private, D3DXMACRO *defs) {
  Kill();

  if (strlen(Filename) > 240)
    return false;
  if (!defs)
    defs = (D3DXMACRO *)::defs;

  struct stat sb, sx;
  char strFileFull[MAX_PATH], *ext;
  FILE *f;

  strcpy(Name, Filename);

  /* getting a compiled binary for replacement if there is any
   */
  strcpy(Filepath, ::EffectDirectory.Get());
  strcat(Filepath, Filename);
  if ((ext = strstr(Filepath, ".fx")))
    *ext = '\0';
  if (!stat((const char *)Filepath, &sb)) {
    UINT size = sb.st_size;
    if (D3DXCreateBuffer(size, &pBinary) == D3D_OK) {
      if (!fopen_s(&f, Filepath, "rb")) {
	fread(pBinary->GetBufferPointer(), 1, size, f);
	fclose(f);

        _DMESSAGE("Loaded binary of %s from %s", Name, Filepath);
      }
      else {
	pBinary->Release();
	pBinary = NULL;
      }
    }
  }
  if (ext)
    *ext = '.';

  /* getting a FX source for compiling if there is any
   */
  strcpy(strFileFull, ::EffectDirectory.Get());
  strcat(strFileFull, Filename);
  if (!stat((const char *)strFileFull, &sx)) {
    UINT size = sx.st_size;
    pSource = new CHAR[size + 1];
    if (pSource != NULL) {
      /* reading in text-mode can yield any number of less characters */
      memset(pSource, 0, size + 1);

      if (!fopen_s(&f, strFileFull, "rb"/*"rt"*/)) {
        fread(pSource, 1, size, f);
        fclose(f);

        _DMESSAGE("Loaded source of %s from %s", Name, Filepath);

	sourceLen = strlen(pSource);
      }
      else {
      	delete[] pSource;

      	pSource = NULL;
        sourceLen = 0;
      }
    }

    /* if the HLSL source is newer than the binary, attempt to recompile
     */
    if (pSource && (sx.st_mtime > sb.st_mtime)) {
      if (pBinary) pBinary->Release();
      if (pEffect) pEffect->Release();

      pBinary = NULL;
      pEffect = NULL;
    }
  }

  this->pDefine = defs;
  this->Enabled = false;
  this->Private = Private;
  this->ParentRefID = refID;

  return (pSource != NULL);
}

bool EffectRecord::RuntimeFlush() {
  /* release all runtime resources (for example for recompilation) */
  if (pBinary) pBinary->Release();
  if (pEffect) pEffect->Release();
  if (pErrorMsgs) pErrorMsgs->Release();
  if (pDisasmbly) pDisasmbly->Release();

  pBinary = NULL;
  pEffect = NULL;
  pErrorMsgs = NULL;
  pDisasmbly = NULL;

  return true;
}

bool EffectRecord::RuntimeEffect(const char *fx) {
  int size = strlen(fx);

  /* put a new source */
  if (pSource) delete[] pSource;
  pSource = new CHAR[size + 1];
  strcpy(pSource, fx);
  pSource[size] = '\0';
  sourceLen = size;

  /* trigger recompile */
  return RuntimeFlush();
}

bool EffectRecord::SaveEffect() {
  LPD3DXBUFFER p = NULL;

  /* cascade, the highest possible is selected */
  if (pBinary)
    p = pBinary;

  if (p) {
    FILE *f;

    if (!fopen_s(&f, Filepath, "wb")) {
      fwrite(p->GetBufferPointer(), 1, p->GetBufferSize(), f);
      fclose(f);

      _DMESSAGE("Saved binary of %s to %s", Name, Filepath);

      return true;
    }
  }

  return false;
}

bool EffectRecord::CompileEffect(EffectManager *FXMan, bool forced) {
  /* nobody wants the automatic recompile */
  if (0/*!::CompileSources.Get()*/ && !forced)
    return false;

  LPSTR src = NULL; int len;
  LPD3DXBUFFER p = NULL;
  ID3DXEffect *x = NULL;
  bool save = false;

  /* cascade, the highest possible is selected */
  if (pSource) {
    src = pSource;
    len = sourceLen;

    p = pBinary;
    x = pEffect;
  }

  /* recompile only, if there is one already, just ignore */
  if (!x && src) {
    if (pDisasmbly)
    pDisasmbly->Release();
    pDisasmbly = NULL;

    if (pErrorMsgs)
    pErrorMsgs->Release();
    pErrorMsgs = NULL;

    D3DXCreateEffect(
      slimOBGEDirect3DDevice9,
      src,
      len,
      pDefine,
      &incl,
      D3DXSHADER_DEBUG | (
      ::UseLegacyCompiler.Get() ? D3DXSHADER_USE_LEGACY_D3DX9_31_DLL : (
      ::Optimize.Get()          ? D3DXSHADER_OPTIMIZATION_LEVEL3 : 0)),
      FXMan ? FXMan->EffectPool : NULL,
      &x,
      &pErrorMsgs
      );

    /* this didn't go so well, if it's a legacy "error", just try again */
    if (pErrorMsgs && strstr((char*)pErrorMsgs->GetBufferPointer(), "X3539")) {
      pErrorMsgs->Release();
      pErrorMsgs = NULL;

      D3DXCreateEffect(
	slimOBGEDirect3DDevice9,
	src,
	len,
	pDefine,
	&incl,
	D3DXSHADER_DEBUG | (
	D3DXSHADER_USE_LEGACY_D3DX9_31_DLL),
	FXMan ? FXMan->EffectPool : NULL,
	&x,
	&pErrorMsgs
	);
    }

    /* this didn't go so well */
    if (pErrorMsgs) {
      _MESSAGE("Shader compiling messages occured in %s:", Filepath);
      _MESSAGE((char *)pErrorMsgs->GetBufferPointer());

      save = !strstr((char *)pErrorMsgs->GetBufferPointer(), "error");
    }
    else
      save = true;
  }

  /* cascade, the highest possible is selected */
  if (pSource) {
    pBinary = p;
    pEffect = x;
  }

  /* auto-save or not */
  if (save)
    SaveEffect();

  if (x) {
    if (!pDisasmbly)
      D3DXDisassembleEffect(
        x,
        FALSE,
        &pDisasmbly
      );

    ApplyCompileDirectives();
  }

  return (pSource && (pEffect != NULL));
}

void EffectRecord::ApplyCompileDirectives() {
  if (!HasEffect()) return;
  LPCSTR pName = NULL; pEffect->GetString("Name", &pName);
  if (pName)
    strcpy(Name, (char *)pName);

  /* obtain a copy of the old resources */
  TextureManager *TexMan = TextureManager::GetSingleton();
  std::vector<int> prevTextures = Textures; Textures.clear();
  D3DXEFFECT_DESC Description;
  pEffect->GetDesc(&Description);
  Parameters = 0;

  /* extract parameter informations */
  for (int par = 0; par < Description.Parameters; par++) {
    D3DXHANDLE handle;

    if ((handle = pEffect->GetParameter(NULL, par))) {
      D3DXPARAMETER_DESC Description;
      pEffect->GetParameterDesc(handle, &Description);

      if (Description.Type == D3DXPT_TEXTURECUBE) {
	D3DXHANDLE handle2;

	if ((handle2 = pEffect->GetAnnotationByName(handle, "filename"))) {
	  LPCSTR pString = NULL; pEffect->GetString(handle2, &pString);

	  _MESSAGE("Found filename : %s", pString);

	  int TexNum = TexMan->LoadDependtTexture((char *)pString, TR_CUBIC);
	  if (TexNum != -1) {
	    pEffect->SetTexture(handle, TexMan->GetTexture(TexNum)->GetTexture());
	    Textures.push_back(TexNum);
	  }
	}
      }
      else if (Description.Type == D3DXPT_TEXTURE3D) {
	D3DXHANDLE handle2;

	if ((handle2 = pEffect->GetAnnotationByName(handle, "filename"))) {
	  LPCSTR pString = NULL; pEffect->GetString(handle2, &pString);

	  _MESSAGE("Found filename : %s", pString);

	  int TexNum = TexMan->LoadDependtTexture((char *)pString, TR_VOLUMETRIC);
	  if (TexNum != -1) {
	    pEffect->SetTexture(handle, TexMan->GetTexture(TexNum)->GetTexture());
	    Textures.push_back(TexNum);
	  }
	}
      }
      else if ((Description.Type == D3DXPT_TEXTURE) ||
	       (Description.Type == D3DXPT_TEXTURE1D) ||
	       (Description.Type == D3DXPT_TEXTURE2D)) {
	  D3DXHANDLE handle2;

	  if ((handle2 = pEffect->GetAnnotationByName(handle, "filename"))) {
	    LPCSTR pString = NULL; pEffect->GetString(handle2, &pString);

	    _MESSAGE("Found filename : %s", pString);

	    int TexNum = TexMan->LoadDependtTexture((char *)pString, TR_PLANAR);
	    if (TexNum != -1) {
	      pEffect->SetTexture(handle, TexMan->GetTexture(TexNum)->GetTexture());
	      Textures.push_back(TexNum);
	    }
	  }
	  else {
	    /**/ if (Description.Name == strstr(Description.Name, "obge_PrevRendertarget"))
	      Parameters |= EFFECTBUF_PREV;
	    else if (Description.Name == strstr(Description.Name, "obge_PastRendertarget"))
	      Parameters |= EFFECTBUF_PAST;
	    else if (Description.Name == strstr(Description.Name, "oblv_CurrDepthStencilZ"))
	      Parameters |= EFFECTBUF_ZBUF;
	    else if (Description.Name == strstr(Description.Name, "oblv_CurrDepthStencilW"))
	      Parameters |= EFFECTBUF_ZBUF;
	    else if (Description.Name == strstr(Description.Name, "oblv_CurrLinearDepthZ"))
	      Parameters |= EFFECTBUF_ZBUF | EFFECTBUF_LBUF;
	    else if (Description.Name == strstr(Description.Name, "oblv_CurrEyeProjectedZXYD_EFFECTPASS"))
	      Parameters |= EFFECTBUF_ZBUF | EFFECTBUF_LBUF | EFFECTBUF_EPOS;
	    else if (Description.Name == strstr(Description.Name, "oblv_CurrEyeProjectedNormals_EFFECTPASS"))
	      Parameters |= EFFECTBUF_ZBUF | EFFECTBUF_LBUF | EFFECTBUF_EPOS | EFFECTBUF_ENRM;
	    else if (Description.Name == strstr(Description.Name, "oblv_CurrWorldProjectedZXYL_EFFECTPASS"))
	      Parameters |= EFFECTBUF_ZBUF | EFFECTBUF_LBUF | EFFECTBUF_WPOS;
	    else if (Description.Name == strstr(Description.Name, "oblv_CurrWorldProjectedNormals_EFFECTPASS"))
	      Parameters |= EFFECTBUF_ZBUF | EFFECTBUF_LBUF | EFFECTBUF_WPOS | EFFECTBUF_WNRM;
	  }
      }
    }
  }

  /* clear */
  Priority = (EFFECTGROUP_MAIN    << 24) |
	     (EFFECTCLASS_NEUTRAL << 17);
  Class = EFFECTCLASS_NEUTRAL;
  Flags = 0;
  memset(FlagsPass, 0, sizeof(FlagsPass));

  /* extract technique informations */
  for (int teq = 0; teq < Description.Techniques; teq++) {
    D3DXHANDLE handle;

    if ((handle = pEffect->GetTechnique(teq))) {
      D3DXTECHNIQUE_DESC Description;
      pEffect->GetTechniqueDesc(handle, &Description);

      for (int ann = 0; ann < Description.Annotations; ann++) {
	D3DXHANDLE handle2;

	if ((handle2 = pEffect->GetAnnotation(handle, ann))) {
	  D3DXPARAMETER_DESC Description;
	  pEffect->GetParameterDesc(handle2, &Description);

	  // int group = ...;
	  // int fxclass = ...;
	  // int rendertarget0-3 = ...;
	  // int conditions = ...;
	  // string dependency = ".fx";
	  // int dependency = ".fx";
	  if ((Description.Name == strstr(Description.Name, "group"))) {
	    if (Description.Type == D3DXPT_INT) {
	      INT pri; pEffect->GetInt(handle2, &pri);
	      this->Priority = (pri << 24) | (this->Priority & ~(0xFF << 24));
	      /* priority-field: <8bit group><7bit class><1bit order><16bit prio> */
	    }
	  }
	  else if ((Description.Name == strstr(Description.Name, "fxclass"))) {
	    if (Description.Type == D3DXPT_INT) {
	      INT cls; pEffect->GetInt(handle2, &cls);
	      this->Priority = (cls << 17) | (this->Priority & ~(0x7F << 17));
	      /* priority-field: <8bit group><7bit class><1bit order><16bit prio> */
	      this->Class = cls;
	    }
	  }
	  else if ((Description.Name == strstr(Description.Name, "rendertarget"))) {
	    if (Description.Type == D3DXPT_INT) {
	    }
	  }
	  else if ((Description.Name == strstr(Description.Name, "conditions"))) {
	    if (Description.Type == D3DXPT_INT) {
	      pEffect->GetInt(handle2, &this->Flags);
	    }
	  }
	  else if ((Description.Name == strstr(Description.Name, "dependency"))) {
	    if (Description.Type == D3DXPT_INT) {
	      INT dep; pEffect->GetInt(handle2, &dep);
	      INT prv = (this->Priority >> 16) & 0xFF;
	      if (dep < prv)
		dep = prv;
	      this->Priority = (dep << 17) | (1 << 16) | (this->Priority & ~(0xFF << 16));
	      /* priority-field: <8bit group><7bit class><1bit order><16bit prio> */
	    }
	    else if (Description.Type == D3DXPT_STRING) {
	      LPCSTR pString = NULL; pEffect->GetString(handle2, &pString);

	      EffectManager::GetSingleton()->AddDependtEffect(pString, 0);
	    }
	  }
	}
      }

      for (int pas = 0; (pas < Description.Passes) && (pas < 16); pas++) {
	D3DXHANDLE handle2;

	if ((handle2 = pEffect->GetPass(handle, pas))) {
	  D3DXPASS_DESC Description;
	  pEffect->GetPassDesc(handle2, &Description);

	  for (int ann = 0; ann < Description.Annotations; ann++) {
	    D3DXHANDLE handle3;

	    if ((handle3 = pEffect->GetAnnotation(handle, ann))) {
	      D3DXPARAMETER_DESC Description;
	      pEffect->GetParameterDesc(handle3, &Description);

	      // int conditions = ...;
	      if ((Description.Name == strstr(Description.Name, "conditions"))) {
		if (Description.Type == D3DXPT_INT) {
		  pEffect->GetInt(handle2, &this->FlagsPass[pas]);
		}
	      }
	    }
	  }
	}
      }
    }
  }

  /* release previous texture */
  std::vector<int>::iterator PTexture = prevTextures.begin();

  while (PTexture != prevTextures.end()) {
    TexMan->ReleaseTexture(*PTexture);
    PTexture++;
  }

  /* reflect changes in the applicable conditions */
  EffectManager::GetSingleton()->Recalculate();
}

inline void EffectRecord::ApplyPermanents(EffectManager *FXMan) {
#ifdef	OLD_QUEUE
  pEffect->SetTexture("obge_PrevRendertarget0_EFFECTPASS", FXMan->thisframeTex);
  pEffect->SetTexture("obge_LastRendertarget0_EFFECTPASS", FXMan->lastpassTex);
  pEffect->SetTexture("obge_PastRendertarget0_MAINPASS"  , FXMan->lastframeTex);
  pEffect->SetTexture("oblv_CurrDepthStencilZ_MAINPASS"  , FXMan->depth);
#else
//pEffect->SetTexture("obge_PrevRendertarget0_EFFECTPASS", FXMan->thisframeTex);
//pEffect->SetTexture("obge_LastRendertarget0_EFFECTPASS", FXMan->lastpassTex);
//pEffect->SetTexture("obge_PastRendertarget0_MAINPASS"  , FXMan->lastframeTex);

  /* convert WHATEVER to linearized form (CurrDS) */
  /* convert linearized to unlinearize form (OrigDS) */
  if (FXMan->RenderTransferZ) {
    if (FXMan->RenderTransferZ > EFFECTBUF_RAWZ) {
      FXMan->CurrNM.SetTexture("oblv_CurrWorldProjectedNormals_EFFECTPASS", pEffect);
      FXMan->CurrDS.SetTexture("oblv_CurrWorldProjectedZXYL_EFFECTPASS", pEffect);

      FXMan->CurrNM.SetTexture("oblv_CurrEyeProjectedNormals_EFFECTPASS", pEffect);
      FXMan->CurrDS.SetTexture("oblv_CurrEyeProjectedZXYD_EFFECTPASS", pEffect);

      FXMan->CurrDS.SetTexture("oblv_CurrLinearDepthZ_EFFECTPASS", pEffect);
      FXMan->OrigDS.SetTexture("oblv_CurrDepthStencilZ_MAINPASS", pEffect);
    }
    else
      /* convert linearized to unlinearize form (OrigDS) */
      FXMan->CurrDS.SetTexture("oblv_CurrDepthStencilZ_MAINPASS", pEffect);
  }
  else
    /* convert linearized to unlinearize form (OrigDS) */
    FXMan->OrigDS.SetTexture("oblv_CurrDepthStencilZ_MAINPASS", pEffect);
#endif

  pEffect->SetVector("oblv_ReciprocalResolution_MAINPASS", &Constants.rcpres);

  /* deprecated */
#ifndef	NO_DEPRECATED
  pEffect->SetBool("bHasDepth", Constants.bHasDepth);
  pEffect->SetFloatArray("rcpres", (float *)&Constants.rcpres, 2);
#endif
}

inline void EffectRecord::ApplySharedConstants() {
  pEffect->SetMatrix("oblv_WorldTransform_MAINPASS", &Constants.wrld);
//pEffect->SetMatrix("oblv_WorldInverse_MAINPASS", &Constants.wrld_inv);
  pEffect->SetMatrix("oblv_ViewTransform_MAINPASS", &Constants.view);
  pEffect->SetMatrix("oblv_ViewInverse_MAINPASS", &Constants.view_inv);
  pEffect->SetMatrix("oblv_ProjectionTransform_MAINPASS", &Constants.proj);
  pEffect->SetMatrix("oblv_ProjectionInverse_MAINPASS", &Constants.proj_inv);

  pEffect->SetMatrix("oblv_PastViewProjectionTransform_MAINPASS", &Constants.pastviewproj);
  pEffect->SetMatrix("oblv_ViewProjectionTransform_MAINPASS", &Constants.viewproj);
  pEffect->SetMatrix("oblv_ViewProjectionInverse_MAINPASS", &Constants.viewproj_inv);

  pEffect->SetMatrix("oblv_PastWorldViewProjectionTransform_MAINPASS", &Constants.pastwrldviewproj);
  pEffect->SetMatrix("oblv_WorldViewProjectionTransform_MAINPASS", &Constants.wrldviewproj);
  pEffect->SetMatrix("oblv_WorldViewProjectionInverse_MAINPASS", &Constants.wrldviewproj_inv);

  pEffect->SetFloatArray("oblv_CameraForward_MAINPASS", &Constants.EyeForward.x, 3);
  pEffect->SetMatrix("oblv_CameraFrustum_MAINPASS", &Constants.EyeFrustum);
  pEffect->SetVector("oblv_CameraPosition_MAINPASS", &Constants.EyePosition);

  pEffect->SetVector("oblv_ProjectionDepthRange_MAINPASS", &Constants.ZRange);
  pEffect->SetVector("oblv_ProjectionFoV_MAINPASS", &Constants.FoV);

  pEffect->SetVector("oblv_FogRange", &Constants.FogRange);
  pEffect->SetVector("oblv_SunDirection", &Constants.SunDir);
  pEffect->SetVector("oblv_SunTiming", &Constants.SunTiming);

  pEffect->SetIntArray("oblv_GameTime", &Constants.iGameTime.x, 4);
  pEffect->SetIntArray("obge_Tick", &Constants.iTikTiming.x, 4);

#ifndef	NO_DEPRECATED
  pEffect->SetVector("f4Time", &Constants.time);
#endif

#ifndef	OBGE_NOSHADER
  pEffect->SetTexture("oblv_Rendertarget0_REFLECTIONPASS", passTexture[OBGEPASS_REFLECTION]);

  /* deprecated */
#ifndef	NO_DEPRECATED
  pEffect->SetBool("bHasReflection", !!passTexture[OBGEPASS_REFLECTION]);
#endif
#endif
}

inline void EffectRecord::ApplyUniqueConstants() {
}

inline void EffectRecord::OnLostDevice(void) {
  if (HasEffect())
    pEffect->OnLostDevice();
}

inline void EffectRecord::OnResetDevice(void) {
  if (HasEffect())
    pEffect->OnResetDevice();
}

inline void EffectRecord::Render(IDirect3DDevice9 *D3DDevice, IDirect3DSurface9 *RenderTo, IDirect3DSurface9 *RenderCopy) {
  if (!IsEnabled())
    return;

  markerStart(D3DDevice);

  // Have to do this in case the effect has no vertex effect. The stupid effect system
  // uses the last vertex effect that was active and much strangeness occurs.
  D3DDevice->SetVertexShader(NULL);

#ifdef	OBGE_STATEBLOCKS
  UINT pass = 0;
  UINT passes; pEffect->Begin(&passes, D3DXFX_DONOTSAVESTATE);
#else
  UINT pass = 0;
  UINT passes; pEffect->Begin(&passes, 0);
#endif

  while (true) {
    pEffect->BeginPass(pass);
    D3DDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
    pEffect->EndPass();

    if (++pass >= passes)
      break;

    minStretchRect(D3DDevice, RenderTo, 0, RenderCopy, 0, D3DTEXF_NONE);
  }

  pEffect->End();

  markerStop(D3DDevice);
}

inline bool EffectRecord::Render(IDirect3DDevice9 *D3DDevice, EffectQueue *Queue) {
  if (!IsEnabled())
    return false;

  markerStart(D3DDevice);

  // Have to do this in case the effect has no vertex effect. The stupid effect system
  // uses the last vertex effect that was active and much strangeness occurs.
  D3DDevice->SetVertexShader(NULL);

#ifndef OBGE_CONSTANTPOOLS
  ApplySharedConstants();
#endif
  ApplyUniqueConstants();

#ifdef	OBGE_STATEBLOCKS
  UINT pass = 0; Queue->Begin(pEffect);
  UINT passes; pEffect->Begin(&passes, D3DXFX_DONOTSAVESTATE);
#else
  UINT pass = 0; Queue->Begin(pEffect);
  UINT passes; pEffect->Begin(&passes, 0);
#endif

  while (true) {
    pEffect->BeginPass(pass);
    D3DDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
    pEffect->EndPass();

    if (++pass >= passes)
      break;

    Queue->Step(pEffect);
  }

  pEffect->End();

#ifdef	OBGE_DEVLING
  passScene = this->GetName();
#endif

  markerStop(D3DDevice);

  Queue->End(pEffect);

  return true;
}

inline void EffectRecord::Render(IDirect3DDevice9 *D3DDevice) {
  if (!IsEnabled())
    return;

  markerStart(D3DDevice);

  // Have to do this in case the effect has no vertex effect. The stupid effect system
  // uses the last vertex effect that was active and much strangeness occurs.
  D3DDevice->SetVertexShader(NULL);

#ifndef OBGE_CONSTANTPOOLS
  ApplySharedConstants();
#endif
  ApplyUniqueConstants();

#ifdef	OBGE_STATEBLOCKS
  UINT pass = 0;
  UINT passes; pEffect->Begin(&passes, D3DXFX_DONOTSAVESTATE);
#else
  UINT pass = 0;
  UINT passes; pEffect->Begin(&passes, 0);
#endif

  while (true) {
    pEffect->BeginPass(pass);
    D3DDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
    pEffect->EndPass();

    if (++pass >= passes)
      break;
  }

  pEffect->End();

  markerStop(D3DDevice);
}

ID3DXEffect *EffectRecord::GetEffect() const {
  return this->pEffect;
}

bool EffectRecord::HasEffect() const {
  return (this->pEffect != NULL);
}

void EffectRecord::Enable(bool Enabled) {
  /* do nothing */
  if (this->Enabled == Enabled)
    return;

  this->Enabled = Enabled;

  /* reflect changes in the applicable conditions */
  EffectManager::GetSingleton()->Recalculate();
}

bool EffectRecord::IsEnabled() const {
  return this->Enabled && HasEffect();
}

inline UINT32 EffectRecord::GetRefID() const {
  return this->ParentRefID;
}

inline const char *EffectRecord::GetPath() const {
  return this->Filepath;
}

inline const char *EffectRecord::GetName() const {
  return this->Name;
}

inline bool EffectRecord::IsPrivate() const {
  return this->Private;
}

inline unsigned long EffectRecord::GetParameters() const {
  return this->Parameters;
}

inline unsigned long EffectRecord::GetConditions() const {
  return this->Flags;
}

inline unsigned long EffectRecord::GetConditions(int pass) const {
  return this->FlagsPass[pass];
}

inline void EffectRecord::SetPriority(int pri) {
  this->Priority = (this->Priority & (0xFF << 24)) | (pri & ~(0xFF << 24));
}

bool EffectRecord::SetEffectConstantB(const char *name, bool value) {
  HRESULT hr = (HasEffect() ? pEffect->SetBool(name, value) : -1);
  return (hr == D3D_OK);
}

bool EffectRecord::SetEffectConstantI(const char *name, int value) {
  HRESULT hr = (HasEffect() ? pEffect->SetInt(name, value) : -1);
  return (hr == D3D_OK);
}

bool EffectRecord::SetEffectConstantI(const char *name, int *values, int num) {
  HRESULT hr = (HasEffect() ? pEffect->SetIntArray(name, values, num) : -1);
  return (hr == D3D_OK);
}

bool EffectRecord::SetEffectConstantF(const char *name, float value) {
  HRESULT hr = (HasEffect() ? pEffect->SetFloat(name, value) : -1);
  return (hr == D3D_OK);
}

bool EffectRecord::SetEffectConstantF(const char *name, float *values, int num) {
  HRESULT hr = (HasEffect() ? pEffect->SetFloatArray(name, values, num) : -1);
  return (hr == D3D_OK);
}

bool EffectRecord::SetEffectConstantV(const char *name, v1_2_416::NiVector4 *value) {
  HRESULT hr = (HasEffect() ? pEffect->SetVector(name, value) : -1);
  return (hr == D3D_OK);
}

bool EffectRecord::SetEffectSamplerTexture(const char *name, int TextureNum) {
  TextureManager *TexMan = TextureManager::GetSingleton();
  if (!HasEffect()) return false;

  IDirect3DBaseTexture9 *OldTexture = NULL;
  /* get old one */
  pEffect->GetTexture(name, (LPDIRECT3DBASETEXTURE9 *)&OldTexture);
  /* and dereference */
  pEffect->SetTexture(name, NULL);

  /* remove any trace of the texture from this effect */
  if (OldTexture) {
    int OldTextureNum = TexMan->FindTexture(OldTexture);
    TexMan->ReleaseTexture(OldTexture);

    /* remove from vector */
    if (OldTextureNum != -1)
      Textures.erase(std::find(Textures.begin(), Textures.end(), OldTextureNum));
  }

  /* apply the new texture and remember it */
  TextureRecord *NewTexture = TexMan->GetTexture(TextureNum);
  if (NewTexture) {
    HRESULT hr = pEffect->SetTexture(name, NewTexture->GetTexture());

    /* add to vector */
    if (hr == D3D_OK)
      Textures.push_back(TextureNum);

    return (hr == D3D_OK);
  }

  return false;
}

void EffectRecord::PurgeTexture(IDirect3DBaseTexture9 *texture, int TexNum) {
  if (!HasEffect()) return;
  D3DXEFFECT_DESC Description;
  pEffect->GetDesc(&Description);

  for (int par = 0; par < Description.Parameters; par++) {
    D3DXHANDLE handle;

    if ((handle = pEffect->GetParameter(NULL, par))) {
      D3DXPARAMETER_DESC Description;

      pEffect->GetParameterDesc(handle, &Description);

      if ((Description.Type = D3DXPT_TEXTURE) ||
	  (Description.Type = D3DXPT_TEXTURE1D) ||
	  (Description.Type = D3DXPT_TEXTURE2D) ||
	  (Description.Type = D3DXPT_TEXTURE3D) ||
	  (Description.Type = D3DXPT_TEXTURECUBE)) {
	// NB must set to NULL otherwise strange things happen
	IDirect3DBaseTexture9 *EffectTexture = NULL;
	pEffect->GetTexture(handle, &EffectTexture);

	if (EffectTexture == texture) {
	  pEffect->SetTexture(handle, NULL);

	  _DMESSAGE("Removing texture %s from effect %s", Description.Name, GetPath());

	  /* remove from vector */
	  if (TexNum != -1)
	    Textures.erase(std::find(Textures.begin(), Textures.end(), TexNum));
	}
      }
    }
  }
}

void EffectRecord::SaveVars(OBSESerializationInterface *Interface) {
  if (!HasEffect()) return;
  TextureManager *TexMan = TextureManager::GetSingleton();
  D3DXEFFECT_DESC Description;
  pEffect->GetDesc(&Description);

  _MESSAGE("pEffect %s has %d parameters.", Filepath, Description.Parameters);

  for (int par = 0; par < Description.Parameters; par++) {
    D3DXHANDLE handle;

    if ((handle = pEffect->GetParameter(NULL, par))) {
      D3DXPARAMETER_DESC Description;
      pEffect->GetParameterDesc(handle, &Description);

      switch (Description.Type) {
        case D3DXPT_TEXTURE:
        case D3DXPT_TEXTURE1D:
        case D3DXPT_TEXTURE2D:
        case D3DXPT_TEXTURE3D:
	case D3DXPT_TEXTURECUBE: {
          IDirect3DBaseTexture9 *Texture = NULL;
          int TextureNum; TextureType TextureData;
	  ManagedTextureRecord *MTexture;

	  /* find the DX-resource */
          pEffect->GetTexture(handle, &Texture);
	  /* and the corresponding ID */
          TextureNum = TexMan->FindTexture(Texture);

	  if (TextureNum == -1)
	    break;

	  /* and the corresponding structure */
	  MTexture = TexMan->GetTexture(TextureNum);

	  /* save the texture only if it's not private,
	   * those in the effect-files themself
	   * are private textures for example
	   */
	  if (!MTexture->IsPrivate()) {
	    TextureData.tex = TextureNum;
	    strcpy(TextureData.Name, Description.Name);
	    Interface->WriteRecord('STEX', SHADERVERSION, &TextureData, sizeof(TextureData));

	    _MESSAGE("Found texture: name - %s, texnum - %d", TextureData.Name, TextureNum);
	  }

	} break;
        case D3DXPT_INT: {
          IntType IntData;

          IntData.size = Description.Elements;
          if (IntData.size == 0)
            IntData.size = 1;

          pEffect->GetIntArray(handle, (int *)&IntData.data, IntData.size);
          strcpy(IntData.Name, Description.Name);
          Interface->WriteRecord('SINT', SHADERVERSION, &IntData, sizeof(IntData));

          _MESSAGE("Found int: name - %s, size - %d, data[0] - %d", IntData.Name, IntData.size, IntData.data[0]);

        } break;
        case D3DXPT_FLOAT: {
          FloatType FloatData;

          FloatData.size = Description.Elements;
          if (FloatData.size == 0)
            FloatData.size = 1;

          pEffect->GetFloatArray(handle, (float *)&FloatData.data, FloatData.size);
          strcpy(FloatData.Name, Description.Name);
          Interface->WriteRecord('SFLT', SHADERVERSION, &FloatData, sizeof(FloatData));

          _MESSAGE("Found float: name - %s, size - %d, data[0] - %f", FloatData.Name, FloatData.size, FloatData.data[0]);

        } break;
      }
    }
  }
}

// *********************************************************************************************************

ManagedEffectRecord::ManagedEffectRecord() {
  RefCount = 0;
}

ManagedEffectRecord::~ManagedEffectRecord() {
}

void ManagedEffectRecord::ClrRef() {
  RefCount = 0;
}

int ManagedEffectRecord::AddRef() {
  return ++RefCount;
}

int ManagedEffectRecord::Release() {
  if (RefCount)
    RefCount--;

  if (!RefCount) {
//  delete this;
    return 0;
  }

  return RefCount;
}

// *********************************************************************************************************

EffectManager *EffectManager::Singleton = NULL;

EffectManager::EffectManager() {
  LARGE_INTEGER freq;

  QueryPerformanceFrequency(&freq);

  Constants.iTikTiming.w = (int)(freq.QuadPart);

  EffectIndex = 0;
  MaxEffectIndex = 0;

  EffectPool = NULL;
  EffectVertex = NULL;
  EffectDepth = NULL;
  EffectShare = NULL;

#ifdef	OLD_QUEUE
  // these are all private
  thisframeTex = NULL;
  lastpassTex = NULL;
  lastframeTex = NULL;
  thisframeSurf = NULL;
  lastpassSurf = NULL;
  lastframeSurf = NULL;

  HasDepth = false;

  depth = NULL;
  depthSurface = NULL;
  depthRAWZ = NULL;

  RAWZflag = false;
#else
  RenderTransferZ = (IsRAWZ() ? EFFECTBUF_RAWZ : 0);
  RenderBuf = 0;
  RenderCnd = 0;
  RenderFmt = D3DFMT_UNKNOWN;
#endif

  if (FreezeTweaks.Get())
    defs[DEFS_iface].Definition = TWEAK_FROZEN;
  else
    defs[DEFS_iface].Definition = TWEAK_DYNAMIC;
}

EffectManager::~EffectManager() {
  Singleton = NULL;

  if (EffectPool)
    while (EffectPool->Release()) {};
  if (EffectVertex)
    while (EffectVertex->Release()) {};

#ifdef	OLD_QUEUE
  if (thisframeSurf)
    thisframeSurf->Release();
  if (lastframeSurf)
    lastframeSurf->Release();
  if (lastpassSurf)
    lastpassSurf->Release();

  if (thisframeTex)
    while (thisframeTex->Release()) {};
  if (lastframeTex)
    while (lastframeTex->Release()) {};
  if (lastpassTex)
    while (lastpassTex->Release()) {};

  if (IsRAWZ() && depth)
    while (depth->Release()) {};
#else
  if (RenderBuf & EFFECTBUF_COPY) CopyRT.Release();
  if (RenderBuf & EFFECTBUF_PAST) PastRT.Release();
  if (RenderBuf & EFFECTBUF_PREV) PrevRT.Release();
                                  LastRT.Release();

  if (RenderBuf & EFFECTBUF_ZBUF) if (RenderTransferZ)
                                  CurrDS.Release(),
                                  CurrNM.Release();
#endif
}

EffectManager *EffectManager::GetSingleton() {
  if (!EffectManager::Singleton)
    EffectManager::Singleton = new EffectManager();

  return EffectManager::Singleton;
}

void EffectManager::Reset() {
  ManagedEffects.clear();
  Effects.clear();

  EffectIndex = 0;
  MaxEffectIndex = 0;

#ifdef	OLD_QUEUE
#else
  RenderBuf = 0;
  RenderCnd = 0;
#endif
}

bool EffectManager::SetTransferZ(long MaskZ) {
  /* mask out Z-relevant bits */
  MaskZ |= (IsRAWZ() ? EFFECTBUF_RAWZ : 0);
  MaskZ &= EFFECTBUF_TRANSFERZMASK;

  /* nodo */
  if (RenderTransferZ == MaskZ)
    return true;

  /* redo */
  if ((RenderTransferZ & (EFFECTBUF_LBUF | EFFECTBUF_EPOS | EFFECTBUF_WPOS)) !=
      (MaskZ           & (EFFECTBUF_LBUF | EFFECTBUF_EPOS | EFFECTBUF_WPOS)))
    CurrDS.Release();

  if ((RenderTransferZ & (EFFECTBUF_LBUF | EFFECTBUF_ENRM | EFFECTBUF_WNRM)) !=
      (MaskZ           & (EFFECTBUF_LBUF | EFFECTBUF_ENRM | EFFECTBUF_WNRM)))
    CurrNM.Release();

//if (RenderTransferZ & (EFFECTBUF_RAWZ) !=
//    MaskZ           & (EFFECTBUF_RAWZ))
//  delete EffectDepth;

  /* put all indicators into the effects (optimize) */
  defs[DEFS_INRAWZ].Definition[0] = (MaskZ &           EFFECTBUF_RAWZ          ? '1' : '0');
  defs[DEFS_INLINZ].Definition[0] = (MaskZ &           EFFECTBUF_LBUF          ? '1' : '0');
  defs[DEFS_INPRJZ].Definition[0] = (MaskZ & (EFFECTBUF_EPOS | EFFECTBUF_WPOS) ? '1' : '0');
  defs[DEFS_INNRMZ].Definition[0] = (MaskZ & (EFFECTBUF_ENRM | EFFECTBUF_WNRM) ? '1' : '0');

  if (MaskZ && !EffectDepth) {
    EffectDepth = new EffectRecord();

    if (!EffectDepth->LoadEffect("TransferZ.fx", 0) ||
	!EffectDepth->CompileEffect(this)) {
      delete EffectDepth;
      EffectDepth = NULL;

      _MESSAGE("ERROR - TransferZ.fx is missing and required! Please reinstall OBGE");
      exit(0); return false;
    }

    EffectDepth->Enable(true);
  }
  else if (!MaskZ && EffectDepth) {
    delete EffectDepth;
    EffectDepth = NULL;
  }

  RenderTransferZ = MaskZ;
  return true;
}

void EffectManager::InitialiseFrameTextures() {
#ifdef	OLD_QUEUE
  HRESULT hr;

  if ((BufferTexturesNumBits.data > 32) ||
      ((BufferTexturesNumBits.data % 8) > 0))
    BufferTexturesNumBits.data = 8;

  UInt32 Width = v1_2_416::GetRenderer()->SizeWidth;
  UInt32 Height = v1_2_416::GetRenderer()->SizeHeight;

  _MESSAGE("Creating full screen textures.");
  _MESSAGE("Width = %d, Height = %d", Width, Height);

  if (BufferTexturesNumBits.data == 32) {
    hr = GetD3DDevice()->CreateTexture(Width, Height, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A32B32G32R32F, D3DPOOL_DEFAULT, &thisframeTex, 0);

    if (FAILED(hr)) {
      thisframeTex->Release();
      thisframeTex = NULL;

      BufferTexturesNumBits.data = 16;
    }
    else {
      GetD3DDevice()->CreateTexture(Width, Height, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A32B32G32R32F, D3DPOOL_DEFAULT, &lastpassTex, 0);
      GetD3DDevice()->CreateTexture(Width, Height, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A32B32G32R32F, D3DPOOL_DEFAULT, &lastframeTex, 0);
    }
  }

  if (BufferTexturesNumBits.data == 16) {
    hr = GetD3DDevice()->CreateTexture(Width, Height, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A16B16G16R16F, D3DPOOL_DEFAULT, &thisframeTex, 0);

    if (FAILED(hr)) {
      thisframeTex->Release();
      thisframeTex = NULL;

      BufferTexturesNumBits.data = 8;
    }
    else {
      GetD3DDevice()->CreateTexture(Width, Height, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A16B16G16R16F, D3DPOOL_DEFAULT, &lastpassTex, 0);
      GetD3DDevice()->CreateTexture(Width, Height, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A16B16G16R16F, D3DPOOL_DEFAULT, &lastframeTex, 0);
    }
  }

  if (BufferTexturesNumBits.data == 8) {
    GetD3DDevice()->CreateTexture(Width, Height, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &thisframeTex, 0);
    GetD3DDevice()->CreateTexture(Width, Height, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &lastpassTex, 0);
    GetD3DDevice()->CreateTexture(Width, Height, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &lastframeTex, 0);
  }

  _MESSAGE("Setting full screen surfaces.");

  thisframeTex->GetSurfaceLevel(0, &thisframeSurf);
  lastpassTex->GetSurfaceLevel(0, &lastpassSurf);
  lastframeTex->GetSurfaceLevel(0, &lastframeSurf);

  _MESSAGE("Setting depth texture.");

  if (IsRAWZ()) {
    GetD3DDevice()->CreateTexture(v1_2_416::GetRenderer()->SizeWidth, v1_2_416::GetRenderer()->SizeHeight, 1, D3DUSAGE_RENDERTARGET, D3DFMT_R32F, D3DPOOL_DEFAULT, &depth, 0);

    depthRAWZ = GetDepthBufferTexture();
    depth->GetSurfaceLevel(0, &depthSurface);

    RAWZflag = true;
    _MESSAGE("RAWZ depth texture - fix applied.");
  }
  else
    depth = GetDepthBufferTexture();
#else
  IDirect3D9 *lastOBGEDirect3D9;
  lastOBGEDirect3DDevice9->GetDirect3D(&lastOBGEDirect3D9);

  int bitz = BufferRawZDepthNumBits.Get();
  int bits = BufferTexturesNumBits.Get();

  if (RenderFmt == D3DFMT_UNKNOWN) {
    D3DFORMAT frmt = D3DFMT_UNKNOWN;
    D3DDISPLAYMODE d3ddm;

    if (lastOBGEDirect3D9->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &d3ddm) == D3D_OK) {
      while (bits != 0) {
	/**/ if (bits >=  32) frmt = (RenderBuf & EFFECTBUF_ACHN ? D3DFMT_A32B32G32R32F : D3DFMT_A32B32G32R32F);
	else if (bits >=  16) frmt = (RenderBuf & EFFECTBUF_ACHN ? D3DFMT_A16B16G16R16F : D3DFMT_A16B16G16R16F);
	else if (bits >=   8) frmt = (RenderBuf & EFFECTBUF_ACHN ? D3DFMT_A8R8G8B8      : D3DFMT_X8R8G8B8);
	else if (bits <= -16) frmt = (RenderBuf & EFFECTBUF_ACHN ? D3DFMT_A16B16G16R16  : D3DFMT_A16B16G16R16);
	else if (bits <= -10) frmt = (RenderBuf & EFFECTBUF_ACHN ? D3DFMT_A2B10G10R10   : D3DFMT_A2B10G10R10);
	else if (bits <=  -8) frmt = (RenderBuf & EFFECTBUF_ACHN ? D3DFMT_A8R8G8B8      : D3DFMT_X8R8G8B8);
	else if (bits <=  -5) frmt = (RenderBuf & EFFECTBUF_ACHN ? D3DFMT_A1R5G5B5      : D3DFMT_R5G6B5);
	else if (bits <=  -4) frmt = (RenderBuf & EFFECTBUF_ACHN ? D3DFMT_A4R4G4B4      : D3DFMT_X4R4G4B4);
	else if (bits <=  -3) frmt = (RenderBuf & EFFECTBUF_ACHN ? D3DFMT_R3G3B2	: D3DFMT_R3G3B2);
	else                  frmt = D3DFMT_UNKNOWN;

	if (frmt != D3DFMT_UNKNOWN) {
	  HRESULT hr;
	  if ((hr = lastOBGEDirect3D9->CheckDeviceFormat(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, d3ddm.Format, D3DUSAGE_RENDERTARGET, D3DRTYPE_SURFACE, frmt)) == D3D_OK)
	    break;
	}

	/**/ if (bits >=  32) bits =  16;
	else if (bits >=  16) bits =   8;
	else if (bits >=   8) bits =   0;
	else if (bits <= -16) bits = -10;
	else if (bits <= -10) bits =  -8;
	else if (bits <=  -8) bits =  -5;
	else if (bits <=  -5) bits =  -4;
	else if (bits <=  -4) bits =  -3;
	else if (bits <=  -3) bits =   0;
      }
    }

    /* TODO extract this out of Oblivion and make the
     * interior/exterior format switch possible
     */
    if (frmt == D3DFMT_UNKNOWN) {
      if (IsHDR())
	frmt = D3DFMT_A16B16G16R16F;
      else
	frmt = D3DFMT_A8R8G8B8;
    }

    if (frmt != D3DFMT_UNKNOWN) {
      _MESSAGE("Creating full screen textures.");
      _MESSAGE("Width = %d, Height = %d", v1_2_416::GetRenderer()->SizeWidth, v1_2_416::GetRenderer()->SizeHeight);
      _MESSAGE("Format = %s", findFormat(frmt));

      RenderFmt = frmt;
    }
  }

  if (RenderFmt != D3DFMT_UNKNOWN) {
    /* TODO extract this out of Oblivion and make the
     * interior/exterior format switch possible
     */
    if (IsHDR())
      RenderBuf |= (RenderFmt != D3DFMT_A16B16G16R16F) ? EFFECTBUF_COPY : 0;
    else
      RenderBuf |= (RenderFmt != D3DFMT_A8R8G8B8     ) ? EFFECTBUF_COPY : 0;

    /* it seems the rendertarget is resolved just fine without the copy */
    if (IsMultiSampled() && 0)
      RenderBuf |=					 EFFECTBUF_COPY    ;

    if (RenderBuf & EFFECTBUF_ZBUF) OrigDS.Initialise(GetDepthBufferTexture());
    if (RenderBuf & EFFECTBUF_COPY) CopyRT.Initialise(RenderFmt);
    if (RenderBuf & EFFECTBUF_PAST) PastRT.Initialise(RenderFmt);
    if (RenderBuf & EFFECTBUF_PREV) PrevRT.Initialise(RenderFmt);
				    LastRT.Initialise(RenderFmt);

    if (RenderBuf & EFFECTBUF_ZBUF) if (RenderTransferZ)
      CurrDS.Initialise(
	RenderTransferZ & EFFECTBUF_EPOS
	  ? (!bitz || (bitz > 16) ? D3DFMT_A32B32G32R32F : D3DFMT_A16B16G16R16F)  // linear
	  :
	RenderTransferZ & EFFECTBUF_LBUF
	  ? (!bitz || (bitz > 16) ? D3DFMT_R32F          : D3DFMT_R16F         )	// linear
	  : (!bitz || (bitz > 16) ? D3DFMT_R32F          : D3DFMT_R16F         )	// non-linear
      );
    if (RenderBuf & EFFECTBUF_ENRM) if (RenderTransferZ)
      CurrNM.Initialise(
//	    (!bitz || (bitz >  8) ? D3DFMT_A16B16G16R16F : D3DFMT_A8B8G8R8     )  // linear
	    (!bitz || (bitz >  8) ? D3DFMT_A16B16G16R16  : D3DFMT_A8B8G8R8     )  // linear
      );
 }
#endif

  RenderCnd = (RenderCnd & ~EFFECTCOND_HASZBUF) | (OrigDS.IsValid() || CurrDS.IsValid() ? EFFECTCOND_HASZBUF : 0);
  RenderCnd = (RenderCnd & ~EFFECTCOND_HASENRM) | (                    CurrNM.IsValid() ? EFFECTCOND_HASENRM : 0);
#ifndef	OBGE_NOSHADER
  RenderCnd = (RenderCnd & ~EFFECTCOND_HASMIPS) | (AMFilter != D3DTEXF_NONE		? EFFECTCOND_HASMIPS : 0);
#endif
}

void EffectManager::ReleaseFrameTextures() {
#ifdef	OLD_QUEUE
  if (thisframeSurf) {
    _MESSAGE("Releasing thisframe surface.");

    thisframeSurf->Release();
    thisframeSurf = NULL;
  }

  if (thisframeTex) {
    _MESSAGE("Releasing thisframe texture.");

    while (thisframeTex->Release()) {}
    thisframeTex = NULL;
  }

  if (lastpassSurf) {
    _MESSAGE("Releasing lastpass surface.");

    lastpassSurf->Release();
    lastpassSurf = NULL;
  }

  if (lastpassTex) {
    _MESSAGE("Releasing lastpass texture.");

    while (lastpassTex->Release()) {}
    lastpassTex = NULL;
  }

  if (lastframeSurf) {
    _MESSAGE("Releasing lastframe surface.");

    lastframeSurf->Release();
    lastframeSurf = NULL;
  }

  if (lastframeTex) {
    _MESSAGE("Releasing lastframe texture.");

    while (lastframeTex->Release()) {}
    lastframeTex = NULL;
  }
#else
  /* over frames */
  if (RenderBuf & EFFECTBUF_PAST) PastRT.Release();
  if (RenderBuf & EFFECTBUF_PREV) PrevRT.Release();
				  LastRT.Release();

  if (RenderBuf & EFFECTBUF_ZBUF) if (RenderTransferZ)
				  CurrDS.Release(),
				  CurrNM.Release();
#endif
}

void EffectManager::InitialiseBuffers() {
  float minx, minu, uadj, vadj;
  void *VertexPointer;

  const float W = (float)v1_2_416::GetRenderer()->SizeWidth;
  const float H = (float)v1_2_416::GetRenderer()->SizeHeight;

  Constants.rcpres[0] = 1.0f / W;
  Constants.rcpres[1] = 1.0f / H;
  Constants.rcpres[2] = W / H;
  Constants.rcpres[3] = W * H;

  uadj = Constants.rcpres[0] * 0.5;
  vadj = Constants.rcpres[1] * 0.5;

  if (SplitScreen.data) {
    minx = 0;
    minu = 0.5;
  }
  else {
    minx = -1;
    minu = 0;
  }

  EffectQuad ShaderVertices[] = {
    {minx, +1, 1, minu + uadj, 0 + vadj, 0},
    {minx, -1, 1, minu + uadj, 1 + vadj, 1},
    {1   , +1, 1, 1    + uadj, 0 + vadj, 2},
    {1   , -1, 1, 1    + uadj, 1 + vadj, 3}
  };

  _MESSAGE("Creating effect vertex buffers.");

  if (lastOBGEDirect3DDevice9->CreateVertexBuffer(4 * sizeof(EffectQuad), D3DUSAGE_WRITEONLY, EFFECTQUADFORMAT, D3DPOOL_DEFAULT, &EffectVertex, 0) != D3D_OK) {
    _MESSAGE("ERROR - Unable to create the vertex buffer!");
    exit(0); return;
  }

  EffectVertex->Lock(0, 0, &VertexPointer, 0);
  CopyMemory(VertexPointer, ShaderVertices, sizeof(ShaderVertices));
  EffectVertex->Unlock();

#ifdef OBGE_CONSTANTPOOLS
  _MESSAGE("Creating effect constants pool.");

  /* utilize effect-pools to set less parameters per frame */
  if (D3DXCreateEffectPool(&EffectPool) != D3D_OK) {
    _MESSAGE("ERROR - Unable to create constants-pool!");
    exit(0); return;
  }

  EffectShare = new EffectRecord();
  if (!EffectShare->LoadEffect("Constants.fx", 0) ||
      !EffectShare->CompileEffect(this)) {
    delete EffectShare;
    EffectShare = NULL;

    _MESSAGE("ERROR - Constants.fx is missing and required! Please reinstall OBGE");
    exit(0); return;
  }
#endif

#ifndef	NO_DEPRECATED
  Constants.bHasDepth = ::HasDepth();
#endif
}

void EffectManager::ReleaseBuffers() {
  if (EffectVertex) {
    _MESSAGE("Releasing effect vertex buffer.");

    while (EffectVertex->Release()) {}
    EffectVertex = NULL;
  }

  if (EffectPool) {
    _MESSAGE("Releasing effect constants pool.");

    while (EffectPool->Release()) {}
    EffectPool = NULL;
  }

  if (EffectShare) {
    delete EffectShare;
    EffectShare = NULL;
  }
}

void EffectManager::OnReleaseDevice() {
  ReleaseBuffers();
  ReleaseFrameTextures();

  /* prevent locking up because other resources have allready been freed */
  ManagedEffectList prevEffects = ManagedEffects; ManagedEffects.clear();
  ManagedEffectList::iterator SEffect = prevEffects.begin();

  while (SEffect != prevEffects.end()) {
    delete (*SEffect);
    SEffect++;
  }

  Reset();

  if (EffectDepth) { delete EffectDepth; EffectDepth = NULL; }
  if (EffectShare) { delete EffectShare; EffectShare = NULL; }
}

void EffectManager::OnLostDevice() {
  ReleaseBuffers();
  ReleaseFrameTextures();

  ManagedEffectList::iterator SEffect = ManagedEffects.begin();

  while (SEffect != ManagedEffects.end()) {
    (*SEffect)->OnLostDevice();
    SEffect++;
  }

  if (EffectDepth) EffectDepth->OnLostDevice();
  if (EffectShare) EffectShare->OnLostDevice();
}

void EffectManager::OnResetDevice() {
  ReleaseBuffers();
  ReleaseFrameTextures();

  InitialiseBuffers();
  InitialiseFrameTextures();

  ManagedEffectList::iterator SEffect = ManagedEffects.begin();

  while (SEffect != ManagedEffects.end()) {
    (*SEffect)->OnResetDevice();
    (*SEffect)->ApplyPermanents(this);

    SEffect++;
  }

  if (EffectDepth) EffectDepth->OnResetDevice();
  if (EffectShare) EffectShare->OnResetDevice();
}

inline void EffectManager::UpdateFrameConstants(v1_2_416::NiDX9Renderer *Renderer) {
  v1_2_416::NiCamera **pMainCamera = (v1_2_416::NiCamera **)0x00B43124;
  const char *CamName = (*pMainCamera)->m_pcName;
  Renderer->SetCameraViewProj(*pMainCamera);

  /* update matrices after the view/projection has been set */
  Constants.UpdateView      ((const D3DXMATRIX &)Renderer->m44View      );
  Constants.UpdateProjection((const D3DXMATRIX &)Renderer->m44Projection);

  D3DXMatrixTranslation(&Constants.wrld,
			-(*pMainCamera)->m_worldTranslate.x,
			-(*pMainCamera)->m_worldTranslate.y,
			-(*pMainCamera)->m_worldTranslate.z);

  (*pMainCamera)->m_worldRotate.GetForwardVector(&Constants.EyeForward);

  /* products and inverses */
  Constants.UpdateProducts();

  // Sunrise is at 06:00, Sunset at 20:00
  bool DayTime =
    (Constants.iGameTime.x >= Constants.SunTiming.x) &&
    (Constants.iGameTime.x <= Constants.SunTiming.w);
  bool SunHasBenCulled =
    (Constants.SunDir.w == 0.0);

#ifdef OBGE_CONSTANTPOOLS
  /* we use pooling, just this has to be set */
  EffectShare->ApplySharedConstants();
#endif

  /* setup flags for effect-filtering */
  RenderCnd = (RenderCnd & ~EFFECTCOND_ISDAY  ) | ( DayTime			    ? EFFECTCOND_ISDAY   : 0);
  RenderCnd = (RenderCnd & ~EFFECTCOND_ISNIGHT) | (!DayTime			    ? EFFECTCOND_ISNIGHT : 0);
  RenderCnd = (RenderCnd & ~EFFECTCOND_HASSUN ) | (!SunHasBenCulled                 ? EFFECTCOND_HASSUN  : 0);
#ifndef	OBGE_NOSHADER
  RenderCnd = (RenderCnd & ~EFFECTCOND_HASREFL) | (passTexture[OBGEPASS_REFLECTION] ? EFFECTCOND_HASREFL : 0);
#endif
}

void EffectManager::Render(IDirect3DDevice9 *D3DDevice, IDirect3DSurface9 *RenderTo, IDirect3DSurface9 *RenderFrom) {
  v1_2_416::NiDX9Renderer *Renderer = v1_2_416::GetRenderer();

  markerStop(D3DDevice);

  // Sets up the viewport.
  float test[4] = { 0.0, 1.0, 1.0, 0.0 };
  Renderer->SetupScreenSpaceCamera(test);

  // Set up world/view/proj matrices to identity in case there's no vertex effect.
  D3DXMATRIX mIdent;
  D3DXMatrixIdentity(&mIdent);

  D3DDevice->SetTransform(D3DTS_PROJECTION, &mIdent);
  D3DDevice->SetTransform(D3DTS_VIEW, &mIdent);
  D3DDevice->SetTransform(D3DTS_WORLD, &mIdent);

  UpdateFrameConstants(Renderer);

  Renderer->RenderStateManager->SetRenderState(D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_ALPHA | D3DCOLORWRITEENABLE_BLUE | D3DCOLORWRITEENABLE_GREEN | D3DCOLORWRITEENABLE_RED, false);
  Renderer->RenderStateManager->SetRenderState(D3DRS_ALPHATESTENABLE, false, false);
  Renderer->RenderStateManager->SetRenderState(D3DRS_ALPHABLENDENABLE, false, false);
  Renderer->RenderStateManager->SetRenderState(D3DRS_ZENABLE, D3DZB_FALSE, false);
  Renderer->RenderStateManager->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE, false);

  /* is it multi-sampled? this has to be done before everything else
   * as a geometry-call will be done to resolve the depth-target
   */
  bool resz;
  if ((resz = ResolveDepthBuffer(D3DDevice))) {
//  OrigDS.Initialise(GetDepthBufferTexture());
//  FXMan->OrigDS.SetTexture("oblv_CurrDepthStencilZ_MAINPASS", pEffect);
  }

  /* full-screen quad */
  D3DDevice->SetStreamSource(0, EffectVertex, 0, sizeof(EffectQuad));
  Renderer->RenderStateManager->SetFVF(EFFECTQUADFORMAT, false);

  /* current state:
   *
   * - active Rendertarget[0] is RenderTo
   * - the last (internal) effect's output is in RenderFrom
   */

#ifdef	OLD_QUEUE
  if (RAWZflag) {
    D3DDevice->EndScene();

    D3DDevice->SetRenderTarget(0, depthSurface);
    D3DDevice->BeginScene();
    RenderRAWZfix(D3DDevice, RenderTo);
    D3DDevice->EndScene();

    D3DDevice->SetRenderTarget(0, RenderTo);
    D3DDevice->BeginScene();
  }

  D3DDevice->StretchRect(RenderFrom, 0, thisframeSurf, 0, D3DTEXF_NONE);
  D3DDevice->StretchRect(RenderFrom, 0, RenderTo, 0, D3DTEXF_NONE);
  // Blank screen fix when EffectList is empty.

  ManagedEffectList::iterator SEffect = ManagedEffects.begin();

  while (SEffect != ManagedEffects.end()) {
    if ((*SEffect)->IsEnabled()) {
      (*SEffect)->ApplySharedConstants(&EffectConst);
      (*SEffect)->ApplyUniqueConstants();
      (*SEffect)->Render(D3DDevice, RenderTo, lastpassSurf);

      D3DDevice->StretchRect(RenderTo, 0, thisframeSurf, 0, D3DTEXF_NONE);
    }

    SEffect++;
  }

  D3DDevice->StretchRect(RenderTo, 0, lastframeSurf, 0, D3DTEXF_NONE);
#else
  /* rendertarget without texture, this can happen when the
   * color-buffer is multi-sampled
   */
#ifndef	OBGE_NOSHADER
  if ((OrigRT.Initialise(RenderFrom) != D3D_OK))
#endif
  {
    OrigRT.Initialise(RenderFmt);
    OrigRT.Copy(D3DDevice, RenderFrom);
  }

  /* rendertarget without texture, non-HDR & non-Bloom special case
   * this basically is the raw backbuffer I think
   */
  if ((TrgtRT.Initialise(RenderTo) != D3D_OK)) {
    CopyRT.Initialise(RenderFmt);
    RenderBuf |= EFFECTBUF_COPY;
  }

//if (RenderBuf & EFFECTBUF_ZBUF) {
  if (EffectDepth) {
    D3DXHANDLE tec;

    OrigDS.SetTexture("zbufferTexture", EffectDepth->GetEffect());
    CurrDS.SetRenderTarget(D3DDevice);

    if (RenderTransferZ > EFFECTBUF_RAWZ) {
      if (RenderTransferZ & EFFECTBUF_EPOS) {
        /* convert WHATEVER to projected form (CurrDS) */
	tec = EffectDepth->GetEffect()->GetTechniqueByName("projecteye");

	if (RenderTransferZ & EFFECTBUF_ENRM) {
	  EffectDepth->GetEffect()->SetTechnique(tec);
	  EffectDepth->Render(D3DDevice);

	  CurrDS.SetTexture("zbufferTexture", EffectDepth->GetEffect());
	  CurrNM.SetRenderTarget(D3DDevice);

	  /* convert projected positions to normals (OrigDS) */
	  tec = EffectDepth->GetEffect()->GetTechniqueByName("normaleye");
	}
      }
      else
        /* convert WHATEVER to linearized form (CurrDS) */
	tec = EffectDepth->GetEffect()->GetTechniqueByName("linearize");

      if (RenderTransferZ & EFFECTBUF_RAWZ) {
	EffectDepth->GetEffect()->SetTechnique(tec);
	EffectDepth->Render(D3DDevice);

        CurrDS.SetTexture("zbufferTexture", EffectDepth->GetEffect());
        OrigDS.SetRenderTarget(D3DDevice);

	/* convert linearized to unlinearize form (OrigDS) */
        tec = EffectDepth->GetEffect()->GetTechniqueByName("unlinearize");
      }
    }
    else
      /* convert WHATEVER to unlinearize form (CurrDS) */
      tec = EffectDepth->GetEffect()->GetTechniqueByName("copy");

    /* OrigDS always contains "nonlinearized" INTZ */
    /* CurrDS always contains "linearized" or "projected" */
    /* CurrNM always contains "normal" */

    EffectDepth->GetEffect()->SetTechnique(tec);
    EffectDepth->Render(D3DDevice);
  }

  RenderQueue.device = D3DDevice;
  RenderQueue.Init(
    (RenderBuf & EFFECTBUF_PAST) ? &PastRT : NULL,
    (RenderBuf & EFFECTBUF_PREV) ? &PrevRT : NULL,
				   &LastRT
  );

  /* over effects */
  RenderQueue.Begin(
				   &OrigRT,
				   &TrgtRT,
    (RenderBuf & EFFECTBUF_COPY) ? &CopyRT : NULL
  );

#if 0 //def	OBGE_STATEBLOCKS
  /* auto backup (not strictly needed, all states can be changed) */
  IDirect3DStateBlock9 *pStateBlock = NULL;
  D3DDevice->CreateStateBlock(D3DSBT_ALL, &pStateBlock);
#endif

  /* count if something happens */
  int run = 0;

  for (ManagedEffectList::iterator e = ManagedEffects.begin(); e != ManagedEffects.end(); e++) {
    /* check if the conditions meet */
    unsigned long EffectCnd = (*e)->GetConditions();
    if ((RenderCnd & EffectCnd) == EffectCnd)
      run += (*e)->Render(D3DDevice, &RenderQueue);
  }

  /* nothing happend */
  if (!run) TrgtRT.Copy(D3DDevice, &OrigRT);

#if 0 //def	OBGE_STATEBLOCKS
  /* auto restore (not strictly needed, all states can be changed) */
  pStateBlock->Apply();
  pStateBlock->Release();
#endif

  RenderQueue.End(
				   &TrgtRT
  );
#endif

  markerStart(D3DDevice);
}

void EffectManager::RenderRAWZfix(IDirect3DDevice9 *D3DDevice, IDirect3DSurface9 *RenderTo) {
#ifdef	OLD_QUEUE
  if (!EffectDepth) {
    EffectDepth = new EffectRecord();

    if (!EffectDepth->LoadEffect("RAWZfix.fx", 0)) {
      _MESSAGE("ERROR - RAWZfix.fx is missing! Please reinstall OBGE");
      exit(0); return;
    }

    EffectDepth->CompileEffect();
    EffectDepth->GetEffect()->SetTexture("RAWZdepth", depthRAWZ);
  }

  EffectDepth->Render(D3DDevice, RenderTo, lastpassSurf);
#endif
}

int EffectManager::AddPrivateEffect(const char *Filename, UINT32 refID) {
  ManagedEffectRecord *NewEffect = new ManagedEffectRecord();

  if (!NewEffect->LoadEffect(Filename, refID, true)) {
    delete NewEffect;
    return -1;
  }

  /* you are allowed to fail */
  NewEffect->CompileEffect(this);

  /* prepare */
  NewEffect->SetPriority(ManagedEffects.size());
  NewEffect->AddRef();

  /* append and sort */
  ManagedEffects.push_back(NewEffect);
  std::sort(ManagedEffects.begin(), ManagedEffects.end(), ManagedEffectRecord::compare);

  /* register */
  Effects[EffectIndex++] = NewEffect;

  return EffectIndex - 1;
}

int EffectManager::AddManagedEffect(const char *Filename, UINT32 refID) {
  EffectRegistry::iterator SEffect = Effects.begin();

  /* search for a non-private effect */
  while (SEffect != Effects.end()) {
    if (!SEffect->second->IsPrivate()) {
      if (!_stricmp(Filename, SEffect->second->GetName())/*&& ((pEffect->second->ParentRefID & 0xff000000) == (refID & 0xff000000))*/) {
	_MESSAGE("Loading effect \"%s\" that already exists. Returning index \"%d\" of existing effect.", SEffect->second->GetName(), SEffect->first);

	SEffect->second->AddRef();
	return SEffect->first;
      }
    }

    SEffect++;
  }

  ManagedEffectRecord *NewEffect = new ManagedEffectRecord();

  if (!NewEffect->LoadEffect(Filename, refID, false)) {
    delete NewEffect;
    return -1;
  }

  /* you are allowed to fail */
  NewEffect->CompileEffect(this);

  /* prepare */
  NewEffect->SetPriority(ManagedEffects.size());
  NewEffect->AddRef();

  /* append and sort */
  ManagedEffects.push_back(NewEffect);
  std::sort(ManagedEffects.begin(), ManagedEffects.end(), ManagedEffectRecord::compare);

  /* register */
  Effects[EffectIndex++] = NewEffect;

  return EffectIndex - 1;
}

int EffectManager::AddDependtEffect(const char *Filename, UINT32 refID) {
  EffectRegistry::iterator SEffect = Effects.begin();

  /* search for a any effect */
  while (SEffect != Effects.end()) {
    if (!_stricmp(Filename, SEffect->second->GetName())/*&& ((pEffect->second->ParentRefID & 0xff000000) == (refID & 0xff000000))*/) {
      _MESSAGE("Loading effect \"%s\" that already exists. Returning index \"%d\" of existing effect.", SEffect->second->GetName(), SEffect->first);

      SEffect->second->AddRef();
      return SEffect->first;
    }

    SEffect++;
  }

  ManagedEffectRecord *NewEffect = new ManagedEffectRecord();

  if (!NewEffect->LoadEffect(Filename, refID, true)) {
    delete NewEffect;
    return -1;
  }

  /* you are allowed to fail */
  NewEffect->CompileEffect(this);

  /* prepare */
  NewEffect->SetPriority(ManagedEffects.size());
  NewEffect->AddRef();

  /* append and sort */
  ManagedEffects.push_back(NewEffect);
  std::sort(ManagedEffects.begin(), ManagedEffects.end(), ManagedEffectRecord::compare);

  /* register */
  Effects[EffectIndex++] = NewEffect;

  return EffectIndex - 1;
}

int EffectManager::FindEffect(const char *Filename) {
  EffectRegistry::iterator SEffect = Effects.begin();

  /* search for a non-private effect */
  while (SEffect != Effects.end()) {
    if (!SEffect->second->IsPrivate()) {
      if (!_stricmp(Filename, SEffect->second->GetName()))
	return SEffect->first;
    }

    SEffect++;
  }

  return -1;
}

bool EffectManager::ReleaseEffect(int EffectNum) {
  if (!IsEffectValid(EffectNum))
    return false;

  ManagedEffectRecord *OldEffect = Effects[EffectNum];

  /* reached zero */
  if (!OldEffect->Release()) {
    /* remove from map */
    Effects.erase(EffectNum);
    /* remove from vector */
    ManagedEffects.erase(std::find(ManagedEffects.begin(), ManagedEffects.end(), OldEffect));

    delete OldEffect;
    return true;
  }

  return false;
}

void EffectManager::Recalculate() {
  /* redo the conditions */
  RenderBuf = 0;
  RenderCnd = 0;

  for (ManagedEffectList::iterator e = ManagedEffects.begin(); e != ManagedEffects.end(); e++) {
    if ((*e)->IsEnabled()) {
      /* integrate */
      RenderBuf |= (*e)->GetParameters();
      RenderCnd |= (*e)->GetConditions();
    }
  }

  SetTransferZ(RenderBuf);

  /* update the buffers */
  InitialiseFrameTextures();

  for (ManagedEffectList::iterator e = ManagedEffects.begin(); e != ManagedEffects.end(); e++) {
    if ((*e)->IsEnabled()) {
      /* update */
      (*e)->ApplyPermanents(this);
    }
  }
}

void EffectManager::LoadEffectList() {
  FILE *EffectFile;
  char EffectBuffer[260];
  int lastpos;
  bool enabled;

  if (::UseEffectList.Get()) {
    _MESSAGE("Loading the effects.");

    if (!fopen_s(&EffectFile, ::EffectListFile.Get(), "rt")) {
      while (!feof(EffectFile)) {
        if (fgets(EffectBuffer, 260, EffectFile)) {
          lastpos = strlen(EffectBuffer) - 1;
          enabled = true;

          if (EffectBuffer[lastpos] == 10 ||
	      EffectBuffer[lastpos] == 13)
            EffectBuffer[lastpos] = 0;

          if (EffectBuffer[lastpos - 2] == '=') {
            enabled = (EffectBuffer[lastpos - 1] == '1');
	    EffectBuffer[lastpos - 2] = 0;
          }

	  /* TODO: don't count this as ref */
          int EffectNum = AddManagedEffect(EffectBuffer, 0);
	  if (EffectNum != -1) {
	    ManagedEffectRecord *pEffect;

	    pEffect = GetEffect(EffectNum);
            pEffect->Enable(enabled);
	  //pEffect->ClrRef();
	  }
        }
      }

      fclose(EffectFile);
    }
    else
      _MESSAGE("Error opening shaderlist.txt file.");
  }
  else
    _MESSAGE("EffectList has been disabled by the INI file.");
}

void EffectManager::NewGame() {
  if (PurgeOnNewGame.Get()) {
    /* prevent locking up because other resources have allready been freed */
    EffectRegistry prevEffects = Effects; Effects.clear();
    EffectRegistry::iterator Effect = prevEffects.begin();

    /* delete terminally */
    while (Effect != prevEffects.end()) {
      delete Effect->second;
      Effect++;
    }

    /* redo everything */
    Reset();
  }
  else {
    EffectRegistry::iterator Effect = Effects.begin();

    /* disable and remove all refs */
    while (Effect != Effects.end()) {
      /* disable, but have them cached */
      Effect->second->Enable(false);
      /* reset to initial state */
      Effect->second->ApplyCompileDirectives();
      /* TODO ClrRef clears enabled as well, logical, no? */
      Effect->second->ClrRef();

      /* delete private shaders I guess */
      Effect++;
    }

    /* redo the conditions */
    RenderBuf = 0;
    RenderCnd = 0;
  }
}

void EffectManager::SaveGame(OBSESerializationInterface *Interface) {
  int temp;

  _MESSAGE("EffectManager::SaveGame");

  ManagedEffectList::iterator pEffect = ManagedEffects.begin();

  Interface->WriteRecord('SIDX', SHADERVERSION, &EffectIndex, sizeof(EffectIndex));

  _MESSAGE("Save-game will reference %i effects.", EffectIndex);

  while (pEffect != ManagedEffects.end()) {
    if (!(*pEffect)->IsPrivate()) {
      if ((*pEffect)->GetEffect()) {
	/* this is very bad ... linear search */
	int EffectNum = -1;

	EffectRegistry::iterator IEffect = Effects.begin();
	while (IEffect != Effects.end()) {
	  if (IEffect->second == (*pEffect)) {
	    EffectNum = IEffect->first;
	    break;
	  }

	  IEffect++;
	}

	/* locals */
	const char *Name   = (*pEffect)->GetName();
	const bool Enabled = (*pEffect)->IsEnabled();
	const UINT32 RefID = (*pEffect)->GetRefID();

        Interface->WriteRecord('SNUM', SHADERVERSION, &EffectNum, sizeof(EffectNum));
        Interface->WriteRecord('SPAT', SHADERVERSION, Name, strlen(Name) + 1);
        Interface->WriteRecord('SENB', SHADERVERSION, &Enabled, sizeof(Enabled));
        Interface->WriteRecord('SREF', SHADERVERSION, &RefID, sizeof(RefID));

	/* save variables and non-private textures (those that
	 * have been replaced/changed from the source)
	 */
	(*pEffect)->SaveVars(Interface);

        Interface->WriteRecord('SEOD', SHADERVERSION, &temp, 1);
      }
    }

    pEffect++;
  }

  Interface->WriteRecord('SEOF', SHADERVERSION, &temp, 1);
}

void EffectManager::LoadGame(OBSESerializationInterface *Interface) {
  UInt32 type, version, length;
  int LoadShaderNum;
  char LoadFilepath[260];
  bool LoadEnabled;
  UInt32 LoadRefID;
  bool InUse;

  Interface->GetNextRecordInfo(&type, &version, &length);

  if (type == 'SIDX') {
    Interface->ReadRecordData(&EffectIndex, length);
    _MESSAGE("Save-game references to %i effects.", EffectIndex);
  }
  else {
    _MESSAGE("No effect data in save-game.");
    return;
  }

  Interface->GetNextRecordInfo(&type, &version, &length);

  while (type != 'SEOF') {
    if (type == 'SNUM') {
      Interface->ReadRecordData(&LoadShaderNum, length);
      _MESSAGE("Found SNUM record = %d", LoadShaderNum);
    }
    else {
      _MESSAGE("Error loading game. type!=SNUM");
      return;
    }

    Interface->GetNextRecordInfo(&type, &version, &length);

    if (type == 'SPAT') {
      Interface->ReadRecordData(LoadFilepath, length);
      _MESSAGE("Filename = %s", LoadFilepath);
    }
    else {
      _MESSAGE("Error loading game. type!=SPAT");
      return;
    }

    Interface->GetNextRecordInfo(&type, &version, &length);

    if (type == 'SENB') {
      Interface->ReadRecordData(&LoadEnabled, length);
      _MESSAGE("Enabled = %d", LoadEnabled);
    }
    else {
      _MESSAGE("Error loading game. type!=SENB");
      return;
    }

    Interface->GetNextRecordInfo(&type, &version, &length);

    if (type == 'SREF') {
      Interface->ReadRecordData(&LoadRefID, length);
      _MESSAGE("RefID = 0x%08x", LoadRefID);

      if (LoadRefID == 0) {
        _MESSAGE("NULL refID. Will load effect as I can't resolve it's state.");
        InUse = true;
      }
      else {
        InUse = Interface->ResolveRefID(LoadRefID, &LoadRefID);
        _MESSAGE("Is in use = %d", InUse);
      }
    }
    else {
      _MESSAGE("Error loading game. type!=SREF");
      return;
    }

    /* the only effects we load here are effect which have been
     * added by scripts, the effects from the shader-list may have
     * saved entries, but we ignore them
     */
    int EffectNum = -1;
    if (InUse)
      EffectNum = AddManagedEffect(LoadFilepath, LoadRefID);
    else
      EffectNum = FindEffect(LoadFilepath);

    if (EffectNum != -1) {
      ManagedEffectRecord *NewEffect = GetEffect(EffectNum);
      ID3DXEffect *pEffect = NewEffect->GetEffect();

      Interface->GetNextRecordInfo(&type, &version, &length);

      while (type != 'SEOD') {
	switch (type) {
	    case 'STEX':
	      TextureType TextureData;

	      Interface->ReadRecordData(&TextureData, length);
	      NewEffect->SetEffectSamplerTexture(TextureData.Name, TextureData.tex);
	      _MESSAGE("Texture %s = %d", TextureData.Name, TextureData.tex);
	      break;
	    case 'SINT':
	      IntType IntData;

	      Interface->ReadRecordData(&IntData, length);
	      NewEffect->SetEffectConstantI(IntData.Name, (int *)&IntData.data, IntData.size);
	      _MESSAGE("Int %s = %d(%d)", IntData.Name, IntData.data[0], IntData.size);
	      break;
	    case 'SFLT':
	      FloatType FloatData;

	      Interface->ReadRecordData(&FloatData, length);
	      NewEffect->SetEffectConstantF(FloatData.Name, (float *)&FloatData.data, FloatData.size);
	      _MESSAGE("Float %s = %f(%d)", FloatData.Name, FloatData.data[0], FloatData.size);
	      break;
	}

	Interface->GetNextRecordInfo(&type, &version, &length);
      }

      NewEffect->Enable(LoadEnabled);
      if (LoadShaderNum <= 0)
	LoadShaderNum = EffectIndex;

      /* register */
      if (LoadShaderNum != EffectNum) {
	Effects.erase(EffectNum);
	Effects[LoadShaderNum] = NewEffect;
      }

      if (EffectIndex <= LoadShaderNum)
	EffectIndex = LoadShaderNum + 1;
    }

    else {
      Interface->GetNextRecordInfo(&type, &version, &length);
      while (type != 'SEOD') {
        Interface->ReadRecordData(LoadFilepath, length);
        Interface->GetNextRecordInfo(&type, &version, &length);
      }
    }

    Interface->GetNextRecordInfo(&type, &version, &length);
  }
}

bool EffectManager::EnableEffect(int EffectNum, bool State) {
  ManagedEffectRecord *pEffect;

  if ((pEffect = GetEffect(EffectNum))) {
    pEffect->Enable(State); return true; }

  return false;
}

bool EffectManager::SetEffectConstantB(int EffectNum, char *name, bool value) {
  ManagedEffectRecord *pEffect;

  if ((pEffect = GetEffect(EffectNum)))
    return pEffect->SetEffectConstantB(name, value);

  return false;
}

bool EffectManager::SetEffectConstantI(int EffectNum, char *name, int value) {
  ManagedEffectRecord *pEffect;

  if ((pEffect = GetEffect(EffectNum)))
    return pEffect->SetEffectConstantI(name, value);

  return false;
}

bool EffectManager::SetEffectConstantF(int EffectNum, char *name, float value) {
  ManagedEffectRecord *pEffect;

  if ((pEffect = GetEffect(EffectNum)))
    return pEffect->SetEffectConstantF(name, value);

  return false;
}

bool EffectManager::SetEffectConstantV(int EffectNum, char *name, v1_2_416::NiVector4 *value) {
  ManagedEffectRecord *pEffect;

  if ((pEffect = GetEffect(EffectNum)))
    return pEffect->SetEffectConstantV(name, value);

  return false;
}

bool EffectManager::SetEffectSamplerTexture(int EffectNum, char *name, int TextureNum) {
  ManagedEffectRecord *pEffect;

  if ((pEffect = GetEffect(EffectNum)))
    return pEffect->SetEffectSamplerTexture(name, TextureNum);

  return false;
}

bool EffectManager::GetEffectState(int EffectNum) {
  ManagedEffectRecord *pEffect;

  if ((pEffect = GetEffect(EffectNum)))
    return pEffect->IsEnabled();

  return false;
}

void EffectManager::PurgeTexture(IDirect3DBaseTexture9 *texture, int TexNum) {
  ManagedEffectList::iterator pEffect = ManagedEffects.begin();

  while (pEffect != ManagedEffects.end()) {
    if (*pEffect && (*pEffect)->HasEffect())
      (*pEffect)->PurgeTexture(texture, TexNum);

    pEffect++;
  }
}

/* -------------------------------------------------------------------------------------------------
 */

void EffectManager::UseLegacyCompiler(bool yes) {
  ::UseLegacyCompiler.Set(yes);
}

void EffectManager::CompileSources(bool yes) {
  /*::CompileSources.Set(yes)*/;
}

void EffectManager::Optimize(bool yes) {
  ::Optimize.Set(yes);
}

bool EffectManager::UseLegacyCompiler() {
  return ::UseLegacyCompiler.Get();
}

bool EffectManager::CompileSources() {
  return true;
}

bool EffectManager::Optimize() {
  return ::Optimize.Get();
}

const char *EffectManager::EffectDirectory() {
  return ::EffectDirectory.Get();
}

const char *EffectManager::EffectListFile() {
  return ::EffectListFile.Get();
}

bool EffectManager::UseEffectList() {
  return ::UseEffectList.Get();
}
