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

#include "obse/GameAPI.h"
#include "obse/PluginAPI.h"

extern OBSESerializationInterface *g_serialization;
extern OBSEArrayVarInterface	  *g_arrayvar;

typedef OBSEArrayVarInterface::Array	OBSEArray;
typedef OBSEArrayVarInterface::Element	OBSEElement;

OBSEArray* StringMapFromStdMap(const std::map<std::string, OBSEElement>& data, Script* callingScript);
OBSEArray* MapFromStdMap(const std::map<double, OBSEElement>& data, Script* callingScript);
OBSEArray* ArrayFromStdVector(const std::vector<OBSEElement>& data, Script* callingScript);

#define EXTRACTARGS paramInfo, arg1, opcodeOffsetPtr, thisObj, arg3, scriptObj, eventList

static ParamInfo kParams_OneString[1] =
{
  { "string",	kParamType_String,	0 },
};

static ParamInfo kParams_OneInt[1] =
{
  { "int", kParamType_Integer, 0 },
};

static ParamInfo kParams_OneOptionalInt[1] =
{
  { "int", kParamType_Integer, 1 }, 
};

static ParamInfo kParams_OneIntOneOptInt[2] =
{
  { "int", kParamType_Integer, 0 },
  { "int", kParamType_Integer, 1 },
};

static ParamInfo kParams_TwoInt[2] =
{
  { "int", kParamType_Integer, 0 },
  { "int", kParamType_Integer, 0 },
};

static ParamInfo kParams_IntFloat[2] =
{
  { "int", kParamType_Integer, 0 },
  { "float", kParamType_Float, 0 },
};

static ParamInfo kParams_IntString[2] =
{
  { "int", kParamType_Integer, 0 },
  { "string", kParamType_String, 0 }
};

static ParamInfo kParams_Int2Floats[3] =
{
  { "int", kParamType_Integer, 0 },
  { "float", kParamType_Float, 0 },
  { "float", kParamType_Float, 0 },
};

static ParamInfo kParams_Int3Floats[4] =
{
  { "int", kParamType_Integer, 0 },
  { "float", kParamType_Float, 0 },
  { "float", kParamType_Float, 0 },
  { "float", kParamType_Float, 0 },
};

static ParamInfo kParams_StringInt[2] =
{
  { "string", kParamType_String, 0 },
  { "int", kParamType_Integer, 0 },
};

static ParamInfo kParams_StringOptInt[2] =
{
  { "string", kParamType_String, 0 },
  { "int", kParamType_Integer, 1 },
};

static ParamInfo kParams_IntStringInt[3] =
{
  { "int", kParamType_Integer, 0 },
  { "string", kParamType_String, 0 },
  { "int", kParamType_Integer, 0 },
};

static ParamInfo kParams_IntStringFloat[3] =
{
  { "int", kParamType_Integer, 0 },
  { "string", kParamType_String, 0 },
  { "float", kParamType_Float, 0 },
};

static ParamInfo kParams_StringStringInt[3] =
{
  { "string", kParamType_String, 0 },
  { "string", kParamType_String, 0 },
  { "int", kParamType_Integer, 0 },
};

static ParamInfo kParams_IntStringInt4[6] =
{
  { "int", kParamType_Integer, 0 },
  { "string", kParamType_String, 0 },
  { "int", kParamType_Integer, 0 },
  { "int", kParamType_Integer, 0 },
  { "int", kParamType_Integer, 0 },
  { "int", kParamType_Integer, 0 },
};

static ParamInfo kParams_IntStringFloat4[6] =
{
  { "int", kParamType_Integer, 0 },
  { "string", kParamType_String, 0 },
  { "float", kParamType_Float, 0 },
  { "float", kParamType_Float, 0 },
  { "float", kParamType_Float, 0 },
  { "float", kParamType_Float, 0 },
};

static ParamInfo kParams_StringStringInt4[6] =
{
  { "string", kParamType_String, 0 },
  { "string", kParamType_String, 0 },
  { "int", kParamType_Integer, 0 },
  { "int", kParamType_Integer, 0 },
  { "int", kParamType_Integer, 0 },
  { "int", kParamType_Integer, 0 },
};

static ParamInfo kParams_StringStringFloat4[6] =
{
  { "string", kParamType_String, 0 },
  { "string", kParamType_String, 0 },
  { "float", kParamType_Float, 0 },
  { "float", kParamType_Float, 0 },
  { "float", kParamType_Float, 0 },
  { "float", kParamType_Float, 0 },
};
