#pragma once

#include "d3dx9.h"
#include "nodes\NiVector4.h"
#include "nodes\NiDX9Renderer.h"
#include "nodes/NiCamera.h"
#include "OBGE fork/Sky.h"
#include "nodes/NiBillboardNode.h"
#include <vector>
#include <map>
#include <utility>
#include "Rendering.h"
#include "obse/PluginAPI.h"

//#define MYVERTEXFORMAT D3DFVF_XYZRHW|D3DFVF_TEX1
#define MYVERTEXFORMAT D3DFVF_XYZ|D3DFVF_TEX1

#define SHADERVERSION 1

//struct D3D_sShaderVertex { float x,y,z,w,u,v; };
struct D3D_sShaderVertex { float x,y,z,u,v; };

struct TextureType
{
	int tex;
	char Name[100];
};

struct IntType
{
	char Name[100];
	int size;
	int	data[16];
};

struct FloatType
{
	char Name[100];
	int size;
	float data[16];
};

struct Constants
{
// ****** Global static effect constants ******

	float					rcpres[2];
	bool					bHasDepth;

// ****** Global effect constants (Updated each frame) ******

	D3DXMATRIX				world;
	D3DXMATRIX				view;
	D3DXMATRIX				proj;
	v1_2_416::NiVector4		time;
	v1_2_416::NiVector4		SunDir;
	v1_2_416::NiVector3		EyeForward;
};

class EffectRecord
{
public:
	EffectRecord();
	~EffectRecord();

	void						Render(IDirect3DDevice9* D3DDevice,IDirect3DSurface9 *RenderTo);
	void						OnLostDevice(void);
	void						OnResetDevice(void);
	void						ApplyConstants(Constants *ConstList);
	void						ApplyDynamics();
	bool						IsEnabled();
	bool						LoadEffect(char *Filename);
	void						ApplyCompileDirectives(void);
	bool						SetEffectInt(char *name, int value);
	bool						SetEffectFloat(char *name, float value);
	bool						SetEffectVector(char *name, v1_2_416::NiVector4 *value);
	bool						SetEffectTexture(char *name, int TextureNum);
	void						SaveVars(OBSESerializationInterface *Interface);

	char						Name[100];
	char						Filepath[MAX_PATH];
	ID3DXEffect*				Effect;
	bool						Enabled;
	UINT32						ParentRefID; // Associates a effect with the esp/esm file the script the effect was created in.
};

typedef std::vector<EffectRecord*> StaticEffectList;
typedef std::map<int, EffectRecord*> EffectList;

class EffectManager
{
private:
	EffectManager();
public:
	~EffectManager();

	static EffectManager*		GetSingleton(void);
	void						UpdateStaticConstants(void);
	void						UpdateFrameConstants(void);
	void						Render(IDirect3DDevice9 *D3DDevice,IDirect3DSurface9 *RenderTo, IDirect3DSurface9 *RenderFrom);
	void						RenderRAWZfix(IDirect3DDevice9*	D3DDevice,IDirect3DSurface9 *RenderTo);
	void						OnLostDevice(void);
	void						OnResetDevice(void);
	void						InitialiseBuffers(void);
	void						DeviceRelease(void);
	void						LoadEffectList(void);
	void						NewGame(void);
	void						LoadGame(OBSESerializationInterface *Interface);
	void						SaveGame(OBSESerializationInterface *Interface);

	int							AddEffect(char *Filename, bool AllowDuplicates, UINT32 refID);
	bool						AddStaticEffect(char *Filename);
	bool						RemoveEffect(int EffectNum);
	bool						IsEffectValid(int EffectNum);
	bool						EnableEffect(int EffectNum, bool State);
	bool						SetEffectInt(int EffectNum,char *name, int value);
	bool						SetEffectFloat(int EffectNum, char *name, float value);
	bool						SetEffectVector(int EffectNum, char *name, v1_2_416::NiVector4 *value);
	bool						SetEffectTexture(int EffectNum, char *name, int TextureNum);
	void						PurgeTexture(IDirect3DBaseTexture9 *texture);
	bool						GetEffectState(int EffectNum);

	static EffectManager*		Singleton;

	IDirect3DVertexBuffer9*		D3D_EffectBuffer;

	int							EffectIndex;
	int							MaxEffectIndex;
	StaticEffectList			StaticEffects;
	EffectList					Effects;
	EffectRecord*				DepthEffect;

	Constants					EffectConst;
};