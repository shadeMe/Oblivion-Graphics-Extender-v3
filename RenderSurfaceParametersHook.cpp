#include "D3D9.hpp"
#include "RenderSurfaceParametersHook.hpp"
#include "windows.h"
#include "obse_common/SafeWrite.h"
#include <assert.h>

#include "detours/detours.h"

/* ------------------------------------------------------------------------------------------------- */

class Anonymous {
public:
	void TrackReflectionPass(int unk1, int unk2);
};

void (__thiscall Anonymous::* ReflectionPass)(int, int)/* =
	(void (__thiscall TES::*)(int, int))0x0049BEF0*/;
void (__thiscall Anonymous::* TrackReflectionPass)(int, int)/* =
	(void (__thiscall TES::*)(int, int))0x0049BEF0*/;

void Anonymous::TrackReflectionPass(int unk1, int unk2) {
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

	currentPass = OBGEPASS_UNKNOWN;
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
  /* ReflectionPass */
	*((int *)&ReflectionPass) = 0x0049BEF0;
	TrackReflectionPass = &Anonymous::TrackReflectionPass;
	/* GetRenderedSurfaceParameters */
	*((int *)&GetRenderedSurfaceParameters) = 0x007C0D10;

        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());
	DetourAttach(&(PVOID&)ReflectionPass, *((PVOID *)&TrackReflectionPass));
	DetourAttach(&(PVOID&)GetRenderedSurfaceParameters, TrackRenderedSurfaceParameters);
        LONG error = DetourTransactionCommit();

        if (error == NO_ERROR) {
		_MESSAGE("Detoured GetRenderedSurfaceParameters(); succeeded");
        }
        else {
		_MESSAGE("Detoured GetRenderedSurfaceParameters(); failed");
        }

	/* Reflection Render-Surface Dimension (square) */
	SafeWrite32(0x0049BFAF, 1400);

	return;
}
