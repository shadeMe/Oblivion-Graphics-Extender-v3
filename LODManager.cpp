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

#ifndef	OBGE_NOSHADER

#include <sys/stat.h>
#define _USE_MATH_DEFINES
#include <math.h>

#include <string>

#include "ShaderManager.h"
#include "LODManager.h"
#include "TextureManager.h"
#include "MeshManager.h"
#include "GlobalSettings.h"

#include "D3D9.hpp"
#include "D3D9Device.hpp"

static global<int> GridsToLoad(5, "Oblivion.ini", "General", "uGridsToLoad");
static global<int> GridDistantCount(16, "Oblivion.ini", "General", "uGridDistantCount");
static global<bool> EnabledLOD(false, NULL, "General", "bEnabledLOD");
static global<int> FarNearTiles(5, NULL, "LOD", "iFarNearTiles");
static global<int> FarFarTiles(7, NULL, "LOD", "iFarFarTiles");
static global<int> FarInfTiles(11, NULL, "LOD", "iFarInfTiles");

/* #################################################################################################
 */

LODManager *LODManager::Singleton = NULL;

LODManager::LODManager() {
  if (!EnabledLOD.Get())
    return;

  WorldSpace = 0;

  memset(Meshes, 0, sizeof(Meshes));
  memset(Colors, 0, sizeof(Colors));
  memset(Normals, 0, sizeof(Normals));

  memset(MeshIDs, 0xFE, sizeof(MeshIDs));
  memset(ColrIDs, 0xFE, sizeof(ColrIDs));
  memset(NormIDs, 0xFE, sizeof(NormIDs));

  ShaderManager *sm = ShaderManager::GetSingleton();

  /* look-ups will always be done with the original shader-address */
  if ((vNear = sm->GetBuiltInShader("LODNEAR.vso"))) {
    vNear->GetBinary(); vNear->ConstructDX9Shader(SHADER_REPLACED);
    vShader[GRID_FARNEAR] = vNear->pDX9VertexShader; }
  if ((pNear = sm->GetBuiltInShader("LODNEAR.pso"))) {
    pNear->GetBinary(); pNear->ConstructDX9Shader(SHADER_REPLACED);
    pShader[GRID_FARNEAR] = pNear->pDX9PixelShader; }

  /* look-ups will always be done with the original shader-address */
  if ((vFar = sm->GetBuiltInShader("LODFAR.vso"))) {
    vFar->GetBinary(); vFar->ConstructDX9Shader(SHADER_REPLACED);
    vShader[GRID_FARFAR] = vFar->pDX9VertexShader; }
  if ((pFar = sm->GetBuiltInShader("LODFAR.pso"))) {
    pFar->GetBinary(); pFar->ConstructDX9Shader(SHADER_REPLACED);
    pShader[GRID_FARFAR] = pFar->pDX9PixelShader; }

  /* look-ups will always be done with the original shader-address */
  if ((vInf = sm->GetBuiltInShader("LODINF.vso"))) {
    vInf->GetBinary(); vInf->ConstructDX9Shader(SHADER_REPLACED);
    vShader[GRID_FARINF] = vInf->pDX9VertexShader; }
  if ((pInf = sm->GetBuiltInShader("LODINF.pso"))) {
    pInf->GetBinary(); pInf->ConstructDX9Shader(SHADER_REPLACED);
    pShader[GRID_FARINF] = pInf->pDX9PixelShader; }

  /* look-ups will always be done with the original shader-address */
  if ((vWater = sm->GetBuiltInShader("LODWATER.vso"))) {
    vWater->GetBinary(); vWater->ConstructDX9Shader(SHADER_REPLACED);
    vShaderW = vWater->pDX9VertexShader; }
  if ((pWater = sm->GetBuiltInShader("LODWATER.pso"))) {
    pWater->GetBinary(); pWater->ConstructDX9Shader(SHADER_REPLACED);
    pShaderW = pWater->pDX9PixelShader; }
}

LODManager::~LODManager() {
  Singleton = NULL;
}

void LODManager::Reset() {
  if (!EnabledLOD.Get())
    return;

  MeshManager *mm = MeshManager::GetSingleton();
  TextureManager *tm = TextureManager::GetSingleton();

  for (int l = 0; l < GRID_LODS; l++)
  for (int x = 0; x < GRID_SIZE; x++)
  for (int y = 0; y < GRID_SIZE; y++) {
    int mid = MeshIDs[l][y][x]; MeshIDs[l][y][x] = 0xFEFEFEFE;
    int cid = ColrIDs[l][y][x]; ColrIDs[l][y][x] = 0xFEFEFEFE;
    int nid = NormIDs[l][y][x]; NormIDs[l][y][x] = 0xFEFEFEFE;

    if (mid >= 0) mm->ReleaseMesh   (MeshIDs[l][y][x]);
    if (cid >= 0) tm->ReleaseTexture(ColrIDs[l][y][x]);
    if (nid >= 0) tm->ReleaseTexture(NormIDs[l][y][x]);

    Meshes [l][y][x] = NULL;
    Colors [l][y][x] = NULL;
    Normals[l][y][x] = NULL;
  }
}

LODManager *LODManager::GetSingleton() {
  if (LODManager::Singleton)
    return LODManager::Singleton;

  if (!LODManager::Singleton)
    LODManager::Singleton = new LODManager();

  return LODManager::Singleton;
}

void LODManager::InitializeBuffers() {
  void *VertexPointer;

  WaterTile ShaderVerticesT[] = {
    {       0,        0, 0},
    {TILE_DIM,        0, 0},
    {       0, TILE_DIM, 0},
    {TILE_DIM, TILE_DIM, 0}
  };

  WaterTile ShaderVerticesI[] = {
    {-TILE_DIM * 32 * 16, -TILE_DIM * 32 * 16, -512.0f},
    { TILE_DIM * 32 * 16, -TILE_DIM * 32 * 16, -512.0f},
    {-TILE_DIM * 32 * 16,  TILE_DIM * 32 * 16, -512.0f},
    { TILE_DIM * 32 * 16,  TILE_DIM * 32 * 16, -512.0f}
  };

  _MESSAGE("Creating lod vertex buffers.");

  if (lastOBGEDirect3DDevice9->CreateVertexBuffer(4 * sizeof(WaterTile), D3DUSAGE_WRITEONLY, WATERTILEFORMAT, D3DPOOL_DEFAULT, &WaterVertex, 0) != D3D_OK) {
    _MESSAGE("ERROR - Unable to create the vertex buffer!");
    exit(0); return;
  }

  WaterVertex->Lock(0, 0, &VertexPointer, 0);
  CopyMemory(VertexPointer, ShaderVerticesT, sizeof(ShaderVerticesT));
  WaterVertex->Unlock();

  if (lastOBGEDirect3DDevice9->CreateVertexBuffer(4 * sizeof(WaterTile), D3DUSAGE_WRITEONLY, WATERTILEFORMAT, D3DPOOL_DEFAULT, &InfiniteVertex, 0) != D3D_OK) {
    _MESSAGE("ERROR - Unable to create the vertex buffer!");
    exit(0); return;
  }

  InfiniteVertex->Lock(0, 0, &VertexPointer, 0);
  CopyMemory(VertexPointer, ShaderVerticesI, sizeof(ShaderVerticesI));
  InfiniteVertex->Unlock();
}

void LODManager::ReleaseBuffers() {
  if (WaterVertex || InfiniteVertex)
    _MESSAGE("Releasing lod vertex buffer.");

  if (WaterVertex) {
    while (WaterVertex->Release()) {}
    WaterVertex = NULL;
  }

  if (InfiniteVertex) {
    while (InfiniteVertex->Release()) {}
    InfiniteVertex = NULL;
  }
}

void LODManager::OnCreateDevice() {
}

void LODManager::OnReleaseDevice() {
  ReleaseBuffers();
  Reset();
}

void LODManager::OnLostDevice() {
  ReleaseBuffers();
}

void LODManager::OnResetDevice() {
  ReleaseBuffers();
  InitializeBuffers();
}

void LODManager::PurgeTexture(IDirect3DBaseTexture9 *texture, int TexNum) {
  if (!EnabledLOD.Get())
    return;

  MeshManager *mm = MeshManager::GetSingleton();
  TextureManager *tm = TextureManager::GetSingleton();

  for (int l = 0; l < GRID_LODS; l++)
  for (int x = 0; x < GRID_SIZE; x++)
  for (int y = 0; y < GRID_SIZE; y++) {
    if ((ColrIDs[l][y][x] == TexNum) ||
        (NormIDs[l][y][x] == TexNum)) {
      int mid = MeshIDs[l][y][x]; MeshIDs[l][y][x] = 0xFEFEFEFE;
      int cid = ColrIDs[l][y][x]; ColrIDs[l][y][x] = 0xFEFEFEFE;
      int nid = NormIDs[l][y][x]; NormIDs[l][y][x] = 0xFEFEFEFE;

      if (mid >=      0) mm->ReleaseMesh   (MeshIDs[l][y][x]);
      if (cid != TexNum) tm->ReleaseTexture(ColrIDs[l][y][x]);
      if (nid != TexNum) tm->ReleaseTexture(NormIDs[l][y][x]);

      Meshes [l][y][x] = NULL;
      Colors [l][y][x] = NULL;
      Normals[l][y][x] = NULL;
    }
  }
}

void LODManager::PurgeTexture(IDirect3DTexture9 *texture, int TexNum) {
  PurgeTexture((IDirect3DBaseTexture9 *)texture, TexNum);
}

void LODManager::PurgeTexture(IDirect3DCubeTexture9 *texture, int TexNum) {
}

void LODManager::PurgeTexture(IDirect3DVolumeTexture9 *texture, int TexNum) {
}

void LODManager::PurgeMesh(ID3DXBaseMesh *mesh, int MeshNum) {
  if (!EnabledLOD.Get())
    return;

  MeshManager *mm = MeshManager::GetSingleton();
  TextureManager *tm = TextureManager::GetSingleton();

  for (int l = 0; l < GRID_LODS; l++)
  for (int x = 0; x < GRID_SIZE; x++)
  for (int y = 0; y < GRID_SIZE; y++) {
    if ((MeshIDs[l][y][x] == MeshNum)) {
      int mid = MeshIDs[l][y][x]; MeshIDs[l][y][x] = 0xFEFEFEFE;
      int cid = ColrIDs[l][y][x]; ColrIDs[l][y][x] = 0xFEFEFEFE;
      int nid = NormIDs[l][y][x]; NormIDs[l][y][x] = 0xFEFEFEFE;

      if (mid != MeshNum) mm->ReleaseMesh   (MeshIDs[l][y][x]);
      if (cid >=       0) tm->ReleaseTexture(ColrIDs[l][y][x]);
      if (nid >=       0) tm->ReleaseTexture(NormIDs[l][y][x]);

      Meshes [l][y][x] = NULL;
      Colors [l][y][x] = NULL;
      Normals[l][y][x] = NULL;
    }
  }
}

/* -------------------------------------------------------------------------------------------------
 */

template<int inner, int extend, int lod>
void LODManager::Render(IDirect3DDevice9 *D3DDevice) {
  const char *meshpath =
    (lod == GRID_FARNEAR ? "landscape\\lod\\farnear\\" :
    (lod == GRID_FARFAR  ? "landscape\\lod\\farfar\\" :
			   "landscape\\lod\\farinf\\"));
  const char *textpath =
    (lod == GRID_FARNEAR ? "landscapelod\\generated\\farnear\\" :
    (lod == GRID_FARFAR  ? "landscapelod\\generated\\farfar\\" :
			   "landscapelod\\generated\\farinf\\"));

  int nativeminx = (GRID_SIZE * 32) + (Constants.Coordinates.x - GridDistantCount.Get());
  int nativeminy = (GRID_SIZE * 32) + (Constants.Coordinates.y - GridDistantCount.Get());
  int nativemaxx = (GRID_SIZE * 32) + (Constants.Coordinates.x + GridDistantCount.Get());
  int nativemaxy = (GRID_SIZE * 32) + (Constants.Coordinates.y + GridDistantCount.Get());

  /* y-axis has flipped rounding */
  nativeminx = (nativeminx / 32) - GRID_SIZE;
  nativeminy = (nativeminy / 32) - GRID_SIZE + 0;
  nativemaxx = (nativemaxx / 32) - GRID_SIZE;
  nativemaxy = (nativemaxy / 32) - GRID_SIZE + 0;

  int gridx = Constants.Coordinates.x / 32;
  int gridy = Constants.Coordinates.y / 32;
  for (int x = (gridx - extend); x <= (gridx + extend); x++)
  for (int y = (gridy - extend); y <= (gridy + extend); y++) {
    /* TODO: try radius, seems it's not a box */
    /* leave out Oblivion's native tiles */
    if ((x >= nativeminx) && (x <= nativemaxx) &&
	(y >= nativeminy) && (y <= nativemaxy))
      continue;
    /* leave out other LOD's inner tiles */
    if ((abs(gridx - x) <= inner) &&
	(abs(gridy - y) <= inner))
      continue;

    /* where are we? */
    const float TileOffset[4] = {x * TILE_DIM, y * TILE_DIM, 0, 0};

    /* filter outside-array coordinates */
    if (((GRID_OFFSET + y) >= 0) && ((GRID_OFFSET + y) < GRID_SIZE) &&
	((GRID_OFFSET + x) >= 0) && ((GRID_OFFSET + x) < GRID_SIZE)) {

      /* never seen, never attempted */
      if (MeshIDs[lod][GRID_OFFSET + y][GRID_OFFSET + x] < -1) {
	/* TODO: 32 means 32x32 cells, in theory that can be different as well */
	char buf[256]; sprintf(buf, "%02d.%02d.%02d.32", WorldSpace, x * 32, y * 32);
	char pth[256]; strcpy(pth, meshpath); strcat(pth, buf); strcat(pth, ".x");

	/* no textures without mesh, but we can render texture-free */
	if ((MeshIDs[lod][GRID_OFFSET + y][GRID_OFFSET + x] =
	    MeshManager::GetSingleton()->LoadPrivateMesh(pth, MR_REGULAR)) != -1) {

	  if (ColrIDs[lod][GRID_OFFSET + y][GRID_OFFSET + x] < -1) {
	    strcpy(pth, textpath); strcat(pth, buf); strcat(pth, ".dds");
	    ColrIDs[lod][GRID_OFFSET + y][GRID_OFFSET + x] =
	      TextureManager::GetSingleton()->LoadPrivateTexture(pth, TR_PLANAR);
	  }

	  if (NormIDs[lod][GRID_OFFSET + y][GRID_OFFSET + x] < -1) {
	    strcpy(pth, textpath); strcat(pth, buf); strcat(pth, "_fn.dds");
	    NormIDs[lod][GRID_OFFSET + y][GRID_OFFSET + x] =
	      TextureManager::GetSingleton()->LoadPrivateTexture(pth, TR_PLANAR);
	  }

	  /* put the addresses */
	  ManagedMeshRecord    *mesh = Meshes [lod][GRID_OFFSET + y][GRID_OFFSET + x] =    MeshManager::GetSingleton()->GetMesh   (MeshIDs[lod][GRID_OFFSET + y][GRID_OFFSET + x]);
	  ManagedTextureRecord *colr = Colors [lod][GRID_OFFSET + y][GRID_OFFSET + x] = TextureManager::GetSingleton()->GetTexture(ColrIDs[lod][GRID_OFFSET + y][GRID_OFFSET + x]);
	  ManagedTextureRecord *norm = Normals[lod][GRID_OFFSET + y][GRID_OFFSET + x] = TextureManager::GetSingleton()->GetTexture(NormIDs[lod][GRID_OFFSET + y][GRID_OFFSET + x]);

	  /* failure to load all resources */
	  if (!mesh || !colr || !norm) {
	    if (mesh) mesh->Release();
	    if (colr) colr->Release();
	    if (norm) norm->Release();

	    MeshIDs[lod][GRID_OFFSET + y][GRID_OFFSET + x] = -1;
	    ColrIDs[lod][GRID_OFFSET + y][GRID_OFFSET + x] = -1;
	    NormIDs[lod][GRID_OFFSET + y][GRID_OFFSET + x] = -1;

	    continue;
	  }

#if	defined(OBGE_GAMMACORRECTION)
	  /* remember DeGamma for this kind of texture */
	  static const bool PotDeGamma = true;
	  colr->GetTexture()->SetPrivateData(GammaGUID, &PotDeGamma, sizeof(PotDeGamma), 0);
#endif
	}
      }

      /* get the addresses */
      ManagedMeshRecord    *mesh = Meshes [lod][GRID_OFFSET + y][GRID_OFFSET + x];
      ManagedTextureRecord *colr = Colors [lod][GRID_OFFSET + y][GRID_OFFSET + x];
      ManagedTextureRecord *norm = Normals[lod][GRID_OFFSET + y][GRID_OFFSET + x];

      ID3DXMesh *m;
      if (mesh && (m = (ID3DXMesh *)mesh->GetMesh())) {
#if 0
	DWORD FVF  = m->GetFVF();
	DWORD size = m->GetNumBytesPerVertex();
	DWORD numf = m->GetNumFaces();
	DWORD numv = m->GetNumVertices();

	IDirect3DIndexBuffer9 *pIB; m->GetIndexBuffer(&pIB);
	IDirect3DVertexBuffer9 *pVB; m->GetVertexBuffer(&pVB);

	D3DDevice->SetStreamSource(0, pVB, 0, size);
	D3DDevice->SetFVF(FVF);
	D3DDevice->SetTexture(0, colr->GetTexture());
	D3DDevice->SetTexture(1, norm->GetTexture());
	D3DDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, numv, 0, numf);
#endif

	D3DDevice->SetTexture(0, colr ? colr->GetTexture() : NULL);
	D3DDevice->SetTexture(1, norm ? norm->GetTexture() : NULL);

	D3DDevice->SetVertexShader(vShader[lod]);
	D3DDevice->SetPixelShader (pShader[lod]);
	D3DDevice->SetVertexShaderConstantF(32, TileOffset, 1);

	m->DrawSubset(0);
      }
    }

    /* water-planes */
    D3DDevice->SetVertexShader(vShaderW);
    D3DDevice->SetPixelShader (pShaderW);
    D3DDevice->SetVertexShaderConstantF(32, TileOffset, 1);

    D3DDevice->SetStreamSource(0, WaterVertex, 0, sizeof(WaterTile));
    D3DDevice->SetFVF(WATERTILEFORMAT);
    D3DDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
  }

  const float TileOffset[4] = {0, 0, 0, 1};

  /* infini-plane */
  D3DDevice->SetVertexShader(vShaderW);
  D3DDevice->SetPixelShader (pShaderW);
  D3DDevice->SetVertexShaderConstantF(32, TileOffset, 1);

  D3DDevice->SetStreamSource(0, InfiniteVertex, 0, sizeof(WaterTile));
  D3DDevice->SetFVF(WATERTILEFORMAT);
  D3DDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
}

void LODManager::Render(IDirect3DDevice9 *D3DDevice, IDirect3DSurface9 *RenderTo, IDirect3DSurface9 *RenderFrom) {
  if (EnabledLOD.Get() && Constants.Exteriour && !Constants.Oblivion) {
    /* we switched worldspace */
    if (WorldSpace != Constants.WorldSpace) {
      Reset();

      /* mark this to be the current worldspace */
      WorldSpace = Constants.WorldSpace;
    }

#if 1 //def	OBGE_STATEBLOCKS
    /* auto backup (not strictly needed, all states can be changed) */
    IDirect3DStateBlock9 *pStateBlock = NULL;
    D3DDevice->CreateStateBlock(D3DSBT_ALL, &pStateBlock);
#endif

    if ((currentPass == OBGEPASS_MAIN) && 0)
      markerReset(D3DDevice);

    /* my settings are:
     * near 10.0
     * far  283408....	^= sqrt(2 * ((half-tile + full-tile) ^ 2))
     *			^= pytagoras of 1.5 tiles radius or 3 tile diameter
     */
 // D3DDevice->SetTransform(D3DTS_PROJECTION, &Constants.proj);
 // D3DDevice->SetTransform(D3DTS_VIEW, &Constants.view);
 // D3DDevice->SetTransform(D3DTS_WORLD, &Constants.wrld);

    D3DDevice->SetRenderState(D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_ALPHA | D3DCOLORWRITEENABLE_BLUE | D3DCOLORWRITEENABLE_GREEN | D3DCOLORWRITEENABLE_RED);
//  D3DDevice->SetRenderState(D3DRS_ALPHATESTENABLE, false);
    D3DDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, false);
//  D3DDevice->SetRenderState(D3DRS_STENCILENABLE, false);
    D3DDevice->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
    D3DDevice->SetRenderState(D3DRS_ZENABLE, D3DZB_TRUE);
//  D3DDevice->SetRenderState(D3DRS_ZFUNC, D3DCMP_LESS);	// works much better
    D3DDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW);	// because geometry is CCW
  //D3DDevice->SetRenderState(D3DRS_CLIPPING, FALSE);

    /* don't change anything in the reflection-pass */
    if (currentPass == OBGEPASS_MAIN) {
      D3DDevice->SetTexture(2, passTexture[OBGEPASS_REFLECTION]);

      /* write to the incoming surface */
      D3DDevice->SetRenderTarget(0, RenderFrom);
      D3DDevice->SetDepthStencilSurface(GetDepthBufferSurface());
//    D3DDevice->SetDepthStencilSurface(GetStencilSurface());
    }

#define	INNER	(1 >> 1)
#define	FARNEAR	(5 >> 1)
#define	FARFAR	(7 >> 1)
#define	FARINF	(25 >> 1)
    Render<INNER,   FARNEAR, GRID_FARNEAR>(D3DDevice);
    Render<FARNEAR, FARFAR,  GRID_FARFAR >(D3DDevice);
    Render<FARFAR,  FARINF,  GRID_FARINF >(D3DDevice);

 //   D3DDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, true);	// menus come after
 //   D3DDevice->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
//  D3DDevice->SetRenderState(D3DRS_ZFUNC, D3DCMP_LESSEQUAL);

    if ((currentPass == OBGEPASS_MAIN) && 0)
      markerReset(D3DDevice);

    /* don't change anything in the reflection-pass */
    if (currentPass == OBGEPASS_MAIN) {
      D3DDevice->SetRenderTarget(0, RenderTo);
//    D3DDevice->SetDepthStencilSurface(NULL);
    }

#if 1 //def	OBGE_STATEBLOCKS
    /* auto restore (not strictly needed, all states can be changed) */
    pStateBlock->Apply();
    pStateBlock->Release();
#endif
  }
}

void LODManager::DoLOD(bool enable) {
  ::EnabledLOD.Set(enable);
}

bool LODManager::DoLOD() {
  return ::EnabledLOD.Get();
}
#else
#include "LODManager.h"

LODManager *LODManager::Singleton = NULL;

LODManager::LODManager() {
}

LODManager::~LODManager() {
  Singleton = NULL;
}

LODManager *LODManager::GetSingleton() {
  if (LODManager::Singleton)
    return LODManager::Singleton;

  if (!LODManager::Singleton)
    LODManager::Singleton = new LODManager();

  return LODManager::Singleton;
}
#endif
