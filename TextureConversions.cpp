
#include <d3d9.h>
#include <d3dx9.h>

#include "D3D9.hpp"
#include "D3D9Device.hpp"

#include "TextureConversions.h"
#define STB_DXT_IMPLEMENTATION
#include "TextureConversions-stb_dxt_104.h"
#include "TextureConversions-squish.h"

inline int rint(float n) {
  return max(min((int)floor(n + 0.5), 255), 0);

  //if (n >= 0.0)
  //  return max((int)floor(n + 0.5), 255);
  //if (n <= 0.0)
  //  return min((int) ceil(n - 0.5),   0);
}

/* AWSOME:
 * http://aras-p.info/texts/CompactNormalStorage.html
 */

/* make normals occupy the full[0,255] color-cube */
#define	NORMALS_CUBESPACE

/* http://www.gamedev.net/topic/539608-mip-mapping-normal-maps/page__whichpage__1%25EF%25BF%25BD */
#define	NORMALS_SCALEBYLEVEL  2

/* different ways of encoding the normals */
#undef	NORMALS_INTEGER
#undef	NORMALS_FLOAT_XYZ
#undef	NORMALS_FLOAT_XYZ_TANGENTSPACE
#undef	NORMALS_FLOAT_XY_TANGENTSPACE
#define	NORMALS_FLOAT_DXDY_TANGENTSPACE	0.5
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
 *  return (float2(atan2(nrmNorm.y, nrmNorm.x) / M_PI, nrmNorm.z) + 1.0) * 0.5;
 *
 * Decode:
 *  float2 spGPUAngles = spherical.xy * 2.0 - 1.0;
 *  float2 sincosTheta; sincos(spGPUAngles.x * M_PI, sincosTheta.x, sincosTheta.y);
 *  float2 sincosPhi = float2(sqrt(1.0 - spGPUAngles.y * spGPUAngles.y), spGPUAngles.y);
 *
 * return float3(sincosTheta.y * sincosPhi.x, sincosTheta.x * sincosPhi.x, sincosPhi.y);
 *
 * Storing z instead of acos(z) just saves some ops the decoding, you still need sin(phi) and cos(phi) to reconstruct XY. You just happen to already have cos(acos(z)), and the trig identity: '1 - sin(x)^2 = cos(x)^2' does the rest.
 *
 * Edit:
 *  Didn't answer the question I guess. You are seeing odd results because the conversion back from spherical is missing a step. You are computing sincos(theta) but not sincos(phi). The reason why I say store just normal.z and not acos(normal.z) is because the length of the vector is 1.0 (did I mention this method of encode/decode only works on normalized vectors) and so doing acos(normal.z/1) and then recovering cos(acos(normal.z/1)) is a very silly thing to do. Instead I store normal.z, and then compute sin(phi) by using the law of sines.
 */

#define ACCUMODE_LINEAR		(0 << 0)
#define ACCUMODE_SCALE		(1 << 0)
#define NORMMODE_CUBESPACE	(1 << 0)

#define TRGTMODE_CODING		(7 << 1)
#define TRGTMODE_CODING_RGB	(0 << 1)
#define TRGTMODE_CODING_XYZt	(1 << 1)
#define TRGTMODE_CODING_XYt	(2 << 1)
#define TRGTMODE_CODING_DXDYt	(3 << 1)
#define TRGTMODE_CODING_DXDYDZt (4 << 1)
#define TRGTMODE_CODING_AZt	(5 << 1)
#define TRGTMODE_CODING_XYZ	(7 << 1)

/* ####################################################################################
 */

template<int mode>
void AccuRGBH(long *bs, long *br, ULONG b, int level, int l) {
  /* seperate the channels and build the sum */
  bs[0] += (b >> 24) & 0xFF; /*h*/
  bs[1] += (b >> 16) & 0xFF; /*b*/
  bs[2] += (b >>  8) & 0xFF; /*g*/
  bs[3] += (b >>  0) & 0xFF; /*r*/

  /* collect magnitudes */
  br[0] = max(bs[0], br[1]); /*h*/
  br[1] = max(bs[1], br[1]); /*b*/
  br[2] = max(bs[2], br[2]); /*g*/
  br[3] = max(bs[3], br[3]); /*r*/
}

template<int mode>
void AccuRGBH(float *bs, float *br, ULONG b, int level, int l) {
  /* seperate the channels and build the sum */
  bs[0] += (b >> 24) & 0xFF; /*h*/
  bs[1] += (b >> 16) & 0xFF; /*b*/
  bs[2] += (b >>  8) & 0xFF; /*g*/
  bs[3] += (b >>  0) & 0xFF; /*r*/

  /* collect magnitudes */
  br[0] = max(bs[0], br[1]); /*h*/
  br[1] = max(bs[1], br[1]); /*b*/
  br[2] = max(bs[2], br[2]); /*g*/
  br[3] = max(bs[3], br[3]); /*r*/
}

template<int mode>
void AccuXYZD(long *ns, long *nr, ULONG n, int level, int l) {
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

  /* collect magnitudes */
  nr[1] = max(abs(vec[1]), nr[1]); /*z[-1,1]*/
  nr[2] = max(abs(vec[2]), nr[2]); /*y[-1,1]*/
  nr[3] = max(abs(vec[3]), nr[3]); /*x[-1,1]*/

  ns[0] += vec[0];
  ns[1] += vec[1];
  ns[2] += vec[2];
  ns[3] += vec[3];
}

template<int mode>
void AccuXYZD(float *nn, float *nr, ULONG n, int level, int l) {
  float vec[4], len;

  vec[0] = ((n >> 24) & 0xFF);
  vec[1] = ((n >> 16) & 0xFF); vec[1] /= 0xFF; vec[1] -= 0.5; vec[1] /= 0.5;
  vec[2] = ((n >>  8) & 0xFF); vec[2] /= 0xFF; vec[2] -= 0.5; vec[2] /= 0.5;
  vec[3] = ((n >>  0) & 0xFF); vec[3] /= 0xFF; vec[3] -= 0.5; vec[3] /= 0.5;

  if (mode & ACCUMODE_SCALE) {
    /* lower z (heighten the virtual displacement) every level */
    vec[1] *= (level * NORMALS_SCALEBYLEVEL) - l;
    vec[1] /= (level * NORMALS_SCALEBYLEVEL);
  }

  if ((mode & TRGTMODE_CODING) == TRGTMODE_CODING_DXDYDZt) {
    /* calculate maximum partial derivative */
    float rel = (
      max(
	fabs(vec[2]),
	fabs(vec[3])
      )
      / fabs(vec[1])
    );

    /* collect magnitudes */
    nr[1] = max(     rel    , nr[1]); /*r[ 0,inf]*/
    nr[2] = max(fabs(vec[2]), nr[2]); /*y[-1,1]*/
    nr[3] = max(fabs(vec[3]), nr[3]); /*x[-1,1]*/
  }
  else if ((mode & TRGTMODE_CODING) != TRGTMODE_CODING_XYZ) {
    /* collect magnitudes */
    nr[1] = max(fabs(vec[1]), nr[1]); /*z[-1,1]*/
    nr[2] = max(fabs(vec[2]), nr[2]); /*y[-1,1]*/
    nr[3] = max(fabs(vec[3]), nr[3]); /*x[-1,1]*/
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

/* ####################################################################################
 */

template<int mode>
ULONG NormRGBH(long *bs, long *br, int av) {
  ULONG b = 0;

  /* build average of each channel an join */
  b |= ((bs[0] / av) << 24); /*h*/
  b |= ((bs[1] / av) << 16); /*b*/
  b |= ((bs[2] / av) <<  8); /*g*/
  b |= ((bs[3] / av) <<  0); /*r*/

  return b;
}

template<int mode>
ULONG NormRGBH(float *bs, float *br, int av) {
  ULONG b = 0;

  /* build average of each channel an join */
  b |= (rint(bs[0] / av) << 24); /*d[ 0,1]*/
  b |= (rint(bs[1] / av) << 16); /*z[-1,1]*/
  b |= (rint(bs[2] / av) <<  8); /*y[-1,1]*/
  b |= (rint(bs[3] / av) <<  0); /*x[-1,1]*/

  return b;
}

template<int mode>
ULONG NormXYZD(long *ns, long *nr, int av) {
  ULONG n = 0;

  n |= ((ns[0] + 0x00) / av) << 24; /*d[ 0,1]*/
  n |= ((ns[1] / av) << 16) + 0x80; /*z[-1,1]*/
  n |= ((ns[2] / av) <<  8) + 0x80; /*y[-1,1]*/
  n |= ((ns[3] / av) <<  0) + 0x80; /*x[-1,1]*/

  return n;
}

template<int mode>
ULONG NormXYZD(float *nn, float *nr, int av) {
  ULONG n = 0;
  float vec[4], len;
  float derivb = NORMALS_FLOAT_DXDY_TANGENTSPACE;	// [0.5,1.0]

  vec[0] = nn[0] / av;
  vec[1] = nn[1] / av;
  vec[2] = nn[2] / av;
  vec[3] = nn[3] / av;

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
      float derivx = 1.0 / nr[1];				// [...,1.0]
      if (derivb < derivx)
	derivb = derivx;
      if (derivb > 1.0)
	derivb = 1.0;
    }

    vec[1] =
      max(
	fabs(vec[1]),
      max(
	fabs(vec[2]) * derivb,
	fabs(vec[3]) * derivb
      ));
  }
  else if ((mode & TRGTMODE_CODING) != TRGTMODE_CODING_XYZ) {
    vec[1] = max(0.0, vec[1]);
  }

  /* ################################################################# */
  if ((mode & TRGTMODE_CODING) == TRGTMODE_CODING_XYZ) {
    if (mode & NORMMODE_CUBESPACE)
      len = max(vec[1], max(vec[2], vec[3]));
    else
      len = sqrt(vec[1] * vec[1] + vec[2] * vec[2] + vec[3] * vec[3]);

    vec[1] /= len; vec[1] *= 0.5; vec[1] += 0.5; vec[1] *= 0xFF;
    vec[2] /= len; vec[2] *= 0.5; vec[2] += 0.5; vec[2] *= 0xFF;
    vec[3] /= len; vec[3] *= 0.5; vec[3] += 0.5; vec[3] *= 0xFF;
  }
  /* ################################################################# */
  else if ((mode & TRGTMODE_CODING) == TRGTMODE_CODING_XYZt) {
    if (mode & NORMMODE_CUBESPACE)
      len = max(vec[1], max(vec[2], vec[3]));
    else
      len = sqrt(vec[1] * vec[1] + vec[2] * vec[2] + vec[3] * vec[3]);

    vec[1] /= len; vec[1] *= 1.0; vec[1] += 0.0; vec[1] *= 0xFF;
    vec[2] /= len; vec[2] *= 0.5; vec[2] += 0.5; vec[2] *= 0xFF;
    vec[3] /= len; vec[3] *= 0.5; vec[3] += 0.5; vec[3] *= 0xFF;
  }
  /* ################################################################# */
  else if ((mode & TRGTMODE_CODING) == TRGTMODE_CODING_XYt) {
    if (mode & NORMMODE_CUBESPACE) {
      float lnn;

      len = sqrt(vec[1] * vec[1] + vec[2] * vec[2] + vec[3] * vec[3]);
      lnn = sqrt(vec[2] * vec[2] + vec[3] * vec[3]);

      float factor = (2.0 - max(vec[2] / lnn, vec[3] / lnn)) / len;

      vec[1]  = 1.0;
      vec[2] *= factor; vec[2] *= 0.5; vec[2] += 0.5; vec[2] *= 0xFF;
      vec[3] *= factor; vec[3] *= 0.5; vec[3] += 0.5; vec[3] *= 0xFF;
    }
    else {
      len = sqrt(vec[1] * vec[1] + vec[2] * vec[2] + vec[3] * vec[3]);

      vec[1]  = 1.0;
      vec[2] /= len; vec[2] *= 0.5; vec[2] += 0.5; vec[2] *= 0xFF;
      vec[3] /= len; vec[3] *= 0.5; vec[3] += 0.5; vec[3] *= 0xFF;
    }
  }
  /* ################################################################# */
  else if (((mode & TRGTMODE_CODING) == TRGTMODE_CODING_DXDYt) ||
	   ((mode & TRGTMODE_CODING) == TRGTMODE_CODING_DXDYDZt)) {
    if (mode & NORMMODE_CUBESPACE) {
      float lnn;

      len = vec[1] / derivb;
      lnn = sqrt(vec[2] * vec[2] + vec[3] * vec[3]);

      float factor = (2.0 - max(vec[2] / lnn, vec[3] / lnn)) / len;

      vec[1] /= len;    vec[1] *= 0.5; vec[1] += 0.5; vec[1] *= 0xFF;
      vec[2] *= factor; vec[2] *= 0.5; vec[2] += 0.5; vec[2] *= 0xFF;
      vec[3] *= factor; vec[3] *= 0.5; vec[3] += 0.5; vec[3] *= 0xFF;

#if 0
      if (1) {
	float chk[4], cln, fct;

	chk[2] = ((vec[2] / 0xFF) - 0.5) / 0.5;
	chk[3] = ((vec[3] / 0xFF) - 0.5) / 0.5;

	cln = sqrt(chk[2] * chk[2] + chk[3] * chk[3]);

	fct = 2.0 - max(chk[2] / cln, chk[3] / cln);

	chk[1]  = 1.0 * derivb;
	chk[2] /= fct;
	chk[3] /= fct;

	cln = sqrt(chk[1] * chk[1] + chk[2] * chk[2] + chk[3] * chk[3]);

	chk[1] /= cln;
	chk[2] /= cln;
	chk[3] /= cln;

	cln = 1;
      }
#endif
    }
    else {
      /* this format is fully compatible with the built-in shaders */
      len = vec[1] / derivb;

      vec[1] /= len; vec[1] *= 0.5; vec[1] += 0.5; vec[1] *= 0xFF;
      vec[2] /= len; vec[2] *= 0.5; vec[2] += 0.5; vec[2] *= 0xFF;
      vec[3] /= len; vec[3] *= 0.5; vec[3] += 0.5; vec[3] *= 0xFF;
    }
  }
  /* ################################################################# */
  else if ((mode & TRGTMODE_CODING) == TRGTMODE_CODING_AZt) {
    float ang;

    len = sqrt(vec[1] * vec[1] + vec[2] * vec[2] + vec[3] * vec[3]);
    ang = atan2(vec[2], vec[3]) / M_PI; vec[2] = ang;

    vec[1] *= 1.0; vec[1] += 0.0; vec[1] *= 0xFF;
    vec[2] *= 0.5; vec[2] += 0.5; vec[3] *= 0xFF;
    vec[3]  = 1.0;
  }

  n |= (rint(vec[0]) << 24); /*d[ 0,1]*/
  n |= (rint(vec[1]) << 16); /*z[-1,1]*/
  n |= (rint(vec[2]) <<  8); /*y[-1,1]*/
  n |= (rint(vec[3]) <<  0); /*x[-1,1]*/

  return n;
}

/* ####################################################################################
 */

bool TextureConvert(D3DSURFACE_DESC &info, LPDIRECT3DTEXTURE9 *tex) {
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

#include "GUIs_DebugWindow.hpp"

#define TCOMPRESS_RGBH	      0
#define TCOMPRESS_XYZD	      1

template<typename UTYPE, typename type, int format>
bool TextureCompress(LPDIRECT3DTEXTURE9 *tex) {
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

  /* create the textures */
  lastOBGEDirect3DDevice9->CreateTexture(texo.Width, texo.Height, 0, 0, D3DFMT_DXT5, D3DPOOL_SYSTEMMEM, &text, NULL);

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

      /* generate this level's 4x4-block from the original surface */
      for (int ly = 0; ly < 4; ly += 1)
      for (int lx = 0; lx < 4; lx += 1) {
	type ts[4] = {0}; int yl = ((y + ly) << l);
	type tr[4] = {0}; int xl = ((x + lx) << l);

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

	  if (format == TCOMPRESS_RGBH)
	    AccuRGBH<ACCUMODE_LINEAR>(ts, tr, t, level, l);
	  else if (format == TCOMPRESS_XYZD)
	    AccuXYZD<ACCUMODE_SCALE>(ts, tr, t, level, l);
	}

	/* build average of each channel an join */
	ULONG t;

	if (format == TCOMPRESS_RGBH)
	  t = NormRGBH<TRGTMODE_CODING_RGB>(ts, tr, av);
	else if (format == TCOMPRESS_XYZD)
	  t = NormXYZD<TRGTMODE_CODING_DXDYDZt>(ts, tr, av);

	/* write the result ABGR */
	bTex[0][(ly * 4) + lx] = t;
      }

      /* compress to DXT5 */
#if 0
      if (format == TCOMPRESS_RGBH)
	stb_compress_dxt_block((unsigned char *)dTex, (unsigned char *)bTex[0], true, STB_DXT_DITHER | STB_DXT_HIGHQUAL);
      else
	stb_compress_dxt_block((unsigned char *)dTex, (unsigned char *)bTex[0], true, STB_DXT_NORMAL | STB_DXT_HIGHQUAL);
#else
      if (format == TCOMPRESS_RGBH)
	squish::Compress((unsigned char *)bTex[0], dTex, squish::kDxt5 | squish::kColourIterativeClusterFit | squish::kColourMetricPerceptual);
      else if (format == TCOMPRESS_XYZD)
	squish::Compress((unsigned char *)bTex[0], dTex, squish::kDxt5 | squish::kColourIterativeClusterFit | squish::kColourMetricUniform);
#endif

      /* advance pointer of compressed blocks */
      dTex += (128 / 32);

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

bool TextureCompressT(LPDIRECT3DTEXTURE9 *base) {
  bool res = true;

  res = res && TextureCompress<ULONG, long , TCOMPRESS_RGBH>(base);

  return res;
}

bool TextureCompressNM(LPDIRECT3DTEXTURE9 *norm) {
  bool res = true;

  res = res && TextureCompress<float, float, TCOMPRESS_XYZD>(norm);

  return res;
}

bool TextureCompressPM(LPDIRECT3DTEXTURE9 *base, LPDIRECT3DTEXTURE9 *norm) {
  bool res = true;

  res = res && TextureCompress<ULONG, long , TCOMPRESS_RGBH>(base);
  res = res && TextureCompress<float, float, TCOMPRESS_XYZD>(norm);

  return res;
}

bool TextureCompressQDM(LPDIRECT3DTEXTURE9 *base, LPDIRECT3DTEXTURE9 *norm) {
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

  /* create the textures */
  lastOBGEDirect3DDevice9->CreateTexture(baseo.Width, baseo.Height, 0, 0, D3DFMT_DXT5, D3DPOOL_SYSTEMMEM, &baset, NULL);
  lastOBGEDirect3DDevice9->CreateTexture(normo.Width, normo.Height, 0, 0, D3DFMT_DXT5, D3DPOOL_SYSTEMMEM, &normt, NULL);

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
      ULONG bBase[2][4*4];
      ULONG bNorm[2][4*4];

      /* generate this level's 4x4-block from the original surface */
      for (int ly = 0; ly < 4; ly += 1)
      for (int lx = 0; lx < 4; lx += 1) {
	long bs[4] = {0}; int yl = ((y + ly) << l);
	long br[4] = {0};
	long ns[4] = {0}; int xl = ((x + lx) << l);
	long nr[4] = {0};
	float nn[4] = {0.0};
	float rn[4] = {0.0};

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

	  AccuRGBH<ACCUMODE_LINEAR>(bs, br, b, level, l);
#if	defined(NORMALS_INTEGER)
	  AccuXYZD<ACCUMODE_SCALE>(ns, nr, n, level, l);
#else
	  AccuXYZD<ACCUMODE_SCALE>(nn, rn, n, level, l);
#endif
	}

	/* build average of each channel an join */
	ULONG b, n;

	b = NormRGBH<TRGTMODE_CODING_RGB>(bs, br, av);
#if	defined(NORMALS_INTEGER)
	n = NormXYZD<TRGTMODE_CODING_DXDYt | NORMMODE_CUBESPACE>(ns, nr, av);
#else
	n = NormXYZD<TRGTMODE_CODING_DXDYt | NORMMODE_CUBESPACE>(nn, rn, av);
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
      squish::Compress((unsigned char *)bBase[0], dBase, squish::kDxt5 | squish::kColourIterativeClusterFit | squish::kColourMetricPerceptual);
      squish::Compress((unsigned char *)bNorm[0], dNorm, squish::kDxt5 | squish::kColourIterativeClusterFit | squish::kColourMetricUniform);
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
