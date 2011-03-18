#pragma once

#include "D3D9.h"
#include "Nodes\NiDX9Renderer.h"

    /* TESWaterCulling-Filter
     *
     * Iguess this walks through the loaded scene-graph and marks objects visible/invisible.
     * There is a check for for the "node-type" as follows:
     *
     * 	Filtered away:
     * 		8,11,14,19,23 (?)
     * 	bUseWaterReflectionsActors:
     * 		17,18 (may PC, NPC?)
     * 	bUseWaterReflectionsTrees:
     * 		12,13 (?)
     * 	bUseWaterReflectionsStatics:
     * 		0,5,6,10 (?)
     * 	bUseWaterReflectionsMisc:
     * 		1-4,7,9,15,16,20-22,24 (?)
     *
     * Can anyone relate, I don't know maybe CS-IDs, with these IDs?
     */

    /* I try to complete this list, currently it's not entirely complete ...
     * Also not all purpose-indicators are fully correct, some as water-surface declared
     * render-targets are for the reflection, and apparently the as reflection-surface declared
     * render-target is not for reflections, or at least not the main render-target.
     */
    enum SurfaceIDs {
//	principle create hdr surface	0x007C2450
//	secondary create hdr surface	0x007BDFC0

//	"0" means recreate existing surface?
//	0				0x007c2470|72  = static	24	D3DFMT_A16B16G16R16F	1x1
//	0				0x007c2485|87  = static	24	D3DFMT_A16B16G16R16F	1x1
//	0				0x007c24A0|A2  = static	24	D3DFMT_A16B16G16R16F	1x1
//	0				0x007c24B5|C0  = static	26	D3DFMT_A16B16G16R16F	4x4
//	0				0x007c24F0|F2  = static	26	D3DFMT_A16B16G16R16F	64x64
//	0				0x007c25C6     = bckbuf	0	? Water			? Water
//	0				0x007c264B     = bckbuf	0	? Water			? Water
//	0				0x007c26CA     = bckbuf	0	? Water			? Water
//	0				0x007BE102     = query	0	?			?
//	0				0x0049BFAE     = static	0	0			100x100 reflection?

//	SURFACE_ID_HDR0			0x007BE4C5|C7  = calc	0	?			?
	SURFACE_ID_HDR0 = 0,		// HDR-Surface		26	D3DFMT_A16B16G16R16F	100x100	->	256x256
//	SURFACE_ID_HDR1			0x007c2558     = query	--	--------------------	-------
//	SURFACE_ID_HDR1			0x007BE214     = query	--	--------------------	-------
	SURFACE_ID_HDR1 = 1,		// HDR-Surface		26	D3DFMT_A16B16G16R16F	100x???	->	256x???		<-	sparse-samplemap, ScreenHeight/2
//	SURFACE_ID_HDR2			0x007c250E     = query	--	--------------------	-------
//	SURFACE_ID_HDR2			0x007BE755     = query	--	--------------------	-------
	SURFACE_ID_HDR2 = 2,		// HDR-Surface		26	D3DFMT_A16B16G16R16F	100x100	->	256x256
	SURFACE_ID_HDR3 = 3,		// HDR-Surface		26								<-	ScreenSize
//	SURFACE_ID_HDR4			0x007c2535     = query	--	--------------------	-------
//	SURFACE_ID_HDR4			0x007BE59A|66E = query	--	--------------------	-------
	SURFACE_ID_HDR4 = 4,		// ?			22	D3DFMT_A16B16G16R16F					<-	ScreenSize

	SURFACE_ID_UNK5 = 5,		// ?			26	D3DFMT_R32F						<-	ScreenSize-related

//	SURFACE_ID_WATER6		0x0049D9AD     = static	6	D3DFMT_R32F|0		100x100|80x80 twice
//	SURFACE_ID_WATER6		0x0049DA8F     = static	6	0			WaterSurfaceResolution
//	SURFACE_ID_WATER6		0x0049E833     = static	6	0			100x100
//	SURFACE_ID_WATER6		0x0049DBC3     = static	6	0			100x100
//	SURFACE_ID_WATER6		0x007EA020     = static	6	0			0
	SURFACE_ID_WATER6  = 6,		// Water-Displacement	6	D3DFMT_A32B32G32R32F	100x100	->	256x256		<-	WaterSurfaceResolution 100|80 HiRes.ini
	SURFACE_ID_WATER7  = 7,		//			6				100x100	->	256x256
//	SURFACE_ID_WATER8		0x0049DAE7     = query	--	--------------------	-------
	SURFACE_ID_WATER8  = 8,		// Water-Surface	6				80x80	->	128x128
	SURFACE_ID_WATER9  = 9,		//			C	D3DFMT_L16		100x1	->	256x1		<-	WaterSurfaceResolution 100|80 HiRes.ini
//	SURFACE_ID_WATER10		0x007E9DCD     = query	--	--------------------	-------
	SURFACE_ID_WATER10 = 10,	//			C	D3DFMT_A32B32G32R32F	100x16	->	256x16		<-	WaterSurfaceResolution 100|80 HiRes.ini
	SURFACE_ID_WATER11 = 11,	//			C	D3DFMT_R32F		100x100	->	256x256		<-	WaterSurfaceResolution 100|80 HiRes.ini
	SURFACE_ID_WATER12 = 12,	//			C	D3DFMT_A32B32G32R32F	100x100	->	256x256		<-	WaterSurfaceResolution 100|80 HiRes.ini

	SURFACE_ID_REFL13 = 13,		// Water-Surface	2	D3DFMT_A8R8G8B8		100x100	->	256x256		<-	via Water-Pass loop, preceeded by NiCamera-Constructor
	SURFACE_ID_REFL14 = 14,		//			6	D3DFMT_A8R8G8B8		100x100	->	256x256		<-	via Render-Pass loop, preceeded by NiCamera-Constructor

	SURFACE_ID_NONHDR15 = 15,	// Non FP16 HDR? ...	6				100x100	->	256x256		<-	BlurShaderResolution ini
	SURFACE_ID_NONHDR16 = 16,	//			6				100x100	->	256x256		<-	BlurShaderResolution ini
	SURFACE_ID_NONHDR17 = 17,	//			6				80x80	->	128x128
	SURFACE_ID_NONHDR18 = 18,	//			6				80x80	->	128x128
	SURFACE_ID_NONHDR19 = 19,	//			6				100x100	->	256x256		<-	BlurShaderResolution ini

	SURFACE_ID_UNK20 = 20,		//			46								<-	ScreenSize
	SURFACE_ID_UNK21 = 21,		//			2				100x100	->	256x256
	SURFACE_ID_UNK22 = 22,		//			6				200x200	->	512x512

	SURFACE_ID_SHADOW23 = 23,	// Shadowmap		6	D3DFMT_R32F		800x800	->	2048x2048	<-	ShadowMapResolution ini
	SURFACE_ID_SHADOW24 = 24,	//			10				100x100	->	256x256		<-	via Shadow-Pass

	SURFACE_ID_LAST = 0x7FFFFFFF
    };

void CreateRenderSurfaceHook(void);
