#include "GlobalSettings.h"

// *********************************************************************************

void SetPrivateProfile(char *section, char*setting, int value, char *filename)
{
	char valuestring[34];

	_itoa(value,valuestring,10);

	WritePrivateProfileStringA(section,setting,valuestring,filename);
}

void GetPrivateProfile(char *section, char *setting, int &value, char *filename)
{
	value=GetPrivateProfileIntA(section,setting,value,filename);
}

void SetPrivateProfile(char *section, char*setting, float value, char *filename)
{
	char valuestring[34];

	if ((value > 0) && (value <= 1))
	  sprintf(valuestring,"%g",value);
	else
	  _itoa((int)value,valuestring,10);

	WritePrivateProfileStringA(section,setting,valuestring,filename);
}

void GetPrivateProfile(char *section, char *setting, float &value, char *filename)
{
	char *TempBuffer=new(char[100]);
	GetPrivateProfileStringA(section,setting,NULL,TempBuffer,100,filename);
	if(TempBuffer[0]!=0){
	  if (sscanf(TempBuffer, "%g", &value)) {
	    if ((value > 1))
	      value = (int)value;
	  }
	}
}

void SetPrivateProfile(char *section, char *setting, bool value, char *filename)
{
	WritePrivateProfileStringA(section,setting,value?"1":"0",filename);
}

void GetPrivateProfile(char *section,char *setting, bool &value, char *filename)
{
	int tempvalue;

	if (value)
		tempvalue=1;
	else
		tempvalue=0;

	tempvalue=GetPrivateProfileIntA(section,setting,tempvalue,filename);
	
	if(tempvalue!=0)
		value=true;
	else
		value=false;
}

// *******************************************************************************

INIList *INIList::Singleton=NULL;
char globalbase::SettingsPath[260];
bool globalbase::IsPathSet=false;

INIList *INIList::GetSingleton()
{
	if(!Singleton)
		Singleton=new(INIList);
	return(Singleton);
}

void INIList::RegisterSetting(globalbase *global)
{
	GlobalsList.push_back(global);
}

void INIList::ReadAllFromINI()
{
	GlobalsType::iterator GlobalIt=GlobalsList.begin();

	while(GlobalIt!=GlobalsList.end())
	{
		(*GlobalIt)->GetINI();
		GlobalIt++;
	}
}

void INIList::WriteAllToINI()
{
	GlobalsType::iterator GlobalIt=GlobalsList.begin();

	while(GlobalIt!=GlobalsList.end())
	{
		(*GlobalIt)->SetINI();
		GlobalIt++;
	}
}


