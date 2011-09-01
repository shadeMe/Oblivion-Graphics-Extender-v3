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

#include <d3d9.h>
#include <d3dx9.h>

#include "D3D9.hpp"
#include "D3D9Device.hpp"

#include "TextureConversions.h"
#define STB_DXT_IMPLEMENTATION
#include "TextureConversions-stb_dxt_104.h"
#include "TextureConversions-squish.h"

#define	fabs(x)	((x) >= 0 ? (x) : -(x))

inline int rint(float n) {
  return max(min((int)floor(n + 0.5f), 255.0f), 0.0f);

  //if (n >= 0.0f)
  //  return max((int)floor(n + 0.5f), 255);
  //if (n <= 0.0f)
  //  return min((int) ceil(n - 0.5f),   0);
}

/* AWSOME:
 * http://aras-p.info/texts/CompactNormalStorage.html
 */

#define	MIPMAP_MINIMUM		1	// 4

/* make normals occupy the full[0,255] color-cube */
#define	NORMALS_CUBESPACE

/* http://www.gamedev.net/topic/539608-mip-mapping-normal-maps/page__whichpage__1%25EF%25BF%25BD */
#define	NORMALS_SCALEBYLEVEL  2

/* different ways of encoding the normals */
#undef	NORMALS_INTEGER
#undef	NORMALS_FLOAT_XYZ
#undef	NORMALS_FLOAT_XYZ_TANGENTSPACE
#undef	NORMALS_FLOAT_XY_TANGENTSPACE
#define	NORMALS_FLOAT_DXDY_TANGENTSPACE	0.5f
/* http://diaryofagraphicsprogrammer.blogspot.com/2009/01/partial-derivative-normal-maps.html
 *
 * The idea is to store the paritial derivate of the normal in two channels of the map like this
 *
 * dx = (-nx/nz);
 * dy = (-ny/nz);
 *
 * Then you can reconstruct the normal like this:
 *
 * nx = -dx;
 * ny = -dy;
 * nz = 1;
 * normalize(n);
 *
 * The advantage is that you do not have to reconstruct Z, so you can skip one instruction in
 * each pixel shader that uses normal maps.
 */
#undef	NORMALS_FLOAT_AZ_TANGENTSPACE
/* http://www.gamedev.net/topic/535230-storing-normals-as-spherical-coordinates/
 *
 * Encode:
 *  return (float2(atan2(nrmNorm.y, nrmNorm.x) / M_PI, nrmNorm.z) + 1.0f) * 0.5f;
 *
 * Decode:
 *  float2 spGPUAngles = spherical.xy * 2.0f - 1.0f;
 *  float2 sincosTheta; sincos(spGPUAngles.x * M_PI, sincosTheta.x, sincosTheta.y);
 *  float2 sincosPhi = float2(sqrt(1.0f - spGPUAngles.y * spGPUAngles.y), spGPUAngles.y);
 *
 * return float3(sincosTheta.y * sincosPhi.x, sincosTheta.x * sincosPhi.x, sincosPhi.y);
 *
 * Storing z instead of acos(z) just saves some ops the decoding, you still need sin(phi) and cos(phi) to reconstruct XY. You just happen to already have cos(acos(z)), and the trig identity: '1 - sin(x)^2 = cos(x)^2' does the rest.
 *
 * Edit:
 *  Didn't answer the question I guess. You are seeing odd results because the conversion back from spherical is missing a step. You are computing sincos(theta) but not sincos(phi). The reason why I say store just normal.z and not acos(normal.z) is because the length of the vector is 1.0f (did I mention this method of encode/decode only works on normalized vectors) and so doing acos(normal.z/1) and then recovering cos(acos(normal.z/1)) is a very silly thing to do. Instead I store normal.z, and then compute sin(phi) by using the law of sines.
 */

#define ACCUMODE_LINEAR		( 0 << 0)	// RGBH
#define ACCUMODE_GAMMA		( 1 << 0)	// RGBH
#define ACCUMODE_FLAT		( 0 << 0)	// XYZD
#define ACCUMODE_SCALE		( 1 << 0)	// XYZD

#define NORMMODE_LINEAR		( 0 << 0)	// RGBH
#define NORMMODE_GAMMA		( 1 << 0)	// RGBH

#define TRGTNORM_CUBESPACE	( 1 << 0)	// XYZD
#define TRGTMODE_CODING		(15 << 1)
#define TRGTMODE_CODING_RGB	( 0 << 1)
#define TRGTMODE_CODING_XYZt	( 1 << 1)
#define TRGTMODE_CODING_XYt	( 2 << 1)
#define TRGTMODE_CODING_DXDYt	( 3 << 1)
#define TRGTMODE_CODING_DXDYDZt ( 4 << 1)
#define TRGTMODE_CODING_AZt	( 5 << 1)
#define TRGTMODE_CODING_XYZ	( 7 << 1)
#define TRGTMODE_CODING_XY	( 8 << 1)

/* ####################################################################################
 */

template<int mode>
static void AccuRGBH(long *bs, ULONG b, int level, int l) {
  /* seperate the channels and build the sum */
  bs[0] += (b >> 24) & 0xFF; /*h*/
  bs[1] += (b >> 16) & 0xFF; /*b*/
  bs[2] += (b >>  8) & 0xFF; /*g*/
  bs[3] += (b >>  0) & 0xFF; /*r*/
}

template<int mode>
static void AccuRGBM(long *bs, ULONG b, int level, int l) {
  /* seperate the channels and build the sum */
  bs[0]  = max(bs[0],
           (b >> 24) & 0xFF); /*h*/
  bs[1] += (b >> 16) & 0xFF; /*b*/
  bs[2] += (b >>  8) & 0xFF; /*g*/
  bs[3] += (b >>  0) & 0xFF; /*r*/
}

template<int mode>
static void AccuRGBH(float *bs, ULONG b, int level, int l) {
  /* seperate the channels and build the sum */
  bs[0] +=             ((b >> 24) & 0xFF)              ; /*h*/
  bs[1] += powf((float)((b >> 16) & 0xFF) / 0xFF, 2.2f); /*b*/
  bs[2] += powf((float)((b >>  8) & 0xFF) / 0xFF, 2.2f); /*g*/
  bs[3] += powf((float)((b >>  0) & 0xFF) / 0xFF, 2.2f); /*r*/
}

template<int mode>
static void AccuRGBM(float *bs, ULONG b, int level, int l) {
  /* seperate the channels and build the sum */
  bs[0]  = max(bs[0],
                       ((b >> 24) & 0xFF)             ); /*h*/
  bs[1] += powf((float)((b >> 16) & 0xFF) / 0xFF, 2.2f); /*b*/
  bs[2] += powf((float)((b >>  8) & 0xFF) / 0xFF, 2.2f); /*g*/
  bs[3] += powf((float)((b >>  0) & 0xFF) / 0xFF, 2.2f); /*r*/
}

template<int mode>
static void AccuXYZD(long *ns, ULONG n, int level, int l) {
  long vec[4];

  vec[0] = ((n >> 24) & 0xFF) - 0x00; /*d[ 0,1]*/
  vec[1] = ((n >> 16) & 0xFF) - 0x80; /*z[-1,1]*/
  vec[2] = ((n >>  8) & 0xFF) - 0x80; /*y[-1,1]*/
  vec[3] = ((n >>  0) & 0xFF) - 0x80; /*x[-1,1]*/

  if (mode & ACCUMODE_SCALE) {
    /* lower z (heighten the virtual displacement) every level */
    vec[1] *= (level * NORMALS_SCALEBYLEVEL) - l;
    vec[1] /= (level * NORMALS_SCALEBYLEVEL);
  }

  ns[0] += vec[0];
  ns[1] += vec[1];
  ns[2] += vec[2];
  ns[3] += vec[3];
}

template<int mode>
static void AccuXYZD(float *nn, ULONG n, int level, int l) {
  float vec[4], len;

  vec[0] = ((n >> 24) & 0xFF);
  vec[1] = ((n >> 16) & 0xFF); vec[1] /= 0xFF; vec[1] -= 0.5f; vec[1] /= 0.5f;
  vec[2] = ((n >>  8) & 0xFF); vec[2] /= 0xFF; vec[2] -= 0.5f; vec[2] /= 0.5f;
  vec[3] = ((n >>  0) & 0xFF); vec[3] /= 0xFF; vec[3] -= 0.5f; vec[3] /= 0.5f;

  if (mode & ACCUMODE_SCALE) {
    /* lower z (heighten the virtual displacement) every level */
    vec[1] *= (level * NORMALS_SCALEBYLEVEL) - l;
    vec[1] /= (level * NORMALS_SCALEBYLEVEL);
  }

  len = sqrt(vec[1] * vec[1] + vec[2] * vec[2] + vec[3] * vec[3]);

  vec[1] /= len;
  vec[2] /= len;
  vec[3] /= len;

  nn[0] += vec[0];
  nn[1] += vec[1];
  nn[2] += vec[2];
  nn[3] += vec[3];
}

template<int mode>
static void AccuXYCD(long *nn, ULONG n, int level, int l) {
  abort();
}

template<int mode>
static void AccuXYCD(float *nn, ULONG n, int level, int l) {
  float vec[5], len;

  vec[0] = ((n >> 24) & 0xFF);
  vec[1] = ((n >> 16) & 0xFF);
  vec[2] = ((n >>  8) & 0xFF); vec[2] /= 0xFF; vec[2] -= 0.5f; vec[2] /= 0.5f;
  vec[3] = ((n >>  0) & 0xFF); vec[3] /= 0xFF; vec[3] -= 0.5f; vec[3] /= 0.5f;
  vec[4] = sqrt(1.0f - min(1.0f, vec[2] * vec[2] + vec[3] * vec[3]));

  if (mode & ACCUMODE_SCALE) {
    /* lower z (heighten the virtual displacement) every level */
    vec[4] *= (level * NORMALS_SCALEBYLEVEL) - l;
    vec[4] /= (level * NORMALS_SCALEBYLEVEL);
  }

  len = sqrt(vec[4] * vec[4] + vec[2] * vec[2] + vec[3] * vec[3]);

  vec[2] /= len;
  vec[3] /= len;
  vec[4] /= len;

  nn[0] += vec[0];
  nn[1] += vec[1];
  nn[2] += vec[2];
  nn[3] += vec[3];
  nn[4] += vec[4];
}

/* ####################################################################################
 */

template<int mode>
static void NormRGBH(long *obs, long *bs, int av) {
  /* build average of each channel an join */
  obs[0] = bs[0] / av; /*h*/
  obs[1] = bs[1] / av; /*b*/
  obs[2] = bs[2] / av; /*g*/
  obs[3] = bs[3] / av; /*r*/
}

template<int mode>
static void NormRGBM(long *obs, long *bs, int av) {
  /* build average of each channel an join */
  obs[0] = bs[0]     ; /*h*/
  obs[1] = bs[1] / av; /*b*/
  obs[2] = bs[2] / av; /*g*/
  obs[3] = bs[3] / av; /*r*/
}

template<int mode>
static void NormRGBH(float *obs, float *bs, int av) {
  /* build average of each channel an join */
  obs[0] =     (bs[0] / av             )       ; /*d[ 0,1]*/
  obs[1] = powf(bs[1] / av, 1.0f / 2.2f) * 0xFF; /*z[-1,1]*/
  obs[2] = powf(bs[2] / av, 1.0f / 2.2f) * 0xFF; /*y[-1,1]*/
  obs[3] = powf(bs[3] / av, 1.0f / 2.2f) * 0xFF; /*x[-1,1]*/
}

template<int mode>
static void NormRGBM(float *obs, float *bs, int av) {
  /* build average of each channel an join */
  obs[0] =      bs[0]                          ; /*h*/
  obs[1] = powf(bs[1] / av, 1.0f / 2.2f) * 0xFF; /*b*/
  obs[2] = powf(bs[2] / av, 1.0f / 2.2f) * 0xFF; /*g*/
  obs[3] = powf(bs[3] / av, 1.0f / 2.2f) * 0xFF; /*r*/
}

template<int mode>
static void NormXYZD(long *ons, long *ns, int av) {
  ons[0] = ns[0] / av; /*d[ 0,1]*/
  ons[1] = ns[1] / av; /*z[-1,1]*/
  ons[2] = ns[2] / av; /*y[-1,1]*/
  ons[3] = ns[3] / av; /*x[-1,1]*/
}

template<int mode>
static void NormXYZD(float *onn, float *nn, int av) {
  float len;

  len = sqrt(nn[1] * nn[1] + nn[2] * nn[2] + nn[3] * nn[3]);

  onn[0] = nn[0] / av;  /*d[ 0,1]*/
  onn[1] = nn[1] / len; /*z[-1,1]*/
  onn[2] = nn[2] / len; /*y[-1,1]*/
  onn[3] = nn[3] / len; /*x[-1,1]*/
}

template<int mode>
static void NormXYCD(long *onn, long *nn, int av) {
  abort();
}

template<int mode>
static void NormXYCD(float *onn, float *nn, int av) {
  float len;

  len = sqrt(nn[4] * nn[4] + nn[2] * nn[2] + nn[3] * nn[3]);

  onn[0] = nn[0] / av;  /*d[ 0,1]*/
  onn[1] = nn[1] / av;  /*c[-1,1]*/
  onn[2] = nn[2] / len; /*y[-1,1]*/
  onn[3] = nn[3] / len; /*x[-1,1]*/
//onn[4] = nn[4] / len; /*z[-1,1]*/
}

/* ####################################################################################
 */

template<int mode>
static void LookRGBH(long *bs, long *br) {
  /* collect magnitudes */
  br[0] = max(bs[0], br[1]); /*h*/
  br[1] = max(bs[1], br[1]); /*b*/
  br[2] = max(bs[2], br[2]); /*g*/
  br[3] = max(bs[3], br[3]); /*r*/
}

template<int mode>
void LookRGBH(float *bs, float *br) {
  /* collect magnitudes */
  br[0] = max(bs[0], br[1]); /*h*/
  br[1] = max(bs[1], br[1]); /*b*/
  br[2] = max(bs[2], br[2]); /*g*/
  br[3] = max(bs[3], br[3]); /*r*/
}

template<int mode>
static void LookXYZD(long *ns, long *nr) {
  /* collect magnitudes */
  nr[1] = max(abs(ns[1]), nr[1]); /*z[-1,1]*/
  nr[2] = max(abs(ns[2]), nr[2]); /*y[-1,1]*/
  nr[3] = max(abs(ns[3]), nr[3]); /*x[-1,1]*/
}

template<int mode>
static void LookXYZD(float *nn, float *nr) {
  if ((mode & TRGTMODE_CODING) == TRGTMODE_CODING_DXDYDZt) {
    /* calculate maximum partial derivative */
    float rel = (
      max(
	fabs(nn[2]),
	fabs(nn[3])
      )
      / fabs(nn[1])
    );

    /* collect magnitudes */
    nr[1] = max(     rel   , nr[1]); /*r[ 0,inf]*/
    nr[2] = max(fabs(nn[2]), nr[2]); /*y[-1,1]*/
    nr[3] = max(fabs(nn[3]), nr[3]); /*x[-1,1]*/
  }
  else if ((mode & TRGTMODE_CODING) != TRGTMODE_CODING_XYZ) {
    /* collect magnitudes */
    nr[1] = max(fabs(nn[1]), nr[1]); /*z[-1,1]*/
    nr[2] = max(fabs(nn[2]), nr[2]); /*y[-1,1]*/
    nr[3] = max(fabs(nn[3]), nr[3]); /*x[-1,1]*/
  }
}

template<int mode>
static void LookXYCD(long *nn, long *nr) {
  abort();
}

template<int mode>
static void LookXYCD(float *nn, float *nr) {
  /* collect magnitudes */
  nr[2] = max(abs(nn[2]), nr[2]); /*y[-1,1]*/
  nr[3] = max(abs(nn[3]), nr[3]); /*x[-1,1]*/
}

/* ####################################################################################
 */

template<int mode>
static ULONG JoinRGBH(long *bs, long *br) {
  ULONG b = 0;

  /* build average of each channel an join */
  b |= (bs[0] << 24); /*h*/
  b |= (bs[1] << 16); /*b*/
  b |= (bs[2] <<  8); /*g*/
  b |= (bs[3] <<  0); /*r*/

  return b;
}

template<int mode>
static ULONG JoinRGBH(float *bs, float *br) {
  ULONG b = 0;

  /* build average of each channel an join */
  b |= (rint(bs[0]) << 24); /*d[ 0,1]*/
  b |= (rint(bs[1]) << 16); /*z[-1,1]*/
  b |= (rint(bs[2]) <<  8); /*y[-1,1]*/
  b |= (rint(bs[3]) <<  0); /*x[-1,1]*/

  return b;
}

template<int mode>
static ULONG JoinXYZD(long *ns, long *nr) {
  ULONG n = 0;

  n |= ((ns[0] + 0x00)) << 24; /*d[ 0,1]*/
  n |= ((ns[1]) << 16) + 0x80; /*z[-1,1]*/
  n |= ((ns[2]) <<  8) + 0x80; /*y[-1,1]*/
  n |= ((ns[3]) <<  0) + 0x80; /*x[-1,1]*/

  return n;
}

template<int mode>
static ULONG JoinXYZD(float *nn, float *nr) {
  ULONG n = 0;
  float vec[4], len;
  float derivb = NORMALS_FLOAT_DXDY_TANGENTSPACE;		// [0.5f,1.0f]

  vec[0] = nn[0];
  vec[1] = nn[1];
  vec[2] = nn[2];
  vec[3] = nn[3];

  /* ################################################################# */
  if (((mode & TRGTMODE_CODING) == TRGTMODE_CODING_DXDYt) ||
      ((mode & TRGTMODE_CODING) == TRGTMODE_CODING_DXDYDZt)) {
    if ((mode & TRGTMODE_CODING) == TRGTMODE_CODING_DXDYDZt) {
      /* get the minimum divider we have to support
       * based on the vectors in the 4x4 block
       *
       * this will select a constant z-multiplier over
       * the full 4x4 block with that we calculate the
       * partial derivative:
       *
       *  x / z * multiplier;
       *  y / z * multiplier;
       *  z / z * multiplier;
       */
      float derivx = 1.0f / nr[1];				// [...,1.0f]
      if (derivb < derivx)
	derivb = derivx;
      if (derivb > 1.0f)
	derivb = 1.0f;
    }

#if 0
    vec[1] =
      max(
	fabs(vec[1]),
      max(
	fabs(vec[2]) * derivb,
	fabs(vec[3]) * derivb
      ));
#else
    float up = derivb *
      max(
	fabs(vec[2]),
	fabs(vec[3])
      )
      / fabs(vec[1]);

    if (up > 1.0f) {
      vec[2] /= up;
      vec[3] /= up;
    }
#endif
  }
  else if ((mode & TRGTMODE_CODING) != TRGTMODE_CODING_XYZ) {
    vec[1] = max(0.0f, vec[1]);
  }

  /* ################################################################# */
  if ((mode & TRGTMODE_CODING) == TRGTMODE_CODING_XYZ) {
    if (mode & TRGTNORM_CUBESPACE)
      len = max(vec[1], max(vec[2], vec[3]));
    else
      len = sqrt(vec[1] * vec[1] + vec[2] * vec[2] + vec[3] * vec[3]);

    vec[1] /= len; vec[1] *= 0.5f; vec[1] += 0.5f; vec[1] *= 0xFF;
    vec[2] /= len; vec[2] *= 0.5f; vec[2] += 0.5f; vec[2] *= 0xFF;
    vec[3] /= len; vec[3] *= 0.5f; vec[3] += 0.5f; vec[3] *= 0xFF;
  }
  /* ################################################################# */
  else if ((mode & TRGTMODE_CODING) == TRGTMODE_CODING_XYZt) {
    if (mode & TRGTNORM_CUBESPACE)
      len = max(vec[1], max(vec[2], vec[3]));
    else
      len = sqrt(vec[1] * vec[1] + vec[2] * vec[2] + vec[3] * vec[3]);

    vec[1] /= len; vec[1] *= 1.0f; vec[1] += 0.0f; vec[1] *= 0xFF;
    vec[2] /= len; vec[2] *= 0.5f; vec[2] += 0.5f; vec[2] *= 0xFF;
    vec[3] /= len; vec[3] *= 0.5f; vec[3] += 0.5f; vec[3] *= 0xFF;
  }
  /* ################################################################# */
  else if ((mode & TRGTMODE_CODING) == TRGTMODE_CODING_XYt) {
    if (mode & TRGTNORM_CUBESPACE) {
      float lnn;

      len = sqrt(vec[1] * vec[1] + vec[2] * vec[2] + vec[3] * vec[3]);
      lnn = sqrt(vec[2] * vec[2] + vec[3] * vec[3]);

      float factor = (2.0f - max(vec[2] / lnn, vec[3] / lnn)) / len;

      vec[1]  = 1.0f;
      vec[2] *= factor; vec[2] *= 0.5f; vec[2] += 0.5f; vec[2] *= 0xFF;
      vec[3] *= factor; vec[3] *= 0.5f; vec[3] += 0.5f; vec[3] *= 0xFF;
    }
    else {
      len = sqrt(vec[1] * vec[1] + vec[2] * vec[2] + vec[3] * vec[3]);

      vec[1]  = 1.0f;
      vec[2] /= len; vec[2] *= 0.5f; vec[2] += 0.5f; vec[2] *= 0xFF;
      vec[3] /= len; vec[3] *= 0.5f; vec[3] += 0.5f; vec[3] *= 0xFF;
    }
  }
  /* ################################################################# */
  else if (((mode & TRGTMODE_CODING) == TRGTMODE_CODING_DXDYt) ||
	   ((mode & TRGTMODE_CODING) == TRGTMODE_CODING_DXDYDZt)) {
    if (mode & TRGTNORM_CUBESPACE) {
      /* this format is a bit special */
      len = vec[1] / derivb;

      float lnn = sqrt(vec[2] * vec[2] + vec[3] * vec[3]);
      float factor = (2.0f - max(vec[2] / lnn, vec[3] / lnn)) / len;

      vec[1] /= len;    vec[1] *= 0.5f; vec[1] += 0.5f; vec[1] *= 0xFF;
      vec[2] *= factor; vec[2] *= 0.5f; vec[2] += 0.5f; vec[2] *= 0xFF;
      vec[3] *= factor; vec[3] *= 0.5f; vec[3] += 0.5f; vec[3] *= 0xFF;
		        derivb *= 0.5f; derivb += 0.5f; derivb *= 0xFF;

#if 0
      if (1) {
	float chk[4], cln, fct;

	chk[2] = ((vec[2] / 0xFF) - 0.5f) / 0.5f;
	chk[3] = ((vec[3] / 0xFF) - 0.5f) / 0.5f;

	cln = sqrt(chk[2] * chk[2] + chk[3] * chk[3]);

	fct = 2.0f - max(chk[2] / cln, chk[3] / cln);

	chk[1]  = 1.0f * derivb;
	chk[2] /= fct;
	chk[3] /= fct;

	cln = sqrt(chk[1] * chk[1] + chk[2] * chk[2] + chk[3] * chk[3]);

	chk[1] /= cln;
	chk[2] /= cln;
	chk[3] /= cln;

	cln = 1;
      }
#endif

      assert(fabs(vec[1] - derivb) < 0.001f);
    }
    else {
      /* this format is fully compatible with the built-in shaders */
      len = vec[1] / derivb;

      vec[1] /= len; vec[1] *= 0.5f; vec[1] += 0.5f; vec[1] *= 0xFF;
      vec[2] /= len; vec[2] *= 0.5f; vec[2] += 0.5f; vec[2] *= 0xFF;
      vec[3] /= len; vec[3] *= 0.5f; vec[3] += 0.5f; vec[3] *= 0xFF;
		     derivb *= 0.5f; derivb += 0.5f; derivb *= 0xFF;

      assert(fabs(vec[1] - derivb) < 0.001f);
    }
  }
  /* ################################################################# */
  else if ((mode & TRGTMODE_CODING) == TRGTMODE_CODING_AZt) {
    float ang;

    len = sqrt(vec[1] * vec[1] + vec[2] * vec[2] + vec[3] * vec[3]);
    ang = atan2(vec[2], vec[3]) / M_PI; vec[2] = ang;

    vec[1] *= 1.0f; vec[1] += 0.0f; vec[1] *= 0xFF;
    vec[2] *= 0.5f; vec[2] += 0.5f; vec[3] *= 0xFF;
    vec[3]  = 1.0f;
  }

  n |= (rint(vec[0]) << 24); /*d[ 0,1]*/
  n |= (rint(vec[1]) << 16); /*z[-1,1]*/
  n |= (rint(vec[2]) <<  8); /*y[-1,1]*/
  n |= (rint(vec[3]) <<  0); /*x[-1,1]*/

  return n;
}

template<int mode>
static ULONG JoinXYCD(long *nn, long *nr) {
  abort(); return 0;
}

template<int mode>
static ULONG JoinXYCD(float *nn, float *nr) {
  ULONG n = 0;
  float vec[5], len;

  vec[0] = nn[0];
  vec[1] = nn[1];
  vec[2] = nn[2];
  vec[3] = nn[3];
  vec[4] = sqrt(1.0f - min(1.0f, vec[2] * vec[2] + vec[3] * vec[3]));
  len = sqrt(vec[4] * vec[4] + vec[2] * vec[2] + vec[3] * vec[3]);

  vec[2] /= len; vec[2] *= 0.5f; vec[2] += 0.5f; vec[2] *= 0xFF;
  vec[3] /= len; vec[3] *= 0.5f; vec[3] += 0.5f; vec[3] *= 0xFF;
  vec[4] /= len; vec[4] *= 0.5f; vec[4] += 0.5f; vec[4] *= 0xFF;

  n |= (rint(vec[0]) << 24); /*d[ 0,1]*/
  n |= (rint(vec[1]) << 16); /*c[-1,1]*/
  n |= (rint(vec[2]) <<  8); /*y[-1,1]*/
  n |= (rint(vec[3]) <<  0); /*x[-1,1]*/

  return n;
}

/* ####################################################################################
 */

static bool TextureConvert(D3DSURFACE_DESC &info, LPDIRECT3DTEXTURE9 *tex) {
  LPDIRECT3DTEXTURE9 replct;
  LPDIRECT3DSURFACE9 stex, srep;
  HRESULT res;

//lastOBGEDirect3DDevice9->CreateTexture(info.Width, info.Height, 0, 0, D3DFMT_A8B8G8R8, D3DPOOL_SYSTEMMEM, &replct, NULL);
//lastOBGEDirect3DDevice9->CreateTexture(info.Width, info.Height, 0, 0, D3DFMT_A8B8G8R8, D3DPOOL_DEFAULT, &replct, NULL);
//lastOBGEDirect3DDevice9->CreateTexture(info.Width, info.Height, 1, 0, D3DFMT_A8B8G8R8, D3DPOOL_DEFAULT, &replct, NULL);
//lastOBGEDirect3DDevice9->CreateTexture(info.Width, info.Height, 1, D3DUSAGE_AUTOGENMIPMAP, D3DFMT_A8B8G8R8, D3DPOOL_DEFAULT, &replct, NULL);
//lastOBGEDirect3DDevice9->CreateTexture(info.Width, info.Height, 1, D3DUSAGE_AUTOGENMIPMAP, D3DFMT_A8B8G8R8, D3DPOOL_DEFAULT, &replct, NULL);
//lastOBGEDirect3DDevice9->CreateTexture(info.Width, info.Height, 1, D3DUSAGE_AUTOGENMIPMAP, D3DFMT_A8B8G8R8, D3DPOOL_SYSTEMMEM, &replct, NULL);
  lastOBGEDirect3DDevice9->CreateTexture(info.Width, info.Height, 0, 0, D3DFMT_A8R8G8B8, D3DPOOL_SYSTEMMEM, &replct, NULL);
  if (!replct)
    return false;

  (*tex)->GetSurfaceLevel(0, &stex);
  replct->GetSurfaceLevel(0, &srep);

  res = D3DXLoadSurfaceFromSurface(srep, NULL, NULL, stex, NULL, NULL, D3DX_FILTER_NONE, 0);

  stex->Release();
  srep->Release();

  if (res == D3D_OK) {
    (*tex)->Release();
    (*tex) = replct;

    return true;
  }
  else {
    replct->Release();
    replct = (*tex);

    return false;
  }
}

/* ####################################################################################
 */

#define D3DFMT_ATI1	((D3DFORMAT)MAKEFOURCC('A', 'T', 'I', '1'))
#define D3DFMT_ATI2	((D3DFORMAT)MAKEFOURCC('A', 'T', 'I', '2'))

#include "GUIs_DebugWindow.hpp"

#define TCOMPRESS_A		0
#define TCOMPRESS_LA		1
#define TCOMPRESS_RGB		2
#define TCOMPRESS_RGBA		3
#define TCOMPRESS_RGBH		TCOMPRESS_RGBA
#define	TCOMPRESS_COLOR(fmt)	(((fmt) >= TCOMPRESS_A) && ((fmt) <= TCOMPRESS_RGBH))

#define TCOMPRESS_xyZ		4
#define TCOMPRESS_XY		5
#define TCOMPRESS_XYz		6
#define TCOMPRESS_XYZ		7
#define TCOMPRESS_XYZD		8
#define	TCOMPRESS_NORMAL(fmt)	(((fmt) >= TCOMPRESS_xyZ) && ((fmt) <= TCOMPRESS_XYZD))

static int TCOMPRESS_CHANNELS(int fmt) {
  switch (fmt) {
    case TCOMPRESS_A		: return 1;
    case TCOMPRESS_LA		: return 2;
    case TCOMPRESS_RGB		: return 3;
    case TCOMPRESS_RGBA		: return 4;
    case TCOMPRESS_XY		: return 2;
    case TCOMPRESS_xyZ		: return 1;
    case TCOMPRESS_XYz		: return 2;
    case TCOMPRESS_XYZ		: return 3;
    case TCOMPRESS_XYZD		: return 4;
  }

  return 0;
}

template<typename UTYPE, typename type, int format>
static bool TextureCompressDXT(LPDIRECT3DTEXTURE9 *tex) {
#ifdef	OBGE_DEVLING
  DebugWindow *dw = DebugWindow::Get();
#endif
  LPDIRECT3DTEXTURE9 text;
  D3DSURFACE_DESC texd, texo;
  D3DLOCKED_RECT texr;
  D3DLOCKED_RECT texs;

  (*tex)->GetLevelDesc(0, &texo);

#if 0
  /* Converts a height map into a normal map. The (x,y,z)
   * components of each normal are mapped to the (r,g,t)
   * channels of the output texture.
   */
  HRESULT D3DXComputeNormalMap(
    __out  LPDIRECT3DTEXTURE9 pTexture,
    __in   LPDIRECT3DTEXTURE9 pSrcTexture,
    __in   const PALETTEENTRY *pSrcPalette,
    __in   DWORD Flags,
    __in   DWORD Channel,
    __in   FLOAT Amplitude
    );
#endif

  /* convert to ARGB8 (TODO: support at least the 16bit formats as well) */
  if ((texo.Format != D3DFMT_A8B8G8R8) && !TextureConvert(texo, tex))
    return false;

  /* the lowest mip-level contains a row or a column of 4x4 blocks
   * we won't generate mip-levels for mips smaller than the BTC-area
   */
  int levels = 1;
  int ww = texo.Width;
  int hh = texo.Height;
  while ((ww > MIPMAP_MINIMUM) && (hh > MIPMAP_MINIMUM)) {
    ww = (ww + 1) >> 1;
    hh = (hh + 1) >> 1;

    levels++;
  }

  /* create the textures */
  int flags = squish::kColourIterativeClusterFit;

  switch (TCOMPRESS_CHANNELS(format)) {
    case 4: flags |= squish::kDxt5; lastOBGEDirect3DDevice9->CreateTexture(texo.Width, texo.Height, levels, 0, D3DFMT_DXT5, D3DPOOL_SYSTEMMEM, &text, NULL); break;
    case 3: flags |= squish::kDxt1; lastOBGEDirect3DDevice9->CreateTexture(texo.Width, texo.Height, levels, 0, D3DFMT_DXT1, D3DPOOL_SYSTEMMEM, &text, NULL); break;
    case 2: flags |= squish::kDxt5; lastOBGEDirect3DDevice9->CreateTexture(texo.Width, texo.Height, levels, 0, D3DFMT_ATI2, D3DPOOL_SYSTEMMEM, &text, NULL); break;
    case 1: flags |= squish::kDxt5; lastOBGEDirect3DDevice9->CreateTexture(texo.Width, texo.Height, levels, 0, D3DFMT_ATI1, D3DPOOL_SYSTEMMEM, &text, NULL); break;
  }

  /**/ if (TCOMPRESS_COLOR(format))
    flags |= squish::kColourMetricPerceptual;
  else if (TCOMPRESS_NORMAL(format))
    flags |= squish::kColourMetricUniform;

  /* damit */
  if (!text) {
    if (text) text->Release();

    return false;
  }

  (*tex)->LockRect(0, &texs, NULL, 0);

  DWORD texl = text->GetLevelCount();
  DWORD level = texl;

  for (int l = 0; l < level; l++) {
    /* square dimension of this surface-level */
    /* square area of this surface-level */
    int lv = (1 << l);
    int av = lv * lv;

    text->GetLevelDesc(l, &texd);
    text->LockRect(l, &texr, NULL, 0);

    ULONG *sTex = (ULONG *)texs.pBits;
    ULONG *dTex = (ULONG *)texr.pBits;

    /* loop over 4x4-blocks of this level (DXT5) */
    for (int y = 0; y < texd.Height; y += 4) {
#ifdef	OBGE_DEVLING
      if (dw) dw->SetProgress(l, level, y, texd.Height);
#endif

    for (int x = 0; x < texd.Width; x += 4) {
      UTYPE bTex[2][4*4];
      type  fTex[2][4*4][4];

      /* generate this level's 4x4-block from the original surface */
      for (int ly = 0; ly < 4; ly += 1)
      for (int lx = 0; lx < 4; lx += 1) {
	int yl = ((y + ly) << l);
	int xl = ((x + lx) << l);

	type ts[8] = {0};

	/* access all pixels this level's 4x4-block represents in
	 * the full dimension original surface (high quality mip-mapping)
	 */
	for (int oy = 0; oy < lv; oy += 1)
	for (int ox = 0; ox < lv; ox += 1) {
	  /* assume tiling: wrap pixels around */
	  int posx = (xl + ox) % texo.Width;
	  int posy = (yl + oy) % texo.Height;

	  ULONG t = sTex[(posy * texo.Width) + posx];

	  /* read ARGB -> ABGR */
	  t = //t;
	      (((t >> 24) & 0xFF) << 24 /*h*/)
	    | (((t >> 16) & 0xFF) <<  0 /*r*/)
	    | (((t >>  8) & 0xFF) <<  8 /*g*/)
	    | (((t >>  0) & 0xFF) << 16 /*t*/);

	  /**/ if (TCOMPRESS_COLOR (format))
	    AccuRGBH<ACCUMODE_LINEAR>(ts, t, level, l);
	  else if (TCOMPRESS_XY  == format)
	    AccuXYCD<ACCUMODE_SCALE >(ts, t, level, l);
	  else if (TCOMPRESS_NORMAL(format))
	    AccuXYZD<ACCUMODE_SCALE >(ts, t, level, l);
	}

	/* build average of each channel */
	/**/ if (TCOMPRESS_COLOR (format))
	  NormRGBH<TRGTMODE_CODING_RGB    >(fTex[0][(ly * 4) + lx], ts, av);
	else if (TCOMPRESS_XY  == format)
	  NormXYCD<TRGTMODE_CODING_XY     >(fTex[0][(ly * 4) + lx], ts, av);
	else if (TCOMPRESS_xyZ == format || TCOMPRESS_XYz == format)
	  NormXYZD<TRGTMODE_CODING_XYZ    >(fTex[0][(ly * 4) + lx], ts, av);
	else if (TCOMPRESS_NORMAL(format))
	  NormXYZD<TRGTMODE_CODING_DXDYDZt>(fTex[0][(ly * 4) + lx], ts, av);
      }

      type tr[4] = {0};

      /* analyze this level's 4x4-block */
      for (int ly = 0; ly < 4; ly += 1)
      for (int lx = 0; lx < 4; lx += 1) {
	/**/ if (TCOMPRESS_COLOR (format))
	  LookRGBH<TRGTMODE_CODING_RGB    >(fTex[0][(ly * 4) + lx], tr);
	else if (TCOMPRESS_XY  == format)
	  LookXYCD<TRGTMODE_CODING_XY     >(fTex[0][(ly * 4) + lx], tr);
	else if (TCOMPRESS_xyZ == format || TCOMPRESS_XYz == format)
	  LookXYZD<TRGTMODE_CODING_XYZ    >(fTex[0][(ly * 4) + lx], tr);
	else if (TCOMPRESS_NORMAL(format))
	  LookXYZD<TRGTMODE_CODING_DXDYDZt>(fTex[0][(ly * 4) + lx], tr);
      }

      /* generate this level's 4x4-block from the original surface */
      for (int ly = 0; ly < 4; ly += 1)
      for (int lx = 0; lx < 4; lx += 1) {
	/* build average of each channel an join */
	ULONG t;

	/**/ if (TCOMPRESS_COLOR (format))
	  t = JoinRGBH<TRGTMODE_CODING_RGB    >(fTex[0][(ly * 4) + lx], tr);
	else if (TCOMPRESS_XY  == format)
	  t = JoinXYCD<TRGTMODE_CODING_XY     >(fTex[0][(ly * 4) + lx], tr);
	else if (TCOMPRESS_xyZ == format || TCOMPRESS_XYz == format)
	  t = JoinXYZD<TRGTMODE_CODING_XYZ    >(fTex[0][(ly * 4) + lx], tr);
	else if (TCOMPRESS_NORMAL(format))
	  t = JoinXYZD<TRGTMODE_CODING_DXDYDZt>(fTex[0][(ly * 4) + lx], tr);

	/* write the result ABGR, BGR */
        switch (TCOMPRESS_CHANNELS(format)) {
          /* ABGR -> RGBA */
          case 4: bTex[0][(ly * 4) + lx] = (t); break;
          /* -BGR -> RGB- */
	  case 3: bTex[0][(ly * 4) + lx] = (t) | 0xFF000000; break;
	  /* --YX -> XY-- */
	  /* AL-- -> LA-- */
          case 2: /**/ if (format == TCOMPRESS_LA) bTex[0][(ly * 4) + lx] = (t <<  0) & 0xFF000000,
						   bTex[1][(ly * 4) + lx] = (t <<  8) & 0xFF000000;
          	  else                             bTex[0][(ly * 4) + lx] = (t << 16) & 0xFF000000,
						   bTex[1][(ly * 4) + lx] = (t << 24) & 0xFF000000;
		  break;
          /* -Z-- -> Z--- */
          /* A--- -> A--- */
          case 1: /**/ if (format == TCOMPRESS_A  ) bTex[0][(ly * 4) + lx] = (t <<  0) & 0xFF000000;
          	  else if (format == TCOMPRESS_xyZ) bTex[0][(ly * 4) + lx] = (t <<  8) & 0xFF000000;
          	  else                              bTex[0][(ly * 4) + lx] = (t << 24) & 0xFF000000;
          	  break;
        }
      }

      /* compress to DXT5 */
#if 0
      /**/ if (TCOMPRESS_COLOR(format))
	stb_compress_dxt_block((unsigned char *)dTex, (unsigned char *)bTex[0], true, STB_DXT_DITHER | STB_DXT_HIGHQUAL);
      else if (TCOMPRESS_NORMAL(format))
	stb_compress_dxt_block((unsigned char *)dTex, (unsigned char *)bTex[0], true, STB_DXT_NORMAL | STB_DXT_HIGHQUAL);
#else
      switch (TCOMPRESS_CHANNELS(format)) {
        case 4: squish::Compress         ((unsigned char *)bTex[0], dTex + 0, flags); break;
        case 3: squish::Compress         ((unsigned char *)bTex[0], dTex + 0, flags); break;
        case 2: squish::CompressAlphaDxt5((unsigned char *)bTex[0], 0xFFFF, dTex + 0);
        	squish::CompressAlphaDxt5((unsigned char *)bTex[1], 0xFFFF, dTex + 2); break;
        case 1: squish::CompressAlphaDxt5((unsigned char *)bTex[0], 0xFFFF, dTex + 0); break;
      }
#endif

      /* advance pointer of compressed blocks */
      switch (TCOMPRESS_CHANNELS(format)) {
        case 4: dTex += ((8+8) / 4); break; /* 4x4x4 -> 16bytes */
        case 3: dTex += (( 8 ) / 4); break; /* 4x4x3 ->  8bytes */
        case 2: dTex += ((4+4) / 2); break; /* 4x4x2 -> 16bytes */
        case 1: dTex += (( 4 ) / 2); break; /* 4x4x1 ->  8bytes */
      }

#if 0
      for (int ly = 0; ly < 4; ly += 1)
      for (int lx = 0; lx < 4; lx += 1) {
	dTex[((y + ly) * texd.Width) + (x + lx)] = bTex[0][(ly * 4) + lx];
      }
#endif
    }
    }

    text->UnlockRect(l);
  }

  (*tex)->UnlockRect(0);
  (*tex)->Release();
  (*tex) = text;

  return true;
}

bool TextureCompressRGBH(LPDIRECT3DTEXTURE9 *base, bool gamma) {
  bool res = true;

  if (gamma) res = res && TextureCompressDXT<ULONG, float, TCOMPRESS_RGBH>(base);
  else       res = res && TextureCompressDXT<ULONG, long , TCOMPRESS_RGBH>(base);

  return res;
}

bool TextureCompressRGB(LPDIRECT3DTEXTURE9 *base, bool gamma) {
  bool res = true;

  if (gamma) res = res && TextureCompressDXT<ULONG, float, TCOMPRESS_RGB>(base);
  else       res = res && TextureCompressDXT<ULONG, long , TCOMPRESS_RGB>(base);

  return res;
}

bool TextureCompressLA(LPDIRECT3DTEXTURE9 *base) {
  bool res = true;

  res = res && TextureCompressDXT<ULONG, long , TCOMPRESS_LA>(base);

  return res;
}

bool TextureCompressA(LPDIRECT3DTEXTURE9 *base) {
  bool res = true;

  res = res && TextureCompressDXT<ULONG, long , TCOMPRESS_A>(base);

  return res;
}

bool TextureCompressXYZD(LPDIRECT3DTEXTURE9 *norm) {
  bool res = true;

  res = res && TextureCompressDXT<ULONG, float, TCOMPRESS_XYZD>(norm);

  return res;
}

bool TextureCompressXY_Z(LPDIRECT3DTEXTURE9 *norm, LPDIRECT3DTEXTURE9 *z) {
  bool res = true;

  /* TODO: not really fast */
  (*z = *norm)->AddRef();

  res = res && TextureCompressDXT<ULONG, float, TCOMPRESS_XYz>(norm);
  res = res && TextureCompressDXT<ULONG, float, TCOMPRESS_xyZ>(z);

  return res;
}

bool TextureCompressXYZ(LPDIRECT3DTEXTURE9 *norm) {
  bool res = true;

  res = res && TextureCompressDXT<ULONG, float, TCOMPRESS_XYZ>(norm);

  return res;
}

bool TextureCompressXY(LPDIRECT3DTEXTURE9 *norm) {
  bool res = true;

  res = res && TextureCompressDXT<ULONG, float, TCOMPRESS_XY>(norm);

  return res;
}

bool TextureCompressPM(LPDIRECT3DTEXTURE9 *base, LPDIRECT3DTEXTURE9 *norm) {
  bool res = true;

  res = res && TextureCompressDXT<ULONG, long , TCOMPRESS_RGBH>(base);
  res = res && TextureCompressDXT<ULONG, float, TCOMPRESS_XYZD>(norm);

  return res;
}

template<typename UTYPE, typename type, bool LODed>
static bool TextureCompressQDM(LPDIRECT3DTEXTURE9 *base, LPDIRECT3DTEXTURE9 *norm) {
#ifdef	OBGE_DEVLING
  DebugWindow *dw = DebugWindow::Get();
#endif
  LPDIRECT3DTEXTURE9 baset;
  LPDIRECT3DTEXTURE9 normt;
  D3DSURFACE_DESC based, baseo;
  D3DSURFACE_DESC normd, normo;
  D3DLOCKED_RECT baser;
  D3DLOCKED_RECT normr;
  D3DLOCKED_RECT bases;
  D3DLOCKED_RECT norms;

  (*base)->GetLevelDesc(0, &baseo);
  (*norm)->GetLevelDesc(0, &normo);

#if 0
  /* Converts a height map into a normal map. The (x,y,z)
   * components of each normal are mapped to the (r,g,b)
   * channels of the output texture.
   */
  HRESULT D3DXComputeNormalMap(
    __out  LPDIRECT3DTEXTURE9 pTexture,
    __in   LPDIRECT3DTEXTURE9 pSrcTexture,
    __in   const PALETTEENTRY *pSrcPalette,
    __in   DWORD Flags,
    __in   DWORD Channel,
    __in   FLOAT Amplitude
    );
#endif

  /* they have to have the same dimension */
  if ((baseo.Width  != normo.Width ) ||
      (baseo.Height != normo.Height))
    return false;

  /* convert to ARGB8 (TODO: support at least the 16bit formats as well) */
  if ((baseo.Format != D3DFMT_A8B8G8R8) && !TextureConvert(baseo, base))
    return false;
  if ((normo.Format != D3DFMT_A8B8G8R8) && !TextureConvert(normo, norm))
    return false;

  /* the lowest mip-level contains a row or a column of 4x4 blocks
   * we won't generate mip-levels for mips smaller than the BTC-area
   */
  int levels = 1;
  int ww = baseo.Width;
  int hh = baseo.Height;
  while ((ww > MIPMAP_MINIMUM) && (hh > MIPMAP_MINIMUM)) {
    ww = (ww + 1) >> 1;
    hh = (hh + 1) >> 1;

    levels++;
  }

  /* create the textures */
  int flags = squish::kColourIterativeClusterFit | squish::kDxt5;

  /* create the textures */
  lastOBGEDirect3DDevice9->CreateTexture(baseo.Width, baseo.Height, levels, 0, D3DFMT_DXT5, D3DPOOL_SYSTEMMEM, &baset, NULL);
  lastOBGEDirect3DDevice9->CreateTexture(normo.Width, normo.Height, levels, 0, D3DFMT_DXT5, D3DPOOL_SYSTEMMEM, &normt, NULL);

  /* damit */
  if (!baset || !normt) {
    if (baset) baset->Release();
    if (normt) normt->Release();

    return false;
  }

  (*base)->LockRect(0, &bases, NULL, 0);
  (*norm)->LockRect(0, &norms, NULL, 0);

  DWORD basel = baset->GetLevelCount();
  DWORD norml = normt->GetLevelCount();
  DWORD level = max(basel, norml);

  for (int l = 0; l < level; l++) {
    /* square dimension of this surface-level */
    /* square area of this surface-level */
    int lv = (1 << l);
    int av = lv * lv;

    baset->GetLevelDesc(l, &based);
    normt->GetLevelDesc(l, &normd);

    baset->LockRect(l, &baser, NULL, 0);
    normt->LockRect(l, &normr, NULL, 0);

    ULONG *sBase = (ULONG *)bases.pBits;
    ULONG *sNorm = (ULONG *)norms.pBits;
    ULONG *dBase = (ULONG *)baser.pBits;
    ULONG *dNorm = (ULONG *)normr.pBits;

    /* loop over 4x4-blocks of this level (DXT5) */
    for (int y = 0; y < based.Height; y += 4) {
#ifdef	OBGE_DEVLING
      if (dw) dw->SetProgress(l, level, y, based.Height);
#endif

    for (int x = 0; x < based.Width; x += 4) {
      UTYPE bBase[2][4*4];
      ULONG bNorm[2][4*4];
      type  fBase[2][4*4][4];
      float fNorm[2][4*4][4];

      /* generate this level's 4x4-block from the original surface */
      for (int ly = 0; ly < 4; ly += 1)
      for (int lx = 0; lx < 4; lx += 1) {
	type  bs[8] = {0}; int yl = ((y + ly) << l);
	long  ns[8] = {0}; int xl = ((x + lx) << l);
	float nn[8] = {0.0f};

	/* access all pixels this level's 4x4-block represents in
	 * the full dimension original surface (high quality mip-mapping)
	 */
	for (int oy = 0; oy < lv; oy += 1)
	for (int ox = 0; ox < lv; ox += 1) {
	  /* assume tiling: wrap pixels around */
	  int posx = (xl + ox) % baseo.Width;
	  int posy = (yl + oy) % baseo.Height;

	  ULONG b = sBase[(posy * baseo.Width) + posx];
	  ULONG n = sNorm[(posy * baseo.Width) + posx];

	  /* transfer heightmap into the normal-map (overwrite) */
	  if (LODed)
	    n = (n & 0x00FFFFFF) | (b & 0xFF000000);

	  /* read ARGB -> ABGR */
	  b = //b;
	      (((b >> 24) & 0xFF) << 24 /*h*/)
	    | (((b >> 16) & 0xFF) <<  0 /*r*/)
	    | (((b >>  8) & 0xFF) <<  8 /*g*/)
	    | (((b >>  0) & 0xFF) << 16 /*b*/);

	  n = //n;
	      (((n >> 24) & 0xFF) << 24 /*d*/)
	    | (((n >> 16) & 0xFF) <<  0 /*x*/)
	    | (((n >>  8) & 0xFF) <<  8 /*y*/)
	    | (((n >>  0) & 0xFF) << 16 /*z*/);

	  AccuRGBM<ACCUMODE_LINEAR>(bs, b, level, l);
#if	defined(NORMALS_INTEGER)
	  AccuXYZD<ACCUMODE_SCALE >(ns, n, level, l);
#else
	  AccuXYZD<ACCUMODE_SCALE >(nn, n, level, l);
#endif
	}

	/* build average of each channel */
	NormRGBM<TRGTMODE_CODING_RGB                         >(fBase[0][(ly * 4) + lx], bs, av);
#if	defined(NORMALS_INTEGER)
	NormXYZD<TRGTMODE_CODING_DXDYDZt | TRGTNORM_CUBESPACE>(fNorm[0][(ly * 4) + lx], ns, av);
#else
	NormXYZD<TRGTMODE_CODING_DXDYDZt | TRGTNORM_CUBESPACE>(fNorm[0][(ly * 4) + lx], nn, av);
#endif
      }

      type  br[4] = {0};
      long  nr[4] = {0};
      float rn[4] = {0.0f};

      /* analyze this level's 4x4-block */
      for (int ly = 0; ly < 4; ly += 1)
      for (int lx = 0; lx < 4; lx += 1) {
	LookRGBH<TRGTMODE_CODING_RGB                         >(fBase[0][(ly * 4) + lx], br);
#if	defined(NORMALS_INTEGER)
	LookXYZD<TRGTMODE_CODING_DXDYDZt | TRGTNORM_CUBESPACE>(fNorm[0][(ly * 4) + lx], nr);
#else
	LookXYZD<TRGTMODE_CODING_DXDYDZt | TRGTNORM_CUBESPACE>(fNorm[0][(ly * 4) + lx], rn);
#endif
      }

      /* generate this level's 4x4-block from the original surface */
      for (int ly = 0; ly < 4; ly += 1)
      for (int lx = 0; lx < 4; lx += 1) {
	/* build average of each channel an join */
	UTYPE b;
	ULONG n;

	b = JoinRGBH<TRGTMODE_CODING_RGB                         >(fBase[0][(ly * 4) + lx], br);
#if	defined(NORMALS_INTEGER)
	n = JoinXYZD<TRGTMODE_CODING_DXDYDZt | TRGTNORM_CUBESPACE>(fNorm[0][(ly * 4) + lx], nr);
#else
	n = JoinXYZD<TRGTMODE_CODING_DXDYDZt | TRGTNORM_CUBESPACE>(fNorm[0][(ly * 4) + lx], rn);
#endif

	/* write the result ABGR */
	bBase[0][(ly * 4) + lx] = b;
	bNorm[0][(ly * 4) + lx] = n;
      }

      /* compress to DXT5 */
#if 0
      stb_compress_dxt_block((unsigned char *)dBase, (unsigned char *)bBase[0], true, STB_DXT_DITHER | STB_DXT_HIGHQUAL);
      stb_compress_dxt_block((unsigned char *)dNorm, (unsigned char *)bNorm[0], true, STB_DXT_NORMAL | STB_DXT_HIGHQUAL);
#else
      squish::Compress((unsigned char *)bBase[0], dBase, flags | squish::kColourMetricPerceptual);
      squish::Compress((unsigned char *)bNorm[0], dNorm, flags | squish::kColourMetricUniform);
#endif

      /* advance pointer of compressed blocks */
      dBase += (128 / 32);
      dNorm += (128 / 32);

#if 0
      for (int ly = 0; ly < 4; ly += 1)
      for (int lx = 0; lx < 4; lx += 1) {
	dBase[((y + ly) * based.Width) + (x + lx)] = bBase[0][(ly * 4) + lx];
	dNorm[((y + ly) * based.Width) + (x + lx)] = bNorm[0][(ly * 4) + lx];
      }
#endif
    }
    }

    baset->UnlockRect(l);
    normt->UnlockRect(l);
  }

  (*base)->UnlockRect(0);
  (*norm)->UnlockRect(0);

  (*base)->Release();
  (*norm)->Release();

  (*base) = baset;
  (*norm) = normt;

  return true;
}

bool TextureCompressQDM(LPDIRECT3DTEXTURE9 *base, LPDIRECT3DTEXTURE9 *norm, bool gamma, bool LODed) {
  bool res = true;

  if (gamma) if (LODed) res = res && TextureCompressQDM<ULONG, float, true >(base, norm);
             else       res = res && TextureCompressQDM<ULONG, float, false>(base, norm);
  else       if (LODed) res = res && TextureCompressQDM<ULONG, long , true >(base, norm);
             else       res = res && TextureCompressQDM<ULONG, long , false>(base, norm);

  return res;
}

#endif
