#pragma once

#include <vector>

#include "obse\PluginAPI.h"
#include "d3dx9.h"
#include "nodes\NiDX9Renderer.h"

#include "DepthBufferHook.h"
#include "Rendering.h"

#include "D3D9Identifiers.hpp"

#ifndef	NO_DEPRECATED
#define TEXTUREVERSION 1
#else
#define TEXTUREVERSION 2
#endif

typedef enum {
  TR_PLANAR = 0,
  TR_CUBIC = 1,
  TR_VOLUMETRIC = 2
} TextureRecordType;

class TextureRecord
{
public:
	TextureRecord();
	~TextureRecord();

	bool LoadTexture(TextureRecordType type, const char *fp, bool NONPOW2 = true, bool Private = false);

	void SetTexture(IDirect3DTexture9* tex, const char *fp, bool NONPOW2 = true, bool Private = false);
	void SetTexture(IDirect3DCubeTexture9* tex, const char *fp, bool NONPOW2 = true, bool Private = false);
	void SetTexture(IDirect3DVolumeTexture9* tex, const char *fp, bool NONPOW2 = true, bool Private = false);

	IDirect3DBaseTexture9* GetTexture() const;

	bool HasTexture(char *path) const;
	bool HasTexture(IDirect3DBaseTexture9* tex) const;
	bool HasTexture(IDirect3DTexture9* tex) const;
	bool HasTexture(IDirect3DCubeTexture9* tex) const;
	bool HasTexture(IDirect3DVolumeTexture9* tex) const;
	bool HasTexture() const;

	TextureRecordType GetType() const;
	bool IsType(TextureRecordType type) const;

	const char *GetPath() const;
	bool IsNONPOW2() const;
	bool IsPrivate() const;

public:
	void Purge(int TexNum = -1);
	void Kill();

private:
	char				Filepath[MAX_PATH];
	bool				Private;
	UINT32				ParentRefID;
	// Associates a effect with the esp/esm file the script the effect was created in.

	bool				NONPOW2;
	TextureRecordType		type;
	union {
	  IDirect3DBaseTexture9*	texture;
	  IDirect3DTexture9*		textureP;
	  IDirect3DCubeTexture9*	textureC;
	  IDirect3DVolumeTexture9*	textureV;
	};
};

class ManagedTextureRecord : public TextureRecord
{
public:
	ManagedTextureRecord();
	~ManagedTextureRecord();

	void ClrRef();
	int AddRef();
	int Release();
	int RefCount;
};

//pedef std::vector<TextureRecord *> TextureList;
typedef std::vector<ManagedTextureRecord *> ManagedTextureList;
typedef std::map<int, ManagedTextureRecord *> TextureRegistry;

class TextureManager
{
public:
	TextureManager();
	~TextureManager();

	static TextureManager *GetSingleton();
	static TextureManager *Singleton;

	void						NewGame(void);
	void						LoadGame(OBSESerializationInterface *Interface);
	void						SaveGame(OBSESerializationInterface *Interface);

private:
	void						Clear();

public:
	int						LoadPrivateTexture(const char *Filename, TextureRecordType type, bool NONPOW2 = true);
	int						LoadManagedTexture(const char *Filename, TextureRecordType type, bool NONPOW2 = true);
	int						LoadDependtTexture(const char *Filename, TextureRecordType type, bool NONPOW2 = true);
	inline bool					IsTextureValid(int TextureNum) const { return Textures.count(TextureNum) != 0; };
	inline ManagedTextureRecord *			GetTexture(int TextureNum) { return (IsTextureValid(TextureNum) ? Textures[TextureNum] : NULL); };
	bool						ReleaseTexture(int TextureNum);
	void						FreeTexture(int TextureNum);
	template<class IDirect3DTextureType9> bool	ReleaseTexture(IDirect3DTextureType9 *texture);
	template<class IDirect3DTextureType9> int	FindTexture(IDirect3DTextureType9 *texture);

private:
	int						TextureIndex;
	int						MaxTextureIndex;

	TextureRegistry					Textures;
//	EffectList					Textures;
	ManagedTextureList				ManagedTextures;

public:
#ifdef OBGE_GAMMACORRECTION
	void						DoDeGamma(bool enable);
	void						DoReGamma(bool enable);
	bool						DoDeGamma();
	bool						DoReGamma();
#endif

#ifdef	OBGE_ANISOTROPY
	void 						SetAnisotropy(int af);
	void						SetLODBias(float bias);
	int						SetAnisotropy();
	float						SetLODBias();
#endif
};

