#pragma once

#include <d3dx9.h>

#include "Nodes/NiVector4.h"
#include "Nodes/NiDX9Renderer.h"
#include "Nodes/NiCamera.h"
#include "OBGE fork/Sky.h"
#include "Nodes/NiBillboardNode.h"

#include <vector>
#include <map>
#include <utility>

#include "Rendering.h"
#include "obse/PluginAPI.h"
#include "D3D9Identifiers.hpp"

//efine MYVERTEXFORMAT D3DFVF_XYZRHW|D3DFVF_TEX1
#define MYVERTEXFORMAT D3DFVF_XYZ|D3DFVF_TEX1

#define SHADERVERSION 1

class EffectManager;

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
	int data[16];
};

struct FloatType
{
	char Name[100];
	int size;
	float data[16];
};

struct EffectConstants
{
	// ****** Global static effect constants ******
	v1_2_416::NiVector4		rcpres;
	bool				bHasDepth;

	// ****** Global effect constants (Updated each frame) ******
	D3DXMATRIX			world;
	D3DXMATRIX			view;
	D3DXMATRIX			proj;

	v1_2_416::NiVector4		ZRange;
	v1_2_416::NiVector4		SunDir;
	v1_2_416::NiVector4		SunTiming;
	v1_2_416::NiVector3		EyeForward;

	struct { int x, y, z, w; }	GameTime;
	struct { int x, y, z, w; }	TikTiming;

	/* deprecated */
#ifndef	NO_DEPRECATED
	v1_2_416::NiVector4		time;
#endif
};

class EffectBuffer
{
public:
	EffectBuffer();
	~EffectBuffer();

	/* initialize the buffer from internal resources */
	HRESULT				Initialise(IDirect3DTexture9 *text);
	/* initialize the buffer from internal resources */
	HRESULT				Initialise(enum OBGEPass pass,
						   IDirect3DSurface9 *surf);
	/* initialize the buffer from newly allocated resources */
	HRESULT				Initialise(D3DFORMAT rt0,
						   D3DFORMAT rt1 = D3DFMT_UNKNOWN,
						   D3DFORMAT rt2 = D3DFMT_UNKNOWN,
						   D3DFORMAT rt3 = D3DFMT_UNKNOWN);
	void				Release();
	bool				IsValid();

	bool				IsTexture(IDirect3DBaseTexture9 *text);
	void				SetTexture(const char *fmt, ID3DXEffect *Effect);
	void				SetRenderTarget(IDirect3DDevice9 *Device);
	void				Copy(IDirect3DDevice9 *Device, EffectBuffer *from);

private:
	IDirect3DTexture9 *		Tex[4];
	IDirect3DSurface9 *		Srf[4];
};

class EffectQueue
{
public:
	/* over frames */
	void Init(EffectBuffer *past,
		  EffectBuffer *prev,
		  EffectBuffer *alt);

	/* over effects */
	void Begin(EffectBuffer *orig,
		   EffectBuffer *target,
		   EffectBuffer *alt);
	void End(EffectBuffer *target);

	/* over passes */
	void Begin(ID3DXEffect *Effect);
	void Step(ID3DXEffect *Effect);
	void End(ID3DXEffect *Effect);

public:
	IDirect3DDevice9 *device;
private:
	EffectBuffer *past, *orig, *prev;
	EffectBuffer *queue[2], *rotate[2];
	int alterning, pos;
};

class EffectRecord
{
	friend class GUIs_ShaderDeveloper;

public:
	EffectRecord();
	~EffectRecord();

	bool LoadEffect(const char *Filename, UINT32 refID, bool Private = false, D3DXMACRO *defs = NULL);
	bool RuntimeFlush();
	bool RuntimeEffect(const char *fx);

	bool CompileEffect(bool forced = false);
	bool SaveEffect();

	void ApplyCompileDirectives();
	void ApplyPermanents(EffectConstants *ConstList, EffectManager *FXMan);
	void ApplyConstants(EffectConstants *ConstList);
	void ApplyDynamics();

	void OnLostDevice(void);
	void OnResetDevice(void);

	void Render(IDirect3DDevice9 *D3DDevice, IDirect3DSurface9 *RenderTo, IDirect3DSurface9 *RenderCopy);
	bool Render(IDirect3DDevice9 *D3DDevice, EffectConstants *ConstList, EffectQueue *Queue);
	void Render(IDirect3DDevice9 *D3DDevice);

	bool SetEffectConstantB(const char *name, bool value);
	bool SetEffectConstantI(const char *name, int value);
	bool SetEffectConstantI(const char *name, int *values, int num);
	bool SetEffectConstantF(const char *name, float value);
	bool SetEffectConstantF(const char *name, float *values, int num);
	bool SetEffectConstantV(const char *name, v1_2_416::NiVector4 *value);
	bool SetEffectSamplerTexture(const char *name, int TextureNum);

	void PurgeTexture(IDirect3DBaseTexture9 *texture, int TexNum = -1);
	void SaveVars(OBSESerializationInterface *Interface);

	ID3DXEffect *GetEffect() const;
	bool HasEffect() const;

	void Enable(bool Enabled);
	bool IsEnabled() const;

	UINT32 GetRefID() const;
	const char *GetPath() const;
	const char *GetName() const;
	bool IsPrivate() const;
	void SetPriority(int pri);
	unsigned long GetParameters() const;
	unsigned long GetConditions() const;
	unsigned long GetConditions(int pass) const;

public:
	void Purge();
	void Kill();

	static bool compare(const EffectRecord *d1, const EffectRecord *d2) {
	  return
	    (unsigned long)d1->Priority
	    <
	    (unsigned long)d2->Priority;
	}

private:
	char				Name[100];
	char				Filepath[MAX_PATH];
	bool				Enabled;
	bool				Private;
	UINT32				ParentRefID;
	// Associates a effect with the esp/esm file the script the effect was created in.

	/* source-code buffers (FX) */
	bool				bFX;
	LPSTR 				pSource;
	UINT				sourceLen;

	/* compiled results */
	D3DXMACRO *			pDefine;
	LPD3DXBUFFER			pBinary;
	LPD3DXBUFFER			pErrorMsgs;
	ID3DXEffect *			pEffect;

	/* re-allocate textures after kill() */
	std::vector<int>		Textures;

protected:
	unsigned long			Parameters;
	int				Priority;
	int				Class;
	int				Flags;
	int				FlagsPass[16];
};

class ManagedEffectRecord : public EffectRecord
{
public:
	ManagedEffectRecord();
	~ManagedEffectRecord();

	static bool compare(const ManagedEffectRecord *d1, const ManagedEffectRecord *d2) {
	  return
	    (unsigned long)d1->Priority
	    <
	    (unsigned long)d2->Priority;
	}

	void ClrRef();
	int AddRef();
	int Release();
	int RefCount;
};

//pedef std::vector<EffectRecord *> EffectList;
typedef std::vector<ManagedEffectRecord *> ManagedEffectList;
typedef std::map<int, ManagedEffectRecord *> EffectRegistry;

class EffectManager
{
	friend class EffectRecord;
	friend class GUIs_ShaderDeveloper;

public:
	EffectManager();
	~EffectManager();

	static EffectManager *GetSingleton(void);
	static EffectManager *Singleton;

	void						OnLostDevice(void);
	void						OnResetDevice(void);
	void						OnReleaseDevice(void);

	void						InitialiseBuffers();
	void						InitialiseFrameTextures();
	void						ReleaseBuffers();
	void						ReleaseFrameTextures();

	bool						SetRAWZ(bool enabled);
	void						LoadEffectList(void);

	void						NewGame(void);
	void						LoadGame(OBSESerializationInterface *Interface);
	void						SaveGame(OBSESerializationInterface *Interface);

private:
	void						Reset();

	void						UpdateStaticConstants(void);
	void						UpdateFrameConstants(v1_2_416::NiDX9Renderer *Renderer);

public:
	void						Render(IDirect3DDevice9 *D3DDevice, IDirect3DSurface9 *RenderTo, IDirect3DSurface9 *RenderFrom);
	void						RenderRAWZfix(IDirect3DDevice9 *D3DDevice, IDirect3DSurface9 *RenderTo);

	int						AddPrivateEffect(const char *Filename, UINT32 refID);
	int						AddManagedEffect(const char *Filename, UINT32 refID);
	int						AddDependtEffect(const char *Filename, UINT32 refID);

	inline bool					IsEffectValid(int EffectNum) const { return Effects.count(EffectNum) != 0; };
	inline ManagedEffectRecord *			GetEffect(int EffectNum) { return (IsEffectValid(EffectNum) ? Effects[EffectNum] : NULL); };
	bool						EnableEffect(int EffectNum, bool State);
	bool						ReleaseEffect(int EffectNum);
	void						FreeEffect(int EffectNum);
	bool						GetEffectState(int EffectNum);
	int						FindEffect(const char *Filename);

	bool						SetEffectConstantB(int EffectNum, char *name, bool value);
	bool						SetEffectConstantI(int EffectNum, char *name, int value);
	bool						SetEffectConstantI(int EffectNum, char *name, int *values, int num);
	bool						SetEffectConstantF(int EffectNum, char *name, float value);
	bool						SetEffectConstantF(int EffectNum, char *name, float *values, int num);
	bool						SetEffectConstantV(int EffectNum, char *name, v1_2_416::NiVector4 *value);
	bool						SetEffectSamplerTexture(int EffectNum, char *name, int TextureNum);

	void						Recalculate();
	void						PurgeTexture(IDirect3DBaseTexture9 *texture, int TexNum = -1);

private:
	int						EffectIndex;
	int						MaxEffectIndex;

	EffectRegistry					Effects;
//	EffectList					Effects;
	ManagedEffectList				ManagedEffects;

	IDirect3DVertexBuffer9 *			EffectVertex;
	EffectRecord *					EffectDepth;
	EffectConstants					EffectConst;

	EffectBuffer					OrigRT, CopyRT, TrgtRT;
	EffectBuffer					LastRT, PrevRT, PastRT;
	EffectBuffer					OrigDS, CurrDS;

	bool						RenderRawZ;
	EffectQueue					RenderQueue;
	unsigned long					RenderBuf;
	unsigned long					RenderCnd;
	D3DFORMAT					RenderFmt;

public:
	void UseLegacyCompiler(bool yes);
	void CompileSources(bool yes);
	void Optimize(bool yes);

	bool UseLegacyCompiler();
	bool CompileSources();
	bool Optimize();
	bool UseEffectList();
	const char *EffectDirectory();
	const char *EffectListFile();

#ifdef	OLD_QUEUE
	IDirect3DTexture9*				thisframeTex;
	IDirect3DSurface9*				thisframeSurf;

	IDirect3DTexture9*				lastpassTex;
	IDirect3DSurface9*				lastpassSurf;

	IDirect3DTexture9*				lastframeTex;
	IDirect3DSurface9*				lastframeSurf;

	bool						HasDepth;

	IDirect3DTexture9*				depth;
	IDirect3DSurface9*				depthSurface;
	IDirect3DTexture9*				depthRAWZ;

	bool						RAWZflag;
#endif
};