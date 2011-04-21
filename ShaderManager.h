#pragma once

#include "d3dx9.h"
#include "D3DX9Shader.h"
#include <vector>
#include <map>
#include <utility>
#include "D3D9.hpp"

class ShaderRecord
{
public:
	ShaderRecord();
	~ShaderRecord();

	void						SetBinary(int len, void *org);
	void *						GetBinary();

	bool						LoadShader(char *Filename);
	bool						SaveShader();
	bool						AssembleShader();
	bool						CompileShader();
	bool						DisassembleShader();
	void						ApplyCompileDirectives(void);

	char						Name[100];
	char						Filepath[MAX_PATH];
	bool						Replaced;

	/* source-code buffers (asm or HLSL) */
	bool						bAssembler;
	LPSTR 						pAssembly;
	UINT						assemblyLen;

	bool						bHLSL;
	LPSTR 						pSource;
	UINT						sourceLen;

	LPD3DXBUFFER					pDisassembly;

	/* version and flags */
	LPSTR						pProfile;
	bool						bPartialPrecision;

	/* compiled results */
	LPD3DXBUFFER					pShaderOriginal;
	LPD3DXBUFFER					pShaderReplaced;

	/* D3DXGetShaderConstantTableEx() */
	LPD3DXCONSTANTTABLE				pConstantsOriginal;
	LPD3DXCONSTANTTABLE				pConstantsReplaced;

	LPD3DXBUFFER					pErrorMsgs;
};

typedef std::vector<ShaderRecord*> BuiltInShaderList;
typedef std::map<int, ShaderRecord*> ShaderList;

class ShaderManager
{
private:
	ShaderManager();
public:
	~ShaderManager();

	static ShaderManager*		GetSingleton(void);
	static ShaderManager*		Singleton;

	ShaderRecord*					GetBuiltInShader(char *Filename);

	BuiltInShaderList				BuiltInShaders;
	ShaderList					Shaders;
};