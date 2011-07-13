#include <sys/stat.h>

#include <algorithm>

#include "TextureManager.h"
#include "ShaderManager.h"
#include "EffectManager.h"
#include "ScreenElements.h"
#include "obse\pluginapi.h"
#include "GlobalSettings.h"

#include "D3D9.hpp"
#include "D3D9Device.hpp"

static global<bool> PurgeOnNewGame(false, NULL, "Textures", "bPurgeOnNewGame");
static global<bool> DoDeGamma(false, NULL, "Textures", "bSamplerDeGamma");
static global<bool> DoReGamma(false, NULL, "Textures", "bSamplerReGamma");
static global<int> SetAnisotropy(1, NULL, "Textures", "iSamplerAnisotropy");
static global<float> SetLODBias(0.0, NULL, "Textures", "fSamplerLODBias");

// *********************************************************************************************************

TextureRecord::TextureRecord() {
  type = TR_PLANAR;
  texture = NULL;
  Filepath[0] = 0;
  Private = false;
}

TextureRecord::~TextureRecord() {
  Kill();
}

bool TextureRecord::LoadTexture(TextureRecordType type, const char *fp, bool NONPOW2, bool Private) {
  if (type == TR_PLANAR) {
    IDirect3DTexture9 *tex = NULL;
    HRESULT res;

    if (!NONPOW2 || FAILED(res = D3DXCreateTextureFromFileEx(
                 lastOBGEDirect3DDevice9,
                 fp,
                 D3DX_DEFAULT_NONPOW2,
                 D3DX_DEFAULT_NONPOW2,
                 D3DX_FROM_FILE,
                 0,
                 D3DFMT_UNKNOWN,
                 D3DPOOL_MANAGED,
                 D3DX_DEFAULT,
                 D3DX_DEFAULT,
                 0,
                 0,
                 0,
                 &tex)) || !tex)
      if (FAILED(res = D3DXCreateTextureFromFile(
                   lastOBGEDirect3DDevice9,
                   fp,
                   &tex)) || !tex)
        return false;

    SetTexture(tex, fp, NONPOW2, Private);
  }
  else if (type == TR_CUBIC) {
    IDirect3DCubeTexture9 *tex = NULL;
    HRESULT res;

    if (!NONPOW2 || FAILED(res = D3DXCreateCubeTextureFromFileEx(
                 lastOBGEDirect3DDevice9,
                 fp,
                 D3DX_DEFAULT_NONPOW2,
                 D3DX_FROM_FILE,
                 0,
                 D3DFMT_UNKNOWN,
                 D3DPOOL_MANAGED,
                 D3DX_DEFAULT,
                 D3DX_DEFAULT,
                 0,
                 0,
                 0,
                 &tex)) || !tex)
      if (FAILED(res = D3DXCreateCubeTextureFromFile(lastOBGEDirect3DDevice9, fp, &tex)) || !tex)
        return false;

    SetTexture(tex, fp, NONPOW2, Private);
  }
  else if (type == TR_VOLUMETRIC) {
    IDirect3DVolumeTexture9 *tex = NULL;
    HRESULT res;

    if (!NONPOW2 || FAILED(res = D3DXCreateVolumeTextureFromFileEx(
                 lastOBGEDirect3DDevice9,
                 fp,
                 D3DX_DEFAULT_NONPOW2,
                 D3DX_DEFAULT_NONPOW2,
                 D3DX_DEFAULT_NONPOW2,
                 D3DX_FROM_FILE,
                 0,
                 D3DFMT_UNKNOWN,
                 D3DPOOL_MANAGED,
                 D3DX_DEFAULT,
                 D3DX_DEFAULT,
                 0,
                 0,
                 0,
		 &tex)) || !tex) {
      if (FAILED(res = D3DXCreateVolumeTextureFromFile(
                   lastOBGEDirect3DDevice9,
                   fp,
		   &tex)) || !tex) {

#if 0
	struct stat vf; FILE *f; void *data;
	if (!stat((const char *)fp, &vf)) {
	  if ((data = malloc(vf.st_size))) {
	    if (!fopen_s(&f, fp, "rb")) {
	      fread(data, 1, vf.st_size, f);
	      fclose(f);

	      if (!NONPOW2 || FAILED(res = D3DXCreateVolumeTextureFromFileInMemoryEx(
		lastOBGEDirect3DDevice9,
		data,
		vf.st_size,
		D3DX_DEFAULT_NONPOW2,
		D3DX_DEFAULT_NONPOW2,
		D3DX_DEFAULT_NONPOW2,
		D3DX_FROM_FILE,
		0,
		D3DFMT_UNKNOWN,
		D3DPOOL_MANAGED,
		D3DX_DEFAULT,
		D3DX_DEFAULT,
		0,
		0,
		0,
		&tex)) || !tex) {
		  if (FAILED(res = D3DXCreateVolumeTextureFromFileInMemory(
		    lastOBGEDirect3DDevice9,
		    data,
		    vf.st_size,
		    &tex)) || !tex) {
		  }
	      }
	    }

	    free(data);
	  }
	}
#endif

	if (FAILED(res))
	  return false;
      }
    }

    SetTexture(tex, fp, NONPOW2, Private);
  }

  return true;
}

void TextureRecord::SetTexture(IDirect3DTexture9 *tex, const char *fp, bool NONPOW2, bool Private) {
  this->type = TR_PLANAR;
  this->textureP = tex;

  strcpy_s(this->Filepath, 256, fp);

  this->NONPOW2 = NONPOW2;
  this->Private = Private;
}

void TextureRecord::SetTexture(IDirect3DCubeTexture9 *tex, const char *fp, bool NONPOW2, bool Private) {
  this->type = TR_CUBIC;
  this->textureC = tex;

  strcpy_s(this->Filepath, 256, fp);

  this->NONPOW2 = NONPOW2;
  this->Private = Private;
}

void TextureRecord::SetTexture(IDirect3DVolumeTexture9 *tex, const char *fp, bool NONPOW2, bool Private) {
  this->type = TR_VOLUMETRIC;
  this->textureV = tex;

  strcpy_s(this->Filepath, 256, fp);

  this->NONPOW2 = NONPOW2;
  this->Private = Private;
}

IDirect3DBaseTexture9 *TextureRecord::GetTexture() const {
  return this->texture;
}

TextureRecordType TextureRecord::GetType() const {
  return this->type;
}

bool TextureRecord::IsType(TextureRecordType type) const {
  return (this->type == type);
}

bool TextureRecord::HasTexture(char *path) const {
  return !strcmp(this->Filepath, path);
}

bool TextureRecord::HasTexture(IDirect3DBaseTexture9 *tex) const {
  return (this->texture == tex);
}

bool TextureRecord::HasTexture(IDirect3DTexture9 *tex) const {
  return this->IsType(TR_PLANAR) && (this->textureP == tex);
}

bool TextureRecord::HasTexture(IDirect3DCubeTexture9 *tex) const {
  return this->IsType(TR_CUBIC) && (this->textureC == tex);
}

bool TextureRecord::HasTexture(IDirect3DVolumeTexture9 *tex) const {
  return this->IsType(TR_VOLUMETRIC) && (this->textureV == tex);
}

bool TextureRecord::HasTexture() const {
  return (this->texture != NULL);
}

const char *TextureRecord::GetPath() const {
  return this->Filepath;
}

bool TextureRecord::IsNONPOW2() const {
  return this->NONPOW2;
}

bool TextureRecord::IsPrivate() const {
  return this->Private;
}

void TextureRecord::Purge(int TexNum) {
  if (this->IsType(TR_PLANAR)) {
       HUDManager::GetSingleton()->PurgeTexture(this->textureP, TexNum);
    EffectManager::GetSingleton()->PurgeTexture(this->textureP, TexNum);
    ShaderManager::GetSingleton()->PurgeTexture(this->textureP, TexNum);
  }
  else if (this->IsType(TR_CUBIC)) {
       HUDManager::GetSingleton()->PurgeTexture(this->textureC, TexNum);
    EffectManager::GetSingleton()->PurgeTexture(this->textureC, TexNum);
    ShaderManager::GetSingleton()->PurgeTexture(this->textureC, TexNum);
  }
  else if (this->IsType(TR_VOLUMETRIC)) {
    HUDManager::GetSingleton()->PurgeTexture(this->textureV, TexNum);
    EffectManager::GetSingleton()->PurgeTexture(this->textureV, TexNum);
    ShaderManager::GetSingleton()->PurgeTexture(this->textureV, TexNum);
  }
}

void TextureRecord::Kill() {
  if (this->IsType(TR_PLANAR)) {
    if (textureP)
      while (textureP->Release()) {}
  }
  else if (this->IsType(TR_CUBIC)) {
    if (textureC)
      while (textureC->Release()) {}
  }
  else if (this->IsType(TR_VOLUMETRIC)) {
    if (textureV)
      while (textureV->Release()) {}
  }

  this->texture = NULL;
  this->Filepath[0] = '\0';
}

// *********************************************************************************************************

ManagedTextureRecord::ManagedTextureRecord() {
  RefCount = 0;
}

ManagedTextureRecord::~ManagedTextureRecord() {
}

void ManagedTextureRecord::ClrRef() {
  RefCount = 0;
}

int ManagedTextureRecord::AddRef() {
  return ++RefCount;
}

int ManagedTextureRecord::Release() {
  if (RefCount)
    RefCount--;

  if (!RefCount) {
//  delete this;
    return 0;
  }

  return RefCount;
}

// *********************************************************************************************************

TextureManager::TextureManager() {
  TextureIndex = 0;
  MaxTextureIndex = 0;

#ifndef	OBGE_NOSHADER
#ifdef OBGE_GAMMACORRECTION
  DeGamma = ::DoDeGamma.Get();
  ReGamma = ::DoReGamma.Get();

  Constants.Gamma.x = DeGamma ? 1.0 : 0.0;
  Constants.Gamma.y = DeGamma ? 2.2 : 1.0;
  Constants.Gamma.z = ReGamma ? 1.0 : 0.0;
  Constants.Gamma.w = ReGamma ? 2.2 : 1.0;
#endif

#ifdef	OBGE_ANISOTROPY
  Anisotropy = ::SetAnisotropy.Get();
  LODBias = ::SetLODBias.Get();

  if (lastOBGEDirect3D9) {
    if (Anisotropy > lastOBGEDirect3D9CAPS.MaxAnisotropy)
      Anisotropy = lastOBGEDirect3D9CAPS.MaxAnisotropy;
  }

  if (Anisotropy <= 1)
    AFilters = 0;
#endif
#endif
}

TextureManager::~TextureManager() {
  Singleton = NULL;
}

TextureManager *TextureManager::Singleton = NULL;

TextureManager *TextureManager::GetSingleton() {
  if (!Singleton)
    Singleton = new TextureManager();

  return Singleton;
}

void TextureManager::Clear() {
  ManagedTextures.clear();
  Textures.clear();

  TextureIndex = 0;
  MaxTextureIndex = 0;
}

int TextureManager::LoadPrivateTexture(const char *Filename, TextureRecordType type, bool NONPOW2) {
  if (strlen(Filename) > 240)
    return NULL;

  char NewPath[256];
  strcpy_s(NewPath, 256, "data\\textures\\");
  strcat_s(NewPath, 256, Filename);

  _MESSAGE("Loading texture (%s)", NewPath);

  ManagedTextureRecord *NewTex = new ManagedTextureRecord();

  if (!NewTex->LoadTexture(type, NewPath, NONPOW2, true)) {
    delete NewTex;
    return -1;
  }

  /* prepare */
  NewTex->AddRef();

  /* append and sort */
  ManagedTextures.push_back(NewTex);

  /* register */
  Textures[TextureIndex++] = NewTex;

  return TextureIndex - 1;
}

int TextureManager::LoadManagedTexture(const char *Filename, TextureRecordType type, bool NONPOW2) {
  if (strlen(Filename) > 240)
    return NULL;

  char NewPath[256];
  strcpy_s(NewPath, 256, "data\\textures\\");
  strcat_s(NewPath, 256, Filename);

  _MESSAGE("Loading texture (%s)", NewPath);

  TextureRegistry::iterator Texture = Textures.begin();

  /* search for non-private texture */
  while (Texture != Textures.end()) {
    if (!Texture->second->IsPrivate()) {
      if (!_stricmp(NewPath, Texture->second->GetPath())/*&& ((Effect->second->ParentRefID & 0xff000000) == (refID & 0xff000000))*/) {
        _MESSAGE("Linking to existing texture.");

        Texture->second->AddRef();
        return Texture->first;
      }
    }

    Texture++;
  }

  ManagedTextureRecord *NewTex = new ManagedTextureRecord();

  if (!NewTex->LoadTexture(type, NewPath, NONPOW2, false)) {
    delete NewTex;
    return -1;
  }

  /* prepare */
  NewTex->AddRef();

  /* append and sort */
  ManagedTextures.push_back(NewTex);

  /* register */
  Textures[TextureIndex++] = NewTex;

  return TextureIndex - 1;
}

int TextureManager::LoadDependtTexture(const char *Filename, TextureRecordType type, bool NONPOW2) {
  if (strlen(Filename) > 240)
    return NULL;

  char NewPath[256];
  strcpy_s(NewPath, 256, "data\\textures\\");
  strcat_s(NewPath, 256, Filename);

  _MESSAGE("Loading texture (%s)", NewPath);

  TextureRegistry::iterator Texture = Textures.begin();

  /* search for any texture */
  while (Texture != Textures.end()) {
    if (!_stricmp(NewPath, Texture->second->GetPath())/*&& ((Effect->second->ParentRefID & 0xff000000) == (refID & 0xff000000))*/) {
      _MESSAGE("Linking to existing texture.");

      Texture->second->AddRef();
      return Texture->first;
    }

    Texture++;
  }

  ManagedTextureRecord *NewTex = new ManagedTextureRecord();

  if (!NewTex->LoadTexture(type, NewPath, NONPOW2, true)) {
    delete NewTex;
    return -1;
  }

  /* prepare */
  NewTex->AddRef();

  /* append and sort */
  ManagedTextures.push_back(NewTex);

  /* register */
  Textures[TextureIndex++] = NewTex;

  return TextureIndex - 1;
}

bool TextureManager::ReleaseTexture(int TextureNum) {
  if (!IsTextureValid(TextureNum))
    return false;

  _DMESSAGE("Releasing managed texture.");

  ManagedTextureRecord *OldTexture = Textures[TextureNum];

  /* reached zero */
  if (!OldTexture->Release()) {
    OldTexture->Purge(TextureNum);

    /* remove from map */
    Textures.erase(TextureNum);
    /* remove from vector */
    ManagedTextures.erase(std::find(ManagedTextures.begin(), ManagedTextures.end(), OldTexture));

    _DMESSAGE("and removing it from memory.");
    delete OldTexture;

    return true;
  }

  return false;
}

template<>
int TextureManager::FindTexture(IDirect3DBaseTexture9 *texture) {
  TextureRegistry::iterator Texture = Textures.begin();

  while (Texture != Textures.end()) {
    if (Texture->second->HasTexture(texture))
      return Texture->first;

    Texture++;
  }

  return -1;
}

template<>
bool TextureManager::ReleaseTexture(IDirect3DBaseTexture9 *texture) {
  return ReleaseTexture(FindTexture<IDirect3DBaseTexture9>(texture));
}

void TextureManager::FreeTexture(int TextureNum) {
  if (!IsTextureValid(TextureNum)) {
    _MESSAGE("Tried to free a non existant texture");
    return;
  }

  ManagedTextureRecord *OldTexture = Textures[TextureNum];

  if (OldTexture->HasTexture()) {
    OldTexture->Purge(TextureNum);

    /* remove from map */
    Textures.erase(TextureNum);
    /* remove from vector */
    ManagedTextures.erase(std::find(ManagedTextures.begin(), ManagedTextures.end(), OldTexture));

    _DMESSAGE("Freeing %s", OldTexture->GetPath());
    delete OldTexture;

    return;
  }

  return;
}

void TextureManager::NewGame() {
  if (PurgeOnNewGame.Get()) {
    /* prevent locking up because other resources have allready been freed */
    TextureRegistry prevTextures = Textures; Textures.clear();
    TextureRegistry::iterator Texture = prevTextures.begin();

    while (Texture != prevTextures.end()) {
      ManagedTextureRecord *OldTexture = Texture->second;

      if (OldTexture->HasTexture())
        OldTexture->Purge(Texture->first);

      delete OldTexture;

      Texture++;
    }

    Clear();
  }
  else {
    TextureRegistry::iterator Texture = Textures.begin();

    /* disable and remove all refs */
    while (Texture != Textures.end()) {
      /* TODO ClrRef clears enabled as well, logical, no? */
      if (!Texture->second->IsPrivate())
	Texture->second->ClrRef();

      /* delete private shaders I guess */
      Texture++;
    }
  }
}

void TextureManager::SaveGame(OBSESerializationInterface *Interface) {
  int temp;

  _MESSAGE("TextureManager::SaveGame");

  TextureRegistry::iterator Texture = Textures.begin();

  Interface->WriteRecord('TIDX', TEXTUREVERSION, &TextureIndex, sizeof(TextureIndex));

  _MESSAGE("Save-game will reference %i textures.", TextureIndex);

  while (Texture != Textures.end()) {
    if (!Texture->second->IsPrivate()) {
      if (Texture->second->HasTexture()) {
        const char *path = Texture->second->GetPath();
        const bool fromfile = Texture->second->IsNONPOW2();
        const TextureRecordType type = Texture->second->GetType();

        Interface->WriteRecord('TNUM', TEXTUREVERSION, &Texture->first, sizeof(Texture->first));
        Interface->WriteRecord('TPAT', TEXTUREVERSION, path, strlen(path) + 1);
        Interface->WriteRecord('TFFL', TEXTUREVERSION, &fromfile, sizeof(fromfile));
        Interface->WriteRecord('TTYP', TEXTUREVERSION, &type, sizeof(type));

#ifdef	NO_DEPRECATED
        Interface->WriteRecord('TEOD', TEXTUREVERSION, &temp, 1);
#endif
      }
    }

    Texture++;
  }

#ifdef	NO_DEPRECATED
  Interface->WriteRecord('TEOF', TEXTUREVERSION, &temp, 1);
#endif
}

void TextureManager::LoadGame(OBSESerializationInterface *Interface) {
  int maxtex = 0;
  UInt32 type, version, length;
  int OldTextureNum = -1;
  int TextureNum = -1;
  bool NONPOW2;

  /* obtain a copy of the old resources */
  TextureRegistry prevTextures = Textures;
  TextureRecordType TextureType;
  char TexturePath[260];

  Interface->GetNextRecordInfo(&type, &version, &length);

  if (type == 'TIDX') {
    Interface->ReadRecordData(&maxtex, length);
    _MESSAGE("Save-game references to %i textures.", maxtex);
  }
  else {
    _MESSAGE("No texture data found in save-game.");
    return;
  }

#ifdef	NO_DEPRECATED
  Interface->GetNextRecordInfo(&type, &version, &length);

  while (type != 'TEOF') {
    if (type == 'TNUM') {
      OldTextureNum = TextureNum;
      Interface->ReadRecordData(&TextureNum, length);
      _MESSAGE("Found TNUM record = %i.", TextureNum);
    }
    else {
      _MESSAGE("Error loading game. type!=TNUM");
      return;
    }
#else
  while (TextureNum < (maxtex - 1)) {
    Interface->GetNextRecordInfo(&type, &version, &length);

    if (type == 'TNUM') {
      OldTextureNum = TextureNum;
      Interface->ReadRecordData(&TextureNum, length);
      _MESSAGE("Found TNUM record = %i.", TextureNum);
#endif

      Interface->GetNextRecordInfo(&type, &version, &length);

      if (type == 'TPAT') {
        Interface->ReadRecordData(TexturePath, length);
        _MESSAGE("Found TPAT record = %s", TexturePath);
      }
      else {
        _MESSAGE("Error loading texture list. type!=TPAT");
        return;
      }

      Interface->GetNextRecordInfo(&type, &version, &length);

      if (type == 'TFFL') {
        Interface->ReadRecordData(&NONPOW2, length);
        _MESSAGE("Found TFFL record = %i", NONPOW2);
      }
      else {
        _MESSAGE("Error loading texture list. type!=TFFL");
        return;
      }

      Interface->GetNextRecordInfo(&type, &version, &length);

      if (type == 'TTYP') {
        Interface->ReadRecordData(&TextureType, length);
        _MESSAGE("Found TTYP record = %i", TextureType);
      }
      else {
        TextureType = TR_PLANAR;
        _MESSAGE("Error loading texture list. type!=TTYP");
      }

      if (LoadManagedTexture(TexturePath, TextureType, NONPOW2) == -1) {
        _MESSAGE("Error loading texture list: texture (%s) no longer exists.", TexturePath);
      }

#ifdef	NO_DEPRECATED
      else {
	Interface->GetNextRecordInfo(&type, &version, &length);
	while (type != 'SEOD') {
	  Interface->ReadRecordData(TexturePath, length);
	  Interface->GetNextRecordInfo(&type, &version, &length);
	}
      }

      Interface->GetNextRecordInfo(&type, &version, &length);
#else
    }
    else {
      _MESSAGE("Error loading texture list: too small.");
      return;
    }
#endif
  }

  /* release previous non-private textures, those are supposed
   * to be handled entirely by the game-save
   */
  TextureRegistry::iterator PTexture = prevTextures.begin();

  while (PTexture != prevTextures.end()) {
    if (PTexture->second &&
       !PTexture->second->IsPrivate())
      ReleaseTexture(PTexture->first);

    PTexture++;
  }
}

#ifdef OBGE_GAMMACORRECTION
void TextureManager::DoDeGamma(bool enable) {
  ::DoDeGamma.Set(enable);

  DeGamma = enable;

  Constants.Gamma.x = DeGamma ? 1.0 : 0.0;
  Constants.Gamma.y = DeGamma ? 2.2 : 1.0;
}

void TextureManager::DoReGamma(bool enable) {
  ::DoReGamma.Set(enable);

  ReGamma = enable;

  Constants.Gamma.z = ReGamma ? 1.0 : 0.0;
  Constants.Gamma.w = ReGamma ? 2.2 : 1.0;
}

bool TextureManager::DoDeGamma() {
  return ::DoDeGamma.Get();
}

bool TextureManager::DoReGamma() {
  return ::DoReGamma.Get();
}
#endif

#ifdef	OBGE_ANISOTROPY
void TextureManager::SetAnisotropy(int af) {
  ::SetAnisotropy.Set(af);

  Anisotropy = af;
  if (lastOBGEDirect3D9) {
    if (Anisotropy > lastOBGEDirect3D9CAPS.MaxAnisotropy)
      Anisotropy = lastOBGEDirect3D9CAPS.MaxAnisotropy;
  }
}

void TextureManager::SetLODBias(float bias) {
  ::SetLODBias.Set(bias);

  LODBias = bias;
}

int TextureManager::SetAnisotropy() {
  return ::SetAnisotropy.Get();
}

float TextureManager::SetLODBias() {
  return ::SetLODBias.Get();
}
#endif
