#if	defined(OBGE_LOGGING) || defined(OBGE_DEVLING)
#pragma once

#include <map>
#include <string>

#include "D3D9.hpp"
#include "D3D9Device.hpp"

extern std::map <std::string, IDirect3DBaseTexture9 *> textureFiles;

const char *findTexture(IDirect3DBaseTexture9 *tex);

void CreateTextureIOHook(void);

#endif
