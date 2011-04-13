#pragma once

#include "obse\PluginAPI.h"
#include "d3dx9.h"
#include <vector>
#include "DepthBufferHook.h"
#include "Rendering.h"
#include "nodes\NiDX9Renderer.h"

#define TEXTUREVERSION 1

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

	void SetTexture(IDirect3DTexture9* tex, const char* fp, bool ff = false);
	void SetTexture(IDirect3DCubeTexture9* tex, const char* fp, bool ff = false);
	void SetTexture(IDirect3DVolumeTexture9* tex, const char* fp, bool ff = false);

	IDirect3DBaseTexture9* GetTexture() const;

	bool HasTexture(IDirect3DBaseTexture9* tex) const;
	bool HasTexture(IDirect3DTexture9* tex) const;
	bool HasTexture(IDirect3DCubeTexture9* tex) const;
	bool HasTexture(IDirect3DVolumeTexture9* tex) const;
	bool HasTexture() const;

	TextureRecordType GetType() const;
	bool IsType(TextureRecordType type) const;

	const char *GetPath() const;
	bool IsFromFile() const;

	void Release();
	void Purge();

private:
	TextureRecordType type;
	union {
	  IDirect3DBaseTexture9*	texture;
	  IDirect3DTexture9*		textureP;
	  IDirect3DCubeTexture9*	textureC;
	  IDirect3DVolumeTexture9*	textureV;
	};

	char				Filepath[260];
	bool				FromFile;
};

class StaticTextureRecord : public TextureRecord
{
public:
	StaticTextureRecord();
	~StaticTextureRecord();

	int AddRef();
	int Release();

	int				RefCount;
};

typedef std::vector<TextureRecord*> TextureList;
typedef std::vector<StaticTextureRecord*> StaticTextureList;

class TextureManager
{
public:
	TextureManager();
	~TextureManager();

	static TextureManager*	GetSingleton();
	static TextureManager*	Singleton;

	void				InitialiseFrameTextures(void);
	void				DeviceRelease(void);
	int				LoadTexture(char *Filename, TextureRecordType type, DWORD FromFile);
	StaticTextureRecord*		LoadStaticTexture(char *Filename, TextureRecordType type);
	void				NewGame(void);
	void				LoadGame(OBSESerializationInterface *Interface);
	void				SaveGame(OBSESerializationInterface *Interface);
	bool				IsValidTexture(int TextureNum);
	TextureRecord*			GetTexture(int TextureNum);
	void				FreeTexture(int index);
	template<class IDirect3DTextureType9>
	void				ReleaseTexture(IDirect3DTextureType9* texture);
	template<class IDirect3DTextureType9>
	int				FindTexture(IDirect3DTextureType9* texture);

	TextureList			Textures;
	StaticTextureList		StaticTextures;

	IDirect3DTexture9*		thisframeTex;						
	IDirect3DSurface9*		thisframeSurf;

	IDirect3DTexture9*		lastpassTex;						
	IDirect3DSurface9*		lastpassSurf;					
	
	IDirect3DTexture9*		lastframeTex;
	IDirect3DSurface9*		lastframeSurf;
	
	bool				HasDepth;
	
	IDirect3DTexture9*		depth;
	IDirect3DSurface9*		depthSurface;
	IDirect3DTexture9*		depthRAWZ;
	
	bool				RAWZflag;
};

