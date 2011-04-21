#include "ShaderManager.h"
#include "GlobalSettings.h"
#include "D3D9.hpp"
#include <sys/stat.h>

static global<bool> UseShaderOverride(true,NULL,"Shaders","bUseShaderOverride");
static global<bool> SaveShaderOverride(true,NULL,"Shaders","bSaveShaderOverride");
static global<bool> CompileSources(false,NULL,"Shaders","bCompileSources");
static global<bool> Optimize(false,NULL,"Shaders","bOptimize");
static global<bool> MaximumSM(false,NULL,"Shaders","bMaximumSM");
static global<bool> UpgradeSM(false,NULL,"Shaders","bUpgradeSM1X");
static global<char*> ShaderOverrideDirectory("data\\shaders\\override\\",NULL,"Shaders","sShaderOverrideDirectory");

// *********************************************************************************************************

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
      case D3DXINC_LOCAL: strcpy(strFileFull, ShaderOverrideDirectory.Get()); strcat(strFileFull, pName); break;
      case D3DXINC_SYSTEM: strcpy(strFileFull, ""); strcat(strFileFull, pName); break;
    }

    if (stat((const char *)strFileFull, &s))
      return E_FAIL;

    UINT size = s.st_size;

    BYTE* pData = new BYTE[size];
    if (pData == NULL)
        return E_OUTOFMEMORY;

    if (fopen_s(&f, strFileFull, "rt"))
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

// *********************************************************************************************************

ShaderRecord::ShaderRecord() {
  pShaderReplaced = NULL;
  pConstantsReplaced = NULL;
  pShaderOriginal = NULL;
  pConstantsOriginal = NULL;
  pErrorMsgs = NULL;
  pDisassembly = NULL;
  pProfile = NULL;
  pAssembly = NULL;
  pSource = NULL;
  assemblyLen = 0;
  sourceLen = 0;
}

ShaderRecord::~ShaderRecord() {
  if (pShaderReplaced)
    pShaderReplaced->Release();
  if (pConstantsReplaced)
    pConstantsReplaced->Release();
  if (pShaderOriginal)
    pShaderOriginal->Release();
  if (pConstantsOriginal)
    pConstantsOriginal->Release();
  if (pErrorMsgs)
    pErrorMsgs->Release();
  if (pDisassembly)
    pDisassembly->Release();
  if (pProfile)
    delete[] pProfile;
  if (pAssembly)
    delete[] pAssembly;
  if (pSource)
    delete[] pSource;
}

bool ShaderRecord::LoadShader(char *Filename) {
  struct stat sb, sv, sa, sh;
  char strFileFull[MAX_PATH];
  FILE *f;

  strcpy(Name, Filename);

  strcpy(Filepath, ShaderOverrideDirectory.Get());
  strcat(Filepath, Filename);
  if (!stat((const char *)Filepath, &sb)) {
    UINT size = sb.st_size;
    if (D3DXCreateBuffer(size, &pShaderReplaced) == D3D_OK) {
      if (!fopen_s(&f, Filepath, "rb")) {
	fread(pShaderReplaced->GetBufferPointer(), 1, size, f);
	fclose(f);

        _MESSAGE("Loaded binary of %s from %s", Name, Filepath);

	if (D3DXGetShaderConstantTable(
	  (const DWORD *)pShaderReplaced->GetBufferPointer(),
	  &pConstantsReplaced
	) != D3D_OK) {
	  pConstantsReplaced = NULL;
	}
      }
      else {
	pShaderReplaced->Release();
	pShaderReplaced = NULL;
      }
    }
  }

  strcpy(strFileFull, ShaderOverrideDirectory.Get());
  strcat(strFileFull, Filename);
  strcat(strFileFull, ".version");
  if (!stat((const char *)strFileFull, &sv) && !MaximumSM.Get()) {
    UINT size = sv.st_size;
    pProfile = new CHAR[size + 1];
    if (pProfile != NULL) {
      /* reading in text-mode can yield any number of less characters */
      memset(pProfile, 0, size + 1);

      if (!fopen_s(&f, strFileFull, "rt")) {
        fread(pProfile, 1, size, f);
        fclose(f);

        _MESSAGE("Loaded version of %s from %s", Name, Filepath);

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

  strcpy(strFileFull, ShaderOverrideDirectory.Get());
  strcat(strFileFull, Filename);
  strcat(strFileFull, ".asm");
  if (!stat((const char *)strFileFull, &sa)) {
    UINT size = sa.st_size;
    pAssembly = new CHAR[size + 1];
    if (pAssembly != NULL) {
      /* reading in text-mode can yield any number of less characters */
      memset(pAssembly, 0, size + 1);

      if (!fopen_s(&f, strFileFull, "rt")) {
        fread(pAssembly, 1, size, f);
        fclose(f);

        _MESSAGE("Loaded assembly of %s from %s", Name, Filepath);

        assemblyLen = strlen(pAssembly);
      }
      else {
      	delete[] pAssembly;

      	pAssembly = NULL;
        assemblyLen = 0;
      }
    }

    if (pAssembly && (sa.st_mtime > sb.st_mtime)) {
      if (pShaderReplaced)
	pShaderReplaced->Release();
      if (pConstantsReplaced)
	pConstantsReplaced->Release();

      pShaderReplaced = NULL;
      pConstantsReplaced = NULL;
    }
  }

  strcpy(strFileFull, ShaderOverrideDirectory.Get());
  strcat(strFileFull, Filename);
  strcat(strFileFull, ".hlsl");
  if (!stat((const char *)strFileFull, &sh)) {
    UINT size = sh.st_size;
    pSource = new CHAR[size + 1];
    if (pSource != NULL) {
      /* reading in text-mode can yield any number of less characters */
      memset(pSource, 0, size + 1);

      if (!fopen_s(&f, strFileFull, "rt")) {
        fread(pSource, 1, size, f);
        fclose(f);

        _MESSAGE("Loaded source of %s from %s", Name, Filepath);

	sourceLen = strlen(pSource);
      }
      else {
      	delete[] pSource;

      	pSource = NULL;
        sourceLen = 0;
      }
    }

    if (pSource && (sh.st_mtime > sb.st_mtime)) {
      if (pShaderReplaced)
	pShaderReplaced->Release();
      if (pConstantsReplaced)
	pConstantsReplaced->Release();

      pShaderReplaced = NULL;
      pConstantsReplaced = NULL;
    }
  }

  if (!pProfile && lastOBGEDirect3DDevice9) {
    if (strstr(Filename, ".vso")) {
      pProfile = strdup(D3DXGetVertexShaderProfile(lastOBGEDirect3DDevice9));
    }
    else if (strstr(Filename, ".pso")) {
      pProfile = strdup(D3DXGetPixelShaderProfile(lastOBGEDirect3DDevice9));
    }
  }

  return TRUE;
}

bool ShaderRecord::SaveShader() {
  if (!SaveShaderOverride.Get())
    return FALSE;

  if (pShaderReplaced) {
    FILE *f;

    if (!fopen_s(&f, Filepath, "wb")) {
      fwrite(pShaderReplaced->GetBufferPointer(), 1, pShaderReplaced->GetBufferSize(), f);
      fclose(f);

      _MESSAGE("Saved binary of %s to %s", Name, Filepath);

      return TRUE;
    }
  }

  return FALSE;
}

bool ShaderRecord::AssembleShader() {
  if (!CompileSources.Get())
    return FALSE;

  if (!pShaderReplaced && pAssembly) {
    if (pErrorMsgs)
      pErrorMsgs->Release();

    D3DXAssembleShader(
      pAssembly,
      assemblyLen,
      NULL,
      &incl,
      D3DXSHADER_DEBUG,
      &pShaderReplaced,
      &pErrorMsgs
    );

    if (pErrorMsgs) {
      _MESSAGE("Shader assembling errors occured in %s:", Filepath);
      _MESSAGE((char*)pErrorMsgs->GetBufferPointer());
    }
  }

  if (!pConstantsReplaced && pShaderReplaced) {
    D3DXGetShaderConstantTable(
      (const DWORD *)pShaderReplaced->GetBufferPointer(),
      &pConstantsReplaced
    );
  }

  SaveShader();

  return (pShaderReplaced != NULL);
}

bool ShaderRecord::CompileShader() {
  if (!CompileSources.Get())
    return FALSE;

  if (!pShaderReplaced && pSource) {
    if (pErrorMsgs)
      pErrorMsgs->Release();

    D3DXCompileShader(
	pSource,
	sourceLen,
	NULL,
	&incl,
	"main",
	pProfile,
	D3DXSHADER_DEBUG | (Optimize.Get() ? D3DXSHADER_OPTIMIZATION_LEVEL3 : 0),
	&pShaderReplaced,
	&pErrorMsgs,
	&pConstantsReplaced
    );

    if (pErrorMsgs && strstr((char*)pErrorMsgs->GetBufferPointer(), "X3539")) {
      pErrorMsgs->Release();
      pErrorMsgs = NULL;

      D3DXCompileShader(
	  pSource,
	  sourceLen,
	  NULL,
	  &incl,
	  "main",
	  pProfile,
	  D3DXSHADER_DEBUG | (UpgradeSM.Get() ? D3DXSHADER_ENABLE_BACKWARDS_COMPATIBILITY : D3DXSHADER_USE_LEGACY_D3DX9_31_DLL),
	  &pShaderReplaced,
	  &pErrorMsgs,
	  &pConstantsReplaced
      );
    }

    if (pErrorMsgs) {
      _MESSAGE("Shader compiling errors occured in %s:", Filepath);
      _MESSAGE((char*)pErrorMsgs->GetBufferPointer());
    }
  }

  SaveShader();

  return (pShaderReplaced != NULL);
}

bool ShaderRecord::DisassembleShader() {
  bool succ = TRUE;

  if (pAssembly)
    succ = AssembleShader();
  else if (pSource)
    succ = CompileShader();

  if (succ) {
    if (pDisassembly)
      pDisassembly->Release();

    if (pShaderReplaced)
      D3DXDisassembleShader(
        (const DWORD *)pShaderReplaced->GetBufferPointer(),
	FALSE,
	NULL,
	&pDisassembly
      );
    else if (pShaderOriginal)
      D3DXDisassembleShader(
	(const DWORD *)pShaderOriginal->GetBufferPointer(),
	FALSE,
	NULL,
	&pDisassembly
      );
  }

  return (pDisassembly != NULL);
}

void ShaderRecord::ApplyCompileDirectives(void) {
}

void ShaderRecord::SetBinary(int len, void *org) {
  if (pShaderOriginal)
    delete pShaderOriginal;

  if (D3DXCreateBuffer(len, &pShaderOriginal) == D3D_OK) {
    memcpy(pShaderOriginal->GetBufferPointer(), org, len);

    if (D3DXGetShaderConstantTable((const DWORD *)org, &pConstantsOriginal) != D3D_OK) {
      pConstantsOriginal = NULL;
    }
  }
}

void *ShaderRecord::GetBinary() {
  if (!pShaderReplaced)
    AssembleShader();
  if (!pShaderReplaced)
    CompileShader();

  if (pShaderReplaced)
    return pShaderReplaced->GetBufferPointer();
  if (pShaderOriginal)
    return pShaderOriginal->GetBufferPointer();

  return NULL;
}


// *********************************************************************************************************

ShaderManager *ShaderManager::Singleton=NULL;

ShaderManager::ShaderManager()
{
}

ShaderManager::~ShaderManager()
{
	Singleton=NULL;
}

ShaderManager*	ShaderManager::GetSingleton()
{
	if (UseShaderOverride.Get()) {
		_MESSAGE("Replacing the built-in shaders.");
		if(GetFileAttributes(ShaderOverrideDirectory.Get()) == INVALID_FILE_ATTRIBUTES)
		{
			_MESSAGE("Override directory %s doesn't exist, deactivating feature.", ShaderOverrideDirectory.Get());
			UseShaderOverride.Set(FALSE);
			return NULL;
		}
	}

	if(!ShaderManager::Singleton)
		ShaderManager::Singleton=new(ShaderManager);

	return(ShaderManager::Singleton);
}

ShaderRecord *ShaderManager::GetBuiltInShader(char *Filename)
{
	BuiltInShaderList::iterator BShader=BuiltInShaders.begin();
	while(BShader!=BuiltInShaders.end())
	{
		if(!_stricmp((*BShader)->Filepath,Filename))
		{
			return(*BShader);
		}
		BShader++;
	}

	ShaderRecord	*NewShader=NULL;
	NewShader=new(ShaderRecord);

	if(!NewShader->LoadShader(Filename))
	{
		delete(NewShader);
		return NULL;
	}

	BuiltInShaders.push_back(NewShader);
	return NewShader;
}
