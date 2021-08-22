#include <windows.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include "pluginirc.h"
#include "pluginirc_net.h"
#include "pluginirc_stuff.h"
#include "pluginirc_irc.h"
#include "pluginirc_skills.h"
#include "pluginirc_access.h"
#include "pluginirc_users.h"
#include "pluginirc_privmsg.h"
#include "pluginirc_auth.h"

/*
* MUITO IMPORTANTE
* COLEQUE O NOME EXATO DO ARQUIVO .DLL
*/
#define DLLNAME "pluginirc.dll"
struct ExpFunc *IF=0;

HANDLE hIrcThread=0, hIrcPingThread=0;
BOOL IrcConnected=0;
SOCKET sockIrc=0;

int WS = 0;


char Server[50]={0};
unsigned short ServerPort=7000;
char IrcNick[30]={0};
char IrcSenha[30]={0};
char LastChan[30]={0};
char MainChan[50] = "#testuobot";
char SeeSkillUp = 0;

char SaveCmd[30] = ".save";
unsigned int LastSave = 0;

long LastOnlineReq = 0;

int LogIrc = 0;
int SpeechIrc = 1;

char AmCmd[30] = ".am";
unsigned int LastAm = 0;
int FirstAm = 1;

int CheckedHDSerial = 0;

int IrcMain(LPVOID Inutil);

static char *LastLine = 0;
char Servidor[50] = {0};

int Uo_Handle_Speech(unsigned char *Buf, int Len)
{
	char *Nome = Buf+14, *Texto = Buf+44;
	char SpeechType = Buf[9];
	BOOL IsUnicode = FALSE;
	unsigned int Serial = IF->unpack32(Buf+3);
	

	if(!IrcConnected)
		return 0;

	/* Se o pacote for unicode, modificar os datacode */
	if(Buf[0] == 0xae)
	{
		GameObject *Eu = IF->FindSerial(*IF->mserial);
		IsUnicode = TRUE;
		Nome = Buf+18;
		Texto = malloc(((Len-48)/2)+1);
		IF->UnicodeToAscii(Buf+48, Len-48, Texto);

		if(Eu)
		{
			if(!stricmp(Nome, Eu->Name))
			{
				Irc_Say(MainChan, "Eu: %s", Texto);
				free(Texto);
				return 0;
			}
		}

		if(SpeechIrc && SpeechType == 0)
			Irc_Say(MainChan, "%s: %s", Nome, Texto);

		if( !WS && SpeechType == 0 && (strstr(Nome, "onselhe") || strstr(Nome, "GM") || strstr(Nome, "Admin") || strstr(Nome, "Staff")) )
		{
			ChanListTag *Chan = Chan_Search(MainChan);

			IF->LogPrint(3, "[STAFF] [%s] -> [%s]\n", Nome, Texto);
			if(Chan != 0)
			{
				unsigned int x;
				Irc_Say(MainChan, "%s[STAFF]: %s", Nome, Texto);
				for(x=0; x < Chan->UserListCount; x++)
				{
					if(Chan->UserList[x].NickFlag&2)
						Irc_Say(MainChan, "%s STAFF POR PERTO", Chan->UserList[x].NickName);
				}
			}
		}

	}
	
	/* nao e unicode */
	else
	{
		if(SpeechType == 0 && Serial == 0x01010101 && Buf[11] == 0x37)
			Irc_Say(MainChan, "BCAST: %s", Texto);
	}

	if(LogIrc)
		IF->LogPrint(3, "[SPEECH] [%s] -> [%s]\n", Nome, Texto);

	if(SpeechType == 9 && !WS)
	{
		GameObject *Obj = IF->FindSerial(Serial);
		if(Obj != 0)
		{
			if(strstr(Obj->Name, "Staff") || strstr(Nome, "Staff") || strstr(Texto, "Staff"))
			{
				unsigned int x;
				ChanListTag *Chan = Chan_Search(MainChan);

				Irc_Say(MainChan, "[ISEE] [%s]", Obj->Name);

				if(Chan)
				{
					for(x=0; x < Chan->UserListCount; x++)
					{
						if(Chan->UserList[x].NickFlag&2)
							Irc_Say(MainChan, "%s STAFF POR PERTO", Chan->UserList[x].NickName);
							
					}
				}
			}
		}
	}

	if(WS && (SpeechType == 0 || SpeechType == 1) && !strcmp(Nome, "System"))
	{
		Irc_Say(MainChan, "System: %s", Texto);
	}


	if(!WS && !stricmp(Nome, "System"))
	{
		/* Se tiver salvando mundo, avisar no mainchan */
		if(strstr(Texto, "Salvando"))
		{
			Irc_Say(MainChan, "%s", SaveCmd);
			if(LastSave != 0)
				Irc_Say(MainChan, "Ultimo save foi ha %d minutos", (time(0) - LastSave)/60);
			LastSave = time(0);
		}

		/* Se receber .online enviar no canal */
		else if(strstr(Texto, "Populacao de Britannia:"))
		{
			Irc_Say(MainChan, "%s", Texto);
		}
	}
	if(IsUnicode)
		free(Texto);
	return 0;
}

int Uo_Handle_Gumps(unsigned char *buf, unsigned int len)
{
	GumpType gmr;

	LineType *Lines = 0;
	char *Line=0;

	unsigned int x = 0;
	unsigned short LineSize = 0;

	gmr.cmd = buf[0]&0xff;
	gmr.dtlen = IF->unpack16(buf+1);
	gmr.serial = IF->unpack32(buf+3);
	gmr.tipo = IF->unpack32(buf+7);
	gmr.unk1 = IF->unpack32(buf+11);
	gmr.unk2 = IF->unpack32(buf+15);
	gmr.laylen = IF->unpack16(buf+19);
	gmr.lay = (char *)buf+21;
	gmr.nlines = IF->unpack16(buf+21+gmr.laylen);
	gmr.linelen = IF->unpack16(buf+21+gmr.laylen+2);
	gmr.lines = (char *)buf+21+gmr.laylen+2;

	if(gmr.nlines == 0) return 0;

	Line = gmr.lines;

	/* Alocar espaco para gmr.nlines structs */
	Lines = malloc(sizeof(LineType)*gmr.nlines);

	/* Reinterpretar todas as linhas, transformando em ascii */
	for(x=0; x < (unsigned int)gmr.nlines; x++)
	{
		/* Tamanho da linha unicode */
		LineSize = IF->unpack16(Line);
		/* Alocar espaco para o texto em ascii */
		Lines[x].Text = malloc(LineSize+1);
		/* Transformar o texto em ascii e guarda na struct */
		IF->UnicodeToAscii(Line+2, LineSize*2, Lines[x].Text);
		/* Proxima linha */
		Line += 2+(LineSize*2);
	}

	gmr.lay[gmr.laylen] = 0;

	if( gmr.nlines == 1 && !strcmp(Lines[0].Text, "ANTI-MACRO") )
	{
		/* Se nao for o am de qdo vc conecta no shard, avisar no lastchan */
		if(!FirstAm)
		{
			if(IrcConnected)
			{
				Irc_Say(MainChan, "%s", AmCmd);
				if(LastAm!=0)
					Irc_Say(MainChan, "Ultimo am foi ha: %d minutos", (time(0) - LastAm)/60);
				LastAm = time(0);
			}
		}
		else
		{
			IF->LogPrint(3, "[IRC] Primeiro am ja foi, agora todo am sera logado\n");
			Irc_Say(MainChan, "Loguei ingame");

			FirstAm = 0;
		}
	}

	/* Liberar espaco alocado */
	for(x=0; x < (unsigned int)gmr.nlines; x++)
		free(Lines[x].Text);
	free(Lines);

	return 0;
}

void SeeAllPlayer(char *Chan)
{
	GameObject *Obj = IF->FindSerial(*IF->mserial);
	GameObject *Eu = Obj;
	unsigned int x = 0, count = 0;

	Obj = Obj->Next;
	Irc_Say(Chan, "Listando players por perto");
	for(x=0; x < (*IF->QtdObjects) - 1; x++)
	{
		if(Obj->Character!=0)
		{
			short x = (Eu->X - Obj->X), y =  (Eu->Y - Obj->Y);
			short Distancia = (short)sqrt( ( (x*x)+(y*y) ) );
			char *Where;
			IF->GetRefName(&Where, x, y);
			
			Irc_Say(Chan, "%d: [%s] Hp[%d] Ser[%#X] Z[%d] D[%d] Distancia[%d] L[%s]",
				count++, Obj->Name, IF->Life(Obj->Character->HitPoints, Obj->Character->MaxHitPoints), Obj->Serial,
				Obj->Z, Obj->Direction&7, Distancia, Where);
		}
		Obj = Obj->Next;
	}
	Irc_Say(Chan, "Encontrados %d players", count);
}

void ProcessMsg(unsigned char *buf)
{
	char Pkt[200] = {0};
	char Msgs[10][100]={0};

	sscanf(buf, "%99s %99s %99s %99s %99s %99s %99s %99s", Msgs[0], Msgs[1], Msgs[2], Msgs[3], Msgs[4], Msgs[5], Msgs[6], Msgs[7]);
	//IF->LogPrint(3, "[DEBUG] 0:[%s] 1:[%s] 2:[%s] 3:[%s]\n", Msgs[0], Msgs[1], Msgs[2], Msgs[3]);

	/* ident */
	if(!stricmp(buf, "NOTICE AUTH :*** No Ident response"))
	{
		strcpy(Pkt, "USER UoBot \"\" \"irc.brasnet.org\" :UoBot oWnZ\n");
		net_send(Pkt, strlen(Pkt));
		IF->LogPrint(3, "[IRC] Enviando ident\n");
		return;
	}

	/* servidor */
	if(Msgs[0][0] == ':' && Servidor[0] == 0)
	{
		strcpy(Servidor, Msgs[0]);
		IF->LogPrint(3, "[IRC] Servidor: %s\n", Servidor+1);
		return;
	}

	if(!stricmp(Msgs[0], Servidor))
	{
		if(LogIrc)
			IF->LogPrint(3, "[IRC] [%s] [%s]\n", Servidor, Msgs[2]);

		/* /names */
		if(!strcmp(Msgs[1], "353"))
		{
			char *ChanName = Msgs[4];
			char *SplitDue = strstr(buf+1, ":");
			unsigned char NickFlag = 0;
			SplitDue += 1;
			if(LogIrc)
				IF->LogPrint(3, "[IRC] [353] [%s]\n", SplitDue);
			SplitDue = strtok(SplitDue, " ");
			while(SplitDue != NULL)
			{
				NickFlag = 0;
				if(SplitDue[0] == '@')
				{
					NickFlag = 2;
					SplitDue++;
				}
				else if(SplitDue[0] == '+')
				{
					NickFlag = 1;
					SplitDue++;
				}

				User_Add(ChanName, SplitDue, NickFlag);
				SplitDue = strtok(NULL, " ");
			}

			return;
		}
	}

	/* invite */
	if(!strcmp(Msgs[1], "INVITE"))
	{
		if(strstr(Msgs[0], "skys"))
		{
			IF->LogPrint(3, "[IRC] Recebido convite para entrar no %s\n", Msgs[3]+1);
			Irc_Join(Msgs[3]+1);
		}
		return;
	}

	/* privmsg */
	if(!strcmp(Msgs[1], "PRIVMSG"))
	{
		char *NickName, *ChanName, *Texto, *LogTexto;

		LogTexto = StrChrNum(buf, ':', 2);

		NickName = StrTok(Msgs[0]);
		ChanName = Msgs[2];
		
		IF->LogPrint(1, "[IRC] [%s/%s] [%s]\n", ChanName, NickName, LogTexto);

		if(!strcmp(ChanName, IrcNick))
			return;

		if(Msgs[4][0] == 0)
			return;
		Texto = StrChrNum(buf, ' ', 4);

		if(Texto[0] == 0)
			return;

		if(!strcmp(Msgs[3]+1, IrcNick))
		{
			Process_PRIVMSG(ChanName, NickName, Texto);
		}

		return;
	}

	/* notice */
	if(!strcmp(Msgs[1], "NOTICE"))
	{
		char *Texto = StrChrNum(buf, ':', 2);

		IF->LogPrint(1, "[IRC] NOTICE: [%s] -> [%s]\n", StrTok(Msgs[0]), Texto);

		if(strstr(Msgs[0], "NickServ") && strstr(buf, "/NickServ IDENTIFY"))
		{
			IF->LogPrint(3, "[IRC] Identificando nick\n");
			Irc_Raw("NickServ IDENTIFY %s", IrcSenha);
			Irc_Raw("chanserv invite #ubteam");
			Irc_Join("#ubteam");
		}

		if(strstr(Msgs[0], "NickServ") && strstr(buf, "Senha aceita"))
		{
			IF->LogPrint(3, "[IRC] Nick identificado, entrando no %s\n", MainChan);
			Irc_Join(MainChan);
		}
		return;
	}

	/* join */
	if(!strcmp(Msgs[1], "JOIN"))
	{
		UserListTag *User = User_Add(StrTok(Msgs[2]), StrTok(Msgs[0]), 0);

		IF->LogPrint(1, "[IRC] User [%s] entrou no canal [%s]\n", User->NickName, StrTok(Msgs[2]));

		if(strstr(Msgs[0], "botdoskys"))
		{
			//IF->LogPrint(3, "[IRC] Entrei no %s, dando .identify\n", Msgs[2]+1);
			//Irc_Say(Msgs[2]+1, ".identify");
		}
		return;
	}

	/* part */
	else if(!strcmp(Msgs[1], "PART"))
	{
		char *NickName = StrTok(Msgs[0]);
		char *ChanName = Msgs[2];
		User_Del(NickName, ChanName);
		IF->LogPrint(1, "[IRC] User [%s] saiu do canal [%s]\n", NickName, ChanName);
		return;
	}

	/* kick */
	else if(!strcmp(Msgs[1], "KICK"))
	{
		char *NickName = Msgs[3];
		char *ChanName = Msgs[2];
		if(!strcmp(NickName, IrcNick))
		{
			IF->LogPrint(3, "[IRC] Fui kikado do canal [%s]\n", ChanName);
			Chan_Del(ChanName);
			return;
		}
		User_Del(NickName, ChanName);
		IF->LogPrint(1, "[IRC] User [%s] foi kikado do canal [%s]\n", NickName, ChanName);
		return;
	}

	/* quit */
	else if(!strcmp(Msgs[1], "QUIT"))
	{
		char *NickName = StrTok(Msgs[0]);
		User_Del(NickName, 0);
		IF->LogPrint(1, "[IRC] User [%s] deu quit\n", NickName);
		return;
	}

	/* mode */
	else if(!strcmp(Msgs[1], "MODE"))
	{
		char *ChanName = Msgs[2];
		char *Modes = Msgs[3];
		char *Nicks = strstr(buf, Modes) + strlen(Msgs[3]) + 1;

		if(!stricmp(Msgs[2], IrcNick))
			return;

		User_ChangeFlag(ChanName, Modes, Nicks);
		return;
	}

	/* nick */
	else if(!strcmp(Msgs[1], "NICK"))
	{
		char *OldNickName = StrTok(Msgs[0]);
		char *NickName = StrTok(Msgs[2]);
		User_ChangeNick(OldNickName, NickName);
		IF->LogPrint(1, "[IRC] Nick [%s] mudou para [%s]\n", OldNickName, NickName);
		return;
	}

}

void InitIrcVars(void)
{
	unsigned int a;
	for(a=0; a < ChanListCount; a++)
	{
		if(ChanList[a].UserList != 0)
			free(ChanList[a].UserList);

		ChanList[a].UserList = 0;
	}

	if(ChanList != 0)
		free(ChanList);

	ChanListCount = 0;
}

int IrcMain(LPVOID Inutil)
{
	unsigned char buf[2000];
	unsigned int x = 0;
	int len = 0;

	LineType *Lines = 0;
	unsigned int LineCount = 0;

	char *Line = buf;
	char *EndOfLine = 0;

	sprintf(buf, "NICK %s\n", IrcNick);

	/* Conectar ao irc */
	Access_Add("skys", 1);
Connect:
	if((sockIrc=net_connect(Server, (unsigned short)ServerPort))==SOCKET_ERROR)
	{
		IF->LogPrint(1, "[ERROR] Não foi possivel conectar\n");
		Sleep(2000);
		goto Connect;
	}
	
	/* Enviar nick */
	net_send(buf, strlen(buf));
	IF->LogPrint(3, "[IRC] Enviando nick %s\n", IrcNick);

	LastLine = 0;
	Servidor[0] = 0;

	while(1)
	{
		if((len=net_recv(buf, sizeof(buf))) <= 0)
		{
			IF->LogPrint(3, "[IRC] Conexao caiu\n");
			closesocket(sockIrc);
			sockIrc = 0;
			goto Connect;
		}
		buf[len] = 0;

		Line = buf;
		LineCount = 0;
		Lines = 0;

		while(*Line!=0)
		{
			if((EndOfLine = strstr(Line, "\r\n")) != 0)
			{
				*EndOfLine++ = 0;
				*EndOfLine++ = 0;
				LineCount++;
				Lines = realloc(Lines, LineCount*sizeof(LineType));
				if(LastLine == 0)
				{
					Lines[LineCount - 1].Text = malloc(strlen(Line)+1);
					strcpy(Lines[LineCount - 1].Text, Line);
				}
				else
				{
					Lines[LineCount - 1].Text = malloc(strlen(Line)+strlen(LastLine)+1);
					sprintf(Lines[LineCount - 1].Text, "%s%s", LastLine, Line);
					free(LastLine);
					LastLine = 0;
				}
				Line = EndOfLine;
			}
			else
			{
				if(LastLine!=0)
				{
					LastLine = realloc(LastLine, strlen(LastLine)+strlen(Line)+1);
					strcat(LastLine, Line);
				}
				else
				{
					LastLine = malloc(strlen(Line)+1);
					strcpy(LastLine, Line);
				}
				break;
			}
		}
	
		if(!LineCount)
			continue;
		for(x=0; x < LineCount; x++)
		{
			IF->LogPrint((LogIrc ? 3 : 0), "[IRC] [%d] [%s]\n", x, Lines[x].Text);
			ProcessMsg(Lines[x].Text);
		}

		/* Liberar o espaco alocado */
		for(x=0;x < LineCount; x++)
			free(Lines[x].Text);
		free(Lines);
	}
	return 0;
}

int IrcPingFunc(LPVOID Inutil)
{
	unsigned int PingRox=time(0);
	unsigned char buf[40]={0};

	Sleep(10000);
	while(1)
	{
		PingRox++;
		sprintf(buf, "PING %d\n", PingRox);
		net_send(buf, strlen(buf));
		Sleep(30000);
	}
	return 0;
}

void HandleConsole(unsigned char *msg)
{
	unsigned int len=strlen(msg)+strlen(LastChan);
	unsigned char *buf;
	if(!IrcConnected || LastChan[0]==0)
	{
		IF->LogPrint(1, "[IRC] Primeiro vc deve entrar em um canal, use /join\n");
		return;
	}
	buf=malloc(len+20);
	sprintf(buf, "PRIVMSG %s :%s\n", LastChan, msg);
	net_send(buf, strlen(buf));
	free(buf);
}

void Command_Connect(CmdInfoTag *CI)
{
	if(CI->Argc != 6)
	{
		IF->LogPrint(1, "[PLUGIN] Uso: %s <ip> <port> <nick> <senha> <chan>\n", CI->Args[0]);
		IF->CmdThreadExit(CI);
		return;
	}

	/* Adicinar handle antes que de o primeiro am */
	FirstAm = 1;
	IF->AddPacketHandler(0xb0, (void *)Uo_Handle_Gumps);
	IF->AddPacketHandler(0x3a, (void *)uo_handle_updateskill);

	Sleep(5000);
	/* Qdo conectar, zerar todos os timers e FirstAm */
	LastAm = 0;
	LastSave = 0;

	/* SEMPRE adicionar handlers ao conectar */
	IF->AddPacketHandler(0xae, (void *)Uo_Handle_Speech);
	IF->AddPacketHandler(0x1c, (void *)Uo_Handle_Speech);

	/* Se ja estiver conectado ao irc, nao eh necessario reconectar */
	if(IrcConnected != 0)
		while(1) Sleep(60000);

	IF->HandleCmd("/antistaff off");
	ServerPort = (unsigned short)ArgToInt(CI->Args[2]);	
	strncpy(Server, CI->Args[1], 100);

	strcpy(MainChan, CI->Args[5]);

	sprintf(IrcNick, "%s", CI->Args[3]);
	sprintf(IrcSenha, "%s", CI->Args[4]);

	if(!CheckedHDSerial)
	{
		while(!CheckPass())
		{
			IF->LogPrint(3, "[AUTH] Não consegui autenticar, serial:[%X]\n", HDSerial);
			Sleep(1000);
		}
		CheckedHDSerial = 1;
		IF->LogPrint(3, "[AUTH] Autenticado\n");
	}
	
	IF->LogPrint(3, "[IRC] Server/Port: [%s:%d] Nick/Senha:[%s/%s] MainChan:[%s]\n", Server, ServerPort, IrcNick, IrcSenha, MainChan);

	/* Criar uma thread para receber os pacotes do irc */
	if((hIrcThread = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)IrcMain, (LPVOID)0, 0, (LPDWORD)&hIrcThread))==0){
		IF->LogPrint(1, "[ERROR] Nao foi possivel criar a thred\n");
		IF->CmdThreadExit(CI);
		return;
	}

	/* Criar uma thread para enviar ping request */
	if((hIrcPingThread = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)IrcPingFunc, (LPVOID)0, 0, (LPDWORD)&hIrcPingThread))==0){
		IF->LogPrint(1, "[ERROR] Nao foi possivel criar a pingthread\n");
		closesocket(sockIrc);
		TerminateThread(hIrcThread, -1);
		IF->CmdThreadExit(CI);
		return;
	}

	/* Hookar o console para tudo que for digitado, ser enviado ao canal do irc */
	//IF->HookConsoleTo((void *)HandleConsole);
	IrcConnected = 1;
	IF->LogPrint(1, "[PLUGIN] Conectado no irc\n");
	while(1) Sleep(60000);
}

void Command_Disconnect(CmdInfoTag *CI)
{
	if(IrcConnected == FALSE)
	{
		IF->LogPrint(1, "[IRC] Voce nao esta conectando\n");
		IF->CmdThreadExit(CI);
		return;
	}
	Irc_Raw("QUIT");
	closesocket(sockIrc);
	TerminateThread(hIrcThread, -1);
	TerminateThread(hIrcPingThread, -1);
	hIrcThread = 0;
	hIrcPingThread = 0;
	IrcConnected = 0;
	//IF->HookConsoleTo((void *)0);

	/* desabilitar o anti-staff */
	IF->HandleCmd("/antistaff on");

	IF->LogPrint(1, "[IRC] Desconectado\n");
	IF->CmdThreadExit(CI);
}

void Command_JoinChannel(CmdInfoTag *CI)
{
	unsigned char buf[50]={0};
	if(IrcConnected == FALSE)
	{
		IF->LogPrint(1, "[IRC] Voce nao esta conectando\n");
		IF->CmdThreadExit(CI);
		return;
	}
	if(CI->Argc != 2)
	{
		IF->LogPrint(1, "[IRC] Uso: %s <canal>\n", CI->Args[0]);
		IF->CmdThreadExit(CI);
		return;
	}
	sprintf(buf, "JOIN %s\n", CI->Args[1]);
	net_send(buf, strlen(buf));
	IF->LogPrint(1, "[IRC] Tentando entrar no canal %s\n", CI->Args[1]);
	IF->CmdThreadExit(CI);
}

void Command_MainChan(CmdInfoTag *CI)
{
	if(CI->Argc != 2)
	{
		IF->LogPrint(2, "[IRC] Uso: %s [#chan]\n", CI->Args[0]);
		IF->CmdThreadExit(CI);
		return;
	}
	strcpy(MainChan, CI->Args[1]);
	IF->LogPrint(3, "[IRC] Novo MainChan %s\n", MainChan);
	Irc_Join(MainChan);
	IF->CmdThreadExit(CI);
}

void Command_LogIrc(CmdInfoTag *CI)
{
	if(LogIrc)
		LogIrc = 0;
	else
		LogIrc = 1;
	IF->LogPrint(3, "[IRC] Logging set to %s\n", (LogIrc? "on" : "off"));
	IF->CmdThreadExit(CI);
}

void Command_ListUsers(CmdInfoTag *CI)
{
	unsigned int a,b;
	for(a=0; a < ChanListCount; a++)
	{
		IF->LogPrint(3, "[CHAN] Listando users no canal %s\n", ChanList[a].ChanName);
		for(b=0; b < ChanList[a].UserListCount; b++)
		{
			IF->LogPrint(3, "[CHAN] %d: %d:%s\n", b, ChanList[a].UserList[b].NickFlag, ChanList[a].UserList[b].NickName);
		}
	}
	IF->CmdThreadExit(CI);
}

void Command_SayIrc(CmdInfoTag *CI)
{
	unsigned char buf[2000]={0};
	int x=0;

	if(IrcConnected == FALSE)
	{
		IF->LogPrint(1, "[IRC] Voce nao esta conectando\n");
		IF->CmdThreadExit(CI);
		return;
	}
	if(CI->Argc < 2)
	{
		IF->LogPrint(1, "[IRC] Uso: %s <msgs>\n", CI->Args[0]);
		IF->CmdThreadExit(CI);
		return;
	}

	for(x=0; x < CI->Argc-1; x++)
	{
		sprintf(buf, "%s%s ", buf, CI->Args[x+1]);
	}
	Irc_Say(MainChan, buf);
	IF->CmdThreadExit(CI);
}

void Command_RawMsg(CmdInfoTag *CI)
{
	unsigned char buf[2000]={0};
	int x=0;

	if(IrcConnected == FALSE)
	{
		IF->LogPrint(1, "[IRC] Voce nao esta conectando\n");
		IF->CmdThreadExit(CI);
		return;
	}
	if(CI->Argc < 2)
	{
		IF->LogPrint(1, "[IRC] Uso: %s <msgs>\n", CI->Args[0]);
		IF->CmdThreadExit(CI);
		return;
	}
	for(x=0; x < CI->Argc-1; x++)
	{
		sprintf(buf, "%s%s ", buf, CI->Args[x+1]);
	}
	IF->LogPrint(1, "[IRC] Raw: [%s]\n", buf);
	buf[strlen(buf)-1] = '\n';
	net_send(buf, strlen(buf));
	IF->CmdThreadExit(CI);
}

BOOL APIENTRY DllMain(HINSTANCE hDLLInst, DWORD fdwReason, LPVOID lpvReserved){
	return TRUE;
}

void Command_WS(CmdInfoTag *CI)
{
	WS = ~WS;
	IF->LogPrint(1, "[WS] Ws: [%s]\n", (WS ? "on" : "off"));
	IF->CmdThreadExit(CI);
}

int PluginInit(struct ExpFunc *TmpFunc)
{
	HMODULE hModule=GetModuleHandle(DLLNAME);
	IF = TmpFunc;
	IF->PluginAddCommand("connect", (void *)Command_Connect, hModule);
	IF->PluginAddCommand("disconnect", (void *)Command_Disconnect, hModule);
	IF->PluginAddCommand("join", (void *)Command_JoinChannel, hModule);
	IF->PluginAddCommand("raw", (void *)Command_RawMsg, hModule);
	IF->PluginAddCommand("mainchan", (void *)Command_MainChan, hModule);
	IF->PluginAddCommand("logirc", (void *)Command_LogIrc, hModule);
	IF->PluginAddCommand("listusers", (void *)Command_ListUsers, hModule);
	IF->PluginAddCommand("sayirc", (void *)Command_SayIrc, hModule);
	IF->PluginAddCommand("ws", (void *)Command_WS, hModule);
	IF->LogPrint(3, "[PLUGIN] Plugins Inicializado\n");

	return 1;
}

void PluginEnd(int ErrNum)
{
	if(ErrNum == 0)
	{
	//	Irc_Say(MainChan, "Desconectei do shard");
	}
	else if(ErrNum == ERRNUM_PINGTIMEOUT)
		Irc_Say(MainChan, "Desconectei do shard por ping timeout");
	else if(ErrNum == ERRNUM_RECONNECTTIME)
		Irc_Say(MainChan, "Desconectei do shard por reconnecttimeout");
	else if(ErrNum == ERRNUM_DESCONECTAR)
	{
		Irc_Say(MainChan, "Desconectei do shard pelo botao desconectar");
		if(IrcConnected)
		{
			Irc_Raw("QUIT");
			closesocket(sockIrc);
			TerminateThread(hIrcThread, -1);
			TerminateThread(hIrcPingThread, -1);
			//IF->HookConsoleTo((void *)0);
		}
		IrcConnected = 0;
	}
}