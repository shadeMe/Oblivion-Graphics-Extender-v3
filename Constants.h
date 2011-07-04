#pragma once

#include <d3dx9.h>

#define _USE_MATH_DEFINES
#include <math.h>

#include "Nodes/NiVector4.h"
#include "Nodes/NiDX9Renderer.h"
#include "Nodes/NiCamera.h"
#include "OBGE fork/Sky.h"
#include "Nodes/NiBillboardNode.h"
#include "obse/GameObjects.h"

extern struct sConstants
{
	// ****** Global static shader constants ******
	v1_2_416::NiVector4		rcpres;
	v1_2_416::NiVector4		rcpresh;	// heightmap
	v1_2_416::NiVector4		rcpresd;	// displacement

	// ****** Global shader constants (Updated each scene) ******
	D3DXMATRIX			wrld;
	D3DXMATRIX			view;
	D3DXMATRIX			view_inv;
	D3DXMATRIX			proj;
	D3DXMATRIX			proj_inv;
	D3DXMATRIX			viewproj;
	D3DXMATRIX			viewproj_inv;
	D3DXMATRIX			wrldviewproj;
	D3DXMATRIX			wrldviewproj_inv;

	v1_2_416::NiVector4		ZRange;
	v1_2_416::NiVector4		FoV;

	v1_2_416::NiVector4		SunDir;
	v1_2_416::NiVector4		SunPos;
	v1_2_416::NiVector4		SunTiming;
	v1_2_416::NiVector4		SunCoEffs;
	v1_2_416::NiVector4		SunColor;

	v1_2_416::NiVector4		PlayerPosition;
	v1_2_416::NiVector3		EyeForward;

	/* the four frustum rays in eye-space */
	D3DXMATRIX			EyeFrustum;

	v1_2_416::NiVector4		fGameTime;
	v1_2_416::NiVector4		fTikTiming;

	struct { int x, y, z, w; }	iGameTime;
	struct { int x, y, z, w; }	iTikTiming;

	/* deprecated */
#ifndef	NO_DEPRECATED
	v1_2_416::NiVector4		time;
	bool				bHasDepth;
#endif

	inline void UpdateWorld(const D3DXMATRIX &mx) {
	  wrld = mx;
	}

	inline void UpdateView(const D3DXMATRIX &mx) {
	  view = mx;
	}

	/* this is very likely constant over the playing-session */
	inline void UpdateProjection(const D3DMATRIX &mx) {
	  proj = mx;

	  /* row-major:
	   *
	   *  w 	0 	 0 	0
	   *  0 	h 	 0 	0
	   *  0 	0 	 Q 	1
	   *  0 	0 	-QN 	0
	   *
	   * w = X scaling factor
	   * h = Y scaling factor
	   * N = near Z
	   * F = far Z
	   * Q = F / (F-N)
	   */
	  ZRange.x = (-proj._43             /  proj._33       );
	  ZRange.y = ( proj._33 * ZRange.x) / (proj._33 - 1.0f);
	  ZRange.z = ZRange.y - ZRange.x;
	  ZRange.w = ZRange.y + ZRange.x;

#define acot(x)	(M_PI_2 - atan(x))
	  FoV.x = acot(proj._11) * 1;
	  FoV.y = acot(proj._22) * 1;
	  FoV.z = (FoV.x * 360.0) / M_PI;
	  FoV.w = (FoV.y * 360.0) / M_PI;
	}

	inline void UpdateProducts() {
	      viewproj =        view * proj;
	  wrldviewproj = wrld * view * proj;

	  /* inverse(s) */
	  D3DXMatrixInverse(&proj_inv        , NULL, &proj        );
	  D3DXMatrixInverse(&view_inv        , NULL, &view        );
	  D3DXMatrixInverse(&viewproj_inv    , NULL, &viewproj    );
	  D3DXMatrixInverse(&wrldviewproj_inv, NULL, &wrldviewproj);

	  /* http://www.lighthouse3d.com/tutorials/view-frustum-culling/geometric-approach-extracting-the-planes/
	   *
	   * Hfar = 2 * tan(fov / 2) * farDist
	   * Wfar = Hfar * ratio 
	   */
	  float Hfar = tan(FoV.y * 0.5) * ZRange.y;
	  float Wfar = tan(FoV.x * 0.5) * ZRange.y;
	  /* tan(0,522206 * 0,5) * 283408,90625 = 75727,6944923935580 */
	  /* tan(0,654498 * 0,5) * 283408,90625 = 96204,2860302489128 */

#if 1
	  /* we assume the camera is never rotated */
	  D3DXVECTOR3 Forw(EyeForward.x,  EyeForward.y, EyeForward.z);
	  D3DXVECTOR3 Wvec(EyeForward.y, -EyeForward.x, 0); // rotate 90�
	  D3DXVECTOR3 Hvec;
	  /* (0,1,0) */
	  /* (1,0,0) */

	  /* camera-ray hitting the far-plane */
	  D3DXVec3Normalize(&Forw, &Forw); Forw *= ZRange.y;
	  /* floor-level perpendicular left/right vector */
	  D3DXVec3Normalize(&Wvec, &Wvec); Wvec *= Wfar;
	  /* perpendicular up/down vector */
	  D3DXVec3Cross(&Hvec, &Wvec, &Forw);
	  D3DXVec3Normalize(&Hvec, &Hvec); Hvec *= Hfar;
	  /* (0            283408,90625 0           ) forw */
	  /* (96204,286030 0            0           ) left */
	  /* (0            0            75727,694492) up   */
#else
	  D3DXVECTOR3 Forw(0, 0, 1); Forw *= ZRange.y;
	  D3DXVECTOR3 Wvec(1, 0, 0); Wvec *= Wfar;
	  D3DXVECTOR3 Hvec(0, 1, 0); Hvec *= Hfar;
#endif

	  /* construct the four frustum corners (top-left) */
	  EyeFrustum._11 = Forw.x - Wvec.x + Hvec.x;
	  EyeFrustum._12 = Forw.y - Wvec.y + Hvec.y;
	  EyeFrustum._13 = Forw.z - Wvec.z + Hvec.z;
	  /* 0 - 96204,286030 + 0 = -96204,286030 */
	  /* 283408,90625 - 0 + 0 = 283408,906250 */
	  /* 0 - 0 + 75727,694492 =  75727,694492 */

	  /* construct the four frustum corners (bottom-left) */
	  EyeFrustum._21 = Forw.x - Wvec.x - Hvec.x;
	  EyeFrustum._22 = Forw.y - Wvec.y - Hvec.y;
	  EyeFrustum._23 = Forw.z - Wvec.z - Hvec.z;
	  /* 0 - 96204,286030 - 0 = -96204,286030 */
	  /* 283408,90625 - 0 - 0 = 283408,906250 */
	  /* 0 - 0 - 75727,694492 = -75727,694492 */

	  /* construct the four frustum corners (top-right) */
	  EyeFrustum._31 = Forw.x + Wvec.x + Hvec.x;
	  EyeFrustum._32 = Forw.y + Wvec.y + Hvec.y;
	  EyeFrustum._33 = Forw.z + Wvec.z + Hvec.z;
	  /* 0 + 96204,286030 + 0 =  96204,286030 */
	  /* 283408,90625 + 0 + 0 = 283408,906250 */
	  /* 0 + 0 + 75727,694492 =  75727,694492 */

	  /* construct the four frustum corners (bottom-right) */
	  EyeFrustum._41 = Forw.x + Wvec.x - Hvec.x;
	  EyeFrustum._42 = Forw.y + Wvec.y - Hvec.y;
	  EyeFrustum._43 = Forw.z + Wvec.z - Hvec.z;
	  /* 0 + 96204,286030 - 0 =  96204,286030 */
	  /* 283408,90625 + 0 - 0 = 283408,906250 */
	  /* 0 + 0 - 75727,694492 = -75727,694492 */
	}

	void Update();
	void Update(v1_2_416::NiDX9Renderer *Renderer);
	void UpdateSun();
	void UpdateSunCoEffs();
	void UpdateSunColor();
} Constants;
