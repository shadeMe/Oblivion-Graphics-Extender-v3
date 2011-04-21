#include "Commands_Shaders.h"
#include "Commands_Misc.h"
#include "Commands_Params.h"
#include "EffectManager.h"
#include "Script.h"
#include "OBSEShaderInterface.h"

static bool LoadEffect_Execute(COMMAND_ARGS)
{
	*result=0;
	
	char path[256];
	int AllowDuplicates=0;

	if(!ExtractArgs(EXTRACTARGS, &path, &AllowDuplicates)) return true;

	if(IsEnabled())
	{
		_MESSAGE("Shader (%s) - Script refID = %x %s",path,scriptObj->refID,(scriptObj->refID==0)?"(Error NULL refID)":" ");
		*result = EffectManager::GetSingleton()->AddEffect(path,AllowDuplicates!=0,scriptObj->refID);
	}

	return true;
}

static bool ApplyFullscreenShader_Execute(COMMAND_ARGS)
{
	*result=0;

	DWORD id, HUD;
	if(!ExtractArgs(EXTRACTARGS, &id, &HUD)) return true;

	if(!IsEnabled() || !EffectManager::GetSingleton()->EnableEffect(id,true))
		*result=-1;

	return true;
}
static bool RemoveFullscreenShader_Execute(COMMAND_ARGS)
{
	*result = 0;
	
	DWORD id;
	int Delete=0;

	if(!ExtractArgs(EXTRACTARGS, &id, &Delete)) return true;
	
	if(IsEnabled())
	{
		if(!Delete)
		{
			if(!EffectManager::GetSingleton()->EnableEffect(id,false))
			{
				*result=-1;
			}
		}
		else
			if(!EffectManager::GetSingleton()->RemoveEffect(id))
			{
				*result=-1;
			}
	}
	else
		*result=-1;
	return true;
}

static bool SetEffectInt_Execute(COMMAND_ARGS)
{
	*result = 0;
	
	DWORD id;
	char var[256];
	int i;
	if(!ExtractArgs(EXTRACTARGS, &id, &var, &i)) return true;

	if(IsEnabled())
		EffectManager::GetSingleton()->SetEffectInt(id,var,i);

	return true;
}
static bool SetEffectFloat_Execute(COMMAND_ARGS)
{
	*result = 0;
	
	DWORD id;
	char var[256];
	float f;
	if(!ExtractArgs(EXTRACTARGS, &id, &var, &f)) return true;

	if(IsEnabled())
		EffectManager::GetSingleton()->SetEffectFloat(id,var,f);
	

	return true;
}
static bool SetEffectVector_Execute(COMMAND_ARGS)
{
	*result = 0;
	
	DWORD id;
	char var[256];
	v1_2_416::NiVector4 v;
	if(!ExtractArgs(EXTRACTARGS, &id, &var, &v[0], &v[1], &v[2], &v[3])) return true;

	if(IsEnabled())
		EffectManager::GetSingleton()->SetEffectVector(id,var,&v);
	
	return true;
}
static bool SetEffectTexture_Execute(COMMAND_ARGS)
{
	*result = 0;

	DWORD id;
	char var[256];
	DWORD i;
	if(!ExtractArgs(EXTRACTARGS, &id, &var, &i)) return true;

	if(IsEnabled())
		EffectManager::GetSingleton()->SetEffectTexture(id,var,i);

	return true;
}

static bool IsEffectEnabled_Execute(COMMAND_ARGS)
{
	*result=0;
	DWORD id;
	
	if(!ExtractArgs(EXTRACTARGS, &id)) return true;
	*result=EffectManager::GetSingleton()->GetEffectState(id);
	return true;
}

CommandInfo kCommandInfo_LoadEffect =
{
	"LoadShader",
	"",
	0,
	"Loads an effect file. (Must be in the .fx format)",
	0,
	2,
	kParams_StringOptInt,
	LoadEffect_Execute,
	0,
	0,
	0
};

CommandInfo kCommandInfo_ApplyFullscreenShader =
{
	"ApplyFullscreenShader",
	"",
	0,
	"Applies a fullscreen shader to oblivion",
	0,
	2,
	kParams_OneIntOneOptInt,
	ApplyFullscreenShader_Execute,
	0,
	0,
	0
};

CommandInfo kCommandInfo_RemoveFullscreenShader =
{
	"RemoveFullscreenShader",
	"",
	0,
	"Removes a fullscreen shader from oblivion",
	0,
	2,
	kParams_OneIntOneOptInt,
	RemoveFullscreenShader_Execute,
	0,
	0,
	0
};

CommandInfo kCommandInfo_SetEffectInt =
{
	"SetShaderInt",
	"",
	0,
	"Sets an integer variable in a fullscreen shader",
	0,
	3,
	kParams_SetFullscreenShaderInt,
	SetEffectInt_Execute,
	0,
	0,
	0
};
CommandInfo kCommandInfo_SetEffectFloat =
{
	"SetShaderFloat",
	"",
	0,
	"Sets a float variable in a fullscreen shader",
	0,
	3,
	kParams_SetFullscreenShaderFloat,
	SetEffectFloat_Execute,
	0,
	0,
	0
};
CommandInfo kCommandInfo_SetEffectVector =
{
	"SetShaderVector",
	"",
	0,
	"Sets an array of 4 floats in a fullscreen shader",
	0,
	6,
	kParams_SetFullscreenShaderVector,
	SetEffectVector_Execute,
	0,
	0,
	0
};
CommandInfo kCommandInfo_SetEffectTexture =
{
	"SetShaderTexture",
	"",
	0,
	"Sets a texture variable in a fullscreen shader",
	0,
	3,
	kParams_SetFullscreenShaderInt,
	SetEffectTexture_Execute,
	0,
	0,
	0
};

CommandInfo kCommandInfo_IsEffectEnabled =
{
	"IsShaderEnabled",
	"",
	0,
	"Returns the state of the loaded shader",
	0,
	1,
	kParams_OneInt,
	IsEffectEnabled_Execute,
	0,
	0,
	0
};