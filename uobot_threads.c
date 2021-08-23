#include <windows.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "uobot_log.h"
#include "uobot_net.h"
#include "uobot_handles.h"
#include "uobot_obj.h"
#include <time.h>
#include "uobot_jornal.h"
#include "uobot_target.h"
#include "uobot_spells.h"
#include "uobot_exthand.h"
#include "uobot_threads.h"
#include "uobot_windows.h"
#include "uobot_plugins.h"
#include "uobot.h"


CmdThreadsTag *CmdThreads=0;
unsigned int CmdThreadsCount=0;

int WINAPI RunThread(CmdThreadsTag *HInfo);
HANDLE hThreadWork=0;

DWORD WINAPI Thread_Routine(LPVOID lpParameter)
{
	struct ThreadInfo *TInfo=(struct ThreadInfo *)lpParameter;
	Sleep(TInfo->SleepTime);
	//TInfo->LogPrint(3, "[THREAD] Sending packet %#X\n", TInfo->buf[0]&0xFF);
	TInfo->send_server(TInfo->buf, TInfo->len);
	free(TInfo->buf);
	free(TInfo);
	return 0;
}

int Thread_SendServer(char *buf, int len, int SleepTime)
{
	char *ThreadBuf;
	struct ThreadInfo *TInfo;
	if((ThreadBuf=malloc(len))==NULL)
	{
		LogPrint(3, "[ERROR] Nao foi possivel alocar espaco para ThreadBuf\n");
		exit(-1);
	}
	if((TInfo=(struct ThreadInfo *)malloc(sizeof(struct ThreadInfo)))==NULL)
	{
		LogPrint(3, "[ERROR] Nao foi possivel alocar espaco para ThreadInfo\n");
		exit(-1);
	}
	memcpy(ThreadBuf, buf, len);
	TInfo->buf = ThreadBuf;
	TInfo->len = len;
	TInfo->SleepTime = SleepTime;
	TInfo->send_server = send_server;
	TInfo->LogPrint = LogPrint;

	if((hThreadWork = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)Thread_Routine, (LPVOID)TInfo, 0, (LPDWORD)&hThreadWork))==0){
		LogPrint(3, "[ERROR] Nao foi possivel criar a threadwork\n");
		exit(-1);
	}
	return 1;
}

CmdInfoTag *GetThreadCI(HANDLE hHandle)
{
	unsigned int x=0;
	for(x=0; x < CmdThreadsCount; x++)
	{
		if(CmdThreads[x].hHandle == hHandle)
			return CmdThreads[x].Info;
	}
	return 0;
}

int GetThreadIdx(int Idx)
{
	unsigned int x=0;
	for(x=0; x < CmdThreadsCount; x++)
	{
		if(CmdThreads[x].CmdIdx == Idx)
			return x;
	}
	return -1;
}

int MakeCmdThread(int Idx, char **Args, int Argc)
{
	int x=0;
	CmdInfoTag TmpTag={0};	
	if(GetThreadIdx(Idx)!=-1)
	{
		LogPrint(1, "[ERROR] Thread [%s] ja esta rodando\n", Commands[Idx].Command);
		return -1;
	}
	if(Commands[Idx].Place == 0)
	{
		TmpTag.Argc = Argc;
		TmpTag.Args = Args;
		Commands[Idx].Handler(&TmpTag);
		for(x=0; x < Argc; x++)
			free(Args[x]);
		free(Args);
		return 1;
	}

	CmdThreadsCount++;
	CmdThreads = (CmdThreadsTag *)realloc(CmdThreads, CmdThreadsCount*sizeof(CmdThreadsTag));
	CmdThreads[CmdThreadsCount - 1].CmdIdx = Idx;
	CmdThreads[CmdThreadsCount - 1].Info = (CmdInfoTag *)malloc(sizeof(CmdInfoTag));
	CmdThreads[CmdThreadsCount - 1].Info->Args = Args;
	CmdThreads[CmdThreadsCount - 1].Info->Argc = Argc;
	CmdThreads[CmdThreadsCount - 1].IsSuspended = (IsConnected ? 0 : 1);

	if((CmdThreads[CmdThreadsCount - 1].hHandle = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)Commands[Idx].Handler, (LPVOID)CmdThreads[CmdThreadsCount - 1].Info, (CmdThreads[CmdThreadsCount - 1].IsSuspended ? CREATE_SUSPENDED : 0), (LPDWORD)&CmdThreads[CmdThreadsCount - 1].hHandle))==0)
	/*if((CmdThreads[CmdThreadsCount - 1].hHandle = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)RunThread, (LPVOID)&CmdThreads[CmdThreadsCount - 1], (CmdThreads[CmdThreadsCount - 1].IsSuspended ? CREATE_SUSPENDED : 0), (LPDWORD)&CmdThreads[CmdThreadsCount - 1].hHandle))==0)*/
	{
		LogPrint(3, "[ERROR] Nao foi possivel iniciar comando [%s] [%#X] com [%d] args\n", 
			Commands[Idx].Command, Commands[Idx].Handler, Argc);
		for(x=0; x < Argc; x++)
		{
			free(CmdThreads[CmdThreadsCount - 1].Info->Args[x]);
		}
		free(CmdThreads[CmdThreadsCount - 1].Info->Args);
		CmdThreadsCount--;
		CmdThreads = (CmdThreadsTag *)realloc(CmdThreads, CmdThreadsCount*sizeof(CmdThreadsTag));
	}
	LogPrint(3, "[THREAD] Thread [%d] [%s] criada com sucesso, suspend [%d]\n",
		CmdThreads[CmdThreadsCount - 1].hHandle, Commands[Idx].Command, !IsConnected);
	
	return 1;
}

int GetIdxFromInfo(CmdInfoTag *CI)
{
	unsigned int x;
	for(x=0; x < CmdThreadsCount; x++)
	{
		if(CmdThreads[x].Info == CI)
			return x;
	}
	return -1;
}

unsigned int RemakeCmds(void)
{
	unsigned int x=0;
	SuspendAllPThreads();
	for(x=0; x < CmdThreadsCount; x++)
	{
		TerminateThread(CmdThreads[x].hHandle, 0);		
		LogPrint(1, "[REMAKE] Kilado thread [%d]\n", CmdThreads[x].hHandle);
		CmdThreads[x].hHandle = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)Commands[CmdThreads[x].CmdIdx].Handler, (LPVOID)CmdThreads[x].Info, CREATE_SUSPENDED, (LPDWORD)&CmdThreads[x].hHandle);
		LogPrint(1, "[REMAKE] Thread [%d] [%s] executada\n", CmdThreads[x].hHandle, Commands[CmdThreads[x].CmdIdx].Command);
	}
	return x;
}


unsigned int SuspendAllPThreads(void)
{
	unsigned int x,count=0;
	for(x=0; x < CmdThreadsCount; x++)
	{
		if(!CmdThreads[x].IsSuspended)
		{
			while(SuspendThread(CmdThreads[x].hHandle)==-1) Sleep(50);
			/*LogPrint(1, "[SUSPEND] Thread [%d] [%s] parada\n",
				CmdThreads[x].CmdIdx, Commands[CmdThreads[x].CmdIdx].Command);*/
			CmdThreads[x].IsSuspended = 1;
			count++;
		}
	}
//	LogPrint(3, "[SUSPEND] %d threads paralizadas\n", count);
	return count;
}


void ResumeAllPThreads(void)
{
	unsigned int x,count=0;
	for(x=0; x < CmdThreadsCount; x++)
	{
		if(CmdThreads[x].IsSuspended)
		{
			ResumeThread(CmdThreads[x].hHandle);
			/*LogPrint(3, "[SUSPEND] Thread [%d] [%s] liberada\n",
				CmdThreads[x].hHandle, Commands[CmdThreads[x].CmdIdx].Command);*/
			count++;
			CmdThreads[x].IsSuspended = 0;
		}
	}
	LogPrint(3, "[RESUMED] %d threads liberadas\n", count);
}

void CmdListThreads(void)
{
	unsigned int x=0;
	LogPrint(3, "[LTHREADS] Listando %d threads\n", CmdThreadsCount);
	for(x=0; x < CmdThreadsCount; x++)
	{
		LogPrint(3, "[LTHREADS] Thread [%d] rodando cmd [%s], Suspend [%d]\n",
			CmdThreads[x].hHandle, Commands[CmdThreads[x].CmdIdx].Command, CmdThreads[x].IsSuspended);
	}
}

int CmdThreadKill(CmdInfoTag *CI)
{
	unsigned int x,newcount=0;
	CmdThreadsTag *NewCmdThreads=0;
	int CmdIdx=GetIdxFromInfo(CI),i=0,IsMainThread=0;
	HANDLE hHandle;
	if(CmdIdx==-1)
	{
		LogPrint(3, "[ERROR] Nao eh possivel terminar essa thread\n");
		return 0;
	}
	if(Commands[CmdIdx].Place == 0) IsMainThread = 1;
	else hHandle = CmdThreads[CmdIdx].hHandle;
	if(CmdThreadsCount != 1)
		NewCmdThreads = malloc((CmdThreadsCount-1)*sizeof(CmdThreadsTag));
	for(x = 0; x < CmdThreadsCount; x++)
	{
		if(CmdThreads[x].Info == CI)
		{
			LogPrint(3, "[EXITTHREAD] Thread [%d] [%s] finalizada\n", CmdThreads[x].hHandle, Commands[CmdThreads[x].CmdIdx].Command);
			for(i=0; i < CmdThreads[x].Info->Argc; i++)
			{
				free(CmdThreads[x].Info->Args[i]);
			}
			free(CmdThreads[x].Info->Args);
			if(TerminateThread(CmdThreads[x].hHandle, 0)==0)
				LogPrint(3, "[ERRO] Nao foi possivel kilar thread %d\n", CmdThreads[x].hHandle);
			continue;
		}
		NewCmdThreads[newcount].CmdIdx = CmdThreads[x].CmdIdx;
		NewCmdThreads[newcount].hHandle = CmdThreads[x].hHandle;
		NewCmdThreads[newcount].Info = CmdThreads[x].Info;
		NewCmdThreads[newcount].IsSuspended = CmdThreads[x].IsSuspended;
		newcount++;
	}
	free(CmdThreads);
	CmdThreads = NewCmdThreads;
	CmdThreadsCount = newcount;
	return 1;
}


unsigned int CmdKillAll(void)
{
	unsigned int count=0;
	while(CmdThreadsCount)
	{
		if(CmdThreads[0].IsSuspended)
			CmdThreadKill(CmdThreads[0].Info);
		else 
		{
			CmdThreads[0].IsSuspended = 1;
			while(SuspendThread(CmdThreads[0].hHandle)==-1) Sleep(50);
			CmdThreadKill(CmdThreads[0].Info);
		}
		count++;
	}
	return count;
}

int CmdThreadExit(CmdInfoTag *CI)
{
	unsigned int x,newcount=0;
	CmdThreadsTag *NewCmdThreads;
	int CmdIdx=GetIdxFromInfo(CI),i=0,IsMainThread=0;
	HANDLE hHandle;
	if(CmdIdx==-1)
	{
		LogPrint(3, "[ERROR] Nao eh possivel terminar essa thread\n");
		return 0;
	}
	if(Commands[CmdIdx].Place == 0) IsMainThread = 1;
	else hHandle = CmdThreads[CmdIdx].hHandle;
	NewCmdThreads = malloc((CmdThreadsCount-1)*sizeof(CmdThreadsTag));
	for(x = 0; x < CmdThreadsCount; x++)
	{
		if(CmdThreads[x].Info == CI)
		{
			LogPrint(3, "[EXITTHREAD] Thread [%d] [%s] finalizada\n", CmdThreads[x].hHandle, Commands[CmdThreads[x].CmdIdx].Command);
			for(i=0; i < CmdThreads[x].Info->Argc; i++)
			{
				free(CmdThreads[x].Info->Args[i]);
			}
			free(CmdThreads[x].Info->Args);
			continue;
		}
		NewCmdThreads[newcount].CmdIdx = CmdThreads[x].CmdIdx;
		NewCmdThreads[newcount].hHandle = CmdThreads[x].hHandle;
		NewCmdThreads[newcount].Info = CmdThreads[x].Info;
		NewCmdThreads[newcount].IsSuspended = CmdThreads[x].IsSuspended;
		newcount++;
	}
	free(CmdThreads);
	CmdThreads = NewCmdThreads;
	CmdThreadsCount = newcount;
	if(!IsMainThread) ExitThread(-1);
	return 1;
}

/* Para nao necessitar mais utilizar o cmdthreadexit
 * toda vez q tiver q sair de uma thread
 * (nao funfa)
 */
int WINAPI RunThread(CmdThreadsTag *HInfo)
{
	Commands[HInfo->CmdIdx].Handler(HInfo->Info);
	CmdThreadExit(HInfo->Info);
	return 0;
}