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

#include "Commands_HUD.h"
#include "Commands_Misc.h"
#include "Commands_Params.h"
#include "ScreenElements.h"
#include "OBSEShaderInterface.h"

static bool CreateHUDElement_Execute(COMMAND_ARGS)
{
	if(IsEnabled())
	{
		Sprite	*TempSprite=new(Sprite);
		*result = HUDManager::GetSingleton()->AddScreenElement(TempSprite);
	}
	return true;
}

static bool SetHUDElementTexture_Execute(COMMAND_ARGS)
{

	*result=0;
	DWORD id;
	int tex;
	if(!ExtractArgs(EXTRACTARGS, &id, &tex)) return true;

	if(IsEnabled())
		HUDManager::GetSingleton()->index(id)->SetTexture(tex);

	return true;
}
static bool SetHUDElementColour_Execute(COMMAND_ARGS)
{
	*result=0;

	DWORD id;
	float r, g, b;
	if(!ExtractArgs(EXTRACTARGS, &id, &r, &g, &b)) return true;

	if(IsEnabled())
		HUDManager::GetSingleton()->index(id)->SetColor(r,g,b);

	return true;
}
static bool SetHUDElementPosition_Execute(COMMAND_ARGS)
{
	*result=0;

	DWORD id;
	float x, y;
	if(!ExtractArgs(EXTRACTARGS, &id, &x, &y)) return true;

	if(IsEnabled())
		HUDManager::GetSingleton()->index(id)->SetPosition(x,y,0);

	return true;
}
static bool SetHUDElementScale_Execute(COMMAND_ARGS)
{
	*result=0;

	DWORD id;
	float x, y;
	if(!ExtractArgs(EXTRACTARGS, &id, &x, &y)) return true;

	if(IsEnabled())
		HUDManager::GetSingleton()->index(id)->SetScale(x,y);

	return true;
}
static bool SetHUDElementRotation_Execute(COMMAND_ARGS)
{
	*result=0;

	DWORD id;
	float rot;
	if(!ExtractArgs(EXTRACTARGS, &id, &rot)) return true;

	if(IsEnabled())
		HUDManager::GetSingleton()->index(id)->SetRotation(rot);

	return true;
}

CommandInfo kCommandInfo_CreateHUDElement =
{
	"CreateHUDElement",
	"",
	0,
	"Creates a new HUD element",
	0,
	0,
	0,
	CreateHUDElement_Execute,
	0,
	0,
	0
};
CommandInfo kCommandInfo_SetHUDElementTexture =
{
	"SetHUDElementTexture",
	"",
	0,
	"Sets the texture of a HUD element",
	0,
	2,
	kParams_TwoInt,
	SetHUDElementTexture_Execute,
	0,
	0,
	0
};
CommandInfo kCommandInfo_SetHUDElementColour =
{
	"SetHUDElementColour",
	"",
	0,
	"Sets the colour of a HUD element",
	0,
	4,
	kParams_Int3Floats,
	SetHUDElementColour_Execute,
	0,
	0,
	0
};
CommandInfo kCommandInfo_SetHUDElementPosition =
{
	"SetHUDElementPosition",
	"",
	0,
	"Sets the position of a HUD element",
	0,
	3,
	kParams_Int2Floats,
	SetHUDElementPosition_Execute,
	0,
	0,
	0
};
CommandInfo kCommandInfo_SetHUDElementScale =
{
	"SetHUDElementScale",
	"",
	0,
	"Sets the scale of a HUD element",
	0,
	3,
	kParams_Int2Floats,
	SetHUDElementScale_Execute,
	0,
	0,
	0
};
CommandInfo kCommandInfo_SetHUDElementRotation =
{
	"SetHUDElementRotation",
	"",
	0,
	"Sets the rotation of a HUD element",
	0,
	2,
	kParams_IntFloat,
	SetHUDElementRotation_Execute,
	0,
	0,
	0
};