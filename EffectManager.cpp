#include "EffectManager.h"
#include "TextureManager.h"
#include "GlobalSettings.h"
#include "D3D9.hpp"

static global<bool> UseShaderList(true,NULL,"Effects","bUseShaderList");
static global<char*> ShaderListFile("data\\shaders\\shaderlist.txt",NULL,"Effects","sShaderListFile");
static global<bool> UseLegacyCompiler(false,NULL,"Effects","bUseLegacyCompiler");
static global<bool> Optimize(false,NULL,"Effects","bOptimize");
static global<bool> SplitScreen(false,NULL,"Effects","bRenderHalfScreen");

EffectRecord::EffectRecord()
{
	Name[0]=0;
	Effect=NULL;
	Enabled=false;
	ParentRefID = 0xFF000000;
}

EffectRecord::~EffectRecord()
{
	if(Effect)
		while(Effect->Release()){};
}

void EffectRecord::Render(IDirect3DDevice9*	D3DDevice,IDirect3DSurface9 *RenderTo)
{
	if(!Enabled)
		return;
	UINT passes;

	Effect->Begin(&passes,NULL);
	UINT pass=0;
	while(true)
	{
		Effect->BeginPass(pass);
		D3DDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
		Effect->EndPass();
		if(++pass>=passes)
			break;
		D3DDevice->StretchRect(RenderTo,0,TextureManager::GetSingleton()->lastpassSurf,0,D3DTEXF_NONE);
	}
	Effect->End();
	return;
}

void EffectRecord::OnLostDevice(void)
{
	if(Effect)
		Effect->OnLostDevice();
	return;
}

void EffectRecord::OnResetDevice(void)
{
	if(Effect)
		Effect->OnResetDevice();
	return;
}

void EffectRecord::ApplyConstants(Constants *ConstList)
{
	Effect->SetMatrix("m44world",&ConstList->world);
	Effect->SetMatrix("m44view",&ConstList->view);
	Effect->SetMatrix("m44proj",&ConstList->proj);
	Effect->SetVector("f4Time",&ConstList->time);
	Effect->SetVector("f4SunDir",&ConstList->SunDir);
	Effect->SetFloatArray("f3EyeForward",&ConstList->EyeForward.x,3);
	return;
}

void EffectRecord::ApplyDynamics()
{
	Effect->SetTexture("reflection",passTexture[OBGEPASS_REFLECTION]);
	Effect->SetBool("bHasReflection",!!passTexture[OBGEPASS_REFLECTION]);
	return;
}

bool EffectRecord::IsEnabled()
{
	return(Enabled);
}

bool EffectRecord::LoadEffect(char *Filename)
{
	HRESULT hr;

	if(Effect)
	{
		while(Effect->Release()){};
		Name[0]=0;
	}

	if(strlen(Filename)>240)
		return(false);

	char NewPath[260];
	strcpy(NewPath,"data\\shaders\\");
	strcat(NewPath,Filename);

	_MESSAGE("Loading effect (%s)",NewPath);

	LPD3DXBUFFER pCompilationErrors=0;
	if(UseLegacyCompiler.data)
		hr=D3DXCreateEffectFromFileA(GetD3DDevice(),NewPath,0,0,D3DXFX_NOT_CLONEABLE|D3DXSHADER_USE_LEGACY_D3DX9_31_DLL,0,&Effect,&pCompilationErrors);
	else
		hr=D3DXCreateEffectFromFileA(GetD3DDevice(),NewPath,0,0,D3DXFX_NOT_CLONEABLE|(Optimize.Get() ? D3DXSHADER_OPTIMIZATION_LEVEL3 : 0),0,&Effect,&pCompilationErrors);

	if(hr!=D3D_OK && pCompilationErrors && !UseLegacyCompiler.data)
	{
		pCompilationErrors->Release();
		pCompilationErrors=NULL;
		_MESSAGE("Effect compilation errors occured - trying legacy compiler.");
		hr=D3DXCreateEffectFromFileA(GetD3DDevice(),NewPath,0,0,D3DXFX_NOT_CLONEABLE|D3DXSHADER_USE_LEGACY_D3DX9_31_DLL,0,&Effect,&pCompilationErrors);
	}

	if (hr!=D3D_OK)
	{
		if(pCompilationErrors)
		{
			_MESSAGE("Effect compilation errors occured:");
			_MESSAGE(Filename);
			_MESSAGE((char*)pCompilationErrors->GetBufferPointer());
			pCompilationErrors->Release();
		}
		else
		{
			_MESSAGE("Failed to load.");
		}
	return(false);
	}

	ApplyCompileDirectives();
	strcpy(Filepath,Filename);
	Enabled=true;
	return(true);
}

void EffectRecord::ApplyCompileDirectives()
{
  LPCSTR pName=NULL;
  Effect->GetString("Name",&pName);
  if (pName)
    strcpy(Name,(char *)pName);

  D3DXEFFECT_DESC Description;
  Effect->GetDesc(&Description);

  for (int par=0;par<Description.Parameters;par++)
  {
    D3DXHANDLE handle;
    handle = Effect->GetParameter(NULL,par);

    if(handle)
    {
      D3DXPARAMETER_DESC Description;
      Effect->GetParameterDesc(handle,&Description);

      if (Description.Type==D3DXPT_TEXTURECUBE)
      {
	D3DXHANDLE handle2;
	handle2 = Effect->GetAnnotationByName(handle,"filename");

	if (handle2)
	{
	  LPCSTR	pString=NULL;
	  Effect->GetString(handle2,&pString);

	  _MESSAGE("Found filename : %s",pString);

	  StaticTextureRecord *Tex = TextureManager::GetSingleton()->LoadStaticTexture((char *)pString, TR_CUBIC);
	  if (Tex)
	    Effect->SetTexture(handle,Tex->GetTexture());
	}
      }
      else if (Description.Type==D3DXPT_TEXTURE3D)
      {
	D3DXHANDLE handle2;
	handle2 = Effect->GetAnnotationByName(handle,"filename");

	if (handle2)
	{
	  LPCSTR	pString=NULL;
	  Effect->GetString(handle2,&pString);

	  _MESSAGE("Found filename : %s",pString);

	  StaticTextureRecord *Tex = TextureManager::GetSingleton()->LoadStaticTexture((char *)pString, TR_VOLUMETRIC);
	  if (Tex)
	    Effect->SetTexture(handle,Tex->GetTexture());
	}
      }
      else if ((Description.Type==D3DXPT_TEXTURE) ||
	       (Description.Type==D3DXPT_TEXTURE1D) ||
	       (Description.Type==D3DXPT_TEXTURE2D))
      {
	D3DXHANDLE handle2;
	handle2 = Effect->GetAnnotationByName(handle,"filename");

	if (handle2)
	{
	  LPCSTR	pString=NULL;
	  Effect->GetString(handle2,&pString);

	  _MESSAGE("Found filename : %s",pString);

	  StaticTextureRecord *Tex = TextureManager::GetSingleton()->LoadStaticTexture((char *)pString, TR_PLANAR);
	  if (Tex)
	    Effect->SetTexture(handle,Tex->GetTexture());
	}
      }
    }
  }
}

bool EffectRecord::SetEffectInt(char *name, int value)
{
	HRESULT hr=Effect->SetInt(name,value);
	return(hr==D3D_OK);
}

bool EffectRecord::SetEffectFloat(char *name, float value)
{
	HRESULT hr=Effect->SetFloat(name,value);
	return(hr==D3D_OK);
}

bool EffectRecord::SetEffectVector(char *name,v1_2_416::NiVector4 *value)
{
	HRESULT hr=Effect->SetVector(name,value);
	return(hr==D3D_OK);
}

bool EffectRecord::SetEffectTexture(char *name, int TextureNum)
{
	TextureRecord* texture;
	texture = TextureManager::GetSingleton()->GetTexture(TextureNum);

	IDirect3DBaseTexture9* OldTexture = NULL;
	Effect->GetTexture(name, (LPDIRECT3DBASETEXTURE9 *)&OldTexture);
	if(OldTexture)
		TextureManager::GetSingleton()->ReleaseTexture(OldTexture);

	HRESULT hr = Effect->SetTexture(name,texture->GetTexture());
	return (hr == D3D_OK);
}

void EffectRecord::SaveVars(OBSESerializationInterface *Interface)
{
	D3DXEFFECT_DESC Description;
	IDirect3DBaseTexture9 *Texture;

	Effect->GetDesc(&Description);
	_MESSAGE("Effect %s has %i parameters.",Filepath,Description.Parameters);
	for (int par=0;par<Description.Parameters;par++)
	{
		D3DXHANDLE	handle;
		handle=Effect->GetParameter(NULL,par);
		if(handle)
		{
			D3DXPARAMETER_DESC Description;
			Effect->GetParameterDesc(handle,&Description);
			switch (Description.Type)
			{
			case D3DXPT_TEXTURE:
			case D3DXPT_TEXTURE1D:
			case D3DXPT_TEXTURE2D:
			case D3DXPT_TEXTURE3D:
			case D3DXPT_TEXTURECUBE:

				int tex;
				Texture=NULL;

				TextureType TextureData;

				Effect->GetTexture(handle, &Texture);
				tex=TextureManager::GetSingleton()->FindTexture(Texture);
				strcpy(TextureData.Name,Description.Name);
				if(tex>0)
				{
					TextureData.tex=tex;
					Interface->WriteRecord('STEX',SHADERVERSION,&TextureData,sizeof(TextureData));
					_MESSAGE("Found texture: name - %s, texnum - %n",TextureData.Name,tex);
				}
				else
				{
					_MESSAGE("Found texture: name - %s - not in texture list.",TextureData.Name);
				}
				break;
			case D3DXPT_INT:

				IntType IntData;

				IntData.size=Description.Elements;
				if (IntData.size==0)
					IntData.size=1;
				Effect->GetIntArray(handle,(int *)&IntData.data,IntData.size);
				strcpy(IntData.Name,Description.Name);
				Interface->WriteRecord('SINT',SHADERVERSION,&IntData,sizeof(IntData));
				_MESSAGE("Found int: name - %s, size - %i, data[0] - %i",IntData.Name,IntData.size, IntData.data[0]);
				break;
			case D3DXPT_FLOAT:

				FloatType FloatData;

				FloatData.size=Description.Elements;
				if(FloatData.size==0)
					FloatData.size=1;
				Effect->GetFloatArray(handle,(float *)&FloatData.data,FloatData.size);
				strcpy(FloatData.Name,Description.Name);
				Interface->WriteRecord('SFLT',SHADERVERSION,&FloatData,sizeof(FloatData));
				_MESSAGE("Found float: name - %s, size - %i, data[0] - %f",FloatData.Name,FloatData.size, FloatData.data[0]);
				break;
			}
		}
	}
}

// *********************************************************************************************************

EffectManager *EffectManager::Singleton=NULL;

EffectManager::EffectManager()
{
	EffectIndex=0;
	MaxEffectIndex=0;
	DepthEffect=NULL;
}

EffectManager::~EffectManager()
{
	Singleton=NULL;
	if(D3D_EffectBuffer)
		while(D3D_EffectBuffer->Release()){};
}

EffectManager*	EffectManager::GetSingleton()
{
	if(!EffectManager::Singleton)
		EffectManager::Singleton=new(EffectManager);
	return(EffectManager::Singleton);
}

void EffectManager::UpdateFrameConstants()
{
	v1_2_416::NiDX9Renderer *Renderer = v1_2_416::GetRenderer();

	OBGEfork::Sun *pSun = OBGEfork::Sky::GetSingleton()->sun;
	float (_cdecl *GetTimer)(bool, bool)=(float(*)(bool, bool))0x0043F490; // (TimePassed,GameTime)
	v1_2_416::NiCamera **pMainCamera = (v1_2_416::NiCamera **)0x00B43124;
	char *CamName;

	Renderer->SetCameraViewProj(*pMainCamera);
	D3DXMatrixTranslation(&EffectConst.world,-(*pMainCamera)->m_worldTranslate.x,-(*pMainCamera)->m_worldTranslate.y,-(*pMainCamera)->m_worldTranslate.z);

	EffectConst.view = (D3DXMATRIX)Renderer->m44View;
	EffectConst.proj = (D3DXMATRIX)Renderer->m44Projection;

	CamName = (*pMainCamera)->m_pcName;
	(*pMainCamera)->m_worldRotate.GetForwardVector(&EffectConst.EyeForward);

	EffectConst.time.x = GetTimer(0,1);
	EffectConst.time.w = (int)EffectConst.time.x%60;
	EffectConst.time.z = (int)(EffectConst.time.x/60)%60;
	EffectConst.time.y = (int)(EffectConst.time.x/60)/60;

	EffectConst.SunDir.x = pSun->SunBillboard.Get()->ParentNode->m_localTranslate.x;
	EffectConst.SunDir.y = pSun->SunBillboard.Get()->ParentNode->m_localTranslate.y;
	EffectConst.SunDir.z = pSun->SunBillboard.Get()->ParentNode->m_localTranslate.z;
	EffectConst.SunDir.Normalize3();
}

void EffectManager::Render(IDirect3DDevice9 *D3DDevice,IDirect3DSurface9 *RenderTo, IDirect3DSurface9 *RenderFrom)
{
	v1_2_416::NiDX9Renderer *Renderer = v1_2_416::GetRenderer();

	D3DDevice->SetStreamSource(0,D3D_EffectBuffer,0,sizeof(D3D_sShaderVertex));
	Renderer->RenderStateManager->SetFVF(MYVERTEXFORMAT,false);

	// Sets up the viewport.
	float test[4] = { 0.0, 1.0, 1.0, 0.0 };
	Renderer->SetupScreenSpaceCamera(test);

	Renderer->RenderStateManager->SetRenderState(D3DRS_COLORWRITEENABLE,0xF,false);
	Renderer->RenderStateManager->SetRenderState(D3DRS_ALPHABLENDENABLE,false,false);
	Renderer->RenderStateManager->SetRenderState(D3DRS_ZENABLE,D3DZB_FALSE,false);
	Renderer->RenderStateManager->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE,false);

	UpdateFrameConstants();

	TextureManager* TexMan = TextureManager::GetSingleton();

	// Set up world/view/proj matrices to identity in case there's no vertex effect.
	D3DXMATRIX mIdent;
	D3DXMatrixIdentity(&mIdent);

	D3DDevice->SetTransform(D3DTS_PROJECTION, &mIdent);
	D3DDevice->SetTransform(D3DTS_VIEW, &mIdent);
	D3DDevice->SetTransform(D3DTS_WORLD,&mIdent);

	if(TexMan->RAWZflag)
	{
		D3DDevice->EndScene();
		D3DDevice->SetRenderTarget(0,TexMan->depthSurface);
		D3DDevice->BeginScene();
		RenderRAWZfix(D3DDevice,RenderTo);
		D3DDevice->EndScene();
		D3DDevice->SetRenderTarget(0,RenderTo);
		D3DDevice->BeginScene();
	}

	D3DDevice->StretchRect(RenderFrom,0,TexMan->thisframeSurf,0,D3DTEXF_NONE);
	D3DDevice->StretchRect(RenderFrom,0,RenderTo,0,D3DTEXF_NONE); // Blank screen fix when EffectList is empty.

	if(UseShaderList.data)
	{
		StaticEffectList::iterator SEffect=StaticEffects.begin();
		while(SEffect!=StaticEffects.end())
		{
			if((*SEffect)->IsEnabled())
			{
				(*SEffect)->ApplyConstants(&EffectConst);
				(*SEffect)->ApplyDynamics();
				D3DDevice->SetVertexShader(NULL);

				// Have to do this in case the effect has no vertex effect. The stupid effect system
				// uses the last vertex effect that was active and much strangeness occurs.
				(*SEffect)->Render(D3DDevice,RenderTo);
				D3DDevice->StretchRect(RenderTo,0,TexMan->thisframeSurf,0,D3DTEXF_NONE);
			}
			SEffect++;
		}
	}

	EffectList::iterator Effect=Effects.begin();
	while(Effect!=Effects.end())
	{
		if(Effect->second->IsEnabled())
		{
			Effect->second->ApplyConstants(&EffectConst);
			Effect->second->ApplyDynamics();
			D3DDevice->SetVertexShader(NULL);

			Effect->second->Render(D3DDevice,RenderTo);
			D3DDevice->StretchRect(RenderTo,0,TexMan->thisframeSurf,0,D3DTEXF_NONE);
		}
		Effect++;
	}

	D3DDevice->StretchRect(RenderTo,0,TexMan->lastframeSurf,0,D3DTEXF_NONE);

	return;
}

void EffectManager::RenderRAWZfix(IDirect3DDevice9* D3DDevice,IDirect3DSurface9 *RenderTo)
{
	if(!DepthEffect)
	{
		DepthEffect=new(EffectRecord);
		if(!DepthEffect->LoadEffect("RAWZfix.fx"))
		{
			_MESSAGE("ERROR - RAWZfix.fx is missing! Please reinstall OBGEv2.2");
			return;
		}
		DepthEffect->Effect->SetTexture("RAWZdepth",TextureManager::GetSingleton()->depthRAWZ);
	}
	DepthEffect->Render(D3DDevice,RenderTo);
	return;
}

int EffectManager::AddEffect(char *Filename, bool AllowDuplicates, UINT32 refID)
{
	EffectList::iterator Effect=Effects.begin();

	if (!AllowDuplicates)
	{
		while(Effect!=Effects.end())
		{
			if(!_stricmp(Effect->second->Filepath,Filename)&&((Effect->second->ParentRefID&0xff000000)==(refID&0xff000000)))
			{
				_MESSAGE("Loading effect that already exists. Returning index of existing effect.");
				return(Effect->first);
			}
			Effect++;
		}
	}

	EffectRecord	*NewEffect=new(EffectRecord);

	while(1)
	{
		Effect=Effects.find(EffectIndex);
		if (Effect==Effects.end())
			break;
		EffectIndex++;
	}

	if(!NewEffect->LoadEffect(Filename))
	{
		delete(NewEffect);
		return(-1);
	}

	_MESSAGE("Setting effects screen texture.");
	TextureManager	*TexMan=TextureManager::GetSingleton();
	NewEffect->Effect->SetTexture("reflection",passTexture[OBGEPASS_REFLECTION]);
	NewEffect->Effect->SetTexture("thisframe",TexMan->thisframeTex);
	NewEffect->Effect->SetTexture("lastpass",TexMan->lastpassTex);
	NewEffect->Effect->SetTexture("lastframe",TexMan->lastframeTex);
	NewEffect->Effect->SetTexture("Depth",TexMan->depth);
	NewEffect->Effect->SetFloatArray("rcpres",(float*)&EffectConst.rcpres,2);
	NewEffect->Effect->SetBool("bHasReflection",!!passTexture[OBGEPASS_REFLECTION]);
	NewEffect->Effect->SetBool("bHasDepth",EffectConst.bHasDepth);

	NewEffect->ParentRefID=refID;

	Effects.insert(std::make_pair(EffectIndex,NewEffect));

	Console_Print("Loaded effect number %i",EffectIndex);
	return(EffectIndex++);
}

bool EffectManager::AddStaticEffect(char *Filename)
{
	EffectRecord	*NewEffect=NULL;

	NewEffect=new(EffectRecord);

	if(!NewEffect->LoadEffect(Filename))
	{
		delete(NewEffect);
		return(false);
	}

	_MESSAGE("Setting effects screen texture.");
	TextureManager	*TexMan=TextureManager::GetSingleton();
	NewEffect->Effect->SetTexture("reflection",passTexture[OBGEPASS_REFLECTION]);
	NewEffect->Effect->SetTexture("thisframe",TexMan->thisframeTex);
	NewEffect->Effect->SetTexture("lastpass",TexMan->lastpassTex);
	NewEffect->Effect->SetTexture("lastframe",TexMan->lastframeTex);
	NewEffect->Effect->SetTexture("Depth",TexMan->depth);
	NewEffect->Effect->SetFloatArray("rcpres",(float*)&EffectConst.rcpres,2);
	NewEffect->Effect->SetBool("bHasReflection",!!passTexture[OBGEPASS_REFLECTION]);
	NewEffect->Effect->SetBool("bHasDepth",EffectConst.bHasDepth);

	StaticEffects.push_back(NewEffect);

	return(true);
}

bool EffectManager::RemoveEffect(int EffectNum)
{
	if(Effects.erase(EffectNum))
	{
		return(true);
	}
	return(false);
}

void EffectManager::InitialiseBuffers()
{
	float minx,minu,uadj,vadj;

	EffectConst.rcpres[0]=1.0f/(float)v1_2_416::GetRenderer()->SizeWidth;
	EffectConst.rcpres[1]=1.0f/(float)v1_2_416::GetRenderer()->SizeHeight;

	uadj=EffectConst.rcpres[0]*0.5;
	vadj=EffectConst.rcpres[1]*0.5;

	if(SplitScreen.data)
	{
		minx = 0;
		minu = 0.5;
	}
	else
	{
		minx = -1;
		minu = 0;
	}

	D3D_sShaderVertex ShaderVertices[] =
	{
		{minx ,+1 ,1, minu+uadj ,0+vadj},
		{minx ,-1 ,1, minu+uadj ,1+vadj},
		{1    ,+1 ,1, 1   +uadj ,0+vadj},
		{1    ,-1 ,1, 1   +uadj ,1+vadj}
	};

	_MESSAGE("Creating vertex buffers.");
	GetD3DDevice()->CreateVertexBuffer(4*sizeof(D3D_sShaderVertex),D3DUSAGE_WRITEONLY,MYVERTEXFORMAT,D3DPOOL_DEFAULT,&D3D_EffectBuffer,0);
	void* VertexPointer;

	D3D_EffectBuffer->Lock(0,0,&VertexPointer,0);
	CopyMemory(VertexPointer,ShaderVertices,sizeof(ShaderVertices));
	D3D_EffectBuffer->Unlock();

	EffectConst.bHasDepth=HasDepth();

	return;
}

void EffectManager::DeviceRelease()
{
	if(D3D_EffectBuffer)
	{
		_MESSAGE("Releasing effect vertex buffer.");
		while(D3D_EffectBuffer->Release()){}
		D3D_EffectBuffer=NULL;
	}

	StaticEffectList::iterator SEffect=StaticEffects.begin();
	while(SEffect!=StaticEffects.end())
	{
		while((*SEffect)->Effect->Release()){}
		(*SEffect)->Effect=NULL;
		SEffect++;
	}
	StaticEffects.clear();

	EffectList::iterator Effect=Effects.begin();
	while(Effect!=Effects.end())
	{
		while(Effect->second->Effect->Release()){}
		Effect->second->Effect=NULL;
		Effect++;
	}
	Effects.clear();

	if(DepthEffect)
	{
		while(DepthEffect->Effect->Release()){}
		DepthEffect->Effect=NULL;
	}
}

void EffectManager::OnLostDevice()
{
	if(D3D_EffectBuffer)
	{
		_MESSAGE("Releasing effect vertex buffer.");
		while(D3D_EffectBuffer->Release()){}
		D3D_EffectBuffer=NULL;
	}

	StaticEffectList::iterator SEffect=StaticEffects.begin();
	while(SEffect!=StaticEffects.end())
	{
		(*SEffect)->OnLostDevice();
		SEffect++;
	}

	EffectList::iterator Effect=Effects.begin();
	while(Effect!=Effects.end())
	{
		Effect->second->OnLostDevice();
		Effect++;
	}

	if(DepthEffect)
		DepthEffect->OnLostDevice();
}

void EffectManager::OnResetDevice()
{
	TextureManager	*TexMan=TextureManager::GetSingleton();

	InitialiseBuffers();

	StaticEffectList::iterator SEffect=StaticEffects.begin();
	while(SEffect!=StaticEffects.end())
	{
		(*SEffect)->OnResetDevice();
		(*SEffect)->Effect->SetTexture("reflection",passTexture[OBGEPASS_REFLECTION]);
		(*SEffect)->Effect->SetTexture("thisframe",TexMan->thisframeTex);
		(*SEffect)->Effect->SetTexture("lastpass",TexMan->lastpassTex);
		(*SEffect)->Effect->SetTexture("lastframe",TexMan->lastframeTex);
		(*SEffect)->Effect->SetTexture("Depth",TexMan->depth);
		(*SEffect)->Effect->SetFloatArray("rcpres",(float*)&EffectConst.rcpres,2);
		(*SEffect)->Effect->SetBool("bHasReflection",!!passTexture[OBGEPASS_REFLECTION]);
		(*SEffect)->Effect->SetBool("bHasDepth",EffectConst.bHasDepth);
		SEffect++;
	}

	EffectList::iterator Effect=Effects.begin();
	while(Effect!=Effects.end())
	{
		Effect->second->OnResetDevice();
		Effect->second->Effect->SetTexture("reflection",passTexture[OBGEPASS_REFLECTION]);
		Effect->second->Effect->SetTexture("thisframe",TexMan->thisframeTex);
		Effect->second->Effect->SetTexture("lastpass",TexMan->lastpassTex);
		Effect->second->Effect->SetTexture("lastframe",TexMan->lastframeTex);
		Effect->second->Effect->SetTexture("Depth",TexMan->depth);
		Effect->second->Effect->SetFloatArray("rcpres",(float*)&EffectConst.rcpres,2);
		Effect->second->Effect->SetBool("bHasReflection",!!passTexture[OBGEPASS_REFLECTION]);
		Effect->second->Effect->SetBool("bHasDepth",EffectConst.bHasDepth);
		Effect++;
	}
}

void EffectManager::LoadEffectList()
{
	FILE *EffectFile;
	char EffectBuffer[260];
	int lastpos;

	if(UseShaderList.data)
	{
		_MESSAGE("Loading the effects.");
		if(!fopen_s(&EffectFile,ShaderListFile.Get(),"rt"))
		{
			while(!feof(EffectFile))
			{
				if(fgets(EffectBuffer,260,EffectFile))
				{
					lastpos=strlen(EffectBuffer)-1;
					if (EffectBuffer[lastpos]==10||EffectBuffer[lastpos]==13)
						EffectBuffer[lastpos]=0;
					AddStaticEffect(EffectBuffer);
				}
			}
			fclose(EffectFile);
		}
		else
		{
			_MESSAGE("Error opening shaderlist.txt file.");
		}
	}
	else
	{
		_MESSAGE("EffectList has been disabled by the INI file.");
	}
}

void EffectManager::NewGame()
{
	StaticEffectList::iterator SEffect=StaticEffects.begin();

	while(SEffect!=StaticEffects.end())
	{
		while((*SEffect)->Effect->Release()){}
		(*SEffect)->Effect=NULL;
		SEffect++;
	}

	StaticEffects.clear();

	EffectList::iterator Effect=Effects.begin();

	while(Effect!=Effects.end())
	{
		while(Effect->second->Effect->Release()){}
		Effect->second->Effect=NULL;
		Effect++;
	}

	Effects.clear();
}

void EffectManager::SaveGame(OBSESerializationInterface *Interface)
{
	int temp;

	_MESSAGE("EffectManager::SaveGame");

	EffectList::iterator Effect=Effects.begin();

	Interface->WriteRecord('SIDX',SHADERVERSION,&EffectIndex,sizeof(EffectIndex));
	_MESSAGE("Effect index = %i",EffectIndex);

	while(Effect!=Effects.end())
	{
		if(Effect->second->Effect)
		{
			Interface->WriteRecord('SNUM',SHADERVERSION,&Effect->first,sizeof(Effect->first));
			Interface->WriteRecord('SPAT',SHADERVERSION,Effect->second->Filepath,strlen(Effect->second->Filepath)+1);
			Interface->WriteRecord('SENB',SHADERVERSION,&Effect->second->Enabled,sizeof(Effect->second->Enabled));
			Interface->WriteRecord('SREF',SHADERVERSION,&Effect->second->ParentRefID,sizeof(Effect->second->ParentRefID));
			Effect->second->SaveVars(Interface);
			Interface->WriteRecord('SEOD',SHADERVERSION,&temp,1);
		}
		Effect++;
	}
	Interface->WriteRecord('SEOF',SHADERVERSION,&temp,1);
}

void EffectManager::LoadGame(OBSESerializationInterface *Interface)
{
	UInt32	type, version, length;
	int LoadShaderNum;
	char LoadFilepath[260];
	bool LoadEnabled;
	UInt32 LoadRefID;
	bool InUse;

	Interface->GetNextRecordInfo(&type, &version, &length);

	if (type=='SIDX')
	{
		Interface->ReadRecordData(&EffectIndex,length);
		_MESSAGE("Effect Index = %i",EffectIndex);
	}
	else
	{
		_MESSAGE("No effect data in save file.");
		return;
	}

	Interface->GetNextRecordInfo(&type, &version, &length);

	while(type!='SEOF')
	{
		if(type=='SNUM')
		{
			Interface->ReadRecordData(&LoadShaderNum,length);
			_MESSAGE("Effect num = %i",LoadShaderNum);
		}
		else
		{
			_MESSAGE("Error loading effect list. type!=SNUM");
			return;
		}

		Interface->GetNextRecordInfo(&type, &version, &length);

		if(type=='SPAT')
		{
			Interface->ReadRecordData(LoadFilepath,length);
			_MESSAGE("Filename = %s",LoadFilepath);
		}
		else
		{
			_MESSAGE("Error loading effect list. type!=SPAT");
			return;
		}

		Interface->GetNextRecordInfo(&type, &version, &length);

		if(type=='SENB')
		{
			Interface->ReadRecordData(&LoadEnabled,length);
			_MESSAGE("Enabled = %i",LoadEnabled);
		}
		else
		{
			_MESSAGE("Error loading effect list. type!=SENB");
			return;
		}

		Interface->GetNextRecordInfo(&type, &version, &length);

		if(type=='SREF')
		{
			Interface->ReadRecordData(&LoadRefID,length);
			_MESSAGE("RefID = %X",LoadRefID);
			if (LoadRefID==0)
			{
				_MESSAGE("NULL refID. Will load effect as I can't resolve it's state.");
				InUse=true;
			}
			else
			{
				InUse=Interface->ResolveRefID(LoadRefID,&LoadRefID);
				_MESSAGE("Is in use = %i",InUse);
			}
		}
		else
		{
			_MESSAGE("Error loading effect list. type!=SREF");
			return;
		}

		EffectRecord *NewEffect=new(EffectRecord);

		if(InUse && NewEffect->LoadEffect(LoadFilepath))
		{
			NewEffect->Enabled=LoadEnabled;
			NewEffect->ParentRefID=LoadRefID;

			TextureManager	*TexMan=TextureManager::GetSingleton();

			Interface->GetNextRecordInfo(&type, &version, &length);

			while(type!='SEOD')
			{
				switch(type)
				{
				case 'STEX':
					TextureType TextureData;

					Interface->ReadRecordData(&TextureData,length);

					NewEffect->SetEffectTexture(TextureData.Name,TextureData.tex);
					_MESSAGE("Texture %s = %i",TextureData.Name,TextureData.tex);
					break;
				case 'SINT':
					IntType IntData;

					Interface->ReadRecordData(&IntData,length);

					NewEffect->Effect->SetIntArray(IntData.Name,(int *)&IntData.data,IntData.size);
					_MESSAGE("Int %s = %i(%i)",IntData.Name,IntData.data[0],IntData.size);
					break;
				case 'SFLT':
					FloatType FloatData;

					Interface->ReadRecordData(&FloatData,length);

					NewEffect->Effect->SetFloatArray(FloatData.Name, (float *)&FloatData.data, FloatData.size);
					_MESSAGE("Float %s = %f(%i)",FloatData.Name, FloatData.data[0], FloatData.size);
					break;
				}
				Interface->GetNextRecordInfo(&type, &version, &length);
			}

			// Need to set after loading shaders vars to save file doesn't override them.
			_MESSAGE("Setting effects screen texture.");
			NewEffect->Effect->SetTexture("reflection",passTexture[OBGEPASS_REFLECTION]);
			NewEffect->Effect->SetTexture("thisframe",TexMan->thisframeTex);
			NewEffect->Effect->SetTexture("lastpass",TexMan->lastpassTex);
			NewEffect->Effect->SetTexture("lastframe",TexMan->lastframeTex);
			NewEffect->Effect->SetTexture("Depth",TexMan->depth);
			NewEffect->Effect->SetFloatArray("rcpres",(float*)&EffectConst.rcpres,2);
			NewEffect->Effect->SetBool("bHasReflection",!!passTexture[OBGEPASS_REFLECTION]);
			NewEffect->Effect->SetBool("bHasDepth",EffectConst.bHasDepth);

			Effects.insert(std::make_pair(LoadShaderNum,NewEffect));
			_MESSAGE("Inserting the effect into the list.");
		}
		else
		{
			delete(NewEffect);

			Interface->GetNextRecordInfo(&type, &version, &length);

			while(type!='SEOD')
			{
				Interface->ReadRecordData(&LoadFilepath,length);
				Interface->GetNextRecordInfo(&type, &version, &length);
			}
		}
		Interface->GetNextRecordInfo(&type,&version,&length);
	}
}

bool EffectManager::IsEffectValid(int EffectNum)
{
  if(Effects.find(EffectNum)==Effects.end())
    return(false);
  return(true);
}

bool EffectManager::EnableEffect(int EffectNum, bool State)
{
  EffectList::iterator Effect;

  Effect=Effects.find(EffectNum);
  if(Effect!=Effects.end())
  {
    Effect->second->Enabled=State;
    return(true);
  }
  return(false);
}

bool EffectManager::SetEffectInt(int EffectNum, char *name, int value)
{
  EffectList::iterator Effect;

  Effect=Effects.find(EffectNum);
  if(Effect!=Effects.end())
  {
    return(Effect->second->SetEffectInt(name,value));
  }
  return(false);
}

bool EffectManager::SetEffectFloat(int EffectNum, char *name, float value)
{
  EffectList::iterator Effect;

  Effect=Effects.find(EffectNum);
  if(Effect!=Effects.end())
  {
    return(Effect->second->SetEffectFloat(name,value));
  }
  return(false);
}

bool EffectManager::SetEffectVector(int EffectNum, char *name, v1_2_416::NiVector4 *value)
{
  EffectList::iterator Effect;

  Effect=Effects.find(EffectNum);
  if(Effect!=Effects.end())
  {
    return(Effect->second->SetEffectVector(name,value));
  }
  return(false);
}

bool EffectManager::SetEffectTexture(int EffectNum, char *name, int TextureNum)
{
  EffectList::iterator Effect;

  Effect=Effects.find(EffectNum);
  if(Effect!=Effects.end())
  {
    return(Effect->second->SetEffectTexture(name,TextureNum));
  }
  return(false);
}

void EffectManager::PurgeTexture(IDirect3DBaseTexture9 *texture)
{
  EffectList::iterator Effect=Effects.begin();

  while(Effect!=Effects.end())
  {
    D3DXEFFECT_DESC Description;
    Effect->second->Effect->GetDesc(&Description);
    for (int par=0;par<Description.Parameters;par++)
    {
      D3DXHANDLE	handle;
      handle=Effect->second->Effect->GetParameter(NULL,par);
      if(handle)
      {
	D3DXPARAMETER_DESC Description;
	Effect->second->Effect->GetParameterDesc(handle,&Description);
	if((Description.Type=D3DXPT_TEXTURE) ||
	   (Description.Type=D3DXPT_TEXTURE1D) ||
	   (Description.Type=D3DXPT_TEXTURE2D) ||
	   (Description.Type=D3DXPT_TEXTURE3D) ||
	   (Description.Type=D3DXPT_TEXTURECUBE))
	{
	  IDirect3DBaseTexture9 *ShaderTexture=NULL;				// NB must set to NULL otherwise strange things happen
	  Effect->second->Effect->GetTexture(handle,&ShaderTexture);
	  if(ShaderTexture==texture)
	  {
	    Effect->second->Effect->SetTexture(handle,NULL);
	    _MESSAGE("Removing texture %s from effect %i",Description.Name,Effect->first);
	  }

	}
      }
    }
    Effect++;
  }
}

bool EffectManager::GetEffectState(int EffectNum)
{
  EffectList::iterator Effect;

  Effect=Effects.find(EffectNum);
  if(Effect!=Effects.end())
    return(Effect->second->IsEnabled());
  return(false);
}