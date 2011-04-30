#include <sys/stat.h>

#include "ShaderManager.h"
#include "GlobalSettings.h"

#include "D3D9.hpp"

static global<bool> UseShaderOverride(true,NULL,"Shaders","bUseShaderOverride");
static global<bool> UseLegacyCompiler(false,NULL,"Shaders","bUseLegacyCompiler");
static global<bool> SaveShaderOverride(true,NULL,"Shaders","bSaveShaderOverride");
static global<bool> CompileSources(false,NULL,"Shaders","bCompileSources");
static global<bool> RuntimeSources(false,NULL,"Shaders","bRuntimeSources");
static global<bool> Optimize(false,NULL,"Shaders","bOptimize");
static global<bool> MaximumSM(false,NULL,"Shaders","bMaximumSM");
static global<bool> UpgradeSM(false,NULL,"Shaders","bUpgradeSM1X");
static global<char*> ShaderOverrideDirectory("data\\shaders\\override\\",NULL,"Shaders","sShaderOverrideDirectory");

/* #################################################################################################
 */

class IncludeManager : public ID3DXInclude
{
public:
    STDMETHOD(Open)(D3DXINCLUDE_TYPE IncludeType, LPCSTR pFileName, LPCVOID pParentData, LPCVOID *ppData, UINT *pBytes);
    STDMETHOD(Close)(LPCVOID pData);
};

HRESULT IncludeManager::Open(D3DXINCLUDE_TYPE IncludeType, LPCSTR pName, LPCVOID pParentData, LPCVOID *ppData, UINT *pBytes)
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

HRESULT IncludeManager::Close(LPCVOID pData)
{
    BYTE* pData2 = (BYTE*)pData;
    if (pData2)
      delete[] pData2;
    return S_OK;
}

IncludeManager incl;

/* #################################################################################################
 */

ShaderRecord::ShaderRecord() {
  Filepath[0] = '\0';
  Name[0] = '\0';

  pAssociate = NULL;
  pOblivionBinary = NULL;

  /* shader source, binaries and constant tables */
  pProfile = NULL;
  pSourceReplaced = NULL; sourceLen = 0;
  pAsmblyReplaced = NULL; asmblyLen = 0;
  pShaderReplaced = NULL;
  pConstsReplaced = NULL;

  pSourceRuntime = NULL; runtimeLen = 0;
  pShaderRuntime = NULL;
  pConstsRuntime = NULL;

  pShaderOriginal = NULL;
  pConstsOriginal = NULL;

  pErrorMsgs = NULL;
  pDisasmbly = NULL;

  /* shader class and the identifier for which of the binaries is selected */
  pDX9ShaderClss = NULL;
  pDX9ShaderType = SHADER_UNSET;
  pDX9ShaderWant = SHADER_UNSET;
}

ShaderRecord::~ShaderRecord() {
  /* shader source, binaries and constant tables */
  if (pProfile) delete[] pProfile;

  if (pSourceReplaced) delete[] pSourceReplaced;
  if (pAsmblyReplaced) delete[] pAsmblyReplaced;
  if (pShaderReplaced) pShaderReplaced->Release();
  if (pConstsReplaced) pConstsReplaced->Release();

  if (pSourceRuntime) delete[] pSourceRuntime;
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
  else
    _DMESSAGE("Upgraded version of %s to max.", Name);

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
      if (pShaderReplaced) pShaderReplaced->Release();
      if (pConstsReplaced) pConstsReplaced->Release();

      pShaderReplaced = NULL;
      pConstsReplaced = NULL;
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
      }
      else {
      	delete[] pSourceReplaced;

      	pSourceReplaced = NULL;
        sourceLen = 0;
      }
    }

    /* if the HLSL source is newer than the binary, attempt to recompile
     */
    if (pSourceReplaced && (sh.st_mtime > sb.st_mtime)) {
      if (pShaderReplaced) pShaderReplaced->Release();
      if (pConstsReplaced) pConstsReplaced->Release();

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
    if (iType == SHADER_VERTEX) {
      pProfile = strdup(D3DXGetVertexShaderProfile(lastOBGEDirect3DDevice9));
    }
    else if (iType == SHADER_PIXEL) {
      pProfile = strdup(D3DXGetPixelShaderProfile(lastOBGEDirect3DDevice9));
    }
  }

  return true;
}

bool ShaderRecord::RuntimeFlush() {
  /* release all runtime resources (for example for recompilation) */
  if (pShaderRuntime) pShaderRuntime->Release();
  if (pConstsRuntime) pConstsRuntime->Release();

  if (pErrorMsgs) pErrorMsgs->Release();
  if (pDisasmbly) pDisasmbly->Release();

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
  const DWORD *pFunction = NULL, *pClassFunction = NULL;
  char choosen = SHADER_UNSET;

  /* cascade, the highest possible is selected */
  if (pShaderRuntime && (which >= SHADER_RUNTIME))
    p = pShaderRuntime, choosen = SHADER_RUNTIME;
  else if (pShaderReplaced && (which >= SHADER_REPLACED))
    p = pShaderReplaced, choosen = SHADER_REPLACED;
  else if (pShaderOriginal && (which >= SHADER_ORIGINAL))
    p = pShaderOriginal, choosen = SHADER_ORIGINAL;
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

bool ShaderRecord::DestroyDX9Shader() {
  if (pDX9ShaderClss)
    pDX9ShaderClss->Release();

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
      _MESSAGE("Shader assembling errors occured in %s:", Filepath);
      _MESSAGE((char*)pErrorMsgs->GetBufferPointer());
    }
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
  SaveShader();

  return (pAsmblyReplaced && (pShaderReplaced != NULL));
}

bool ShaderRecord::CompileShader(bool forced) {
  /* nobody wants the automatic recompile */
  if (!::CompileSources.Get() && !forced)
    return false;

  LPSTR src = NULL; int len;
  LPD3DXBUFFER p = NULL;
  LPD3DXCONSTANTTABLE c = NULL;

  /* cascade, the highest possible is selected */
  if (pSourceRuntime) {
    src = pSourceRuntime;
    len = runtimeLen;

    p = pShaderRuntime;
    c = pConstsRuntime;
  }
  else if (pSourceReplaced) {
    src = pSourceReplaced;
    len = sourceLen;

    p = pShaderReplaced;
    c = pConstsReplaced;
  }

  /* recompile only, if there is one already, just ignore */
  if (!p && src) {
    if (pErrorMsgs)
    pErrorMsgs->Release();
    pErrorMsgs = NULL;

    D3DXCompileShader(
	src,
	len,
	NULL,
	&incl,
	"main",
	pProfile,
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
	  NULL,
	  &incl,
	  "main",
	  pProfile,
	  D3DXSHADER_DEBUG | (
	    ::UpgradeSM.Get() ? D3DXSHADER_ENABLE_BACKWARDS_COMPATIBILITY : D3DXSHADER_USE_LEGACY_D3DX9_31_DLL),
	  &p,
	  &pErrorMsgs,
	  &c
      );
    }

    /* this didn't go so well */
    if (pErrorMsgs) {
      _MESSAGE("Shader compiling errors occured in %s:", Filepath);
      _MESSAGE((char*)pErrorMsgs->GetBufferPointer());
    }
  }

  /* cascade, the highest possible is selected */
  if (pSourceRuntime) {
    pShaderRuntime = p;
    pConstsRuntime = c;
  }
  else if (pSourceReplaced) {
    pShaderReplaced = p;
    pConstsReplaced = c;
  }

  /* auto-save or not */
  SaveShader();

  return (pSourceRuntime  && (pShaderRuntime  != NULL)) ||
	 (pSourceReplaced && (pShaderReplaced != NULL));
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
  if (pShaderReplaced) return (pOblivionBinary = (const DWORD *)pShaderReplaced->GetBufferPointer());
  if (pShaderOriginal) return (pOblivionBinary = (const DWORD *)pShaderOriginal->GetBufferPointer());

  return (pOblivionBinary = NULL);
}

/* #################################################################################################
 */

RuntimeShaderRecord::RuntimeShaderRecord() {
  pAssociate = NULL;
  pFunction = NULL;
  pShader = NULL;
  bActive = false;

  memset(frame_used, -1, sizeof(frame_used));
}

RuntimeShaderRecord::~RuntimeShaderRecord() {
  /* we don't have any resources on our own, nothing to release */
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
    pFunction = pAssociate->pOblivionBinary;
  }

  return bActive;
}

bool RuntimeShaderRecord::ActivateShader(char which) {
  /* create the necessary resources, to incorporate them at any time */
  if ((bActive = (pAssociate && pAssociate->ConstructDX9Shader(which))))
    bActive = ((pShader = pAssociate->pDX9ShaderClss) != NULL);

  /* for now we don't want any replacement */
  if (!::RuntimeSources.Get())
    return false;

  return bActive;
}

/* -------------------------------------------------------------------------------------------------
 */

IDirect3DPixelShader9 *RuntimeShaderRecord::GetRuntimeShader(IDirect3DPixelShader9 *Shader) const {
  /* for now we don't want any replacement (return Oblivion's own shader class) */
  if (!::RuntimeSources.Get())
    return Shader;

  /* we have some replacement resource, and we're going to
   * pass that, do a real quick sanity check first
   */
  if (bActive && pAssociate && (pShader == pAssociate->pDX9ShaderClss))
    return pPixelShader;

  /* that didn't go so well */
  return Shader;
}

IDirect3DVertexShader9 *RuntimeShaderRecord::GetRuntimeShader(IDirect3DVertexShader9 *Shader) const {
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
  Clear();
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

      return NULL;
    }
  }

  if (!ShaderManager::Singleton)
    ShaderManager::Singleton = new ShaderManager();

  return ShaderManager::Singleton;
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

RuntimeShaderRecord *ShaderManager::GetRuntimeShader(const DWORD *Function, IUnknown *Shader) {
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

IDirect3DPixelShader9 *ShaderManager::GetRuntimeShader(IDirect3DPixelShader9 *Shader) {
  /* for now we don't want any replacement (return Oblivion's own shader class) */
  if (!::RuntimeSources.Get())
    return Shader;

  /* we have some replacement resource directly from the LUT, and we're going to pass that */
  RuntimeShaderRecord *hit = Shaders[(IUnknown *)Shader];
  if (hit)
    return hit->GetRuntimeShader(Shader);

  /* that didn't go so well */
  return Shader;
}

IDirect3DVertexShader9 *ShaderManager::GetRuntimeShader(IDirect3DVertexShader9 *Shader) {
  /* for now we don't want any replacement (return Oblivion's own shader class) */
  if (!::RuntimeSources.Get())
    return Shader;

  /* we have some replacement resource directly from the LUT, and we're going to pass that */
  RuntimeShaderRecord *hit = Shaders[(IUnknown *)Shader];
  if (hit)
    return hit->GetRuntimeShader(Shader);

  /* that didn't go so well */
  return Shader;
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
