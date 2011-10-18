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

#pragma once

#define D3DXFX_LARGEADDRESS_HANDLE
#include <d3dx9.h>

#include "SpoofShader.h"
#include "TextBuffer.h"
#include "EffectManager.h"
#include <stdio.h>
#include "obse\PluginAPI.h"

// This class overloads the SpoofShader class and renders the memory dump to the screen.
// TO DO: Tidy up this class. Protect internal vars.

class OBSEShaderInterface : public SpoofShader
{
public:

	static OBSEShaderInterface	*GetSingleton(void);	// Use this to access the OBSEShaderInterface object. If the object doesn't exist
	static OBSEShaderInterface	*Singleton;		// then one will be created. DO NOT USE new.

	virtual void			ShaderCode(IDirect3DDevice9 *D3DDevice, IDirect3DSurface9 *RenderTo, IDirect3DSurface9 *RenderFrom, DeviceInfo *Info);
	virtual void			InitializeEffects(void);
	virtual void			DeviceLost(void);
	virtual void			DeviceReset(void);
	virtual void			DeviceRelease(void);		// Basically the deconstructor. Kept seperate do I can't confuse with "Oblivion's" deconstructor.
	virtual void			NewGame(void);
	virtual void			LoadGame(OBSESerializationInterface *Interface);
	virtual void			SaveGame(OBSESerializationInterface *Interface);

	TextBuffer 			*MemoryDumpString;

	float rcpres[2];

	LPD3DXFONT			pFont;
	LPD3DXFONT			pFont2;
	D3DXFONT_DESC			FontDescription;
	D3DXFONT_DESC			FontDescription2;
	RECT				FontRect;
	D3DCOLOR			FontColor;
	D3DCOLOR			FontColor2;

	bool				DebugOn;
};

static pSpoofShaderList *obImageSpaceShaderList = (pSpoofShaderList *)0x00B42D7C;
static void (*obAddImageSpaceShader)(SpoofShader *) = (void(*)(SpoofShader *))0x00803790;

bool LostDevice(bool stage,void *parameters);

static OBSEMessagingInterface* messanger = NULL;
static PluginHandle handle = kPluginHandle_Invalid;

void SetMessaging(OBSEMessagingInterface *Interface, PluginHandle Handle);
OBSEMessagingInterface	*GetMessaging(void);
PluginHandle		 GetHandle(void);
bool IsEnabled();
bool IsHDR();
bool IsBloom();
bool IsPlain();
