#if	defined(OBGE_LOGGING) || defined(OBGE_DEVLING) || defined(OBGE_GAMMACORRECTION)
#pragma once

#include <map>
#include <string>

#include "D3D9.hpp"
#include "D3D9Device.hpp"

#if	defined(OBGE_LOGGING) || defined(OBGE_DEVLING)
extern std::map <std::string, IDirect3DBaseTexture9 *> textureFiles;

const char *findTexture(IDirect3DBaseTexture9 *tex);
#endif

void CreateTextureIOHook(void);

#endif
