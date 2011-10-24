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

#include <assert.h>

#include "TextureManager.h"
#include "TextureIOHook.hpp"
#include "windows.h"
#include "obse_common/SafeWrite.h"
#include "GlobalSettings.h"

#include "D3D9.hpp"
#include "D3D9Device.hpp"
#include "D3D9Identifiers.hpp"

#include "Hooking/detours/detours.h"
#include "Hooking/D3DX.hpp"

#if	defined(OBGE_LOGGING) || defined(OBGE_DEVLING) || defined(OBGE_GAMMACORRECTION)

std::map<std::string, IDirect3DBaseTexture9 *> textureFiles;
std::map<IDirect3DBaseTexture9 *, std::string> textureClass;
CRITICAL_SECTION textureLock;

/* ------------------------------------------------------------------------------------------------- */

#if	defined(OBGE_LOGGING) || defined(OBGE_DEVLING)
const char *findTexture(IDirect3DBaseTexture9 *tex) {
  static char buf[256];
  buf[0] = '\0';

  /* search loaded texture files */
  std::map<std::string, IDirect3DBaseTexture9 *>::iterator TFile = textureFiles.begin();
  while (TFile != textureFiles.end()) {
    if ((*TFile).second == tex) {
      std::string str = (*TFile).first;
      const char *ptr = str.data(), *ofs;

      if ((ofs = strstr(ptr, "Textures\\")) ||
	  (ofs = strstr(ptr, "textures\\")) ||
	  (ofs = strstr(ptr, "textures/"))) {
	sprintf(buf, "%s", ofs + 9);
      }
      else
	sprintf(buf, "%s", ptr);

      return buf;
    }

    TFile++;
  }

  return "unknown";
}
#endif

/* ------------------------------------------------------------------------------------------------- */

class Anonymous {
public:
	bool TrackLoadTextureFile(const char *texture, void *renderer, void *flags);
};

/* 00760DA0 == NiDX9SourceTextureData_LoadTextureFile */

bool (__thiscall Anonymous::* LoadTextureFile)(const char *, void *, void *)/* =
	(void * (__stdcall *)(char *))00760DA0*/;
bool (__thiscall Anonymous::* TrackLoadTextureFile)(const char *, void *, void *)/* =
	(void * (__stdcall *)(char *))00760DA0*/;

bool Anonymous::TrackLoadTextureFile(const char *texture, void *renderer, void *flags) {
	EnterCriticalSection(&textureLock);

	lastOBGEDirect3DBaseTexture9 = NULL;

	bool r = (this->*LoadTextureFile)(texture, renderer, flags);

	if (r && lastOBGEDirect3DBaseTexture9) {
	  textureFiles[texture] = lastOBGEDirect3DBaseTexture9;
	  textureClass[lastOBGEDirect3DBaseTexture9] = texture;

	  char *textlwr = strlwr(strdup(texture)), *slash;
	  while ((slash = strchr(textlwr, '\\')))
	    *slash = '/';
	  char *filelwr = strrchr(textlwr, '/');
	  if (filelwr) filelwr++;
	  else filelwr = textlwr;

	  /* regular textures ------------------------------------ */
	  if (!strstr(textlwr, "_g."))
	  if (!strstr(textlwr, "_n."))
	  if (!strstr(textlwr, "_d.")) {

#if	defined(OBGE_GAMMACORRECTION)
	    /* remember DeGamma for this kind of texture */
	  //if (DeGamma) {
	      char race[256]; int age;

	      if (/* menus are on the backbuffer, no shader there */
		  !strstr(textlwr, "menus/") &&
		  /* faces contain blend-factors, no colors */
		  !strstr(textlwr, "faces/") &&
		  /* fires are emitter, no need for gamma */
		  !strstr(textlwr, "fire/" ) &&
		  /* age-maps contain blend-factors, no colors */
		  (sscanf(filelwr, "head%[a-z]%d.dds", race, &age) != 2)) {
		static const bool PotDeGamma = true;

		lastOBGEDirect3DBaseTexture9->SetPrivateData(GammaGUID, &PotDeGamma, sizeof(PotDeGamma), 0);
	      }
//	    }
#endif

	  }

	  /* LOD textures ---------------------------------------- */
	  {

#if	defined(OBGE_LODSHADERS)
	    /* remember LOD for this kind of texture */
//	    if (DoLODReplacement) {
	      if (strstr(textlwr, "landscapelod") ||
		  strstr(textlwr, "lowres") ||
		  strstr(textlwr, "lodres")) {
		static const bool PotLODtext = true;

		lastOBGEDirect3DBaseTexture9->SetPrivateData(LODtxGUID, &PotLODtext, sizeof(PotLODtext), 0);
	      }
//	    }
#endif
	  }

	  free((void *)textlwr);
	}

	_DMESSAGE("Texture load: %s (%s)", texture, r ? "success" : "failed");

	LeaveCriticalSection(&textureLock);
	return r;
}

/* ------------------------------------------------------------------------------------------------- */

void CreateTextureIOHook(void) {
	/* GetTextureBinary */
	*((int *)&LoadTextureFile) = 0x00760DA0;
	TrackLoadTextureFile = &Anonymous::TrackLoadTextureFile;

        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());
	DetourAttach(&(PVOID&)LoadTextureFile, *((PVOID *)&TrackLoadTextureFile));
        LONG error = DetourTransactionCommit();

        if (error == NO_ERROR) {
		_MESSAGE("Detoured LoadTextureFile(); succeeded");
        }
        else {
		_MESSAGE("Detoured LoadTextureFile(); failed");
        }

	InitializeCriticalSection(&textureLock);

	return;
}

#endif
