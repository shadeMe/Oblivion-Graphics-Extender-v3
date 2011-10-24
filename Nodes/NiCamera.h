/*	Name:			NiCamera
**	Description:	Camera node.
**	Inheritance:	NiRefObject::NiObject::NiObjectNET::NiAVObject::
**	Size:			124
**	vTable:			00A7E4C4
**	vTable size:	21
**	Constructor:	0070D59
**	Includes:		NiViewPort
*/

#pragma once

#include "NiAVObject.h"
#include "NiNodes.h"
#include "NiViewPort.h"

namespace v1_2_416
{
	class NiCamera:public NiAVObject
	{
	public:
		NiCamera();
		virtual ~NiCamera();

		float		m_WorldToCam[4][4];			// 0AC - 0E8
		NiFrustum	m_kViewFrustum;				// 0EC - 104
		float		m_fMinNearPlaneDist;			// 108
		float		m_fMaxFarNearRatio;			// 10C
		NiViewPort	m_kPort;				// 110 - 11C
		float		m_fLODAdjust;				// 120
	};
}

