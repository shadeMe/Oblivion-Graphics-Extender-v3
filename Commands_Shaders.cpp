#ifndef	OBGE_NOSHADER

#include "Commands_Shaders.h"
#include "Commands_Misc.h"
#include "Commands_Params.h"
#include "ShaderManager.h"
#include "Script.h"
#include "OBSEShaderInterface.h"

static bool SetShaderConstantB_Execute(COMMAND_ARGS) {
  *result = 0;

  char name[256];
  char var[256];
  int b;

  if (!ExtractArgs(EXTRACTARGS, &name, &var, &b))
    return true;

  if (!IsEnabled())
    return true;

#ifndef	NDEBUG
  if (!name[0]) {
    Console_Print("Shader-name is invalid");
    *result = -1; return true;
  }
#endif

  ShaderManager::GetSingleton()->SetShaderConstantB(name, var, !!b);
  return true;
}

static bool SetShaderConstantI_Execute(COMMAND_ARGS) {
  *result = 0;

  char name[256];
  char var[256];
  int i[4];

  if (!ExtractArgs(EXTRACTARGS, &name, &var, &i[0], &i[1], &i[2], &i[3]))
    return true;

  if (!IsEnabled())
    return true;

#ifndef	NDEBUG
  if (!name[0]) {
    Console_Print("Shader-name is invalid");
    *result = -1; return true;
  }
#endif

  ShaderManager::GetSingleton()->SetShaderConstantI(name, var, i);

  return true;
}

static bool SetShaderConstantF_Execute(COMMAND_ARGS) {
  *result = 0;

  char name[256];
  char var[256];
  float f[4];

  if (!ExtractArgs(EXTRACTARGS, &name, &var, &f[0], &f[1], &f[2], &f[3]))
    return true;

  if (!IsEnabled())
    return true;

#ifndef	NDEBUG
  if (!name[0]) {
    Console_Print("Shader-name is invalid");
    *result = -1; return true;
  }
#endif

  ShaderManager::GetSingleton()->SetShaderConstantF(name, var, f);
  return true;
}

static bool SetShaderSamplerTexture_Execute(COMMAND_ARGS) {
  *result = 0;

  char name[256];
  char var[256];
  DWORD t;

  if (!ExtractArgs(EXTRACTARGS, &name, &var, &t))
    return true;

  if (!IsEnabled())
    return true;

#ifndef	NDEBUG
  if (!name[0]) {
    Console_Print("Shader-name is invalid");
    *result = -1; return true;
  }

  if (t < 0) {
    Console_Print("Texture-ID %i is invalid", t);
    *result = -1; return true;
  }
#endif

  ShaderManager::GetSingleton()->SetShaderSamplerTexture(name, var, t);
  return true;
}

CommandInfo kCommandInfo_SetShaderConstantB = {
  "SetShaderConstantB",
  "",
  0,
  "Sets an boolean variable in a built-in shader",
  0,
  3,
  kParams_StringStringInt,
  SetShaderConstantB_Execute,
  0,
  0,
  0
};

CommandInfo kCommandInfo_SetShaderConstantI = {
  "SetShaderConstantI",
  "",
  0,
  "Sets an integer-vector variable in a built-in shader",
  0,
  6,
  kParams_StringStringInt4,
  SetShaderConstantI_Execute,
  0,
  0,
  0
};

CommandInfo kCommandInfo_SetShaderConstantF = {
  "SetShaderConstantF",
  "",
  0,
  "Sets an float-vector variable in a built-in shader",
  0,
  6,
  kParams_StringStringFloat4,
  SetShaderConstantF_Execute,
  0,
  0,
  0
};

CommandInfo kCommandInfo_SetShaderSamplerTexture = {
  "SetShaderSamplerTexture",
  "",
  0,
  "Sets a sampler-texture variable in a built-in shader",
  0,
  3,
  kParams_StringStringInt,
  SetShaderSamplerTexture_Execute,
  0,
  0,
  0
};

#endif
