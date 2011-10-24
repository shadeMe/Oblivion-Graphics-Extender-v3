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

#ifndef	D3D9IDENTIFIERS_HPP
#define	D3D9IDENTIFIERS_HPP

#define D3DXFX_LARGEADDRESS_HANDLE
#include <d3d9.h>
#include <d3dx9.h>

#ifndef	NDEBUG
#define	_DMESSAGE	_MESSAGE
#else
#define	_DMESSAGE(...)	0
#endif

/* ------------------------------------------------------------------------------- */

#ifndef	OBGE_NOSHADER

/* these are build-switches from now on */
//efine	OBGE_LOGGING
#undef	OBGE_HOOKING
#define	OBGE_STATEBLOCKS	0	// 0 = just hint
#define	OBGE_CONSTANTPOOLS	0	// 0 = just hint
#define	OBGE_GAMMACORRECTION	0	// 0 = just hint, 1 = apply globally
#define	OBGE_LODSHADERS		0	// 0 = just hint, 1 = apply globally
#define	OBGE_CUSTOMPASS		0	// 0 = just hint, 1 = apply globally
#define	OBGE_ANISOTROPY		0	// 0 = just hint
#define	OBGE_AUTOMIPMAP		0	// 0 = just hint, 1 = call GenerateMipSubLevels
#define	OBGE_TRACKER		0	// replace by OBGE-implementation, 0 = only rendertargets, 1 = all
#undef	OBGE_TRACKER_SURFACES		// replace by OBGE-implementation, 0 = only rendertargets, 1 = all
#undef	OBGE_TRACKER_TEXTURES		// replace by OBGE-implementation, 0 = only rendertargets, 1 = all

//efine	OBGE_DEVLING
#ifdef	OBGE_DEVLING
#define	OBGE_PROFILE			// enable profiling of times
#define	OBGE_TESSELATION		// enable visualizing tesselation
#endif

enum OBGEPass {
  OBGEPASS_ANY			= 0x0,

  OBGEPASS_REFLECTION		= 0x1,	// off screen-space
  OBGEPASS_WATER	 	= 0x2,	// off screen-space
  OBGEPASS_WATERHEIGHTMAP	= 0x3,	// off screen-space
  OBGEPASS_WATERDISPLACEMENT 	= 0x4,	// off screen-space
  OBGEPASS_SHADOW		= 0x5,	// off screen-space
  OBGEPASS_MAIN			= 0x6,	// missing
  OBGEPASS_CUSTOM		= 0x6,	// missing
  OBGEPASS_EFFECTS		= 0x7,	// on screen-space
  OBGEPASS_HDR			= 0x8,	// on screen-space, hdr
  OBGEPASS_POST			= 0x9,	// on screen-space, blur, hit, menu, etc.

  OBGEPASS_VIDEO		= 0xA,	// bink
  OBGEPASS_UNKNOWN		= 0xB,

  OBGEPASS_MIN			= OBGEPASS_ANY + 1,
  OBGEPASS_MAX			= OBGEPASS_UNKNOWN,
  OBGEPASS_NUM			= OBGEPASS_UNKNOWN + 1,
};

extern const char *passNames[OBGEPASS_NUM];
extern const char *passScens[OBGEPASS_NUM][16];
#endif

/* ------------------------------------------------------------------------------- */

#define	CODE_INTZ	(D3DFORMAT)MAKEFOURCC('I','N','T','Z')
#define	CODE_DF24	(D3DFORMAT)MAKEFOURCC('D','F','2','4')
#define	CODE_DF16	(D3DFORMAT)MAKEFOURCC('D','F','1','6')
#define	CODE_RAWZ	(D3DFORMAT)MAKEFOURCC('R','A','W','Z')
#define	CODE_RESZ	(D3DFORMAT)MAKEFOURCC('R','E','S','Z')
#define	CODE_NULL	(D3DFORMAT)MAKEFOURCC('N','U','L','L')

const char *findShader(void *iface, UINT len, const DWORD* buf);
const char *findShader(void *iface);
const char *findFormat(D3DFORMAT fmt);
const char *findUsage(DWORD use);
const char *findTextureState(D3DTEXTURESTAGESTATETYPE tstate);
const char *findSamplerState(D3DSAMPLERSTATETYPE sstate);
const char *findSamplerStateValue(D3DSAMPLERSTATETYPE sstate, DWORD svalue);
const char *findRenderState(D3DRENDERSTATETYPE rstate);
const char *findRenderStateValue(D3DRENDERSTATETYPE rstate, DWORD rvalue);
const char *findFVF(DWORD FVF);

#endif
