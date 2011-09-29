/* Version: MPL 1.1/LGPL 3.0
 *
 * "The contents of this file are subject to the Mozilla Public License
 * Version 1.1 (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS"
 * basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
 * License for the specific language governing rights and limitations
 * under the License.
 *
 * The Original Code is the Oblivion Graphics Extender, short OBGE.
 *
 * The Initial Developer of the Original Code is
 * Ethatron <niels@paradice-insight.us>. Portions created by The Initial
 * Developer are Copyright (C) 2011 The Initial Developer.
 * All Rights Reserved.
 *
 * Contributor(s):
 *  Timeslip (Version 1)
 *  scanti (Version 2)
 *  IlmrynAkios (Version 3)
 *
 * Alternatively, the contents of this file may be used under the terms
 * of the GNU Library General Public License Version 3 license (the
 * "LGPL License"), in which case the provisions of LGPL License are
 * applicable instead of those above. If you wish to allow use of your
 * version of this file only under the terms of the LGPL License and not
 * to allow others to use your version of this file under the MPL,
 * indicate your decision by deleting the provisions above and replace
 * them with the notice and other provisions required by the LGPL License.
 * If you do not delete the provisions above, a recipient may use your
 * version of this file under either the MPL or the LGPL License."
 */

#include <string>
#include <vector>
#include <map>

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

static bool GetEffects_Execute(COMMAND_ARGS) {
  *result = 0;

  DWORD which;
  char var[256];
  if (!ExtractArgs(EXTRACTARGS, &which, &var))
    return true;

  if (!IsEnabled())
    return true;

  std::map<std::string,int> all;
  if (EffectManager::GetSingleton()->GetEffects(which, all)) {
    std::map<std::string,OBSEElement> obsem;

    for (std::map<std::string,int>::iterator it = all.begin(); it != all.end(); it++) {
      const std::string name = it->first;
      int id = it->second;

      obsem[name] = OBSEElement(id);
    }

    OBSEArray *arr = StringMapFromStdMap(obsem, scriptObj);
    if (g_arrayvar->AssignCommandResult(arr, result))
      ;
  }

  return true;
}

static bool GetEffectConstantHelps_Execute(COMMAND_ARGS) {
  *result = 0;

  DWORD id;
  char var[256];
  if (!ExtractArgs(EXTRACTARGS, &id, &var))
    return true;

  if (!IsEnabled())
    return true;

#ifndef	NDEBUG
  if (id < 0) {
    Console_Print("EffectID %i is invalid", id);
    *result = -1; return true;
  }
#endif

  std::map<std::string,std::string> all;
  if (EffectManager::GetSingleton()->GetEffectConstantHelps(id, all)) {
    std::map<std::string,OBSEElement> obsem;

    for (std::map<std::string,std::string>::iterator it = all.begin(); it != all.end(); it++) {
      const std::string name = it->first;
      const char *type = it->second.data();

      obsem[name] = OBSEElement(type);
    }

    OBSEArray *arr = StringMapFromStdMap(obsem, scriptObj);
    if (g_arrayvar->AssignCommandResult(arr, result))
      ;
  }

  return true;
}

static bool GetEffectConstantTypes_Execute(COMMAND_ARGS) {
  *result = 0;

  DWORD id;
  char var[256];
  if (!ExtractArgs(EXTRACTARGS, &id, &var))
    return true;

  if (!IsEnabled())
    return true;

#ifndef	NDEBUG
  if (id < 0) {
    Console_Print("EffectID %i is invalid", id);
    *result = -1; return true;
  }
#endif

  std::map<std::string,int> all;
  if (EffectManager::GetSingleton()->GetEffectConstantTypes(id, all)) {
    std::map<std::string,OBSEElement> obsem;

    for (std::map<std::string,int>::iterator it = all.begin(); it != all.end(); it++) {
      const std::string name = it->first;
      int type = it->second;

      obsem[name] = OBSEElement(type);
    }

    OBSEArray *arr = StringMapFromStdMap(obsem, scriptObj);
    if (g_arrayvar->AssignCommandResult(arr, result))
      ;
  }

  return true;
}

static bool GetEffectConstantHelp_Execute(COMMAND_ARGS) {
  *result = 0;

  DWORD id;
  char var[256];
  if (!ExtractArgs(EXTRACTARGS, &id, &var))
    return true;

  if (!IsEnabled())
    return true;

#ifndef	NDEBUG
  if (id < 0) {
    Console_Print("EffectID %i is invalid", id);
    *result = -1; return true;
  }
#endif

  const char *help = "";
  if (EffectManager::GetSingleton()->GetEffectConstantHelp(id, var, &help))
    g_stringvar->Assign(PASS_COMMAND_ARGS, help);

  return true;
}

static bool GetEffectConstantType_Execute(COMMAND_ARGS) {
  *result = 0;

  DWORD id;
  char var[256];
  if (!ExtractArgs(EXTRACTARGS, &id, &var))
    return true;

  if (!IsEnabled())
    return true;

#ifndef	NDEBUG
  if (id < 0) {
    Console_Print("EffectID %i is invalid", id);
    *result = -1; return true;
  }
#endif

  int type;
  if (EffectManager::GetSingleton()->GetEffectConstantType(id, var, &type))
    *result = type;

  return true;
}

static bool GetEffectConstantB_Execute(COMMAND_ARGS) {
  *result = 0;

  DWORD id;
  char var[256];
  if (!ExtractArgs(EXTRACTARGS, &id, &var))
    return true;

  if (!IsEnabled())
    return true;

#ifndef	NDEBUG
  if (id < 0) {
    Console_Print("EffectID %i is invalid", id);
    *result = -1; return true;
  }
#endif

  bool value;
  if (EffectManager::GetSingleton()->GetEffectConstantB(id, var, &value))
    *result = value;

  return true;
}

static bool GetEffectConstantI_Execute(COMMAND_ARGS) {
  *result = 0;

  DWORD id;
  char var[256];
  if (!ExtractArgs(EXTRACTARGS, &id, &var))
    return true;

  if (!IsEnabled())
    return true;

#ifndef	NDEBUG
  if (id < 0) {
    Console_Print("EffectID %i is invalid", id);
    *result = -1; return true;
  }
#endif

  int value;
  if (EffectManager::GetSingleton()->GetEffectConstantI(id, var, &value))
    *result = value;

  return true;
}

static bool GetEffectConstantF_Execute(COMMAND_ARGS) {
  *result = 0;

  DWORD id;
  char var[256];
  if (!ExtractArgs(EXTRACTARGS, &id, &var))
    return true;

  if (!IsEnabled())
    return true;

#ifndef	NDEBUG
  if (id < 0) {
    Console_Print("EffectID %i is invalid", id);
    *result = -1; return true;
  }
#endif

  float value;
  if (EffectManager::GetSingleton()->GetEffectConstantF(id, var, &value))
    *result = value;

  return true;
}

static bool GetEffectConstantV_Execute(COMMAND_ARGS) {
  *result = 0;

  DWORD id;
  char var[256];
  if (!ExtractArgs(EXTRACTARGS, &id, &var))
    return true;

  if (!IsEnabled())
    return true;

#ifndef	NDEBUG
  if (id < 0) {
    Console_Print("EffectID %i is invalid", id);
    *result = -1; return true;
  }
#endif

  std::vector<float> value(4);
  if (EffectManager::GetSingleton()->GetEffectConstantV(id, var, &value[0])) {
    std::vector<OBSEElement> obsev(4);

    obsev[0] = value[0];
    obsev[1] = value[1];
    obsev[2] = value[2];
    obsev[3] = value[3];

    OBSEArray *arr = ArrayFromStdVector(obsev, scriptObj);
    if (g_arrayvar->AssignCommandResult(arr, result))
      ;
  }

  return true;
}

static bool GetEffectSamplerTexture_Execute(COMMAND_ARGS) {
  *result = 0;

  DWORD id;
  char var[256];
  if (!ExtractArgs(EXTRACTARGS, &id, &var))
    return true;

  if (!IsEnabled())
    return true;

#ifndef	NDEBUG
  if (id < 0) {
    Console_Print("EffectID %i is invalid", id);
    *result = -1; return true;
  }
#endif

  int i;
  if (EffectManager::GetSingleton()->GetEffectSamplerTexture(id, var, &i)) {
    *result = i;

#ifndef	NDEBUG
    if (i < 0) {
      Console_Print("TextureID %i is invalid", i);
      *result = -1; return true;
    }
#endif
  }

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

CommandInfo kCommandInfo_GetEffects = {
  "GetEffects",
  "",
  0,
  "Gets the ids of all effects",
  0,
  1,
  kParams_OneInt,
  GetEffects_Execute,
  0,
  0,
  0
};

CommandInfo kCommandInfo_GetEffectConstantHelps = {
  "GetEffectConstantHelps",
  "",
  0,
  "Gets the help-texts of all variables in an effect",
  0,
  1,
  kParams_OneInt,
  GetEffectConstantHelps_Execute,
  0,
  0,
  0
};

CommandInfo kCommandInfo_GetEffectConstantTypes = {
  "GetEffectConstantTypes",
  "",
  0,
  "Gets the types of all variables in an effect",
  0,
  1,
  kParams_OneInt,
  GetEffectConstantTypes_Execute,
  0,
  0,
  0
};

CommandInfo kCommandInfo_GetEffectConstantHelp = {
  "GetEffectConstantHelp",
  "",
  0,
  "Gets the help-text of a variable in an effect",
  0,
  2,
  kParams_IntString,
  GetEffectConstantHelp_Execute,
  0,
  0,
  0
};

CommandInfo kCommandInfo_GetEffectConstantType = {
  "GetEffectConstantType",
  "",
  0,
  "Gets the type of a variable in an effect",
  0,
  2,
  kParams_IntString,
  GetEffectConstantType_Execute,
  0,
  0,
  0
};

CommandInfo kCommandInfo_GetEffectConstantB = {
  "GetEffectConstantB",
  "",
  0,
  "Gets the boolean value of a variable in an effect",
  0,
  2,
  kParams_IntString,
  GetEffectConstantB_Execute,
  0,
  0,
  0
};

CommandInfo kCommandInfo_GetEffectConstantI = {
  "GetEffectConstantI",
  "",
  0,
  "Gets the integer value of a variable in an effect",
  0,
  2,
  kParams_IntString,
  GetEffectConstantI_Execute,
  0,
  0,
  0
};

CommandInfo kCommandInfo_GetEffectConstantF = {
  "GetEffectConstantF",
  "",
  0,
  "Gets the float value of a variable in an effect",
  0,
  2,
  kParams_IntString,
  GetEffectConstantF_Execute,
  0,
  0,
  0
};

CommandInfo kCommandInfo_GetEffectConstantV = {
  "GetEffectConstantV",
  "",
  0,
  "Gets the array of 4 floats of a variable in an effect",
  0,
  2,
  kParams_IntString,
  GetEffectConstantV_Execute,
  0,
  0,
  0
};

CommandInfo kCommandInfo_GetEffectSamplerTexture = {
  "GetEffectSamplerTexture",
  "",
  0,
  "Gets the id assigned to a texture variable in an effect",
  0,
  2,
  kParams_IntString,
  GetEffectSamplerTexture_Execute,
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
