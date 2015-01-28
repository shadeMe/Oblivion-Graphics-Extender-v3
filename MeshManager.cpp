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

#include <sys/stat.h>

#include <algorithm>

#include "MeshManager.h"
#include "ShaderManager.h"
#include "EffectManager.h"
#include "LODManager.h"
#include "ScreenElements.h"
#include "obse\pluginapi.h"
#include "GlobalSettings.h"

#include "D3D9.hpp"
#include "D3D9Device.hpp"

static global<bool> PurgeOnNewGame(true, NULL, "Meshes", "bPurgeOnNewGame");

// *********************************************************************************************************

MeshRecord::MeshRecord() {
  type = MR_REGULAR;
  mesh = NULL;
  Filepath[0] = 0;
  Private = false;
}

MeshRecord::~MeshRecord() {
  Kill();
}

bool MeshRecord::LoadMesh(MeshRecordType type, const char *fp, bool Private) {
  HRESULT res;

  if (type == MR_REGULAR) {
    ID3DXMesh *mesh = NULL;

    if (FAILED(res = D3DXLoadMeshFromX(
                 fp,
//               D3DXMESH_DONOTCLIP |
                 D3DXMESH_VB_MANAGED |
                 D3DXMESH_IB_MANAGED |
                 D3DXMESH_MANAGED,
                 lastOBGEDirect3DDevice9,
                 &pAdjacency,
                 &pMaterials,
                 &pEffectInstances,
                 &NumMaterials,
                 &mesh)) || !mesh)
      return false;

    SetMesh(mesh, fp, Private);
  }

  return true;
}

void MeshRecord::SetMesh(ID3DXMesh *mesh, const char *fp, bool Private) {
  this->type = MR_REGULAR;
  this->meshR = mesh;

  strcpy_s(this->Filepath, 256, fp);

  this->Private = Private;
/*
  this->FVF = mesh->GetFVF();
  mesh->GetIndexBuffer(&IB);
  mesh->GetVertexBuffer(&VB);
*/
}

void MeshRecord::SetMesh(ID3DXPatchMesh *mesh, const char *fp, bool Private) {
  this->type = MR_PATCH;
  this->meshP = mesh;

  strcpy_s(this->Filepath, 256, fp);

  this->Private = Private;
/*
  this->FVF = mesh->GetFVF();
  mesh->GetIndexBuffer(&IB);
  mesh->GetVertexBuffer(&VB);
*/
}

ID3DXBaseMesh *MeshRecord::GetMesh() const {
  return this->mesh;
}

MeshRecordType MeshRecord::GetType() const {
  return this->type;
}

bool MeshRecord::IsType(MeshRecordType type) const {
  return (this->type == type);
}

bool MeshRecord::HasMesh(char *path) const {
  return !strcmp(this->Filepath, path);
}

bool MeshRecord::HasMesh(ID3DXBaseMesh *mesh) const {
  return (this->mesh == mesh);
}

bool MeshRecord::HasMesh(ID3DXMesh *mesh) const {
  return this->IsType(MR_REGULAR) && (this->meshR == mesh);
}

bool MeshRecord::HasMesh(ID3DXPatchMesh *mesh) const {
  return this->IsType(MR_PATCH) && (this->meshP == mesh);
}

bool MeshRecord::HasMesh() const {
  return (this->mesh != NULL);
}

const char *MeshRecord::GetPath() const {
  return this->Filepath;
}

bool MeshRecord::IsPrivate() const {
  return this->Private;
}

void MeshRecord::Purge(int MeshNum) {
  if (this->IsType(MR_REGULAR)) {
    LODManager::GetSingleton()->PurgeMesh(this->meshR, MeshNum);
  }
  else if (this->IsType(MR_PATCH)) {
//  LODManager::GetSingleton()->PurgeMesh(this->meshP, MeshNum);
  }
}

void MeshRecord::Kill() {
  if (this->IsType(MR_REGULAR)) {
    if (meshR)
      while (meshR->Release()) {}
  }
  else if (this->IsType(MR_PATCH)) {
    if (meshP)
      while (meshP->Release()) {}
  }

  this->mesh = NULL;
  this->Filepath[0] = '\0';
}

// *********************************************************************************************************

ManagedMeshRecord::ManagedMeshRecord() {
  RefCount = 0;
}

ManagedMeshRecord::~ManagedMeshRecord() {
}

void ManagedMeshRecord::ClrRef() {
  RefCount = 0;
}

int ManagedMeshRecord::AddRef() {
  return ++RefCount;
}

int ManagedMeshRecord::Release() {
  if (RefCount)
    RefCount--;

  if (!RefCount) {
//  delete this;
    return 0;
  }

  return RefCount;
}

// *********************************************************************************************************

MeshManager::MeshManager() {
  MeshIndex = 0;
  MaxMeshIndex = 0;
}

MeshManager::~MeshManager() {
  Singleton = NULL;
}

MeshManager *MeshManager::Singleton = NULL;

MeshManager *MeshManager::GetSingleton() {
  if (!Singleton)
    Singleton = new MeshManager();

  return Singleton;
}

void MeshManager::Clear() {
  ManagedMeshes.clear();
  Meshes.clear();

  MeshIndex = 0;
  MaxMeshIndex = 0;
}

int MeshManager::LoadPrivateMesh(const char *Filename, MeshRecordType type) {
  if (strlen(Filename) > 240)
    return -1;

  char NewPath[256];
  strcpy_s(NewPath, 256, "data\\meshes\\");
  strcat_s(NewPath, 256, Filename);

  ManagedMeshRecord *NewMesh = new ManagedMeshRecord();
  if (!NewMesh->LoadMesh(type, NewPath, true)) {
    delete NewMesh;
    return -1;
  }

  _MESSAGE("Loading mesh (%s)", NewPath);

  /* prepare */
  NewMesh->AddRef();

  /* append and sort */
  ManagedMeshes.push_back(NewMesh);

  /* register */
  Meshes[MeshIndex++] = NewMesh;

  return MeshIndex - 1;
}

int MeshManager::LoadManagedMesh(const char *Filename, MeshRecordType type) {
  if (strlen(Filename) > 240)
    return NULL;

  char NewPath[256];
  strcpy_s(NewPath, 256, "data\\meshes\\");
  strcat_s(NewPath, 256, Filename);

  MeshRegistry::iterator Mesh = Meshes.begin();

  /* search for non-private mesh */
  while (Mesh != Meshes.end()) {
    if (!Mesh->second->IsPrivate()) {
      if (!_stricmp(NewPath, Mesh->second->GetPath())/*&& ((Effect->second->ParentRefID & 0xff000000) == (refID & 0xff000000))*/) {
        _MESSAGE("Linking to existing mesh (%s)", NewPath);

        Mesh->second->AddRef();
        return Mesh->first;
      }
    }

    Mesh++;
  }

  ManagedMeshRecord *NewMesh = new ManagedMeshRecord();
  if (!NewMesh->LoadMesh(type, NewPath, false)) {
    delete NewMesh;
    return -1;
  }

  _MESSAGE("Loading mesh (%s)", NewPath);

  /* prepare */
  NewMesh->AddRef();

  /* append and sort */
  ManagedMeshes.push_back(NewMesh);

  /* register */
  Meshes[MeshIndex++] = NewMesh;

  return MeshIndex - 1;
}

int MeshManager::LoadDependtMesh(const char *Filename, MeshRecordType type) {
  if (strlen(Filename) > 240)
    return NULL;

  char NewPath[256];
  strcpy_s(NewPath, 256, "data\\meshes\\");
  strcat_s(NewPath, 256, Filename);

  MeshRegistry::iterator Mesh = Meshes.begin();

  /* search for any mesh */
  while (Mesh != Meshes.end()) {
    if (!_stricmp(NewPath, Mesh->second->GetPath())/*&& ((Effect->second->ParentRefID & 0xff000000) == (refID & 0xff000000))*/) {
      _MESSAGE("Linking to existing mesh (%s)", NewPath);

      Mesh->second->AddRef();
      return Mesh->first;
    }

    Mesh++;
  }

  ManagedMeshRecord *NewMesh = new ManagedMeshRecord();
  if (!NewMesh->LoadMesh(type, NewPath, true)) {
    delete NewMesh;
    return -1;
  }

  _MESSAGE("Loading mesh (%s)", NewPath);

  /* prepare */
  NewMesh->AddRef();

  /* append and sort */
  ManagedMeshes.push_back(NewMesh);

  /* register */
  Meshes[MeshIndex++] = NewMesh;

  return MeshIndex - 1;
}

bool MeshManager::ReleaseMesh(int MeshNum) {
  if (!IsMeshValid(MeshNum))
    return false;

  _DMESSAGE("Releasing managed mesh.");

  ManagedMeshRecord *OldMesh = Meshes[MeshNum];

  /* reached zero */
  if (!OldMesh->Release()) {
    /* purge from shader/effect contant-tables */
    OldMesh->Purge(MeshNum);

    /* remove from map */
    Meshes.erase(MeshNum);
    /* remove from vector */
    ManagedMeshes.erase(std::find(ManagedMeshes.begin(), ManagedMeshes.end(), OldMesh));

    _DMESSAGE("and removing it from memory.");
    delete OldMesh;

    return true;
  }

  return false;
}

template<>
int MeshManager::FindMesh(ID3DXBaseMesh *mesh) {
  MeshRegistry::iterator Mesh = Meshes.begin();

  while (Mesh != Meshes.end()) {
    if (Mesh->second->HasMesh(mesh))
      return Mesh->first;

    Mesh++;
  }

  return -1;
}

template<>
bool MeshManager::ReleaseMesh(ID3DXBaseMesh *mesh) {
  return ReleaseMesh(FindMesh<ID3DXBaseMesh>(mesh));
}

void MeshManager::FreeMesh(int MeshNum) {
  if (!IsMeshValid(MeshNum)) {
    _MESSAGE("Tried to free a non existant mesh");
    return;
  }

  ManagedMeshRecord *OldMesh = Meshes[MeshNum];

  if (OldMesh->HasMesh()) {
    OldMesh->Purge(MeshNum);

    /* remove from map */
    Meshes.erase(MeshNum);
    /* remove from vector */
    ManagedMeshes.erase(std::find(ManagedMeshes.begin(), ManagedMeshes.end(), OldMesh));

    _DMESSAGE("Freeing %s", OldMesh->GetPath());
    delete OldMesh;

    return;
  }

  return;
}

void MeshManager::NewGame() {
  if (PurgeOnNewGame.Get()) {
    /* prevent locking up because other resources have allready been freed */
    MeshRegistry prevMeshes = Meshes; Meshes.clear();
    MeshRegistry::iterator Mesh = prevMeshes.begin();

    while (Mesh != prevMeshes.end()) {
      ManagedMeshRecord *OldMesh = Mesh->second;

      if (OldMesh->HasMesh())
        OldMesh->Purge(Mesh->first);

      delete OldMesh;

      Mesh++;
    }

    Clear();
  }
  else {
    MeshRegistry::iterator Mesh = Meshes.begin();

    /* disable and remove all refs */
    while (Mesh != Meshes.end()) {
      /* TODO ClrRef clears enabled as well, logical, no? */
      if (!Mesh->second->IsPrivate())
	Mesh->second->ClrRef();

      /* delete private shaders I guess */
      Mesh++;
    }
  }
}

void MeshManager::SaveGame(OBSESerializationInterface *Interface) {
#ifdef	NO_DEPRECATED
  int temp;
#endif

  _MESSAGE("MeshManager::SaveGame");

  MeshRegistry::iterator Mesh = Meshes.begin();

  Interface->WriteRecord('MIDX', MESHVERSION, &MeshIndex, sizeof(MeshIndex));

  _MESSAGE("Save-game will reference %i meshes.", MeshIndex);

  while (Mesh != Meshes.end()) {
    if (!Mesh->second->IsPrivate()) {
      if (Mesh->second->HasMesh()) {
        const char *path = Mesh->second->GetPath();
        const MeshRecordType type = Mesh->second->GetType();

        Interface->WriteRecord('MNUM', MESHVERSION, &Mesh->first, sizeof(Mesh->first));
        Interface->WriteRecord('MPAT', MESHVERSION, path, strlen(path) + 1);
        Interface->WriteRecord('MTYP', MESHVERSION, &type, sizeof(type));

#ifdef	NO_DEPRECATED
        Interface->WriteRecord('MEOD', MESHVERSION, &temp, 1);
#endif
      }
    }

    Mesh++;
  }

#ifdef	NO_DEPRECATED
  Interface->WriteRecord('MEOF', MESHVERSION, &temp, 1);
#endif
}

void MeshManager::LoadGame(OBSESerializationInterface *Interface) {
  int maxtex = 0;
  UInt32 type, version, length;
  int OldMeshNum = -1;
  int MeshNum = -1;

  /* obtain a copy of the old resources */
  MeshRegistry prevMeshes = Meshes;
  MeshRecordType MeshType;
  char MeshPath[260];

  Interface->GetNextRecordInfo(&type, &version, &length);

  if (type == 'MIDX') {
    Interface->ReadRecordData(&maxtex, length);
    _MESSAGE("Save-game references to %i meshes.", maxtex);
  }
  else {
    _MESSAGE("No mesh data found in save-game.");
    return;
  }

#ifdef	NO_DEPRECATED
  Interface->GetNextRecordInfo(&type, &version, &length);

  while (type != 'MEOF') {
    if (type == 'MNUM') {
      OldMeshNum = MeshNum;
      Interface->ReadRecordData(&MeshNum, length);
      _MESSAGE("Found MNUM record = %i.", MeshNum);
    }
    else {
      _MESSAGE("Error loading game. type!=MNUM");
      return;
    }
#else
  while (MeshNum < (maxtex - 1)) {
    Interface->GetNextRecordInfo(&type, &version, &length);

    if (type == 'MNUM') {
      OldMeshNum = MeshNum;
      Interface->ReadRecordData(&MeshNum, length);
      _MESSAGE("Found MNUM record = %i.", MeshNum);
#endif

      Interface->GetNextRecordInfo(&type, &version, &length);

      if (type == 'MPAT') {
        Interface->ReadRecordData(MeshPath, length);
        _MESSAGE("Found MPAT record = %s", MeshPath);
      }
      else {
        _MESSAGE("Error loading mesh list. type!=MPAT");
        return;
      }

      Interface->GetNextRecordInfo(&type, &version, &length);

      if (type == 'MTYP') {
        Interface->ReadRecordData(&MeshType, length);
        _MESSAGE("Found MTYP record = %i", MeshType);
      }
      else {
        MeshType = MR_REGULAR;
        _MESSAGE("Error loading mesh list. type!=MTYP");
      }

      if (LoadManagedMesh(MeshPath, MeshType) == -1) {
        _MESSAGE("Error loading mesh list: mesh (%s) no longer exists.", MeshPath);
      }

#ifdef	NO_DEPRECATED
      else {
	Interface->GetNextRecordInfo(&type, &version, &length);
	while (type != 'MEOD') {
	  Interface->ReadRecordData(MeshPath, length);
	  Interface->GetNextRecordInfo(&type, &version, &length);
	}
      }

      Interface->GetNextRecordInfo(&type, &version, &length);
#else
    }
    else {
      _MESSAGE("Error loading mesh list: too small.");
      return;
    }
#endif
  }

  /* release previous non-private meshes, those are supposed
   * to be handled entirely by the game-save
   */
  MeshRegistry::iterator PMesh = prevMeshes.begin();

  while (PMesh != prevMeshes.end()) {
    if (PMesh->second &&
       !PMesh->second->IsPrivate())
      ReleaseMesh(PMesh->first);

    PMesh++;
  }
}
