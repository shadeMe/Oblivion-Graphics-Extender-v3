/* Version: MPL 1.1/LGPL 3.0
 *
 * "The contents of this file are subject to the Mozilla Public License
 * Version 1.1 (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS"
 * basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
 * License for the specific language governing rights and limitations
 * under the License.
 *
 * The Original Code is the Oblivion Graphics Extender, short OBGE.
 *
 * The Initial Developer of the Original Code is
 * Ethatron <niels@paradice-insight.us>. Portions created by The Initial
 * Developer are Copyright (C) 2011 The Initial Developer.
 * All Rights Reserved.
 *
 * Contributor(s):
 *  Timeslip (Version 1)
 *  scanti (Version 2)
 *  IlmrynAkios (Version 3)
 *
 * Alternatively, the contents of this file may be used under the terms
 * of the GNU Library General Public License Version 3 license (the
 * "LGPL License"), in which case the provisions of LGPL License are
 * applicable instead of those above. If you wish to allow use of your
 * version of this file only under the terms of the LGPL License and not
 * to allow others to use your version of this file under the MPL,
 * indicate your decision by deleting the provisions above and replace
 * them with the notice and other provisions required by the LGPL License.
 * If you do not delete the provisions above, a recipient may use your
 * version of this file under either the MPL or the LGPL License."
 */

#pragma once

#define D3DXFX_LARGEADDRESS_HANDLE
#include <d3dx9.h>
#include <d3dx9shader.h>

#ifndef	OBGE_NOSHADER

#include <vector>

#include "TextureManager.h"
#include "MeshManager.h"

#define	TILE_DIM	131072.0f
#define	GRID_SIZE	16
#define	GRID_OFFSET	8
typedef ManagedMeshRecord *MeshGrid[GRID_SIZE][GRID_SIZE];
typedef ManagedTextureRecord *TextGrid[GRID_SIZE][GRID_SIZE];
typedef int IDCache[GRID_SIZE][GRID_SIZE];

#define	GRID_FARNEAR	0
#define	GRID_FARFAR	1
#define	GRID_FARINF	2
#define	GRID_LODS	3

class LODManager {

public:
	LODManager();
	~LODManager();

	static LODManager*		GetSingleton(void);
	static LODManager*		Singleton;

	void						OnCreateDevice(void);
	void						OnLostDevice(void);
	void						OnResetDevice(void);
	void						OnReleaseDevice(void);

	void						InitializeBuffers();
	void						ReleaseBuffers();

	bool						ChangedShaders();
	bool						ReloadShaders();
private:
	void						Reset();

//	template<int inner = INNER, int extend = FARNEAR, int lod = GRID_FARNEAR>
	template<int inner, int extend, int lod>
	void						Render(IDirect3DDevice9 *D3DDevice);
public:
	void						Render(IDirect3DDevice9 *Device, IDirect3DSurface9 *RenderTo, IDirect3DSurface9 *RenderFrom);

	void						PurgeTexture(IDirect3DBaseTexture9 *texture, int TexNum = -1);
	void						PurgeTexture(IDirect3DTexture9 *texture, int TexNum = -1);
	void						PurgeTexture(IDirect3DCubeTexture9 *texture, int TexNum = -1);
	void						PurgeTexture(IDirect3DVolumeTexture9 *texture, int TexNum = -1);
	void						PurgeMesh(ID3DXBaseMesh *mesh, int MeshNum = -1);

private:
	ShaderRecord					*vNear, *vFar, *vInf, *vWater;
	ShaderRecord					*pNear, *pFar, *pInf, *pWater;

	IDirect3DVertexShader9				*vShader[GRID_LODS], *vShaderW;
	IDirect3DPixelShader9				*pShader[GRID_LODS], *pShaderW;

	int						WorldSpace;

	struct WaterTile { float x,y,z; };

#define WATERTILEFORMAT D3DFVF_XYZ

	IDirect3DVertexBuffer9 *			WaterVertex;
	IDirect3DVertexBuffer9 *			InfiniteVertex;

	MeshGrid					Meshes[GRID_LODS];
	TextGrid					Colors[GRID_LODS];
	TextGrid					Normals[GRID_LODS];

	IDCache						MeshIDs[GRID_LODS];
	IDCache						ColrIDs[GRID_LODS];
	IDCache						NormIDs[GRID_LODS];

public:
	void						DoLOD(bool enable);
	bool						DoLOD();
};

#else
class LODManager {

public:
	LODManager();
	~LODManager();

	static LODManager*		GetSingleton(void);
	static LODManager*		Singleton;

	static void					Render(IDirect3DDevice9 *Device, IDirect3DSurface9 *RenderTo, IDirect3DSurface9 *RenderFrom) {};
	static void					OnCreateDevice(void) {};
	static void					OnLostDevice(void) {};
	static void					OnResetDevice(void) {};
	static void					OnReleaseDevice(void) {};
	static void					InitializeBuffers(void) {};
	static void					ReleaseBuffers(void) {};
	static void					PurgeTexture(IDirect3DBaseTexture9 *texture, int TexNum = -1) {};
	static void					PurgeMesh(ID3DXBaseMesh *mesh, int MeshNum = -1) {};
};
#endif
