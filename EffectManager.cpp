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

#include <sys/stat.h>

#include <string>
#include <map>
#include <algorithm>

#include "EffectManager.h"
#include "TextureManager.h"
#include "GlobalSettings.h"
#include "OBSEShaderInterface.h"

#include "D3D9.hpp"
#include "D3D9Device.hpp"

static global<bool> UseEffectList(true, NULL, "Effects", "bUseEffectList");
static global<char *> EffectDirectory("data\\shaders\\", NULL, "Effects", "sEffectDirectory");
static global<char *> EffectListFile("data\\shaders\\shaderlist.txt", NULL, "Effects", "sEffectListFile");
static global<bool> UseLegacyCompiler(false, NULL, "Effects", "bUseLegacyCompiler");
static global<bool> CompileSources(true, NULL, "Effects", "bCompileSources");
static global<bool> Optimize(false, NULL, "Effects", "bOptimize");
static global<bool> FreezeTweaks(false, NULL, "Effects", "bFreezeTweaks");
static global<bool> SplitScreen(false, NULL, "Effects", "bRenderHalfScreen");
static global<bool> PurgeOnNewGame(false, NULL, "Effects", "bPurgeOnNewGame");
static global<int> BufferTexturesNumBits(0, NULL, "ScreenBuffers", "iBufferTexturesNumBits");
static global<int> BufferRawZDepthNumBits(0, NULL, "ScreenBuffers", "iBufferRawZDepthNumBits");

/* #################################################################################################
 */

class FXIncludeManager : public ID3DXInclude
{
public:
    STDMETHOD(Open)(D3DXINCLUDE_TYPE IncludeType, LPCSTR pFileName, LPCVOID pParentData, LPCVOID *ppData, UINT *pBytes);
    STDMETHOD(Close)(LPCVOID pData);
};

HRESULT FXIncludeManager::Open(D3DXINCLUDE_TYPE IncludeType, LPCSTR pName, LPCVOID pParentData, LPCVOID *ppData, UINT *pBytes)
{
    struct stat s;
    FILE *f;
    char strFileFull[MAX_PATH];

    switch (IncludeType) {
      case D3DXINC_LOCAL: strcpy(strFileFull, ::EffectDirectory.Get()); strcat(strFileFull, pName); break;
      case D3DXINC_SYSTEM: strcpy(strFileFull, ""); strcat(strFileFull, pName); break;
    }

    if (stat((const char *)strFileFull, &s))
      return E_FAIL;

    UINT size = s.st_size;

    BYTE* pData = new BYTE[size];
    if (pData == NULL)
        return E_OUTOFMEMORY;

    if (fopen_s(&f, strFileFull, "rb"/*"rt"*/))
      return E_FAIL;

    fread(pData, 1, size, f);
    fclose(f);

    *ppData = pData;
    *pBytes = size;

    return S_OK;
}

HRESULT FXIncludeManager::Close(LPCVOID pData)
{
    BYTE* pData2 = (BYTE*)pData;
    if (pData2)
      delete[] pData2;
    return S_OK;
}

static FXIncludeManager incl;

/* #################################################################################################
 */

#define	strprete(i)	#i
#define	stringify(i)	strprete(i)

#define	EFFECTGROUP_FIRST	0x00
#define	EFFECTGROUP_PRE		0x10
#define	EFFECTGROUP_MAIN	0x40
#define	EFFECTGROUP_POST	0xC0
#define	EFFECTGROUP_LAST	0xFF

#define	EFFECTCLASS_NEUTRAL	0x00
#define	EFFECTCLASS_AO	        0x01
#define	EFFECTCLASS_LIGHT	0x02
#define	EFFECTCLASS_WATER	0x03
#define	EFFECTCLASS_DOF		0x04
#define	EFFECTCLASS_COLOR	0x05
#define	EFFECTCLASS_FILTER	0x06

#define	EFFECTCOND_HASSUN	(1 << 0)
#define	EFFECTCOND_HASWATER	(1 << 1)
#define	EFFECTCOND_INTERIOUR	(1 << 8)
#define	EFFECTCOND_EXTERIOUR	(1 << 9)
#define	EFFECTCOND_UNDERWATER	(1 << 10)
#define	EFFECTCOND_ISDAY	(1 << 16)
#define	EFFECTCOND_ISNIGHT	(1 << 17)
#define	EFFECTCOND_HASREFL	(1 << 18)
#define	EFFECTCOND_HASACHN	(1 << 19)	// special one
#define	EFFECTCOND_HASMIPS	(1 << 20)	// special one
#define	EFFECTCOND_HASSBUF	(1 << 25)	// special one
#define	EFFECTCOND_HASWNRM	(1 << 26)	// special one
#define	EFFECTCOND_HASWPOS	(1 << 27)	// special one
#define	EFFECTCOND_HASENRM	(1 << 28)	// special one
#define	EFFECTCOND_HASEPOS	(1 << 29)	// special one
#define	EFFECTCOND_HASLBUF	(1 << 30)	// special one
#define	EFFECTCOND_HASZBUF	(1 << 31)	// special one

#define	EFFECTBUF_RAWZ		(1 <<  0)	// special one
#define	EFFECTBUF_COPY		(1 <<  1)	// need frame "copy" because the effect surfaces have other format
#define	EFFECTBUF_PREV		(1 <<  2)	// need previous effect "copy"
#define	EFFECTBUF_PAST		(1 <<  3)	// need previous frame "copy"
#define	EFFECTBUF_SKIP		(1 <<  4)	// the first pass is skip-swap or blend
#define	EFFECTBUF_ACHN		EFFECTCOND_HASACHN
#define	EFFECTBUF_SBUF		EFFECTCOND_HASSBUF
#define	EFFECTBUF_WNRM		EFFECTCOND_HASWNRM
#define	EFFECTBUF_WPOS		EFFECTCOND_HASWPOS
#define	EFFECTBUF_ENRM		EFFECTCOND_HASENRM
#define	EFFECTBUF_EPOS		EFFECTCOND_HASEPOS
#define	EFFECTBUF_LBUF		EFFECTCOND_HASLBUF
#define	EFFECTBUF_ZBUF		EFFECTCOND_HASZBUF
#define	EFFECTBUF_ZMASK		(EFFECTCOND_HASWNRM | EFFECTCOND_HASWPOS | EFFECTCOND_HASENRM | EFFECTCOND_HASEPOS | EFFECTCOND_HASLBUF | EFFECTCOND_HASZBUF | EFFECTBUF_RAWZ)
#define	EFFECTBUF_TRANSFERZMASK	(EFFECTCOND_HASWNRM | EFFECTCOND_HASWPOS | EFFECTCOND_HASENRM | EFFECTCOND_HASEPOS | EFFECTCOND_HASLBUF |                      EFFECTBUF_RAWZ)

#define	EFFECTOPT_GATHER	(1 << 0)
#define	EFFECTOPT_SKIPSWAP	(1 << 1)
#define	EFFECTOPT_STENCIL	(1 << 2)
#define	EFFECTOPT_BLENDING	EFFECTOPT_SKIPSWAP

/* deprecated */
#ifndef	NO_DEPRECATED
#undef	EFFECTBUF_ACHN
#define	EFFECTBUF_ACHN		-1
#endif

typedef struct _myD3DXMACRO
{
  const char *Name;
  char *Definition;
} myD3DXMACRO;

/* little endian float representations of "GET4" and "GET1" */
#define GET4_FLOAT  1.9769241532685555e-7f  // 4TEG
#define GET1_FLOAT  7.722359973705295e-10f  // 1TEG
#define GET4_DWORD  ((DWORD)MAKEFOURCC('G', 'E', 'T', '4'))
#define GET1_DWORD  ((DWORD)MAKEFOURCC('G', 'E', 'T', '1'))

#define RT_SIGNED   '0'
#define RT_UNSIGNED '1'
#define RT_FLOAT    '0'
#define RT_INTEGER  '1'

static char IN_RAWZ[] = "0";
static char IN_LINZ[] = "0";
static char IN_PRJZ[] = "0";
static char IN_NRMZ[] = "0";
static char FETCHM [] = "MIPMAPLODBIAS";
static char FETCH4 [] = "0";
static char FETCH4G[] = "0";
static char FETCH1G[] = "0";
static char STARGET[] = "1";
static char TTARGET[] = "1";
static char *TWEAK_DYNAMIC = "extern";
static char *TWEAK_FROZEN  = "static const";

#define	DEFS_INRAWZ	0
#define	DEFS_INLINZ	1
#define	DEFS_INPRJZ	2
#define	DEFS_INNRMZ	3
#define	DEFS_iface	4
#define	DEFS_FETCHM	5
#define	DEFS_FETCH4	6
#define	DEFS_FETCH4G	7
#define	DEFS_FETCH1G	8
#define	DEFS_STARGET	9
#define	DEFS_TTARGET	10

static myD3DXMACRO defs[] = {
  {"IN_RAWZ"	        	, IN_RAWZ},
  {"IN_LINZ"	        	, IN_LINZ},
  {"IN_PRJZ"	        	, IN_PRJZ},
  {"IN_NRMZ"	        	, IN_NRMZ},
  {"iface"	        	, TWEAK_DYNAMIC},
  {"FETCHMODE"	        	, FETCHM},
  {"FETCH4"	        	, FETCH4},
  {"GET4"	        	, FETCH4G},
  {"GET1"	        	, FETCH1G},

  {"RENDERTARGET_SIGN"	        , STARGET},
  {"RENDERTARGET_TYPE"	        , TTARGET},

  {"UNSIGNED"	        	, "1"},
  {"SIGNED"	        	, "0"},
  {"INTEGER"	        	, "1"},
  {"FLOAT"	        	, "0"},
#ifndef	OBGE_CONSTANTPOOLS
  {"shared"	        	, ""},
#endif

  {"EFFECTGROUP_FIRST"		, stringify(EFFECTGROUP_FIRST		)},
  {"EFFECTGROUP_PRE"		, stringify(EFFECTGROUP_PRE		)},
  {"EFFECTGROUP_MAIN"		, stringify(EFFECTGROUP_MAIN		)},
  {"EFFECTGROUP_POST"		, stringify(EFFECTGROUP_POST		)},
  {"EFFECTGROUP_LAST"		, stringify(EFFECTGROUP_LAST		)},

  {"EFFECTCLASS_NEUTRAL"	, stringify(EFFECTCLASS_NEUTRAL		)},
  {"EFFECTCLASS_LIGHT"		, stringify(EFFECTCLASS_LIGHT		)},
  {"EFFECTCLASS_AO"		, stringify(EFFECTCLASS_AO		)},
  {"EFFECTCLASS_WATER"		, stringify(EFFECTCLASS_WATER		)},
  {"EFFECTCLASS_DOF"		, stringify(EFFECTCLASS_DOF		)},
  {"EFFECTCLASS_COLOR"		, stringify(EFFECTCLASS_COLOR		)},
  {"EFFECTCLASS_FILTER"		, stringify(EFFECTCLASS_FILTER		)},

  {"EFFECTCOND_HASSUN"	        , stringify(EFFECTCOND_HASSUN	        )},
  {"EFFECTCOND_HASWATER"	, stringify(EFFECTCOND_HASWATER	        )},
  {"EFFECTCOND_HASREFLECTIONS"	, stringify(EFFECTCOND_HASREFL	        )},
  {"EFFECTCOND_HASZBUFFER"	, stringify(EFFECTCOND_HASZBUF		)},
  {"EFFECTCOND_HASSBUFFER"	, stringify(EFFECTCOND_HASSBUF		)},
  {"EFFECTCOND_HASMIPMAPS"	, stringify(EFFECTCOND_HASMIPS		)},
  {"EFFECTCOND_INTERIOUR"	, stringify(EFFECTCOND_INTERIOUR	)},
  {"EFFECTCOND_EXTERIOUR"	, stringify(EFFECTCOND_EXTERIOUR	)},
  {"EFFECTCOND_UNDERWATER"	, stringify(EFFECTCOND_UNDERWATER	)},
  {"EFFECTCOND_ISDAY"	        , stringify(EFFECTCOND_ISDAY	        )},
  {"EFFECTCOND_ISNIGHT"	        , stringify(EFFECTCOND_ISNIGHT	        )},

  {"EFFECTCOND_SBUFFER"		, stringify(EFFECTCOND_HASSBUF		)}, // hm, error
  {"EFFECTCOND_ZBUFFER"		, stringify(EFFECTCOND_HASZBUF		)}, // hm, error
  {"EFFECTCOND_LBUFFER"		, stringify(EFFECTCOND_HASLBUF		)}, // hm, error
  {"EFFECTCOND_PBUFFER"		, stringify(EFFECTCOND_HASEPOS		)}, // hm, error
  {"EFFECTCOND_NBUFFER"		, stringify(EFFECTCOND_HASENRM		)}, // hm, error
  {"EFFECTCOND_MIPMAPS"		, stringify(EFFECTCOND_HASMIPS		)}, // hm, error
  {"EFFECTCOND_ACHANNEL"	, stringify(EFFECTCOND_HASACHN		)}, // hm, error

  {"EFFECTOPT_GATHER"		, stringify(EFFECTOPT_GATHER		)},
  {"EFFECTOPT_SKIPSWAP"		, stringify(EFFECTOPT_SKIPSWAP		)},
  {"EFFECTOPT_STENCIL"		, stringify(EFFECTOPT_STENCIL		)},
  {"EFFECTOPT_BLENDING"		, stringify(EFFECTOPT_BLENDING		)},

  {"D3DFMT_DEFAULT"	        , "0"},		// same format as main-pass surface

/* deprecated */
#ifndef	NO_DEPRECATED
  {"lastframe"			, "obge_PastRendertarget0_MAINPASS"	 },
  {"lastpass"			, "obge_LastRendertarget0_EFFECTPASS"	 },
  {"thisframe"			, "obge_PrevRendertarget0_EFFECTPASS"	 },

  {"reflection"			, "oblv_Rendertarget0_REFLECTIONPASS"	 },
  {"Depth"			, "oblv_CurrDepthStencilZ_MAINPASS"	 },
//{"rcpres"			, "oblv_ReciprocalResolution_MAINPASS"	 },

  {"m44world"			, "oblv_WorldTransform_MAINPASS"	 },
  {"m44view"			, "oblv_ViewTransform_MAINPASS"		 },
  {"m44proj"			, "oblv_ProjectionTransform_MAINPASS"	 },
  {"f3EyeForward"		, "oblv_CameraForward_MAINPASS"		 },
//{"f4Time"			, "oblv_GameTime"			 },
  {"f4SunDir"			, "oblv_SunDirection"			 },
#endif

  {NULL, NULL}
};

/* #################################################################################################
 * we have to use string-tables because we can not use fake-handles when LAA is enabled
 */

typedef struct _myD3DXCONST
{
  D3DXHANDLE Handle;
  const char *Name;
} myD3DXCONST;

enum {
  obge_PastRendertarget0_MAINPASS,
  obge_PastRendertarget1_MAINPASS,
  obge_PastRendertarget2_MAINPASS,
  obge_PastRendertarget3_MAINPASS,

  oblv_CurrRendertarget0_MAINPASS,
  oblv_CurrRendertarget1_MAINPASS,
  oblv_CurrRendertarget2_MAINPASS,
  oblv_CurrRendertarget3_MAINPASS,

  obge_PrevRendertarget0_EFFECTPASS,
  obge_PrevRendertarget1_EFFECTPASS,
  obge_PrevRendertarget2_EFFECTPASS,
  obge_PrevRendertarget3_EFFECTPASS,

  obge_LastRendertarget0_EFFECTPASS,
  obge_LastRendertarget1_EFFECTPASS,
  obge_LastRendertarget2_EFFECTPASS,
  obge_LastRendertarget3_EFFECTPASS,

  oblv_CurrCustomtarget0_MAINPASS,
  oblv_CurrCustomtarget1_MAINPASS,
  oblv_CurrCustomtarget2_MAINPASS,
  oblv_CurrCustomtarget3_MAINPASS,

  obge_PrevCustomtarget0_EFFECTPASS,
  obge_PrevCustomtarget1_EFFECTPASS,
  obge_PrevCustomtarget2_EFFECTPASS,
  obge_PrevCustomtarget3_EFFECTPASS,

  obge_LastCustomtarget0_EFFECTPASS,
  obge_LastCustomtarget1_EFFECTPASS,
  obge_LastCustomtarget2_EFFECTPASS,
  obge_LastCustomtarget3_EFFECTPASS,

  oblv_CurrWorldProjectedNormals_EFFECTPASS,
  oblv_CurrWorldProjectedZXYL_EFFECTPASS,
  oblv_CurrEyeProjectedNormals_EFFECTPASS,
  oblv_CurrEyeProjectedZXYD_EFFECTPASS,
  oblv_CurrLinearDepthZ_EFFECTPASS,
  oblv_CurrDepthStencilZ_MAINPASS,

  oblv_ReciprocalResolution_MAINPASS,

  oblv_WorldTransform_MAINPASS,
//oblv_WorldInverse_MAINPASS,
  oblv_ViewTransform_MAINPASS,
  oblv_ViewInverse_MAINPASS,
  oblv_ProjectionTransform_MAINPASS,
  oblv_ProjectionInverse_MAINPASS,

  oblv_PastViewProjectionTransform_MAINPASS,
  oblv_ViewProjectionTransform_MAINPASS,
  oblv_ViewProjectionInverse_MAINPASS,

  oblv_PastWorldViewProjectionTransform_MAINPASS,
  oblv_WorldViewProjectionTransform_MAINPASS,
  oblv_WorldViewProjectionInverse_MAINPASS,

  oblv_CameraForward_MAINPASS,
  oblv_CameraFrustum_MAINPASS,
  oblv_CameraPosition_MAINPASS,

  oblv_ProjectionDepthRange_MAINPASS,
  oblv_ProjectionFoV_MAINPASS,

  oblv_FogRange,
  oblv_FogColor,
  oblv_SunDirection,
  oblv_SunTiming,

  oblv_GameTime,
  obge_Tick,

#ifndef	NO_DEPRECATED
  depr_f4Time,
  depr_bHasDepth,
  depr_rcpres,
#endif

#ifndef	OBGE_NOSHADER
  oblv_Rendertarget0_REFLECTIONPASS,
  oblv_Rendertarget1_REFLECTIONPASS,
  oblv_Rendertarget2_REFLECTIONPASS,
  oblv_Rendertarget3_REFLECTIONPASS,

#ifndef	NO_DEPRECATED
  depr_bHasReflection,
#endif
#endif

  /* special */
  zbufferTexture,

  hndl_Size,
};

static myD3DXCONST hndl[] = {
  { NULL, "obge_PastRendertarget0_MAINPASS" },
  { NULL, "obge_PastRendertarget1_MAINPASS" },
  { NULL, "obge_PastRendertarget2_MAINPASS" },
  { NULL, "obge_PastRendertarget3_MAINPASS" },

  { NULL, "oblv_CurrRendertarget0_MAINPASS" },
  { NULL, "oblv_CurrRendertarget1_MAINPASS" },
  { NULL, "oblv_CurrRendertarget2_MAINPASS" },
  { NULL, "oblv_CurrRendertarget3_MAINPASS" },

  { NULL, "obge_PrevRendertarget0_EFFECTPASS" },
  { NULL, "obge_PrevRendertarget1_EFFECTPASS" },
  { NULL, "obge_PrevRendertarget2_EFFECTPASS" },
  { NULL, "obge_PrevRendertarget3_EFFECTPASS" },

  { NULL, "obge_LastRendertarget0_EFFECTPASS" },
  { NULL, "obge_LastRendertarget1_EFFECTPASS" },
  { NULL, "obge_LastRendertarget2_EFFECTPASS" },
  { NULL, "obge_LastRendertarget3_EFFECTPASS" },

  { NULL, "oblv_CurrCustomtarget0_MAINPASS" },
  { NULL, "oblv_CurrCustomtarget1_MAINPASS" },
  { NULL, "oblv_CurrCustomtarget2_MAINPASS" },
  { NULL, "oblv_CurrCustomtarget3_MAINPASS" },

  { NULL, "obge_PrevCustomtarget0_EFFECTPASS" },
  { NULL, "obge_PrevCustomtarget1_EFFECTPASS" },
  { NULL, "obge_PrevCustomtarget2_EFFECTPASS" },
  { NULL, "obge_PrevCustomtarget3_EFFECTPASS" },

  { NULL, "obge_LastCustomtarget0_EFFECTPASS" },
  { NULL, "obge_LastCustomtarget1_EFFECTPASS" },
  { NULL, "obge_LastCustomtarget2_EFFECTPASS" },
  { NULL, "obge_LastCustomtarget3_EFFECTPASS" },

  { NULL, "oblv_CurrWorldProjectedNormals_EFFECTPASS" },
  { NULL, "oblv_CurrWorldProjectedZXYL_EFFECTPASS" },
  { NULL, "oblv_CurrEyeProjectedNormals_EFFECTPASS" },
  { NULL, "oblv_CurrEyeProjectedZXYD_EFFECTPASS" },
  { NULL, "oblv_CurrLinearDepthZ_EFFECTPASS" },
  { NULL, "oblv_CurrDepthStencilZ_MAINPASS" },

  { NULL, "oblv_ReciprocalResolution_MAINPASS" },

  { NULL, "oblv_WorldTransform_MAINPASS" },
//{ NULL, "oblv_WorldInverse_MAINPASS" },
  { NULL, "oblv_ViewTransform_MAINPASS" },
  { NULL, "oblv_ViewInverse_MAINPASS" },
  { NULL, "oblv_ProjectionTransform_MAINPASS" },
  { NULL, "oblv_ProjectionInverse_MAINPASS" },

  { NULL, "oblv_PastViewProjectionTransform_MAINPASS" },
  { NULL, "oblv_ViewProjectionTransform_MAINPASS" },
  { NULL, "oblv_ViewProjectionInverse_MAINPASS" },

  { NULL, "oblv_PastWorldViewProjectionTransform_MAINPASS" },
  { NULL, "oblv_WorldViewProjectionTransform_MAINPASS" },
  { NULL, "oblv_WorldViewProjectionInverse_MAINPASS" },

  { NULL, "oblv_CameraForward_MAINPASS" },
  { NULL, "oblv_CameraFrustum_MAINPASS" },
  { NULL, "oblv_CameraPosition_MAINPASS" },

  { NULL, "oblv_ProjectionDepthRange_MAINPASS" },
  { NULL, "oblv_ProjectionFoV_MAINPASS" },

  { NULL, "oblv_FogRange" },
  { NULL, "oblv_FogColor" },
  { NULL, "oblv_SunDirection" },
  { NULL, "oblv_SunTiming" },

  { NULL, "oblv_GameTime" },
  { NULL, "obge_Tick" },

#ifndef	NO_DEPRECATED
  { NULL, "f4Time" },
  { NULL, "bHasDepth" },
  { NULL, "rcpres" },
#endif

#ifndef	OBGE_NOSHADER
  { NULL, "oblv_Rendertarget0_REFLECTIONPASS" },
  { NULL, "oblv_Rendertarget1_REFLECTIONPASS" },
  { NULL, "oblv_Rendertarget2_REFLECTIONPASS" },
  { NULL, "oblv_Rendertarget3_REFLECTIONPASS" },

#ifndef	NO_DEPRECATED
  { NULL, "bHasReflection" },
#endif
#endif

  /* special */
  { NULL, "zbufferTexture" },
};

/* #################################################################################################
 */

EffectBuffer::EffectBuffer() {
  for (int rt = 0; rt < EBUFRT_NUM; rt++) {
    Tex[rt] = NULL;
    Srf[rt] = NULL;
    mne[rt] = false;
  }
}

EffectBuffer::~EffectBuffer() {
  Release();
}

inline HRESULT EffectBuffer::Initialize(IDirect3DTexture9 *text) {
  Release(0, 1);

  Tex[0] = text;
//Tex[0]->GetSurfaceLevel(0, &Srf[0]);

#if	defined(OBGE_AUTOMIPMAP)
  if (Tex[0])
    Tex[0]->SetAutoGenFilterType(AMFilter);
#endif

  return (Srf[0] ? D3D_OK : S_FALSE);
}

inline HRESULT EffectBuffer::Initialize(IDirect3DSurface9 *surf) {
  IDirect3DTexture9 *text = NULL;

#ifndef	OBGE_NOSHADER
  text = surfaceTexture[surf] ? surfaceTexture[surf]->tex : NULL;
#endif
  surf =                surf                                    ;

  if (!text) {
    /* no alternative replacement target available, so we
     * preserve the surface anyway
     */
    if (!Tex[0]) {
      Tex[0] = text;
      Srf[0] = surf;
    }

    return S_FALSE;
  }

  /* this is now the texture/surface-pair to use */
  Release(0, 1);

  Tex[0] = text;
  Srf[0] = surf;

#if	defined(OBGE_AUTOMIPMAP)
  if (Tex[0])
    Tex[0]->SetAutoGenFilterType(AMFilter);
#endif

  return D3D_OK;
}

inline HRESULT EffectBuffer::Initialize(const D3DFORMAT fmt[EBUFRT_NUM]) {
  UInt32 Width  = v1_2_416::GetRenderer()->SizeWidth;
  UInt32 Height = v1_2_416::GetRenderer()->SizeHeight;
  HRESULT hr;

#if	defined(OBGE_AUTOMIPMAP)
#define EFFECT_USAGE  D3DUSAGE_RENDERTARGET | D3DUSAGE_AUTOGENMIPMAP
#else
#define EFFECT_USAGE  D3DUSAGE_RENDERTARGET
#endif

  for (int rt = 0; rt < EBUFRT_NUM; rt++) {
    if (!Tex[rt] && (fmt[rt] != D3DFMT_UNKNOWN)) {
//    Release(rt, rt + 1);

      if ((hr = lastOBGEDirect3DDevice9->CreateTexture(Width, Height, 1, EFFECT_USAGE, fmt[rt], D3DPOOL_DEFAULT, &Tex[0], 0)) == D3D_OK) {
	Tex[rt]->GetSurfaceLevel(0, &Srf[rt]);
#if	defined(OBGE_AUTOMIPMAP)
	Tex[rt]->SetAutoGenFilterType(AMFilter);
#endif
      }

      mne[rt] = true;

      _DMESSAGE("Creating EffectBuffer surface %s: %s", findFormat(fmt[rt]), Tex[rt] ? "success" : "failed");
    }
  }

  return D3D_OK;
}

inline void EffectBuffer::Release(int rmin, int rnum) {
  for (int rt = rmin; rt < rnum; rt++) {
    if (Tex[rt] && mne[rt]) Tex[rt]->Release(); Tex[rt] = NULL; mne[rt] = false;
    if (Srf[rt] && mne[rt]) Srf[rt]->Release(); Srf[rt] = NULL; mne[rt] = false;
  }
}

inline bool EffectBuffer::IsValid() const {
  for (int rt = 0; rt < EBUFRT_NUM; rt++)
    if (Tex[rt]) return true;

  return false;
}

bool EffectBuffer::IsTexture(IDirect3DBaseTexture9 *text) const {
  for (int rt = 0; rt < EBUFRT_NUM; rt++)
    if (Tex[rt] == text) return true;

  return false;
}

inline void EffectBuffer::SetTexture(const char *fmt, ID3DXEffect *pEffect) const {
  char buf[256];
  for (int rt = 0; rt < EBUFRT_NUM; rt++) {
    if (Tex[rt]) {
      sprintf(buf, fmt, rt);
      D3DXHANDLE hl = pEffect->GetParameterByName(NULL, buf);
      if (hl) {
	pEffect->SetTexture(hl, Tex[rt]);
#if	defined(OBGE_AUTOMIPMAP)
	Tex[rt]->SetAutoGenFilterType(AMFilter);
	assert(Tex[rt]->GetAutoGenFilterType() == AMFilter);
#endif
      }
    }
  }
}

inline void EffectBuffer::SetTexture(const D3DXHANDLE *hs, ID3DXEffect *pEffect) const {
  for (int rt = 0; rt < EBUFRT_NUM; rt++) {
    if (Tex[rt] && hs[rt]) {
      pEffect->SetTexture(hs[rt], Tex[rt]);
#if	defined(OBGE_AUTOMIPMAP)
      Tex[rt]->SetAutoGenFilterType(AMFilter);
      assert(Tex[rt]->GetAutoGenFilterType() == AMFilter);
#endif
    }
  }
}

inline void EffectBuffer::SetTexture(const D3DXHANDLE hs, ID3DXEffect *pEffect) const {
  for (int rt = 0; rt < 1; rt++) {
    if (Tex[rt] && hs) {
      pEffect->SetTexture(hs, Tex[rt]);
#if	defined(OBGE_AUTOMIPMAP)
      Tex[rt]->SetAutoGenFilterType(AMFilter);
      assert(Tex[rt]->GetAutoGenFilterType() == AMFilter);
#endif
    }
  }
}

inline void EffectBuffer::SetRenderTarget(IDirect3DDevice9 *Device) const {
  for (int rt = 0; rt < EBUFRT_NUM; rt++) {
    if (Srf[rt])
      Device->SetRenderTarget(rt, Srf[rt]);
  }
}

inline void EffectBuffer::Copy(IDirect3DDevice9 *Device, EffectBuffer *from) const {
  for (int rt = 0; rt < EBUFRT_NUM; rt++) {
    if (from->Srf[rt] && Srf[rt] && (from->Srf[rt] != Srf[rt]))
      minStretchRect(Device, from->Srf[rt], 0, Srf[rt], 0, D3DTEXF_NONE);
  }
}

inline void EffectBuffer::Copy(IDirect3DDevice9 *Device, IDirect3DSurface9 *from) const {
  for (int rt = 0; rt < EBUFRT_NUM; rt++) {
    if (from          && Srf[rt] && (from          != Srf[rt]))
      minStretchRect(Device, from, 0, Srf[rt], 0, D3DTEXF_NONE);
  }
}

/* #################################################################################################
 */

EffectQueue::EffectQueue(bool custom) {
  SetCustom(custom);
}

EffectQueue::~EffectQueue() {
}

void EffectQueue::SetCustom(bool custom) {
  if (custom) {
    currHL = oblv_CurrCustomtarget0_MAINPASS;
    prevHL = obge_PrevCustomtarget0_EFFECTPASS;
    lastHL = obge_LastCustomtarget0_EFFECTPASS;
  }
  else {
    currHL = oblv_CurrRendertarget0_MAINPASS;
    prevHL = obge_PrevRendertarget0_EFFECTPASS;
    lastHL = obge_LastRendertarget0_EFFECTPASS;
  }
}

#define EQLAST	0
#define EQPREV	1

inline void EffectQueue::Init(EffectBuffer *prev,
			      EffectBuffer *alt, bool stencil) {
  this->prev = prev;
  this->prvl = NULL;

  queue[1] = alt;

  /* set alternate trashable depth/stencil buffer */
  if ((dsc = (stencil ? D3DCLEAR_ZBUFFER | D3DCLEAR_STENCIL : 0)))
    device->SetDepthStencilSurface(GetStencilSurface());
}

inline void EffectQueue::Begin(EffectBuffer *orig,
			       EffectBuffer *target,
			       EffectBuffer *alt) {

  this->orig = orig;

  /* both buffers in the queue are of distinct type from target (the original surface) */
  if (!alt)
    (queue[0] = target);
  else
    (queue[0] = alt)->Copy(device, target);

  /* 1)  begin() alterning = 0
   *  a) step()  alterning = 1, 1 == alt,    setrendertarget(alt)
   *     end()
   *  b) step()  alterning = 0, 0 == target, setrendertarget(target)
   *     step()  alterning = 1, 1 == alt,    setrendertarget(alt)
   *     skip()  alterning = 1, 1 == alt,    setrendertarget(alt)
   *     end()
   *  c) step()  alterning = 0, 0 == target, setrendertarget(target)
   *     step()  alterning = 1, 1 == alt,    setrendertarget(alt)
   *     end()
   *     end()   alterning = 1, 1 == alt,    copy alt to target
   */
  alterning = 0;

  /* initially prev is orig */
  rotate[EQPREV] = orig;
  rotate[EQLAST] = orig;
}

inline void EffectQueue::Begin(const D3DXHANDLE *h, ID3DXEffect *pEffect, unsigned long Parameters) {
  /* configure the backup of the past effect's rendertarget per effect */
  prvl = (Parameters & EFFECTBUF_PREV ? prev : NULL);
  dclr = D3DCLEAR_TARGET | dsc;

  /* set this effects initial constant parameters (unbind any possible future RT) */
  /*     */ orig->SetTexture(h + currHL, pEffect);

  /* we need a valid incoming state (textures and rt bound correctly) */
  if ((Parameters & EFFECTBUF_SKIP)) {
    if ((Parameters & EFFECTBUF_PREV) && prev) {
      rotate[EQPREV] = (prev->Copy(device, rotate[EQPREV]), prev);
      rotate[EQPREV]->SetTexture(h + prevHL, pEffect);
      rotate[EQPREV]->SetTexture(h + lastHL, pEffect);
    }
    else {
      /*     */ orig->SetTexture(h + prevHL, pEffect);
      /*     */ orig->SetTexture(h + lastHL, pEffect);
    }

    /* the very first pass of all can not write to the last rendertarget
     * which is the original surface, which is either not writable, or
     * need to be preserved for later access
     */
    if (!alterning) {
      if (rotate[EQLAST]->IsValid()) {
	queue[0]->Copy(device, rotate[EQLAST]);
	dclr &= ~D3DCLEAR_TARGET;
      }
    }
    else
      dclr &= ~D3DCLEAR_TARGET;

    (rotate[EQLAST] = queue[(alterning += 0) & 1])->SetRenderTarget(device);
    device->Clear(0, NULL, dclr, 0, 1.0f, 0); dclr = D3DCLEAR_TARGET;
  }
}

inline void EffectQueue::Swap(const D3DXHANDLE *h, ID3DXEffect *pEffect) {
  rotate[EQPREV] = (prvl ? prvl->Copy(device, rotate[EQPREV]), prvl : rotate[EQPREV]);
  rotate[EQPREV]->SetTexture(h + prevHL, pEffect);
  rotate[EQLAST]->SetTexture(h + lastHL, pEffect);

  (rotate[EQLAST] = queue[(alterning += 1) & 1])->SetRenderTarget(device);
  device->Clear(0, NULL, dclr, 0, 1.0f, 0); dclr = D3DCLEAR_TARGET;
}

inline void EffectQueue::End(const D3DXHANDLE *h, ID3DXEffect *pEffect) {
  rotate[EQPREV] = rotate[EQLAST];
}

inline void EffectQueue::Pass(const D3DXHANDLE *h, ID3DXEffect *pEffect) {
//rotate[EQPREV]->SetTexture(h + obge_PrevCustomtarget0_EFFECTPASS, pEffect);
  rotate[EQLAST]->SetTexture(h + obge_LastCustomtarget0_EFFECTPASS, pEffect);

  /* reset queue */
  alterning = 0;

  /* successive prev is orig */
  rotate[EQPREV] = orig;
  rotate[EQLAST] = orig;
}

inline void EffectQueue::End(EffectBuffer *past,
			     EffectBuffer *target) {
  /* this occurs when "target" never entered the queue,
   * or when we have an odd number of accumulated passes
   */
  target->Copy(device, rotate[EQLAST]);

  /* not allocated if not needed! make a copy for the
   * next frame, means frame 0 is always invalid
   */
  if (past)
    past->Copy(device, orig);

  /* restore original rendertarget/depthbuffer (Oblivion expects this!) */
  target->SetRenderTarget(device);
  device->SetDepthStencilSurface(GetDepthBufferSurface());
}

#undef EQLAST
#undef EQPREV

// *********************************************************************************************************

ManagedEffectQueue::ManagedEffectQueue() {
  RefCount = 0;
}

ManagedEffectQueue::~ManagedEffectQueue() {
}

void ManagedEffectQueue::ClrRef() {
  RefCount = 0;
}

int ManagedEffectQueue::AddRef() {
  return ++RefCount;
}

int ManagedEffectQueue::Release() {
  if (RefCount)
    RefCount--;

  if (!RefCount) {
//  delete this;
    return 0;
  }

  return RefCount;
}

/* #################################################################################################
 */

EffectRecord::EffectRecord() {
  Filepath[0] = '\0';
  Name[0] = '\0';
  ParentRefID = 0;

  Enabled = false;
  Private = false;

  /* shader source, binaries and constant tables */
  pDefine = NULL;
  pBinary = NULL;
  pEffect = NULL;
  pSource = NULL; sourceLen = 0;
  pErrorMsgs = NULL;
  pDisasmbly = NULL;
  CustomQueue = NULL;

  Parameters = 0;
  Priority = 0;
  Flags = 0;
  Options = 0;
  Class = 0;
}

EffectRecord::~EffectRecord() {
  if (pBinary) pBinary->Release();
  if (pEffect) pEffect->Release();
  if (pSource) delete[] pSource;
  if (pErrorMsgs) pErrorMsgs->Release();
  if (pDisasmbly) pDisasmbly->Release();
  if (CustomQueue) EffectManager::GetSingleton()->ReleaseQueue(CustomQueue);

  /* release previous texture */
  TextureManager *TexMan = TextureManager::GetSingleton();
  std::vector<int>::iterator PTexture = Textures.begin();

  while (PTexture != Textures.end()) {
    TexMan->ReleaseTexture(*PTexture);
    PTexture++;
  }
}

void EffectRecord::Kill() {
  if (pBinary) pBinary->Release();
  if (pEffect) pEffect->Release();
  if (pSource) delete[] pSource;
  if (pErrorMsgs) pErrorMsgs->Release();
  if (pDisasmbly) pDisasmbly->Release();
  if (CustomQueue) EffectManager::GetSingleton()->ReleaseQueue(CustomQueue);

  this->pDefine = NULL;
  this->pBinary = NULL;
  this->pEffect = NULL;
  this->pSource = NULL;
  this->pErrorMsgs = NULL;
  this->pDisasmbly = NULL;
  this->CustomQueue = NULL;

  this->Name[0] = '\0';
  this->Filepath[0] = '\0';
}

bool EffectRecord::LoadEffect(const char *Filename, UINT32 refID, bool Private, D3DXMACRO *defs) {
  if (refID != NULL) {
    int a = 0;
  }

  Kill();

  if (strlen(Filename) > 240)
    return false;
  if (!defs)
    defs = (D3DXMACRO *)::defs;

  struct stat sb, sx;
  char strFileFull[MAX_PATH], *ext;
  FILE *f;

  strcpy(Name, Filename);

  /* getting a compiled binary for replacement if there is any
   */
  strcpy(Filepath, ::EffectDirectory.Get());
  strcat(Filepath, Filename);
  if ((ext = strstr(Filepath, ".fx")))
    ext[3] = 'o', ext[4] = '\0';
  if (!stat((const char *)Filepath, &sb)) {
    UINT size = sb.st_size;
    if (D3DXCreateBuffer(size, &pBinary) == D3D_OK) {
      if (!fopen_s(&f, Filepath, "rb")) {
	fread(pBinary->GetBufferPointer(), 1, size, f);
	fclose(f);

        _DMESSAGE("Loaded binary of %s from %s", Name, Filename);
      }
      else {
	pBinary->Release();
	pBinary = NULL;
      }
    }
  }
  if (ext)
    ext[3] = '\0';

  /* getting a FX source for compiling if there is any
   */
  strcpy(strFileFull, ::EffectDirectory.Get());
  strcat(strFileFull, Filename);
  if (!stat((const char *)strFileFull, &sx)) {
    UINT size = sx.st_size;
    pSource = new CHAR[size + 1];
    if (pSource != NULL) {
      /* reading in text-mode can yield any number of less characters */
      memset(pSource, 0, size + 1);

      if (!fopen_s(&f, strFileFull, "rb"/*"rt"*/)) {
        fread(pSource, 1, size, f);
        fclose(f);

        _DMESSAGE("Loaded source of %s from %s", Name, Filename);

	sourceLen = strlen(pSource);
      }
      else {
      	delete[] pSource;

      	pSource = NULL;
        sourceLen = 0;
      }
    }

    /* if the FX source is newer than the binary, attempt to recompile
     */
    if (pSource && (sx.st_mtime > sb.st_mtime)) {
      if (pBinary) pBinary->Release();
      if (pEffect) pEffect->Release();

      pBinary = NULL;
      pEffect = NULL;
    }
  }

  this->pDefine = defs;
  this->Enabled = false;
  this->Private = Private;
  this->ParentRefID = refID;

  return (pSource != NULL);
}

bool EffectRecord::RuntimeFlush() {
  /* release all runtime resources (for example for recompilation) */
  if (pBinary) pBinary->Release();
  if (pEffect) pEffect->Release();
  if (pErrorMsgs) pErrorMsgs->Release();
  if (pDisasmbly) pDisasmbly->Release();
  if (CustomQueue) EffectManager::GetSingleton()->ReleaseQueue(CustomQueue);

  pBinary = NULL;
  pEffect = NULL;
  pErrorMsgs = NULL;
  pDisasmbly = NULL;
  CustomQueue = NULL;

  return true;
}

bool EffectRecord::RuntimeEffect(const char *fx) {
  int size = strlen(fx);

  /* put a new source */
  if (pSource) delete[] pSource;
  pSource = new CHAR[size + 1];
  strcpy(pSource, fx);
  pSource[size] = '\0';
  sourceLen = size;

  /* trigger recompile */
  return RuntimeFlush();
}

bool EffectRecord::SaveEffect() {
  LPD3DXBUFFER p = NULL;

  /* cascade, the highest possible is selected */
  if (pBinary)
    p = pBinary;

  if (p) {
    char *ext;
    FILE *f;

    if ((ext = strstr(Filepath, ".fx")))
      ext[3] = 'o', ext[4] = '\0';
    if (!fopen_s(&f, Filepath, "wb")) {
      fwrite(p->GetBufferPointer(), 1, p->GetBufferSize(), f);
      fclose(f);

      _DMESSAGE("Saved binary of %s to %s", Name, Filepath);

      if (ext)
	ext[3] = '\0';

      return true;
    }
    if (ext)
      ext[3] = '\0';
  }

  return false;
}
bool EffectRecord::CompileEffect(EffectManager *FXMan, bool forced) {
  /* nobody wants the automatic recompile */
  if (!::CompileSources.Get() && !forced)
    return false;
  LPSTR src = NULL; int len;
  LPD3DXBUFFER p = NULL;
  ID3DXEffectCompiler *c = NULL;
  ID3DXEffect *x = NULL;
  bool save = false;
  HRESULT res;

  /* cascade, the highest possible is selected */
  if (pSource) {
    src = pSource;
    len = sourceLen;

    p = pBinary;
    x = pEffect;
  }

  /* recompile only, if there is one already, just ignore */
  if (!p && src) {
    if (pDisasmbly)
    pDisasmbly->Release();
    pDisasmbly = NULL;

    if (pErrorMsgs)
    pErrorMsgs->Release();
    pErrorMsgs = NULL;

    res = D3DXCreateEffectCompiler(
      src,
      len,
      pDefine,
      &incl,
      D3DXFX_LARGEADDRESSAWARE |
      D3DXSHADER_DEBUG | (
      ::UseLegacyCompiler.Get() ? D3DXSHADER_USE_LEGACY_D3DX9_31_DLL : (
      ::Optimize.Get()          ? D3DXSHADER_OPTIMIZATION_LEVEL3 : 0)),
      &c,
      &pErrorMsgs
    );

    /* this didn't go so well, if it's a legacy "error", just try again */
    if (pErrorMsgs && strstr((char*)pErrorMsgs->GetBufferPointer(), "X3539")) {
      pErrorMsgs->Release();
      pErrorMsgs = NULL;

      res = D3DXCreateEffectCompiler(
	src,
	len,
	pDefine,
	&incl,
	D3DXFX_LARGEADDRESSAWARE |
	D3DXSHADER_DEBUG | (
	D3DXSHADER_USE_LEGACY_D3DX9_31_DLL),
	&c,
	&pErrorMsgs
      );
    }

    /* this didn't go so well */
    if (pErrorMsgs) {
      char *msg = (char *)pErrorMsgs->GetBufferPointer();

      _MESSAGE("Effect compiling messages occured in %s:", Filepath);
      _MESSAGE((char *)pErrorMsgs->GetBufferPointer());
    }

    if (c) {
      if (pErrorMsgs)
      pErrorMsgs->Release();
      pErrorMsgs = NULL;

      res = c->CompileEffect(/*
	D3DXFX_LARGEADDRESSAWARE*/ 0 |
	D3DXSHADER_DEBUG | (
	::UseLegacyCompiler.Get() ? D3DXSHADER_USE_LEGACY_D3DX9_31_DLL : (
	::Optimize.Get()          ? D3DXSHADER_OPTIMIZATION_LEVEL3 : 0)),
	&p,
	&pErrorMsgs
      );

      /* this didn't go so well, if it's a legacy "error", just try again */
      if (pErrorMsgs && strstr((char*)pErrorMsgs->GetBufferPointer(), "X3539")) {
	pErrorMsgs->Release();
	pErrorMsgs = NULL;

	res = c->CompileEffect(/*
	  D3DXFX_LARGEADDRESSAWARE*/ 0 |
	  D3DXSHADER_DEBUG | (
	  D3DXSHADER_USE_LEGACY_D3DX9_31_DLL),
	  &p,
	  &pErrorMsgs
	);
      }

      c->Release();
    }

    /* this didn't go so well */
    if (pErrorMsgs) {
      _MESSAGE("Effect compiling messages occured in %s:", Filepath);
      _MESSAGE((char *)pErrorMsgs->GetBufferPointer());

      save = !strstr((char *)pErrorMsgs->GetBufferPointer(), "error");
    }
    else
      save = true;
  }

  /* recompile only, if there is one already, just ignore */
  if (!x && (p || src)) {
    if (pDisasmbly)
    pDisasmbly->Release();
    pDisasmbly = NULL;

    /* try from binary */
    if (p) {
      if (pErrorMsgs)
      pErrorMsgs->Release();
      pErrorMsgs = NULL;

      res = D3DXCreateEffect(
	slimOBGEDirect3DDevice9,
	p->GetBufferPointer(),
	p->GetBufferSize(),
	pDefine,
	&incl,
	D3DXFX_LARGEADDRESSAWARE |
	D3DXSHADER_DEBUG | (
	::UseLegacyCompiler.Get() ? D3DXSHADER_USE_LEGACY_D3DX9_31_DLL : (
	::Optimize.Get()          ? D3DXSHADER_OPTIMIZATION_LEVEL3 : 0)),
	FXMan ? FXMan->EffectPool : NULL,
	&x,
	&pErrorMsgs
      );
    }

    /* try from source */
    if (!x && src) {
      if (pErrorMsgs)
      pErrorMsgs->Release();
      pErrorMsgs = NULL;

      res = D3DXCreateEffect(
	slimOBGEDirect3DDevice9,
	src,
	len,
	pDefine,
	&incl,
	D3DXFX_LARGEADDRESSAWARE |
	D3DXSHADER_DEBUG | (
	::UseLegacyCompiler.Get() ? D3DXSHADER_USE_LEGACY_D3DX9_31_DLL : (
	::Optimize.Get()          ? D3DXSHADER_OPTIMIZATION_LEVEL3 : 0)),
	FXMan ? FXMan->EffectPool : NULL,
	&x,
	&pErrorMsgs
      );
    }

    /* this didn't go so well, if it's a legacy "error", just try again */
    if (pErrorMsgs && strstr((char*)pErrorMsgs->GetBufferPointer(), "X3539")) {
      /* try from binary */
      if (p) {
	if (pErrorMsgs)
	pErrorMsgs->Release();
	pErrorMsgs = NULL;

	res = D3DXCreateEffect(
	  slimOBGEDirect3DDevice9,
	  p->GetBufferPointer(),
	  p->GetBufferSize(),
	  pDefine,
	  &incl,
	  D3DXFX_LARGEADDRESSAWARE |
	  D3DXSHADER_DEBUG | (
	  D3DXSHADER_USE_LEGACY_D3DX9_31_DLL),
	  FXMan ? FXMan->EffectPool : NULL,
	  &x,
	  &pErrorMsgs
	);
      }

      /* try from source */
      if (!x && src) {
	if (pErrorMsgs)
	pErrorMsgs->Release();
	pErrorMsgs = NULL;

	res = D3DXCreateEffect(
	  slimOBGEDirect3DDevice9,
	  src,
	  len,
	  pDefine,
	  &incl,
	  D3DXFX_LARGEADDRESSAWARE |
	  D3DXSHADER_DEBUG | (
	  D3DXSHADER_USE_LEGACY_D3DX9_31_DLL),
	  FXMan ? FXMan->EffectPool : NULL,
	  &x,
	  &pErrorMsgs
	);
      }
    }

    /* this didn't go so well */
    if (pErrorMsgs) {
      _MESSAGE("Effect compiling messages occured in %s:", Filepath);
      _MESSAGE((char *)pErrorMsgs->GetBufferPointer());
    }
  }

  /* cascade, the highest possible is selected */
  if (pSource) {
    pBinary = p;
    pEffect = x;
  }

  /* auto-save or not */
  if (save)
    SaveEffect();

  if (x) {
    if (!pDisasmbly)
      D3DXDisassembleEffect(
        x,
        FALSE,
        &pDisasmbly
      );

    ApplyCompileDirectives(FXMan);
  }

  return (pSource && (pEffect != NULL));
}

void EffectRecord::ApplyCompileDirectives(EffectManager *FXMan) {
  if (!HasEffect()) return;

  /* get an alternative name if possible */
  D3DXHANDLE hName = pEffect->GetParameterByName(NULL, "Name");
  LPCSTR pName = NULL;
  if (hName) pEffect->GetString(hName, &pName);
  if (pName) strcpy(Name, (char *)pName);

  /* obtain a copy of the old resources */
  TextureManager *TexMan = TextureManager::GetSingleton();
  std::vector<int> prevTextures = Textures; Textures.clear();
  D3DXEFFECT_DESC Description;
  pEffect->GetDesc(&Description);

  Parameters = 0;
  ParametersCustom = 0;

  /* extract parameter informations */
  for (int par = 0; par < Description.Parameters; par++) {
    D3DXHANDLE handle;

    if ((handle = pEffect->GetParameter(NULL, par))) {
      D3DXPARAMETER_DESC Description;
      pEffect->GetParameterDesc(handle, &Description);

      if (Description.Type == D3DXPT_TEXTURECUBE) {
	D3DXHANDLE handle2;

	if ((handle2 = pEffect->GetAnnotationByName(handle, "filename"))) {
	  LPCSTR pString = NULL; pEffect->GetString(handle2, &pString);

	  _MESSAGE("Found texture to load: %s", pString);

	  int TexNum = TexMan->LoadDependtTexture((char *)pString, TR_CUBIC);
	  if (TexNum != -1) {
	    pEffect->SetTexture(handle, TexMan->GetTexture(TexNum)->GetTexture());
	    Textures.push_back(TexNum);
	  }
	}
      }
      else if (Description.Type == D3DXPT_TEXTURE3D) {
	D3DXHANDLE handle2;

	if ((handle2 = pEffect->GetAnnotationByName(handle, "filename"))) {
	  LPCSTR pString = NULL; pEffect->GetString(handle2, &pString);

	  _MESSAGE("Found texture to load: %s", pString);

	  int TexNum = TexMan->LoadDependtTexture((char *)pString, TR_VOLUMETRIC);
	  if (TexNum != -1) {
	    pEffect->SetTexture(handle, TexMan->GetTexture(TexNum)->GetTexture());
	    Textures.push_back(TexNum);
	  }
	}
      }
      else if ((Description.Type == D3DXPT_TEXTURE) ||
	       (Description.Type == D3DXPT_TEXTURE1D) ||
	       (Description.Type == D3DXPT_TEXTURE2D)) {
	  D3DXHANDLE handle2;

	  if ((handle2 = pEffect->GetAnnotationByName(handle, "filename"))) {
	    LPCSTR pString = NULL; pEffect->GetString(handle2, &pString);

	    _MESSAGE("Found texture to load: %s", pString);

	    int TexNum = TexMan->LoadDependtTexture((char *)pString, TR_PLANAR);
	    if (TexNum != -1) {
	      pEffect->SetTexture(handle, TexMan->GetTexture(TexNum)->GetTexture());
	      Textures.push_back(TexNum);
	    }
	  }
	  else {
	    /**/ if (Description.Name == strstr(Description.Name, "obge_PrevRendertarget"))
	      Parameters |= EFFECTBUF_PREV;
	    else if (Description.Name == strstr(Description.Name, "obge_PastRendertarget"))
	      Parameters |= EFFECTBUF_PAST;
	    else if (Description.Name == strstr(Description.Name, "oblv_CurrDepthStencilZ"))
	      Parameters |= EFFECTBUF_ZBUF;
	    else if (Description.Name == strstr(Description.Name, "oblv_CurrDepthStencilW"))
	      Parameters |= EFFECTBUF_ZBUF;
	    else if (Description.Name == strstr(Description.Name, "oblv_CurrLinearDepthZ"))
	      Parameters |= EFFECTBUF_ZBUF | EFFECTBUF_LBUF;
	    else if (Description.Name == strstr(Description.Name, "oblv_CurrEyeProjectedZXYD_EFFECTPASS"))
	      Parameters |= EFFECTBUF_ZBUF | EFFECTBUF_LBUF | EFFECTBUF_EPOS;
	    else if (Description.Name == strstr(Description.Name, "oblv_CurrEyeProjectedNormals_EFFECTPASS"))
	      Parameters |= EFFECTBUF_ZBUF | EFFECTBUF_LBUF | EFFECTBUF_EPOS | EFFECTBUF_ENRM;
	    else if (Description.Name == strstr(Description.Name, "oblv_CurrWorldProjectedZXYL_EFFECTPASS"))
	      Parameters |= EFFECTBUF_ZBUF | EFFECTBUF_LBUF | EFFECTBUF_WPOS;
	    else if (Description.Name == strstr(Description.Name, "oblv_CurrWorldProjectedNormals_EFFECTPASS"))
	      Parameters |= EFFECTBUF_ZBUF | EFFECTBUF_LBUF | EFFECTBUF_WPOS | EFFECTBUF_WNRM;
	  }
      }
    }
  }

  /* extract parameter handles */
  memset(h, 0, sizeof(h));
  for (int s = 0; s < hndl_Size; s++)
    h[s] = pEffect->GetParameterByName(NULL, hndl[s].Name);

  /* clear */
  Priority = (EFFECTGROUP_MAIN    << 24) |
	     (EFFECTCLASS_NEUTRAL << 17);
  Class = EFFECTCLASS_NEUTRAL;
  Flags = 0;
  Options = 0;
  OptionsCustom = 0;
  memset(FlagsPass, 0, sizeof(FlagsPass));
  memset(OptionsPass, 0, sizeof(OptionsPass));
  memset(OptionsCustomPass, 0, sizeof(OptionsCustomPass));
  D3DFORMAT format = D3DFMT_UNKNOWN;

  /* extract technique informations in reverse (top technique selected) */
  for (int teq = Description.Techniques - 1; teq >= 0; teq--) {
    D3DXHANDLE handle;

    if ((handle = pEffect->GetTechnique(teq))) {
      D3DXTECHNIQUE_DESC Description;
      pEffect->GetTechniqueDesc(handle, &Description);

      /* custom render-queue */
      bool prolog = false;
      if ((prolog = !stricmp(Description.Name, "prolog")))
	;
      else
	pEffect->SetTechnique(handle);

      for (int ann = 0; ann < Description.Annotations; ann++) {
	D3DXHANDLE handle2;

	if ((handle2 = pEffect->GetAnnotation(handle, ann))) {
	  D3DXPARAMETER_DESC Description;
	  pEffect->GetParameterDesc(handle2, &Description);

	  // int group = ...;
	  // int fxclass = ...;
	  // int rendertarget0-3 = ...;
	  // int conditions = ...;
	  // string dependency = ".fx";
	  // int dependency = ".fx";
	  if ((Description.Name == strstr(Description.Name, "group"))) {
	    if (Description.Type == D3DXPT_INT) {
	      INT pri; pEffect->GetInt(handle2, &pri);
	      this->Priority = (pri << 24) | (this->Priority & ~(0xFF << 24));
	      /* priority-field: <8bit group><7bit class><1bit order><16bit prio> */
	    }
	  }
	  else if ((Description.Name == strstr(Description.Name, "fxclass"))) {
	    if (Description.Type == D3DXPT_INT) {
	      INT cls; pEffect->GetInt(handle2, &cls);
	      this->Priority = (cls << 17) | (this->Priority & ~(0x7F << 17));
	      /* priority-field: <8bit group><7bit class><1bit order><16bit prio> */
	      this->Class = cls;
	    }
	  }
	  else if ((Description.Name == strstr(Description.Name, "rendertarget"))) {
	    if (Description.Type == D3DXPT_INT) {
	      pEffect->GetInt(handle2, (INT *)&format);
	      if ( prolog) if (!CustomQueue) CustomQueue = FXMan->RequestQueue(format);
	    }
	  }
	  else if ((Description.Name == strstr(Description.Name, "conditions"))) {
	    if (Description.Type == D3DXPT_INT) {
	      if (!prolog) pEffect->GetInt(handle2, &this->Flags);
	    }
	  }
	  else if ((Description.Name == strstr(Description.Name, "options"))) {
	    if (Description.Type == D3DXPT_INT) {
	      if (!prolog) pEffect->GetInt(handle2, &this->Options);
	      else         pEffect->GetInt(handle2, &this->OptionsCustom);
	    }
	  }
	  else if ((Description.Name == strstr(Description.Name, "dependency"))) {
	    if (Description.Type == D3DXPT_INT) {
	      INT dep; pEffect->GetInt(handle2, &dep);
	      INT prv = (this->Priority >> 16) & 0xFF;
	      if (dep < prv)
		dep = prv;
	      this->Priority = (dep << 17) | (1 << 16) | (this->Priority & ~(0xFF << 16));
	      /* priority-field: <8bit group><7bit class><1bit order><16bit prio> */
	    }
	    else if (Description.Type == D3DXPT_STRING) {
	      LPCSTR pString = NULL; pEffect->GetString(handle2, &pString);

	      EffectManager::GetSingleton()->AddDependtEffect(pString, 0);
	    }
	  }
	}
      }

      /* TODO, if just one pass, prev is not needed */
      for (int pas = 0; (pas < Description.Passes) && (pas < 16); pas++) {
	D3DXHANDLE handle2;

	if ((handle2 = pEffect->GetPass(handle, pas))) {
	  D3DXPASS_DESC Description;
	  pEffect->GetPassDesc(handle2, &Description);

	  for (int ann = 0; ann < Description.Annotations; ann++) {
	    D3DXHANDLE handle3;

	    if ((handle3 = pEffect->GetAnnotation(handle2, ann))) {
	      D3DXPARAMETER_DESC Description;
	      pEffect->GetParameterDesc(handle3, &Description);

	      // int conditions = ...;
	      if ((Description.Name == strstr(Description.Name, "conditions"))) {
		if (Description.Type == D3DXPT_INT) {
		  if (!prolog) pEffect->GetInt(handle3, &this->FlagsPass[pas]);
		}
	      }
	      else if ((Description.Name == strstr(Description.Name, "options"))) {
		if (Description.Type == D3DXPT_INT) {
		  if (!prolog) pEffect->GetInt(handle3, &this->OptionsPass[pas]);
		  else         pEffect->GetInt(handle3, &this->OptionsCustomPass[pas]);
		}
	      }
	    }
	  }
	}
      }

      /* no double-buffering for the custom pass, the main pass has always two buffers */
      if (Description.Passes == 1) {
	if (prolog) OptionsCustomPass[0] |= EFFECTOPT_SKIPSWAP;
	else        ;
      }

      /* if the first pass is skip, act accordingly */
      if (prolog) ParametersCustom |= ((OptionsCustomPass[0] & EFFECTOPT_SKIPSWAP) ? EFFECTBUF_SKIP : 0);
      else        Parameters       |= ((OptionsPass      [0] & EFFECTOPT_SKIPSWAP) ? EFFECTBUF_SKIP : 0);
    }
  }

  /* allocate and initialize custom queue (ready to go) */
  if (CustomQueue) {
    /* onepass, it's froozen, so only last is used all the time (no swap occurs) */
    CustomQueue->TrgtRT.Initialize(format); if (!(ParametersCustom & EFFECTBUF_SKIP))
    CustomQueue->LastRT.Initialize(format);

    /* Orig is a fake ... */
    CustomQueue->Init(NULL, &CustomQueue->LastRT, (OptionsCustom & EFFECTOPT_STENCIL) ? true : false);
    CustomQueue->Begin(&CustomQueue->OrigRT, &CustomQueue->TrgtRT, NULL);
  }

  /* release previous texture */
  std::vector<int>::iterator PTexture = prevTextures.begin();

  while (PTexture != prevTextures.end()) {
    TexMan->ReleaseTexture(*PTexture);
    PTexture++;
  }

  /* reflect changes in the applicable conditions */
  EffectManager::GetSingleton()->Recalculate();
}

inline void EffectRecord::ApplyPermanents(EffectManager *FXMan) {
#ifdef	OLD_QUEUE
  SetTexture(obge_PrevRendertarget0_EFFECTPASS, FXMan->thisframeTex);
  SetTexture(obge_LastRendertarget0_EFFECTPASS, FXMan->lastpassTex);
  SetTexture(obge_PastRendertarget0_MAINPASS  , FXMan->lastframeTex);
  SetTexture(oblv_CurrDepthStencilZ_MAINPASS  , FXMan->depth);
#else
//SetTexture(obge_PrevRendertarget0_EFFECTPASS, FXMan->thisframeTex);
//SetTexture(obge_LastRendertarget0_EFFECTPASS, FXMan->lastpassTex);
//SetTexture(obge_PastRendertarget0_MAINPASS  , FXMan->lastframeTex);

  /* past is a frame-buffer copy and never changes */
  FXMan->PastRT.SetTexture(h + obge_PastRendertarget0_MAINPASS, pEffect);

  /* convert WHATEVER to linearized form (CurrDS) */
  /* convert linearized to unlinearize form (OrigDS) */
  if (FXMan->RenderTransferZ) {
    if (FXMan->RenderTransferZ > EFFECTBUF_RAWZ) {
      FXMan->CurrNM.SetTexture(h[oblv_CurrWorldProjectedNormals_EFFECTPASS], pEffect);
      FXMan->CurrDS.SetTexture(h[oblv_CurrWorldProjectedZXYL_EFFECTPASS], pEffect);

      FXMan->CurrNM.SetTexture(h[oblv_CurrEyeProjectedNormals_EFFECTPASS], pEffect);
      FXMan->CurrDS.SetTexture(h[oblv_CurrEyeProjectedZXYD_EFFECTPASS], pEffect);

      FXMan->CurrDS.SetTexture(h[oblv_CurrLinearDepthZ_EFFECTPASS], pEffect);
      FXMan->OrigDS.SetTexture(h[oblv_CurrDepthStencilZ_MAINPASS], pEffect);
    }
    else
      /* convert linearized to unlinearize form (OrigDS) */
      FXMan->CurrDS.SetTexture(h[oblv_CurrDepthStencilZ_MAINPASS], pEffect);
  }
  else
    /* convert linearized to unlinearize form (OrigDS) */
    FXMan->OrigDS.SetTexture(h[oblv_CurrDepthStencilZ_MAINPASS], pEffect);
#endif

  SetVector(oblv_ReciprocalResolution_MAINPASS, &Constants.rcpres);

  /* deprecated */
#ifndef	NO_DEPRECATED
  SetBool(depr_bHasDepth, Constants.bHasDepth);
  SetFloatArray(depr_rcpres, (float *)&Constants.rcpres, 2);
#endif
}

inline void EffectRecord::ApplyCustomConstants() {
  if (CustomQueue)
    CustomQueue->Pass(h, pEffect);
}

inline void EffectRecord::ApplySharedConstants() {
  SetMatrix(oblv_WorldTransform_MAINPASS, &Constants.wrld);
//SetMatrix(oblv_WorldInverse_MAINPASS, &Constants.wrld_inv);
  SetMatrix(oblv_ViewTransform_MAINPASS, &Constants.view);
  SetMatrix(oblv_ViewInverse_MAINPASS, &Constants.view_inv);
  SetMatrix(oblv_ProjectionTransform_MAINPASS, &Constants.proj);
  SetMatrix(oblv_ProjectionInverse_MAINPASS, &Constants.proj_inv);

  SetMatrix(oblv_PastViewProjectionTransform_MAINPASS, &Constants.pastviewproj);
  SetMatrix(oblv_ViewProjectionTransform_MAINPASS, &Constants.viewproj);
  SetMatrix(oblv_ViewProjectionInverse_MAINPASS, &Constants.viewproj_inv);

  SetMatrix(oblv_PastWorldViewProjectionTransform_MAINPASS, &Constants.pastwrldviewproj);
  SetMatrix(oblv_WorldViewProjectionTransform_MAINPASS, &Constants.wrldviewproj);
  SetMatrix(oblv_WorldViewProjectionInverse_MAINPASS, &Constants.wrldviewproj_inv);

  SetFloatArray(oblv_CameraForward_MAINPASS, &Constants.EyeForward.x, 3);
  SetMatrix(oblv_CameraFrustum_MAINPASS, &Constants.EyeFrustum);
  SetVector(oblv_CameraPosition_MAINPASS, &Constants.EyePosition);

  SetVector(oblv_ProjectionDepthRange_MAINPASS, &Constants.ZRange);
  SetVector(oblv_ProjectionFoV_MAINPASS, &Constants.FoV);

  SetVector(oblv_FogRange, &Constants.FogRange);
  SetVector(oblv_FogColor, &Constants.FogColor);
  SetVector(oblv_SunDirection, &Constants.SunDir);
  SetVector(oblv_SunTiming, &Constants.SunTiming);

  SetIntArray(oblv_GameTime, &Constants.iGameTime.x, 4);
  SetIntArray(obge_Tick, &Constants.iTikTiming.x, 4);

#ifndef	NO_DEPRECATED
  SetVector(depr_f4Time, &Constants.time);
#endif

#ifndef	OBGE_NOSHADER
  SetTexture(oblv_Rendertarget0_REFLECTIONPASS, passTexture[OBGEPASS_REFLECTION]);

  /* deprecated */
#ifndef	NO_DEPRECATED
  SetBool(depr_bHasReflection, !!passTexture[OBGEPASS_REFLECTION]);
#endif
#endif
}

inline void EffectRecord::ApplyUniqueConstants() {
}

inline void EffectRecord::OnLostDevice(void) {
  if (HasEffect())
    pEffect->OnLostDevice();
}

inline void EffectRecord::OnResetDevice(void) {
  if (HasEffect())
    pEffect->OnResetDevice();
}

inline void EffectRecord::Render(IDirect3DDevice9 *D3DDevice, IDirect3DSurface9 *RenderTo, IDirect3DSurface9 *RenderCopy) {
  if (!IsEnabled())
    return;

  markerStart(D3DDevice);

  // Have to do this in case the effect has no vertex effect. The stupid effect system
  // uses the last vertex effect that was active and much strangeness occurs.
  D3DDevice->SetVertexShader(NULL);

  /* sampler-states have become increasingly complex, we really need to save some state from now on */
#ifdef	OBGE_STATEBLOCKS
  UINT pass = 0;
  UINT passes; pEffect->Begin(&passes, D3DXFX_DONOTSAVESHADERSTATE | D3DXFX_DONOTSAVESAMPLERSTATE /*| D3DXFX_DONOTSAVESTATE*/);
#else
  UINT pass = 0;
  UINT passes; pEffect->Begin(&passes, 0);
#endif

  while (true) {
    /* this sets the sampler-values */
    pEffect->BeginPass(pass);
    D3DDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
    pEffect->EndPass();

    if (++pass >= passes)
      break;

    minStretchRect(D3DDevice, RenderTo, 0, RenderCopy, 0, D3DTEXF_NONE);
  }

  pEffect->End();

  markerStop(D3DDevice);
}

inline bool EffectRecord::Render(IDirect3DDevice9 *D3DDevice, ManagedEffectQueue *Queue) {
  if (!IsEnabled())
    return false;

  markerStart(D3DDevice);

  // Have to do this in case the effect has no vertex effect. The stupid effect system
  // uses the last vertex effect that was active and much strangeness occurs.
  D3DDevice->SetVertexShader(NULL);

#ifndef OBGE_CONSTANTPOOLS
  ApplySharedConstants();
#endif
  ApplyUniqueConstants();

  /* sampler-states have become increasingly complex, we really need to save some state from now on */
#ifdef	OBGE_STATEBLOCKS
  UINT pass = 0; Queue->Begin(h, pEffect, ParametersCustom);
  UINT passes; pEffect->Begin(&passes, D3DXFX_DONOTSAVESHADERSTATE | D3DXFX_DONOTSAVESAMPLERSTATE /*| D3DXFX_DONOTSAVESTATE*/);
#else
  UINT pass = 0; Queue->Begin(h, pEffect, Parameters);
  UINT passes; pEffect->Begin(&passes, 0);
#endif

  /* Begin() makes no swap */
  do {
    /* allow freeze of swap-chain (fe. just one pass) */
    if (!(OptionsCustomPass[pass] & EFFECTOPT_SKIPSWAP))
      Queue->Swap(h, pEffect);

    /* this sets the sampler-values */
    pEffect->BeginPass(pass);
    D3DDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
    pEffect->EndPass();
  } while (++pass < passes);

  pEffect->End();

#ifdef	OBGE_DEVLING
  sprintf(Prolog, "%s [prolog]", this->GetName());
  passScene = Prolog;
#endif

  markerStop(D3DDevice);

  Queue->End(h, pEffect);

  return true;
}

inline bool EffectRecord::Render(IDirect3DDevice9 *D3DDevice, EffectQueue *Queue) {
  if (!IsEnabled())
    return false;

  /* run the custom queue first (with almost all bells'n'wistles) */
  if (CustomQueue) {
    D3DXHANDLE teq = pEffect->GetCurrentTechnique();
    D3DXHANDLE pro = pEffect->GetTechniqueByName("prolog");

    pEffect->SetTechnique(pro);
    CustomQueue->device = D3DDevice;
    Render(D3DDevice, CustomQueue);
    pEffect->SetTechnique(teq);

    ApplyCustomConstants();
  }

  markerStart(D3DDevice);

  // Have to do this in case the effect has no vertex effect. The stupid effect system
  // uses the last vertex effect that was active and much strangeness occurs.
  D3DDevice->SetVertexShader(NULL);

#ifndef OBGE_CONSTANTPOOLS
  ApplySharedConstants();
#endif
  ApplyUniqueConstants();

  /* sampler-states have become increasingly complex, we really need to save some state from now on */
#ifdef	OBGE_STATEBLOCKS
  UINT pass = 0; Queue->Begin(h, pEffect, Parameters);
  UINT passes; pEffect->Begin(&passes, D3DXFX_DONOTSAVESHADERSTATE | D3DXFX_DONOTSAVESAMPLERSTATE /*| (Options & EFFECTOPT_BLENDING ? 0 : D3DXFX_DONOTSAVESTATE)*/);
#else
  UINT pass = 0; Queue->Begin(h, pEffect, Parameters);
  UINT passes; pEffect->Begin(&passes, 0);
#endif

  /* Begin() makes no swap */
  do {
    /* allow freeze of swap-chain (fe. writing to temporary rendertargets) */
    if (!(OptionsPass[pass] & EFFECTOPT_SKIPSWAP))
      Queue->Swap(h, pEffect);

    /* this sets the sampler-values */
    pEffect->BeginPass(pass);
    D3DDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
    pEffect->EndPass();
  } while (++pass < passes);

  pEffect->End();

#ifdef	OBGE_DEVLING
  passScene = this->GetName();
#endif

  markerStop(D3DDevice);

  Queue->End(h, pEffect);

  return true;
}

inline void EffectRecord::Render(IDirect3DDevice9 *D3DDevice) {
  if (!IsEnabled())
    return;

  markerStart(D3DDevice);

  // Have to do this in case the effect has no vertex effect. The stupid effect system
  // uses the last vertex effect that was active and much strangeness occurs.
  D3DDevice->SetVertexShader(NULL);

#ifndef OBGE_CONSTANTPOOLS
  ApplySharedConstants();
#endif
  ApplyUniqueConstants();

  /* sampler-states have become increasingly complex, we really need to save some state from now on */
#ifdef	OBGE_STATEBLOCKS
  UINT pass = 0;
  UINT passes; pEffect->Begin(&passes, D3DXFX_DONOTSAVESHADERSTATE | D3DXFX_DONOTSAVESAMPLERSTATE /*| D3DXFX_DONOTSAVESTATE*/);
#else
  UINT pass = 0;
  UINT passes; pEffect->Begin(&passes, 0);
#endif

  do {
    pEffect->BeginPass(pass);
    D3DDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
    pEffect->EndPass();
  } while (++pass < passes);

  pEffect->End();

  markerStop(D3DDevice);
}

ID3DXEffect *EffectRecord::GetEffect() const {
  return this->pEffect;
}

bool EffectRecord::HasEffect() const {
  return (this->pEffect != NULL);
}

void EffectRecord::Enable(bool Enabled) {
  /* do nothing */
  if (this->Enabled == Enabled)
    return;

  this->Enabled = Enabled;

  /* reflect changes in the applicable conditions */
  EffectManager::GetSingleton()->Recalculate();
}

bool EffectRecord::IsEnabled() const {
  return this->Enabled && HasEffect();
}

inline UINT32 EffectRecord::GetRefID() const {
  return this->ParentRefID;
}

inline const char *EffectRecord::GetPath() const {
  return this->Filepath;
}

inline const char *EffectRecord::GetName() const {
  return this->Name;
}

inline bool EffectRecord::IsPrivate() const {
  return this->Private;
}

inline unsigned long EffectRecord::GetParameters() const {
  return this->Parameters;
}

inline unsigned long EffectRecord::GetConditions() const {
  return this->Flags;
}

inline unsigned long EffectRecord::GetConditions(int pass) const {
  return this->FlagsPass[pass];
}

inline unsigned long EffectRecord::GetOptions() const {
  return this->Options;
}

inline unsigned long EffectRecord::GetOptions(int pass) const {
  return this->OptionsPass[pass];
}

inline void EffectRecord::SetPriority(int pri) {
  this->Priority = (this->Priority & (0xFF << 24)) | (pri & ~(0xFF << 24));
}

bool EffectRecord::GetEffectConstantHelps(std::map<std::string,std::string> &all) const {
  if (!HasEffect()) return false;
  all.clear();

  D3DXEFFECT_DESC Description;
  pEffect->GetDesc(&Description);

  /* extract parameter informations */
  for (int par = 0; par < Description.Parameters; par++) {
    D3DXHANDLE handle, handle2;

    if ((handle = pEffect->GetParameter(NULL, par))) {
      D3DXPARAMETER_DESC Description;
      pEffect->GetParameterDesc(handle, &Description);
      std::string name; name.assign(Description.Name);

      /* filter */
      if (!strncmp(Description.Name, "oblv_", 5) ||
	  !strncmp(Description.Name, "obge_", 5))
	continue;

      if ((handle2 = pEffect->GetAnnotationByName(handle, "help"))) {
	LPCSTR pString = NULL; pEffect->GetString(handle2, &pString);

	all[name] = pString;
      }
    }
  }

  return true;
}

bool EffectRecord::GetEffectConstantTypes(std::map<std::string,int> &all) const {
  if (!HasEffect()) return false;
  all.clear();

  D3DXEFFECT_DESC Description;
  pEffect->GetDesc(&Description);

  /* extract parameter informations */
  for (int par = 0; par < Description.Parameters; par++) {
    D3DXHANDLE handle;

    if ((handle = pEffect->GetParameter(NULL, par))) {
      D3DXPARAMETER_DESC Description;
      pEffect->GetParameterDesc(handle, &Description);
      std::string name; name.assign(Description.Name);

      /* filter */
      if (!strncmp(Description.Name, "oblv_", 5) ||
	  !strncmp(Description.Name, "obge_", 5))
	continue;

      switch (Description.Type) {
	case D3DXPT_VOID:	  all[name] = 0; break;
	case D3DXPT_BOOL:	  all[name] = 1; break;
	case D3DXPT_INT:	  all[name] = 2; break;
	case D3DXPT_FLOAT:	  all[name] = 3; break;
	case D3DXPT_STRING:	  all[name] = 4; break;
	case D3DXPT_TEXTURE:	  all[name] = 5; break;
	case D3DXPT_TEXTURE1D:	  all[name] = 6; break;
	case D3DXPT_TEXTURE2D:	  all[name] = 7; break;
	case D3DXPT_TEXTURE3D:	  all[name] = 8; break;
	case D3DXPT_TEXTURECUBE:  all[name] = 9; break;
	case D3DXPT_SAMPLER:	  all[name] = 10; break;
	case D3DXPT_SAMPLER1D:	  all[name] = 11; break;
	case D3DXPT_SAMPLER2D:	  all[name] = 12; break;
	case D3DXPT_SAMPLER3D:	  all[name] = 13; break;
	case D3DXPT_SAMPLERCUBE:  all[name] = 14; break;
      }
    }
  }

  return true;
}

bool EffectRecord::GetEffectConstantHelp(const char *name, const char **help) const {
  D3DXHANDLE hl = (HasEffect() ? pEffect->GetParameterByName(NULL, name) : NULL);
  if (hl) {
    D3DXPARAMETER_DESC desc;
    HRESULT hr = pEffect->GetParameterDesc(hl, &desc);
    if (hr == D3D_OK) {
      D3DXHANDLE hl2;

      *help = "No help available";
      if ((hl2 = pEffect->GetAnnotationByName(hl, "help"))) {
	LPCSTR pString = NULL; pEffect->GetString(hl2, &pString);

	*help = pString;
      }

      return true;
    }
  }

  return false;
}

bool EffectRecord::GetEffectConstantType(const char *name, int *type) const {
  D3DXHANDLE hl = (HasEffect() ? pEffect->GetParameterByName(NULL, name) : NULL);
  if (hl) {
    D3DXPARAMETER_DESC desc;
    HRESULT hr = pEffect->GetParameterDesc(hl, &desc);
    if (hr == D3D_OK) {
      switch (desc.Type) {
	default:		 *type = -1; break;
	case D3DXPT_VOID:        *type =  0; break;
	case D3DXPT_BOOL:        *type =  1; break;
	case D3DXPT_INT:         *type =  2; break;
	case D3DXPT_FLOAT:       *type =  3; break;
	case D3DXPT_STRING:      *type =  4; break;
	case D3DXPT_TEXTURE:     *type =  5; break;
	case D3DXPT_TEXTURE1D:   *type =  6; break;
	case D3DXPT_TEXTURE2D:   *type =  7; break;
	case D3DXPT_TEXTURE3D:   *type =  8; break;
	case D3DXPT_TEXTURECUBE: *type =  9; break;
	case D3DXPT_SAMPLER:     *type = 10; break;
	case D3DXPT_SAMPLER1D:   *type = 11; break;
	case D3DXPT_SAMPLER2D:   *type = 12; break;
	case D3DXPT_SAMPLER3D:   *type = 13; break;
	case D3DXPT_SAMPLERCUBE: *type = 14; break;
      }

      return true;
    }
  }

  return false;
}

bool EffectRecord::GetEffectConstantB(const char *name, BOOL *value) const {
  D3DXHANDLE hl = (HasEffect() ? pEffect->GetParameterByName(NULL, name) : NULL);
  HRESULT hr = (hl ? pEffect->GetBool(hl, (BOOL *)value) : -1);
  return (hr == D3D_OK);
}

bool EffectRecord::GetEffectConstantI(const char *name, int *value) const {
  D3DXHANDLE hl = (HasEffect() ? pEffect->GetParameterByName(NULL, name) : NULL);
  HRESULT hr = (hl ? pEffect->GetInt(hl, value) : -1);
  return (hr == D3D_OK);
}

bool EffectRecord::GetEffectConstantI(const char *name, int *values, int num) const {
  D3DXHANDLE hl = (HasEffect() ? pEffect->GetParameterByName(NULL, name) : NULL);
  HRESULT hr = (hl ? pEffect->GetIntArray(hl, values, num) : -1);
  return (hr == D3D_OK);
}

bool EffectRecord::GetEffectConstantF(const char *name, float *value) const {
  D3DXHANDLE hl = (HasEffect() ? pEffect->GetParameterByName(NULL, name) : NULL);
  HRESULT hr = (hl ? pEffect->GetFloat(hl, value) : -1);
  return (hr == D3D_OK);
}

bool EffectRecord::GetEffectConstantF(const char *name, float *values, int num) const {
  D3DXHANDLE hl = (HasEffect() ? pEffect->GetParameterByName(NULL, name) : NULL);
  HRESULT hr = (hl ? pEffect->GetFloatArray(hl, values, num) : -1);
  return (hr == D3D_OK);
}

bool EffectRecord::GetEffectConstantV(const char *name, float *value) const {
  D3DXHANDLE hl = (HasEffect() ? pEffect->GetParameterByName(NULL, name) : NULL);
  HRESULT hr = (hl ? pEffect->GetVector(hl, (D3DXVECTOR4 *)value) : -1);
  return (hr == D3D_OK);
}

bool EffectRecord::GetEffectSamplerTexture(const char *name, int *TextureNum) const {
  D3DXHANDLE hl = (HasEffect() ? pEffect->GetParameterByName(NULL, name) : NULL);
  if (!hl) return false;

  /* get old one */
  IDirect3DBaseTexture9 *OldTexture = NULL;
  pEffect->GetTexture(hl, (LPDIRECT3DBASETEXTURE9 *)&OldTexture);

  /* remove any trace of the texture from this effect */
  if (OldTexture) {
    TextureManager *TexMan = TextureManager::GetSingleton();
    *TextureNum = TexMan->FindTexture(OldTexture);
    return true;
  }

  return false;
}

bool EffectRecord::SetEffectConstantB(const char *name, BOOL value) {
  D3DXHANDLE hl = (HasEffect() ? pEffect->GetParameterByName(NULL, name) : NULL);
  HRESULT hr = (hl ? pEffect->SetBool(hl, value) : -1);
  return (hr == D3D_OK);
}

bool EffectRecord::SetEffectConstantI(const char *name, int value) {
  D3DXHANDLE hl = (HasEffect() ? pEffect->GetParameterByName(NULL, name) : NULL);
  HRESULT hr = (hl ? pEffect->SetInt(hl, value) : -1);
  return (hr == D3D_OK);
}

bool EffectRecord::SetEffectConstantI(const char *name, int *values, int num) {
  D3DXHANDLE hl = (HasEffect() ? pEffect->GetParameterByName(NULL, name) : NULL);
  HRESULT hr = (hl ? pEffect->SetIntArray(hl, values, num) : -1);
  return (hr == D3D_OK);
}

bool EffectRecord::SetEffectConstantF(const char *name, float value) {
  D3DXHANDLE hl = (HasEffect() ? pEffect->GetParameterByName(NULL, name) : NULL);
  HRESULT hr = (hl ? pEffect->SetFloat(hl, value) : -1);
  return (hr == D3D_OK);
}

bool EffectRecord::SetEffectConstantF(const char *name, float *values, int num) {
  D3DXHANDLE hl = (HasEffect() ? pEffect->GetParameterByName(NULL, name) : NULL);
  HRESULT hr = (hl ? pEffect->SetFloatArray(hl, values, num) : -1);
  return (hr == D3D_OK);
}

bool EffectRecord::SetEffectConstantV(const char *name, v1_2_416::NiVector4 *value) {
  D3DXHANDLE hl = (HasEffect() ? pEffect->GetParameterByName(NULL, name) : NULL);
  HRESULT hr = (hl ? pEffect->SetVector(hl, value) : -1);
  return (hr == D3D_OK);
}

bool EffectRecord::SetEffectSamplerTexture(const char *name, int TextureNum) {
  D3DXHANDLE hl = (HasEffect() ? pEffect->GetParameterByName(NULL, name) : NULL);
  if (!hl) return false;

  TextureManager *TexMan = TextureManager::GetSingleton();

  /* get old one */
  IDirect3DBaseTexture9 *OldTexture = NULL;
  pEffect->GetTexture(hl, (LPDIRECT3DBASETEXTURE9 *)&OldTexture);
  /* and dereference */
  pEffect->SetTexture(hl, NULL);

  /* remove any trace of the texture from this effect */
  if (OldTexture) {
    int OldTextureNum = TexMan->FindTexture(OldTexture);
    TexMan->ReleaseTexture(OldTexture);

    /* remove from vector */
    if (OldTextureNum != -1) {
      /* precaution (should always work) */
      std::vector<int>::iterator OldPos;
      OldPos = std::find(Textures.begin(), Textures.end(), OldTextureNum);
      if (OldPos != Textures.end())
	Textures.erase(OldPos);
    }
  }

  /* apply the new texture and remember it */
  TextureRecord *NewTexture = TexMan->GetTexture(TextureNum);
  if (NewTexture) {
    HRESULT hr = pEffect->SetTexture(hl, NewTexture->GetTexture());

    /* add to vector */
    if (hr == D3D_OK)
      Textures.push_back(TextureNum);

    return (hr == D3D_OK);
  }

  return false;
}

void EffectRecord::PurgeTexture(IDirect3DBaseTexture9 *texture, int TexNum) {
  if (!HasEffect()) return;
  D3DXEFFECT_DESC Description;
  pEffect->GetDesc(&Description);

  for (int par = 0; par < Description.Parameters; par++) {
    D3DXHANDLE handle;

    if ((handle = pEffect->GetParameter(NULL, par))) {
      D3DXPARAMETER_DESC Description;

      pEffect->GetParameterDesc(handle, &Description);

      if ((Description.Type = D3DXPT_TEXTURE) ||
	  (Description.Type = D3DXPT_TEXTURE1D) ||
	  (Description.Type = D3DXPT_TEXTURE2D) ||
	  (Description.Type = D3DXPT_TEXTURE3D) ||
	  (Description.Type = D3DXPT_TEXTURECUBE)) {
	// NB must set to NULL otherwise strange things happen
	IDirect3DBaseTexture9 *EffectTexture = NULL;
	pEffect->GetTexture(handle, &EffectTexture);

	if (EffectTexture == texture) {
	  pEffect->SetTexture(handle, NULL);

	  _DMESSAGE("Removing texture %s from effect %s", Description.Name, GetPath());

	  /* remove from vector */
	  if (TexNum != -1) {
	    /* this doesn't necessarily match */
	    std::vector<int>::iterator Pos;
	    Pos = std::find(Textures.begin(), Textures.end(), TexNum);
	    if (Pos != Textures.end())
	      Textures.erase(Pos);
	  }
	}
      }
    }
  }
}

void EffectRecord::SaveVars(OBSESerializationInterface *Interface) {
  if (!HasEffect()) return;
  TextureManager *TexMan = TextureManager::GetSingleton();
  D3DXEFFECT_DESC Description;
  pEffect->GetDesc(&Description);

  _MESSAGE("pEffect %s has %d parameters.", Filepath, Description.Parameters);

  for (int par = 0; par < Description.Parameters; par++) {
    D3DXHANDLE handle;

    if ((handle = pEffect->GetParameter(NULL, par))) {
      D3DXPARAMETER_DESC Description;
      pEffect->GetParameterDesc(handle, &Description);

      switch (Description.Type) {
        case D3DXPT_TEXTURE:
        case D3DXPT_TEXTURE1D:
        case D3DXPT_TEXTURE2D:
        case D3DXPT_TEXTURE3D:
	case D3DXPT_TEXTURECUBE: {
          IDirect3DBaseTexture9 *Texture = NULL;
          int TextureNum; TextureType TextureData;
	  ManagedTextureRecord *MTexture;

	  /* find the DX-resource */
          pEffect->GetTexture(handle, &Texture);
	  /* and the corresponding ID */
          TextureNum = TexMan->FindTexture(Texture);

	  if (TextureNum == -1)
	    break;

	  /* and the corresponding structure */
	  MTexture = TexMan->GetTexture(TextureNum);

	  /* save the texture only if it's not private,
	   * those in the effect-files themself
	   * are private textures for example
	   */
	  if (!MTexture->IsPrivate()) {
	    TextureData.tex = TextureNum;
	    strcpy(TextureData.Name, Description.Name);
	    Interface->WriteRecord('STEX', SHADERVERSION, &TextureData, sizeof(TextureData));

	    _MESSAGE("Found texture: name - %s, texnum - %d", TextureData.Name, TextureNum);
	  }

	} break;
        case D3DXPT_INT: {
          IntType IntData;

          IntData.size = Description.Elements;
          if (IntData.size == 0)
            IntData.size = 1;

          pEffect->GetIntArray(handle, (int *)&IntData.data, IntData.size);
          strcpy(IntData.Name, Description.Name);
          Interface->WriteRecord('SINT', SHADERVERSION, &IntData, sizeof(IntData));

          _MESSAGE("Found int: name - %s, size - %d, data[0] - %d", IntData.Name, IntData.size, IntData.data[0]);

        } break;
        case D3DXPT_FLOAT: {
          FloatType FloatData;

          FloatData.size = Description.Elements;
          if (FloatData.size == 0)
            FloatData.size = 1;

          pEffect->GetFloatArray(handle, (float *)&FloatData.data, FloatData.size);
          strcpy(FloatData.Name, Description.Name);
          Interface->WriteRecord('SFLT', SHADERVERSION, &FloatData, sizeof(FloatData));

          _MESSAGE("Found float: name - %s, size - %d, data[0] - %f", FloatData.Name, FloatData.size, FloatData.data[0]);

        } break;
      }
    }
  }
}

// *********************************************************************************************************

ManagedEffectRecord::ManagedEffectRecord() {
  RefCount = 0;
}

ManagedEffectRecord::~ManagedEffectRecord() {
}

void ManagedEffectRecord::ClrRef() {
  RefCount = 0;
}

int ManagedEffectRecord::AddRef() {
  return ++RefCount;
}

int ManagedEffectRecord::Release() {
  if (RefCount)
    RefCount--;

  if (!RefCount) {
//  delete this;
    return 0;
  }

  return RefCount;
}

// *********************************************************************************************************

EffectManager *EffectManager::Singleton = NULL;

EffectManager::EffectManager() {
  LARGE_INTEGER freq;

  QueryPerformanceFrequency(&freq);

  Constants.iTikTiming.w = (int)(freq.QuadPart);

  EffectIndex = 0;
  MaxEffectIndex = 0;

  EffectPool = NULL;
  EffectVertex = NULL;
  EffectDepth = NULL;
  EffectShare = NULL;

#ifdef	OLD_QUEUE
  // these are all private
  thisframeTex = NULL;
  lastpassTex = NULL;
  lastframeTex = NULL;
  thisframeSurf = NULL;
  lastpassSurf = NULL;
  lastframeSurf = NULL;

  HasDepth = false;

  depth = NULL;
  depthSurface = NULL;
  depthRAWZ = NULL;

  RAWZflag = false;
#else
  RenderTransferZ = (IsRAWZ() ? EFFECTBUF_RAWZ : 0);
  RenderBuf = 0;
  RenderCnd = 0;
  RenderOpt = 0;
  RenderFmt = D3DFMT_UNKNOWN;
#endif

  /* enable FETCH4, how stupid, no "(1.1f != 0)" possible for the preprocessor */
  defs[DEFS_FETCH4 ].Definition = (DoesFCH4() ? "1" : "0");
  defs[DEFS_FETCH4G].Definition = (DoesFCH4() ? stringify(GET4_FLOAT) : "0");
  defs[DEFS_FETCH1G].Definition = (DoesFCH4() ? stringify(GET1_FLOAT) : "0");

  /* freeze parameters */
  if (FreezeTweaks.Get())
    defs[DEFS_iface].Definition = TWEAK_FROZEN;
  else
    defs[DEFS_iface].Definition = TWEAK_DYNAMIC;
}

EffectManager::~EffectManager() {
  Singleton = NULL;

  if (EffectPool)
    while (EffectPool->Release()) {};
  if (EffectVertex)
    while (EffectVertex->Release()) {};

#ifdef	OLD_QUEUE
  if (thisframeSurf)
    thisframeSurf->Release();
  if (lastframeSurf)
    lastframeSurf->Release();
  if (lastpassSurf)
    lastpassSurf->Release();

  if (thisframeTex)
    while (thisframeTex->Release()) {};
  if (lastframeTex)
    while (lastframeTex->Release()) {};
  if (lastpassTex)
    while (lastpassTex->Release()) {};

  if (IsRAWZ() && depth)
    while (depth->Release()) {};
#else
  if (RenderBuf & EFFECTBUF_COPY) CopyRT.Release();
  if (RenderBuf & EFFECTBUF_PAST) PastRT.Release();
  if (RenderBuf & EFFECTBUF_PREV) PrevRT.Release();
                                  LastRT.Release();

  if (RenderBuf & EFFECTBUF_ZBUF) if (RenderTransferZ)
                                  CurrDS.Release(),
                                  CurrNM.Release();
#endif
}

EffectManager *EffectManager::GetSingleton() {
  if (!EffectManager::Singleton)
    EffectManager::Singleton = new EffectManager();

  return EffectManager::Singleton;
}

void EffectManager::Reset() {
  ManagedEffects.clear();
  Effects.clear();

  EffectIndex = 0;
  MaxEffectIndex = 0;

#ifdef	OLD_QUEUE
#else
  RenderBuf = 0;
  RenderCnd = 0;
  RenderOpt = 0;
#endif
}

bool EffectManager::SetTransferZ(long MaskZ) {
  /* mask out Z-relevant bits */
  MaskZ |= (IsRAWZ() ? EFFECTBUF_RAWZ : 0);
  MaskZ &= EFFECTBUF_TRANSFERZMASK;

  /* nodo */
  if (RenderTransferZ == MaskZ)
    return true;

  /* redo */
  if ((RenderTransferZ & (EFFECTBUF_LBUF | EFFECTBUF_EPOS | EFFECTBUF_WPOS)) !=
      (MaskZ           & (EFFECTBUF_LBUF | EFFECTBUF_EPOS | EFFECTBUF_WPOS)))
    CurrDS.Release();

  if ((RenderTransferZ & (EFFECTBUF_LBUF | EFFECTBUF_ENRM | EFFECTBUF_WNRM)) !=
      (MaskZ           & (EFFECTBUF_LBUF | EFFECTBUF_ENRM | EFFECTBUF_WNRM)))
    CurrNM.Release();

//if (RenderTransferZ & (EFFECTBUF_RAWZ) !=
//    MaskZ           & (EFFECTBUF_RAWZ))
//  delete EffectDepth;

  /* put all indicators into the effects (optimize) */
  defs[DEFS_INRAWZ].Definition[0] = (MaskZ &           EFFECTBUF_RAWZ          ? '1' : '0');
  defs[DEFS_INLINZ].Definition[0] = (MaskZ &           EFFECTBUF_LBUF          ? '1' : '0');
  defs[DEFS_INPRJZ].Definition[0] = (MaskZ & (EFFECTBUF_EPOS | EFFECTBUF_WPOS) ? '1' : '0');
  defs[DEFS_INNRMZ].Definition[0] = (MaskZ & (EFFECTBUF_ENRM | EFFECTBUF_WNRM) ? '1' : '0');

  if (MaskZ && !EffectDepth) {
    EffectDepth = new EffectRecord();

    if (!EffectDepth->LoadEffect("TransferZ.fx", 0) ||
	!EffectDepth->CompileEffect(this)) {
      delete EffectDepth;
      EffectDepth = NULL;

      _MESSAGE("ERROR - TransferZ.fx is missing and required! Please reinstall OBGE");
      exit(0); return false;
    }

    EffectDepth->Enable(true);
  }
  else if (!MaskZ && EffectDepth) {
    delete EffectDepth;
    EffectDepth = NULL;
  }

  RenderTransferZ = MaskZ;
  return true;
}

void EffectManager::InitializeFrameTextures() {
#ifdef	OLD_QUEUE
  HRESULT hr;

  if ((BufferTexturesNumBits.data > 32) ||
      ((BufferTexturesNumBits.data % 8) > 0))
    BufferTexturesNumBits.data = 8;

  UInt32 Width = v1_2_416::GetRenderer()->SizeWidth;
  UInt32 Height = v1_2_416::GetRenderer()->SizeHeight;

  _MESSAGE("Creating full screen textures.");
  _MESSAGE("Width = %d, Height = %d", Width, Height);

  if (BufferTexturesNumBits.data == 32) {
    hr = GetD3DDevice()->CreateTexture(Width, Height, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A32B32G32R32F, D3DPOOL_DEFAULT, &thisframeTex, 0);

    if (FAILED(hr)) {
      thisframeTex->Release();
      thisframeTex = NULL;

      BufferTexturesNumBits.data = 16;
    }
    else {
      GetD3DDevice()->CreateTexture(Width, Height, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A32B32G32R32F, D3DPOOL_DEFAULT, &lastpassTex, 0);
      GetD3DDevice()->CreateTexture(Width, Height, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A32B32G32R32F, D3DPOOL_DEFAULT, &lastframeTex, 0);
    }
  }

  if (BufferTexturesNumBits.data == 16) {
    hr = GetD3DDevice()->CreateTexture(Width, Height, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A16B16G16R16F, D3DPOOL_DEFAULT, &thisframeTex, 0);

    if (FAILED(hr)) {
      thisframeTex->Release();
      thisframeTex = NULL;

      BufferTexturesNumBits.data = 8;
    }
    else {
      GetD3DDevice()->CreateTexture(Width, Height, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A16B16G16R16F, D3DPOOL_DEFAULT, &lastpassTex, 0);
      GetD3DDevice()->CreateTexture(Width, Height, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A16B16G16R16F, D3DPOOL_DEFAULT, &lastframeTex, 0);
    }
  }

  if (BufferTexturesNumBits.data == 8) {
    GetD3DDevice()->CreateTexture(Width, Height, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &thisframeTex, 0);
    GetD3DDevice()->CreateTexture(Width, Height, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &lastpassTex, 0);
    GetD3DDevice()->CreateTexture(Width, Height, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &lastframeTex, 0);
  }

  _MESSAGE("Setting full screen surfaces.");

  thisframeTex->GetSurfaceLevel(0, &thisframeSurf);
  lastpassTex->GetSurfaceLevel(0, &lastpassSurf);
  lastframeTex->GetSurfaceLevel(0, &lastframeSurf);

  _MESSAGE("Setting depth texture.");

  if (IsRAWZ()) {
    GetD3DDevice()->CreateTexture(v1_2_416::GetRenderer()->SizeWidth, v1_2_416::GetRenderer()->SizeHeight, 1, D3DUSAGE_RENDERTARGET, D3DFMT_R32F, D3DPOOL_DEFAULT, &depth, 0);

    depthRAWZ = GetDepthBufferTexture();
    depth->GetSurfaceLevel(0, &depthSurface);

    RAWZflag = true;
    _MESSAGE("RAWZ depth texture - fix applied.");
  }
  else
    depth = GetDepthBufferTexture();
#else
  IDirect3D9 *lastOBGEDirect3D9;
  lastOBGEDirect3DDevice9->GetDirect3D(&lastOBGEDirect3D9);

  int bitz = BufferRawZDepthNumBits.Get();
  int bits = BufferTexturesNumBits.Get();

  if (RenderFmt == D3DFMT_UNKNOWN) {
    D3DFORMAT frmt = D3DFMT_UNKNOWN;
    D3DDISPLAYMODE d3ddm;

    if (lastOBGEDirect3D9->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &d3ddm) == D3D_OK) {
      while (bits != 0) {
	/**/ if (bits >=  32) frmt = (RenderBuf & EFFECTBUF_ACHN ? D3DFMT_A32B32G32R32F : D3DFMT_A32B32G32R32F);
	else if (bits >=  16) frmt = (RenderBuf & EFFECTBUF_ACHN ? D3DFMT_A16B16G16R16F : D3DFMT_A16B16G16R16F);
	else if (bits >=   8) frmt = (RenderBuf & EFFECTBUF_ACHN ? D3DFMT_A8R8G8B8      : D3DFMT_X8R8G8B8);
	else if (bits <= -16) frmt = (RenderBuf & EFFECTBUF_ACHN ? D3DFMT_A16B16G16R16  : D3DFMT_A16B16G16R16);
	else if (bits <= -10) frmt = (RenderBuf & EFFECTBUF_ACHN ? D3DFMT_A2B10G10R10   : D3DFMT_A2B10G10R10);
	else if (bits <=  -8) frmt = (RenderBuf & EFFECTBUF_ACHN ? D3DFMT_A8R8G8B8      : D3DFMT_X8R8G8B8);
	else if (bits <=  -5) frmt = (RenderBuf & EFFECTBUF_ACHN ? D3DFMT_A1R5G5B5      : D3DFMT_R5G6B5);
	else if (bits <=  -4) frmt = (RenderBuf & EFFECTBUF_ACHN ? D3DFMT_A4R4G4B4      : D3DFMT_X4R4G4B4);
	else if (bits <=  -3) frmt = (RenderBuf & EFFECTBUF_ACHN ? D3DFMT_R3G3B2	: D3DFMT_R3G3B2);
	else                  frmt = D3DFMT_UNKNOWN;

	if (frmt != D3DFMT_UNKNOWN) {
	  HRESULT hr;
	  if ((hr = lastOBGEDirect3D9->CheckDeviceFormat(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, d3ddm.Format, D3DUSAGE_RENDERTARGET, D3DRTYPE_SURFACE, frmt)) == D3D_OK)
	    break;
	}

	/**/ if (bits >=  32) bits =  16;
	else if (bits >=  16) bits =   8;
	else if (bits >=   8) bits =   0;
	else if (bits <= -16) bits = -10;
	else if (bits <= -10) bits =  -8;
	else if (bits <=  -8) bits =  -5;
	else if (bits <=  -5) bits =  -4;
	else if (bits <=  -4) bits =  -3;
	else if (bits <=  -3) bits =   0;
      }
    }

    /* TODO extract this out of Oblivion and make the
     * interior/exterior format switch possible
     */
    if (frmt == D3DFMT_UNKNOWN) {
      if (IsHDR())
	bits = 16, frmt = D3DFMT_A16B16G16R16F;
      else
	bits = 8, frmt = D3DFMT_A8R8G8B8;
    }

    if (frmt != D3DFMT_UNKNOWN) {
      _MESSAGE("Creating full screen textures.");
      _MESSAGE("Width = %d, Height = %d", v1_2_416::GetRenderer()->SizeWidth, v1_2_416::GetRenderer()->SizeHeight);
      _MESSAGE("Format = %s", findFormat(frmt));

      RenderFmt = frmt;
    }
  }

  if (RenderFmt != D3DFMT_UNKNOWN) {
    /* TODO extract this out of Oblivion and make the
     * interior/exterior format switch possible
     */
    if (IsHDR())
      RenderBuf |= (RenderFmt != D3DFMT_A16B16G16R16F) ? EFFECTBUF_COPY : 0;
    else
      RenderBuf |= (RenderFmt != D3DFMT_A8R8G8B8     ) ? EFFECTBUF_COPY : 0;

    /* it seems the rendertarget is resolved just fine without the copy */
    if (IsMultiSampled() && 0)
      RenderBuf |=					 EFFECTBUF_COPY    ;

    if (RenderBuf & EFFECTBUF_ZBUF) OrigDS.Initialize(GetDepthBufferTexture());
    if (RenderBuf & EFFECTBUF_COPY) CopyRT.Initialize(RenderFmt);
    if (RenderBuf & EFFECTBUF_PAST) PastRT.Initialize(RenderFmt);
    if (RenderBuf & EFFECTBUF_PREV) PrevRT.Initialize(RenderFmt);
				    LastRT.Initialize(RenderFmt);

    if (RenderBuf & EFFECTBUF_ZBUF) if (RenderTransferZ)
      CurrDS.Initialize(
	RenderTransferZ & EFFECTBUF_EPOS
	  ? (!bitz || (bitz > 16) ? D3DFMT_A32B32G32R32F : D3DFMT_A16B16G16R16F)  // linear
	  :
	RenderTransferZ & EFFECTBUF_LBUF
	  ? (!bitz || (bitz > 16) ? D3DFMT_R32F          : D3DFMT_R16F         )  // linear
	  : (!bitz || (bitz > 16) ? D3DFMT_R32F          : D3DFMT_R16F         )  // non-linear
      );
    if (RenderBuf & EFFECTBUF_ENRM) if (RenderTransferZ)
      CurrNM.Initialize(
//	    (!bitz || (bitz >  8) ? D3DFMT_A16B16G16R16F : D3DFMT_A8B8G8R8     )  // linear
	    (!bitz || (bitz >  8) ? D3DFMT_A16B16G16R16  : D3DFMT_A8B8G8R8     )  // linear
      );
 }
#endif

  RenderCnd = (RenderCnd & ~EFFECTCOND_HASZBUF) | (OrigDS.IsValid() || CurrDS.IsValid() ? EFFECTCOND_HASZBUF : 0);
  RenderCnd = (RenderCnd & ~EFFECTCOND_HASENRM) | (                    CurrNM.IsValid() ? EFFECTCOND_HASENRM : 0);
#ifndef	OBGE_NOSHADER
  RenderCnd = (RenderCnd & ~EFFECTCOND_HASMIPS) | (AMFilter != D3DTEXF_NONE		? EFFECTCOND_HASMIPS : 0);
#endif

  if (bits > 0) STARGET[0] = RT_SIGNED,   TTARGET[0] = RT_FLOAT;
  if (bits < 0) STARGET[0] = RT_UNSIGNED, TTARGET[0] = RT_INTEGER;
}

void EffectManager::ReleaseFrameTextures() {
#ifdef	OLD_QUEUE
  if (thisframeSurf) {
    _MESSAGE("Releasing thisframe surface.");

    thisframeSurf->Release();
    thisframeSurf = NULL;
  }

  if (thisframeTex) {
    _MESSAGE("Releasing thisframe texture.");

    while (thisframeTex->Release()) {}
    thisframeTex = NULL;
  }

  if (lastpassSurf) {
    _MESSAGE("Releasing lastpass surface.");

    lastpassSurf->Release();
    lastpassSurf = NULL;
  }

  if (lastpassTex) {
    _MESSAGE("Releasing lastpass texture.");

    while (lastpassTex->Release()) {}
    lastpassTex = NULL;
  }

  if (lastframeSurf) {
    _MESSAGE("Releasing lastframe surface.");

    lastframeSurf->Release();
    lastframeSurf = NULL;
  }

  if (lastframeTex) {
    _MESSAGE("Releasing lastframe texture.");

    while (lastframeTex->Release()) {}
    lastframeTex = NULL;
  }
#else
  /* over frames */
  if (RenderBuf & EFFECTBUF_PAST) PastRT.Release();
  if (RenderBuf & EFFECTBUF_PREV) PrevRT.Release();
				  LastRT.Release();

  if (RenderBuf & EFFECTBUF_ZBUF) if (RenderTransferZ)
				  CurrDS.Release(),
				  CurrNM.Release();
#endif
}

void EffectManager::InitializeBuffers() {
  float minx, minu, uadj, vadj;
  void *VertexPointer;

  const float W = (float)v1_2_416::GetRenderer()->SizeWidth;
  const float H = (float)v1_2_416::GetRenderer()->SizeHeight;

  Constants.rcpres[0] = 1.0f / W;
  Constants.rcpres[1] = 1.0f / H;
  Constants.rcpres[2] = W / H;
  Constants.rcpres[3] = W * H;

  uadj = Constants.rcpres[0] * 0.5;
  vadj = Constants.rcpres[1] * 0.5;

  if (SplitScreen.data) {
    minx = 0;
    minu = 0.5;
  }
  else {
    minx = -1;
    minu = 0;
  }

  EffectQuad ShaderVertices[] = {
    {minx, +1, 1, minu + uadj, 0 + vadj, 0},
    {minx, -1, 1, minu + uadj, 1 + vadj, 1},
    {1   , +1, 1, 1    + uadj, 0 + vadj, 2},
    {1   , -1, 1, 1    + uadj, 1 + vadj, 3}
  };

  _MESSAGE("Creating effect vertex buffers.");

  if (lastOBGEDirect3DDevice9->CreateVertexBuffer(4 * sizeof(EffectQuad), D3DUSAGE_WRITEONLY, EFFECTQUADFORMAT, D3DPOOL_DEFAULT, &EffectVertex, 0) != D3D_OK) {
    _MESSAGE("ERROR - Unable to create the vertex buffer!");
    exit(0); return;
  }

  EffectVertex->Lock(0, 0, &VertexPointer, 0);
  CopyMemory(VertexPointer, ShaderVertices, sizeof(ShaderVertices));
  EffectVertex->Unlock();

#ifdef OBGE_CONSTANTPOOLS
  _MESSAGE("Creating effect constants pool.");

  /* utilize effect-pools to set less parameters per frame */
  if (D3DXCreateEffectPool(&EffectPool) != D3D_OK) {
    _MESSAGE("ERROR - Unable to create constants-pool!");
    exit(0); return;
  }

  EffectShare = new EffectRecord();
  if (!EffectShare->LoadEffect("Constants.fx", 0) ||
      !EffectShare->CompileEffect(this)) {
    delete EffectShare;
    EffectShare = NULL;

    _MESSAGE("ERROR - Constants.fx is missing and required! Please reinstall OBGE");
    exit(0); return;
  }
#endif

#ifndef	NO_DEPRECATED
  Constants.bHasDepth = ::HasDepth();
#endif
}

void EffectManager::ReleaseBuffers() {
  if (EffectVertex) {
    _MESSAGE("Releasing effect vertex buffer.");

    while (EffectVertex->Release()) {}
    EffectVertex = NULL;
  }

  if (EffectPool) {
    _MESSAGE("Releasing effect constants pool.");

    while (EffectPool->Release()) {}
    EffectPool = NULL;
  }

  if (EffectShare) {
    delete EffectShare;
    EffectShare = NULL;
  }
}

void EffectManager::OnReleaseDevice() {
  ReleaseBuffers();
  ReleaseFrameTextures();

  /* prevent locking up because other resources have allready been freed */
  ManagedEffectList prevEffects = ManagedEffects; ManagedEffects.clear();
  ManagedEffectList::iterator SEffect = prevEffects.begin();

  while (SEffect != prevEffects.end()) {
    delete (*SEffect);
    SEffect++;
  }

  Reset();

  if (EffectDepth) { delete EffectDepth; EffectDepth = NULL; }
  if (EffectShare) { delete EffectShare; EffectShare = NULL; }
}

void EffectManager::OnLostDevice() {
  ReleaseBuffers();
  ReleaseFrameTextures();

  ManagedEffectList::iterator SEffect = ManagedEffects.begin();

  while (SEffect != ManagedEffects.end()) {
    (*SEffect)->OnLostDevice();
    SEffect++;
  }

  if (EffectDepth) EffectDepth->OnLostDevice();
  if (EffectShare) EffectShare->OnLostDevice();
}

void EffectManager::OnResetDevice() {
  ReleaseBuffers();
  ReleaseFrameTextures();

  InitializeBuffers();
  InitializeFrameTextures();

  ManagedEffectList::iterator SEffect = ManagedEffects.begin();

  while (SEffect != ManagedEffects.end()) {
    (*SEffect)->OnResetDevice();
    (*SEffect)->ApplyPermanents(this);

    SEffect++;
  }

  if (EffectDepth) EffectDepth->OnResetDevice();
  if (EffectShare) EffectShare->OnResetDevice();
}

inline void EffectManager::UpdateFrameConstants(v1_2_416::NiDX9Renderer *Renderer) {
  v1_2_416::NiCamera **pMainCamera = (v1_2_416::NiCamera **)0x00B43124;
  const char *CamName = (*pMainCamera)->m_pcName;
  Renderer->SetCameraViewProj(*pMainCamera);

  /* update matrices after the view/projection has been set */
  Constants.UpdateView      ((const D3DXMATRIX &)Renderer->m44View      );
  Constants.UpdateProjection((const D3DXMATRIX &)Renderer->m44Projection);

  D3DXMatrixTranslation(&Constants.wrld,
			-(*pMainCamera)->m_worldTranslate.x,
			-(*pMainCamera)->m_worldTranslate.y,
			-(*pMainCamera)->m_worldTranslate.z);

  (*pMainCamera)->m_worldRotate.GetForwardVector(&Constants.EyeForward);

  /* products and inverses */
  Constants.UpdateProducts();

  // Sunrise is at 06:00, Sunset at 20:00
  const bool DayTime =
    (Constants.iGameTime.x >= Constants.SunTiming.x) &&
    (Constants.iGameTime.x <= Constants.SunTiming.w);
  const bool SunExists =
    (Constants.Exteriour && !Constants.Oblivion);
  const bool Exteriour =
    (Constants.Exteriour);

#ifdef OBGE_CONSTANTPOOLS
  /* we use pooling, just this has to be set */
  EffectShare->ApplySharedConstants();
#endif

  /* setup flags for effect-filtering */
  RenderCnd = (RenderCnd & ~EFFECTCOND_ISDAY    ) | ( DayTime			      ? EFFECTCOND_ISDAY     : 0);
  RenderCnd = (RenderCnd & ~EFFECTCOND_ISNIGHT  ) | (!DayTime			      ? EFFECTCOND_ISNIGHT   : 0);
  RenderCnd = (RenderCnd & ~EFFECTCOND_HASSUN   ) | ( SunExists			      ? EFFECTCOND_HASSUN    : 0);
  RenderCnd = (RenderCnd & ~EFFECTCOND_INTERIOUR) | (!Exteriour                       ? EFFECTCOND_INTERIOUR : 0);
  RenderCnd = (RenderCnd & ~EFFECTCOND_EXTERIOUR) | ( Exteriour                       ? EFFECTCOND_EXTERIOUR : 0);
#ifndef	OBGE_NOSHADER
  RenderCnd = (RenderCnd & ~EFFECTCOND_HASREFL  ) | (passTexture[OBGEPASS_REFLECTION] ? EFFECTCOND_HASREFL   : 0);
#endif
}

void EffectManager::Render(IDirect3DDevice9 *D3DDevice, IDirect3DSurface9 *RenderTo, IDirect3DSurface9 *RenderFrom) {
  v1_2_416::NiDX9Renderer *Renderer = v1_2_416::GetRenderer();

  markerStop(D3DDevice);

  // Sets up the viewport.
  float test[4] = { 0.0, 1.0, 1.0, 0.0 };
  Renderer->SetupScreenSpaceCamera(test);

  // Set up world/view/proj matrices to identity in case there's no vertex effect.
  D3DXMATRIX mIdent;
  D3DXMatrixIdentity(&mIdent);

  D3DDevice->SetTransform(D3DTS_PROJECTION, &mIdent);
  D3DDevice->SetTransform(D3DTS_VIEW, &mIdent);
  D3DDevice->SetTransform(D3DTS_WORLD, &mIdent);

  UpdateFrameConstants(Renderer);

  Renderer->RenderStateManager->SetRenderState(D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_ALPHA | D3DCOLORWRITEENABLE_BLUE | D3DCOLORWRITEENABLE_GREEN | D3DCOLORWRITEENABLE_RED, false);
  Renderer->RenderStateManager->SetRenderState(D3DRS_ALPHATESTENABLE, false, false);
  Renderer->RenderStateManager->SetRenderState(D3DRS_ALPHABLENDENABLE, false, false);
  Renderer->RenderStateManager->SetRenderState(D3DRS_STENCILENABLE, false, false);
  Renderer->RenderStateManager->SetRenderState(D3DRS_ZENABLE, D3DZB_FALSE, false);
  Renderer->RenderStateManager->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE, false);

  /* is it multi-sampled? this has to be done before everything else
   * as a geometry-call will be done to resolve the depth-target
   */
  bool resz;
  if ((resz = ResolveDepthBuffer(D3DDevice))) {
//  OrigDS.Initialize(GetDepthBufferTexture());
//  FXMan->OrigDS.SetTexture("oblv_CurrDepthStencilZ_MAINPASS", pEffect);
  }

  /* full-screen quad */
  D3DDevice->SetStreamSource(0, EffectVertex, 0, sizeof(EffectQuad));
  Renderer->RenderStateManager->SetFVF(EFFECTQUADFORMAT, false);

  /* current state:
   *
   * - active Rendertarget[0] is RenderTo
   * - the last (internal) effect's output is in RenderFrom
   */

#ifdef	OLD_QUEUE
  if (RAWZflag) {
    D3DDevice->EndScene();

    D3DDevice->SetRenderTarget(0, depthSurface);
    D3DDevice->BeginScene();
    RenderRAWZfix(D3DDevice, RenderTo);
    D3DDevice->EndScene();

    D3DDevice->SetRenderTarget(0, RenderTo);
    D3DDevice->BeginScene();
  }

  D3DDevice->StretchRect(RenderFrom, 0, thisframeSurf, 0, D3DTEXF_NONE);
  D3DDevice->StretchRect(RenderFrom, 0, RenderTo, 0, D3DTEXF_NONE);
  // Blank screen fix when EffectList is empty.

  ManagedEffectList::iterator SEffect = ManagedEffects.begin();

  while (SEffect != ManagedEffects.end()) {
    if ((*SEffect)->IsEnabled()) {
      (*SEffect)->ApplySharedConstants(&EffectConst);
      (*SEffect)->ApplyUniqueConstants();
      (*SEffect)->Render(D3DDevice, RenderTo, lastpassSurf);

      D3DDevice->StretchRect(RenderTo, 0, thisframeSurf, 0, D3DTEXF_NONE);
    }

    SEffect++;
  }

  D3DDevice->StretchRect(RenderTo, 0, lastframeSurf, 0, D3DTEXF_NONE);
#else
  /* rendertarget without texture, this can happen when the
   * color-buffer is multi-sampled
   */
#ifndef	OBGE_NOSHADER
  if ((OrigRT.Initialize(RenderFrom) != D3D_OK))
#endif
  {
    OrigRT.Initialize(RenderFmt);
    OrigRT.Copy(D3DDevice, RenderFrom);
  }

  /* rendertarget without texture, non-HDR & non-Bloom special case
   * this basically is the raw backbuffer I think
   */
  if ((TrgtRT.Initialize(RenderTo) != D3D_OK)) {
    CopyRT.Initialize(RenderFmt);
    RenderBuf |= EFFECTBUF_COPY;
  }

//if (RenderBuf & EFFECTBUF_ZBUF) {
  if (EffectDepth) {
    D3DXHANDLE tec;

    OrigDS.SetTexture("zbufferTexture", EffectDepth->GetEffect());
    CurrDS.SetRenderTarget(D3DDevice);

    if (RenderTransferZ > EFFECTBUF_RAWZ) {
      if (RenderTransferZ & EFFECTBUF_EPOS) {
        /* convert WHATEVER to projected form (CurrDS) */
	tec = EffectDepth->GetEffect()->GetTechniqueByName("projecteye");

	if (RenderTransferZ & EFFECTBUF_ENRM) {
	  EffectDepth->GetEffect()->SetTechnique(tec);
	  EffectDepth->Render(D3DDevice);

	  CurrDS.SetTexture("zbufferTexture", EffectDepth->GetEffect());
	  CurrNM.SetRenderTarget(D3DDevice);

	  /* convert projected positions to normals (OrigDS) */
	  tec = EffectDepth->GetEffect()->GetTechniqueByName("normaleye");
	}
      }
      else
        /* convert WHATEVER to linearized form (CurrDS) */
	tec = EffectDepth->GetEffect()->GetTechniqueByName("linearize");

      if (RenderTransferZ & EFFECTBUF_RAWZ) {
	EffectDepth->GetEffect()->SetTechnique(tec);
	EffectDepth->Render(D3DDevice);

        CurrDS.SetTexture("zbufferTexture", EffectDepth->GetEffect());
        OrigDS.SetRenderTarget(D3DDevice);

	/* convert linearized to unlinearize form (OrigDS) */
        tec = EffectDepth->GetEffect()->GetTechniqueByName("unlinearize");
      }
    }
    else
      /* convert WHATEVER to unlinearize form (CurrDS) */
      tec = EffectDepth->GetEffect()->GetTechniqueByName("copy");

    /* OrigDS always contains "nonlinearized" INTZ */
    /* CurrDS always contains "linearized" or "projected" */
    /* CurrNM always contains "normal" */

    EffectDepth->GetEffect()->SetTechnique(tec);
    EffectDepth->Render(D3DDevice);
  }

  RenderQueue.device = D3DDevice;
  RenderQueue.Init(
    (RenderBuf & EFFECTBUF_PREV) ? &PrevRT : NULL,
				   &LastRT,
    (RenderOpt & EFFECTOPT_STENCIL) ? true : false
  );

  /* over effects */
  RenderQueue.Begin(
				   &OrigRT,
				   &TrgtRT,
    (RenderBuf & EFFECTBUF_COPY) ? &CopyRT : NULL
  );

#if 0 //def	OBGE_STATEBLOCKS
  /* auto backup (not strictly needed, all states can be changed) */
  IDirect3DStateBlock9 *pStateBlock = NULL;
  D3DDevice->CreateStateBlock(D3DSBT_ALL, &pStateBlock);
#endif

  /* count if something happens */
  int run = 0;

  for (ManagedEffectList::iterator e = ManagedEffects.begin(); e != ManagedEffects.end(); e++) {
    /* check if the conditions meet */
    unsigned long EffectCnd = (*e)->GetConditions();
    if ((RenderCnd & EffectCnd) == EffectCnd)
      run += (*e)->Render(D3DDevice, &RenderQueue);
  }

  /* nothing happend */
  if (!run) TrgtRT.Copy(D3DDevice, &OrigRT);

#if 0 //def	OBGE_STATEBLOCKS
  /* auto restore (not strictly needed, all states can be changed) */
  pStateBlock->Apply();
  pStateBlock->Release();
#endif

  RenderQueue.End(
    (RenderBuf & EFFECTBUF_PAST) ? &PastRT : NULL,
				   &TrgtRT
  );
#endif

  markerStart(D3DDevice);
}

inline ManagedEffectQueue *EffectManager::RequestQueue(D3DFORMAT format) {
  ManagedEffectQueue &q = CustomQueues[format];
  if (q.AddRef() == 1) { q.SetCustom(true); } return &q;
}

void EffectManager::ReleaseQueue(D3DFORMAT format) {
  ManagedEffectQueue &q = CustomQueues[format];
  if (q.Release() == 0) { CustomQueues.erase(format); }
}

void EffectManager::ReleaseQueue(ManagedEffectQueue *queue) {
  ManagedQueueList::iterator q;
  for (q = CustomQueues.begin(); q != CustomQueues.end(); q++)
    if (&q->second == queue)
  if (q->second.Release() == 0) { CustomQueues.erase(q); break; }
}

void EffectManager::RenderRAWZfix(IDirect3DDevice9 *D3DDevice, IDirect3DSurface9 *RenderTo) {
#ifdef	OLD_QUEUE
  if (!EffectDepth) {
    EffectDepth = new EffectRecord();

    if (!EffectDepth->LoadEffect("RAWZfix.fx", 0)) {
      _MESSAGE("ERROR - RAWZfix.fx is missing! Please reinstall OBGE");
      exit(0); return;
    }

    EffectDepth->CompileEffect();
    EffectDepth->GetEffect()->SetTexture("RAWZdepth", depthRAWZ);
  }

  EffectDepth->Render(D3DDevice, RenderTo, lastpassSurf);
#endif
}

int EffectManager::AddPrivateEffect(const char *Filename, UINT32 refID) {
  ManagedEffectRecord *NewEffect = new ManagedEffectRecord();

  if (!NewEffect->LoadEffect(Filename, refID, true)) {
    delete NewEffect;
    return -1;
  }

  /* you are allowed to fail */
  NewEffect->CompileEffect(this);

  /* prepare */
  NewEffect->SetPriority(ManagedEffects.size());
  NewEffect->AddRef();

  /* append and sort */
  ManagedEffects.push_back(NewEffect);
  std::sort(ManagedEffects.begin(), ManagedEffects.end(), ManagedEffectRecord::compare);

  /* register */
  Effects[EffectIndex++] = NewEffect;
  assert(Effects.size() == ManagedEffects.size());

  return EffectIndex - 1;
}

int EffectManager::AddManagedEffect(const char *Filename, UINT32 refID) {
  EffectRegistry::iterator SEffect = Effects.begin();

  /* search for a non-private effect */
  while (SEffect != Effects.end()) {
    if (!SEffect->second->IsPrivate()) {
      if (!_stricmp(Filename, SEffect->second->GetName())/*&& ((pEffect->second->ParentRefID & 0xff000000) == (refID & 0xff000000))*/) {
	_MESSAGE("Loading effect \"%s\" that already exists. Returning index \"%d\" of existing effect.", SEffect->second->GetName(), SEffect->first);

	SEffect->second->AddRef();
	return SEffect->first;
      }
    }

    SEffect++;
  }

  ManagedEffectRecord *NewEffect = new ManagedEffectRecord();

  if (!NewEffect->LoadEffect(Filename, refID, false)) {
    delete NewEffect;
    return -1;
  }

  /* you are allowed to fail */
  NewEffect->CompileEffect(this);

  /* prepare */
  NewEffect->SetPriority(ManagedEffects.size());
  NewEffect->AddRef();

  /* append and sort */
  ManagedEffects.push_back(NewEffect);
  std::sort(ManagedEffects.begin(), ManagedEffects.end(), ManagedEffectRecord::compare);

  /* register */
  Effects[EffectIndex++] = NewEffect;
  assert(Effects.size() == ManagedEffects.size());

  return EffectIndex - 1;
}

int EffectManager::AddDependtEffect(const char *Filename, UINT32 refID) {
  EffectRegistry::iterator SEffect = Effects.begin();

  /* search for a any effect */
  while (SEffect != Effects.end()) {
    if (!_stricmp(Filename, SEffect->second->GetName())/*&& ((pEffect->second->ParentRefID & 0xff000000) == (refID & 0xff000000))*/) {
      _MESSAGE("Loading effect \"%s\" that already exists. Returning index \"%d\" of existing effect.", SEffect->second->GetName(), SEffect->first);

      SEffect->second->AddRef();
      return SEffect->first;
    }

    SEffect++;
  }

  ManagedEffectRecord *NewEffect = new ManagedEffectRecord();

  if (!NewEffect->LoadEffect(Filename, refID, true)) {
    delete NewEffect;
    return -1;
  }

  /* you are allowed to fail */
  NewEffect->CompileEffect(this);

  /* prepare */
  NewEffect->SetPriority(ManagedEffects.size());
  NewEffect->AddRef();

  /* append and sort */
  ManagedEffects.push_back(NewEffect);
  std::sort(ManagedEffects.begin(), ManagedEffects.end(), ManagedEffectRecord::compare);

  /* register */
  Effects[EffectIndex++] = NewEffect;
  assert(Effects.size() == ManagedEffects.size());

  return EffectIndex - 1;
}

int EffectManager::FindEffect(const char *Filename) const {
  EffectRegistry::const_iterator SEffect = Effects.begin();

  /* search for a non-private effect */
  while (SEffect != Effects.end()) {
    if (!SEffect->second->IsPrivate()) {
      if (!_stricmp(Filename, SEffect->second->GetName()))
	return SEffect->first;
    }

    SEffect++;
  }

  return -1;
}

bool EffectManager::ReleaseEffect(int EffectNum) {
  if (!IsEffectValid(EffectNum))
    return false;

  ManagedEffectRecord *OldEffect = Effects[EffectNum];

  /* reached zero */
  if (!OldEffect->Release()) {
    /* remove from map */
    Effects.erase(EffectNum);
    /* remove from vector */
    ManagedEffects.erase(std::find(ManagedEffects.begin(), ManagedEffects.end(), OldEffect));

    delete OldEffect;
    return true;
  }

  return false;
}

void EffectManager::Recalculate() {
  /* redo the conditions */
  RenderBuf = 0;
  RenderCnd = 0;
  RenderOpt = 0;

  for (ManagedEffectList::iterator e = ManagedEffects.begin(); e != ManagedEffects.end(); e++) {
    if ((*e)->IsEnabled()) {
      /* integrate */
      RenderBuf |= (*e)->GetParameters();
      RenderCnd |= (*e)->GetConditions();
      RenderOpt |= (*e)->GetOptions();
    }
  }

  SetTransferZ(RenderBuf);

  /* update the buffers */
  InitializeFrameTextures();

  for (ManagedEffectList::iterator e = ManagedEffects.begin(); e != ManagedEffects.end(); e++) {
    if ((*e)->IsEnabled()) {
      /* update */
      (*e)->ApplyPermanents(this);
    }
  }
}

void EffectManager::LoadEffectList() {
  FILE *EffectFile;
  char EffectBuffer[260];
  int lastpos;
  bool enabled;

  if (::UseEffectList.Get()) {
    _MESSAGE("Loading the effects.");

    if (!fopen_s(&EffectFile, ::EffectListFile.Get(), "rt")) {
      while (!feof(EffectFile)) {
        if (fgets(EffectBuffer, 260, EffectFile)) {
          lastpos = strlen(EffectBuffer) - 1;
          enabled = true;

          if (EffectBuffer[lastpos] == 10 ||
	      EffectBuffer[lastpos] == 13)
            EffectBuffer[lastpos] = 0;

          if (EffectBuffer[lastpos - 2] == '=') {
            enabled = (EffectBuffer[lastpos - 1] == '1');
	    EffectBuffer[lastpos - 2] = 0;
          }

	  /* allow comments and sections */
	  if ((EffectBuffer[0] != '\0') &&
	      (EffectBuffer[0] != ';') &&
	      (EffectBuffer[0] != '[')) {
	    /* TODO: don't count this as ref */
	    int EffectNum = AddManagedEffect(EffectBuffer, 0);
	    if (EffectNum != -1) {
	      ManagedEffectRecord *pEffect;

	      pEffect = GetEffect(EffectNum);
	      pEffect->Enable(enabled);
	    //pEffect->ClrRef();
	    }
	  }
        }
      }

      fclose(EffectFile);
    }
    else
      _MESSAGE("Error opening shaderlist.txt file.");
  }
  else
    _MESSAGE("EffectList has been disabled by the INI file.");
}

void EffectManager::NewGame() {
  if (PurgeOnNewGame.Get()) {
    /* prevent locking up because other resources have allready been freed */
    EffectRegistry prevEffects = Effects; Effects.clear();
    EffectRegistry::iterator Effect = prevEffects.begin();

    /* delete terminally */
    while (Effect != prevEffects.end()) {
      delete Effect->second;
      Effect++;
    }

    /* redo everything */
    Reset();
  }
  else {
    EffectRegistry::iterator Effect = Effects.begin();

    /* disable and remove all refs */
    while (Effect != Effects.end()) {
      /* disable, but have them cached */
      Effect->second->Enable(false);
      /* reset to initial state */
      Effect->second->ApplyCompileDirectives(this);
      /* TODO ClrRef clears enabled as well, logical, no? */
      Effect->second->ClrRef();

      /* delete private shaders I guess */
      Effect++;
    }

    /* redo the conditions */
    RenderBuf = 0;
    RenderCnd = 0;
  }
}

void EffectManager::SaveGame(OBSESerializationInterface *Interface) {
  int temp;

  _MESSAGE("EffectManager::SaveGame");

  ManagedEffectList::iterator pEffect = ManagedEffects.begin();

  Interface->WriteRecord('SIDX', SHADERVERSION, &EffectIndex, sizeof(EffectIndex));

  _MESSAGE("Save-game will reference %i effects.", EffectIndex);

  while (pEffect != ManagedEffects.end()) {
    if (!(*pEffect)->IsPrivate()) {
      if ((*pEffect)->GetEffect()) {
	/* this is very bad ... linear search */
	int EffectNum = -1;

	EffectRegistry::iterator IEffect = Effects.begin();
	while (IEffect != Effects.end()) {
	  if (IEffect->second == (*pEffect)) {
	    EffectNum = IEffect->first;
	    break;
	  }

	  IEffect++;
	}

	/* locals */
	const char *Name   = (*pEffect)->GetName();
	const bool Enabled = (*pEffect)->IsEnabled();
	const UINT32 RefID = (*pEffect)->GetRefID();

        Interface->WriteRecord('SNUM', SHADERVERSION, &EffectNum, sizeof(EffectNum));
        Interface->WriteRecord('SPAT', SHADERVERSION, Name, strlen(Name) + 1);
        Interface->WriteRecord('SENB', SHADERVERSION, &Enabled, sizeof(Enabled));
        Interface->WriteRecord('SREF', SHADERVERSION, &RefID, sizeof(RefID));

	/* save variables and non-private textures (those that
	 * have been replaced/changed from the source)
	 */
	(*pEffect)->SaveVars(Interface);

        Interface->WriteRecord('SEOD', SHADERVERSION, &temp, 1);
      }
    }

    pEffect++;
  }

  Interface->WriteRecord('SEOF', SHADERVERSION, &temp, 1);
}

void EffectManager::LoadGame(OBSESerializationInterface *Interface) {
  UInt32 type, version, length;
  int LoadShaderNum, LoadEffectIndex;
  char LoadFilepath[260];
  bool LoadEnabled;
  UInt32 LoadRefID;
  bool InUse;

  Interface->GetNextRecordInfo(&type, &version, &length);

  if (type == 'SIDX') {
    Interface->ReadRecordData(&LoadEffectIndex, length);
    _MESSAGE("Save-game references to max. %i effects.", LoadEffectIndex);
  }
  else {
    _MESSAGE("No effect data in save-game.");
    return;
  }

  Interface->GetNextRecordInfo(&type, &version, &length);

  while (type != 'SEOF') {
    if (type == 'SNUM') {
      Interface->ReadRecordData(&LoadShaderNum, length);
      _MESSAGE("Found SNUM record = %d", LoadShaderNum);
    }
    else {
      _MESSAGE("Error loading game. type!=SNUM");
      return;
    }

    Interface->GetNextRecordInfo(&type, &version, &length);

    if (type == 'SPAT') {
      Interface->ReadRecordData(LoadFilepath, length);
      _MESSAGE("Filename = %s", LoadFilepath);
    }
    else {
      _MESSAGE("Error loading game. type!=SPAT");
      return;
    }

    Interface->GetNextRecordInfo(&type, &version, &length);

    if (type == 'SENB') {
      Interface->ReadRecordData(&LoadEnabled, length);
      _MESSAGE("Enabled = %d", LoadEnabled);
    }
    else {
      _MESSAGE("Error loading game. type!=SENB");
      return;
    }

    Interface->GetNextRecordInfo(&type, &version, &length);

    if (type == 'SREF') {
      Interface->ReadRecordData(&LoadRefID, length);
      _MESSAGE("RefID = 0x%08x", LoadRefID);

      if (LoadRefID == 0) {
        _MESSAGE("NULL refID. Will load effect as I can't resolve it's state.");
        InUse = true;
      }
      else {
        InUse = Interface->ResolveRefID(LoadRefID, &LoadRefID);
        _MESSAGE("Is in use = %d", InUse);
      }
    }
    else {
      _MESSAGE("Error loading game. type!=SREF");
      return;
    }

    /* the only effects we load here are effect which have been
     * added by scripts, the effects from the shader-list may have
     * saved entries, but we ignore them
     */
    int EffectNum = -1;
    if (InUse)
      EffectNum = AddManagedEffect(LoadFilepath, LoadRefID);
    else
      EffectNum = FindEffect(LoadFilepath);

    if (EffectNum != -1) {
      ManagedEffectRecord *NewEffect = GetEffect(EffectNum);
      ID3DXEffect *pEffect = NewEffect->GetEffect();

      Interface->GetNextRecordInfo(&type, &version, &length);

      while (type != 'SEOD') {
	switch (type) {
	    case 'STEX':
	      TextureType TextureData;

	      Interface->ReadRecordData(&TextureData, length);
	      NewEffect->SetEffectSamplerTexture(TextureData.Name, TextureData.tex);
	      _MESSAGE("Texture %s = %d", TextureData.Name, TextureData.tex);
	      break;
	    case 'SINT':
	      IntType IntData;

	      Interface->ReadRecordData(&IntData, length);
	      NewEffect->SetEffectConstantI(IntData.Name, (int *)&IntData.data, IntData.size);
	      _MESSAGE("Int %s = %d(%d)", IntData.Name, IntData.data[0], IntData.size);
	      break;
	    case 'SFLT':
	      FloatType FloatData;

	      Interface->ReadRecordData(&FloatData, length);
	      NewEffect->SetEffectConstantF(FloatData.Name, (float *)&FloatData.data, FloatData.size);
	      _MESSAGE("Float %s = %f(%d)", FloatData.Name, FloatData.data[0], FloatData.size);
	      break;
	}

	Interface->GetNextRecordInfo(&type, &version, &length);
      }

      NewEffect->Enable(LoadEnabled);
      if (LoadShaderNum <= 0)
	LoadShaderNum = EffectIndex;

      /* register */
      if (LoadShaderNum != EffectNum) {
	ManagedEffectRecord *

	/* swap positions */
	OldEffect = Effects[LoadShaderNum];
	Effects[LoadShaderNum] = NewEffect;
	if (OldEffect)
	  Effects[EffectNum] = OldEffect;
	else
	  Effects.erase(EffectNum);

	if (EffectIndex <= LoadShaderNum)
	  EffectIndex = LoadShaderNum + 1;

	assert(Effects.size() == ManagedEffects.size());
      }
    }

    else {
      Interface->GetNextRecordInfo(&type, &version, &length);
      while (type != 'SEOD') {
        Interface->ReadRecordData(LoadFilepath, length);
        Interface->GetNextRecordInfo(&type, &version, &length);
      }
    }

    Interface->GetNextRecordInfo(&type, &version, &length);
  }

  assert(Effects.size() == ManagedEffects.size());
}

bool EffectManager::EnableEffect(int EffectNum, bool State) {
  ManagedEffectRecord *pEffect;

  if ((pEffect = GetEffect(EffectNum))) {
    pEffect->Enable(State); return true; }

  return false;
}

bool EffectManager::GetEffects(int which, std::map<std::string,int> &all) const {
  all.clear();

  ManagedEffectList::const_iterator pEffect = ManagedEffects.begin();

  while (pEffect != ManagedEffects.end()) {
    if (!(*pEffect)->IsPrivate()) {
      if ((*pEffect)->GetEffect()) {
	/**/ if (which < 0) {
	  if ((*pEffect)->IsEnabled()) {
	    pEffect++; continue; }
	}
	else if (which > 0) {
	  if (!(*pEffect)->IsEnabled()) {
	    pEffect++; continue; }
	}

	/* this is very bad ... linear search */
	int EffectNum = -1;

	EffectRegistry::const_iterator IEffect = Effects.begin();
	while (IEffect != Effects.end()) {
	  if (IEffect->second == (*pEffect)) {
	    EffectNum = IEffect->first;
	    break;
	  }

	  IEffect++;
	}

	/* locals */
	std::string name; name.assign((*pEffect)->GetName());

	all[name] = EffectNum;
      }
    }

    pEffect++;
  }

  return true;
}

bool EffectManager::GetEffectConstantHelps(int EffectNum, std::map<std::string,std::string> &all) const {
  const ManagedEffectRecord *pEffect;

  if ((pEffect = GetEffect(EffectNum)))
    return pEffect->GetEffectConstantHelps(all);

  return false;
}

bool EffectManager::GetEffectConstantTypes(int EffectNum, std::map<std::string,int> &all) const {
  const ManagedEffectRecord *pEffect;

  if ((pEffect = GetEffect(EffectNum)))
    return pEffect->GetEffectConstantTypes(all);

  return false;
}

bool EffectManager::GetEffectConstantHelp(int EffectNum, char *name, const char **help) const {
  const ManagedEffectRecord *pEffect;

  if ((pEffect = GetEffect(EffectNum)))
    return pEffect->GetEffectConstantHelp(name, help);

  return false;
}

bool EffectManager::GetEffectConstantType(int EffectNum, char *name, int *type) const {
  const ManagedEffectRecord *pEffect;

  if ((pEffect = GetEffect(EffectNum)))
    return pEffect->GetEffectConstantType(name, type);

  return false;
}

bool EffectManager::GetEffectConstantB(int EffectNum, char *name, BOOL *value) const {
  const ManagedEffectRecord *pEffect;

  if ((pEffect = GetEffect(EffectNum)))
    return pEffect->GetEffectConstantB(name, value);

  return false;
}

bool EffectManager::GetEffectConstantI(int EffectNum, char *name, int *value) const {
  const ManagedEffectRecord *pEffect;

  if ((pEffect = GetEffect(EffectNum)))
    return pEffect->GetEffectConstantI(name, value);

  return false;
}

bool EffectManager::GetEffectConstantF(int EffectNum, char *name, float *value) const {
  const ManagedEffectRecord *pEffect;

  if ((pEffect = GetEffect(EffectNum)))
    return pEffect->GetEffectConstantF(name, value);

  return false;
}

bool EffectManager::GetEffectConstantV(int EffectNum, char *name, float *value) const {
  const ManagedEffectRecord *pEffect;

  if ((pEffect = GetEffect(EffectNum)))
    return pEffect->GetEffectConstantV(name, value);

  return false;
}

bool EffectManager::GetEffectSamplerTexture(int EffectNum, char *name, int *TextureNum) const {
  const ManagedEffectRecord *pEffect;

  if ((pEffect = GetEffect(EffectNum)))
    return pEffect->GetEffectSamplerTexture(name, TextureNum);

  return false;
}

bool EffectManager::SetEffectConstantB(int EffectNum, char *name, BOOL value) {
  ManagedEffectRecord *pEffect;

  if ((pEffect = GetEffect(EffectNum)))
    return pEffect->SetEffectConstantB(name, value);

  return false;
}

bool EffectManager::SetEffectConstantI(int EffectNum, char *name, int value) {
  ManagedEffectRecord *pEffect;

  if ((pEffect = GetEffect(EffectNum)))
    return pEffect->SetEffectConstantI(name, value);

  return false;
}

bool EffectManager::SetEffectConstantF(int EffectNum, char *name, float value) {
  ManagedEffectRecord *pEffect;

  if ((pEffect = GetEffect(EffectNum)))
    return pEffect->SetEffectConstantF(name, value);

  return false;
}

bool EffectManager::SetEffectConstantV(int EffectNum, char *name, v1_2_416::NiVector4 *value) {
  ManagedEffectRecord *pEffect;

  if ((pEffect = GetEffect(EffectNum)))
    return pEffect->SetEffectConstantV(name, value);

  return false;
}

bool EffectManager::SetEffectSamplerTexture(int EffectNum, char *name, int TextureNum) {
  ManagedEffectRecord *pEffect;

  if ((pEffect = GetEffect(EffectNum)))
    return pEffect->SetEffectSamplerTexture(name, TextureNum);

  return false;
}

bool EffectManager::GetEffectState(int EffectNum) const {
  ManagedEffectRecord *pEffect;

  if ((pEffect = GetEffect(EffectNum)))
    return pEffect->IsEnabled();

  return false;
}

void EffectManager::PurgeTexture(IDirect3DBaseTexture9 *texture, int TexNum) {
  ManagedEffectList::iterator pEffect = ManagedEffects.begin();

  while (pEffect != ManagedEffects.end()) {
    if (*pEffect && (*pEffect)->HasEffect())
      (*pEffect)->PurgeTexture(texture, TexNum);

    pEffect++;
  }
}

/* -------------------------------------------------------------------------------------------------
 */

void EffectManager::UseLegacyCompiler(bool yes) {
  ::UseLegacyCompiler.Set(yes);
}

void EffectManager::CompileSources(bool yes) {
  /*::CompileSources.Set(yes)*/;
}

void EffectManager::Optimize(bool yes) {
  ::Optimize.Set(yes);
}

bool EffectManager::UseLegacyCompiler() {
  return ::UseLegacyCompiler.Get();
}

bool EffectManager::CompileSources() {
  return true;
}

bool EffectManager::Optimize() {
  return ::Optimize.Get();
}

const char *EffectManager::EffectDirectory() {
  return ::EffectDirectory.Get();
}

const char *EffectManager::EffectListFile() {
  return ::EffectListFile.Get();
}

bool EffectManager::UseEffectList() {
  return ::UseEffectList.Get();
}
