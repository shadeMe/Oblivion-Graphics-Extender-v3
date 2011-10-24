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

#define _USE_MATH_DEFINES
#include <math.h>

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

#define SHADERVERSION 1

class EffectManager;

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

#include "Constants.h"

class EffectBuffer
{
	friend class GUIs_ShaderDeveloper;

public:
	EffectBuffer();
	~EffectBuffer();

#define EBUFRT_NUM  1
#if	EBUFRT_NUM == 4
	HRESULT				Initialize(const D3DFORMAT rt0,
						   const D3DFORMAT rt1 = D3DFMT_UNKNOWN,
						   const D3DFORMAT rt2 = D3DFMT_UNKNOWN,
						   const D3DFORMAT rt3 = D3DFMT_UNKNOWN) {
						   const D3DFORMAT fmt[EBUFRT_NUM] = {rt0,rt1,rt2,rt3}; return Initialize(fmt); }
#elif	EBUFRT_NUM == 1
	HRESULT				Initialize(const D3DFORMAT rt0) {
						   const D3DFORMAT fmt[EBUFRT_NUM] = {rt0}; return Initialize(fmt); }
#endif

	/* initialize the buffer from internal resources */
	HRESULT				Initialize(IDirect3DTexture9 *text);
	/* initialize the buffer from internal resources */
	HRESULT				Initialize(IDirect3DSurface9 *surf);
	/* initialize the buffer from newly allocated resources */
	HRESULT				Initialize(const D3DFORMAT fmt[EBUFRT_NUM]);
	void				Release(int rmin = 0, int rnum = EBUFRT_NUM);

	bool				IsValid() const;
	bool				IsTexture(IDirect3DBaseTexture9 *text) const;

	void				SetTexture(const char *fmt, ID3DXEffect *Effect) const;
	void				SetTexture(const D3DXHANDLE *hs, ID3DXEffect *pEffect) const;
	void				SetTexture(const D3DXHANDLE hs, ID3DXEffect *pEffect) const;
	void				SetRenderTarget(IDirect3DDevice9 *Device) const;
	void				Copy(IDirect3DDevice9 *Device, EffectBuffer *from) const;
	void				Copy(IDirect3DDevice9 *Device, IDirect3DSurface9 *from) const;

private:
	IDirect3DTexture9 *		Tex[EBUFRT_NUM];
	IDirect3DSurface9 *		Srf[EBUFRT_NUM];
	bool				mne[EBUFRT_NUM];
};

class EffectQueue
{
public:
	EffectQueue(bool custom = false);
	~EffectQueue();

	void SetCustom(bool custom = false);

	/* over frames */
	void Init(EffectBuffer *prev,
		  EffectBuffer *alt, bool stencil = false);

	/* over effects */
	void Begin(EffectBuffer *orig,
		   EffectBuffer *target,
		   EffectBuffer *alt);
	void End(EffectBuffer *past,
		 EffectBuffer *target);

	/* over passes */
	void Begin(const D3DXHANDLE *h, ID3DXEffect *Effect, unsigned long Parameters);
	void Swap(const D3DXHANDLE *h, ID3DXEffect *Effect);
	void Pass(const D3DXHANDLE *h, ID3DXEffect *Effect);
	void End(const D3DXHANDLE *h, ID3DXEffect *Effect);

public:
	IDirect3DDevice9 *device;
private:
	EffectBuffer *orig, *prev, *prvl;
	EffectBuffer *queue[2], *rotate[2];
	int alterning, pos; int dsc, dclr;

	/* indices into the parameter-handles LUT */
	unsigned int currHL, prevHL, lastHL;
};

class ManagedEffectQueue : public EffectQueue
{
	friend class EffectRecord;
	friend class GUIs_ShaderDeveloper;

public:
	ManagedEffectQueue();
	~ManagedEffectQueue();

	void ClrRef();
	int AddRef();
	int Release();
	int RefCount;

private:
	/* may need double-buffering */
	EffectBuffer OrigRT, LastRT, TrgtRT;
};

typedef std::map<D3DFORMAT, ManagedEffectQueue> ManagedQueueList;

class EffectRecord
{
	friend class GUIs_ShaderDeveloper;

public:
	EffectRecord();
	~EffectRecord();

	bool LoadEffect(const char *Filename, UINT32 refID, bool Private = false, D3DXMACRO *defs = NULL);
	bool RuntimeFlush();
	bool RuntimeEffect(const char *fx);

	bool CompileEffect(EffectManager *FXMan, bool forced = false);
	bool SaveEffect();

	void ApplyCompileDirectives(EffectManager *FXMan);
	void ApplyPermanents(EffectManager *FXMan);
	void ApplyCustomConstants();
	void ApplySharedConstants();
	void ApplyUniqueConstants();

	void OnLostDevice(void);
	void OnResetDevice(void);

	void Render(IDirect3DDevice9 *D3DDevice, IDirect3DSurface9 *RenderTo, IDirect3DSurface9 *RenderCopy);
	bool Render(IDirect3DDevice9 *D3DDevice, ManagedEffectQueue *Queue);
	bool Render(IDirect3DDevice9 *D3DDevice, EffectQueue *Queue);
	void Render(IDirect3DDevice9 *D3DDevice);

	bool SetEffectConstantB(const char *name, BOOL value);
	bool SetEffectConstantI(const char *name, int value);
	bool SetEffectConstantI(const char *name, int *values, int num);
	bool SetEffectConstantF(const char *name, float value);
	bool SetEffectConstantF(const char *name, float *values, int num);
	bool SetEffectConstantV(const char *name, v1_2_416::NiVector4 *value);
	bool SetEffectSamplerTexture(const char *name, int TextureNum);

	bool GetEffectConstantHelps(std::map<std::string,std::string> &all) const;
	bool GetEffectConstantTypes(std::map<std::string,int> &all) const;
	bool GetEffectConstantHelp(const char *name, const char **help) const;
	bool GetEffectConstantType(const char *name, int *type) const;
	bool GetEffectConstantB(const char *name, BOOL *value) const;
	bool GetEffectConstantI(const char *name, int *value) const;
	bool GetEffectConstantI(const char *name, int *values, int num) const;
	bool GetEffectConstantF(const char *name, float *value) const;
	bool GetEffectConstantF(const char *name, float *values, int num) const;
	bool GetEffectConstantV(const char *name, float *value) const;
	bool GetEffectSamplerTexture(const char *name, int *TextureNum) const;

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
	unsigned long GetOptions() const;
	unsigned long GetOptions(int pass) const;

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
	char				Prolog[100];
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
	LPD3DXBUFFER			pDisasmbly;
	ID3DXEffect *			pEffect;

	/* re-allocate textures after kill() */
	std::vector<int>		Textures;

protected:
	/* handle-array */
	D3DXHANDLE			h[256];

	inline void SetValue(unsigned int hl, LPCVOID pData, UINT Bytes)  { if (h[hl]) pEffect->SetValue(h[hl], pData, Bytes); }
	inline void SetBool(unsigned int hl, BOOL b)  { if (h[hl]) pEffect->SetBool(h[hl], b); }
	inline void SetBoolArray(unsigned int hl, CONST BOOL* pb, UINT Count)  { if (h[hl]) pEffect->SetBoolArray(h[hl], pb, Count); }
	inline void SetInt(unsigned int hl, INT n)  { if (h[hl]) pEffect->SetInt(h[hl], n); }
	inline void SetIntArray(unsigned int hl, CONST INT* pn, UINT Count)  { if (h[hl]) pEffect->SetIntArray(h[hl], pn, Count); }
	inline void SetFloat(unsigned int hl, FLOAT f)  { if (h[hl]) pEffect->SetFloat(h[hl], f); }
	inline void SetFloatArray(unsigned int hl, CONST FLOAT* pf, UINT Count)  { if (h[hl]) pEffect->SetFloatArray(h[hl], pf, Count); }
	inline void SetVector(unsigned int hl, CONST D3DXVECTOR4* pVector)  { if (h[hl]) pEffect->SetVector(h[hl], pVector); }
	inline void SetVectorArray(unsigned int hl, CONST D3DXVECTOR4* pVector, UINT Count)  { if (h[hl]) pEffect->SetVectorArray(h[hl], pVector, Count); }
	inline void SetMatrix(unsigned int hl, CONST D3DXMATRIX* pMatrix)  { if (h[hl]) pEffect->SetMatrix(h[hl], pMatrix); }
	inline void SetMatrixArray(unsigned int hl, CONST D3DXMATRIX* pMatrix, UINT Count)  { if (h[hl]) pEffect->SetMatrixArray(h[hl], pMatrix, Count); }
	inline void SetMatrixPointerArray(unsigned int hl, CONST D3DXMATRIX** ppMatrix, UINT Count)  { if (h[hl]) pEffect->SetMatrixPointerArray(h[hl], ppMatrix, Count); }
	inline void SetMatrixTranspose(unsigned int hl, CONST D3DXMATRIX* pMatrix)  { if (h[hl]) pEffect->SetMatrixTranspose(h[hl], pMatrix); }
	inline void SetMatrixTransposeArray(unsigned int hl, CONST D3DXMATRIX* pMatrix, UINT Count)  { if (h[hl]) pEffect->SetMatrixTransposeArray(h[hl], pMatrix, Count); }
	inline void SetMatrixTransposePointerArray(unsigned int hl, CONST D3DXMATRIX** ppMatrix, UINT Count)  { if (h[hl]) pEffect->SetMatrixTransposePointerArray(h[hl], ppMatrix, Count); }
	inline void SetString(unsigned int hl, LPCSTR pString)  { if (h[hl]) pEffect->SetString(h[hl], pString); }
	inline void SetTexture(unsigned int hl, LPDIRECT3DBASETEXTURE9 pTexture)  { if (h[hl]) pEffect->SetTexture(h[hl], pTexture); }

	unsigned long			Parameters;
	unsigned long			ParametersCustom;
	int				Priority;
	int				Class;

	int				Flags;
	int				FlagsPass[16];
	int				Options;
	int				OptionsPass[16];
	int				OptionsCustom;
	int				OptionsCustomPass[16];

	/* while it is attractive to give it an entire custom queue
	 * (as it allows to pass custom data from frame to frame) we
	 * currently maintain a custom queue per format (R16F fe.)
	 */
	ManagedEffectQueue *		CustomQueue;
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

	void						InitializeBuffers();
	void						InitializeFrameTextures();
	void						ReleaseBuffers();
	void						ReleaseFrameTextures();

	bool						SetTransferZ(long MaskZ);
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

	inline ManagedEffectQueue *			RequestQueue(D3DFORMAT format);
	void						ReleaseQueue(D3DFORMAT format);
	void						ReleaseQueue(ManagedEffectQueue *queue);

	int						AddPrivateEffect(const char *Filename, UINT32 refID);
	int						AddManagedEffect(const char *Filename, UINT32 refID);
	int						AddDependtEffect(const char *Filename, UINT32 refID);

	inline bool					IsEffectValid(int EffectNum) const { return Effects.count(EffectNum) != 0; };
	inline ManagedEffectRecord *			GetEffect(int EffectNum) const { return (IsEffectValid(EffectNum) ? ((EffectRegistry &)Effects)[EffectNum] : NULL); };
	bool						EnableEffect(int EffectNum, bool State);
	bool						ReleaseEffect(int EffectNum);
	void						FreeEffect(int EffectNum);
	bool						GetEffectState(int EffectNum) const;
	int						FindEffect(const char *Filename) const;
	bool						GetEffects(int which, std::map<std::string,int> &all) const;

	bool						SetEffectConstantB(int EffectNum, char *name, BOOL value);
	bool						SetEffectConstantI(int EffectNum, char *name, int value);
	bool						SetEffectConstantI(int EffectNum, char *name, int *values, int num);
	bool						SetEffectConstantF(int EffectNum, char *name, float value);
	bool						SetEffectConstantF(int EffectNum, char *name, float *values, int num);
	bool						SetEffectConstantV(int EffectNum, char *name, v1_2_416::NiVector4 *value);
	bool						SetEffectSamplerTexture(int EffectNum, char *name, int TextureNum);

	bool						GetEffectConstantHelps(int EffectNum, std::map<std::string,std::string> &all) const;
	bool						GetEffectConstantTypes(int EffectNum, std::map<std::string,int> &all) const;
	bool						GetEffectConstantHelp(int EffectNum, char *name, const char **help) const;
	bool						GetEffectConstantType(int EffectNum, char *name, int *type) const;
	bool						GetEffectConstantB(int EffectNum, char *name, BOOL *value) const;
	bool						GetEffectConstantI(int EffectNum, char *name, int *value) const;
	bool						GetEffectConstantI(int EffectNum, char *name, int *values, int num) const;
	bool						GetEffectConstantF(int EffectNum, char *name, float *value) const;
	bool						GetEffectConstantF(int EffectNum, char *name, float *values, int num) const;
	bool						GetEffectConstantV(int EffectNum, char *name, float *value) const;
	bool						GetEffectSamplerTexture(int EffectNum, char *name, int *TextureNum) const;

	void						Recalculate();
	void						PurgeTexture(IDirect3DBaseTexture9 *texture, int TexNum = -1);

private:
	int						EffectIndex;
	int						MaxEffectIndex;

	EffectRegistry					Effects;
//	EffectList					Effects;
	ManagedEffectList				ManagedEffects;

//ruct EffectQuad { float x,y,z, rhw; float u,v, i; };
struct EffectQuad { float x,y,z;      float u,v, i; };

//efine EFFECTQUADFORMAT D3DFVF_XYZRHW | D3DFVF_TEX1 | D3DFVF_TEXCOORDSIZE3(0)
#define EFFECTQUADFORMAT D3DFVF_XYZ    | D3DFVF_TEX1 | D3DFVF_TEXCOORDSIZE3(0)

	IDirect3DVertexBuffer9 *			EffectVertex;
	EffectRecord *					EffectDepth;
//	EffectConstants					EffectConst;

#if 1 //def OBGE_CONSTANTPOOLS
	ID3DXEffectPool *				EffectPool;
	EffectRecord *					EffectShare;
#endif

	EffectBuffer					OrigRT, CopyRT, TrgtRT;
	EffectBuffer					LastRT, PrevRT, PastRT;
	EffectBuffer					OrigDS, CurrDS, CurrNM;

	long						RenderTransferZ;

	ManagedQueueList				CustomQueues;
	EffectQueue					RenderQueue;
	unsigned long					RenderBuf;
	unsigned long					RenderCnd;
	unsigned long					RenderOpt;
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