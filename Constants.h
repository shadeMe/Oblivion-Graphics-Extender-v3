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

#include <d3dx9.h>

#define _USE_MATH_DEFINES
#include <math.h>

#include "Nodes/NiVector4.h"
#include "Nodes/NiDX9Renderer.h"
#include "Nodes/NiCamera.h"
#include "OBGE fork/Sky.h"
#include "Nodes/NiBillboardNode.h"
#include "obse/GameObjects.h"
#include "obse/GameData.h"
#include "obse/NiProperties.h"
#include "obse/NiObjects.h"

extern struct sConstants
{
	// ****** Global static shader constants ******
	v1_2_416::NiVector4		rcpres;
	v1_2_416::NiVector4		rcpresh;	// heightmap
	v1_2_416::NiVector4		rcpresd;	// displacement

	// ****** Global shader constants (Updated each scene) ******
	D3DXMATRIX			wrld;
//	D3DXMATRIX			wrld_inv;
	D3DXMATRIX			view;
	D3DXMATRIX			view_inv;
	D3DXMATRIX			proj;
	D3DXMATRIX			proj_inv;
	D3DXMATRIX			viewproj;
	D3DXMATRIX			viewproj_inv;
	D3DXMATRIX			wrldviewproj;
	D3DXMATRIX			wrldviewproj_inv;
	D3DXMATRIX			pastviewproj;
	D3DXMATRIX			pastwrldviewproj;

	D3DXMATRIX			cmra;
	D3DXMATRIX			cmra_inv;
	v1_2_416::NiVector3		cmradir, cmraup, cmraright;

	v1_2_416::NiVector4		ZRange;
	v1_2_416::NiVector4		FoV;

	v1_2_416::NiVector4		SunDir;
	v1_2_416::NiVector4		SunPos;
	v1_2_416::NiVector4		SunTiming;
	v1_2_416::NiVector4		SunCoEffs;
	v1_2_416::NiVector4		SunColor;

	v1_2_416::NiVector4		LightDir;
	v1_2_416::NiVector4		LightColor;
	v1_2_416::NiVector4		AmbientColor;

	v1_2_416::NiVector4		FogRange;
	v1_2_416::NiVector4		FogColor;

	v1_2_416::NiVector4		PlayerPosition;

	/* the four frustum rays in eye-space */
	D3DXMATRIX			EyeFrustum;
	v1_2_416::NiVector3		EyeForward;
	v1_2_416::NiVector4		EyePosition;
	v1_2_416::NiVector4		EyeRotation;

	v1_2_416::NiVector4		fGameTime;
	v1_2_416::NiVector4		fTikTiming;

	struct { int x, y, z, w; }	iGameTime;
	struct { int x, y, z, w; }	iTikTiming;

	// DeGamma, strength, Gamma, strength
	v1_2_416::NiVector4		Gamma;

	/* internal variables */
	UInt32				WorldSpace; // 60 == Tamriel, 40728 == Shivering Isles
	bool				Oblivion;
	v1_2_416::NiVector4		Coordinates;
	bool				Exteriour;

	/* deprecated */
#ifndef	NO_DEPRECATED
	v1_2_416::NiVector4		time;
	bool				bHasDepth;
#endif

	inline void UpdateCamera(float *Loc, float *Dir, float *Up, float *Right) {
	  /* exact camera-data, nowhere else to be found ...
	   * had to be hooked as well, runs directly before SetTransform()
	   */
	  D3DXVECTOR3 local(0.0f, 0.0f, 0.0f);

	  cmradir.x = Dir[0]; cmraup.x = Up[0]; cmraright.x = Right[0];
	  cmradir.y = Dir[1]; cmraup.y = Up[1]; cmraright.y = Right[1];
	  cmradir.z = Dir[2]; cmraup.z = Up[2]; cmraright.z = Right[2];

	  /* make the transform to worldcam-space, not world-space (no translation) */
	  D3DXMatrixLookAtRH(&cmra,
	    (const D3DXVECTOR3 *)&local,
	    (const D3DXVECTOR3 *)Dir,
	    (const D3DXVECTOR3 *)Up);

	  D3DXMatrixInverse(&cmra_inv, NULL, &cmra);

	  EyePosition.x = Loc[0];
	  EyePosition.y = Loc[1];
	  EyePosition.z = Loc[2];

	  EyeForward.x = Dir[0];
	  EyeForward.y = Dir[1];
	  EyeForward.z = Dir[2];
	}

	inline void UpdateWorld(const D3DXMATRIX &mx) {
	  /* the world transform is updated each time the local
	   * coordinate-system is changed
	   */
	  wrld = mx;

//	  D3DXMatrixInverse(&wrld_inv, NULL, &wrld);
	}

	inline void UpdateView(const D3DXMATRIX &mx) {
	  /* the view transform is updated each time the camera
	   * rotation and position changes
	   */
	  view = mx;

	  /* assuming the view transform isn't a merged matrix
	   * we can extract the neutral eye-position from the
	   * inverse transform
	   */
	  D3DXMatrixInverse(&view_inv, NULL, &view);

	  /* Since a rotation matrix is orthogonal, R^-1 = R^t (transpose). */
	  D3DXMatrixTranspose(&view_inv, &view);

	  /* make the transform to worldcam-space, not world-space (no translation) */

#if 0
	  D3DXMatrixDecompose(&scl, &qtn, &trn, &view);
	  D3DXQuaternionToAxisAngle(&qtn, &axs, &ang);

#if 0
	  /* current camera/eye-position is at 0x00B3F92C
	   * for the reflection-pass z is negated
	   */
	  EyePosition.x = *((float *)0x00B3F92C) + view_inv._41;
	  EyePosition.y = *((float *)0x00B3F930) + view_inv._42;
	  EyePosition.z = *((float *)0x00B3F934) + view_inv._43;
	  EyePosition.w = 0.0;
#endif

	  /*
	      roll  (rotation around z) :  atan2(xy, xx)
	      pitch (rotation around y) : -arcsin(xz)
	      yaw   (rotation around x) :  atan2(yz,zz)

	      where the matrix is defined in the form:

	      [
		xx, yx, zx, px;
		xy, yy, zy, py;
		xz, yz, zz, pz;
		0, 0, 0, 1
	      ]
	  */

	  EyeRotation.y = -asin(view._32);
	  EyeRotation.x = acos(view._22 / cos(EyeRotation.y));
	  EyeRotation.z = acos(view._33 / cos(EyeRotation.y));
	  EyeRotation.w = 0.0;

	  EyeRotation.y = -asin(view._32);
	  EyeRotation.x = atan2(view._23, view._33);
	  EyeRotation.z = atan2(view._12, view._11);
	  EyeRotation.w = 0.0;

#if 0
	  // Returns the Yaw, Pitch, and Roll components of this
	  // matrix. This function only works with pure rotation
	  // matrices.

	  void GetRotation(float *v) const {
	    // yaw=v[0], pitch=v[1], roll=v[2]
	    // Note, we use the cosf function rather than sinf
	    // just in case the angles are greater than [-1,+1]
	    v[1]  = -asinf(_32);
	    //pitch
	    float cp = cosf(v[1]);
	    //_22 = cr * cp;
	    float cr = _22 / cp;
	    v[2] = acosf(cr);
	    //_33 = cy * cp;
	    float cy = _33 / cp;
	    v[0] = acosf(cy);
	  }

	  // creates a rotation matrix based on euler angles
	  // Y * P * R in the same order as DirectX.
	  void Rotate(const float *v) {
	    //yaw=v[0], pitch=v[1], roll=v[2]
	    float cy = cosf(v[0]);
	    float cp = cosf(v[1]);
	    float cr = cosf(v[2]);
	    float sp = sinf(v[1]);
	    float sr = sinf(v[2]);
	    float sy = sinf(v[0]);

	    _11  = cy * cr+ sr * sp * sy;
	    _12 = sr * cp;
	    _13 = cr * -sy + sr * sp * cy;
	    _21 = -sr * cy + cr * sp * sy;
	    _22 = cr * cp;
	    _23 = -sr * -sy + cr * sp * cy;
	    _31 = cp * sy;
	    _32 = -sp;
	    _33 = cy * cp;
	  }
#endif

	  /* TODO: find out the current camera's rotation
	   * and adjust the up/down mis-behaviour (precision?)
	   */
#endif
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
	  /* this only works if the world transform has been
	   * set to be from the main camera, so the inverses
	   * yield camera-relative world-coordinates
	   */

	  /* backup */
	  pastviewproj     = viewproj;
	  pastwrldviewproj = wrldviewproj;

	  /* make a new one */
	  viewproj     =        view * proj;
	  wrldviewproj = wrld * view * proj;
  
	  /* inverse(s) of projection(s) */
	  D3DXMatrixInverse(&proj_inv        , NULL, &proj        );
	  D3DXMatrixInverse(&viewproj_inv    , NULL, &viewproj    );
	  D3DXMatrixInverse(&wrldviewproj_inv, NULL, &wrldviewproj);

#if 0
	  D3DXVECTOR3 test(0.5f, 0.5f, 0.5f);
	  D3DXVECTOR4 rest;
	  D3DXVECTOR4 resr;

	  // Transform (x, y, z, 1) by matrix.
	  D3DXVec3Transform(&rest, &test, &proj_inv);
	  D3DXVec4Transform(&resr, &rest, &proj);

	  // Transform (x, y, z, 1) by matrix.
	  D3DXVec3Transform(&rest, &test, &proj_inv);
	  rest.x /= rest.w;
	  rest.y /= rest.w;
	  rest.z /= rest.w;
	  rest.w /= rest.w;
	  D3DXVec4Transform(&resr, &rest, &proj);
	  resr.x /= resr.w;
	  resr.y /= resr.w;
	  resr.z /= resr.w;
	  resr.w /= resr.w;
#endif

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
	  D3DXVECTOR3 Wvec(EyeForward.y, -EyeForward.x, 0); // rotate 90°
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
