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

#include <vector>

#define D3DXFX_LARGEADDRESS_HANDLE
#include <d3dx9.h>

#include "obse\PluginAPI.h"
#include "nodes\NiDX9Renderer.h"

#include "DepthBufferHook.h"
#include "Rendering.h"

#include "D3D9Identifiers.hpp"

#ifndef	NO_DEPRECATED
#define MESHVERSION 1
#else
#define MESHVERSION 2
#endif

typedef enum {
  MR_REGULAR = 0,
  MR_PATCH = 1,
  MR_NPATCH = 2,
  MR_PROGRESSIVE = 3
} MeshRecordType;

class MeshRecord
{
public:
	MeshRecord();
	~MeshRecord();

	bool LoadMesh(MeshRecordType type, const char *fp, bool Private = false);

	void SetMesh(ID3DXMesh* tex, const char *fp, bool Private = false);
	void SetMesh(ID3DXPatchMesh* tex, const char *fp, bool Private = false);

	ID3DXBaseMesh* GetMesh() const;

	bool HasMesh(char *path) const;
	bool HasMesh(ID3DXBaseMesh* tex) const;
	bool HasMesh(ID3DXMesh* tex) const;
	bool HasMesh(ID3DXPatchMesh* tex) const;
	bool HasMesh() const;

	MeshRecordType GetType() const;
	bool IsType(MeshRecordType type) const;

	const char *GetPath() const;
	bool IsPrivate() const;

public:
	void Purge(int MeshNum = -1);
	void Kill();

private:
	char				Filepath[MAX_PATH];
	bool				Private;
	UINT32				ParentRefID;
	// Associates a effect with the esp/esm file the script the effect was created in.

	MeshRecordType		type;
	union {
	  ID3DXBaseMesh*	mesh;
	  ID3DXMesh*		meshR;
	  ID3DXPatchMesh*	meshP;
	};

	LPD3DXBUFFER pAdjacency;
	LPD3DXBUFFER pMaterials;
	LPD3DXBUFFER pEffectInstances;
	DWORD NumMaterials;
/*
	DWORD			FVF;
	LPDIRECT3DINDEXBUFFER9	IB;
	LPDIRECT3DINDEXBUFFER9	VB;
*/
};

class ManagedMeshRecord : public MeshRecord
{
public:
	ManagedMeshRecord();
	~ManagedMeshRecord();

	void ClrRef();
	int AddRef();
	int Release();
	int RefCount;
};

//pedef std::vector<MeshRecord *> MeshList;
typedef std::vector<ManagedMeshRecord *> ManagedMeshList;
typedef std::map<int, ManagedMeshRecord *> MeshRegistry;

class MeshManager
{
public:
	MeshManager();
	~MeshManager();

	static MeshManager *GetSingleton();
	static MeshManager *Singleton;

	void						NewGame(void);
	void						LoadGame(OBSESerializationInterface *Interface);
	void						SaveGame(OBSESerializationInterface *Interface);

private:
	void						Clear();

public:
	int						LoadPrivateMesh(const char *Filename, MeshRecordType type);
	int						LoadManagedMesh(const char *Filename, MeshRecordType type);
	int						LoadDependtMesh(const char *Filename, MeshRecordType type);
	inline bool					IsMeshValid(int MeshNum) const { return Meshes.count(MeshNum) != 0; };
	inline ManagedMeshRecord *			GetMesh(int MeshNum) { return (IsMeshValid(MeshNum) ? Meshes[MeshNum] : NULL); };
	bool						ReleaseMesh(int MeshNum);
	void						FreeMesh(int MeshNum);
	template<class ID3DXMeshType> bool		ReleaseMesh(ID3DXMeshType *mesh);
	template<class ID3DXMeshType> int		FindMesh(ID3DXMeshType *mesh);

private:
	int						MeshIndex;
	int						MaxMeshIndex;

	MeshRegistry					Meshes;
//	EffectList					Meshes;
	ManagedMeshList					ManagedMeshes;

public:
};
