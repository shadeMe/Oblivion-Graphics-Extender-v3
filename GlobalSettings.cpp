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

	sprintf(valuestring,"%g",value);

	WritePrivateProfileStringA(section,setting,valuestring,filename);
}

void GetPrivateProfile(char *section, char *setting, float &value, char *filename)
{
	char *TempBuffer=new(char[100]);
	GetPrivateProfileStringA(section,setting,NULL,TempBuffer,100,filename);
	if(TempBuffer[0]!=0){
	  if (sscanf(TempBuffer, "%g", &value)) {
	    ;
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


