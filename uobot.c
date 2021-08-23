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
#include "uobot_plugins.h"
#include "uobot_teleto.h"
#include "uobot_menu.h"
#include "uobot.h"
#include <commctrl.h>
#include "resource.h"
#include "uobot_gumps.h"
#include "uobot_hexcp.h"
#include "uobot_windows.h"
#include "uobot_commands.h"
#include "uobot_vend.h"

char SHARDADDR[100] = "server.mystic.com.br";
short SHARDPORT = 5003;
char LOGUIN[30];
char SENHA[30];
char *VERSION="2.0.7";
char CHARNAME[30];

BOOL PrintDebug=FALSE;

static unsigned char RecvBuf[15000]={0};

struct ExpFunc DllExpFunc = {
	send_server,
	LogDump,
	LogPrint,
	MBOut,
	CountItemInContainer,
	FindItemInContainer, 
	FindSerial,
	GetItemInLayer,
	pack16,
	pack32,
	UnicodeToAscii,
	unpack16,
	unpack32,
	GetJournalLine,
	JournalGetLast,
	SetJournalLine,
	IsInJournal,
	CleanJournal,
	Say,
	Target_WaitTarget,
	Target_WaitTargetGraphic,
	&TargetSent,
	Effect_WaitGraphEffect,
	&GraphicEffectSent,
	UseObject,
	ClickObject,
	GetDistance,
	MoveToContainer,
	PickupItem,
	DropItem,
	EquipItem,
	UnequipItem,
	EquipItemType,
	UseObjectType,
	Life,
	IsItemInLayer,
	ArgToInt,
	CastSpell,
	UseSkill,
	Attack,
	DropHere,
	IsDead,
	CloseShardSocket,
	AddPacketHandler,
	&AntiMacro,
	&mbackpack,
	&mserial,
	CmdThreadExit,
	PluginAddCommand,
	ReqSkillValue,
	WarMode,
	&WDlg,
	Send_Reply,
	&WMenu,
	Send_Choice,
	HookConsoleTo,
	Wait_Gump,
	GumpSearchText,
	GumpSearchXY,
	FreeGump,
	SendGumpChoice,
	&GumpSent,
	GetQuantity,
	SetCatchBag,
	UnSetCatchBag,
	&gobjcount,
	Walk,
	&MenuOptions,
	Target_WaitTargetTile,
	Teleto_Rew,
	Teleto_Go,
	GetRefName,
	Target_SendTarget,
	Target_RecvTarget,
	&SellListSent,
	SellItems,
	HandleCmd,
	MassMove,
	MassMoveGraphic
};


HMODULE hLib=0;
PlInit PluginInit=0;

void sendloguinseed(void)
{
	unsigned char buf[5];
	buf[0] = 0xFF;
	buf[1] = 0xFF;
	buf[2] = 0xFF;
	buf[3] = 0xFF;
	send_server(buf, 4);
}

void sendloguinsenha(void)
{
	// Enviar loguin/senha
	unsigned char buf[70];
	memset(buf, 0x00, 65);
	buf[0] = 0x91;
	buf[1] = 0xFF;
	buf[2] = 0xFF;
	buf[3] = 0xFF;
	buf[4] = 0xFF;
	strcpy(buf+5, LOGUIN);
	strcpy(buf+35, SENHA);
	send_server(buf, 65);
}

void CloseThreads(void)
{
	if(hThread){
		TerminateThread(hThread, 0);
		hThread = 0;
		LogPrint(3, "[THREAD] Ping thread closed\n");
	}
}

void Credits(void)
{
	LogPrint(1, "#########################################################\n");
	LogPrint(1, "######### UOBot - Desenvolvido para uso somente na mystic #############\n");
	LogPrint(1, "####   Creditos para Yuck,SB_Seven(SoX rlz) e Necropotence! Brazucas rulam! ###\n");
	LogPrint(1, "#########################################################\n");
}

void SairDeTudo(void)
{
	closesocket(sockshard);
	CloseThreads();
}


int Start(char *user, char *pass, char *cname)
{
	unsigned char *pnow=RecvBuf;
	int PacketLen, buflen=9000, troca=0, CountRes=0; // Inicializar o winsock
	int iResult=0;
	int IsBufQueue = 0;
	Credits();


	strcpy(LOGUIN, user);
	strcpy(SENHA, pass);
	strcpy(CHARNAME, cname);
	sprintf(RecvBuf, "log%s.txt", LOGUIN);
	LogOpen(RecvBuf);

	// Now all exceptions would be handled in my filter
	SetUnhandledExceptionFilter(MyUnhandledExceptionFilter);

gconect: //Conectando ao shard
	RemoveAllObjs();
	CleanTarget();
	InitPacketHandler();
	gobjcount = 0;
	CountRes = 0;
	ErrNum = 0;

gtrycon1:
	DecompressClean(&HuffObj);

	LogPrint(1, "[NET] [%d] Tentando conectar [%s:%u] [%s]\n", ++troca, SHARDADDR, SHARDPORT, CHARNAME);
	LastTickMsg = 0;
	if((sockshard=net_connect(SHARDADDR, SHARDPORT))==SOCKET_ERROR)
	{
		LogPrint(1, "[ERROR] Nao foi possivel conectar\n");
		Sleep(1000);
		goto gtrycon1;
	}
	LogPrint(3, "[NET] Enviando seed, l:[%s]\n", LOGUIN);
	firstpacket = 1;

	sendloguinseed();
	sendloguinsenha();

	LoguinSeed = 0;
	ErrNum = 0;

	pnow = RecvBuf;
	CountRes = 0;

	// Recv
	while(1)
	{
		LastTickMsg = 0;
		iResult=net_recv(sockshard, pnow, 0, buflen);
		if(iResult<=0) // Socket closed
		{
			LoginTime = 0;
			LastAm = 0;
			AntiMacro = 0;
			LastTickMsg = 0;
			SetWindowText(MainWnd, "UoBot");
			RemoveAllObjs();
			SairDeTudo();
			LogPrint(3, "[TIMEOUT] Conexao caiu, esperar 15 segundos e tentar conectar novamente\n");
			if(CmdThreadsCount)
			{
				Plugin_Done(ErrNum);
				CountRes = RemakeCmds();
//				LogPrint(1, "[SUSPEND] [%d] threads paradas\n", CountRes);
			}
			IsConnected = 0;
			Sleep(15000);
			pnow = RecvBuf;
			CountRes = 0;
			goto gconect;
		}
		pnow = RecvBuf;
		LogPrint((PrintDebug ? 3: 0), "[LOGBUF] Recebido[%d] CountRes[%d] Total[%d]\n", iResult, CountRes, iResult+CountRes);
		if(CountRes)
		{
			LogPrint((PrintDebug ? 2:0), "[LOGBUF] Bytes queued [%d] Recv [%d]\n", CountRes, iResult);
			IsBufQueue = 1;
		}
		else
			IsBufQueue = 0;
		iResult += CountRes;
		CountRes = 0;
		if(PacketLengths[pnow[0]&0xff] >= 0x8000)
		{
			if(iResult<3)
			{
				pnow += iResult;
				CountRes = iResult;
				continue;
			}

		}
		PacketLen = GetPacketLen(pnow, iResult);
		if(iResult!=PacketLen) // Tamanho recebido > tamanho oficial
		{
			while(iResult>0)
			{
				if(PacketLengths[pnow[0]&0xff] >= 0x8000)
					if(iResult<3)
						break;
				PacketLen=GetPacketLen(pnow, iResult);
				LogPrint((PrintDebug ? 3: 0), "[LOGBUF2] iResult[%d] CountRes[%d]\n", iResult, CountRes);
				if(PacketLen>iResult)
				{
					LogPrint((PrintDebug ? 3 : 0), "[LOGBUF] PacketLen[%d]>iResult[%d]\n", PacketLen, iResult);
					break;
				}
				
				if(IsBufQueue)
				{
					LogPrint((PrintDebug ? 2:0), "[LOGBUF] %s\n", PacketNames[*pnow]);
					if(PrintDebug) LogDump(pnow, PacketLen);
				}
				uo_processpkt(pnow, PacketLen);
				pnow += PacketLen;
				iResult -= PacketLen;
			}
			if(iResult>0)
			{
				LogPrint(2, "[LOGBUF] Recv() ultrapassou o limite do buf em [%d] bytes\n", PacketLen-iResult);
				LogPrint(2, "[LOGBUF] %s\n", PacketNames[*pnow]);
				LogDump(pnow, iResult);
				// Copiar os bytes descompactados q sobraram novamente no buffer
				memcpy(RecvBuf, pnow, iResult);
				// Apontar pnow pro RecvBuf+bytes q sobraram pra ler ok
				pnow=RecvBuf+iResult;
				// Adicionar ao tamanho do novo RecvBuffer os bytes q sobraram nesse
				CountRes = iResult;
			} else {
				pnow=RecvBuf;
				CountRes = 0;
			}
		} else {
			if(IsBufQueue)
			{
				LogPrint((PrintDebug ? 2:0), "[LOGBUF] %s\n", PacketNames[*pnow]);
				if(PrintDebug) LogDump(pnow, PacketLen);
			}
			uo_processpkt(pnow, PacketLen);
            pnow = RecvBuf;
			CountRes = 0;
		}
	}
	LogPrint(3, "[ERROR] Impossivel chegar aqui, mas, chegou\n");
	LogClose();
	return 0;
}

 

