#include "Commands_Shaders.h"
#include "Commands_Misc.h"
#include "Commands_Params.h"
#include "EffectManager.h"
#include "Script.h"
#include "OBSEShaderInterface.h"

static bool LoadEffect_Execute(COMMAND_ARGS) {
  *result = 0;

  char path[256];
  int Private = 0;
  if (!ExtractArgs(EXTRACTARGS, &path, &Private))
    return true;

  if (!IsEnabled())
    return true;

#ifndef	NDEBUG
  if (!path[0]) {
    Console_Print("Effect-name is invalid");
    *result = -1; return true;
  }
#endif

  _MESSAGE("Effect (%s) - Script refID = %x %s", path, scriptObj->refID, (scriptObj->refID == 0) ? "(Error NULL refID)" : " ");

  if (Private)
    *result = EffectManager::GetSingleton()->AddPrivateEffect(path, scriptObj->refID);
  else
    *result = EffectManager::GetSingleton()->AddManagedEffect(path, scriptObj->refID);

  return true;
}

static bool EnableEffect(COMMAND_ARGS) {
  *result = 0;

  DWORD id, HUD;
  if (!ExtractArgs(EXTRACTARGS, &id, &HUD))
    return true;

  if (!IsEnabled())
    return true;

#ifndef	NDEBUG
  if (id < 0) {
    Console_Print("EffectID %i is invalid", id);
    *result = -1; return true;
  }
#endif

  if (!EffectManager::GetSingleton()->EnableEffect(id, true))
    *result = -1;

  return true;
}

static bool DisableEffect(COMMAND_ARGS) {
  *result = 0;

  DWORD id;
  int Delete = 0;
  if (!ExtractArgs(EXTRACTARGS, &id))
    return true;

  if (!IsEnabled())
    return true;

#ifndef	NDEBUG
  if (id < 0) {
    Console_Print("EffectID %i is invalid", id);
    *result = -1; return true;
  }
#endif

  if (!EffectManager::GetSingleton()->EnableEffect(id, false))
    *result = -1;

  return true;
}

static bool ReleaseEffect(COMMAND_ARGS) {
  *result = 0;

  DWORD id;
  int Delete = 0;
  if (!ExtractArgs(EXTRACTARGS, &id))
    return true;

  if (!IsEnabled())
    return true;

#ifndef	NDEBUG
  if (id < 0) {
    Console_Print("EffectID %i is invalid", id);
    *result = -1; return true;
  }
#endif

  if (!EffectManager::GetSingleton()->ReleaseEffect(id))
    *result = -1;

  return true;
}

static bool SetEffectConstantB_Execute(COMMAND_ARGS) {
  *result = 0;

  DWORD id;
  char var[256];
  int i;
  if (!ExtractArgs(EXTRACTARGS, &id, &var, &i))
    return true;

  if (!IsEnabled())
    return true;

#ifndef	NDEBUG
  if (id < 0) {
    Console_Print("EffectID %i is invalid", id);
    *result = -1; return true;
  }
#endif

  EffectManager::GetSingleton()->SetEffectConstantB(id, var, !!i);
  return true;
}

static bool SetEffectConstantI_Execute(COMMAND_ARGS) {
  *result = 0;

  DWORD id;
  char var[256];
  int i;
  if (!ExtractArgs(EXTRACTARGS, &id, &var, &i))
    return true;

  if (!IsEnabled())
    return true;

#ifndef	NDEBUG
  if (id < 0) {
    Console_Print("EffectID %i is invalid", id);
    *result = -1; return true;
  }
#endif

  EffectManager::GetSingleton()->SetEffectConstantI(id, var, i);
  return true;
}

static bool SetEffectConstantF_Execute(COMMAND_ARGS) {
  *result = 0;

  DWORD id;
  char var[256];
  float f;
  if (!ExtractArgs(EXTRACTARGS, &id, &var, &f))
    return true;

  if (!IsEnabled())
    return true;

#ifndef	NDEBUG
  if (id < 0) {
    Console_Print("EffectID %i is invalid", id);
    *result = -1; return true;
  }
#endif

  EffectManager::GetSingleton()->SetEffectConstantF(id, var, f);
  return true;
}

static bool SetEffectConstantV_Execute(COMMAND_ARGS) {
  *result = 0;

  DWORD id;
  char var[256];
  v1_2_416::NiVector4 v;
  if (!ExtractArgs(EXTRACTARGS, &id, &var, &v[0], &v[1], &v[2], &v[3]))
    return true;

  if (!IsEnabled())
    return true;

#ifndef	NDEBUG
  if (id < 0) {
    Console_Print("EffectID %i is invalid", id);
    *result = -1; return true;
  }
#endif

  EffectManager::GetSingleton()->SetEffectConstantV(id, var, &v);
  return true;
}

static bool SetEffectSamplerTexture_Execute(COMMAND_ARGS) {
  *result = 0;

  DWORD id;
  char var[256];
  DWORD i;
  if (!ExtractArgs(EXTRACTARGS, &id, &var, &i))
    return true;

  if (!IsEnabled())
    return true;

#ifndef	NDEBUG
  if (id < 0) {
    Console_Print("EffectID %i is invalid", id);
    *result = -1; return true;
  }

  if (i < 0) {
    Console_Print("TextureID %i is invalid", i);
    *result = -1; return true;
  }
#endif

  EffectManager::GetSingleton()->SetEffectSamplerTexture(id, var, i);
  return true;
}

static bool IsEffectEnabled_Execute(COMMAND_ARGS) {
  *result = 0;

  DWORD id;
  if (!ExtractArgs(EXTRACTARGS, &id))
    return true;

#ifndef	NDEBUG
  if (id < 0) {
    Console_Print("EffectID %i is invalid", id);
    *result = -1; return true;
  }
#endif

  *result = EffectManager::GetSingleton()->GetEffectState(id);
  return true;
}

CommandInfo kCommandInfo_LoadEffect = {
  "LoadEffect",
  "",
  0,
  "Loads an effect file. (Must be in the .fx format)",
  0,
  2,
  kParams_StringOptInt,
  LoadEffect_Execute,
  0,
  0,
  0
};

CommandInfo kCommandInfo_EnableEffect = {
  "EnableEffect",
  "",
  0,
  "Applies an effect to oblivion",
  0,
  2,
  kParams_OneIntOneOptInt,
  EnableEffect,
  0,
  0,
  0
};

CommandInfo kCommandInfo_DisableEffect = {
  "DisableEffect",
  "",
  0,
  "Removes an effect from oblivion",
  0,
  1,
  kParams_OneInt,
  DisableEffect,
  0,
  0,
  0
};

CommandInfo kCommandInfo_ReleaseEffect = {
  "ReleaseEffect",
  "",
  0,
  "Reduces the usage-counter of a shared effect by one",
  0,
  1,
  kParams_OneInt,
  ReleaseEffect,
  0,
  0,
  0
};

CommandInfo kCommandInfo_SetEffectConstantB = {
  "SetEffectConstantB",
  "",
  0,
  "Sets a boolean variable in an effect",
  0,
  3,
  kParams_IntStringInt,
  SetEffectConstantB_Execute,
  0,
  0,
  0
};

CommandInfo kCommandInfo_SetEffectConstantI = {
  "SetEffectConstantI",
  "",
  0,
  "Sets an integer variable in an effect",
  0,
  3,
  kParams_IntStringInt,
  SetEffectConstantI_Execute,
  0,
  0,
  0
};

CommandInfo kCommandInfo_SetEffectConstantF = {
  "SetEffectConstantF",
  "",
  0,
  "Sets a float variable in an effect",
  0,
  3,
  kParams_IntStringFloat,
  SetEffectConstantF_Execute,
  0,
  0,
  0
};

CommandInfo kCommandInfo_SetEffectConstantV = {
  "SetEffectConstantV",
  "",
  0,
  "Sets an array of 4 floats in an effect",
  0,
  6,
  kParams_IntStringFloat4,
  SetEffectConstantV_Execute,
  0,
  0,
  0
};

CommandInfo kCommandInfo_SetEffectSamplerTexture = {
  "SetEffectSamplerTexture",
  "",
  0,
  "Sets a texture variable in an effect",
  0,
  3,
  kParams_IntStringInt,
  SetEffectSamplerTexture_Execute,
  0,
  0,
  0
};

CommandInfo kCommandInfo_IsEffectEnabled = {
  "IsEffectEnabled",
  "",
  0,
  "Returns the state of the loaded effect",
  0,
  1,
  kParams_OneInt,
  IsEffectEnabled_Execute,
  0,
  0,
  0
};

/* deprecated */
#ifndef	NO_DEPRECATED
static bool LoadShader_Execute(COMMAND_ARGS) {
  *result = 0;

  DWORD id;
  char path[256];
  int AllowDuplicates = 0;
  if (!ExtractArgs(EXTRACTARGS, &path, &AllowDuplicates))
    return true;

  if (!IsEnabled())
    return true;

  _MESSAGE("Shader (%s) - Script refID = %x %s", path, scriptObj->refID, (scriptObj->refID == 0) ? "(Error NULL refID)" : " ");

  if ((id = EffectManager::GetSingleton()->AddManagedEffect(path, scriptObj->refID)) != -1)
    EffectManager::GetSingleton()->EnableEffect(id, true);

  *result = id;
  return true;
}

CommandInfo kCommandInfo_LoadShader = {
  "LoadShader",
  "",
  0,
  "Loads an effect file. (Must be in the .fx format) [deprecated]",
  0,
  2,
  kParams_StringOptInt,
  LoadShader_Execute,
  0,
  0,
  0
};

CommandInfo kCommandInfo_ApplyFullscreenShader = {
  "ApplyFullscreenShader",
  "",
  0,
  "Applies a fullscreen effect to oblivion [deprecated]",
  0,
  2,
  kParams_OneIntOneOptInt,
  EnableEffect,
  0,
  0,
  0
};

CommandInfo kCommandInfo_RemoveFullscreenShader = {
  "RemoveFullscreenShader",
  "",
  0,
  "Removes a fullscreen effect from oblivion [deprecated]",
  0,
  2,
  kParams_OneIntOneOptInt,
  DisableEffect,
  0,
  0,
  0
};

CommandInfo kCommandInfo_SetShaderInt = {
  "SetShaderInt",
  "",
  0,
  "Sets an integer variable in a fullscreen shader [deprecated]",
  0,
  3,
  kParams_IntStringInt,
  SetEffectConstantI_Execute,
  0,
  0,
  0
};

CommandInfo kCommandInfo_SetShaderFloat = {
  "SetShaderFloat",
  "",
  0,
  "Sets a float variable in a fullscreen shader [deprecated]",
  0,
  3,
  kParams_IntStringFloat,
  SetEffectConstantF_Execute,
  0,
  0,
  0
};

CommandInfo kCommandInfo_SetShaderVector = {
  "SetShaderVector",
  "",
  0,
  "Sets an array of 4 floats in a fullscreen shader [deprecated]",
  0,
  6,
  kParams_IntStringFloat4,
  SetEffectConstantV_Execute,
  0,
  0,
  0
};

CommandInfo kCommandInfo_SetShaderTexture = {
  "SetShaderTexture",
  "",
  0,
  "Sets a texture variable in a fullscreen shader [deprecated]",
  0,
  3,
  kParams_IntStringInt,
  SetEffectSamplerTexture_Execute,
  0,
  0,
  0
};

CommandInfo kCommandInfo_IsShaderEnabled = {
  "IsShaderEnabled",
  "",
  0,
  "Returns the state of the loaded shader [deprecated]",
  0,
  1,
  kParams_OneInt,
  IsEffectEnabled_Execute,
  0,
  0,
  0
};

#endif
