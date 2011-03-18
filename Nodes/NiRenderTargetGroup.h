/*	Name:			NiRenderTargetGroup
**	Description:	Contains the render targets. The engine can handle 4 MRT's but Bethesda only use the first one.
**	Inheritance:	NiObject
**	Size:			0x24
**	vTable:			0x00AB286C
**	vTable size:	0x22
**	Constructor:	0x009A20A0
**	Includes:		Ni2DBuffer, NiDepthStencilBuffer, D3D9
*/

#pragma once

#include "NiObject.h"
#include "Ni2DBuffer.h"
//#include "NiDepthStencilBuffer.h"
#include "D3D9.h"
#include "NiDX9TextureBufferData.h"

namespace v1_2_416
{
	class NiPixelFormat;
	class NiRendererData;
	class NiDepthStencilBuffer;
	class NiDX9ImplicitBufferData;

	class NiRenderTargetGroup: public NiObject
	{
	public:
		NiRenderTargetGroup();
		virtual ~NiRenderTargetGroup();

		virtual UInt32			GetWidth(UInt32 RenderTargetIndex);			// 13
		virtual UInt32			GetHeight(UInt32 RenderTagetIndex);			// 14
		virtual UInt32			GetDepthStencilWidth(void);				// 15
		virtual UInt32			GetDepthStencilHeight(void);				// 16
		virtual NiPixelFormat		*GetPixelFormat(UInt32 RenderTargetIndex);		// 17
		virtual NiPixelFormat		*GetDepthStencilPixelFormat(void);			// 18
		virtual UInt32			GetBufferCount(void);					// 19
		virtual bool			AttachBuffer(Ni2DBuffer* u1, UInt32 RenderTargetIndex);	// 1A - adds a render target texture?
		virtual bool			AttachDepthStencilBuffer(NiDepthStencilBuffer* u1);	// 1B
		virtual Ni2DBuffer		*GetBuffer(UInt32 RenderTargetIndex);			// 1C
		virtual NiDepthStencilBuffer	*GetDepthStencil(void);					// 1D
		virtual NiRendererData		*GetRendererData(void);					// 1E
		virtual void			SetRendererData(NiRendererData* u1);			// 1F
		virtual NiDX9TextureBufferData	*GetRenderTargetData(UInt32 RenderTargetIndex);		// 20
		virtual NiDX9TextureBufferData	*GetDepthStencilBufferRendererData(void);		// 21

		Ni2DBuffer			*RenderTargetBuffer0;					// 08
		Ni2DBuffer			*RenderTargetBuffer1;					// 0C
		Ni2DBuffer			*RenderTargetBuffer2;					// 10
		Ni2DBuffer			*RenderTargetBuffer3;					// 14
		UInt32				numRenderTargets;					// 18 - Number of render targets?
		NiDepthStencilBuffer		*DepthStencilBuffer;					// 1C
		NiRendererData			*RenderData;						// 20

	};
}
