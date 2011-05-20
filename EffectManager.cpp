#include <sys/stat.h>

#include "EffectManager.h"
#include "TextureManager.h"
#include "GlobalSettings.h"
#include "D3D9.hpp"
#include "OBSEShaderInterface.h"

#include <algorithm>

static global<bool> UseShaderList(true, NULL, "Effects", "bUseShaderList");
static global<char*> EffectDirectory("data\\shaders\\",NULL,"Effects","sEffectDirectory");
static global<char *> ShaderListFile("data\\shaders\\shaderlist.txt", NULL, "Effects", "sShaderListFile");
static global<bool> UseLegacyCompiler(false, NULL, "Effects", "bUseLegacyCompiler");
static global<bool> Optimize(false, NULL, "Effects", "bOptimize");
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
#define	EFFECTCLASS_LIGHT	0x01
#define	EFFECTCLASS_AO	        0x02
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
#define	EFFECTCOND_HASREFL	(1 << 30)
#define	EFFECTCOND_HASZBUF	(1 << 31)	// special one

#define	EFFECTBUF_COPY		(1 << 28)	// need frame "copy" because the effect surfaces have other format
#define	EFFECTBUF_PREV		(1 << 29)	// need previous effect "copy"
#define	EFFECTBUF_PAST		(1 << 30)	// need previous frame "copy"
#define	EFFECTBUF_ZBUF		EFFECTCOND_HASZBUF

static D3DXMACRO defs[] = {
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
  {"EFFECTCOND_INTERIOUR"	, stringify(EFFECTCOND_INTERIOUR	)},
  {"EFFECTCOND_EXTERIOUR"	, stringify(EFFECTCOND_EXTERIOUR	)},
  {"EFFECTCOND_UNDERWATER"	, stringify(EFFECTCOND_UNDERWATER	)},
  {"EFFECTCOND_ISDAY"	        , stringify(EFFECTCOND_ISDAY	        )},
  {"EFFECTCOND_ISNIGHT"	        , stringify(EFFECTCOND_ISNIGHT	        )},

  {"EFFECTCOND_ZBUFFER"		, stringify(EFFECTCOND_HASZBUF		)}, // hm, error

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
  Tex[0] = Tex[1] = Tex[2] = Tex[3] = NULL;
  Srf[0] = Srf[1] = Srf[2] = Srf[3] = NULL;
}

EffectBuffer::~EffectBuffer() {
  Release();
}

inline HRESULT EffectBuffer::Initialise(IDirect3DTexture9 *text) {
  Tex[0] = text;
  Srf[0] = NULL;
//Tex[0]->GetSurfaceLevel(0, &Srf[0]);

  Tex[1] = Tex[2] = Tex[3] = NULL;
  Srf[1] = Srf[2] = Srf[3] = NULL;

  return (Srf[0] ? D3D_OK : S_FALSE);
}

inline HRESULT EffectBuffer::Initialise(enum OBGEPass pass, IDirect3DSurface9 *surf) {
  Tex[0] = surfaceTexture[surf] ? surfaceTexture[surf]->tex : NULL;
  Srf[0] =                surf                                    ;

  Tex[1] = Tex[2] = Tex[3] = NULL;
  Srf[1] = Srf[2] = Srf[3] = NULL;

  return (Tex[0] ? D3D_OK : S_FALSE);
}

inline HRESULT EffectBuffer::Initialise(D3DFORMAT rt0, D3DFORMAT rt1, D3DFORMAT rt2, D3DFORMAT rt3) {
  UInt32 Width = v1_2_416::GetRenderer()->SizeWidth;
  UInt32 Height = v1_2_416::GetRenderer()->SizeHeight;
  HRESULT hr;

  if (!Tex[0] && (rt0 != D3DFMT_UNKNOWN)) {
    if ((hr = GetD3DDevice()->CreateTexture(Width, Height, 1, D3DUSAGE_RENDERTARGET, rt0, D3DPOOL_DEFAULT, &Tex[0], 0)) == D3D_OK)
      Tex[0]->GetSurfaceLevel(0, &Srf[0]);
  }

  if (!Tex[1] && (rt1 != D3DFMT_UNKNOWN)) {
    if ((hr = GetD3DDevice()->CreateTexture(Width, Height, 1, D3DUSAGE_RENDERTARGET, rt1, D3DPOOL_DEFAULT, &Tex[1], 0)) == D3D_OK)
      Tex[1]->GetSurfaceLevel(0, &Srf[1]);
  }

  if (!Tex[2] && (rt2 != D3DFMT_UNKNOWN)) {
    if ((hr = GetD3DDevice()->CreateTexture(Width, Height, 1, D3DUSAGE_RENDERTARGET, rt2, D3DPOOL_DEFAULT, &Tex[2], 0)) == D3D_OK)
      Tex[2]->GetSurfaceLevel(0, &Srf[2]);
  }

  if (!Tex[3] && (rt3 != D3DFMT_UNKNOWN)) {
    if ((hr = GetD3DDevice()->CreateTexture(Width, Height, 1, D3DUSAGE_RENDERTARGET, rt3, D3DPOOL_DEFAULT, &Tex[3], 0)) == D3D_OK)
      Tex[3]->GetSurfaceLevel(0, &Srf[3]);
  }

  return D3D_OK;
}

inline void EffectBuffer::Release() {
  for (int rt = 0; rt < 4; rt++) {
    if (Tex[rt]) Tex[rt]->Release();
    if (Srf[rt]) Srf[rt]->Release();

    Tex[rt] = NULL;
    Srf[rt] = NULL;
  }
}

inline bool EffectBuffer::IsValid() {
  for (int rt = 0; rt < 4; rt++) {
    if (Tex[rt]) return true;
  }

  return false;
}

bool EffectBuffer::IsTexture(IDirect3DBaseTexture9 *text) {
  for (int rt = 0; rt < 4; rt++) {
    if (Tex[rt] == text) return true;
  }

  return false;
}

inline void EffectBuffer::SetTexture(const char *fmt, ID3DXEffect *pEffect) {
  char buf[256];
  for (int rt = 0; rt < 4; rt++) {
    if (Tex[rt]) {
      sprintf(buf, fmt, rt);

      pEffect->SetTexture(buf, Tex[rt]);
    }
  }
}

inline void EffectBuffer::SetRenderTarget(IDirect3DDevice9 *Device) {
  for (int rt = 0; rt < 4; rt++) {
    if (Srf[rt]) {
      Device->SetRenderTarget(rt, Srf[rt]);
    }
  }
}

inline void EffectBuffer::Copy(IDirect3DDevice9 *Device, EffectBuffer *from) {
  for (int rt = 0; rt < 4; rt++) {
    if (Srf[rt] && from->Srf[rt])
      Device->StretchRect(from->Srf[rt], 0, Srf[rt], 0, D3DTEXF_NONE);
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
   *     step()  alterning = 1, 1 == alt, setrendertarget(alt)
   *     end()
   *  b) begin() alterning = 0, 0 == target, setrendertarget(target)
   *     step()  alterning = 1, 1 == alt, setrendertarget(alt)
   *     step()  alterning = 0, 0 == target, setrendertarget(target)
   *     end()
   *  c) begin() alterning = 1, 1 == alt, setrendertarget(alt)
   *     step()  alterning = 0, 0 == target, setrendertarget(target)
   *     step()  alterning = 1, 1 == alt, setrendertarget(alt)
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
}

inline void EffectQueue::Step(ID3DXEffect *pEffect) {
  rotate[EQLAST]->SetTexture("obge_LastRendertarget%d_EFFECTPASS", pEffect);

  alterning ^= 1; (rotate[EQLAST] = queue[alterning])->SetRenderTarget(device);
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
   * or when the have an odd number of accumulated passes
   */
  if (target != rotate[EQLAST])
    target->Copy(device, rotate[EQLAST]);

  /* not allocated if not needed! */
  if (past)
    past->Copy(device, orig);
}

#undef EQLAST
#undef EQPREV

/* #################################################################################################
 */

EffectRecord::EffectRecord() {
  Filepath[0] = '\0';
  Name[0] = '\0';
  ParentRefID = 0xFF000000;

  Enabled = false;
  Private = false;

  /* shader source, binaries and constant tables */
  pDefine = NULL;
  pBinary = NULL;
  pEffect = NULL;
  pSource = NULL; sourceLen = 0;
  pErrorMsgs = NULL;

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

  this->pDefine = NULL;
  this->pBinary = NULL;
  this->pEffect = NULL;
  this->pSource = NULL;
  this->pErrorMsgs = NULL;

  this->Name[0] = '\0';
  this->Filepath[0] = '\0';
}

bool EffectRecord::LoadEffect(const char *Filename, UINT32 refID, bool Private, D3DXMACRO *defs) {
  Kill();

  if (strlen(Filename) > 240)
    return false;
  if (!defs)
    defs = ::defs;

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

  return true;
}

bool EffectRecord::RuntimeFlush() {
  /* release all runtime resources (for example for recompilation) */
  if (pBinary) pBinary->Release();
  if (pEffect) pEffect->Release();
  if (pErrorMsgs) pErrorMsgs->Release();

  pBinary = NULL;
  pEffect = NULL;
  pErrorMsgs = NULL;

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
  RuntimeFlush();

  return true;
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

bool EffectRecord::CompileEffect(bool forced) {
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
    if (pErrorMsgs)
      pErrorMsgs->Release();
    pErrorMsgs = NULL;

    D3DXCreateEffect(
      GetD3DDevice(),
      src,
      len,
      pDefine,
      &incl,
      D3DXSHADER_DEBUG | (
      ::UseLegacyCompiler.Get() ? D3DXSHADER_USE_LEGACY_D3DX9_31_DLL : (
      ::Optimize.Get()          ? D3DXSHADER_OPTIMIZATION_LEVEL3 : 0)),
      NULL,
      &x,
      &pErrorMsgs
      );

    /* this didn't go so well, if it's a legacy "error", just try again */
    if (pErrorMsgs && strstr((char*)pErrorMsgs->GetBufferPointer(), "X3539")) {
      pErrorMsgs->Release();
      pErrorMsgs = NULL;

      D3DXCreateEffect(
	GetD3DDevice(),
	src,
	len,
	defs,
	&incl,
	D3DXSHADER_DEBUG | (
	D3DXSHADER_USE_LEGACY_D3DX9_31_DLL),
	NULL,
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

  if (x)
    ApplyCompileDirectives();

  return (pSource && (pEffect != NULL));
}

void EffectRecord::ApplyCompileDirectives() {
  LPCSTR pName = NULL; pEffect->GetString("Name", &pName);
  if (pName)
    strcpy(Name, (char *)pName);

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

	  int TexNum = TexMan->LoadManagedTexture((char *)pString, TR_CUBIC);
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

	  int TexNum = TexMan->LoadManagedTexture((char *)pString, TR_VOLUMETRIC);
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

	    int TexNum = TexMan->LoadManagedTexture((char *)pString, TR_PLANAR);
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

	      EffectManager::GetSingleton()->AddDependentEffect(pString, 0xFF000000);
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
}

inline void EffectRecord::ApplyPermanents(EffectConstants *ConstList, EffectManager *FXMan) {
#ifdef	OLD_QUEUE
  pEffect->SetTexture("obge_PrevRendertarget0_EFFECTPASS", FXMan->thisframeTex);
  pEffect->SetTexture("obge_LastRendertarget0_EFFECTPASS", FXMan->lastpassTex);
  pEffect->SetTexture("obge_PastRendertarget0_MAINPASS"  , FXMan->lastframeTex);
  pEffect->SetTexture("oblv_CurrDepthStencilZ_MAINPASS"  , FXMan->depth);
#else
//pEffect->SetTexture("obge_PrevRendertarget0_EFFECTPASS", FXMan->thisframeTex);
//pEffect->SetTexture("obge_LastRendertarget0_EFFECTPASS", FXMan->lastpassTex);
//pEffect->SetTexture("obge_PastRendertarget0_MAINPASS"  , FXMan->lastframeTex);
  FXMan->CurrDS.SetTexture("oblv_CurrDepthStencilZ_MAINPASS", pEffect); if (FXMan->RenderRawZ)
  FXMan->OrigDS.SetTexture("oblv_CurrDepthStencilR_MAINPASS", pEffect);
#endif

  pEffect->SetVector("oblv_ReciprocalResolution_MAINPASS", &ConstList->rcpres);

  /* deprecated */
#ifndef	NO_DEPRECATED
  pEffect->SetBool("bHasDepth", ConstList->bHasDepth);
  pEffect->SetFloatArray("rcpres", (float *)&ConstList->rcpres, 2);
#endif
}

inline void EffectRecord::ApplyConstants(EffectConstants *ConstList) {
  pEffect->SetMatrix("oblv_WorldTransform_MAINPASS", &ConstList->world);
  pEffect->SetMatrix("oblv_ViewTransform_MAINPASS", &ConstList->view);
  pEffect->SetMatrix("oblv_ProjectionTransform_MAINPASS", &ConstList->proj);
  pEffect->SetVector("oblv_ProjectionDepthRange_MAINPASS", &ConstList->ZRange);
  pEffect->SetFloatArray("oblv_CameraForward_MAINPASS", &ConstList->EyeForward.x, 3);

  pEffect->SetIntArray("oblv_GameTime", &ConstList->GameTime.x, 4);
  pEffect->SetVector("oblv_SunDirection", &ConstList->SunDir);

  /* deprecated */
#ifndef	NO_DEPRECATED
  pEffect->SetVector("f4Time", &ConstList->time);
#endif
}

inline void EffectRecord::ApplyDynamics() {
  pEffect->SetTexture("oblv_Rendertarget0_REFLECTIONPASS", passTexture[OBGEPASS_REFLECTION]);

  /* deprecated */
#ifndef	NO_DEPRECATED
  pEffect->SetBool("bHasReflection", !!passTexture[OBGEPASS_REFLECTION]);
#endif
}

inline void EffectRecord::OnLostDevice(void) {
  if (pEffect)
    pEffect->OnLostDevice();
}

inline void EffectRecord::OnResetDevice(void) {
  if (pEffect)
    pEffect->OnResetDevice();
}

inline void EffectRecord::Render(IDirect3DDevice9 *D3DDevice, IDirect3DSurface9 *RenderTo, IDirect3DSurface9 *RenderCopy) {
  if (!Enabled)
    return;

  D3DDevice->BeginScene();

  // Have to do this in case the effect has no vertex effect. The stupid effect system
  // uses the last vertex effect that was active and much strangeness occurs.
  D3DDevice->SetVertexShader(NULL);

  UINT pass = 0;
  UINT passes; pEffect->Begin(&passes, NULL);

  while (true) {
    pEffect->BeginPass(pass);
    D3DDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
    pEffect->EndPass();

    if (++pass >= passes)
      break;

    D3DDevice->StretchRect(RenderTo, 0, RenderCopy, 0, D3DTEXF_NONE);
  }

  pEffect->End();

  D3DDevice->EndScene();
}

inline bool EffectRecord::Render(IDirect3DDevice9 *D3DDevice, EffectConstants *ConstList, EffectQueue *Queue) {
  if (!Enabled)
    return false;

  D3DDevice->BeginScene();

  // Have to do this in case the effect has no vertex effect. The stupid effect system
  // uses the last vertex effect that was active and much strangeness occurs.
  D3DDevice->SetVertexShader(NULL);

  ApplyConstants(ConstList);
  ApplyDynamics();

  UINT pass = 0; Queue->Begin(pEffect);
  UINT passes; pEffect->Begin(&passes, NULL);

  while (true) {
    pEffect->BeginPass(pass);
    D3DDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
    pEffect->EndPass();

    if (++pass >= passes)
      break;

    Queue->Step(pEffect);
  }

  pEffect->End();

  D3DDevice->EndScene();

  Queue->End(pEffect);

  return true;
}

inline void EffectRecord::Render(IDirect3DDevice9 *D3DDevice) {
  if (!Enabled)
    return;

  D3DDevice->BeginScene();

  // Have to do this in case the effect has no vertex effect. The stupid effect system
  // uses the last vertex effect that was active and much strangeness occurs.
  D3DDevice->SetVertexShader(NULL);

  UINT pass = 0;
  UINT passes; pEffect->Begin(&passes, NULL);

  while (true) {
    pEffect->BeginPass(pass);
    D3DDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
    pEffect->EndPass();

    if (++pass >= passes)
      break;
  }

  pEffect->End();

  D3DDevice->EndScene();
}

inline ID3DXEffect *EffectRecord::GetEffect() const {
  return this->pEffect;
}

inline bool EffectRecord::HasEffect() const {
  return (this->pEffect != NULL);
}

inline void EffectRecord::Enable(bool Enabled) {
  this->Enabled = Enabled;

  /* reflect changes in the applicable conditions */
  EffectManager::GetSingleton()->Recalculate();
}

inline bool EffectRecord::IsEnabled() const {
  return this->Enabled;
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

inline bool EffectRecord::SetEffectConstantB(const char *name, bool value) {
  HRESULT hr = pEffect->SetBool(name, value);
  return (hr == D3D_OK);
}

inline bool EffectRecord::SetEffectConstantI(const char *name, int value) {
  HRESULT hr = pEffect->SetInt(name, value);
  return (hr == D3D_OK);
}

inline bool EffectRecord::SetEffectConstantF(const char *name, float value) {
  HRESULT hr = pEffect->SetFloat(name, value);
  return (hr == D3D_OK);
}

inline bool EffectRecord::SetEffectConstantV(const char *name, v1_2_416::NiVector4 *value) {
  HRESULT hr = pEffect->SetVector(name, value);
  return (hr == D3D_OK);
}

bool EffectRecord::SetEffectSamplerTexture(const char *name, int TextureNum) {
  TextureManager *TexMan = TextureManager::GetSingleton();
  TextureRecord *NewTexture = TexMan->GetTexture(TextureNum);

  IDirect3DBaseTexture9 *OldTexture = NULL;
  pEffect->GetTexture(name, (LPDIRECT3DBASETEXTURE9 *)&OldTexture);
  pEffect->SetTexture(name, NULL);
  if (OldTexture)
    TexMan->ReleaseTexture(OldTexture);
  if (!NewTexture)
    return false;

  HRESULT hr = pEffect->SetTexture(name, NewTexture->GetTexture());
  return (hr == D3D_OK);
}

void EffectRecord::SaveVars(OBSESerializationInterface *Interface) {
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

          pEffect->GetTexture(handle, &Texture);
          TextureNum = TexMan->FindTexture(Texture);
          strcpy(TextureData.Name, Description.Name);

          if (TextureNum >= 0) {
            TextureData.tex = TextureNum;
            Interface->WriteRecord('STEX', SHADERVERSION, &TextureData, sizeof(TextureData));

            _MESSAGE("Found texture: name - %s, texnum - %d", TextureData.Name, TextureNum);
          }
          else
            _MESSAGE("Found texture: name - %s - not in texture list.", TextureData.Name);

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
  EffectIndex = 0;
  MaxEffectIndex = 0;

  EffectVertex = NULL;
  EffectDepth = NULL;

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
  RenderRawZ = false;
  RenderBuf = 0;
  RenderCnd = 0;
  RenderFmt = D3DFMT_UNKNOWN;
#endif
}

EffectManager::~EffectManager() {
  Singleton = NULL;

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
  if (RenderBuf & EFFECTBUF_ZBUF) if (RenderRawZ)
				  CurrDS.Release();
  if (RenderBuf & EFFECTBUF_COPY) CopyRT.Release();
  if (RenderBuf & EFFECTBUF_PAST) PastRT.Release();
  if (RenderBuf & EFFECTBUF_PREV) PrevRT.Release();
                                  LastRT.Release();
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

bool EffectManager::SetRAWZ(bool enabled) {
  RenderRawZ = enabled;

  if (RenderRawZ && !EffectDepth) {
    EffectDepth = new EffectRecord();

    if (!EffectDepth->LoadEffect("RAWZfix.fx", 0xFF000000)) {
      delete EffectDepth;
      EffectDepth = NULL;

      _MESSAGE("ERROR - RAWZfix.fx is missing and required! Please reinstall OBGEv2.2");
      return false;
    }

    EffectDepth->CompileEffect();
  }
  else if (!RenderRawZ && EffectDepth) {
    CurrDS.Release();

    delete EffectDepth;
    EffectDepth = NULL;
  }

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
  int bitz = BufferRawZDepthNumBits.Get();
  int bits = BufferTexturesNumBits.Get();

  if (RenderFmt == D3DFMT_UNKNOWN) {
    D3DFORMAT frmt = D3DFMT_UNKNOWN;
    D3DDISPLAYMODE d3ddm;
    lastOBGEDirect3D9->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &d3ddm);

    while (bits != 0) {
      /**/ if (bits >=  32) frmt = D3DFMT_A32B32G32R32F;
      else if (bits >=  16) frmt = D3DFMT_A16B16G16R16F;
      else if (bits >=   8) frmt = D3DFMT_A8R8G8B8;
      else if (bits <= -16) frmt = D3DFMT_A16B16G16R16;
      else if (bits <= -10) frmt = D3DFMT_A2B10G10R10;
      else if (bits <=  -8) frmt = D3DFMT_A8R8G8B8;
      else if (bits <=  -5) frmt = D3DFMT_A1R5G5B5;
      else if (bits <=  -4) frmt = D3DFMT_A4R4G4B4;
      else if (bits <=  -3) frmt = D3DFMT_R3G3B2;
      else                  frmt = D3DFMT_UNKNOWN;

      if (frmt != D3DFMT_UNKNOWN) {
	HRESULT hr;
	if ((hr = lastOBGEDirect3D9->CheckDeviceFormat(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, d3ddm.Format, D3DUSAGE_RENDERTARGET, D3DRTYPE_SURFACE, frmt)) == D3D_OK)
	  break;
      }

      /**/ if (bits >=  32) bits = 16;
      else if (bits >=  16) bits = 8;
      else if (bits >=   8) bits = 0;
      else if (bits <= -16) bits = -10;
      else if (bits <= -10) bits = -8;
      else if (bits <=  -8) bits = -5;
      else if (bits <=  -5) bits = -4;
      else if (bits <=  -4) bits = -3;
      else if (bits <=  -3) bits = 0;
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

    if (RenderBuf & EFFECTBUF_ZBUF)
      if (!RenderRawZ)		    CurrDS.Initialise(GetDepthBufferTexture());
      else			    OrigDS.Initialise(GetDepthBufferTexture()),
	  			    CurrDS.Initialise(bitz == 32 ? D3DFMT_R32F : D3DFMT_R16F);
    if (RenderBuf & EFFECTBUF_COPY) CopyRT.Initialise(RenderFmt);
    if (RenderBuf & EFFECTBUF_PAST) PastRT.Initialise(RenderFmt);
    if (RenderBuf & EFFECTBUF_PREV) PrevRT.Initialise(RenderFmt);
				    LastRT.Initialise(RenderFmt);
  }
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
  if (RenderBuf & EFFECTBUF_ZBUF) if (RenderRawZ)
				  CurrDS.Release();
  if (RenderBuf & EFFECTBUF_PAST) PastRT.Release();
  if (RenderBuf & EFFECTBUF_PREV) PrevRT.Release();
				  LastRT.Release();
#endif
}

void EffectManager::InitialiseBuffers() {
  float minx, minu, uadj, vadj;
  void *VertexPointer;

  const float W = (float)v1_2_416::GetRenderer()->SizeWidth;
  const float H = (float)v1_2_416::GetRenderer()->SizeHeight;

  EffectConst.rcpres[0] = 1.0f / W;
  EffectConst.rcpres[1] = 1.0f / H;
  EffectConst.rcpres[2] = W / H;
  EffectConst.rcpres[3] = W * H;

  uadj = EffectConst.rcpres[0] * 0.5;
  vadj = EffectConst.rcpres[1] * 0.5;

  if (SplitScreen.data) {
    minx = 0;
    minu = 0.5;
  }
  else {
    minx = -1;
    minu = 0;
  }

  D3D_sShaderVertex ShaderVertices[] = {
    {minx , +1 , 1, minu + uadj , 0 + vadj},
    {minx , -1 , 1, minu + uadj , 1 + vadj},
    {1    , +1 , 1, 1    + uadj , 0 + vadj},
    {1    , -1 , 1, 1    + uadj , 1 + vadj}
  };

  _MESSAGE("Creating effect vertex buffers.");

  GetD3DDevice()->CreateVertexBuffer(4 * sizeof(D3D_sShaderVertex), D3DUSAGE_WRITEONLY, MYVERTEXFORMAT, D3DPOOL_DEFAULT, &EffectVertex, 0);
  EffectVertex->Lock(0, 0, &VertexPointer, 0);
  CopyMemory(VertexPointer, ShaderVertices, sizeof(ShaderVertices));
  EffectVertex->Unlock();

  EffectConst.bHasDepth = ::HasDepth();
}

void EffectManager::ReleaseBuffers() {
  if (EffectVertex) {
    _MESSAGE("Releasing effect vertex buffer.");

    while (EffectVertex->Release()) {}
    EffectVertex = NULL;
  }
}

void EffectManager::OnReleaseDevice() {
  ReleaseBuffers();
  ReleaseFrameTextures();

  ManagedEffectList::iterator SEffect = ManagedEffects.begin();

  while (SEffect != ManagedEffects.end()) {
    delete (*SEffect);
    *SEffect = NULL;
    SEffect++;
  }

  Reset();

  if (EffectDepth) {
    delete EffectDepth;
    EffectDepth = NULL;
  }
}

void EffectManager::OnLostDevice() {
  ReleaseBuffers();
  ReleaseFrameTextures();

  ManagedEffectList::iterator SEffect = ManagedEffects.begin();

  while (SEffect != ManagedEffects.end()) {
    (*SEffect)->OnLostDevice();
    SEffect++;
  }

  if (EffectDepth)
    EffectDepth->OnLostDevice();
}

void EffectManager::OnResetDevice() {
  ReleaseBuffers();
  ReleaseFrameTextures();

  InitialiseBuffers();
  InitialiseFrameTextures();

  ManagedEffectList::iterator SEffect = ManagedEffects.begin();

  while (SEffect != ManagedEffects.end()) {
    (*SEffect)->OnResetDevice();
    (*SEffect)->ApplyPermanents(&EffectConst, this);

    SEffect++;
  }

  if (EffectDepth)
    EffectDepth->OnResetDevice();
}

void EffectManager::UpdateFrameConstants(v1_2_416::NiDX9Renderer *Renderer) {
  OBGEfork::Sun *pSun = OBGEfork::Sky::GetSingleton()->sun;
  float (_cdecl * GetTimer)(bool, bool) = (float( *)(bool, bool))0x0043F490; // (TimePassed,GameTime)
  v1_2_416::NiCamera **pMainCamera = (v1_2_416::NiCamera **)0x00B43124;
  char *CamName;

  Renderer->SetCameraViewProj(*pMainCamera);
  D3DXMatrixTranslation(&EffectConst.world,
			-(*pMainCamera)->m_worldTranslate.x,
			-(*pMainCamera)->m_worldTranslate.y,
			-(*pMainCamera)->m_worldTranslate.z);

  EffectConst.view = (D3DXMATRIX)Renderer->m44View;
  EffectConst.proj = (D3DXMATRIX)Renderer->m44Projection;

  CamName = (*pMainCamera)->m_pcName;
  (*pMainCamera)->m_worldRotate.GetForwardVector(&EffectConst.EyeForward);

  EffectConst.ZRange.y = (EffectConst.proj._43 / EffectConst.proj._33);
  EffectConst.ZRange.x = (EffectConst.proj._33 * EffectConst.ZRange.y) / (EffectConst.proj._33 - 1.0f);
  EffectConst.ZRange.z = EffectConst.ZRange.x - EffectConst.ZRange.y;
  EffectConst.ZRange.w = EffectConst.ZRange.x + EffectConst.ZRange.y;

  EffectConst.GameTime.x = GetTimer(0, 1);
  EffectConst.GameTime.w = (EffectConst.GameTime.x     ) % 60;
  EffectConst.GameTime.z = (EffectConst.GameTime.x / 60) % 60;
  EffectConst.GameTime.y = (EffectConst.GameTime.x / 60) / 60;
  bool Between6and18 = (EffectConst.GameTime.y >= 6) && (EffectConst.GameTime.y < 18);

  /* deprecated */
#ifndef	NO_DEPRECATED
  EffectConst.time.x = GetTimer(0, 1);
  EffectConst.time.w = (int)(EffectConst.time.x     ) % 60;
  EffectConst.time.z = (int)(EffectConst.time.x / 60) % 60;
  EffectConst.time.y = (int)(EffectConst.time.x / 60) / 60;
#endif

  v1_2_416::NiNode *SunContainer = pSun->SunBillboard.Get()->ParentNode;
  bool SunHasBenCulled = SunContainer->m_flags.individual.AppCulled;
  EffectConst.SunDir.x = SunContainer->m_localTranslate.x;
  EffectConst.SunDir.y = SunContainer->m_localTranslate.y;
  EffectConst.SunDir.z = SunContainer->m_localTranslate.z;
  EffectConst.SunDir.Normalize3();

  RenderCnd = (RenderCnd & ~EFFECTCOND_ISDAY  ) | ( Between6and18                   ? EFFECTCOND_ISDAY   : 0);
  RenderCnd = (RenderCnd & ~EFFECTCOND_ISNIGHT) | (!Between6and18                   ? EFFECTCOND_ISNIGHT : 0);
  RenderCnd = (RenderCnd & ~EFFECTCOND_HASSUN ) | (!SunHasBenCulled                 ? EFFECTCOND_HASSUN  : 0);
  RenderCnd = (RenderCnd & ~EFFECTCOND_HASREFL) | (passTexture[OBGEPASS_REFLECTION] ? EFFECTCOND_HASREFL : 0);
  RenderCnd = (RenderCnd & ~EFFECTCOND_HASZBUF) | (CurrDS.IsValid()                 ? EFFECTCOND_HASZBUF : 0);
}

void EffectManager::Render(IDirect3DDevice9 *D3DDevice, IDirect3DSurface9 *RenderTo, IDirect3DSurface9 *RenderFrom) {
  v1_2_416::NiDX9Renderer *Renderer = v1_2_416::GetRenderer();

  UpdateFrameConstants(Renderer);

  D3DDevice->SetStreamSource(0, EffectVertex, 0, sizeof(D3D_sShaderVertex));
  Renderer->RenderStateManager->SetFVF(MYVERTEXFORMAT, false);

  // Sets up the viewport.
  float test[4] = { 0.0, 1.0, 1.0, 0.0 };
  Renderer->SetupScreenSpaceCamera(test);

  Renderer->RenderStateManager->SetRenderState(D3DRS_COLORWRITEENABLE, 0xF, false);
  Renderer->RenderStateManager->SetRenderState(D3DRS_ALPHABLENDENABLE, false, false);
  Renderer->RenderStateManager->SetRenderState(D3DRS_ZENABLE, D3DZB_FALSE, false);
  Renderer->RenderStateManager->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE, false);

  // Set up world/view/proj matrices to identity in case there's no vertex effect.
  D3DXMATRIX mIdent;
  D3DXMatrixIdentity(&mIdent);

  D3DDevice->SetTransform(D3DTS_PROJECTION, &mIdent);
  D3DDevice->SetTransform(D3DTS_VIEW, &mIdent);
  D3DDevice->SetTransform(D3DTS_WORLD, &mIdent);

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
      (*SEffect)->ApplyConstants(&EffectConst);
      (*SEffect)->ApplyDynamics();
      (*SEffect)->Render(D3DDevice, RenderTo, lastpassSurf);

      D3DDevice->StretchRect(RenderTo, 0, thisframeSurf, 0, D3DTEXF_NONE);
    }

    SEffect++;
  }

  D3DDevice->StretchRect(RenderTo, 0, lastframeSurf, 0, D3DTEXF_NONE);
#else
  /* over frames */
  OrigRT.Initialise(OBGEPASS_MAIN, RenderFrom);
  TrgtRT.Initialise(OBGEPASS_MAIN, RenderTo);

  /* rendertarget without texture, non-HDR & non-Bloom special case
   * this basically is the raw backbuffer I think
   */
  if (!TrgtRT.IsValid()) {
    RenderBuf |= EFFECTBUF_COPY;
    CopyRT.Initialise(RenderFmt);
  }

  D3DDevice->EndScene();

  if (RenderBuf & EFFECTBUF_ZBUF) if (RenderRawZ) {
    OrigDS.SetTexture("RAWZdepth", EffectDepth->GetEffect());
    CurrDS.SetRenderTarget(D3DDevice);

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

  /* count if something happens */
  int run = 0;

  for (ManagedEffectList::iterator e = ManagedEffects.begin(); e != ManagedEffects.end(); e++) {
    /* check if the conditions meet */
    unsigned long EffectCnd = (*e)->GetConditions();
    if ((RenderCnd & EffectCnd) == EffectCnd)
      run += (*e)->Render(D3DDevice, &EffectConst, &RenderQueue);
  }

  /* nothing happend */
  if (!run) TrgtRT.Copy(D3DDevice, &OrigRT);

  RenderQueue.End(
				   &TrgtRT
  );

  D3DDevice->BeginScene();
#endif
}

void EffectManager::RenderRAWZfix(IDirect3DDevice9 *D3DDevice, IDirect3DSurface9 *RenderTo) {
#ifdef	OLD_QUEUE
  if (!EffectDepth) {
    EffectDepth = new EffectRecord();

    if (!EffectDepth->LoadEffect("RAWZfix.fx", 0xFF000000)) {
      _MESSAGE("ERROR - RAWZfix.fx is missing! Please reinstall OBGEv2.2");
      return;
    }

    EffectDepth->CompileEffect();
    EffectDepth->GetEffect()->SetTexture("RAWZdepth", depthRAWZ);
  }

  EffectDepth->Render(D3DDevice, RenderTo, lastpassSurf);
#endif
}

int EffectManager::AddPrivateEffect(const char *Filename, UINT32 refID) {
  ManagedEffectRecord *NewEffect = new ManagedEffectRecord();

  if (!NewEffect->LoadEffect(Filename, refID, true) ||
      !NewEffect->CompileEffect()) {
    delete NewEffect;
    return -1;
  }

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

  while (SEffect != Effects.end()) {
    if (!SEffect->second->IsPrivate()) {
      if (!_stricmp(Filename, SEffect->second->GetName())/*&& ((pEffect->second->ParentRefID & 0xff000000) == (refID & 0xff000000))*/) {
	_MESSAGE("Loading effect that already exists. Returning index of existing effect.");

	SEffect->second->AddRef();
	return SEffect->first;
      }
    }

    SEffect++;
  }

  ManagedEffectRecord *NewEffect = new ManagedEffectRecord();

  if (!NewEffect->LoadEffect(Filename, refID, false) ||
      !NewEffect->CompileEffect()) {
    delete NewEffect;
    return -1;
  }

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

int EffectManager::AddDependentEffect(const char *Filename, UINT32 refID) {
  EffectRegistry::iterator SEffect = Effects.begin();

  while (SEffect != Effects.end()) {
    if (!_stricmp(Filename, SEffect->second->GetName())/*&& ((pEffect->second->ParentRefID & 0xff000000) == (refID & 0xff000000))*/) {
      _MESSAGE("Loading effect that already exists. Returning index of existing effect.");

      SEffect->second->AddRef();
      return SEffect->first;
    }

    SEffect++;
  }

  ManagedEffectRecord *NewEffect = new ManagedEffectRecord();

  if (!NewEffect->LoadEffect(Filename, refID, true) ||
      !NewEffect->CompileEffect()) {
    delete NewEffect;
    return -1;
  }

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

  /* update the buffers */
  InitialiseFrameTextures();

  for (ManagedEffectList::iterator e = ManagedEffects.begin(); e != ManagedEffects.end(); e++) {
    if ((*e)->IsEnabled()) {
      /* update */
      (*e)->ApplyPermanents(&EffectConst, this);
    }
  }
}

void EffectManager::LoadEffectList() {
  FILE *EffectFile;
  char EffectBuffer[260];
  int lastpos;

  if (UseShaderList.data) {
    _MESSAGE("Loading the effects.");

    if (!fopen_s(&EffectFile, ShaderListFile.Get(), "rt")) {
      while (!feof(EffectFile)) {
        if (fgets(EffectBuffer, 260, EffectFile)) {
          lastpos = strlen(EffectBuffer) - 1;

          if (EffectBuffer[lastpos] == 10 ||
	      EffectBuffer[lastpos] == 13)
            EffectBuffer[lastpos] = 0;

	  /* TODO: don't count this as ref */
          int EffectNum = AddManagedEffect(EffectBuffer, 0xFF000000);
          if (EffectNum != -1)
            GetEffect(EffectNum)->Enable(true);
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
    EffectRegistry::iterator pEffect = Effects.begin();

    /* delete terminally */
    while (pEffect != Effects.end()) {
      ManagedEffectRecord *OldEffect = pEffect->second;
      delete OldEffect;

      pEffect++;
    }

    /* redo everything */
    Reset();
  }
  else {
    EffectRegistry::iterator pEffect = Effects.begin();

    /* disable and remove all refs */
    while (pEffect != Effects.end()) {
      /* TODO ClrRef clears enabled as well, logical, no? */
      pEffect->second->Enable(false);
      pEffect->second->ClrRef();
      /* delete private shaders I guess */
      pEffect++;
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

  _MESSAGE("pEffect index = %d", EffectIndex);

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
	const char *Filepath = (*pEffect)->GetPath();
	const bool Enabled = (*pEffect)->IsEnabled();
	const UINT32 RefID = (*pEffect)->GetRefID();

        Interface->WriteRecord('SNUM', SHADERVERSION, &EffectNum, sizeof(EffectNum));
        Interface->WriteRecord('SPAT', SHADERVERSION, Filepath, strlen(Filepath) + 1);
        Interface->WriteRecord('SENB', SHADERVERSION, &Enabled, sizeof(Enabled));
        Interface->WriteRecord('SREF', SHADERVERSION, &RefID, sizeof(RefID));
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
  int EffectNum;
  char LoadFilepath[260];
  bool LoadEnabled;
  UInt32 LoadRefID;
  bool InUse;

  Interface->GetNextRecordInfo(&type, &version, &length);

  if (type == 'SIDX') {
    Interface->ReadRecordData(&EffectIndex, length);
    _MESSAGE("pEffect Index = %d", EffectIndex);
  }
  else {
    _MESSAGE("No effect data in save file.");
    return;
  }

  Interface->GetNextRecordInfo(&type, &version, &length);

  while (type != 'SEOF') {
    if (type == 'SNUM') {
      Interface->ReadRecordData(&LoadShaderNum, length);
      _MESSAGE("pEffect num = %d", LoadShaderNum);
    }
    else {
      _MESSAGE("Error loading effect list. type!=SNUM");
      return;
    }

    Interface->GetNextRecordInfo(&type, &version, &length);

    if (type == 'SPAT') {
      Interface->ReadRecordData(LoadFilepath, length);
      _MESSAGE("Filename = %s", LoadFilepath);
    }
    else {
      _MESSAGE("Error loading effect list. type!=SPAT");
      return;
    }

    Interface->GetNextRecordInfo(&type, &version, &length);

    if (type == 'SENB') {
      Interface->ReadRecordData(&LoadEnabled, length);
      _MESSAGE("Enabled = %d", LoadEnabled);
    }
    else {
      _MESSAGE("Error loading effect list. type!=SENB");
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
      _MESSAGE("Error loading effect list. type!=SREF");
      return;
    }

    if (InUse && ((EffectNum = AddManagedEffect(LoadFilepath, LoadRefID)) != -1)) {
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

	      pEffect->SetIntArray(IntData.Name, (int *)&IntData.data, IntData.size);
	      _MESSAGE("Int %s = %d(%d)", IntData.Name, IntData.data[0], IntData.size);
	      break;
	    case 'SFLT':
	      FloatType FloatData;

	      Interface->ReadRecordData(&FloatData, length);

	      pEffect->SetFloatArray(FloatData.Name, (float *)&FloatData.data, FloatData.size);
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
        Interface->ReadRecordData(&LoadFilepath, length);
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

void EffectManager::PurgeTexture(IDirect3DBaseTexture9 *texture) {
  ManagedEffectList::iterator pEffect = ManagedEffects.begin();

  while (pEffect != ManagedEffects.end()) {
    if (*pEffect) {
      D3DXEFFECT_DESC Description;
      ID3DXEffect *DX9Effect;

      DX9Effect = (*pEffect)->GetEffect();
      DX9Effect->GetDesc(&Description);

      for (int par = 0; par < Description.Parameters; par++) {
	D3DXHANDLE handle;

	if ((handle = DX9Effect->GetParameter(NULL, par))) {
	  D3DXPARAMETER_DESC Description;

	  DX9Effect->GetParameterDesc(handle, &Description);

	  if ((Description.Type = D3DXPT_TEXTURE) ||
	      (Description.Type = D3DXPT_TEXTURE1D) ||
	      (Description.Type = D3DXPT_TEXTURE2D) ||
	      (Description.Type = D3DXPT_TEXTURE3D) ||
	      (Description.Type = D3DXPT_TEXTURECUBE)) {
	    // NB must set to NULL otherwise strange things happen
	    IDirect3DBaseTexture9 *ShaderTexture = NULL;
	    DX9Effect->GetTexture(handle, &ShaderTexture);

	    if (ShaderTexture == texture) {
	      DX9Effect->SetTexture(handle, NULL);

	      _DMESSAGE("Removing texture %s from effect %s", Description.Name, (*pEffect)->GetPath());
	    }
	  }
	}
      }
    }

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
