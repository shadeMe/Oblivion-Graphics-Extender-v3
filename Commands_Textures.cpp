#include "Commands_Textures.h"
#include "Commands_Misc.h"
#include "TextureManager.h"
#include "Commands_Params.h"
#include "OBSEShaderInterface.h"

static bool LoadTexture_Execute(COMMAND_ARGS) {
  *result = 0;

  char path[256];
  DWORD NONPOW2;
  if (!ExtractArgs(EXTRACTARGS, &path, &NONPOW2))
    return true;

  if (!IsEnabled())
    return true;

#ifndef	NDEBUG
  if (!path[0]) {
    Console_Print("Texture-name is invalid");
    *result = -1; return true;
  }
#endif

  *result = TextureManager::GetSingleton()->LoadManagedTexture(path, TR_PLANAR, !!NONPOW2);
  return true;
}

static bool LoadCubeTexture_Execute(COMMAND_ARGS) {
  *result = 0;

  char path[256];
  DWORD NONPOW2;
  if (!ExtractArgs(EXTRACTARGS, &path, &NONPOW2))
    return true;

  if (!IsEnabled())
    return true;

#ifndef	NDEBUG
  if (!path[0]) {
    Console_Print("Texture-name is invalid");
    *result = -1; return true;
  }
#endif

  *result = TextureManager::GetSingleton()->LoadManagedTexture(path, TR_CUBIC, !!NONPOW2);
  return true;
}

static bool LoadVolumeTexture_Execute(COMMAND_ARGS) {
  *result = 0;

  char path[256];
  DWORD NONPOW2;
  if (!ExtractArgs(EXTRACTARGS, &path, &NONPOW2))
    return true;

  if (!IsEnabled())
    return true;

#ifndef	NDEBUG
  if (!path[0]) {
    Console_Print("Texture-name is invalid");
    *result = -1; return true;
  }
#endif

  *result = TextureManager::GetSingleton()->LoadManagedTexture(path, TR_VOLUMETRIC, !!NONPOW2);
  return true;
}

static bool ReleaseTexture_Execute(COMMAND_ARGS) {
  *result = 0;

  DWORD id;
  if (!ExtractArgs(EXTRACTARGS, &id))
    return true;

  if (!IsEnabled())
    return true;

#ifndef	NDEBUG
  if (id < 0) {
    Console_Print("Texture-ID %i is invalid", id);
    *result = -1; return true;
  }
#endif

  TextureManager::GetSingleton()->ReleaseTexture(id);
  return true;
}

static bool PurgeManagedTextures_Execute(COMMAND_ARGS) {
  *result = 0;

  if (!IsEnabled())
    return true;

  GetD3DDevice()->EvictManagedResources();
  return true;
}

CommandInfo kCommandInfo_LoadTexture = {
  "LoadTexture",
  "",
  0,
  "Loads a shared texture for use in effects, shaders or HUD elements",
  0,
  2,
  kParams_StringInt,
  LoadTexture_Execute,
  0,
  0,
  0
};

CommandInfo kCommandInfo_LoadCubeTexture = {
  "LoadCubeTexture",
  "",
  0,
  "Loads a shared cube-texture for use in effects, shaders or HUD elements",
  0,
  2,
  kParams_StringInt,
  LoadCubeTexture_Execute,
  0,
  0,
  0
};

CommandInfo kCommandInfo_LoadVolumeTexture = {
  "LoadVolumeTexture",
  "",
  0,
  "Loads a shared volume-texture for use in effects, shaders or HUD elements",
  0,
  2,
  kParams_StringInt,
  LoadVolumeTexture_Execute,
  0,
  0,
  0
};

CommandInfo kCommandInfo_ReleaseTexture = {
  "ReleaseTexture",
  "",
  0,
  "Reduces the usage-counter of a shared texture by one",
  0,
  1,
  kParams_OneInt,
  ReleaseTexture_Execute,
  0,
  0,
  0
};

CommandInfo kCommandInfo_PurgeManagedTextures = {
  "PurgeManagedTextures",
  "",
  0,
  "Evicts managed resources from vram",
  0,
  0,
  0,
  PurgeManagedTextures_Execute,
  0,
  0,
  0
};

/* deprecated */
#ifndef	NO_DEPRECATED
CommandInfo kCommandInfo_FreeTexture = {
  "FreeTexture",
  "",
  0,
  "Reduces the usage-counter of a shared texture by one [deprecated]",
  0,
  1,
  kParams_OneInt,
  ReleaseTexture_Execute,
  0,
  0,
  0
};
#endif
