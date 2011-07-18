#pragma once
#ifndef	OBGE_NOSHADER

#include <d3d9.h>

bool TextureCompressRGBH(LPDIRECT3DTEXTURE9 *base, bool gama);
bool TextureCompressRGB(LPDIRECT3DTEXTURE9 *base, bool gama);
bool TextureCompressLA(LPDIRECT3DTEXTURE9 *alpha);
bool TextureCompressA(LPDIRECT3DTEXTURE9 *alpha);
bool TextureCompressXYZD(LPDIRECT3DTEXTURE9 *norm);
bool TextureCompressXY_Z(LPDIRECT3DTEXTURE9 *norm, LPDIRECT3DTEXTURE9 *z);
bool TextureCompressXYZ(LPDIRECT3DTEXTURE9 *norm);
bool TextureCompressXY(LPDIRECT3DTEXTURE9 *norm);
bool TextureCompressPM(LPDIRECT3DTEXTURE9 *base, LPDIRECT3DTEXTURE9 *norm, bool gama);
bool TextureCompressQDM(LPDIRECT3DTEXTURE9 *base, LPDIRECT3DTEXTURE9 *norm, bool gama, bool LODed);

#endif
