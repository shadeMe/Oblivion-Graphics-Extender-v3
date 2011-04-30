#include "D3D9.hpp"
#include "RenderSurfaceParametersHook.hpp"
#include "windows.h"
#include "obse_common/SafeWrite.h"
#include "GlobalSettings.h"
#include <assert.h>

#include "Hooking/detours/detours.h"

static global<int> ReflectionMapSize(256, NULL, "ScreenBuffers", "iReflectionMapSize");
static global<int> RendererWidth(0, "Oblivion.ini", "Display", "iSize W");

/* ------------------------------------------------------------------------------------------------- */

class Anonymous {

public:
  void TrackCombinerPass(int unk1);

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
};

void (__thiscall Anonymous::* CombinerPass)(int)/* =
	(void (__thiscall TES::*)(int, int))0040C830*/;
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

void (__thiscall Anonymous::* TrackCombinerPass)(int)/* =
	(void (__thiscall TES::*)(int, int))0040C830*/;
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

static enum OBGEPass previousPass;

void Anonymous::TrackCombinerPass(int unk1) {
	previousPass = currentPass;
	currentPass = OBGEPASS_MAIN;

	if (frame_log)
		frame_log->Message("OD3D9: CombinerPass started");
//	else
//		_MESSAGE("OD3D9: CombinerPass started");

	(this->*CombinerPass)(unk1);

	if (frame_log)
		frame_log->Message("OD3D9: CombinerPass finished");
//	else
//		_MESSAGE("OD3D9: CombinerPass finished");

	currentPass = previousPass;
}

void Anonymous::TrackReflectionPass(int unk1, int unk2) {
	previousPass = currentPass;
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
	previousPass = currentPass;
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
	previousPass = currentPass;
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
	previousPass = currentPass;
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
	previousPass = currentPass;
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
	previousPass = currentPass;
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
	previousPass = currentPass;
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
	previousPass = currentPass;
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
	previousPass = currentPass;
	currentPass = OBGEPASS_EFFECTS;

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
	previousPass = currentPass;
	currentPass = OBGEPASS_EFFECTS;

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
	previousPass = currentPass;
	currentPass = OBGEPASS_EFFECTS;

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
	previousPass = currentPass;
	currentPass = OBGEPASS_EFFECTS;

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
	previousPass = currentPass;
	currentPass = OBGEPASS_EFFECTS;

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
	previousPass = currentPass;
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
	previousPass = currentPass;
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
	previousPass = currentPass;
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
	previousPass = currentPass;
	currentPass = OBGEPASS_UNKNOWN;

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
	previousPass = currentPass;
	currentPass = OBGEPASS_UNKNOWN;

	if (frame_log)
		frame_log->Message("OD3D9: IdlePass started");
//	else
//		_MESSAGE("OD3D9: IdlePass started");

	IdlePass(unk1, unk2);

	if (frame_log)
		frame_log->Message("OD3D9: IdlePass finished");
//	else
//		_MESSAGE("OD3D9: IdlePass finished");

	currentPass = previousPass;
}

/* ------------------------------------------------------------------------------------------------- */

void (__stdcall * GetRenderedSurfaceParameters)(v1_2_416::NiDX9Renderer *, enum SurfaceIDs SurfaceTypeID, int *pWidth, int *pHeight, int, int *, int *pFormat) =
	(void (__stdcall *)(v1_2_416::NiDX9Renderer *, enum SurfaceIDs SurfaceTypeID, int *pWidth, int *pHeight, int, int *, int *pFormat))0x007C0D10;


void __stdcall TrackRenderedSurfaceParameters(v1_2_416::NiDX9Renderer *renderer, enum SurfaceIDs SurfaceTypeID, int *pWidth, int *pHeight, int unk1, int *unk2, int *pFormat) {
//	assert(false);

	GetRenderedSurfaceParameters(renderer, SurfaceTypeID, pWidth, pHeight, unk1, unk2, pFormat);

#if 0	/* currently of no use ... maybe you find one :^) */
	const char *SurfaceTypeName = "unknown";
	switch (SurfaceTypeID) {
		case SURFACE_ID_HDR0: SurfaceTypeName = "HDR-BoxSample Surface"; break;
		case SURFACE_ID_HDR1: SurfaceTypeName = "HDR-PointSample Surface"; break;
		case SURFACE_ID_HDR2: SurfaceTypeName = "HDR-Intermediate Surface"; break;
		case SURFACE_ID_HDR3: SurfaceTypeName = "HDR-Origin Surface"; break;
		case SURFACE_ID_HDR4: SurfaceTypeName = "HDR-Destination Surface"; break;

		case SURFACE_ID_UNK5: break;

		case SURFACE_ID_WATER6: SurfaceTypeName = "Water[6] Surface"; break;
		case SURFACE_ID_WATER7: SurfaceTypeName = "Water[7] Surface"; break;
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

	_MESSAGE("OD3D9: Intercepted GetRenderedSurfaceParameters():");
	_MESSAGE("OD3D9: Intercepted Purpose: %s [0x%02x]", SurfaceTypeName, SurfaceTypeID);
	_MESSAGE("OD3D9: Intercepted Format-Flags: 0x%08x", *pFormat);
	_MESSAGE("OD3D9: Intercepted {W,H} before: {%d,%d}", *pWidth, *pHeight);

//	switch (SurfaceTypeID) {
//		case SURFACE_ID_REFL13: *pWidth = *pHeight = 1024; break;
//		case SURFACE_ID_REFL14: *pWidth = *pHeight = 1024; break;
//	}

	_MESSAGE("OD3D9: Intercepted {W,H} after: {%d,%d}", *pWidth, *pHeight);
#endif
}

/* ------------------------------------------------------------------------------------------------- */

void CreateRenderSurfaceHook(void) {
	/* combined passes: 0040C830 */
	/* combined reflection and water passes: 0049E880 */
	/* combined hdr related: 0049E880 */

	/* ReflectionPass */
	*((int *)&CombinerPass)          = 0x0040C830;
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
	/* GetRenderedSurfaceParameters */
	*((int *)&GetRenderedSurfaceParameters) = 0x007C0D10;

        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());

	DetourAttach(&(PVOID&)CombinerPass,          *((PVOID *)&TrackCombinerPass));
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
	DetourAttach(&(PVOID&)GetRenderedSurfaceParameters, TrackRenderedSurfaceParameters);
        LONG error = DetourTransactionCommit();

//	*((int *)&WaterSurfacePass) += 2;  // broken sub esp 8, prefixes 2 bytes before instructions

        if (error == NO_ERROR) {
		_MESSAGE("Detoured GetRenderedSurfaceParameters(); succeeded");
        }
        else {
		_MESSAGE("Detoured GetRenderedSurfaceParameters(); failed");
        }

	/* Reflection Render-Surface Dimension (square) */
	if ((ReflectionMapSize.data >= 256) &&
	    (ReflectionMapSize.data <= 2560)) {

		SafeWrite32(0x0049BFAF, ReflectionMapSize.data);
	}
	else if ((ReflectionMapSize.data == 0) &&
	         (RendererWidth.data >= 256) &&
	         (RendererWidth.data <= 2560)) {

		SafeWrite32(0x0049BFAF, RendererWidth.data);
	}

	return;
}
