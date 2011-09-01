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
