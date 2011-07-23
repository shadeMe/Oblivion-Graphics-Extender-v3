#ifndef	OBGE_NOSHADER

#include <sys/stat.h>
#define _USE_MATH_DEFINES
#include <math.h>

#include <string>

#include "ShaderManager.h"
#include "TextureManager.h"
#include "GlobalSettings.h"

#include "D3D9.hpp"
#include "D3D9Device.hpp"

static global<bool> UseShaderOverride(true,NULL,"Shaders","bUseShaderOverride");
static global<bool> UseLegacyCompiler(false,NULL,"Shaders","bUseLegacyCompiler");
static global<bool> SaveShaderOverride(true,NULL,"Shaders","bSaveShaderOverride");
static global<bool> CompileSources(false,NULL,"Shaders","bCompileSources");
static global<bool> RuntimeSources(false,NULL,"Shaders","bRuntimeSources");
static global<bool> Optimize(false,NULL,"Shaders","bOptimize");
static global<bool> MaximumSM(false,NULL,"Shaders","bMaximumSM");
static global<bool> UpgradeSM(false,NULL,"Shaders","bUpgradeSM1X");
static global<char*> ShaderOverrideDirectory("data\\shaders\\override\\",NULL,"Shaders","sShaderOverrideDirectory");
static global<int> BufferZDepthNumBits(0, NULL, "ScreenBuffers", "iBufferZDepthNumBits");

/* #################################################################################################
 */

class HLSLIncludeManager : public ID3DXInclude
{
public:
    STDMETHOD(Open)(D3DXINCLUDE_TYPE IncludeType, LPCSTR pFileName, LPCVOID pParentData, LPCVOID *ppData, UINT *pBytes);
    STDMETHOD(Close)(LPCVOID pData);
};

HRESULT HLSLIncludeManager::Open(D3DXINCLUDE_TYPE IncludeType, LPCSTR pName, LPCVOID pParentData, LPCVOID *ppData, UINT *pBytes)
{
    struct stat s;
    FILE *f;
    char strFileFull[MAX_PATH];

    switch (IncludeType) {
      case D3DXINC_LOCAL: strcpy(strFileFull, ::ShaderOverrideDirectory.Get()); strcat(strFileFull, pName); break;
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

HRESULT HLSLIncludeManager::Close(LPCVOID pData)
{
    BYTE* pData2 = (BYTE*)pData;
    if (pData2)
      delete[] pData2;
    return S_OK;
}

static HLSLIncludeManager incl;

/* #################################################################################################
 */

#define	strprete(i)	#i
#define	stringify(i)	strprete(i)

#define	DEFAULT		0
#define	FUSED		1
#define	OFF		2
#define	LAZY		3
#define	GREEDY		4
#define	REFLECTIONPASS	(1 << 1) /*(1 << OBGEPASS_REFLECTION)*/
#define	WATERPASS	(1 << 2) /*(1 << OBGEPASS_WATER)*/
#define	SHADOWPASS	(1 << 5) /*(1 << OBGEPASS_SHADOW)*/
#define	MAINPASS	(1 << 6) /*(1 << OBGEPASS_MAIN)*/
#define	EFFECTPASS	(1 << 7) /*(1 << OBGEPASS_EFFECTS)*/

typedef struct _myD3DXMACRO
{
  const char *Name;
  char *Definition;

} myD3DXMACRO;

static char IN_RAWZ[] = "0";
static char *FILTER_LINEAR = "LINEAR";
static char *FILTER_ANISO  = "ANISOTROPIC";
static char  LEVEL_ANISO[4] = "1";

#define	DEFS_INRAWZ	0
#define	DEFS_MIP	1
#define	DEFS_MIN	2
#define	DEFS_MAG	3

static myD3DXMACRO defs[] = {
  {"IN_RAWZ"	        , IN_RAWZ},
  {"MIPDEF"	        , FILTER_LINEAR},
  {"MINDEF"	        , FILTER_LINEAR},
  {"MAGDEF"	        , FILTER_LINEAR},
  {"ANISODEF"	        , LEVEL_ANISO},

  {"FUSED"		, stringify(FUSED  )},
  {"OFF"		, stringify(OFF	   )},
  {"DEFAULT"		, stringify(DEFAULT)},
  {"LAZY"		, stringify(LAZY   )},
  {"GREEDY"		, stringify(GREEDY )},

  {"REFLECTIONPASS"	, stringify(REFLECTIONPASS)},
  {"WATERPASS"		, stringify(WATERPASS     )},
  {"SHADOWPASS"		, stringify(SHADOWPASS    )},
  {"MAINPASS"		, stringify(MAINPASS      )},
  {"EFFECTPASS"		, stringify(EFFECTPASS    )},

  {NULL, NULL},
};

#define SHADER_COLORWRITE	0
#define SHADER_COLORBUFFER	1
#define SHADER_ZWRITE		2
#define SHADER_ZBUFFER		3
#define SHADER_PASSMASK		4

/* #################################################################################################
 */

template<>
RuntimeVariable::mem::iv *ShaderManager::GetGlobalConst(const char *Name, int length, RuntimeVariable::mem::iv *vs) {
  assert(length == 1);

  /* no need to check if it exist */
  return (RuntimeVariable::mem::iv *)&GlobalConst.pInt4[Name].integer;
}

template<>
RuntimeVariable::mem::fv *ShaderManager::GetGlobalConst(const char *Name, int length, RuntimeVariable::mem::fv *vs) {
  assert(length == 1);

  /* no need to check if it exist */
  return (RuntimeVariable::mem::fv *)&GlobalConst.pFloat4[Name].floating;
}

template<>
bool ShaderManager::SetGlobalConst(const char *Name, int length, RuntimeVariable::mem::iv *vs) {
  assert(length == 1);

  /* no need to check if it exist */
  memcpy(&GlobalConst.pInt4[Name].integer, vs, sizeof(RuntimeVariable::mem::iv));

  return true;
}

template<>
bool ShaderManager::SetGlobalConst(const char *Name, int length, RuntimeVariable::mem::fv *vs) {
  assert(length == 1);

  /* no need to check if it exist */
  memcpy(&GlobalConst.pFloat4[Name].floating, vs, sizeof(RuntimeVariable::mem::fv));

  return true;
}

/* #################################################################################################
 */

ShaderRecord::ShaderRecord() {
  Filepath[0] = '\0';
  Name[0] = '\0';

  pAssociate = NULL;
  pOblivionBinary = NULL;
  pOblivionConTab = NULL;

  /* shader source, binaries and constant tables */
  pProfile = NULL;
  pSourceReplaced = NULL; sourceLen = 0; hlslStamp = 0;
  pAsmblyReplaced = NULL; asmblyLen = 0;
  pSrcPrcReplaced = NULL;
  pShaderReplaced = NULL;
  pConstsReplaced = NULL;

  pSourceRuntime = NULL; runtimeLen = 0;
  pSrcPrcRuntime = NULL;
  pShaderRuntime = NULL;
  pConstsRuntime = NULL;

  pShaderOriginal = NULL;
  pConstsOriginal = NULL;

  pErrorMsgs = NULL;
  pDisasmbly = NULL;

  /* shader class and the identifier for which of the binaries is selected */
  pDX9ShaderCoTa = NULL;
  pDX9ShaderClss = NULL;
  pDX9ShaderType = SHADER_UNSET;
  pDX9ShaderWant = SHADER_UNSET;
}

ShaderRecord::~ShaderRecord() {
  /* shader source, binaries and constant tables */
  if (pProfile) delete[] pProfile;

  if (pSourceReplaced) delete[] pSourceReplaced;
  if (pAsmblyReplaced) delete[] pAsmblyReplaced;
  if (pSrcPrcReplaced) pSrcPrcReplaced->Release();
  if (pShaderReplaced) pShaderReplaced->Release();
  if (pConstsReplaced) pConstsReplaced->Release();

  if (pSourceRuntime) delete[] pSourceRuntime;
  if (pSrcPrcRuntime) pSrcPrcRuntime->Release();
  if (pShaderRuntime) pShaderRuntime->Release();
  if (pConstsRuntime) pConstsRuntime->Release();

  if (pShaderOriginal) pShaderOriginal->Release();
  if (pConstsOriginal) pConstsOriginal->Release();

  if (pErrorMsgs) pErrorMsgs->Release();
  if (pDisasmbly) pDisasmbly->Release();

  /* shader class and the identifier for which of the binaries is selected */
  if (pDX9ShaderClss) pDX9ShaderClss->Release();
}

/* -------------------------------------------------------------------------------------------------
 */

bool ShaderRecord::ChangedShader() {
  struct stat sh;
  char strFileFull[MAX_PATH];

  /* getting a HLSL source for compiling if there is any
   */
  strcpy(strFileFull, ::ShaderOverrideDirectory.Get());
  strcat(strFileFull, Name);
  strcat(strFileFull, ".hlsl");
  if (!stat((const char *)strFileFull, &sh)) {
    /* if the HLSL source is newer than the binary, attempt to recompile
     */
    if (sh.st_mtime > hlslStamp) {
      return true;
    }
  }

  return false;
}

bool ShaderRecord::ReloadShader() {
  struct stat sh;
  char strFileFull[MAX_PATH];
  FILE *f;

  /* getting a HLSL source for compiling if there is any
   */
  strcpy(strFileFull, ::ShaderOverrideDirectory.Get());
  strcat(strFileFull, Name);
  strcat(strFileFull, ".hlsl");
  if (!stat((const char *)strFileFull, &sh)) {
    /* if the HLSL source is newer than the binary, attempt to recompile
     */
    if (pSourceRuntime && (sh.st_mtime > hlslStamp)) {
      /* trigger recompile */
      RuntimeFlush();

      delete[] pSourceRuntime;
      pSourceRuntime = NULL;
    }

    if (!pSourceRuntime) {
      UINT size = sh.st_size;
      pSourceRuntime = new CHAR[size + 1];
      if (pSourceRuntime != NULL) {
	/* reading in text-mode can yield any number of less characters */
	memset(pSourceRuntime, 0, size + 1);

	if (!fopen_s(&f, strFileFull, "rb"/*"rt"*/)) {
	  fread(pSourceRuntime, 1, size, f);
	  fclose(f);

	  _DMESSAGE("Loaded source of %s from %s", Name, Filepath);

	  runtimeLen = strlen(pSourceRuntime);
	  hlslStamp = sh.st_mtime;
	}
	else {
	  delete[] pSourceRuntime;

	  pSourceRuntime = NULL;
	  runtimeLen = 0;
	  hlslStamp = 0;
	}

	return true;
      }
    }
  }

  return false;
}

bool ShaderRecord::LoadShader(const char *Filename) {
  struct stat sb, sv, sa, sh;
  char strFileFull[MAX_PATH];
  FILE *f;

  strcpy(Name, Filename);

  /* getting a compiled binary for replacement if there is any
   */
  strcpy(Filepath, ::ShaderOverrideDirectory.Get());
  strcat(Filepath, Filename);
  if (!stat((const char *)Filepath, &sb)) {
    UINT size = sb.st_size;
    if (D3DXCreateBuffer(size, &pShaderReplaced) == D3D_OK) {
      if (!fopen_s(&f, Filepath, "rb")) {
	fread(pShaderReplaced->GetBufferPointer(), 1, size, f);
	fclose(f);

        _DMESSAGE("Loaded binary of %s from %s", Name, Filepath);

	if (D3DXGetShaderConstantTable(
	  (const DWORD *)pShaderReplaced->GetBufferPointer(),
	  &pConstsReplaced
	) != D3D_OK) {
	  pConstsReplaced = NULL;
	}
      }
      else {
	pShaderReplaced->Release();
	pShaderReplaced = NULL;
      }
    }
  }

  /* getting a configuration file (version, partial precision) for compiling
   * if there is any
   */
  strcpy(strFileFull, ::ShaderOverrideDirectory.Get());
  strcat(strFileFull, Filename);
  strcat(strFileFull, ".version");
  if (!::MaximumSM.Get() && !stat((const char *)strFileFull, &sv)) {
    UINT size = sv.st_size;
    pProfile = new CHAR[size + 1];
    if (pProfile != NULL) {
      /* reading in text-mode can yield any number of less characters */
      memset(pProfile, 0, size + 1);

      if (!fopen_s(&f, strFileFull, "rb"/*"rt"*/)) {
        fread(pProfile, 1, size, f);
        fclose(f);

        _DMESSAGE("Loaded version of %s from %s", Name, Filepath);

	// VERSION=vs_1_0
	char *ptr;
	if ((ptr = strrchr(pProfile, '=')))
	  memmove(pProfile, ptr + 1, strlen(ptr));
      }
      else {
      	delete[] pProfile;

      	pProfile = NULL;
      }
    }
  }

  /* getting a assembler source for compiling if there is any
   */
  strcpy(strFileFull, ::ShaderOverrideDirectory.Get());
  strcat(strFileFull, Filename);
  strcat(strFileFull, ".asm");
  if (!stat((const char *)strFileFull, &sa)) {
    UINT size = sa.st_size;
    pAsmblyReplaced = new CHAR[size + 1];
    if (pAsmblyReplaced != NULL) {
      /* reading in text-mode can yield any number of less characters */
      memset(pAsmblyReplaced, 0, size + 1);

      if (!fopen_s(&f, strFileFull, "rb"/*"rt"*/)) {
        fread(pAsmblyReplaced, 1, size, f);
        fclose(f);

        _DMESSAGE("Loaded assembly of %s from %s", Name, Filepath);

        asmblyLen = strlen(pAsmblyReplaced);
      }
      else {
      	delete[] pAsmblyReplaced;

      	pAsmblyReplaced = NULL;
        asmblyLen = 0;
      }
    }

    /* if the assembler source is newer than the binary, attempt to recompile
     */
    if (pAsmblyReplaced && (sa.st_mtime > sb.st_mtime)) {
      if (pSrcPrcReplaced) pSrcPrcReplaced->Release();
      if (pShaderReplaced) pShaderReplaced->Release();
      if (pConstsReplaced) pConstsReplaced->Release();

      pSrcPrcReplaced = NULL;
      pShaderReplaced = NULL;
      pConstsReplaced = NULL;

      assert(NULL);
    }
  }

  /* getting a HLSL source for compiling if there is any
   */
  strcpy(strFileFull, ::ShaderOverrideDirectory.Get());
  strcat(strFileFull, Filename);
  strcat(strFileFull, ".hlsl");
  if (!stat((const char *)strFileFull, &sh)) {
    UINT size = sh.st_size;
    pSourceReplaced = new CHAR[size + 1];
    if (pSourceReplaced != NULL) {
      /* reading in text-mode can yield any number of less characters */
      memset(pSourceReplaced, 0, size + 1);

      if (!fopen_s(&f, strFileFull, "rb"/*"rt"*/)) {
        fread(pSourceReplaced, 1, size, f);
        fclose(f);

        _DMESSAGE("Loaded source of %s from %s", Name, Filepath);

	sourceLen = strlen(pSourceReplaced);
	hlslStamp = sh.st_mtime;
      }
      else {
      	delete[] pSourceReplaced;

      	pSourceReplaced = NULL;
	sourceLen = 0;
	hlslStamp = 0;
      }
    }

    /* if the HLSL source is newer than the binary, attempt to recompile
     */
    if (pSourceReplaced && (sh.st_mtime > sb.st_mtime)) {
      if (pSrcPrcReplaced) pSrcPrcReplaced->Release();
      if (pShaderReplaced) pShaderReplaced->Release();
      if (pConstsReplaced) pConstsReplaced->Release();

      pSrcPrcReplaced = NULL;
      pShaderReplaced = NULL;
      pConstsReplaced = NULL;
    }
  }

  iType = SHADER_UNKNOWN;
  if (strstr(Name, ".vso"))
    iType = SHADER_VERTEX;
  else if (strstr(Name, ".pso"))
    iType = SHADER_PIXEL;

  /* automatically determine the highest shader-model
   */
  if (!pProfile && lastOBGEDirect3DDevice9) {
    if (pSourceReplaced && ::MaximumSM.Get())
      _DMESSAGE("Upgraded version of %s to max.", Name);

    if (iType == SHADER_VERTEX)
      pProfile = strdup(D3DXGetVertexShaderProfile(lastOBGEDirect3DDevice9));
    else if (iType == SHADER_PIXEL)
      pProfile = strdup(D3DXGetPixelShaderProfile(lastOBGEDirect3DDevice9));
  }

  /* get the stripped source
   */
  if (pSourceReplaced) {
    D3DXPreprocessShader(
	pSourceReplaced,
	sourceLen,
	(D3DXMACRO *)::defs,
	&incl,
	&pSrcPrcReplaced,
	&pErrorMsgs
    );
  }

  return true;
}

bool ShaderRecord::RuntimeFlush() {
  /* release all runtime resources (for example for recompilation) */
  if (pSrcPrcRuntime) pSrcPrcRuntime->Release();
  if (pShaderRuntime) pShaderRuntime->Release();
  if (pConstsRuntime) pConstsRuntime->Release();

  if (pErrorMsgs) pErrorMsgs->Release();
  if (pDisasmbly) pDisasmbly->Release();

  pSrcPrcRuntime = NULL;
  pShaderRuntime = NULL;
  pConstsRuntime = NULL;

  pErrorMsgs = NULL;
  pDisasmbly = NULL;

  return true;
}

bool ShaderRecord::CompareShader(const DWORD *Function) const {
  /* lookup if the given function is the on-disk shader binary */
  if (pShaderReplaced) {
    const DWORD *f = (const DWORD *)pShaderReplaced->GetBufferPointer();

    if (f == Function) {
      return true;
    }
  }

  /* lookup if the given function is the original shaderpack binary */
  if (pShaderOriginal) {
     const DWORD *f = (const DWORD *)pShaderOriginal->GetBufferPointer();

     if (f == Function) {
       return true;
     }
  }

  return false;
}

bool ShaderRecord::RuntimeShader(const char *hlsl, const char *version) {
  int size = strlen(hlsl);

  /* put a new source */
  if (pSourceRuntime) delete[] pSourceRuntime;
  pSourceRuntime = new CHAR[size + 1];
  strcpy(pSourceRuntime, hlsl);
  pSourceRuntime[size] = '\0';
  runtimeLen = size;

  /* put a new profile */
  if (version) {
    if (pProfile) free(pProfile);
    pProfile = strdup(version);
  }

  /* trigger recompile */
  RuntimeFlush();

  return true;
}

bool ShaderRecord::ConstructDX9Shader(char which) {
  /* turn over a DX9-ready shader class when runtime is enabled */
  LPD3DXBUFFER p = NULL;
  LPD3DXCONSTANTTABLE ct = NULL;
  const DWORD *pFunction = NULL, *pClassFunction = NULL;
  char choosen = SHADER_UNSET;

  /* cascade, the highest possible is selected */
  /**/ if (pShaderRuntime  && (which >= SHADER_RUNTIME))
    p = pShaderRuntime , ct = pConstsRuntime , choosen = SHADER_RUNTIME;
  else if (pShaderReplaced && (which >= SHADER_REPLACED))
    p = pShaderReplaced, ct = pConstsReplaced, choosen = SHADER_REPLACED;
  else if (pShaderOriginal && (which >= SHADER_ORIGINAL))
    p = pShaderOriginal, ct = pConstsOriginal, choosen = SHADER_ORIGINAL;
  if (p)
    pFunction = (const DWORD *)p->GetBufferPointer();

  /* store for automatic recreation */
  pDX9ShaderWant = which;

#if 0
  /* if the choosen one is the requested one, go ahead */
  if (pDX9ShaderClss) {
    UINT len = 0;
    HRESULT res = 0;

/* damit this are not pointers but function-contents, requires bytewise-compare which is not worth it!

    if (iType == SHADER_VERTEX)
      res = pDX9VertexShader->GetFunction(&pClassFunction, &len);
    else if (iType == SHADER_PIXEL)
      res = pDX9PixelShader->GetFunction(&pClassFunction, &len);

    /* same function */
    if ((res == D3D_OK) && (pClassFunction == pFunction))
      return true;
  }
#endif

  /* recreate DX9 shader class */
  if (pDX9ShaderClss)
    pDX9ShaderClss->Release();

  pDX9ShaderCoTa = ct;
  pDX9ShaderClss = NULL;
  pDX9ShaderType = choosen;

  if (pFunction && lastOBGEDirect3DDevice9) {
    if (iType == SHADER_VERTEX) {
      if (lastOBGEDirect3DDevice9->CreateVertexShader(pFunction, &pDX9VertexShader) != D3D_OK)
        pDX9ShaderClss = NULL;
    }
    else if (iType == SHADER_PIXEL) {
      if (lastOBGEDirect3DDevice9->CreatePixelShader(pFunction, &pDX9PixelShader) != D3D_OK)
        pDX9ShaderClss = NULL;
    }
  }

  return (pDX9ShaderClss != NULL);
}

DWORD *ShaderRecord::GetDX9ShaderTexture(const char *sName, int *TexNum, DWORD *States) {
  const char *src = NULL;

  /**/ if (pDX9ShaderClss && (pDX9ShaderType >= SHADER_RUNTIME))
    src = (const char *)pSrcPrcRuntime->GetBufferPointer();
  else if (pDX9ShaderClss && (pDX9ShaderType >= SHADER_REPLACED))
    src = (const char *)pSrcPrcReplaced->GetBufferPointer();
  else if (pDX9ShaderClss && (pDX9ShaderType >= SHADER_ORIGINAL))
    src = NULL;
  else if (pShaderRuntime  && (pOblivionBinary == (const DWORD *)pShaderRuntime->GetBufferPointer()))
    src = (const char *)pSrcPrcRuntime->GetBufferPointer();
  else if (pShaderReplaced && (pOblivionBinary == (const DWORD *)pShaderReplaced->GetBufferPointer()))
    src = (const char *)pSrcPrcReplaced->GetBufferPointer();
  else if (pShaderOriginal && (pOblivionBinary == (const DWORD *)pShaderOriginal->GetBufferPointer()))
    src = NULL;

  if (src && strstr(sName, "sampler")) {
    const char *key = "sampler";
    std::string tName(sName);
    std::string tPath(sName);

    tName.replace(tName.find(key), strlen(key), "tex");
    tPath.replace(tPath.find(key), strlen(key), "path");

    const char *sampler = strstr(src, sName);
    const char *texture = strstr(src, tName.c_str());
    const char *texpath = strstr(src, tPath.c_str());

    /* all different position */
    if (((texture != sampler) || !texture) &&
        ((texture != texpath) || !texpath) &&
        ((sampler != texpath))) {
      const char *samplert = sampler;
      while (--samplert > src)
	if (*samplert == ';')
	  break;

      const char *samplerb = (sampler  ? strchr(sampler     , '{') : NULL);
      const char *textureb = (texture  ? strchr(texture     , '"') : NULL);
      const char *texpathb = (texpath  ? strchr(texpath     , '"') : NULL);
      const char *samplere = (samplerb ? strchr(samplerb + 1, '}') : NULL);
      const char *texturee = (textureb ? strchr(textureb + 1, '"') : NULL);
      const char *texpathe = (texpathb ? strchr(texpathb + 1, '"') : NULL);

      // check for the sampler-type
      // check there is no ";" between the sampler and it's beginning block "{"
      // check there is no ";" between the texture and it's ending block ";"
      const char *sampleri = (samplert ? strstr(samplert, "sampler") : NULL);
      const char *samplerc = (sampler  ? strchr(sampler , ';') : NULL);
      const char *texturec = (texture  ? strchr(texture , ';') : NULL);
      const char *texpathc = (texpath  ? strchr(texpath , ';') : NULL);

      if ((sampleri <  sampler ) &&
      //  (samplerc >= samplerb) &&
	  (textureb || texpathb) &&
	  ((texturec >= texturee) ||
	   (texpathc >= texpathe))) {
      	char buf[256];
      	int len;

	if (textureb) {
	  len = texturee - (textureb + 1); if (len > 0)
	  strncpy(buf, textureb + 1, len);
	}
	else if (texpathb) {
	  len = texpathe - (texpathb + 1); if (len > 0)
	  strncpy(buf, texpathb + 1, len);
	}

	if (len > 0) {
	  buf[len] = '\0';

	  if (TexNum) {
	    TextureManager *TexMan = TextureManager::GetSingleton();

	    if (sampleri[7] == 'C') *TexNum = TexMan->LoadDependtTexture(buf, TR_CUBIC);
	    if (sampleri[7] == '3') *TexNum = TexMan->LoadDependtTexture(buf, TR_VOLUMETRIC);
	    if (sampleri[7] == '2') *TexNum = TexMan->LoadDependtTexture(buf, TR_PLANAR);
	    if (sampleri[7] == '1') *TexNum = TexMan->LoadDependtTexture(buf, TR_PLANAR);
	    if (sampleri[7] == ' ') *TexNum = TexMan->LoadDependtTexture(buf, TR_PLANAR);
	  }
	}
      }

      if ((sampleri <  sampler ) &&
	  (samplerc >= samplerb) &&
	  (samplere >  samplerb)) {
	char buf[1024];
	int len;

	len = samplere - (samplerb + 1); if (len > 0)
	strncpy(buf, samplerb + 1, len);

	if (len > 0) {
	  buf[len] = '\0';

	  const char *au = strstr(buf, "AddressU");
	  const char *av = strstr(buf, "AddressV");
	  const char *aw = strstr(buf, "AddressW");
	  const char *mp = strstr(buf, "MIPFILTER");
	  const char *mn = strstr(buf, "MINFILTER");
	  const char *mg = strstr(buf, "MAGFILTER");
	  const char *bc = strstr(buf, "Bordercolor");
	  const char *af = strstr(buf, "MaxAnisotropy");
#if	defined(OBGE_GAMMACORRECTION)
	  const char *dg = strstr(buf, "sRGBTexture");
#endif

	  if (au) {
	    char *end = (char *)strchr(au, ';'); if (end) *end = '\0';
	    /**/ if (strstr(au, "WRAP"      )) { *States++ = D3DSAMP_ADDRESSU; *States++ = D3DTADDRESS_WRAP; }
	    else if (strstr(au, "MIRROR"    )) { *States++ = D3DSAMP_ADDRESSU; *States++ = D3DTADDRESS_MIRROR; }
	    else if (strstr(au, "CLAMP"     )) { *States++ = D3DSAMP_ADDRESSU; *States++ = D3DTADDRESS_CLAMP; }
	    else if (strstr(au, "BORDER"    )) { *States++ = D3DSAMP_ADDRESSU; *States++ = D3DTADDRESS_BORDER; }
	    else if (strstr(au, "MIRRORONCE")) { *States++ = D3DSAMP_ADDRESSU; *States++ = D3DTADDRESS_MIRRORONCE; }
	  }
	  if (av) {
	    char *end = (char *)strchr(av, ';'); if (end) *end = '\0';
	    /**/ if (strstr(av, "WRAP"      )) { *States++ = D3DSAMP_ADDRESSV; *States++ = D3DTADDRESS_WRAP; }
	    else if (strstr(av, "MIRROR"    )) { *States++ = D3DSAMP_ADDRESSV; *States++ = D3DTADDRESS_MIRROR; }
	    else if (strstr(av, "CLAMP"     )) { *States++ = D3DSAMP_ADDRESSV; *States++ = D3DTADDRESS_CLAMP; }
	    else if (strstr(av, "BORDER"    )) { *States++ = D3DSAMP_ADDRESSV; *States++ = D3DTADDRESS_BORDER; }
	    else if (strstr(av, "MIRRORONCE")) { *States++ = D3DSAMP_ADDRESSV; *States++ = D3DTADDRESS_MIRRORONCE; }
	  }
	  if (aw) {
	    char *end = (char *)strchr(aw, ';'); if (end) *end = '\0';
	    /**/ if (strstr(aw, "WRAP"      )) { *States++ = D3DSAMP_ADDRESSW; *States++ = D3DTADDRESS_WRAP; }
	    else if (strstr(aw, "MIRROR"    )) { *States++ = D3DSAMP_ADDRESSW; *States++ = D3DTADDRESS_MIRROR; }
	    else if (strstr(aw, "CLAMP"     )) { *States++ = D3DSAMP_ADDRESSW; *States++ = D3DTADDRESS_CLAMP; }
	    else if (strstr(aw, "BORDER"    )) { *States++ = D3DSAMP_ADDRESSW; *States++ = D3DTADDRESS_BORDER; }
	    else if (strstr(aw, "MIRRORONCE")) { *States++ = D3DSAMP_ADDRESSW; *States++ = D3DTADDRESS_MIRRORONCE; }
	  }
	  if (mp) {
	    char *end = (char *)strchr(mp, ';'); if (end) *end = '\0';
	    /**/ if (strstr(mp, "NONE"         )) { *States++ = D3DSAMP_MIPFILTER; *States++ = D3DTEXF_NONE; }
	    else if (strstr(mp, "POINT"        )) { *States++ = D3DSAMP_MIPFILTER; *States++ = D3DTEXF_POINT; }
	    else if (strstr(mp, "LINEAR"       )) { *States++ = D3DSAMP_MIPFILTER; *States++ = D3DTEXF_LINEAR; }
	    else if (strstr(mp, "ANISOTROPIC"  )) { *States++ = D3DSAMP_MIPFILTER; *States++ = D3DTEXF_ANISOTROPIC; }
	    else if (strstr(mp, "PYRAMIDALQUAD")) { *States++ = D3DSAMP_MIPFILTER; *States++ = D3DTEXF_PYRAMIDALQUAD; }
	    else if (strstr(mp, "GAUSSIANQUAD" )) { *States++ = D3DSAMP_MIPFILTER; *States++ = D3DTEXF_GAUSSIANQUAD; }
	  }
	  if (mn) {
	    char *end = (char *)strchr(mn, ';'); if (end) *end = '\0';
	    /**/ if (strstr(mn, "NONE"         )) { *States++ = D3DSAMP_MINFILTER; *States++ = D3DTEXF_NONE; }
	    else if (strstr(mn, "POINT"        )) { *States++ = D3DSAMP_MINFILTER; *States++ = D3DTEXF_POINT; }
	    else if (strstr(mn, "LINEAR"       )) { *States++ = D3DSAMP_MINFILTER; *States++ = D3DTEXF_LINEAR; }
	    else if (strstr(mn, "ANISOTROPIC"  )) { *States++ = D3DSAMP_MINFILTER; *States++ = D3DTEXF_ANISOTROPIC; }
	    else if (strstr(mn, "PYRAMIDALQUAD")) { *States++ = D3DSAMP_MINFILTER; *States++ = D3DTEXF_PYRAMIDALQUAD; }
	    else if (strstr(mn, "GAUSSIANQUAD" )) { *States++ = D3DSAMP_MINFILTER; *States++ = D3DTEXF_GAUSSIANQUAD; }
	  }
	  if (mg) {
	    char *end = (char *)strchr(mg, ';'); if (end) *end = '\0';
	    /**/ if (strstr(mg, "NONE"         )) { *States++ = D3DSAMP_MAGFILTER; *States++ = D3DTEXF_NONE; }
	    else if (strstr(mg, "POINT"        )) { *States++ = D3DSAMP_MAGFILTER; *States++ = D3DTEXF_POINT; }
	    else if (strstr(mg, "LINEAR"       )) { *States++ = D3DSAMP_MAGFILTER; *States++ = D3DTEXF_LINEAR; }
	    else if (strstr(mg, "ANISOTROPIC"  )) { *States++ = D3DSAMP_MAGFILTER; *States++ = D3DTEXF_ANISOTROPIC; }
	    else if (strstr(mg, "PYRAMIDALQUAD")) { *States++ = D3DSAMP_MAGFILTER; *States++ = D3DTEXF_PYRAMIDALQUAD; }
	    else if (strstr(mg, "GAUSSIANQUAD" )) { *States++ = D3DSAMP_MAGFILTER; *States++ = D3DTEXF_GAUSSIANQUAD; }
	  }
	  if (bc) {
	    char *end = (char *)strchr(bc, ';'); if (end) *end = '\0';

	    /* currently fixed */
	    float col = 0.0; sscanf(bc, "Bordercolor = %f", &col);
	    *States++ = D3DSAMP_BORDERCOLOR;
	    *States++ = *((DWORD *)&col);
	  }
	  if (af) {
	    char *end = (char *)strchr(af, ';'); if (end) *end = '\0';

	    /* currently fixed */
	    int col = 0; sscanf(af, "MaxAnisotropy = %d", &col);
	    *States++ = D3DSAMP_MAXANISOTROPY;
	    *States++ = *((DWORD *)&col);
	  }
#if	defined(OBGE_GAMMACORRECTION)
	  if (dg) {
	    char *end = (char *)strchr(dg, ';'); if (end) *end = '\0';

	    /* currently fixed */
	    bool sRGB = false;
	    if (strstr(dg, "true"))
	      sRGB = true;
	    *States++ = D3DSAMP_SRGBTEXTURE;
	    *States++ = *((DWORD *)&sRGB);
	  }
#endif
	}
      }
    }
  }

  return States;
}

DWORD *ShaderRecord::GetDX9RenderStates(DWORD *States) {
  const char *src = NULL, *main;

  /**/ if (pDX9ShaderClss && (pDX9ShaderType >= SHADER_RUNTIME))
    src = (const char *)pSrcPrcRuntime->GetBufferPointer();
  else if (pDX9ShaderClss && (pDX9ShaderType >= SHADER_REPLACED))
    src = (const char *)pSrcPrcReplaced->GetBufferPointer();
  else if (pDX9ShaderClss && (pDX9ShaderType >= SHADER_ORIGINAL))
    src = NULL;
  else if (pShaderRuntime  && (pOblivionBinary == (const DWORD *)pShaderRuntime->GetBufferPointer()))
    src = (const char *)pSrcPrcRuntime->GetBufferPointer();
  else if (pShaderReplaced && (pOblivionBinary == (const DWORD *)pShaderReplaced->GetBufferPointer()))
    src = (const char *)pSrcPrcReplaced->GetBufferPointer();
  else if (pShaderOriginal && (pOblivionBinary == (const DWORD *)pShaderOriginal->GetBufferPointer()))
    src = NULL;

  if (src && (main = strstr(src, "main"))) {
    const char *maint = (main  ? strchr(main     , '{') : NULL);
    const char *mainb = (main  ? strchr(main     , '<') : NULL);
    const char *maine = (mainb ? strchr(mainb + 1, '>') : NULL);

    if ((mainb < maint) &&
	(maine > main )) {
      char buf[1024];
      int len;

      len = maine - (mainb + 1); if (len > 0)
      strncpy(buf, mainb + 1, len);

      if (len > 0) {
	buf[len] = '\0';

	const char *cw = strstr(buf, "ColorWrite");
	const char *cb = strstr(buf, "ColorBuffer");
	const char *zw = strstr(buf, "ZWrite");
	const char *zb = strstr(buf, "ZBuffer");
	const char *pm = strstr(buf, "PassMask");

	if (cb) {
	  char *end = (char *)strchr(cb, ';'); if (end) *end = '\0';
	  /**/ if (strstr(cb, "LAZY"         )) { *States++ = SHADER_COLORBUFFER; *States++ = LAZY; }
	  else if (strstr(cb, "GREEDY"       )) { *States++ = SHADER_COLORBUFFER; *States++ = GREEDY; }
	  else if (strstr(cb, "DEFAULT"      )) { *States++ = SHADER_COLORBUFFER; *States++ = DEFAULT; }
	}
	if (cw) {
	  char *end = (char *)strchr(cw, ';'); if (end) *end = '\0';
	  /**/ if (strstr(cw, "FUSED"        )) { *States++ = SHADER_COLORWRITE; *States++ = FUSED; }
	  else if (strstr(cw, "OFF"          )) { *States++ = SHADER_COLORWRITE; *States++ = OFF; }
	  else if (strstr(cw, "DEFAULT"      )) { *States++ = SHADER_COLORWRITE; *States++ = DEFAULT; }
	}
	if (zb) {
	  char *end = (char *)strchr(zb, ';'); if (end) *end = '\0';
	  /**/ if (strstr(zb, "LAZY"         )) { *States++ = SHADER_ZBUFFER; *States++ = LAZY; }
	  else if (strstr(zb, "GREEDY"       )) { *States++ = SHADER_ZBUFFER; *States++ = GREEDY; }
	  else if (strstr(zb, "DEFAULT"      )) { *States++ = SHADER_ZBUFFER; *States++ = DEFAULT; }
	}
	if (zw) {
	  char *end = (char *)strchr(zw, ';'); if (end) *end = '\0';
	  /**/ if (strstr(zw, "FUSED"        )) { *States++ = SHADER_ZWRITE; *States++ = FUSED; }
	  else if (strstr(zw, "OFF"          )) { *States++ = SHADER_ZWRITE; *States++ = OFF; }
	  else if (strstr(zw, "DEFAULT"      )) { *States++ = SHADER_ZWRITE; *States++ = DEFAULT; }
	}
	if (pm) {
	  char *end = (char *)strchr(pm, ';'); if (end) *end = '\0';
	  unsigned long mask = 0;
	  /**/ if (strstr(pm, "REFLECTIONPASS")) { mask |= REFLECTIONPASS; }
	  else if (strstr(pm, "WATERPASS"     )) { mask |= WATERPASS; }
	  else if (strstr(pm, "SHADOWPASS"    )) { mask |= SHADOWPASS; }
	  else if (strstr(pm, "MAINPASS"      )) { mask |= MAINPASS; }
	  else if (strstr(pm, "EFFECTPASS"    )) { mask |= EFFECTPASS; }
	  *States++ = SHADER_PASSMASK; *States++ = mask;
	}
      }
    }
  }

  return States;
}

bool ShaderRecord::DestroyDX9Shader() {
  if (pDX9ShaderClss)
    pDX9ShaderClss->Release();

  pDX9ShaderCoTa = NULL;
  pDX9ShaderClss = NULL;
  pDX9ShaderType = SHADER_UNSET;

  return true;
}

bool ShaderRecord::SaveShader() {
  /* destroy the runtime-class for recreation */
  DestroyDX9Shader();

  /* save the binary to disk if desired */
  if (!::SaveShaderOverride.Get())
    return false;

  LPD3DXBUFFER p = NULL;

  /* cascade, the highest possible is selected */
  if (pShaderRuntime)
    p = pShaderRuntime;
  else if (pShaderReplaced)
    p = pShaderReplaced;
  else if (pShaderOriginal)
    p = pShaderOriginal;

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

bool ShaderRecord::AssembleShader(bool forced) {
  /* nobody wants the automatic recompile */
  if (!::CompileSources.Get() && !forced)
    return false;

  LPSTR src = NULL; int len;
  LPD3DXBUFFER p = NULL;
  LPD3DXCONSTANTTABLE c = NULL;
  bool save = false;

  /* cascade, the highest possible is selected */
  if (pAsmblyReplaced) {
    src = pAsmblyReplaced;
    len = asmblyLen;

    p = pShaderReplaced;
    c = pConstsReplaced;
  }

  /* recompile only, if there is one already, just ignore */
  if (!p && src) {
    if (pErrorMsgs)
    pErrorMsgs->Release();
    pErrorMsgs = NULL;

    D3DXAssembleShader(
      src,
      len,
      NULL,
      &incl,
      D3DXSHADER_DEBUG,
      &p,
      &pErrorMsgs
    );

    /* this didn't go so well */
    if (pErrorMsgs) {
      _MESSAGE("Shader assembling messages occured in %s:", Filepath);
      _MESSAGE((char *)pErrorMsgs->GetBufferPointer());

      save = !strstr((char *)pErrorMsgs->GetBufferPointer(), "error");
    }
    else
      save = true;
  }

  /* request the constant table seperatly */
  if (!c && p) {
    D3DXGetShaderConstantTable(
      (const DWORD *)p->GetBufferPointer(),
      &c
    );
  }

  /* cascade, the highest possible is selected */
  if (pAsmblyReplaced) {
    pShaderReplaced = p;
    pConstsReplaced = c;
  }

  /* auto-save or not */
  if (save)
    SaveShader();

  return (pAsmblyReplaced && (pShaderReplaced != NULL));
}

bool ShaderRecord::CompileShader(bool forced) {
  /* nobody wants the automatic recompile */
  if (!::CompileSources.Get() && !forced)
    return false;

  D3DXMACRO *defs = (D3DXMACRO *)::defs;
  LPSTR src = NULL; int len;
  LPD3DXBUFFER p = NULL;
  LPD3DXBUFFER s = NULL;
  LPD3DXCONSTANTTABLE c = NULL;
  bool save = false;

  /* cascade, the highest possible is selected */
  if (pSourceRuntime) {
    src = pSourceRuntime;
    len = runtimeLen;

    s = pSrcPrcRuntime;
    p = pShaderRuntime;
    c = pConstsRuntime;
  }
  else if (pSourceReplaced) {
    src = pSourceReplaced;
    len = sourceLen;

    s = pSrcPrcReplaced;
    p = pShaderReplaced;
    c = pConstsReplaced;
  }

  /* recompile only, if there is one already, just ignore */
  if (!p && src) {
    if (pErrorMsgs)
    pErrorMsgs->Release();
    pErrorMsgs = NULL;

    D3DXPreprocessShader(
	src,
	len,
	defs,
	&incl,
	&s,
	&pErrorMsgs
    );

    D3DXCompileShader(
	src,
	len,
	defs,
	&incl,
	"main",
	(pProfile ? pProfile : (iType == SHADER_VERTEX ? "vs_3_0" : "ps_3_0")),
	D3DXSHADER_DEBUG | (
	  ::UseLegacyCompiler.Get() ? D3DXSHADER_USE_LEGACY_D3DX9_31_DLL : (
	  ::Optimize.Get()          ? D3DXSHADER_OPTIMIZATION_LEVEL3 : 0)),
	&p,
	&pErrorMsgs,
	&c
    );

    /* this didn't go so well, if it's a legacy "error", just try again */
    if (pErrorMsgs && strstr((char*)pErrorMsgs->GetBufferPointer(), "X3539")) {
      pErrorMsgs->Release();
      pErrorMsgs = NULL;

      D3DXCompileShader(
	  src,
	  len,
	  defs,
	  &incl,
	  "main",
	  (pProfile ? pProfile : (iType == SHADER_VERTEX ? "vs_3_0" : "ps_3_0")),
	  D3DXSHADER_DEBUG | (
	    ::UpgradeSM.Get() ? D3DXSHADER_ENABLE_BACKWARDS_COMPATIBILITY : D3DXSHADER_USE_LEGACY_D3DX9_31_DLL),
	  &p,
	  &pErrorMsgs,
	  &c
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
  if (pSourceRuntime) {
    pSrcPrcRuntime = s;
    pShaderRuntime = p;
    pConstsRuntime = c;
  }
  else if (pSourceReplaced) {
    pSrcPrcReplaced = s;
    pShaderReplaced = p;
    pConstsReplaced = c;
  }

  /* auto-save or not */
  if (save)
    SaveShader();

  return (pSourceRuntime  && (pSrcPrcRuntime  != NULL) && (pShaderRuntime  != NULL)) ||
	 (pSourceReplaced && (pSrcPrcReplaced != NULL) && (pShaderReplaced != NULL));
}

bool ShaderRecord::DisassembleShader(bool forced) {
  bool succ = true;

  /* trigger an automatic (re)compile if necessary */
  if (pAsmblyReplaced)
    succ = AssembleShader(forced);
  else if (pSourceRuntime || pSourceReplaced)
    succ = CompileShader(forced);

  if (succ) {
    if (pDisasmbly)
    pDisasmbly->Release();
    pDisasmbly = NULL;

    /* cascade, the highest possible is selected */
    if (pSourceRuntime) { if (pShaderRuntime)
      D3DXDisassembleShader(
	(const DWORD *)pShaderRuntime->GetBufferPointer(),
	FALSE,
	NULL,
	&pDisasmbly
      );
    }
    else if (pSourceReplaced) { if (pShaderReplaced)
      D3DXDisassembleShader(
	(const DWORD *)pShaderReplaced->GetBufferPointer(),
	FALSE,
	NULL,
	&pDisasmbly
      );
    }
    else if (pShaderOriginal) {
      D3DXDisassembleShader(
	(const DWORD *)pShaderOriginal->GetBufferPointer(),
	FALSE,
	NULL,
	&pDisasmbly
      );
    }
  }

  return (pDisasmbly != NULL);
}

/* -------------------------------------------------------------------------------------------------
 */

void ShaderRecord::SetBinary(int len, const DWORD *org) {
  /* called from within Oblivion we record the original resource */
  if (pShaderOriginal) pShaderOriginal->Release();
  if (pConstsOriginal) pConstsOriginal->Release();

  pShaderOriginal = NULL;
  pConstsOriginal = NULL;

  /* analyze Oblivion's binary */
  if (D3DXCreateBuffer(len, &pShaderOriginal) == D3D_OK) {
    memcpy(pShaderOriginal->GetBufferPointer(), org, len);

    if (D3DXGetShaderConstantTable(org, &pConstsOriginal) != D3D_OK) {
      pConstsOriginal = NULL;
    }
  }
}

const DWORD *ShaderRecord::GetBinary() {
  /* called from within Oblivion we takeover the original resource */
  bool succ = true;

  /* trigger an automatic (re)compile if necessary */
  if (pAsmblyReplaced)
    succ = AssembleShader();
  else if (pSourceReplaced)
    succ = CompileShader();

  /* cascade, the highest possible is selected */
  if (pShaderReplaced) {
    pOblivionBinary = (const DWORD *)pShaderReplaced->GetBufferPointer();
    pOblivionConTab =                pConstsReplaced;
  }
  else if (pShaderOriginal) {
    pOblivionBinary = (const DWORD *)pShaderOriginal->GetBufferPointer();
    pOblivionConTab =                pConstsOriginal;
  }
  else {
    pOblivionBinary = NULL;
    pOblivionConTab = NULL;
  }

  return pOblivionBinary;
}

/* #################################################################################################
 */

RuntimeShaderRecord::RuntimeShaderRecord() {
  pAssociate = NULL;
  pFunction = NULL;
  pShader = NULL;
  bActive = bMark = false;

  pCustomCT = NULL;
  pBool     = NULL;
  pInt4     = NULL;
  pFloat4   = NULL;
  pTexture  = NULL;

  pCopyRT = NULL; bCFused = true; bCLazy = true;
  pCopyDS = NULL; bDFused = true; bDLazy = true;
  pCopyDZ = NULL; bZFused = true; bZLazy = true;

  bIO = false;
  bMask = 0;

#ifdef	OBGE_DEVLING
  Paired.clear();

  Clear();
  memset(frame_used, -1, sizeof(frame_used));
#endif
}

RuntimeShaderRecord::~RuntimeShaderRecord() {
  /* first release (text points into pCustomCT) */
//rsb.Release();

  if (pCustomCT)
    free(pCustomCT);

  /* release previous texture */
  TextureManager *TexMan = TextureManager::GetSingleton();
  std::vector<int>::iterator PTexture = Textures.begin();

  while (PTexture != Textures.end()) {
    TexMan->ReleaseTexture(*PTexture);
    PTexture++;
  }
}

void RuntimeShaderRecord::Release() {
  if (pCustomCT) {
    free(pCustomCT);
    pCustomCT = NULL;
  }

  pBool    = NULL;
  pInt4    = NULL;
  pFloat4  = NULL;
  pTexture = NULL;

  pCopyRT = NULL; bCFused = true; bCLazy = true;
  pCopyDS = NULL; bDFused = true; bDLazy = true;
  pCopyDZ = NULL; bZFused = true; bZLazy = true;

  bIO = false;
  bMask = false;
}

inline void RuntimeShaderRecord::OnLostDevice(void) {
  if (pAssociate)
    pAssociate->DestroyDX9Shader();
}

inline void RuntimeShaderRecord::OnResetDevice(void) {
  if (pAssociate) {
    pAssociate->DestroyDX9Shader();
    ActivateShader(pAssociate->pDX9ShaderWant);
  }
}

/* -------------------------------------------------------------------------------------------------
 */
struct RuntimeShaderRecord::Buffers RuntimeShaderRecord::rsb[OBGEPASS_NUM] = {NULL};

void RuntimeShaderRecord::Buffers::Release() {
  /* first release (text points into pCustomCT)
  if (pGrabRT) pGrabRT->Release(); pGrabRT = NULL;
  if (pTextRT) pTextRT->Release(); pTextRT = NULL;
  if (pGrabDS) pGrabDS->Release(); pGrabDS = NULL;
  if (pTextDS) pTextDS->Release(); pTextDS = NULL;

  if (pGrabVX) {
    _MESSAGE("Releasing shader vertex buffer.");

    while (pGrabVX->Release()) {}
    pGrabVX = NULL;
  }
   */
}

void RuntimeShaderRecord::Buffers::GrabRT(IDirect3DDevice9 *StateDevice, IDirect3DDevice9 *SceneDevice) {
  IDirect3DSurface9 *pCurrRT;
  if ((pCurrRT = passSurface[OBGEPASS_ANY])) {
    D3DSURFACE_DESC CurrD;              pCurrRT->GetDesc(&CurrD);
    D3DSURFACE_DESC GrabD; if (pGrabRT) pGrabRT->GetDesc(&GrabD);

    /* different */
    if (!pTextRT || memcmp(&CurrD, &GrabD, sizeof(D3DSURFACE_DESC))) {
      if (pGrabRT) pGrabRT->Release();
      if (pTextRT) pTextRT->Release();

      pGrabRT = NULL; pTextRT = NULL;
      if (StateDevice->CreateTexture(CurrD.Width, CurrD.Height, 1, CurrD.Usage, CurrD.Format, CurrD.Pool, &pTextRT, NULL) == D3D_OK)
	pTextRT->GetSurfaceLevel(0, &pGrabRT);
    }

    if (pGrabRT) {
      minStretchRect(SceneDevice, pCurrRT, NULL, pGrabRT, NULL, D3DTEXF_NONE);
      bCFilled = true;
    }
  }
}

void RuntimeShaderRecord::Buffers::GrabDS(IDirect3DDevice9 *StateDevice, IDirect3DDevice9 *SceneDevice) {
#if 0
  IDirect3DSurface9 *pCurrDS;
  if ((pCurrDS = passDepth[OBGEPASS_ANY])) {
    D3DSURFACE_DESC CurrD;              pCurrDS->GetDesc(&CurrD);
    D3DSURFACE_DESC GrabD; if (pGrabDS) pGrabDS->GetDesc(&GrabD);

    /* different */
    if (!pTextDS || memcmp(&CurrD, &GrabD, sizeof(D3DSURFACE_DESC))) {
      if (pGrabDS) pGrabDS->Release();
      if (pTextDS) pTextDS->Release();

      pGrabDS = NULL; pTextDS = NULL;
      if (StateDevice->CreateTexture(CurrD.Width, CurrD.Height, 1, CurrD.Usage, CurrD.Format, CurrD.Pool, &pTextDS, NULL) == D3D_OK)
	pTextDS->GetSurfaceLevel(0, &pGrabDS);
    }

    if (pGrabDS) {
      minStretchRect(SceneDevice, pCurrDS, NULL, pGrabDS, NULL, D3DTEXF_NONE);
      bDFilled = true;
    }
  }
#endif

  IDirect3DTexture9 *pCurrDT;
  if ((pCurrDT = ResolvableDepthBuffer(passDepth[OBGEPASS_ANY], passDepthT[OBGEPASS_ANY]))) {
    D3DSURFACE_DESC CurrD;              pCurrDT->GetLevelDesc(0, &CurrD);
    D3DSURFACE_DESC GrabD; if (pTextDS) pTextDS->GetLevelDesc(0, &GrabD);

    /* different */
    if (!pTextDS || ((CurrD.Width != GrabD.Width) || (CurrD.Height != GrabD.Height))) {
      if (pGrabDS) pGrabDS->Release();
      if (pTextDS) pTextDS->Release();
      if (pGrabVX) pGrabVX->Release();

      pGrabDS = NULL; pTextDS = NULL; pGrabVX = NULL;
      if (SceneDevice->CreateVertexBuffer(4 * sizeof(CameraQuad), D3DUSAGE_WRITEONLY, CAMERAQUADFORMAT, D3DPOOL_DEFAULT, &pGrabVX, 0) == D3D_OK) {
	_MESSAGE("Creating shader vertex buffers.");

	void *VertexPointer = NULL;
	if (pGrabVX->Lock(0, 0, &VertexPointer, 0) == D3D_OK) {
	  /* pixel exact screen-quad */
	  const float width  = CurrD.Width  - 0.5f;
	  const float height = CurrD.Height - 0.5f;

	  CameraQuad ShaderVertices[] = {
	    {-0.5f,  -0.5f,  0.5f,  1.0f,	0.0f, 0.0f},
	    {width,  -0.5f,  0.5f,  1.0f,	1.0f, 0.0f},
	    {-0.5f, height,  0.5f,  1.0f,	0.0f, 1.0f},
	    {width, height,  0.5f,  1.0f,	1.0f, 1.0f}
	  };

	  CopyMemory(VertexPointer, ShaderVertices, sizeof(ShaderVertices));
	  pGrabVX->Unlock();
	}

	if (StateDevice->CreateTexture(
		CurrD.Width, CurrD.Height, 1,
		D3DUSAGE_RENDERTARGET /*CurrD.Usage*/,
		(BufferZDepthNumBits.Get() > 16
		? D3DFMT_R32F
		: D3DFMT_R16F) /*CurrD.Format*/,
		CurrD.Pool, &pTextDS, NULL) == D3D_OK) {
	  if (pTextDS->GetSurfaceLevel(0, &pGrabDS) != D3D_OK) {
	    /* big failure */
	    if (pGrabDS) pGrabDS->Release();
	    if (pTextDS) pTextDS->Release();
	    if (pGrabVX) pGrabVX->Release();

	    pGrabDS = NULL; pTextDS = NULL; pGrabVX = NULL;

	    _DMESSAGE("Creating grab buffers failed!");
	  }
	}
      }
      else
	_DMESSAGE("Creating shader vertex buffers failed!");
    }

    if (pGrabDS) {
      ShaderManager *sm = ShaderManager::GetSingleton();

#ifdef	OBGE_STATEBLOCKS
      /* auto backup */
      IDirect3DStateBlock9 *pStateBlock = NULL;
      SceneDevice->CreateStateBlock(D3DSBT_ALL, &pStateBlock);
#else
      /* manual backup */
//    IDirect3DSurface9 *pCurrRT;
      IDirect3DBaseTexture9 *pCurrTX;
      IDirect3DVertexShader9 *pCurrVS;
      IDirect3DPixelShader9 *pCurrPS;
//    DWORD dCurrZW;
      DWORD dCurrCL;
      DWORD dCurrAB;
//    DWORD dCurrAT;
//    DWORD dCurrST;
//    DWORD dCurrVF;
      IDirect3DVertexBuffer9 *pCurrVX;
      UINT32 dCurrVO;
      UINT32 dCurrVS;

#if 0
      IDirect3DVertexShader9 *pCopyVS;
      IDirect3DPixelShader9  *pCopyPS;

      pCopyVS = (IDirect3DVertexShader9 *)sm->cqv->pDX9ShaderClss;
      pCopyPS = (IDirect3DPixelShader9  *)sm->cqp->pDX9ShaderClss;
#endif

#if 0
      IDirect3DSurface9 *pCurrRT;
      IDirect3DSurface9 *pCurrDS;
      SceneDevice->GetRenderTarget(0, &pCurrRT);
      SceneDevice->GetDepthStencilSurface(&pCurrDS);
      assert(pCurrRT == passSurface[currentPass]);
      assert(pCurrDS == passDepth[currentPass]);

      struct textureSurface *t = surfaceTexture[passDepth[currentPass]];
#endif

      /* backup */
//    SceneDevice->GetRenderState(D3DRS_ZENABLE, &dCurrZW);
      SceneDevice->GetRenderState(D3DRS_CULLMODE, &dCurrCL);
      SceneDevice->GetRenderState(D3DRS_ALPHABLENDENABLE, &dCurrAB);
//    SceneDevice->GetRenderState(D3DRS_ALPHATESTENABLE, &dCurrAT);
//    SceneDevice->GetRenderState(D3DRS_STENCILENABLE, &dCurrST);
//    SceneDevice->GetRenderTarget(0, &pCurrRT);
      SceneDevice->GetTexture(0, &pCurrTX);
      SceneDevice->GetVertexShader(&pCurrVS);
      SceneDevice->GetPixelShader(&pCurrPS);
//    SceneDevice->GetFVF(&dCurrVF);
      SceneDevice->GetStreamSource(0, &pCurrVX, &dCurrVO, &dCurrVS);
#endif

//    SceneDevice->BeginScene();
//    SceneDevice->SetRenderState(D3DRS_ZENABLE, FALSE);
      SceneDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
      SceneDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
//    SceneDevice->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
//    SceneDevice->SetRenderState(D3DRS_STENCILENABLE, FALSE);
      if (pCurrDT != passDepthT[OBGEPASS_ANY])
	ResolveDepthBuffer(SceneDevice);
      SceneDevice->SetDepthStencilSurface(NULL);
      SceneDevice->SetRenderTarget(0, pGrabDS);
//    SceneDevice->Clear(0, NULL, D3DCLEAR_TARGET, 0, 0.0, 0);
      SceneDevice->SetTexture(0, pCurrDT);
      SceneDevice->SetVertexShader((IDirect3DVertexShader9 *)sm->cqv->pDX9ShaderClss);
      SceneDevice->SetPixelShader ((IDirect3DPixelShader9  *)sm->cqp->pDX9ShaderClss);
      SceneDevice->SetPixelShaderConstantF(0, (const float *)&Constants.ZRange, 1);
      SceneDevice->SetPixelShaderConstantF(1, (const float *)&Constants.proj, 4);

      SceneDevice->SetFVF(CAMERAQUADFORMAT);
      SceneDevice->SetStreamSource(0, pGrabVX, 0, sizeof(CameraQuad));
      SceneDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
//    SceneDevice->EndScene();

#ifndef	OBGE_STATEBLOCKS
      /* manual restore */
//    SceneDevice->SetRenderState(D3DRS_ZENABLE, dCurrZW);
      SceneDevice->SetRenderState(D3DRS_CULLMODE, dCurrCL);
      SceneDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, dCurrAB);
//    SceneDevice->SetRenderState(D3DRS_ALPHATESTENABLE, dCurrAT);
//    SceneDevice->SetRenderState(D3DRS_STENCILENABLE, dCurrST);
//    SceneDevice->SetRenderTarget(0, pCurrRT);
//    SceneDevice->SetDepthStencilSurface(pCurrDS);
      SceneDevice->SetTexture(0, pCurrTX);
      SceneDevice->SetVertexShader(pCurrVS);
      SceneDevice->SetPixelShader(pCurrPS);
//    SceneDevice->SetFVF(dCurrVF);
      SceneDevice->SetStreamSource(0, pCurrVX, dCurrVO, dCurrVS);
#else
      /* auto restore */
      pStateBlock->Apply();
      pStateBlock->Release();
#endif

      SceneDevice->SetRenderTarget(0, passSurface[OBGEPASS_ANY]);
      SceneDevice->SetDepthStencilSurface(passDepth[OBGEPASS_ANY]);

      bDFilled = true;
    }
  }
}

void RuntimeShaderRecord::Buffers::GrabDZ(IDirect3DDevice9 *StateDevice, IDirect3DDevice9 *SceneDevice, bool bZFused) {
  pTextDZ = NULL;
  if ((pTextDZ = passDepthT[currentPass])) {
    StateDevice->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
    bZLoaded = (bZFused ? -1 : 1);
  }
}

/* -------------------------------------------------------------------------------------------------
 */
void RuntimeShaderRecord::CreateRuntimeParams(LPD3DXCONSTANTTABLE CoTa) {
  Release();

  TextureManager *TexMan = TextureManager::GetSingleton();
  std::vector<int> prevTextures = Textures; Textures.clear();

  /* we got a table and we can use it */
  if (CoTa && lastOBGEDirect3DDevice9) {
    D3DXCONSTANTTABLE_DESC desc;
    D3DXCONSTANT_DESC cnst;
    UINT count = 1;

    /* get the description, handles are just pointers, you can't release them */
    if (CoTa->GetDesc(&desc) == D3D_OK) {
      /* for assigning global references */
      ShaderManager *sm = ShaderManager::GetSingleton();
      IDirect3DBaseTexture9 *unbound = (sm->unbound ? sm->unbound->GetTexture() : NULL);

      /* counts of what's in the tables */
      int lcls[4] = {0};
      int nums[4] = {0};
      int cnts[5] = {0};
      int size;

      /* count what's in the tables */
      for (int c = 0; c < desc.Constants; c++) {
	D3DXHANDLE handle = CoTa->GetConstant(NULL, c);
	CoTa->GetConstantDesc(handle, &cnst, &count);
	D3DXREGISTER_SET rs = cnst.RegisterSet;

//	switch (cnst.Type) {
//	  case D3DXPT_BOOL: rs = D3DXRS_BOOL; break;
//	  case D3DXPT_INT: rs = D3DXRS_INT4; break;
//	  case D3DXPT_FLOAT: rs = D3DXRS_FLOAT4; break;
//	  case D3DXPT_SAMPLER:
//	  case D3DXPT_SAMPLER1D:
//	  case D3DXPT_SAMPLER2D:
//	  case D3DXPT_SAMPLER3D:
//	  case D3DXPT_SAMPLERCUBE: rs = D3DXRS_SAMPLER; break;
//	}

	if (rs <= 4) {
	  if ((cnst.Name == strstr(cnst.Name, "cust_")) ||
	      (cnst.Name == strstr(cnst.Name, "glob_")) ||
	      (cnst.Name == strstr(cnst.Name, "oblv_")))
	    lcls[rs] += cnst.RegisterCount;
	  if ((cnst.Name == strstr(cnst.Name, "obge_")) ||
	      (cnst.Name == strstr(cnst.Name, "oblv_")) ||
	      (cnst.Name == strstr(cnst.Name, "glob_")) ||
	      (cnst.Name == strstr(cnst.Name, "cust_")))
	    nums[rs] += 1;
	}
      }

      /* check if there is anything we have to do by ourself or
       * if all the variables are handled by Oblivion
       */
      if (nums[D3DXRS_BOOL   ]) nums[D3DXRS_BOOL   ]++;
      if (nums[D3DXRS_INT4   ]) nums[D3DXRS_INT4   ]++;
      if (nums[D3DXRS_FLOAT4 ]) nums[D3DXRS_FLOAT4 ]++;
      if (nums[D3DXRS_SAMPLER]) nums[D3DXRS_SAMPLER]++;

      if (nums[D3DXRS_BOOL   ] +
	  nums[D3DXRS_INT4   ] +
	  nums[D3DXRS_FLOAT4 ] +
	  nums[D3DXRS_SAMPLER]) {

      /* we allocate all resources in one big contiguous block */
      size =
	((nums[D3DXRS_BOOL   ] +
	  nums[D3DXRS_INT4   ] +
	  nums[D3DXRS_FLOAT4 ] +
	  nums[D3DXRS_SAMPLER] +
	  nums[D3DXRS_SAMPLER]) * sizeof(struct RuntimeVariable)) +
	 ((lcls[D3DXRS_INT4   ] * sizeof(RuntimeVariable::mem::iv)) +
	  (lcls[D3DXRS_FLOAT4 ] * sizeof(RuntimeVariable::mem::fv)) +
	  (lcls[D3DXRS_SAMPLER] * sizeof(RuntimeVariable::mem::tv) * 16))
      ;

      pCustomCT = calloc(size, 1);
      pBool    = (RuntimeVariable *)pCustomCT;
      pInt4    = pBool    + nums[D3DXRS_BOOL   ];
      pFloat4  = pInt4    + nums[D3DXRS_INT4   ];
      pTexture = pFloat4  + nums[D3DXRS_FLOAT4 ];
      pSampler = pTexture + nums[D3DXRS_SAMPLER];
      RuntimeVariable::mem::iv *ivs = (RuntimeVariable::mem::iv *)(
		 pSampler + nums[D3DXRS_SAMPLER]);
      RuntimeVariable::mem::fv *fvs = (RuntimeVariable::mem::fv *)(
		 ivs      + lcls[D3DXRS_INT4   ]);
      RuntimeVariable::mem::tv *tvs = (RuntimeVariable::mem::tv *)(
		 fvs      + lcls[D3DXRS_FLOAT4 ]);
      assert((void *)((char *)pCustomCT + size) == (void *)(
		 tvs      + lcls[D3DXRS_SAMPLER] * 16));

      for (int c = 0; c < desc.Constants; c++) {
	D3DXHANDLE handle = CoTa->GetConstant(NULL, c);
	CoTa->GetConstantDesc(handle, &cnst, &count);
	D3DXREGISTER_SET rs = cnst.RegisterSet;

//	switch (cnst.Type) {
//	  case D3DXPT_BOOL: rs = D3DXRS_BOOL; break;
//	  case D3DXPT_INT: rs = D3DXRS_INT4; break;
//	  case D3DXPT_FLOAT: rs = D3DXRS_FLOAT4; break;
//	  case D3DXPT_SAMPLER:
//	  case D3DXPT_SAMPLER1D:
//	  case D3DXPT_SAMPLER2D:
//	  case D3DXPT_SAMPLER3D:
//	  case D3DXPT_SAMPLERCUBE: rs = D3DXRS_SAMPLER; break;
//	}

	if (rs > 4)
	  continue;
	if ((cnst.Name != strstr(cnst.Name, "obge_")) &&
	    (cnst.Name != strstr(cnst.Name, "oblv_")) &&
	    (cnst.Name != strstr(cnst.Name, "glob_")) &&
	    (cnst.Name != strstr(cnst.Name, "cust_")))
	  continue;

	switch (rs) {
	  case D3DXRS_BOOL:
	    /**/ if (cnst.Name == strstr(cnst.Name, "obge_"))
	      break;
	    else if (cnst.Name == strstr(cnst.Name, "oblv_"))
	      break;
	    else if (cnst.Name == strstr(cnst.Name, "glob_"))
	      break;
	    else if (cnst.Name == strstr(cnst.Name, "cust_")) {
	      if (cnst.DefaultValue)
		pBool[cnts[D3DXRS_BOOL]].vals.condition = (bool *)cnst.DefaultValue;
	    }

	    pBool[cnts[D3DXRS_BOOL]].offset = cnst.RegisterIndex;
	    pBool[cnts[D3DXRS_BOOL]].length = cnst.RegisterCount;
	    pBool[cnts[D3DXRS_BOOL]].name = cnst.Name;
	          cnts[D3DXRS_BOOL]++;
	    break;
	  case D3DXRS_INT4:
	    /**/ if (cnst.Name == strstr(cnst.Name, "obge_"))
	      break;
	    else if (cnst.Name == strstr(cnst.Name, "oblv_"))
	      break;
	    else if (cnst.Name == strstr(cnst.Name, "glob_"))
	      pInt4[cnts[D3DXRS_INT4]].vals.integer = sm->GetGlobalConst(cnst.Name, cnst.RegisterCount, ivs);
	    else if (cnst.Name == strstr(cnst.Name, "cust_")) {
	      if (cnst.DefaultValue)
		memcpy(ivs, cnst.DefaultValue, cnst.RegisterCount * sizeof(int) * 4);
	      pInt4[cnts[D3DXRS_INT4]].vals.integer = ivs; ivs += cnst.RegisterCount;
	    }

	    pInt4[cnts[D3DXRS_INT4]].offset = cnst.RegisterIndex;
	    pInt4[cnts[D3DXRS_INT4]].length = cnst.RegisterCount;
	    pInt4[cnts[D3DXRS_INT4]].name = cnst.Name;
	          cnts[D3DXRS_INT4]++;
	    break;
	  case D3DXRS_FLOAT4:
	    /**/ if (cnst.Name == strstr(cnst.Name, "obge_")) {
	      /**/ if (cnst.Name == strstr(cnst.Name, "obge_Tick"))
		pFloat4[cnts[D3DXRS_FLOAT4]].vals.floating = (RuntimeVariable::mem::fv *)&Constants.fTikTiming;
	      else if (cnst.Name == strstr(cnst.Name, "obge_Gamma"))
		pFloat4[cnts[D3DXRS_FLOAT4]].vals.floating = (RuntimeVariable::mem::fv *)&Constants.Gamma;
	      else
		break;
	    }
	    else if (cnst.Name == strstr(cnst.Name, "oblv_")) {
	      /**/ if (cnst.Name == strstr(cnst.Name, "oblv_WorldTransform_CURRENTPASS"))
	        pFloat4[cnts[D3DXRS_FLOAT4]].vals.floating = (RuntimeVariable::mem::fv *)&Constants.wrld;
	      else if (cnst.Name == strstr(cnst.Name, "oblv_ViewTransform_CURRENTPASS"))
		pFloat4[cnts[D3DXRS_FLOAT4]].vals.floating = (RuntimeVariable::mem::fv *)&Constants.view;
	      else if (cnst.Name == strstr(cnst.Name, "oblv_ViewInverse_CURRENTPASS"))
		pFloat4[cnts[D3DXRS_FLOAT4]].vals.floating = (RuntimeVariable::mem::fv *)&Constants.view_inv;
	      else if (cnst.Name == strstr(cnst.Name, "oblv_ProjectionTransform_CURRENTPASS"))
		pFloat4[cnts[D3DXRS_FLOAT4]].vals.floating = (RuntimeVariable::mem::fv *)&Constants.proj;
	      else if (cnst.Name == strstr(cnst.Name, "oblv_ProjectionDepthRange_CURRENTPASS"))
		pFloat4[cnts[D3DXRS_FLOAT4]].vals.floating = (RuntimeVariable::mem::fv *)&Constants.ZRange;
	      else if (cnst.Name == strstr(cnst.Name, "oblv_ProjectionFoV_CURRENTPASS"))
		pFloat4[cnts[D3DXRS_FLOAT4]].vals.floating = (RuntimeVariable::mem::fv *)&Constants.FoV;
	      else if (cnst.Name == strstr(cnst.Name, "oblv_ReciprocalResolution_CURRENTPASS"))
		pFloat4[cnts[D3DXRS_FLOAT4]].vals.floating = (RuntimeVariable::mem::fv *)&Constants.rcpres;
	      else if (cnst.Name == strstr(cnst.Name, "oblv_ReciprocalResolution_WATERHEIGHTMAPPASS"))
		pFloat4[cnts[D3DXRS_FLOAT4]].vals.floating = (RuntimeVariable::mem::fv *)&Constants.rcpresh;
	      else if (cnst.Name == strstr(cnst.Name, "oblv_ReciprocalResolution_WATERDISPLACEMENTPASS"))
		pFloat4[cnts[D3DXRS_FLOAT4]].vals.floating = (RuntimeVariable::mem::fv *)&Constants.rcpresd;
	      else if (cnst.Name == strstr(cnst.Name, "oblv_SunDirection"))
		pFloat4[cnts[D3DXRS_FLOAT4]].vals.floating = (RuntimeVariable::mem::fv *)&Constants.SunDir;
	      else if (cnst.Name == strstr(cnst.Name, "oblv_SunTiming"))
		pFloat4[cnts[D3DXRS_FLOAT4]].vals.floating = (RuntimeVariable::mem::fv *)&Constants.SunTiming;
	      else if (cnst.Name == strstr(cnst.Name, "oblv_FogRange"))
		pFloat4[cnts[D3DXRS_FLOAT4]].vals.floating = (RuntimeVariable::mem::fv *)&Constants.FogRange;
	      else if (cnst.Name == strstr(cnst.Name, "oblv_PlayerPosition"))
		pFloat4[cnts[D3DXRS_FLOAT4]].vals.floating = (RuntimeVariable::mem::fv *)&Constants.PlayerPosition;
	      else if (cnst.Name == strstr(cnst.Name, "oblv_EyePosition"))
		pFloat4[cnts[D3DXRS_FLOAT4]].vals.floating = (RuntimeVariable::mem::fv *)&Constants.EyePosition;
	      else if  (cnst.Name == strstr(cnst.Name, "oblv_GameTime"))
		pFloat4[cnts[D3DXRS_FLOAT4]].vals.floating = (RuntimeVariable::mem::fv *)&Constants.fGameTime;
	      else if (cnst.Name == strstr(cnst.Name, "oblv_TexData")) {
	        /**/ if (cnst.Name[12] == '0')
		  pFloat4[cnts[D3DXRS_FLOAT4]].vals.floating = (RuntimeVariable::mem::fv *)&sm->GlobalConst.pTexture[0].vals.texture.data;
		else if (cnst.Name[12] == '1')
		  pFloat4[cnts[D3DXRS_FLOAT4]].vals.floating = (RuntimeVariable::mem::fv *)&sm->GlobalConst.pTexture[1].vals.texture.data;
		else if (cnst.Name[12] == '2')
		  pFloat4[cnts[D3DXRS_FLOAT4]].vals.floating = (RuntimeVariable::mem::fv *)&sm->GlobalConst.pTexture[2].vals.texture.data;
		else if (cnst.Name[12] == '3')
		  pFloat4[cnts[D3DXRS_FLOAT4]].vals.floating = (RuntimeVariable::mem::fv *)&sm->GlobalConst.pTexture[3].vals.texture.data;
		else if (cnst.Name[12] == '4')
		  pFloat4[cnts[D3DXRS_FLOAT4]].vals.floating = (RuntimeVariable::mem::fv *)&sm->GlobalConst.pTexture[4].vals.texture.data;
		else if (cnst.Name[12] == '5')
		  pFloat4[cnts[D3DXRS_FLOAT4]].vals.floating = (RuntimeVariable::mem::fv *)&sm->GlobalConst.pTexture[5].vals.texture.data;
		else if (cnst.Name[12] == '6')
		  pFloat4[cnts[D3DXRS_FLOAT4]].vals.floating = (RuntimeVariable::mem::fv *)&sm->GlobalConst.pTexture[6].vals.texture.data;
		else if (cnst.Name[12] == '7')
		  pFloat4[cnts[D3DXRS_FLOAT4]].vals.floating = (RuntimeVariable::mem::fv *)&sm->GlobalConst.pTexture[7].vals.texture.data;
	      }
	      else
		break;
	    }
	    else if (cnst.Name == strstr(cnst.Name, "glob_"))
	      pFloat4[cnts[D3DXRS_FLOAT4]].vals.floating = sm->GetGlobalConst(cnst.Name, cnst.RegisterCount, fvs);
	    else if (cnst.Name == strstr(cnst.Name, "cust_")) {
	      if (cnst.DefaultValue)
		memcpy(fvs, cnst.DefaultValue, cnst.RegisterCount * sizeof(float) * 4);
	      pFloat4[cnts[D3DXRS_FLOAT4]].vals.floating = fvs; fvs += cnst.RegisterCount;
	    }

	    pFloat4[cnts[D3DXRS_FLOAT4]].offset = cnst.RegisterIndex;
	    pFloat4[cnts[D3DXRS_FLOAT4]].length = cnst.RegisterCount;
	    pFloat4[cnts[D3DXRS_FLOAT4]].name = cnst.Name;
	            cnts[D3DXRS_FLOAT4]++;
 	    break;
	  case D3DXRS_SAMPLER:
	    /**/ if (cnst.Name == strstr(cnst.Name, "obge_"))
	      break;
	    else if (cnst.Name == strstr(cnst.Name, "oblv_")) {
	      /* the default texture for any kind of dynamic texture is "white" */
	      pTexture[cnts[D3DXRS_SAMPLER]].vals.texture = unbound;

	      /* read and interprete the source and extract optional information */
	      int sts = (RuntimeVariable::mem::tv *)
		pAssociate->GetDX9ShaderTexture(cnst.Name,
		NULL,
		(DWORD *)tvs) - tvs;

	      /* specific sampler-states have been defined */
	      if (tvs->Type) {
		assert(sts < 16);

		pSampler[cnts[D3DXRS_SAMPLER + 1]].vals.state = tvs; tvs += sts + 1;
		pSampler[cnts[D3DXRS_SAMPLER + 1]].offset = cnst.RegisterIndex + (this->iType == SHADER_VERTEX ? D3DVERTEXTEXTURESAMPLER0 : 0);
		pSampler[cnts[D3DXRS_SAMPLER + 1]].length = cnst.RegisterCount;
		pSampler[cnts[D3DXRS_SAMPLER + 1]].name = cnst.Name;
		cnts[D3DXRS_SAMPLER + 1]++;
	      }

	      /**/ if (cnst.Name == strstr(cnst.Name, "oblv_CurrRendertarget0_CURRENTPASS"))
		pCopyRT = (IDirect3DTexture9 **)&pTexture[cnts[D3DXRS_SAMPLER]].vals.texture;
	      else if (cnst.Name == strstr(cnst.Name, "oblv_CurrDepthStenzilZ_CURRENTPASS"))
		pCopyDS = (IDirect3DTexture9 **)&pTexture[cnts[D3DXRS_SAMPLER]].vals.texture;
	      else if (cnst.Name == strstr(cnst.Name, "oblv_CurrDepthStenzilR_CURRENTPASS"))
		pCopyDZ = (IDirect3DTexture9 **)&pTexture[cnts[D3DXRS_SAMPLER]].vals.texture;
	      else
		break;
	    }
	    else if ((cnst.Name == strstr(cnst.Name, "glob_")) ||
		     (cnst.Name == strstr(cnst.Name, "cust_"))) {
	      /* the default texture for any kind of dynamic texture is "white" */
	      pTexture[cnts[D3DXRS_SAMPLER]].vals.texture = unbound;

	      /* read and interprete the source and extract optional information */
	      int tnm = -1;
	      int sts = (RuntimeVariable::mem::tv *)
		pAssociate->GetDX9ShaderTexture(cnst.Name,
		(int *)&tnm,
		(DWORD *)tvs) - tvs;

	      /* specific sampler-states have been defined */
	      if (tvs->Type) {
		assert(sts < 16);

		pSampler[cnts[D3DXRS_SAMPLER + 1]].vals.state = tvs; tvs += sts + 1;
		pSampler[cnts[D3DXRS_SAMPLER + 1]].offset = cnst.RegisterIndex + (this->iType == SHADER_VERTEX ? D3DVERTEXTEXTURESAMPLER0 : 0);
		pSampler[cnts[D3DXRS_SAMPLER + 1]].length = cnst.RegisterCount;
		pSampler[cnts[D3DXRS_SAMPLER + 1]].name = cnst.Name;
			 cnts[D3DXRS_SAMPLER + 1]++;
	      }

	      /* a specific texture has been given */
	      if (tnm != -1) {
		ManagedTextureRecord *tx = TexMan->GetTexture(tnm);
		IDirect3DBaseTexture9 *dx = (tx ? tx->GetTexture() : NULL);

		pTexture[cnts[D3DXRS_SAMPLER]].vals.texture = dx;
		Textures.push_back(tnm);
	      }
	    }

	    pTexture[cnts[D3DXRS_SAMPLER]].offset = cnst.RegisterIndex;
	    pTexture[cnts[D3DXRS_SAMPLER]].length = cnst.RegisterCount;
	    pTexture[cnts[D3DXRS_SAMPLER]].name = cnst.Name;
		     cnts[D3DXRS_SAMPLER]++;
	    break;
	}
      }

      assert(cnts[D3DXRS_BOOL	    ] <= nums[D3DXRS_BOOL	]);
      assert(cnts[D3DXRS_INT4	    ] <= nums[D3DXRS_INT4	]);
      assert(cnts[D3DXRS_FLOAT4	    ] <= nums[D3DXRS_FLOAT4	]);
      assert(cnts[D3DXRS_SAMPLER    ] <= nums[D3DXRS_SAMPLER    ]);
      assert(cnts[D3DXRS_SAMPLER + 1] <= nums[D3DXRS_SAMPLER    ]);

      if (!cnts[D3DXRS_BOOL	  ]) pBool    = NULL;
      if (!cnts[D3DXRS_INT4	  ]) pInt4    = NULL;
      if (!cnts[D3DXRS_FLOAT4	  ]) pFloat4  = NULL;
      if (!cnts[D3DXRS_SAMPLER	  ]) pTexture = NULL;
      if (!cnts[D3DXRS_SAMPLER + 1]) pSampler = NULL;
    }
    }
  }

  /* prepare statechanges */
  if ((bIO = (pCopyRT || pCopyDS || pCopyDZ))) { bMask = -1;
    DWORD States[16], *SBegin, *SEnd;

    SEnd = pAssociate->GetDX9RenderStates(SBegin = States);
    while (SBegin < SEnd) {
      switch (SBegin[0]) {
	case SHADER_COLORBUFFER:
	  /**/ if (SBegin[1] == LAZY  ) bCLazy = true;
	  else if (SBegin[1] == GREEDY) bCLazy = false; break;
	case SHADER_COLORWRITE:
	  /**/ if (SBegin[1] == FUSED ) bCFused = true;
	  else if (SBegin[1] == OFF   ) bCFused = false; break;
	case SHADER_ZBUFFER:
	  /**/ if (SBegin[1] == LAZY  ) bDLazy = true;
	  else if (SBegin[1] == GREEDY) bDLazy = false; break;
	case SHADER_ZWRITE:
	  /**/ if (SBegin[1] == FUSED ) bZFused = true;
	  else if (SBegin[1] == OFF   ) bZFused = false; break;
	case SHADER_PASSMASK:
	  /**/ bMask = SBegin[1]; break;
      }

      SBegin += 2;
    }
  }

  /* release previous texture */
  std::vector<int>::iterator PTexture = prevTextures.begin();

  while (PTexture != prevTextures.end()) {
    TexMan->ReleaseTexture(*PTexture);
    PTexture++;
  }
}

void RuntimeShaderRecord::SetRuntimeParams(IDirect3DDevice9 *StateDevice, IDirect3DDevice9 *SceneDevice) {
  struct RuntimeShaderRecord::Buffers *buf = &rsb[currentPass];

  /* blow the fuse (vertex shader comes after pixel shader and clears?) */
  if ((buf->bZLoaded < 0) && !pCopyDZ && (iType == SHADER_PIXEL)) {
    buf->bZLoaded = 0;

    /* well, I don't really know if it was on before ... */
    StateDevice->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
  }

  if (pCustomCT) {
    if (bMask & (1 << currentPass)) {
      /* check what needs to go on here */
      const bool doRT = pCopyRT && (!buf->pTextRT || !buf->bCFilled || !bCLazy),
		 doDS = pCopyDS && (!buf->pTextDS || !buf->bDFilled || !bDLazy),
		 doDZ = pCopyDZ && (!buf->pTextDZ || !buf->bZLoaded);

      if (doRT /*|| doDS*/)
	markerReset(SceneDevice);
      if (doRT)
	buf->GrabRT(StateDevice, SceneDevice);
      if (doRT /*|| doDS*/)
	markerReset(SceneDevice);
      if (doDS)
	buf->GrabDS(StateDevice, SceneDevice);
      if (doDZ)
	buf->GrabDZ(StateDevice, SceneDevice, bZFused);

      if (pCopyRT && buf->bCFilled) (*pCopyRT) = buf->pTextRT;
      if (pCopyDS && buf->bDFilled) (*pCopyDS) = buf->pTextDS;
      if (pCopyDZ && buf->bZLoaded) (*pCopyDZ) = buf->pTextDZ;
    }

    if (iType == SHADER_VERTEX) {
      RuntimeVariable *rV;

      /* do the textures first, texture-data may be requested */
      if ((rV = pTexture))
	do {
	  // Do we have to bias with D3DDMAPSAMPLER
	  StateDevice->SetTexture(rV->offset, rV->vals.texture);
	} while ((++rV)->length);
      if ((rV = pSampler)) {
	DWORD bak = AFilters; AFilters = 0;
	do {
	  RuntimeVariable::mem::tv *rT;
	  if ((rT = rV->vals.state))
	    do {
	      StateDevice->SetSamplerState(rV->offset, rT->Type, rT->Value);
	    } while ((++rT)->Type);
	} while ((++rV)->length);
	AFilters = bak;
      }

      /* set the constant arrays */
      if ((rV = pBool))
	do {
	  StateDevice->SetVertexShaderConstantB(rV->offset, (const BOOL *)rV->vals.condition, rV->length);
	} while ((++rV)->length);
      if ((rV = pInt4))
	do {
	  StateDevice->SetVertexShaderConstantI(rV->offset, (const int *)rV->vals.integer, rV->length);
	} while ((++rV)->length);
      if ((rV = pFloat4))
	do {
	  StateDevice->SetVertexShaderConstantF(rV->offset, (const float *)rV->vals.floating, rV->length);
	} while ((++rV)->length);
    }
    else if (iType == SHADER_PIXEL) {
      RuntimeVariable *rV;

      /* do the textures first, texture-data may be requested */
      if ((rV = pTexture))
	do {
	  StateDevice->SetTexture(rV->offset, rV->vals.texture);
	} while ((++rV)->length);
      if ((rV = pSampler)) {
	DWORD bak = AFilters; AFilters = 0;
	do {
	  RuntimeVariable::mem::tv *rT;
	  if ((rT = rV->vals.state))
	    do {
	      StateDevice->SetSamplerState(rV->offset, rT->Type, rT->Value);
	    } while ((++rT)->Type);
	} while ((++rV)->length);
	AFilters = bak;
      }

      /* set the constant arrays */
      if ((rV = pBool))
	do {
	  StateDevice->SetPixelShaderConstantB(rV->offset, (const BOOL *)rV->vals.condition, rV->length);
	} while ((++rV)->length);
      if ((rV = pInt4))
	do {
	  StateDevice->SetPixelShaderConstantI(rV->offset, (const int *)rV->vals.integer, rV->length);
	} while ((++rV)->length);
      if ((rV = pFloat4))
	do {
	  StateDevice->SetPixelShaderConstantF(rV->offset, (const float *)rV->vals.floating, rV->length);
	} while ((++rV)->length);
    }
  }
}

bool RuntimeShaderRecord::SetShaderConstantB(const char *name, bool value) {
  RuntimeVariable *rV;
  if ((rV = pBool))
    do {
      if (!stricmp(rV->name, name)) {
      	*(rV->vals.condition) = value;
      	return true;
      }
    } while ((++rV)->length);
  return false;
}

bool RuntimeShaderRecord::SetShaderConstantI(const char *name, int *values) {
  RuntimeVariable *rV;
  if ((rV = pInt4))
    do {
      if (!stricmp(rV->name, name)) {
      	memcpy(rV->vals.integer, values, sizeof(int) * 4);
      	return true;
      }
    } while ((++rV)->length);
  return false;
}

bool RuntimeShaderRecord::SetShaderConstantF(const char *name, float *values) {
  RuntimeVariable *rV;
  if ((rV = pFloat4))
    do {
      if (!stricmp(rV->name, name)) {
      	memcpy(rV->vals.floating, values, sizeof(float) * 4);
      	return true;
      }
    } while ((++rV)->length);
  return false;
}

bool RuntimeShaderRecord::SetShaderSamplerTexture(const char *name, int TextureNum) {
  RuntimeVariable *rV;
  if ((rV = pTexture))
    do {
      if (!stricmp(rV->name, name)) {
	TextureManager *TexMan = TextureManager::GetSingleton();

	IDirect3DBaseTexture9 *OldTexture = NULL;
	/* get old one */
	OldTexture = rV->vals.texture;
	/* and dereference */
	rV->vals.texture = NULL;

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
	  rV->vals.texture = NewTexture->GetTexture();

	  /* add to vector */
	  if (true)
	    Textures.push_back(TextureNum);

	  return true;
	}

	return false;
      }
    } while ((++rV)->length);
  return false;
}

void RuntimeShaderRecord::PurgeTexture(IDirect3DBaseTexture9 *texture, int TexNum) {
  RuntimeVariable *rV;
  if ((rV = pTexture))
    do {
      if (rV->vals.texture == texture) {
	rV->vals.texture = NULL;

	/* remove from vector */
	if (TexNum != -1)
	  Textures.erase(std::find(Textures.begin(), Textures.end(), TexNum));
      }
    } while ((++rV)->length);
}

/* -------------------------------------------------------------------------------------------------
 */

bool RuntimeShaderRecord::AssignShader(IUnknown *Shader, ShaderRecord *Associate) {
  /* TODO: shaderactivelist :^D */
  bActive = ((pShader = Shader) != NULL);

  /* cyclic reference */
  if ((pAssociate = Associate)) {
    pAssociate->pAssociate = this;

    /* the same binary Oblivion gets into it's hands */
    iType = pAssociate->iType;
    pFunction = pAssociate->pOblivionBinary;

    /* and it's constant-table */
    CreateRuntimeParams(pAssociate->pOblivionConTab);
  }

  return bActive;
}

bool RuntimeShaderRecord::ActivateShader(char which) {
  /* create the necessary resources, to incorporate them at any time */
  if ((bActive = (pAssociate && pAssociate->ConstructDX9Shader(which))))
    if ((bActive = ((pShader = pAssociate->pDX9ShaderClss) != NULL)))
      CreateRuntimeParams(pAssociate->pDX9ShaderCoTa);

  /* for now we don't want any replacement */
  if (!::RuntimeSources.Get())
    return false;

  return bActive;
}

/* -------------------------------------------------------------------------------------------------
 */

IDirect3DPixelShader9 *RuntimeShaderRecord::GetShader(IDirect3DPixelShader9 *Shader) const {
  /* for now we don't want any replacement (return Oblivion's own shader class) */
  if (!::RuntimeSources.Get())
    return Shader;

#ifdef	OBGE_DEVLING
  if (bMark) {
    ShaderRecord *id;
    if ((id = ShaderManager::GetSingleton()->idp))
      return (IDirect3DPixelShader9 *)id->pDX9ShaderClss;
  }
#endif

  /* we have some replacement resource, and we're going to
   * pass that, do a real quick sanity check first
   */
  if (bActive && pAssociate && (pShader == pAssociate->pDX9ShaderClss))
    return pPixelShader;

  /* that didn't go so well */
  return Shader;
}

IDirect3DVertexShader9 *RuntimeShaderRecord::GetShader(IDirect3DVertexShader9 *Shader) const {
  /* for now we don't want any replacement (return Oblivion's own shader class) */
  if (!::RuntimeSources.Get())
    return Shader;

  /* we have some replacement resource, and we're going to
   * pass that, do a real quick sanity check first
   */
  if (bActive && pAssociate && (pShader == pAssociate->pDX9ShaderClss))
    return pVertexShader;

  /* that didn't go so well */
  return Shader;
}

/* #################################################################################################
 */

ShaderManager *ShaderManager::Singleton = NULL;

ShaderManager::ShaderManager() {
  LARGE_INTEGER freq;

  QueryPerformanceFrequency(&freq);

  Constants.fTikTiming.w = (freq.QuadPart);

  /* static on compile, has to recompile if changed
   * NOTE: this code is called even before the D3D-device is created!
   */
  if (TextureManager::GetSingleton()->SetAnisotropy()) {
    defs[DEFS_MIN].Definition = (Anisotropy ? FILTER_ANISO : FILTER_LINEAR);
    sprintf(LEVEL_ANISO, "%d", Anisotropy);
  }

#ifdef	OBGE_DEVLING
  Clear();

  /* only thing needed */
  frame_capt = 0;
#endif
}

ShaderManager::~ShaderManager() {
  Singleton = NULL;
}

ShaderManager *ShaderManager::GetSingleton() {
  if (ShaderManager::Singleton)
    return ShaderManager::Singleton;

  if (::UseShaderOverride.Get()) {
    _MESSAGE("Replacing the built-in shaders.");
    if (GetFileAttributes(::ShaderOverrideDirectory.Get()) == INVALID_FILE_ATTRIBUTES) {
      _MESSAGE("Override directory %s doesn't exist, deactivating feature.", ::ShaderOverrideDirectory.Get());
      ::UseShaderOverride.Set(false);
    }
  }

  if (!ShaderManager::Singleton)
    ShaderManager::Singleton = new ShaderManager();

  return ShaderManager::Singleton;
}

void ShaderManager::Reset() {
  BuiltInShaders.clear();
  RuntimeShaders.clear();
  Shaders.clear();

#ifdef	OBGE_DEVLING
  Clear();
#endif
}

bool ShaderManager::SetTransferZ(long MaskZ) {
  defs[DEFS_INRAWZ].Definition[0] = (MaskZ & 1 ? '1' : '0');

  return true;
}

void ShaderManager::OnCreateDevice() {
  if ((idp = GetBuiltInShader("IDENTIFY.pso"))) {
    idp->CompileShader();
    idp->ConstructDX9Shader(SHADER_RUNTIME);
  }

  if ((cqp = GetBuiltInShader("COPYQUAD.pso"))) {
    cqp->CompileShader();
    cqp->ConstructDX9Shader(SHADER_RUNTIME);
  }

  if ((cqv = GetBuiltInShader("COPYQUAD.vso"))) {
    cqv->CompileShader();
    cqv->ConstructDX9Shader(SHADER_RUNTIME);
  }

  int utx; unbound = NULL; TextureManager *TexMan = TextureManager::GetSingleton();
  if (TexMan && (utx = TexMan->LoadDependtTexture("shaders/unbound.dds", TR_PLANAR)) != -1) {
    unbound = TexMan->GetTexture(utx);
    for (int p = 0; p < OBGEPASS_NUM; p++)
      RuntimeShaderRecord::rsb[p].pTextUB = (IDirect3DTexture9 *)unbound;
  }
}

void ShaderManager::OnReleaseDevice() {
  /* prevent locking up because other resources have allready been freed */
  RuntimeShaderList prevRShaders = RuntimeShaders; RuntimeShaders.clear();
  RuntimeShaderList::iterator RShader = prevRShaders.begin();

  while (RShader != prevRShaders.end()) {
    delete (*RShader);
    RShader++;
  }

  /* prevent locking up because other resources have allready been freed */
  BuiltInShaderList prevBShaders = BuiltInShaders; BuiltInShaders.clear();
  BuiltInShaderList::iterator BShader = prevBShaders.begin();

  while (BShader != prevBShaders.end()) {
    delete (*BShader);
    BShader++;
  }

  Reset();
}

void ShaderManager::OnLostDevice() {
  RuntimeShaderList::iterator RShader = RuntimeShaders.begin();

  while (RShader != RuntimeShaders.end()) {
    (*RShader)->OnLostDevice();
    RShader++;
  }
}

void ShaderManager::OnResetDevice() {
  RuntimeShaderList::iterator RShader = RuntimeShaders.begin();

  while (RShader != RuntimeShaders.end()) {
    (*RShader)->OnResetDevice();
    RShader++;
  }
}

/* -------------------------------------------------------------------------------------------------
 */

bool ShaderManager::ChangedShaders() {
  /* prevent locking up because other resources have allready been freed */
  BuiltInShaderList prevBShaders = BuiltInShaders;
  BuiltInShaderList::iterator BShader = prevBShaders.begin();

  while (BShader != prevBShaders.end()) {
    /* just one positive needed */
    if ((*BShader)->ChangedShader())
      return true;

    BShader++;
  }

  return false;
}

bool ShaderManager::ReloadShaders() {
  bool change = false;

  /* prevent locking up because other resources have allready been freed */
  BuiltInShaderList prevBShaders = BuiltInShaders;
  BuiltInShaderList::iterator BShader = prevBShaders.begin();

  while (BShader != prevBShaders.end()) {
    change = change || (*BShader)->ReloadShader();
    BShader++;
  }

  return change;
}

/* -------------------------------------------------------------------------------------------------
 */

void ShaderManager::UpdateFrameConstants() {
}

/* -------------------------------------------------------------------------------------------------
 */

ShaderRecord *ShaderManager::GetBuiltInShader(const char *Name) {
  /* search for an entry with the same name, this actually shouldn't
   * really happen (no double shader-allocation)
   */
  BuiltInShaderList::iterator BShader = BuiltInShaders.begin();
  while (BShader != BuiltInShaders.end()) {
    if (!_stricmp((*BShader)->Name, Name))
      return (*BShader);

    BShader++;
  }

  /* request a new class, load and prepare */
  ShaderRecord *NewShader = NULL;
  NewShader = new ShaderRecord();

  if (!NewShader->LoadShader(Name)) {
    delete NewShader;
    return NULL;
  }

  BuiltInShaders.push_back(NewShader);
  return NewShader;
}

ShaderRecord *ShaderManager::GetBuiltInShader(const DWORD *Function) {
  /* search for an entry with the same function
   */
  BuiltInShaderList::iterator BShader = BuiltInShaders.begin();
  while (BShader != BuiltInShaders.end()) {
    if ((*BShader)->pOblivionBinary == Function)
      return (*BShader);

    BShader++;
  }

  return NULL;
}

RuntimeShaderRecord *ShaderManager::SetRuntimeShader(const DWORD *Function, IUnknown *Shader) {
  /* search for an entry with the same function, this list is constructed on demand and
   * it's faster to search it first
   */
  RuntimeShaderList::iterator RShader = RuntimeShaders.begin();
  while (RShader != RuntimeShaders.end()) {
    /* identify by the same binary Oblivions got into it's hands */
    if ((*RShader)->pAssociate && ((*RShader)->pAssociate->pOblivionBinary == Function)) {
      /* put into the fast LUT for inside the pipeline */
      Shaders[Shader] = (*RShader);
      return (*RShader);
    }

    RShader++;
  }

  RuntimeShaderRecord *NewShader = NULL;
  ShaderRecord *AscShader = NULL;
  NewShader = new RuntimeShaderRecord();
  AscShader = GetBuiltInShader(Function);

  /* there is no failure possible here, if we do not allocate and provide this
   * resource on CreateShader() we won't be able to fetch the runtime replacements
   * quickly from the LUT
   * it doesn't matter if this succeeds or fails, it provides functionality
   */
  NewShader->AssignShader(Shader, AscShader);
  RuntimeShaders.push_back(NewShader);
  Shaders[Shader] = NewShader;
  return NewShader;
}

RuntimeShaderRecord *ShaderManager::GetRuntimeShader(const char *Name) {
  /* search for an entry with the same name, this actually shouldn't
   * really happen (no double shader-allocation)
   */
  RuntimeShaderList::iterator RShader = RuntimeShaders.begin();
  while (RShader != RuntimeShaders.end()) {
    /* identify by the same binary Oblivions got into it's hands */
    if ((*RShader)->pAssociate && !stricmp((*RShader)->pAssociate->Name, Name))
      return (*RShader);

    RShader++;
  }

  return NULL;
}

/* -------------------------------------------------------------------------------------------------
 */

IDirect3DPixelShader9 *ShaderManager::GetShader(IDirect3DPixelShader9 *Shader) {
  /* for now we don't want any replacement (return Oblivion's own shader class) */
  if (!::RuntimeSources.Get())
    return Shader;

  /* we have some replacement resource directly from the LUT, and we're going to pass that */
  RuntimeShaderRecord *hit = Shaders[(IUnknown *)Shader];
  if (hit)
    return hit->GetShader(Shader);

  /* that didn't go so well */
  return Shader;
}

IDirect3DVertexShader9 *ShaderManager::GetShader(IDirect3DVertexShader9 *Shader) {
  /* for now we don't want any replacement (return Oblivion's own shader class) */
  if (!::RuntimeSources.Get())
    return Shader;

  /* we have some replacement resource directly from the LUT, and we're going to pass that */
  RuntimeShaderRecord *hit = Shaders[(IUnknown *)Shader];
  if (hit)
    return hit->GetShader(Shader);

  /* that didn't go so well */
  return Shader;
}

/* -------------------------------------------------------------------------------------------------
 */

bool ShaderManager::SetShaderConstantB(const char *ShaderName, char *name, bool value) {
  RuntimeShaderRecord *pShader;

/*if (!strcmp(ShaderName, "*")) {
    char nm[256]; sprintf(nm, "glob_%s", name);
    return SetGlobalConst(nm, 1, value);
  }
  else*/ if ((pShader = GetRuntimeShader(ShaderName))) {
    char nm[256]; sprintf(nm, "cust_%s", name);
    return pShader->SetShaderConstantB(nm, value);
  }

  return false;
}

bool ShaderManager::SetShaderConstantI(const char *ShaderName, char *name, int *values) {
  RuntimeShaderRecord *pShader;

  if (!strcmp(ShaderName, "*")) {
    char nm[256]; sprintf(nm, "glob_%s", name);
    return SetGlobalConst(nm, 1, (RuntimeVariable::mem::iv *)values);
  }
  else if ((pShader = GetRuntimeShader(ShaderName))) {
    char nm[256]; sprintf(nm, "cust_%s", name);
    return pShader->SetShaderConstantI(nm, values);
  }

  return false;
}

bool ShaderManager::SetShaderConstantF(const char *ShaderName, char *name, float *values) {
  RuntimeShaderRecord *pShader;

  if (!strcmp(ShaderName, "*")) {
    char nm[256]; sprintf(nm, "glob_%s", name);
    return SetGlobalConst(nm, 1, (RuntimeVariable::mem::fv *)values);
  }
  else if ((pShader = GetRuntimeShader(ShaderName))) {
    char nm[256]; sprintf(nm, "cust_%s", name);
    return pShader->SetShaderConstantF(nm, values);
  }

  return false;
}

bool ShaderManager::SetShaderSamplerTexture(const char *ShaderName, char *name, int TextureNum) {
  RuntimeShaderRecord *pShader;

/*if (!strcmp(ShaderName, "*")) {
    char nm[256]; sprintf(nm, "glob_%s", name);
    return SetGlobalConst(nm, 1, (RuntimeVariable::mem::tv *)value);
  }
  else*/ if ((pShader = GetRuntimeShader(ShaderName))) {
    char nm[256]; sprintf(nm, "cust_%s", name);
    return pShader->SetShaderSamplerTexture(nm, TextureNum);
  }

  return false;
}

void ShaderManager::PurgeTexture(IDirect3DBaseTexture9 *texture, int TexNum) {
  RuntimeShaderList::iterator RShader = RuntimeShaders.begin();

  while (RShader != RuntimeShaders.end()) {
    if (*RShader)
      (*RShader)->PurgeTexture(texture, TexNum);

    RShader++;
  }
}

/* -------------------------------------------------------------------------------------------------
 */

void ShaderManager::UseShaderOverride(bool yes) {
  ::UseShaderOverride.Set(yes);
}

void ShaderManager::SaveShaderOverride(bool yes) {
  ::SaveShaderOverride.Set(yes);
}

void ShaderManager::UseLegacyCompiler(bool yes) {
  ::UseLegacyCompiler.Set(yes);
}

void ShaderManager::CompileSources(bool yes) {
  ::CompileSources.Set(yes);
}

void ShaderManager::RuntimeSources(bool yes) {
  ::RuntimeSources.Set(yes);
}

void ShaderManager::Optimize(bool yes) {
  ::Optimize.Set(yes);
}

void ShaderManager::UpgradeSM(bool yes) {
  ::UpgradeSM.Set(yes);
}

void ShaderManager::MaximumSM(bool yes) {
  ::MaximumSM.Set(yes);
}

bool ShaderManager::UseShaderOverride() {
  return ::UseShaderOverride.Get();
}

bool ShaderManager::SaveShaderOverride() {
  return ::SaveShaderOverride.Get();
}

bool ShaderManager::UseLegacyCompiler() {
  return ::UseLegacyCompiler.Get();
}

bool ShaderManager::CompileSources() {
  return ::CompileSources.Get();
}

bool ShaderManager::RuntimeSources() {
  return ::RuntimeSources.Get();
}

bool ShaderManager::Optimize() {
  return ::Optimize.Get();
}

bool ShaderManager::UpgradeSM() {
  return ::UpgradeSM.Get();
}

bool ShaderManager::MaximumSM() {
  return ::MaximumSM.Get();
}

#else
#include "ShaderManager.h"

ShaderManager *ShaderManager::Singleton = NULL;

ShaderManager::ShaderManager() {
}

ShaderManager::~ShaderManager() {
  Singleton = NULL;
}

ShaderManager *ShaderManager::GetSingleton() {
  if (ShaderManager::Singleton)
    return ShaderManager::Singleton;
  if (!ShaderManager::Singleton)
    ShaderManager::Singleton = new ShaderManager();

  return ShaderManager::Singleton;
}
#endif
