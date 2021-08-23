#include <windows.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "uobot_log.h"
#include "uobot_obj.h"
#include "uobot_threads.h"
#include "uobot_windows.h"
#include "uobot_plugins.h"
#include "uobot.h"

HMODULE PluginModuleNow = 0;

PluginType PluginsList[50]={0};
unsigned int PluginsCount=0;

int PluginGetFreeIdx(void)
{
	unsigned int x;
	for(x=0;x < PLUGINSIZE; x++)
		if(!PluginsList[x].Used) return x;
	return -1;
}

int CommandGetQtdFromModule(HMODULE hModule)
{
	unsigned int x,count=0;
	for(x=0;x < CmdListSize; x++)
		if(Commands[x].Place == hModule)
			count++;
	return count;
}


int PluginRemoveCommand(HMODULE hModule)
{
	Cmd *NewCmd;
	unsigned int x=0,i=0,qtos=0;
	if(!CmdListSize)
	{
		LogPrint(3, "[REMOVECMD] CmdListSize = %d\n", CmdListSize);
		return 0;
	}
	if((qtos=CommandGetQtdFromModule(hModule))==0)
	{
		LogPrint(3, "[REMOVECMD] Nao foi achado nenhum cmd para %#X\n", hModule);
		return 0;
	}
	LogPrint(3, "[REMOVECMD] Achei %d commands, tentando remove-los\n", qtos);

	NewCmd = malloc((CmdListSize-qtos)*sizeof(Cmd));
	for(x=0; x < CmdListSize; x++)
	{
		if(Commands[x].Place == hModule) continue;
		strcpy(NewCmd[i].Command, Commands[x].Command);
		NewCmd[i].Handler = Commands[x].Handler;
		NewCmd[i].Place = Commands[x].Place;
		i++;
	}
	free(Commands);
	Commands = NewCmd;
	x = CmdListSize - i;
	CmdListSize = i;
	return x;
}

int PluginAddCommand(const char *Name, void *Handler, HMODULE hModule)
{
	unsigned int i = 0,x=0;
	char *NewName = malloc(strlen(Name)+2);
	sprintf(NewName, "/%s", Name);
	/* can't redefine a command */
	for(i = 0; i < CmdListSize; i++)
	{
		if(!strcmp(Commands[i].Command, NewName))
		{
			LogPrint(1, "[ERROR] Comando %s ja existe\n", NewName);
			return 0;
		}
	}

	/* name must always be present. and either alias or callback */
	if(Name == NULL || Handler == NULL || PluginModuleNow == NULL)
	{
		LogPrint(3, "[ERRO] Name/Handler/hModule = 0\n");
		return 0;

	}

	CmdListSize++;
	Commands = (Cmd*)realloc(Commands, CmdListSize*sizeof(Cmd));
	Commands[CmdListSize - 1].Place = PluginModuleNow;
	sprintf(Commands[CmdListSize - 1].Command, "%s", NewName);
	Commands[CmdListSize - 1].Handler = (CommandHandler)Handler;
	LogPrint(3, "[CMDP] Comando %s adicionado\n", NewName);
	return 1;
}

int PluginGetIdx(char *PluginName)
{
	unsigned int x=0;
	for(x=0; x < PLUGINSIZE; x++)
	{
		if(!PluginsList[x].Used) continue;
		if(!strcmp(PluginsList[x].PluginName, PluginName))
			return x;
	}
	return -1;
}

int PluginRemove(char *PluginName, int Idx)
{
	unsigned int PluginIdx,x=0;
	if(PluginName[0] == 0)
	{
		if(!PluginsList[Idx].Used)
		{
			LogPrint(1, "[ERROR] Idx nao esta sendo usado\n");
			return 0;
		}
		PluginRemoveCommand(PluginsList[Idx].hModule);			
		FreeLibrary(PluginsList[Idx].hModule);
		PluginsList[Idx].Used = 0;
		PluginsCount--;
		return 1;
	}
	if((PluginIdx = PluginGetIdx(PluginName))==-1)
	{
		LogPrint(1, "[ERROR] Nao foi possivel achar plugin %s\n", PluginName);
		return 0;
	}
	PluginRemoveCommand(PluginsList[PluginIdx].hModule);
	FreeLibrary(PluginsList[PluginIdx].hModule);
	PluginsList[PluginIdx].Used = 0;
	PluginsCount--;
	return 1;
}

int PluginIsRun(char *PluginName)
{
	unsigned int x;
	for(x=0; x < PLUGINSIZE; x++)
	{
		if(!PluginsList[x].Used) continue;
		if(!strcmp(PluginsList[x].PluginName, PluginName))
			return 1;
	}
	return 0;
}

void ListPluginsRunning(void)
{
	unsigned int x=0,count=0;
	LogPrint(3, "[PLUGINS] Tem %d dlls abertas\n", PluginsCount);
	for(x=0; x < PLUGINSIZE; x++)
	{
		if(!PluginsList[x].Used) continue;
		LogPrint(3, "[PLUGINS] Dll %s = %#X\n",
			PluginsList[x].PluginName, PluginsList[x].hModule);
	}
}

void ListCommands(void)
{
	unsigned int x=0;
	LogPrint(3, "[CMDS] Listando %d cmds\n", CmdListSize);
	for(x=0; x < CmdListSize; x++)
	{
		LogPrint(3, "[CMDS] Comando %s = %#X = %#X\n",
			Commands[x].Command, Commands[x].Handler, Commands[x].Place);
	}
}

int LoadPluginDll(char *PluginName)
{
	unsigned int FreeIdx=0;
	//typedef DWORD (*PllInit)(struct ExpFunc *exp);
	PlInit PluginInit=0;
	if(PluginIsRun(PluginName))
	{
		LogPrint(1, "[ERROR] Esta dll ja esta anexa\n");
		return 0;
	}
	if((FreeIdx=PluginGetFreeIdx())==-1)
	{
		LogPrint(1, "[ERROR] Voce ja tem dlls demais sendo usadas\n");
		return 0;
	}
	if((PluginsList[FreeIdx].hModule = LoadLibrary(PluginName))==NULL)
	{
		LogPrint(1, "[ERROR] Impossivel abrir %s\n", PluginName);
		return 0;
	}
	if((PluginInit = (PlInit)GetProcAddress(PluginsList[FreeIdx].hModule, "PluginInit"))==NULL)
	{
		LogPrint(1, "[ERROR] Nao foi possivel achar PluginInit\n");
		FreeLibrary(PluginsList[FreeIdx].hModule);
		return 0;
	}

	PluginsList[FreeIdx].DisconnectFunc = (DisconnectFuncType)GetProcAddress(PluginsList[FreeIdx].hModule, "PluginEnd");

	PluginModuleNow = PluginsList[FreeIdx].hModule;

	if(!PluginInit(&DllExpFunc))
	{
		LogPrint(1, "[ERROR] InitPlugin\n");
		return 0;
	}

	PluginsList[FreeIdx].Used = 1;
	strncpy(PluginsList[FreeIdx].PluginName, PluginName, sizeof(PluginsList[FreeIdx].PluginName));
	PluginsCount++;

	return 1;
}

void Plugin_Done(int Num)
{
	unsigned int x=0;

	for(x=0; x < PLUGINSIZE; x++)
	{
		if(!PluginsList[x].Used)
			continue;

		if(PluginsList[x].DisconnectFunc)
			PluginsList[x].DisconnectFunc(Num);
	}
}
			