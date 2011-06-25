#include <assert.h>

#include "RenderSurfaceParametersHook.hpp"
#include "windows.h"
#include "obse_common/SafeWrite.h"
#include "GlobalSettings.h"
#include "OBSEShaderInterface.h"

#include "D3D9.hpp"
#include "D3D9Device.hpp"
#include "D3D9Identifiers.hpp"

#include "Hooking/detours/detours.h"

static global<float> ReflectionMapSize(0, NULL, "ScreenBuffers", "iReflectionMapSize");
static global<int> WaterHeightMapSize(0, NULL, "ScreenBuffers", "iWaterHeightMapSize");
static global<int> WaterDisplacementMapSize(0, NULL, "ScreenBuffers", "iWaterDisplacementMapSize");
static global<int> AutoGenerateMipMaps(D3DTEXF_LINEAR, NULL, "ScreenBuffers", "iAutoGenerateMipMaps");
static global<int> RendererWidth(0, "Oblivion.ini", "Display", "iSize W");
static global<bool> UseWaterReflectionsMisc(0, "Oblivion.ini", "Water", "bUseWaterReflectionsMisc");
static global<bool> UseWaterReflectionsStatics(0, "Oblivion.ini", "Water", "bUseWaterReflectionsStatics");
static global<bool> UseWaterReflectionsTrees(0, "Oblivion.ini", "Water", "bUseWaterReflectionsTrees");
static global<bool> UseWaterReflectionsActors(0, "Oblivion.ini", "Water", "bUseWaterReflectionsActors");
static global<int> SurfaceTextureSize(128, "Oblivion.ini", "Water", "uSurfaceTextureSize");
static global<bool> UseWaterHiRes(0, "Oblivion.ini", "Water", "bUseWaterHiRes");

/* ------------------------------------------------------------------------------------------------- */

class Anonymous {

#ifndef	OBGE_NOSHADER
public:
  void TrackCombinerPass(int unk1);

  void TrackReflectionCull(int unk1);
  void TrackReflectionPass(int unk1, int unk2);
  void TrackWaterSurfacePass();
  void TrackWaterSurfaceLoop();
  void TrackWaterGeometryPass(int unk1, int unk2);
  bool TrackShadowPass();
  bool TrackShadowCanopyPass();
  bool TrackHDRAlphaPass(int unk1, int unk2);

  void TrackCameraPass(int unk1, int unk2);
  void TrackEffectsPass(int unk1, int unk2);

  void TrackHDRPass(int unk1, int unk2, int unk3, int unk4);
  void TrackBlurPass(int unk1, int unk2, int unk3, int unk4);
  void TrackHitPass(int unk1, int unk2, int unk3, int unk4);
  void TrackMenuPass(int unk1, int unk2, int unk3, int unk4);
  void TrackRefractionPass(int unk1, int unk2, int unk3, int unk4);
  void TrackNighteyePass(int unk1, int unk2, int unk3, int unk4);

  void TrackWaterDisplacementPass(int unk1, int unk2, int unk3, int unk4);
  void TrackWaterHeightmapPass(int unk1, int unk2, int unk3, int unk4);

  bool TrackVideoPass(int unk1, int unk2);
  void TrackMiscPass(int unk1);
#endif

public:
  void *TrackRenderedSurface(v1_2_416::NiDX9Renderer *renderer, int Width, int Height, int Flags, D3DFORMAT Format, enum SurfaceIDs SurfaceTypeID);
};

#ifndef	OBGE_NOSHADER
void (__thiscall Anonymous::* CombinerPass)(int)/* =
	(void (__thiscall TES::*)(int, int))0040C830*/;
void (__thiscall Anonymous::* ReflectionCull)(int)/* =
	(void (__thiscall TES::*)(int, int))0049CBF0*/;
void (__thiscall Anonymous::* ReflectionPass)(int, int)/* =
	(void (__thiscall TES::*)(int, int))0x0049BEF0*/;
void (__thiscall Anonymous::* WaterSurfaceLoop)()/* =
	(void (__thiscall TES::*)(int, int))0x0049A200*/;
void (__thiscall Anonymous::* WaterSurfacePass)()/* =
	(void (__thiscall TES::*)(int, int))0x0049D7B0*/;
void (__thiscall Anonymous::* WaterGeometryPass)(int, int)/* =
	(void (__thiscall TES::*)(int, int))0x0049B930*/;
bool (__thiscall Anonymous::* ShadowPass)()/* =
	(void (__thiscall TES::*)(int, int))0x004073D0*/;
bool (__thiscall Anonymous::* ShadowCanopyPass)()/* =
	(void (__thiscall TES::*)(int, int))0x004826F0*/;
bool (__thiscall Anonymous::* HDRAlphaPass)(int, int)/* =
	(void (__thiscall TES::*)(int, int))0x007AAA30*/;
void (__thiscall Anonymous::* HDRPass)(int, int, int, int)/* =
	(void (__thiscall TES::*)(int, int))0x007BDFC0*/;
void (__thiscall Anonymous::* BlurPass)(int, int, int, int)/* =
	(void (__thiscall TES::*)(int, int))0x007B0170*/;
void (__thiscall Anonymous::* HitPass)(int, int, int, int)/* =
	(void (__thiscall TES::*)(int, int))0x007EB3D0*/;
void (__thiscall Anonymous::* MenuPass)(int, int, int, int)/* =
	(void (__thiscall TES::*)(int, int))0x007B18C0*/;
void (__thiscall Anonymous::* RefractionPass)(int, int, int, int)/* =
	(void (__thiscall TES::*)(int, int))0x00800440*/;
void (__thiscall Anonymous::* NighteyePass)(int, int, int, int)/* =
	(void (__thiscall TES::*)(int, int))0x007F5120*/;
void (__thiscall Anonymous::* WaterDisplacementPass)(int, int, int, int)/* =
	(void (__thiscall TES::*)(int, int))0x007DE8A0*/;
void (__thiscall Anonymous::* WaterHeightmapPass)(int, int, int, int)/* =
	(void (__thiscall TES::*)(int, int))0x007E17D0*/;
bool (__thiscall Anonymous::* VideoPass)(int, int)/* =
	(void (__thiscall TES::*)(int, int))0x004106C0*/;
void (__thiscall Anonymous::* MiscPass)(int)/* =
	(void (__thiscall TES::*)(int, int))0x0057F170*/;

void (__cdecl * IdlePass)(int, int)/* =
	(void (__thiscall TES::*)(int, int))0x007D71C0*/;
bool (__cdecl * IsScriptRunning)(int, int)/* =
	(void (__thiscall TES::*)(int, int))0x004F8DB0*/;

void (__thiscall Anonymous::* TrackCombinerPass)(int)/* =
	(void (__thiscall TES::*)(int, int))0040C830*/;
void (__thiscall Anonymous::* TrackReflectionCull)(int)/* =
	(void (__thiscall TES::*)(int, int))0049CBF0*/;
void (__thiscall Anonymous::* TrackReflectionPass)(int, int)/* =
	(void (__thiscall TES::*)(int, int))0x0049BEF0*/;
void (__thiscall Anonymous::* TrackWaterSurfaceLoop)()/* =
	(void (__thiscall TES::*)(int, int))0x0049A200*/;
void (__thiscall Anonymous::* TrackWaterSurfacePass)()/* =
	(void (__thiscall TES::*)(int, int))0x0049D7B0*/;
void (__thiscall Anonymous::* TrackWaterGeometryPass)(int, int)/* =
	(void (__thiscall TES::*)(int, int))0x0049B930*/;
bool (__thiscall Anonymous::* TrackShadowPass)()/* =
	(void (__thiscall TES::*)(int, int))0x004073D0*/;
bool (__thiscall Anonymous::* TrackShadowCanopyPass)()/* =
	(void (__thiscall TES::*)(int, int))0x004826F0*/;
bool (__thiscall Anonymous::* TrackHDRAlphaPass)(int, int)/* =
	(void (__thiscall TES::*)(int, int))0x007AAA30*/;
void (__thiscall Anonymous::* TrackHDRPass)(int, int, int, int)/* =
	(void (__thiscall TES::*)(int, int))0x007BDFC0*/;
void (__thiscall Anonymous::* TrackBlurPass)(int, int, int, int)/* =
	(void (__thiscall TES::*)(int, int))0x007B0170*/;
void (__thiscall Anonymous::* TrackHitPass)(int, int, int, int)/* =
	(void (__thiscall TES::*)(int, int))0x007EB3D0*/;
void (__thiscall Anonymous::* TrackMenuPass)(int, int, int, int)/* =
	(void (__thiscall TES::*)(int, int))0x007B18C0*/;
void (__thiscall Anonymous::* TrackRefractionPass)(int, int, int, int)/* =
	(void (__thiscall TES::*)(int, int))0x00800440*/;
void (__thiscall Anonymous::* TrackNighteyePass)(int, int, int, int)/* =
	(void (__thiscall TES::*)(int, int))0x007F5120*/;
void (__thiscall Anonymous::* TrackWaterDisplacementPass)(int, int, int, int)/* =
	(void (__thiscall TES::*)(int, int))0x007DE8A0*/;
void (__thiscall Anonymous::* TrackWaterHeightmapPass)(int, int, int, int)/* =
	(void (__thiscall TES::*)(int, int))0x007E17D0*/;
bool (__thiscall Anonymous::* TrackVideoPass)(int, int)/* =
	(void (__thiscall TES::*)(int, int))0x004106C0*/;
void (__thiscall Anonymous::* TrackMiscPass)(int)/* =
	(void (__thiscall TES::*)(int, int))0x0057F170*/;
#endif

#ifndef	OBGE_NOSHADER
void Anonymous::TrackReflectionCull(int unk1) {
	*((char *)0x00B07068) = UseWaterReflectionsActors.Get();	// boolUseWaterReflectionsActors
	*((char *)0x00B07070) = UseWaterReflectionsTrees.Get();		// boolUseWaterReflectionsTrees
	*((char *)0x00B07078) = UseWaterReflectionsStatics.Get();	// boolUseWaterReflectionsStatics
	*((char *)0x00B07080) = UseWaterReflectionsMisc.Get();	        // boolUseWaterReflectionsMisc

	(this->*ReflectionCull)(unk1);
}

void Anonymous::TrackCombinerPass(int unk1) {
	enum OBGEPass previousPass = currentPass;
	currentPass = OBGEPASS_MAIN;

	if (frame_log)
		frame_log->Message("OD3D9: CombinerPass started");
//	else
//		_MESSAGE("OD3D9: CombinerPass started");

	{
		/* right location? */
		Constants.Update();
	}

	(this->*CombinerPass)(unk1);

	if (frame_log)
		frame_log->Message("OD3D9: CombinerPass finished");
//	else
//		_MESSAGE("OD3D9: CombinerPass finished");

	currentPass = previousPass;
}

void Anonymous::TrackReflectionPass(int unk1, int unk2) {
	enum OBGEPass previousPass = currentPass;
	currentPass = OBGEPASS_REFLECTION;

	if (frame_log)
		frame_log->Message("OD3D9: ReflectionPass started");
//	else
//		_MESSAGE("OD3D9: ReflectionPass started");

	(this->*ReflectionPass)(unk1, unk2);

	if (frame_log)
		frame_log->Message("OD3D9: ReflectionPass finished");
//	else
//		_MESSAGE("OD3D9: ReflectionPass finished");

	currentPass = previousPass;
}

void Anonymous::TrackWaterSurfaceLoop() {
	enum OBGEPass previousPass = currentPass;
	currentPass = OBGEPASS_WATER;

	if (frame_log)
		frame_log->Message("OD3D9: WaterSurfaceLoop started");
//	else
//		_MESSAGE("OD3D9: WaterSurfaceLoop started");

	(this->*WaterSurfaceLoop)();

	if (frame_log)
		frame_log->Message("OD3D9: WaterSurfaceLoop finished");
//	else
//		_MESSAGE("OD3D9: WaterSurfaceLoop finished");

	currentPass = previousPass;
}

void Anonymous::TrackWaterSurfacePass() {
	enum OBGEPass previousPass = currentPass;
	currentPass = OBGEPASS_WATER;

	if (frame_log)
		frame_log->Message("OD3D9: WaterSurfacePass started");
//	else
//		_MESSAGE("OD3D9: WaterSurfacePass started");

	(this->*WaterSurfacePass)();

	if (frame_log)
		frame_log->Message("OD3D9: WaterSurfacePass finished");
//	else
//		_MESSAGE("OD3D9: WaterSurfacePass finished");

	currentPass = previousPass;
}

void Anonymous::TrackWaterGeometryPass(int unk1, int unk2) {
	enum OBGEPass previousPass = currentPass;
	currentPass = OBGEPASS_WATER;

	if (frame_log)
		frame_log->Message("OD3D9: WaterGeometryPass started");
//	else
//		_MESSAGE("OD3D9: WaterGeometryPass started");

	(this->*WaterGeometryPass)(unk1, unk2);

	if (frame_log)
		frame_log->Message("OD3D9: WaterGeometryPass finished");
//	else
//		_MESSAGE("OD3D9: WaterGeometryPass finished");

	currentPass = previousPass;
}

bool Anonymous::TrackShadowPass() {
	enum OBGEPass previousPass = currentPass;
	currentPass = OBGEPASS_SHADOW;

	if (frame_log)
		frame_log->Message("OD3D9: ShadowPass started");
//	else
//		_MESSAGE("OD3D9: ShadowPass started");

	bool r = (this->*ShadowPass)();

	if (frame_log)
		frame_log->Message("OD3D9: ShadowPass finished");
//	else
//		_MESSAGE("OD3D9: ShadowPass finished");

	currentPass = previousPass;
	return r;
}

bool Anonymous::TrackShadowCanopyPass() {
	enum OBGEPass previousPass = currentPass;
	currentPass = OBGEPASS_SHADOW;

	if (frame_log)
		frame_log->Message("OD3D9: ShadowCanopy started");
//	else
//		_MESSAGE("OD3D9: ShadowCanopy started");

	bool r = (this->*ShadowCanopyPass)();

	if (frame_log)
		frame_log->Message("OD3D9: ShadowCanopy finished");
//	else
//		_MESSAGE("OD3D9: ShadowCanopy finished");

	currentPass = previousPass;
	return r;
}

bool Anonymous::TrackHDRAlphaPass(int unk1, int unk2) {
	enum OBGEPass previousPass = currentPass;
	currentPass = OBGEPASS_HDR;

	if (frame_log)
		frame_log->Message("OD3D9: HDRAlphaPass started");
//	else
//		_MESSAGE("OD3D9: HDRAlphaPass started");

	bool r = (this->*HDRAlphaPass)(unk1, unk2);

	if (frame_log)
		frame_log->Message("OD3D9: HDRAlphaPass finished");
//	else
//		_MESSAGE("OD3D9: HDRAlphaPass finished");

	currentPass = previousPass;
	return r;
}

void Anonymous::TrackHDRPass(int unk1, int unk2, int unk3, int unk4) {
	enum OBGEPass previousPass = currentPass;
	currentPass = OBGEPASS_HDR;

	if (frame_log)
		frame_log->Message("OD3D9: HDRPass started");
//	else
//		_MESSAGE("OD3D9: HDRPass started");

	(this->*HDRPass)(unk1, unk2, unk3, unk4);

	if (frame_log)
		frame_log->Message("OD3D9: HDRPass finished");
//	else
//		_MESSAGE("OD3D9: HDRPass finished");

	currentPass = previousPass;
}

void Anonymous::TrackBlurPass(int unk1, int unk2, int unk3, int unk4) {
	enum OBGEPass previousPass = currentPass;
	currentPass = OBGEPASS_POST;

	if (frame_log)
		frame_log->Message("OD3D9: BlurPass started");
//	else
//		_MESSAGE("OD3D9: BlurPass started");

	(this->*BlurPass)(unk1, unk2, unk3, unk4);

	if (frame_log)
		frame_log->Message("OD3D9: BlurPass finished");
//	else
//		_MESSAGE("OD3D9: BlurPass finished");

	currentPass = previousPass;
}

void Anonymous::TrackHitPass(int unk1, int unk2, int unk3, int unk4) {
	enum OBGEPass previousPass = currentPass;
	currentPass = OBGEPASS_POST;

	if (frame_log)
		frame_log->Message("OD3D9: HitPass started");
//	else
//		_MESSAGE("OD3D9: HitPass started");

	(this->*HitPass)(unk1, unk2, unk3, unk4);

	if (frame_log)
		frame_log->Message("OD3D9: HitPass finished");
//	else
//		_MESSAGE("OD3D9: HitPass finished");

	currentPass = previousPass;
}

void Anonymous::TrackMenuPass(int unk1, int unk2, int unk3, int unk4) {
	enum OBGEPass previousPass = currentPass;
	currentPass = OBGEPASS_POST;

	if (frame_log)
		frame_log->Message("OD3D9: MenuPass started");
//	else
//		_MESSAGE("OD3D9: MenuPass started");

	(this->*MenuPass)(unk1, unk2, unk3, unk4);

	if (frame_log)
		frame_log->Message("OD3D9: MenuPass finished");
//	else
//		_MESSAGE("OD3D9: MenuPass finished");

	currentPass = previousPass;
}

void Anonymous::TrackRefractionPass(int unk1, int unk2, int unk3, int unk4) {
	enum OBGEPass previousPass = currentPass;
	currentPass = OBGEPASS_POST;

	if (frame_log)
		frame_log->Message("OD3D9: RefractionPass started");
//	else
//		_MESSAGE("OD3D9: RefractionPass started");

	(this->*RefractionPass)(unk1, unk2, unk3, unk4);

	if (frame_log)
		frame_log->Message("OD3D9: RefractionPass finished");
//	else
//		_MESSAGE("OD3D9: RefractionPass finished");

	currentPass = previousPass;
}

void Anonymous::TrackNighteyePass(int unk1, int unk2, int unk3, int unk4) {
	enum OBGEPass previousPass = currentPass;
	currentPass = OBGEPASS_POST;

	if (frame_log)
		frame_log->Message("OD3D9: NighteyePass started");
//	else
//		_MESSAGE("OD3D9: NighteyePass started");

	(this->*NighteyePass)(unk1, unk2, unk3, unk4);

	if (frame_log)
		frame_log->Message("OD3D9: NighteyePass finished");
//	else
//		_MESSAGE("OD3D9: NighteyePass finished");

	currentPass = previousPass;
}

void Anonymous::TrackWaterDisplacementPass(int unk1, int unk2, int unk3, int unk4) {
	enum OBGEPass previousPass = currentPass;
	currentPass = OBGEPASS_WATERDISPLACEMENT;

	if (frame_log)
		frame_log->Message("OD3D9: WaterDisplacementPass started");
//	else
//		_MESSAGE("OD3D9: WaterDisplacementPass started");

	(this->*WaterDisplacementPass)(unk1, unk2, unk3, unk4);

	if (frame_log)
		frame_log->Message("OD3D9: WaterDisplacementPass finished");
//	else
//		_MESSAGE("OD3D9: WaterDisplacementPass finished");

	currentPass = previousPass;
}

void Anonymous::TrackWaterHeightmapPass(int unk1, int unk2, int unk3, int unk4) {
	enum OBGEPass previousPass = currentPass;
	currentPass = OBGEPASS_WATERHEIGHTMAP;

	if (frame_log)
		frame_log->Message("OD3D9: WaterHeightmapPass started");
//	else
//		_MESSAGE("OD3D9: WaterHeightmapPass started");

	(this->*WaterHeightmapPass)(unk1, unk2, unk3, unk4);

	if (frame_log)
		frame_log->Message("OD3D9: WaterHeightmapPass finished");
//	else
//		_MESSAGE("OD3D9: WaterHeightmapPass finished");

	currentPass = previousPass;
}

bool Anonymous::TrackVideoPass(int unk1, int unk2) {
	/* bink splash-video and possibly all bink playbacks
	 */
	enum OBGEPass previousPass = currentPass;
	currentPass = OBGEPASS_VIDEO;

	if (frame_log)
		frame_log->Message("OD3D9: VideoPass started");
//	else
//		_MESSAGE("OD3D9: VideoPass started");

	bool r = (this->*VideoPass)(unk1, unk2);

	if (frame_log)
		frame_log->Message("OD3D9: VideoPass finished");
//	else
//		_MESSAGE("OD3D9: VideoPass finished");

	currentPass = previousPass;
	return r;
}

void Anonymous::TrackMiscPass(int unk1) {
	/* occurs as early as the splash-video
	 */
	enum OBGEPass previousPass = currentPass;
//	currentPass = OBGEPASS_UNKNOWN;
	currentPass = OBGEPASS_MAIN;

	if (frame_log)
		frame_log->Message("OD3D9: MiscPass started");
//	else
//		_MESSAGE("OD3D9: MiscPass started");

	(this->*MiscPass)(unk1);

	if (frame_log)
		frame_log->Message("OD3D9: MiscPass finished");
//	else
//		_MESSAGE("OD3D9: MiscPass finished");

	currentPass = previousPass;
}

void __cdecl TrackIdlePass(int unk1, int unk2) {
	/* occurs as early as the menu
	 */
	enum OBGEPass previousPass = currentPass;
//	currentPass = OBGEPASS_UNKNOWN;
	currentPass = OBGEPASS_UNKNOWN;

	if (frame_log)
		frame_log->Message("OD3D9: IdlePass started");
//	else
//		_MESSAGE("OD3D9: IdlePass started");

	// Menu?
	IdlePass(unk1, unk2);

	if (frame_log)
		frame_log->Message("OD3D9: IdlePass finished");
//	else
//		_MESSAGE("OD3D9: IdlePass finished");

	currentPass = previousPass;
}

bool __cdecl TrackIsScriptRunning(int unk1, int unk2) {
	bool res = false;

	__try {
		res = IsScriptRunning(unk1, unk2);
	}
	__except (EXCEPTION_EXECUTE_HANDLER) {
		res = false;
	}

	return res;
}
#endif

/* ------------------------------------------------------------------------------------------------- */

void *(__thiscall Anonymous::* GetRenderedSurface)(v1_2_416::NiDX9Renderer *, int Width, int Height, int Flags, D3DFORMAT Format, enum SurfaceIDs SurfaceTypeID)/* =
	(void *(__thiscall *)(v1_2_416::NiDX9Renderer *, int Width, int Height, int Flags, int Format, enum SurfaceIDs SurfaceTypeID))0x007C1B50*/;

void *(__thiscall Anonymous::* TrackRenderedSurface)(v1_2_416::NiDX9Renderer *renderer, int Width, int Height, int Flags, D3DFORMAT Format, enum SurfaceIDs SurfaceTypeID)/* =
	(void (__thiscall *)(v1_2_416::NiDX9Renderer *renderer, int Width, int Height, int Flags, int Format, enum SurfaceIDs SurfaceTypeID))0x007C1B50*/;

void *Anonymous::TrackRenderedSurface(v1_2_416::NiDX9Renderer *renderer, int Width, int Height, int Flags, D3DFORMAT Format, enum SurfaceIDs SurfaceTypeID) {
//	assert(false);

#if 0	/* apparently the surface-type is always 0 here! Oblivion does not pass any!
         * lok at the parameters function below to detect surface-types
         */
	const char *SurfaceTypeName = "unknown";
	switch (SurfaceTypeID) {
		case SURFACE_ID_HDR0: SurfaceTypeName = "HDR-BoxSample Surface"; break;
		case SURFACE_ID_HDR1: SurfaceTypeName = "HDR-PointSample Surface"; break;
		case SURFACE_ID_HDR2: SurfaceTypeName = "HDR-Intermediate Surface"; break;
		case SURFACE_ID_HDR3: SurfaceTypeName = "HDR-Origin Surface"; break;
		case SURFACE_ID_HDR4: SurfaceTypeName = "HDR-Destination Surface"; break;

		case SURFACE_ID_UNK5: break;

		case SURFACE_ID_WATER6: SurfaceTypeName = "Water pre-heightmap Surface"; break;
		case SURFACE_ID_WATER7: SurfaceTypeName = "Water pre-displacement Surface"; break;
		case SURFACE_ID_WATER8: SurfaceTypeName = "Water[8] Surface"; break;
		case SURFACE_ID_WATER9: SurfaceTypeName = "Water[9] Surface"; break;
		case SURFACE_ID_WATER10: SurfaceTypeName = "Water[10] Surface"; break;
		case SURFACE_ID_WATER11: SurfaceTypeName = "Water[11] Surface"; break;
		case SURFACE_ID_WATER12: SurfaceTypeName = "Water[12] Surface"; break;

		case SURFACE_ID_REFL13: SurfaceTypeName = "First Reflection Surface"; break;
		case SURFACE_ID_REFL14: SurfaceTypeName = "Second Reflection Surface"; break;

		case SURFACE_ID_NONHDR15: break;
		case SURFACE_ID_NONHDR16: break;
		case SURFACE_ID_NONHDR17: break;
		case SURFACE_ID_NONHDR18: break;
		case SURFACE_ID_NONHDR19: break;

		case SURFACE_ID_UNK20: break;
		case SURFACE_ID_UNK21: break;
		case SURFACE_ID_UNK22: break;

		case SURFACE_ID_SHADOW23: SurfaceTypeName = "First Shadowmap Surface"; break;
		case SURFACE_ID_SHADOW24: SurfaceTypeName = "Second Shadowmap Surface"; break;
	}

	_DMESSAGE("OD3D9: Intercepted GetRenderedSurface():");
	_DMESSAGE("OD3D9: Intercepted Purpose: %s [0x%02x]", SurfaceTypeName, SurfaceTypeID);
	_DMESSAGE("OD3D9: Intercepted Flags: 0x%08x", Flags);
	_DMESSAGE("OD3D9: Intercepted Format: %s", findFormat(Format));
	_DMESSAGE("OD3D9: Intercepted {W,H}: {%d,%d}", Width, Height);
#endif

	UInt32 rWidth = v1_2_416::GetRenderer()->SizeWidth;
	UInt32 rHeight = v1_2_416::GetRenderer()->SizeHeight;

	/* these aren't well autodetected by Oblivion, they are
	 * intermediate render-targets for passing data between the
	 * main-pass and post-passes
	 */
	if ((Width == rWidth) && (Height == rHeight) && !Format) {
          if (IsHDR())
	    Format = D3DFMT_A16B16G16R16F;
          else
	    Format = D3DFMT_A8R8G8B8;
	}

	return (this->*GetRenderedSurface)(renderer, Width, Height, Flags, Format, SurfaceTypeID);

//	assert(NULL);
}

/* ------------------------------------------------------------------------------------------------- */

void (__stdcall * GetRenderedSurfaceParameters)(v1_2_416::NiDX9Renderer *, enum SurfaceIDs SurfaceTypeID, int *pWidth, int *pHeight, int, int *, D3DFORMAT *pFormat) =
	(void (__stdcall *)(v1_2_416::NiDX9Renderer *, enum SurfaceIDs SurfaceTypeID, int *pWidth, int *pHeight, int, int *, D3DFORMAT *pFormat))0x007C0D10;


void __stdcall TrackRenderedSurfaceParameters(v1_2_416::NiDX9Renderer *renderer, enum SurfaceIDs SurfaceTypeID, int *pWidth, int *pHeight, int unk1, int *unk2, D3DFORMAT *pFormat) {
//	assert(false);

	GetRenderedSurfaceParameters(renderer, SurfaceTypeID, pWidth, pHeight, unk1, unk2, pFormat);

//	assert(NULL);

#if 0	/* currently of no use ... maybe you find one :^) */
	const char *SurfaceTypeName = "unknown";
	switch (SurfaceTypeID) {
		case SURFACE_ID_HDR0: SurfaceTypeName = "HDR-BoxSample Surface"; break;
		case SURFACE_ID_HDR1: SurfaceTypeName = "HDR-PointSample Surface"; break;
		case SURFACE_ID_HDR2: SurfaceTypeName = "HDR-Intermediate Surface"; break;
		case SURFACE_ID_HDR3: SurfaceTypeName = "HDR-Origin Surface"; break;
		case SURFACE_ID_HDR4: SurfaceTypeName = "HDR-Destination Surface"; break;

		case SURFACE_ID_UNK5: break;

		case SURFACE_ID_WATER6: SurfaceTypeName = "Water pre-heightmap Surface"; break;
		case SURFACE_ID_WATER7: SurfaceTypeName = "Water pre-displacement Surface"; break;
		case SURFACE_ID_WATER8: SurfaceTypeName = "Water[8] Surface"; break;
		case SURFACE_ID_WATER9: SurfaceTypeName = "Water[9] Surface"; break;
		case SURFACE_ID_WATER10: SurfaceTypeName = "Water[10] Surface"; break;
		case SURFACE_ID_WATER11: SurfaceTypeName = "Water[11] Surface"; break;
		case SURFACE_ID_WATER12: SurfaceTypeName = "Water[12] Surface"; break;

		case SURFACE_ID_REFL13: SurfaceTypeName = "First Reflection Surface"; break;
		case SURFACE_ID_REFL14: SurfaceTypeName = "Second Reflection Surface"; break;

		case SURFACE_ID_NONHDR15: break;
		case SURFACE_ID_NONHDR16: break;
		case SURFACE_ID_NONHDR17: break;
		case SURFACE_ID_NONHDR18: break;
		case SURFACE_ID_NONHDR19: break;

		case SURFACE_ID_UNK20: break;
		case SURFACE_ID_UNK21: break;
		case SURFACE_ID_UNK22: break;

		case SURFACE_ID_SHADOW23: SurfaceTypeName = "First Shadowmap Surface"; break;
		case SURFACE_ID_SHADOW24: SurfaceTypeName = "Second Shadowmap Surface"; break;
	}

	_DMESSAGE("OD3D9: Intercepted GetRenderedSurfaceParameters():");
	_DMESSAGE("OD3D9: Intercepted Purpose: %s [0x%02x]", SurfaceTypeName, SurfaceTypeID);
//	_DMESSAGE("OD3D9: Intercepted Flags: 0x%08x", Flags);
	_DMESSAGE("OD3D9: Intercepted Format: %s", findFormat(*pFormat));
	_DMESSAGE("OD3D9: Intercepted {W,H} before: {%d,%d}", *pWidth, *pHeight);
#endif
#if 1
#ifndef	OBGE_NOSHADER
	/* enable default automipmapping */
	AMFilter = (D3DTEXTUREFILTERTYPE)AutoGenerateMipMaps.Get();
#endif

	switch (SurfaceTypeID) {
		case SURFACE_ID_WATER6:
		  /* Water heightmap */
		  if (UseWaterHiRes.Get()) {
		    if (WaterHeightMapSize.Get()) {
		      *pWidth = *pHeight = WaterHeightMapSize.Get();
		    }
		    else {
		      *pWidth = *pHeight = 256;
		    }
		  }
		  else {
		    if (WaterHeightMapSize.Get()) {
		      *pWidth = *pHeight = WaterHeightMapSize.Get();
		    }
		    else {
		      *pWidth = *pHeight = 128;
		    }
		  }

#ifndef	OBGE_NOSHADER
		  {
		    ShaderManager *sm = ShaderManager::GetSingleton();
		    const float W = (float)*pWidth;
		    const float H = (float)*pHeight;

		    /* record constants */
		    Constants.rcpresh[0] = 1.0f / W;
		    Constants.rcpresh[1] = 1.0f / H;
		    Constants.rcpresh[2] = W / H;
		    Constants.rcpresh[3] = W * H;
		  }
#endif
		  break;
		case SURFACE_ID_WATER7:
		  /* Water displacement */
		  if (UseWaterHiRes.Get()) {
		    if (WaterDisplacementMapSize.Get()) {
		      *pWidth = *pHeight = WaterDisplacementMapSize.Get();
		    }
		    else {
		      *pWidth = *pHeight = 256;
		    }
		  }
		  else {
		    if (WaterDisplacementMapSize.Get()) {
		      *pWidth = *pHeight = WaterDisplacementMapSize.Get();
		    }
		    else {
		      *pWidth = *pHeight = 256;
		    }
		  }

#ifndef	OBGE_NOSHADER
		  {
		    ShaderManager *sm = ShaderManager::GetSingleton();
		    const float W = (float)*pWidth;
		    const float H = (float)*pHeight;

		    /* record constants */
		    Constants.rcpresd[0] = 1.0f / W;
		    Constants.rcpresd[1] = 1.0f / H;
		    Constants.rcpresd[2] = W / H;
		    Constants.rcpresd[3] = W * H;
		  }
#endif
		  break;
	//	case SURFACE_ID_WATER12: *pWidth = *pHeight = 256; break;

		/* are these the reflection-rendertargets or water surfaces? */
		case SURFACE_ID_REFL13:
		case SURFACE_ID_REFL14:
		  /* Reflection Render-Surface Dimension (square) */
		  if ((ReflectionMapSize.Get() >= 256) &&
		      (ReflectionMapSize.Get() <= 2560)) {

		      *pWidth = *pHeight = ReflectionMapSize.Get();
		  }
		  else if ((ReflectionMapSize.Get() == 0) &&
			   (RendererWidth.Get() >= 256) &&
			   (RendererWidth.Get() <= 2560)) {

		      *pWidth = *pHeight = RendererWidth.Get();
		  }
		  else if ((ReflectionMapSize.Get() >  0) &&
			   (ReflectionMapSize.Get() <= 1) &&
			   (ReflectionMapSize.Get() * RendererWidth.Get() >= 256) &&
			   (ReflectionMapSize.Get() * RendererWidth.Get() <= 2560)) {

		      *pWidth = *pHeight = ReflectionMapSize.Get() * RendererWidth.Get();
		  }

		  break;
	}

#if 0
	_DMESSAGE("OD3D9: Intercepted {W,H} after: {%d,%d}", *pWidth, *pHeight);
#endif
#endif
}

/* ------------------------------------------------------------------------------------------------- */

void CreateRenderSurfaceHook(void) {
	/* combined passes: 0040C830 */
	/* combined reflection and water passes: 0049E880 */
	/* combined hdr related: 0049E880 */

#ifndef	OBGE_NOSHADER
	/* ReflectionPass */
	*((int *)&CombinerPass)          = 0x0040C830;
	*((int *)&ReflectionCull)        = 0x0049CBF0;
	*((int *)&ReflectionPass)        = 0x0049BEF0;
	*((int *)&WaterSurfaceLoop)      = 0x0049A200;
	*((int *)&WaterSurfacePass)      = 0x0049D7B0;  // broken sub esp 8
	*((int *)&WaterGeometryPass)     = 0x0049B930;
	*((int *)&ShadowPass)            = 0x004073D0;
	*((int *)&ShadowCanopyPass)      = 0x004826F0;

	*((int *)&HDRAlphaPass)          = 0x007AAA30;

	*((int *)&HDRPass)               = 0x007BDFC0;
	*((int *)&BlurPass)              = 0x007B0170;
	*((int *)&HitPass)               = 0x007EB3D0;
	*((int *)&MenuPass)              = 0x007B18C0;
	*((int *)&RefractionPass)        = 0x00800440;
	*((int *)&NighteyePass)          = 0x007F5120;
	*((int *)&WaterDisplacementPass) = 0x007DE8A0;
	*((int *)&WaterHeightmapPass)    = 0x007E17D0;

	*((int *)&VideoPass)             = 0x004106C0;
	*((int *)&MiscPass)              = 0x0057F170;


	TrackCombinerPass          = &Anonymous::TrackCombinerPass;
	TrackReflectionCull        = &Anonymous::TrackReflectionCull;
	TrackReflectionPass        = &Anonymous::TrackReflectionPass;
	TrackWaterSurfaceLoop      = &Anonymous::TrackWaterSurfaceLoop;		// ?
	TrackWaterSurfacePass      = &Anonymous::TrackWaterSurfacePass;		// ?
	TrackWaterGeometryPass     = &Anonymous::TrackWaterGeometryPass;	// ?
	TrackShadowPass            = &Anonymous::TrackShadowPass;
	TrackShadowCanopyPass      = &Anonymous::TrackShadowCanopyPass;

	TrackHDRAlphaPass          = &Anonymous::TrackHDRAlphaPass;

	TrackHDRPass               = &Anonymous::TrackHDRPass;			// BSImageSpaceShader
	TrackBlurPass              = &Anonymous::TrackBlurPass;			// BSImageSpaceShader
	TrackHitPass               = &Anonymous::TrackHitPass;			// BSImageSpaceShader
	TrackMenuPass              = &Anonymous::TrackMenuPass;			// BSImageSpaceShader
	TrackRefractionPass        = &Anonymous::TrackRefractionPass;		// BSImageSpaceShader
	TrackNighteyePass          = &Anonymous::TrackNighteyePass;		// BSImageSpaceShader
	TrackWaterDisplacementPass = &Anonymous::TrackWaterDisplacementPass;	// BSImageSpaceShader
	TrackWaterHeightmapPass    = &Anonymous::TrackWaterHeightmapPass;	// BSImageSpaceShader

	TrackVideoPass             = &Anonymous::TrackVideoPass;
	TrackMiscPass              = &Anonymous::TrackMiscPass;

	*((int *)&IdlePass) = 0x007D71C0;
	*((int *)&IsScriptRunning) = 0x004F8DB0;
#endif

	/* GetRenderedSurfaceParameters */
	*((int *)&GetRenderedSurface          ) = 0x007C1B50;
	*((int *)&GetRenderedSurfaceParameters) = 0x007C0D10;

	TrackRenderedSurface       = &Anonymous::TrackRenderedSurface;

        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());

#ifndef	OBGE_NOSHADER
	DetourAttach(&(PVOID&)CombinerPass,          *((PVOID *)&TrackCombinerPass));
	DetourAttach(&(PVOID&)ReflectionCull,        *((PVOID *)&TrackReflectionCull));
	DetourAttach(&(PVOID&)ReflectionPass,        *((PVOID *)&TrackReflectionPass));
//	DetourAttach(&(PVOID&)WaterSurfaceLoop,      *((PVOID *)&TrackWaterSurfaceLoop));
//	DetourAttach(&(PVOID&)WaterSurfacePass,      *((PVOID *)&TrackWaterSurfacePass));
//	DetourAttach(&(PVOID&)WaterGeometryPass,     *((PVOID *)&TrackWaterGeometryPass));
	DetourAttach(&(PVOID&)ShadowPass,            *((PVOID *)&TrackShadowPass));
	DetourAttach(&(PVOID&)ShadowCanopyPass,      *((PVOID *)&TrackShadowCanopyPass));

	DetourAttach(&(PVOID&)HDRAlphaPass,          *((PVOID *)&TrackHDRAlphaPass));

	DetourAttach(&(PVOID&)HDRPass,               *((PVOID *)&TrackHDRPass));
	DetourAttach(&(PVOID&)BlurPass,              *((PVOID *)&TrackBlurPass));
	DetourAttach(&(PVOID&)HitPass,               *((PVOID *)&TrackHitPass));
	DetourAttach(&(PVOID&)MenuPass,              *((PVOID *)&TrackMenuPass));
	DetourAttach(&(PVOID&)RefractionPass,        *((PVOID *)&TrackRefractionPass));
	DetourAttach(&(PVOID&)NighteyePass,          *((PVOID *)&TrackNighteyePass));
	DetourAttach(&(PVOID&)WaterDisplacementPass, *((PVOID *)&TrackWaterDisplacementPass));
	DetourAttach(&(PVOID&)WaterHeightmapPass,    *((PVOID *)&TrackWaterHeightmapPass));

	DetourAttach(&(PVOID&)VideoPass,             *((PVOID *)&TrackVideoPass));
	DetourAttach(&(PVOID&)MiscPass,              *((PVOID *)&TrackMiscPass));

	DetourAttach(&(PVOID&)IdlePass, TrackIdlePass);
//	DetourAttach(&(PVOID&)IsScriptRunning, TrackIsScriptRunning);
#endif

	DetourAttach(&(PVOID&)GetRenderedSurface, *((PVOID *)&TrackRenderedSurface         ));
	DetourAttach(&(PVOID&)GetRenderedSurfaceParameters,   TrackRenderedSurfaceParameters);
        LONG error = DetourTransactionCommit();

//	*((int *)&WaterSurfacePass) += 2;  // broken sub esp 8, prefixes 2 bytes before instructions

        if (error == NO_ERROR) {
		_MESSAGE("Detoured GetRenderedSurfaceParameters(); succeeded");
        }
        else {
		_MESSAGE("Detoured GetRenderedSurfaceParameters(); failed");
        }

#ifndef	OBGE_NOSHADER
	/* enable default automipmapping */
	AMFilter = (D3DTEXTUREFILTERTYPE)AutoGenerateMipMaps.Get();
#endif

	/* Reflection Render-Surface Dimension (square) */
	if ((ReflectionMapSize.Get() >= 256) &&
	    (ReflectionMapSize.Get() <= 2560)) {

		SafeWrite32(0x0049BFAF, ReflectionMapSize.Get());
	}
	else if ((ReflectionMapSize.Get() == 0) &&
	         (RendererWidth.Get() >= 256) &&
	         (RendererWidth.Get() <= 2560)) {

		SafeWrite32(0x0049BFAF, RendererWidth.Get());
	}
	else if ((ReflectionMapSize.Get() >  0) &&
	         (ReflectionMapSize.Get() <= 1) &&
	         (ReflectionMapSize.Get() * RendererWidth.Get() >= 256) &&
	         (ReflectionMapSize.Get() * RendererWidth.Get() <= 2560)) {

		SafeWrite32(0x0049BFAF, ReflectionMapSize.Get() * RendererWidth.Get());
	}

	//00B07058	// flagUseWaterHiRes
	//00B45FD0	// OneIfWaterHiRes
	//00B45FC8	// WaterSurfaceResolution
	//00B45FCC	// widthStaticSquareForSomeSurface2log2
	if (UseWaterHiRes.Get()) {
		SafeWrite32(0x00B45FD0, 1);
		SafeWrite32(0x00B45FC8, 256);
		SafeWrite32(0x00B45FCC, 8);

		if ((WaterHeightMapSize.Get() >= 256) &&
		    (WaterHeightMapSize.Get() <= 1024)) {
		    	int l = 8;
		    	while ((1 << l) < WaterHeightMapSize.Get())
		    	  l++;

			SafeWrite32(0x00499F0A, 1 << l);	// SetWaterResolution
			SafeWrite32(0x00499F14, l);		// SetWaterResolution
			SafeWrite32(0x007E1092, 1 << l);	// Heightmap constructor
			SafeWrite32(0x007E109C, l);		// Heightmap constructor

			SafeWrite32(0x0049D9A8, 1 << l);	// R32F
			SafeWrite32(0x0049D9C5, 1 << l);	// A8R8G8B8

			WaterHeightMapSize.Set(1 << l);
		}
		else
			WaterHeightMapSize.Set(0);
	}
	else {
		SafeWrite32(0x00B45FD0, 0);
		SafeWrite32(0x00B45FC8, 128);
		SafeWrite32(0x00B45FCC, 1);

		if ((WaterHeightMapSize.Get() >= 32) &&
		    (WaterHeightMapSize.Get() <= 128)) {
		    	int l = 7;
		    	while ((1 << l) > WaterHeightMapSize.Get())
		    	  l--;

			SafeWrite32(0x00499F20, 1 << l);	// SetWaterResolution
			SafeWrite32(0x00499F2A, l);		// SetWaterResolution
			SafeWrite32(0x007E10A8, 1 << l);	// Heightmap constructor
			SafeWrite32(0x007E10B2, l);		// Heightmap constructor

			SafeWrite32(0x0049D9D3, 1 << l);	// R32F
			SafeWrite32(0x0049D9EF, 1 << l);	// A8R8G8B8

			WaterHeightMapSize.Set(1 << l);
		}
		else
			WaterHeightMapSize.Set(0);
	}

	//00B07058	// flagUseWaterHiRes
	//00B45FD0	// OneIfWaterHiRes
	//00B45FC8	// WaterSurfaceResolution
	//00B45FCC	// widthStaticSquareForSomeSurface2log2
	if (UseWaterHiRes.Get()) {
		if ((WaterDisplacementMapSize.Get() >= 256) &&
		    (WaterDisplacementMapSize.Get() <= 1024)) {
		    	int l = 8;
		    	while ((1 << l) < WaterDisplacementMapSize.Get())
		    	  l++;

			SafeWrite32(0x0049DBC4, 1 << l);	// Wading water? (always 256)
			SafeWrite32(0x0049E82B, 1 << l);	// Displacementmap constructor (always 256)

			SafeWrite32(0x004D0225, 1 << l);	// ????? (always 256)
			SafeWrite32(0x004D0260, 1 << l);	// ????? (always 256)
			SafeWrite32(0x004D0299, 1 << l);	// ????? (always 256)

			WaterDisplacementMapSize.Set(1 << l);
		}
		else
			WaterDisplacementMapSize.Set(0);
	}
	else {
		if ((WaterDisplacementMapSize.Get() >= 32) &&
		    (WaterDisplacementMapSize.Get() <= 128)) {
		    	int l = 7;
		    	while ((1 << l) > WaterDisplacementMapSize.Get())
		    	  l--;

			SafeWrite32(0x0049DBC4, 1 << l);	// Wading water?
			SafeWrite32(0x0049E82B, 1 << l);	// Displacementmap constructor (always 256)

			SafeWrite32(0x004D0225, 1 << l);	// ????? (always 256)
			SafeWrite32(0x004D0260, 1 << l);	// ????? (always 256)
			SafeWrite32(0x004D0299, 1 << l);	// ????? (always 256)

			WaterDisplacementMapSize.Set(1 << l);
		}
		else
			WaterDisplacementMapSize.Set(0);
	}

//	SafeWrite8(0x00B07068, UseWaterReflectionsActors.Get());	// boolUseWaterReflectionsActors
//	SafeWrite8(0x00B07070, UseWaterReflectionsTrees.Get());		// boolUseWaterReflectionsTrees
//	SafeWrite8(0x00B07078, UseWaterReflectionsStatics.Get());	// boolUseWaterReflectionsStatics
//	SafeWrite8(0x00B07080, UseWaterReflectionsMisc.Get());		// boolUseWaterReflectionsMisc

	return;
}
