#pragma once

#include <d3dx9.h>
#include <d3dx9shader.h>

#include <vector>
#include <map>
#include <list>
#include <queue>
#include <utility>

#include "Nodes/NiVector4.h"
#include "OBGE fork/Sky.h"
#include "Nodes/NiBillboardNode.h"

#include "D3D9Identifiers.hpp"

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
	bool						RuntimeFlush();
	bool						RuntimeShader(const char *hlsl, const char *version = NULL);
	bool						CompareShader(const DWORD *Function) const;

	bool						AssembleShader(bool forced = false);
	bool						CompileShader(bool forced = false);
	bool						DisassembleShader(bool forced = false);
	bool						SaveShader();

	bool						ConstructDX9Shader(char which);
	DWORD *						GetDX9ShaderTexture(const char *sName, int *TexNum, DWORD *States);
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

	void Release();
	void OnLostDevice(void);
	void OnResetDevice(void);

	void CreateRuntimeParams(LPD3DXCONSTANTTABLE CoTa);
	void SetRuntimeParams(IDirect3DDevice9 *StateDevice, IDirect3DDevice9 *SceneDevice);

	bool SetShaderConstantB(const char *name, bool value);
	bool SetShaderConstantI(const char *name, int *values);
	bool SetShaderConstantF(const char *name, float *values);
	bool SetShaderSamplerTexture(const char *name, int TextureNum);

	bool AssignShader(IUnknown *Shader, ShaderRecord *Associate);
	bool ActivateShader(char which);

	IDirect3DPixelShader9  *GetShader(IDirect3DPixelShader9  *Shader) const;
	IDirect3DVertexShader9 *GetShader(IDirect3DVertexShader9 *Shader) const;

public:
	ShaderRecord *			pAssociate;
	bool				bActive;
	void *				pCustomCT;

	/* get a copy of the z-buffer right from before and pass it to the shader */
	IDirect3DSurface9 *		pGrabRT;
	IDirect3DSurface9 *		pGrabDS;
	IDirect3DSurface9 *		pGrabDZ;
	IDirect3DTexture9 **		pTextRT;
	IDirect3DTexture9 **		pTextDS;
	IDirect3DTexture9 **		pTextDZ;

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
	  if (pass == -1) {
	    memset(traced, -1, sizeof(traced));
	  }
	  else {
	    memset(&traced[pass], -1, sizeof(traced[pass]));
	  }
	}

	/* stats */
	int						frame_used[OBGEPASS_NUM];
	int						frame_pass[OBGEPASS_NUM];

#define	OBGESAMPLER_NUM	16

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

typedef std::list<RuntimeShaderRecord*> RuntimeShaderList;
typedef std::list<ShaderRecord*> BuiltInShaderList;
typedef std::map<IUnknown*, RuntimeShaderRecord*> ShaderList;

struct ShaderConstants
{
	// ****** Global static shader constants ******
	v1_2_416::NiVector4		rcpres;
	v1_2_416::NiVector4		rcpresh;	// heightmap
	v1_2_416::NiVector4		rcpresd;	// displacement

	// ****** Global shader constants (Updated each scene) ******
	D3DXMATRIX			wrld;
	D3DXMATRIX			view;
	D3DXMATRIX			proj;

	v1_2_416::NiVector4		SunDir;
	v1_2_416::NiVector4		GameTime;
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

	void						OnLostDevice(void);
	void						OnResetDevice(void);
	void						OnReleaseDevice(void);

private:
	void						Reset();
public:
	void						UpdateFrameConstants();

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

private:
	BuiltInShaderList				BuiltInShaders;
	RuntimeShaderList				RuntimeShaders;
	ShaderList					Shaders;

public:
	ShaderConstants					ShaderConst;

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

	struct track {
	  /* stats */
	  int					frame_numr;			// sanity
	  int					frame_cntr;			// counter

	  int					frame_used[OBGESCENE_NUM];	// upto 256 scenes
	  int					frame_pass[OBGESCENE_NUM];
#ifdef	OBGE_PROFILE
	  LARGE_INTEGER				frame_time[OBGESCENE_NUM];
#endif

	  IDirect3DSurface9 *			rt[OBGESCENE_NUM];		// upto 256 scenes
	  IDirect3DSurface9 *			ds[OBGESCENE_NUM];

	  D3DMATRIX				transf[OBGESCENE_NUM][3];	// View & Projection
	  DWORD					states[OBGESCENE_NUM][210];	// ...
	} trackd[OBGEPASS_NUM];
#endif
};