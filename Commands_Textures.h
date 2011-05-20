#pragma once

#include "CommandTable.h"

extern CommandInfo kCommandInfo_LoadTexture;
extern CommandInfo kCommandInfo_LoadCubeTexture;
extern CommandInfo kCommandInfo_LoadVolumeTexture;
extern CommandInfo kCommandInfo_ReleaseTexture;
extern CommandInfo kCommandInfo_PurgeManagedTextures;

/* deprecated */
#ifndef	NO_DEPRECATED
extern CommandInfo kCommandInfo_FreeTexture;
#endif
