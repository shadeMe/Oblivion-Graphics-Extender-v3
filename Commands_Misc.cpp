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

#include "Commands_Misc.h"
#include "Commands_Params.h"
#include "nodes\NiDX9Renderer.h"
#include "Rendering.h"
#include "OBSEShaderInterface.h"

#include "D3D9.hpp"
#include "D3D9Device.hpp"

#include "GUIs_DebugWindow.hpp"

void NotImplemented(void) {
  _MESSAGE("This command is not yet implemented.");
  return;
}

static bool GetAvailableGraphicsMemory_Execute(COMMAND_ARGS) {
  if (IsEnabled())
    *result = GetD3DDevice()->GetAvailableTextureMem();
  else
    *result = 0;

  return true;
}

static bool GetScreenWidth_Execute(COMMAND_ARGS) {
  v1_2_416::NiDX9Renderer *renderer = v1_2_416::GetRenderer();

  if (IsEnabled() && renderer)
    *result = renderer->SizeWidth;
  else {
    _MESSAGE("Can't locate renderer data.");
    *result = -1;
  }

  return true;
}

static bool GetScreenHeight_Execute(COMMAND_ARGS) {
  v1_2_416::NiDX9Renderer *renderer = v1_2_416::GetRenderer();

  if (IsEnabled() && renderer)
    *result = renderer->SizeHeight;
  else {
    _MESSAGE("Can't locate renderer data.");
    *result = -1;
  }

  return true;
}
static bool ForceGraphicsReset_Execute(COMMAND_ARGS) {
  NotImplemented();
  *result = 0;
  return true;
}

#ifdef	OBGE_LOGGING
static bool DumpFrameScript_Execute(COMMAND_ARGS) {
  if (IsEnabled())
    ((OBGEDirect3DDevice9 *)GetD3DDevice())->DumpFrameScript();

  *result = 0;
  return true;
}
static bool DumpFrameSurfaces_Execute(COMMAND_ARGS) {
  if (IsEnabled())
    ((OBGEDirect3DDevice9 *)GetD3DDevice())->DumpFrameSurfaces();

  *result = 0;
  return true;
}
#endif

#ifdef	OBGE_DEVLING
static bool OpenShaderDeveloper_Execute(COMMAND_ARGS) {
  if (IsEnabled())
    DebugWindow::Create();

  *result = 0;
  return true;
}
#endif

CommandInfo kCommandInfo_GetAvailableGraphicsMemory = {
  "GetAvailableGraphicsMemory",
  "",
  0,
  "Returns an approximate amount of remaining graphics memory",
  0,
  0,
  0,
  GetAvailableGraphicsMemory_Execute,
  0,
  0,
  0
};
CommandInfo kCommandInfo_GetScreenWidth = {
  "GetScreenWidth",
  "",
  0,
  "Returns the x resolution of the backbuffer",
  0,
  0,
  0,
  GetScreenWidth_Execute,
  0,
  0,
  0
};
CommandInfo kCommandInfo_GetScreenHeight = {
  "GetScreenHeight",
  "",
  0,
  "Returns the y resolution of the backbuffer",
  0,
  0,
  0,
  GetScreenHeight_Execute,
  0,
  0,
  0
};
CommandInfo kCommandInfo_ForceGraphicsReset = {
  "ForceGraphicsReset",
  "",
  0,
  "Resets the graphics device in the same way as alt-tabbing",
  0,
  0,
  0,
  ForceGraphicsReset_Execute,
  0,
  0,
  0
};

#ifdef	OBGE_LOGGING
CommandInfo kCommandInfo_DumpFrameScript = {
  "DumpFrameScript",
  "",
  0,
  "Prints the logged script of the next frame",
  0,
  0,
  0,
  DumpFrameScript_Execute,
  0,
  0,
  0
};

CommandInfo kCommandInfo_DumpFrameSurfaces = {
  "DumpFrameSurfaces",
  "",
  0,
  "Prints the logged surface-resources",
  0,
  0,
  0,
  DumpFrameSurfaces_Execute,
  0,
  0,
  0
};
#endif

#ifdef	OBGE_DEVLING
CommandInfo kCommandInfo_OpenShaderDeveloper = {
  "OpenShaderDeveloper",
  "",
  0,
  "Opens the shader-developer interface in another window",
  0,
  0,
  0,
  OpenShaderDeveloper_Execute,
  0,
  0,
  0
};
#endif
