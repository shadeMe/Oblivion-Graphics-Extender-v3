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
	D3DXMATRIX			proj;

	v1_2_416::NiVector4		ZRange;
	v1_2_416::NiVector4		FoV;

	v1_2_416::NiVector4		SunDir;
	v1_2_416::NiVector4		SunPos;
	v1_2_416::NiVector4		SunTiming;
	v1_2_416::NiVector4		SunCoEffs;
	v1_2_416::NiVector4		SunColor;

	v1_2_416::NiVector4		PlayerPosition;
	v1_2_416::NiVector3		EyeForward;

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

	void Update();
	void Update(v1_2_416::NiDX9Renderer *Renderer);
	void UpdateSun();
	void UpdateSunCoEffs();
	void UpdateSunColor();
} Constants;
