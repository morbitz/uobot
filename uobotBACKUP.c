#include <windows.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "uobot_log.h"
#include "uobot_net.h"
#include "uobot_handles.h"
#include "uobot_obj.h"
#include <time.h>
#include "UOEncryption.h"
#include "uobot_jornal.h"
#include "uobot_target.h"
#include "uobot_spells.h"
#include "uobot.h"

char *SHARDADDR = "server.mystic.com.br";
short SHARDPORT = 5003;
char *LOGUIN;
char *SENHA;
char *VERSION="2.0.7";

BOOL PrintDebug=FALSE;

struct ExpFunc DllExpFunc = {
	send_server,
	LogDump,
	LogPrint,
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
	CastSpell,
	UseSkill,
	Attack,
	&mbackpack,
	&mserial };


HMODULE hLib=0;
PlInit PluginInit=0;

int main(int argc, char **argv){
	char buf[8192],*pnow;
	WSADATA wsaData;
	int PacketLen, buflen=sizeof(buf)-100, troca=0; // Inicializar o winsock
	int iResult = WSAStartup(MAKEWORD(2,2), &wsaData);

	if(iResult != NO_ERROR)
		LogPrint(3, "[ERROR] Error at WSAStartup()\n");
	DecompressClean(&HuffObj);
	LogOpen("log.txt");
	if(argc!=5)
	{
		LogPrint(3, "Usage: uobot.exe loguin senha plugin debug\n");
		return -1;
	}
	if((LOGUIN = malloc(strlen(argv[1])+1))==NULL||(SENHA=malloc(strlen(argv[2])+1))==NULL)
	{
		LogPrint(3, "[ERROR] Nao foi possivel alocar espaco pro loguin/senha\n");
		return -1;
	}
	strcpy(LOGUIN, argv[1]);
	strcpy(SENHA, argv[2]);
	if(atoi(argv[4])==1) PrintDebug = TRUE;
	if(!strcmp(argv[3], "nada")) goto gconect;
	if((hLib=LoadLibrary((LPCTSTR)argv[3]))==NULL)
	{
		LogPrint(3, "[ERROR] Nao foi possivel dar load no plugin (%s)\n", argv[1]);
		return -1;
	}
	if((PluginInit=(PlInit)GetProcAddress(hLib, "PluginInit"))==NULL)
	{
		LogPrint(3, "[ERROR] Nao foi possivel achar PluginInit()\n");
		return -1;
	}
	gobjects.Next = 0;
gconect: //Conectando ao shard
	RemoveAllObjs();
	CleanTarget();
	gobjcount = 0;
	LogPrint(3, "Tentando conectar [%s:%u]\n", SHARDADDR, SHARDPORT);
	// Funcao DecompressClean da lib UOEncryption
	
	if((sockshard=net_connect(SHARDADDR, SHARDPORT))==SOCKET_ERROR)
	{
		LogPrint(3, "[ERROR] Nao foi possivel conectar\n");
		return -1;
	}
	LogPrint(3, "[C->S] Enviando seed, l:[%s] s:[%s]\n", LOGUIN, SENHA);
	firstpacket = 1;
	LoguinSeed = 1;
	uo_sendloguin(LOGUIN, SENHA);
	if(hThread){
		TerminateThread(hThread, 0);
		hThread = 0;
	}
	if(hPluginThread){
		TerminateThread(hPluginThread, 0);
		hPluginThread = 0;
	}
	// Recv
	while(1)
	{
		if(LoguinSeed)
			iResult=net_recv(sockshard, buf, 0, buflen);
		else 
			iResult=net_recv(sockshard, buf, 0, buflen);

		if(iResult==-1) // Socket closed
		{
			closesocket(sockshard);
			LogPrint(3, "[TIMEOUT] Conexao caiu, ESPERAR 15 SEGUNDOS E tentar conectar novamente..\n");
			Sleep(15000);
			goto gconect;
		}
		if(iResult==0) // Timeout, nao pode
		{
			LogPrint(3, "[TIMEOUT] Timeout 30 segundos, tentando reconectar\n");
			goto gconect;
		}
		PacketLen = GetPacketLen(buf, iResult);
		if(iResult>PacketLen) // Tamanho recebido > tamanho oficial
		{
			pnow = buf;
			while((pnow-buf)<iResult)
			{
				PacketLen=GetPacketLen(pnow, (iResult-(pnow-buf)));
				uo_processpkt(pnow, PacketLen);
				pnow += PacketLen;
				if((pnow-buf)>iResult)
				{
					LogPrint(3, "[ERROR] Ultrapassou o limite do buf em [%u] bytes\n", (pnow-buf)-iResult);
				}
			}
		} else {
			uo_processpkt(buf, iResult);
		}
	}
	return 0;
}

  
