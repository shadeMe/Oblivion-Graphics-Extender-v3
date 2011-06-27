#pragma once
#ifndef	OBGE_NOSHADER

#include <d3dx9.h>
#include <d3dx9shader.h>

#define _USE_MATH_DEFINES
#include <math.h>

#include <vector>
#include <map>
#include <set>
#include <list>
#include <queue>
#include <string>
#include <utility>

#include "Nodes/NiVector4.h"
#include "OBGE fork/Sky.h"
#include "Nodes/NiBillboardNode.h"

#include "D3D9Identifiers.hpp"
#include "TextureManager.h"

class ShaderRecord;
class RuntimeShaderRecord;

class ShaderRecord
{
	friend class GUIs_ShaderDeveloper;

public:
	ShaderRecord();
	~ShaderRecord();

	void						SetBinary(int len, const DWORD *org);
	const DWORD *					GetBinary();

	bool						LoadShader(const char *Filename);
	bool						ChangedShader();
	bool						ReloadShader();
	bool						RuntimeFlush();
	bool						RuntimeShader(const char *hlsl, const char *version = NULL);
	bool						CompareShader(const DWORD *Function) const;

	bool						AssembleShader(bool forced = false);
	bool						CompileShader(bool forced = false);
	bool						DisassembleShader(bool forced = false);
	bool						SaveShader();

	bool						ConstructDX9Shader(char which);
	DWORD *						GetDX9ShaderTexture(const char *sName, int *TexNum, DWORD *States);
	DWORD *						GetDX9RenderStates(DWORD *States);
	bool						DestroyDX9Shader();

public:
	RuntimeShaderRecord *				pAssociate;
	const DWORD *					pOblivionBinary;
	LPD3DXCONSTANTTABLE				pOblivionConTab;

	char						Name[100];
	char						Filepath[MAX_PATH];
	bool						Replaced;

	/* source-code buffers (asm or HLSL) */
	bool						bAssembler;
	LPSTR 						pAsmblyReplaced;
	UINT						asmblyLen;

	bool						bHLSL;
	LPSTR 						pSourceReplaced;
	UINT						sourceLen;

	bool						bRT;
	LPSTR 						pSourceRuntime;
	UINT						runtimeLen;

	time_t						hlslStamp;
	LPD3DXBUFFER					pErrorMsgs;
	LPD3DXBUFFER					pDisasmbly;

#define	SHADER_UNKNOWN	 0
#define	SHADER_VERTEX	-1
#define	SHADER_PIXEL	 1

	/* version and flags */
	char						iType;
	LPSTR						pProfile;
	bool						bPartialPrecision;

	/* compiled results */
	LPD3DXBUFFER					pShaderOriginal;
	LPD3DXBUFFER					pShaderReplaced;
	LPD3DXBUFFER					pShaderRuntime;

	/* D3DXGetShaderConstantTableEx() */
	LPD3DXCONSTANTTABLE				pConstsOriginal;
	LPD3DXCONSTANTTABLE				pConstsReplaced;
	LPD3DXCONSTANTTABLE				pConstsRuntime;

#define	SHADER_UNSET	-1
#define	SHADER_ORIGINAL	 0
#define	SHADER_REPLACED	 1
#define	SHADER_RUNTIME	 2

	/* allocated results */
	char						pDX9ShaderWant;
	char						pDX9ShaderType;
	LPD3DXCONSTANTTABLE				pDX9ShaderCoTa;
	union {
	  IDirect3DVertexShader9 *			pDX9VertexShader;
	  IDirect3DPixelShader9 *			pDX9PixelShader;
	  IUnknown *					pDX9ShaderClss;
	};
};

struct RuntimeConstant {
  union mem {
    bool condition;
    struct iv { int vec[4]; } integer;
    struct fv { float vec[4]; } floating;
    struct tv { D3DSAMPLERSTATETYPE Type; DWORD Value; } state;
    struct tx { IDirect3DBaseTexture9 *texture; float data[4]; } texture;
  } vals;
};

struct RuntimeVariable {
  int offset, length;
  const char *name;
  union mem {
    bool condition;
    struct iv { int vec[4]; } *integer;
    struct fv { float vec[4]; } *floating;
    struct tv { D3DSAMPLERSTATETYPE Type; DWORD Value; } *state;
    IDirect3DBaseTexture9 *texture;
  } vals;
};

class RuntimeShaderRecord
{
public:
	RuntimeShaderRecord();
	~RuntimeShaderRecord();

	static inline void Reset() {
	  for (int o = 0; o < OBGEPASS_NUM; o++) {
	    rsb[o].bCLoaded = rsb[o].bDLoaded = rsb[o].bZLoaded = 0;
	    rsb[o].bCFilled = rsb[o].bDFilled = rsb[o].bZFilled = 0;
	  }
	};

	void Release();
	void OnLostDevice(void);
	void OnResetDevice(void);

	void CreateRuntimeParams(LPD3DXCONSTANTTABLE CoTa);
	void SetRuntimeParams(IDirect3DDevice9 *StateDevice, IDirect3DDevice9 *SceneDevice);

	bool SetShaderConstantB(const char *name, bool value);
	bool SetShaderConstantI(const char *name, int *values);
	bool SetShaderConstantF(const char *name, float *values);
	bool SetShaderSamplerTexture(const char *name, int TextureNum);

	void PurgeTexture(IDirect3DBaseTexture9 *texture, int TexNum = -1);
	bool AssignShader(IUnknown *Shader, ShaderRecord *Associate);
	bool ActivateShader(char which);

	IDirect3DPixelShader9  *GetShader(IDirect3DPixelShader9  *Shader) const;
	IDirect3DVertexShader9 *GetShader(IDirect3DVertexShader9 *Shader) const;

public:
	ShaderRecord *			pAssociate;
	bool				bActive, bMark;
	void *				pCustomCT;
	bool				bIO;
	unsigned int			bMask;

	/* get a copy of the z-buffer right from before and pass it to the shader */
	static struct Buffers {
	  void Release();

struct CameraQuad { float x,y,z, rhw; float u,v; };
//ruct CameraQuad { float x,y,z;      float u,v; };

#define CAMERAQUADFORMAT D3DFVF_XYZRHW | D3DFVF_TEX1
//efine CAMERAQUADFORMAT D3DFVF_XYZ    | D3DFVF_TEX1

	  IDirect3DVertexBuffer9 *	pGrabVX;
	  IDirect3DTexture9 * 		pTextUB;

	  void GrabRT(IDirect3DDevice9 *StateDevice, IDirect3DDevice9 *SceneDevice);
	  void GrabDS(IDirect3DDevice9 *StateDevice, IDirect3DDevice9 *SceneDevice);
	  void GrabDZ(IDirect3DDevice9 *StateDevice, IDirect3DDevice9 *SceneDevice, bool bZFused);

	  IDirect3DSurface9 *		pGrabRT; char bCLoaded;
	  IDirect3DSurface9 *		pGrabDS; char bDLoaded;
	  IDirect3DSurface9 *		pGrabDZ; char bZLoaded;
	  IDirect3DTexture9 * 		pTextRT; bool bCFilled;
	  IDirect3DTexture9 * 		pTextDS; bool bDFilled;
	  IDirect3DTexture9 * 		pTextDZ; bool bZFilled;

	} rsb[OBGEPASS_NUM];

	  IDirect3DTexture9 **		pCopyRT; bool bCFused; bool bCLazy;
	  IDirect3DTexture9 **		pCopyDS; bool bDFused; bool bDLazy;
	  IDirect3DTexture9 **		pCopyDZ; bool bZFused; bool bZLazy;

	RuntimeVariable *		pBool;
	RuntimeVariable *		pInt4;
	RuntimeVariable *		pFloat4;
	RuntimeVariable *		pTexture;
	RuntimeVariable *		pSampler;

	/* Oblivion's POV */
	char				iType;
	const DWORD *			pFunction;
	union {
	  IDirect3DVertexShader9 *	pVertexShader;
	  IDirect3DPixelShader9 *	pPixelShader;
	  IUnknown *			pShader;
	};

	/* re-allocate textures after kill() */
	std::vector<int>		Textures;

#ifdef	OBGE_DEVLING
	void Clear(int pass = -1) {
	  if (pass == -1)
	    memset(traced, -1, sizeof(traced));
	  else
	    memset(&traced[pass], -1, sizeof(traced[pass]));
	}

	/* stats */
	int					frame_used[OBGEPASS_NUM];
	int					frame_pass[OBGEPASS_NUM];

#define	OBGESAMPLER_NUM	16

	/* collect associated pairs of shaders */
	std::set<RuntimeShaderRecord *>		Paired;

	/* we assume a shader isn't used twice in a specific pass
	 * thus we don't track over all of [pass][scene]
	 */
	struct trace {
	  DWORD					states_s[OBGESAMPLER_NUM][14];	// sampler states, D3DSAMPLERSTATETYPE 14
//	  DWORD					states_t[OBGESAMPLER_NUM][33];	// texturesstage states, D3DTEXTURESTAGESTATETYPE 33

	  IDirect3DBaseTexture9 *		values_s[OBGESAMPLER_NUM];	// ps 16, vs 4
	  int					values_b[16][4];		// ps 16, vs 16
	  int					values_i[16][4];		// ps 16, vs 16
	  float					values_c[256][4];
	} traced[OBGEPASS_NUM];
#endif
};

typedef std::list<RuntimeShaderRecord *> RuntimeShaderList;
typedef std::list<ShaderRecord *> BuiltInShaderList;
typedef std::map<IUnknown *, RuntimeShaderRecord *> ShaderList;
typedef std::map<std::string, RuntimeConstant::mem> ConstsList;

#include "Constants.h"

struct GlobalConstants
{
//	ConstsList		pBool;
	ConstsList		pInt4;
	ConstsList		pFloat4;
//	ConstsList		pTexture;
	RuntimeConstant		pTexture[16];
//	ConstsList		pSampler;
};

class ShaderManager
{
	friend class ShaderRecord;
	friend class GUIs_ShaderDeveloper;

public:
	ShaderManager();
	~ShaderManager();

	static ShaderManager*		GetSingleton(void);
	static ShaderManager*		Singleton;

	bool						SetRAWZ(bool enabled);
	void						OnCreateDevice(void);
	void						OnLostDevice(void);
	void						OnResetDevice(void);
	void						OnReleaseDevice(void);

	bool						ChangedShaders();
	bool						ReloadShaders();
private:
	void						Reset();
public:
	template<class ReturnType> ReturnType *		GetGlobalConst(const char *Name, int length, ReturnType *vs);
	template<class ReturnType> bool			SetGlobalConst(const char *Name, int length, ReturnType *vs);
	void						UpdateFrameConstants();
	inline void					Begin() { RuntimeShaderRecord::Reset(); };

public:
	ShaderRecord *					GetBuiltInShader(const char *Name);
	ShaderRecord *					GetBuiltInShader(const DWORD *Function);
	RuntimeShaderRecord *				GetRuntimeShader(const char *Name);
	RuntimeShaderRecord *				SetRuntimeShader(const DWORD *Function, IUnknown *Shader);
	inline RuntimeShaderRecord *			GetRuntimeShader(IUnknown *Shader) { return Shaders[Shader]; }
	IDirect3DPixelShader9  *			GetShader(IDirect3DPixelShader9  *Shader);
	IDirect3DVertexShader9 *			GetShader(IDirect3DVertexShader9 *Shader);

	bool						SetShaderConstantB(const char *ShaderName, char *name, bool value);
	bool						SetShaderConstantI(const char *ShaderName, char *name, int *values);
	bool						SetShaderConstantF(const char *ShaderName, char *name, float *values);
	bool						SetShaderSamplerTexture(const char *ShaderName, char *name, int TextureNum);

	void						PurgeTexture(IDirect3DBaseTexture9 *texture, int TexNum = -1);

private:
	BuiltInShaderList				BuiltInShaders;
	RuntimeShaderList				RuntimeShaders;
	ShaderList					Shaders;

public:
	ShaderRecord *					idp;  // IDENTIFY.pso
	ShaderRecord *					cqp;  // COPYQUAD.pso
	ShaderRecord *					cqv;  // COPYQUAD.vso
	TextureRecord *					unbound;

//	ShaderConstants					ShaderConst;
	GlobalConstants					GlobalConst;

public:
	void UseShaderOverride(bool yes);
	void SaveShaderOverride(bool yes);
	void UseLegacyCompiler(bool yes);
	void CompileSources(bool yes);
	void RuntimeSources(bool yes);
	void Optimize(bool yes);
	void UpgradeSM(bool yes);
	void MaximumSM(bool yes);

	bool UseShaderOverride();
	bool SaveShaderOverride();
	bool UseLegacyCompiler();
	bool CompileSources();
	bool RuntimeSources();
	bool Optimize();
	bool UpgradeSM();
	bool MaximumSM();

#ifdef	OBGE_DEVLING
	void Clear(int pass = -1, bool t = true) {
	  if (pass == -1) {
	    memset(traced, -1, sizeof(traced));
	    if (!t) return;
	    memset(trackd, -1, sizeof(trackd));

	    for (int p = 0; p < OBGEPASS_NUM; p++)
	      trackd[p].frame_cntr = 0;
	  }
	  else {
	    memset(&traced[pass], -1, sizeof(traced[pass]));
	    if (!t) return;
	    memset(&trackd[pass], -1, sizeof(trackd[pass]));

	    trackd[pass].frame_cntr = 0;
	  }
	}

	/* here we trac scene-wide variables, some of these
	 * are later passed to the shader-trace info because
	 * ty are set even before SetShader()
	 */
	struct trace {
	  DWORD					states_s[OBGESAMPLER_NUM][14];	// sampler states, D3DSAMPLERSTATETYPE 14
//	  DWORD					states_t[OBGESAMPLER_NUM][33];	// texturesstage states, D3DTEXTURESTAGESTATETYPE 33

	  IDirect3DBaseTexture9 *		values_s[OBGESAMPLER_NUM];	// ps 16, vs 4
	  IDirect3DSurface9 *			target_s[OBGESAMPLER_NUM];	// ps 16, vs 4
	} traced[OBGEPASS_NUM];

#define	OBGESCENE_NUM	256
#define	OBGEFRAME_NUM	256

	struct track {
	  /* stats */
	  int					frame_numr;			// sanity
	  int					frame_cntr;			// counter

	  int					frame_used[OBGESCENE_NUM];	// upto 256 scenes
	  int					frame_pass[OBGESCENE_NUM];
	  const char *                          frame_name[OBGESCENE_NUM];
#ifdef	OBGE_PROFILE
	  LARGE_INTEGER				frame_time[OBGESCENE_NUM];
#endif

	  IDirect3DSurface9 *			rt[OBGESCENE_NUM];		// upto 256 scenes
	  IDirect3DSurface9 *			ds[OBGESCENE_NUM];

	  D3DMATRIX				transf[OBGESCENE_NUM][3];	// View & Projection
	  DWORD					states[OBGESCENE_NUM][210];	// ...
	} trackd[OBGEPASS_NUM];

#ifdef	OBGE_PROFILE
	int					frame_capt;
	LARGE_INTEGER				frame_time;

	struct history {
	  LARGE_INTEGER				frame_totl;

	  /* stats */
	  struct track {
	    int					frame_cntr;			// counter

	    LARGE_INTEGER			frame_hist[OBGESCENE_NUM];
	  } trackd[OBGEPASS_NUM];
	} trackh[OBGEFRAME_NUM];
#endif
#endif
};

#else
#include <d3dx9.h>

class ShaderManager {

public:
	ShaderManager();
	~ShaderManager();

	static ShaderManager*		GetSingleton(void);
	static ShaderManager*		Singleton;

	bool						SetRAWZ(bool enabled) {};
	void						OnCreateDevice(void) {};
	void						OnLostDevice(void) {};
	void						OnResetDevice(void) {};
	void						OnReleaseDevice(void) {};
	void						PurgeTexture(IDirect3DBaseTexture9 *texture, int TexNum = -1) {};
};
#endif
