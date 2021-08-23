#include <math.h>
#include <windows.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include "uobot_log.h"
#include "uobot_net.h"
#include "uobot_handles.h"
#include "uobot_obj.h"
#include "uobot_jornal.h"
#include "uobot_target.h"
#include "uobot_spells.h"
#include "uobot_exthand.h"
#include "uobot_threads.h"
#include "uobot_plugins.h"
#include "uobot_hexcp.h"
#include "uobot_commands.h"
#include "uobot_windows.h"
#include "uobot_update.h"
#include "uobot_teleto.h"
#include "uobot.h"
#include "resource.h"

int ErrNum = 0;

NOTIFYICONDATA Notify = {0};

typedef struct {
	char *LName;
} LayerType;

LayerType LayerName[] = 
{
	{"One handed weapon"},
	{"Two handed weapon, shield"},
	{"Shoes"},
	{"Pants"},
	{"Shirt"},
	{"Helm/Hat"},
	{"Gloves"},
	{"Ring"},
	{"Unused"},
	{"Neck"},
	{"Hair"},
	{"Waist (half apron)"},
	{"Torso (inner) (chest armor)"},
	{"Bracelet"},
	{"Unused"},
	{"Facial Hair"},
	{"Torso (middle)"},
	{"Earrings"},
	{"Arms"},
	{"Back (cloak)"},
	{"Backpack"},
	{"Torso (outer) (robe)"},
	{"Legs (outer) (skirt/kilt)"},
	{"Legs (inner) (leg armor)"},
	{"Mount (horse, ostard, etc)"},
	{"NPC Buy Restock container"},
	{"NPC Buy no restock container"},
	{"NPC Sell container"},
	{"PC Bank"}
};

ReagTag Reags[8] = 
{
	{"BP", G_BP, 0},
	{"BM", G_BM, 0},
	{"MR", G_MR, 0},
	{"SA", G_SA, 0},
	{"SS", G_SS, 0},
	{"NS", G_NS, 0},
	{"GA", G_GA, 0},
	{"GS", G_GS, 0},
};

unsigned short m_speechcolor = 0x2b2;

unsigned int ReconnectTime=0;
unsigned int LoginTime=0;
unsigned int PingTimeout = 100;

int SeeStaffType = 0;
int CancelPage = 0;


char AvisoFile[50] = "c:\\am.mp3";
int AceitarCoisas = 0;
int LastAm = 0;
int RespAm = 1;
int InsertM2aCrypt = 1;
int AntiStaff = 1;
int DetectSpeech = 0;

void Command_Warmode(CmdInfoTag *CI)
{
	if(CI->Argc != 2)
	{
		LogPrint(1, "[WARMODE] Uso: %s 0/1\n", CI->Args[0]);
		return;
	}
	WarMode(atoi(CI->Args[1]));
	LogPrint(1, "[WARMODE] Changin warmode to: %d\n", atoi(CI->Args[1]));
}

void Command_Teleto(CmdInfoTag *CI)
{
	GameObject *obj;

	if((obj=FindSerial(mserial))==0)
	{
		LogPrint(1, "[TELETO] Voce precisa estar conectado\n");
		return;
	}

    if(CI->Argc != 3)
	{
		LogPrint(1, "[TELETO] Uso: %s go/rew namefle\n", CI->Args[0]);
		return;
	}

	if(!strcmp(CI->Args[1], "go") && CI->Argc == 3)
	{
        if(!Teleto_Go(CI->Args[2]))
        {
            LogPrint(3, "[TELETO] Erro reproduzindo %s\n", CI->Args[2]);
            return;
        }
        return;
   	}

	else if(!strcmp(CI->Args[1], "rew") && CI->Argc == 3)
	{
        if(!Teleto_Rew(CI->Args[2]))
        {
            LogPrint(3, "[TELETO] Erro reproduzindo %s\n", CI->Args[2]);
            return;
        }
        return;
	}

    else
	{
		LogPrint(1, "[TELETO] Uso: %s go/rew [moveFile]", CI->Args[0]);
		return;
	}

	return;
}

void Command_Disarm(CmdInfoTag *CI)
{
	unsigned int ItemLay1, ItemLay2;
	GameObject *Obj;
	ItemLay1 = GetItemInLayer(mserial, 1);
	ItemLay2 = GetItemInLayer(mserial, 2);
	if(ItemLay1!=INVALID_SERIAL)
	{
		MoveToContainer(ItemLay1, 1, mbackpack);
		Obj=FindSerial(ItemLay1);
		LogPrint(3, "[DISARM] Movendo item [%#X] [%s] para a mochila\n",
			Obj->Serial, Obj->Name);
	}
	if(ItemLay2!=INVALID_SERIAL)
	{
		MoveToContainer(ItemLay2, 1, mbackpack);
		Obj=FindSerial(ItemLay2);
		LogPrint(3, "[DISARM] Movendo item [%#X] [%s] para a mochila\n",
			Obj->Serial, Obj->Name);
	}
}

void Command_LoadDll(CmdInfoTag *CI)
{
	if(CI->Argc != 2)
	{
		LogPrint(1, "[LOADPL] Uso: %s plugin.dll\n", CI->Args[0]);
		return;
	}
	if(LoadPluginDll(CI->Args[1]))
		LogPrint(3, "[LOADDLL] Dll %s loaded\n", CI->Args[1]);
	else
		LogPrint(3, "[LOADDLL] Nao foi possivel abrir dll %s\n", CI->Args[1]);

}

void Command_KillAllThreads(CmdInfoTag *CI)
{
	unsigned int count=0;
	count = CmdKillAll();	
	LogPrint(1, "[KILLALL] Kilado %d threads\n", count);
}

void Command_KillThread(CmdInfoTag *CI)
{
	CmdInfoTag *TmpTag;
	int Idx;
	if(CI->Argc != 2)
	{
		LogPrint(1, "[KILLTHREAD] Uso: %s thread_handle\n", CI->Args[0]);
		return;
	}
	LogPrint(1, "[KILLTHREAD] Tentando kilar thread %d\n", atoi(CI->Args[1]));
	if((TmpTag=GetThreadCI((HANDLE)atoi(CI->Args[1])))==0)
	{
		LogPrint(3, "[KILLTHREAD] Esta thread ja morreu\n");
		return;
	}
	if((Idx=GetIdxFromInfo(TmpTag))==-1)
	{
		LogPrint(3, "[KILLTHREAD] Esta thread ja morreu ou nao pode ser kilada\n");
		return;
	}
	if(CmdThreads[Idx].IsSuspended)
		CmdThreadKill(TmpTag);
	else 
	{
		CmdThreads[Idx].IsSuspended = 1;
		while(SuspendThread(CmdThreads[Idx].hHandle)==-1) Sleep(50);
		CmdThreadKill(TmpTag);
	}
}

void Command_ListCmds(CmdInfoTag *CI)
{
	ListCommands();
}

void Command_ListDlls(CmdInfoTag *CI)
{
	ListPluginsRunning();
}

void Command_ListThreads(CmdInfoTag *CI)
{
	CmdListThreads();
}

void Command_AllNames(CmdInfoTag *CI)
{
	unsigned int X,count=0;
	GameObject *Obj=&gobjects, *Eu=&gobjects;

	LogPrint(1, "[ALLNAMES] Players\n");
	for(X=0; X < gobjcount; X++)
	{
		if(Obj->Character!=0)
		{
			short x = (Eu->X - Obj->X), y =  (Eu->Y - Obj->Y);
			short Distancia = (short)sqrt( ( (x*x)+(y*y) ) );
			char *Where;
			GetRefName(&Where, x, y);

			LogPrint(1, "%d: [%s] Hp[%d] Ser[%#X] X[%d] Y[%d] Z[%d] D[%s] Distancia[%d] L[%s]\n",
				count++, Obj->Name, Life(Obj->Character->HitPoints, Obj->Character->MaxHitPoints), Obj->Serial,
				Obj->X, Obj->Y, Obj->Z, DirNames[Obj->Direction&7].Name, Distancia, Where);
		}
		Obj = Obj->Next;
	}
	LogPrint(1, "[ALLNAMES] Encontrados %d players\n", count);
}

void Command_UnloadDll(CmdInfoTag *CI)
{
	if(CI->Argc != 2)
	{
		LogPrint(3, "[UNDLL] Uso: %s plugin.dll\n", CI->Args[0]);
		return;
	}
	PluginRemove(CI->Args[1], -1);
}

void Command_ListAllItens(CmdInfoTag *CI)
{
	GameObject *Obj=&gobjects;
	unsigned int x=0,count=0;
	LogPrint(3, "[LITENS] Listando todos os itens\n");
	for(x=0; x < gobjcount; x++)
	{
		LogPrint(3, "[LITENS] [%d] Ser:%#X G:%#X C:%u Qtd:%u IsCont:%s\n",
			count, Obj->Serial, Obj->Graphic, Obj->Color, Obj->Quantity, (Obj->IsContainer ? "sim":"nao"));
		LogPrint(3, "[LITENS] [%d] [%s]\n", count, Obj->Name);
		count++;
		Obj = Obj->Next;
	}
	LogPrint(3, "[LITENS] Fim da lista, listados %d objetos\n", count);
}


void Command_ListItens(CmdInfoTag *CI)
{
	GameObject *Obj=&gobjects;
	unsigned int x=0,count=0;
	LogPrint(1, "[LITENS] Listando itens na sua mochila\n");
	for(x=0; x < gobjcount; x++)
	{
		if(Obj->Container == mbackpack)
		{
			LogPrint(1, "[LITENS] [%d] Ser:%#X G:%#X C:%u Qtd:%u IsCont:%s\n",
				count, Obj->Serial, Obj->Graphic, Obj->Color, Obj->Quantity, (Obj->IsContainer ? "sim":"nao"));
			LogPrint(1, "[LITENS] [%d] [%s]\n", count, Obj->Name);
			count++;
		}
		Obj = Obj->Next;
	}
	LogPrint(1, "[LITENS] Fim da lista, listados %d objetos\n", count);
}

void Command_ListItensInLayer(CmdInfoTag *CI)
{
	GameObject *Obj=&gobjects;
	unsigned int x=0,count=0;
	LogPrint(1, "[LITENS] Listando itens no seu corpo\n");
	for(x=0; x < gobjcount; x++)
	{
		if(Obj->Container == mserial)
		{
			LogPrint(1, "[LITENS] [%d] [%s] Layer: [%d] [%s]\n",count, Obj->Name, Obj->Layer, LayerName[Obj->Layer-1].LName);
			LogPrint(1, "[LITENS] [%d] Ser:%#X G:%#X C:%u Qtd:%u IsCont:%s\n",
				count, Obj->Serial, Obj->Graphic, Obj->Color, Obj->Quantity, (Obj->IsContainer ? "sim":"nao"));
			count++;
		}
		Obj = Obj->Next;
	}
	LogPrint(1, "[LITENS] Fim da lista, listados %d objetos\n", count);
}

void Command_SeeJournal(CmdInfoTag *CI)
{
	unsigned int x=0,count=0;
	LogPrint(1, "[JORNAL] Listando as linhas no jornal\n");
	for(x=0; x < JournalCount; x++)
	{
		if(Journal[x].Text!=NULL)
			LogPrint(3, "[JORNAL] [%d] %s\n",
			count, Journal[x].Text);
		count++;
	}
	LogPrint(1, "[JORNAL] Listado [%d] linhas\n", count);
}

void Command_ReqObjNames(CmdInfoTag *CI)
{
	GameObject *Obj=&gobjects;
	char *ClickPkt=0;
	unsigned int x=0,count=0;
	LogPrint(1, "[LITENS] Clicando nos itens em sua mochila/corpo\n");
	for(x=0; x < gobjcount; x++)
	{
		if(Obj->Container == mserial || Obj->Container == mbackpack)
		{
			count++;
			ClickPkt = realloc(ClickPkt, count*5);
			ClickPkt[(count-1)*5] = 0x09;
			pack32(ClickPkt+((count-1)*5)+1, Obj->Serial); 
		}
		Obj = Obj->Next;
	}
	send_server(ClickPkt, count*5);
	free(ClickPkt);
	LogPrint(1, "[LITENS] Clickado em %d objetos\n", count);
}

void Command_ReqAllObjNames(CmdInfoTag *CI)
{
	GameObject *Obj=&gobjects;
	char *ClickPkt=0;
	unsigned int x=0,count=0;
	LogPrint(1, "[LITENS] Clicando em todos os itens\n");
	for(x=0; x < gobjcount; x++)
	{
		count++;
		ClickPkt = realloc(ClickPkt, count*5);
		ClickPkt[(count-1)*5] = 0x09;
		pack32(ClickPkt+((count-1)*5)+1, Obj->Serial); 
		Obj = Obj->Next;
	}
	send_server(ClickPkt, count*5);
	free(ClickPkt);
	LogPrint(1, "[LITENS] Clickado em %d objetos\n", count);
}


void Command_CleanJournal(CmdInfoTag *CI)
{
	CleanJournal();
}

void Command_ClickObj(CmdInfoTag *CI)
{
	GameObject *obj;
	if(CI->Argc != 2)
	{
		LogPrint(1, "[CLICK] Uso: %s serial\n", CI->Args[0]);
		return;
	}
	if((obj=FindSerial(ArgToInt(CI->Args[1])))==0)
	{
		LogPrint(1, "[CLICK] Nao foi possivel achar serial: [%#x]\n", CI->Args[1]);
		return;
	}

	ClickObject(obj->Serial);
}

void Command_SetLogFlag(CmdInfoTag *CI)
{
	if(CI->Argc != 2)
	{
		LogPrint(1, "[SETLOGFLAG] Uso: %s on/off\n", CI->Args[0]);
		return;
	}
	if(!strcmp(CI->Args[1], "on"))
	{
		LogFlag = 0;
		LogPrint(1, "[SETLOGFLAG] LogFlag setado para on\n");
	}
	else if(!strcmp(CI->Args[1], "off"))
	{
		LogPrint(1, "[SETLOGFLAG] LogFlag setado para off\n");
		LogFlag = 1;
	}
	else
		LogPrint(1, "[SETLOGFLAG] Uso: %s on/off\n", CI->Args[0]);
}

void Command_SetDebug(CmdInfoTag *CI)
{
	if(CI->Argc != 2)
	{
		LogPrint(1, "[SETDEBUG] Uso: %s on/off\n", CI->Args[0]);
		return;
	}
	if(!strcmp(CI->Args[1], "on"))
	{
		LogPrint(1, "[SETDEBUG] Debug setado para on\n");
		PrintDebug = 1;
	}
	else if(!strcmp(CI->Args[1], "off"))
	{
		LogPrint(1, "[SETDEBUG] Debug setado para off\n");
		PrintDebug = 0;
		DebugClientMsg = 0;
	}
	else if(!strcmp(CI->Args[1], "on2"))
	{
		LogPrint(1, "[SETDEBUG] Debug clientmsg setado para on\n");
		DebugClientMsg = 1;
	}
	else
		LogPrint(1, "[SETDEBUG] Uso: %s on/off\n", CI->Args[0]);
}


void Command_ListSkills(CmdInfoTag *CI)
{
	unsigned int x=0,count=0;
	LogPrint(1, "[LISTSKILLS] Listando skills\n");
	for(x=0; x < SkillsSize; x++)
	{
		if(SkillsName[x].LastValue == 0) continue;
		LogPrint(1, "[SKILL] [%d] [%s] [%.1f]\n",
			count, SkillsName[x].Name, (double)SkillsName[x].LastValue/10);
		count++;
	}
	LogPrint(1, "[LISTSKILLS] Listados %d skills\n", count);
}

void Command_FindName(CmdInfoTag *CI)
{
	GameObject *Obj=&gobjects;
	unsigned int x=0,count=0;
	if(CI->Argc != 2)
	{
		LogPrint(1, "[FINDNAME] Uso: %s name\n", CI->Args[0]);
		return;
	}
	LogPrint(1, "[FINDNAME] Procurando nome [%s]\n", CI->Args[1]);
	for(x=0; x < gobjcount; x++)
	{
		if(strstr(Obj->Name, CI->Args[1])!=0)
		{
			LogPrint(1, "[GRAPHIC] [%d] Name: [%s]\n", count, Obj->Name);
			LogPrint(1, "[GRAPHIC] [%d] Ser:%#X G:%#X C:%u Qtd:%u IsCont:%s\n",
				count, Obj->Serial, Obj->Graphic, Obj->Color, Obj->Quantity, (Obj->IsContainer ? "sim":"nao"));
			count++;
		}
		Obj = Obj->Next;
	}
	LogPrint(1, "[FINDNAME] Achado %d objetos\n", count);
}

void Command_FindGraphic(CmdInfoTag *CI)
{
	unsigned short Graphic;
	GameObject *Obj=&gobjects;
	unsigned int x=0,count=0;
	if(CI->Argc != 2)
	{
		LogPrint(1, "[FINDGRAPHIC] Uso: %s graphic\n", CI->Args[0]);
		return;
	}
	Graphic = (unsigned short)ArgToInt(CI->Args[1]);
	LogPrint(1, "[FINDGRAPHIC] Procurando graphic [%#X]\n", Graphic);
	for(x=0; x < gobjcount; x++)
	{
		if(Obj->Graphic==Graphic)
		{
			LogPrint(1, "[GRAPHIC] [%d] Name: [%s]\n", count, Obj->Name);
			LogPrint(1, "[GRAPHIC] [%d] Ser:%#X G:%#X C:%u Qtd:%u IsCont:%s\n",
				count, Obj->Serial, Obj->Graphic, Obj->Color, Obj->Quantity, (Obj->IsContainer ? "sim":"nao"));
			count++;
		}
		Obj = Obj->Next;
	}
	LogPrint(1, "[FINDGRAPHIC] Achado %d objetos\n", count);
}

void Command_ViewMemoUse(CmdInfoTag *CI)
{
	unsigned int Total=0;
	LogPrint(1, "[MEMO] Objects: [%d] bytes\n", sizeof(GameObject)*gobjcount);
	Total += sizeof(GameObject)*gobjcount;
	LogPrint(1, "[MEMO] Jornal: [%d] bytes\n", sizeof(JournalEntry)*JournalCount);
	Total += sizeof(JournalEntry)*JournalCount;
	LogPrint(1, "[MEMO] Plugins: [%d] bytes\n", sizeof(PluginsList));
	Total += sizeof(PluginsList);
	LogPrint(1, "[MEMO] Cmds: [%d] bytes\n", sizeof(Cmd)*CmdListSize);
	Total += sizeof(Cmd)*CmdListSize;
	LogPrint(1, "[MEMO] Windows: [%d] bytes\n", MAXTEXTSIZE);
	Total += MAXTEXTSIZE;
	LogPrint(1, "[MEMO] Threads: [%d] bytes\n", sizeof(CmdThreadsTag)*CmdThreadsCount);
	Total += sizeof(CmdThreadsTag)*CmdThreadsCount;
	LogPrint(1, "[MEMO] Total: [%.3f] Kbytes\n", (double)Total/1024);
}

void Command_SetJournal(CmdInfoTag *CI)
{
	if(CI->Argc != 2)
	{
		LogPrint(1, "[SETJOR] Uso: %s on/off\n", CI->Args[0]);
		return;
	}
	if(!strcmp(CI->Args[1], "on"))
	{
		LogPrint(1, "[SETJOR] Jornal on\n");
		JournalReady = 1;
	}
	else if(!strcmp(CI->Args[1], "off"))
	{
		LogPrint(1, "[SETJOR] Jornal off\n");
		JournalReady = 0;
	}
	else
		LogPrint(1, "[SETJOR] Uso: %s on/off\n", CI->Args[0]);
}

void Command_ListReags(CmdInfoTag *CI)
{
	unsigned int x=0;
	LogPrint(1, "[REAGS] Listando 8 reags\n");
	for(x=0; x < 8; x++)
	{
		Reags[x].Qtd = GetQuantity(mbackpack, Reags[x].Graphic);
		LogPrint(1, "%s: %d\n", Reags[x].Name, Reags[x].Qtd);
	}
	LogPrint(1, "[REAGS] Listados 8 reags\n");
}

void Command_ContainerList(CmdInfoTag *CI)
{
	unsigned int x=0, Container=0,count=0;
	GameObject *Obj=&gobjects;
	if(CI->Argc != 2)
	{
		LogPrint(1, "[LIST] Uso: %s container_serial\n", CI->Args[0]);
		return;
	}
	Container = ArgToInt(CI->Args[1]);
	LogPrint(1, "[LIST] Procurando itens no container [%#x]\n", Container);
	for(x=0; x < gobjcount; x++)
	{
		if(Obj->Container == Container)
		{
			LogPrint(1, "%d: N:[%s] S:[%#x] G:[%#x] C:[%#x]\n",
				count, Obj->Name, Obj->Serial, Obj->Graphic, Obj->Color);
			count++;
		}
		Obj= Obj->Next;
	}
	LogPrint(1, "[LIST] Encontrados [%d] itens no container [%#x]\n", count, Container);
}

void Command_WaitTarget(CmdInfoTag *CI)
{
	unsigned int Serial=0;
	if(CI->Argc != 2)
	{
		LogPrint(1, "[WAITTARGET] Uso: %s serial\n", CI->Args[0]);
		return;
	}
	Serial = ArgToInt(CI->Args[1]);
	Target_WaitTarget(Serial);
	LogPrint(1, "[WAITTARGET] WaitTarget setado para [%#x]\n", Serial);
}

void Command_UseObject(CmdInfoTag *CI)
{
	unsigned int Serial=0;
	if(CI->Argc != 2)
	{
		LogPrint(1, "[USEOBJECT] Uso: %s serial\n", CI->Args[0]);
		return;
	}
	Serial = ArgToInt(CI->Args[1]);
	UseObject(Serial);
	LogPrint(1, "[USEOBJECT] Dois cliques em [%#x]\n", Serial);
}

void Command_UseObjectType(CmdInfoTag *CI)
{
	unsigned short Graphic=0;
	if(CI->Argc != 2)
	{
		LogPrint(1, "[USEOBJECTTYPE] Uso: %s graphic\n", CI->Args[0]);
		return;
	}
	Graphic = (unsigned short)ArgToInt(CI->Args[1]);
	UseObjectType(Graphic);
	LogPrint(1, "[USEOBJECTTYPE] Type [%#x]\n", Graphic);
}

void Command_SeeIp(CmdInfoTag *CI)
{
	SOCKET Http;
	int ret = 0;
	//char buf[500] = {0}, siteip[] = "www.whatismyip.com", myip[50] = {0};
	char buf[500] = {0}, siteip[] = "www.uobot.com.br", myip[50] = {0};
	char *recvbuf = 0, *point = 0;
	int recvbufsize = 0;

	
	sprintf(buf,	"GET /seemyip.php HTTP/1.1\r\n"
					"Host: www.uobot.com.br\r\n"
					"Accept: */*\r\n"
					"User-Agent: Mozilla/4.0 (compatible; MSIE 4.01; Windows 98)\r\n"
					"Pragma: no-cache\r\n"
					"Cache-Control: no-cache\r\n"
					"Connection: close\r\n\r\n");

	CheckHang = 0;

	if( (Http = net_connect(siteip, 80)) == SOCKET_ERROR)
	{
		LogPrint(3, "[ERROR] Nao foi possivel conectar no [%s]\n", siteip);
		LastMessageTime = time(0);
		CheckHang = 1;
		return;
	}


	if((ret = send(Http, buf, strlen(buf), 0)) <= 0)
	{
		LogPrint(3, "[ERROR] Nao foi possivel enviar http request\n");
		closesocket(Http);
		LastMessageTime = time(0);
		CheckHang = 1;
		return;
	}

	ret = 0;
	do {
		recvbufsize += ret;
		recvbuf = realloc(recvbuf, recvbufsize+201);
	} while( (ret = recv(Http, recvbuf+recvbufsize, 200, 0)) > 0);

	recvbuf[recvbufsize] = 0;

	closesocket(Http);

//	if( (point = strstr(recvbuf, "Your IP")) == NULL)
	if( (point = strstr(recvbuf, "seu ip:")) == NULL)
	{
		LogPrint(3, "[ERROR] Nao foi possivel localizar meu ip\n");
		free(recvbuf);
		LastMessageTime = time(0);
		CheckHang = 1;
		return;
	}

	point += 9;

	//sscanf(point, "%*s %*s %*s %20s", myip);
	
	sscanf(point, "%20s", myip);

	LogPrint(3, "[IP] Seu ip atual eh [%s]\n", myip);

	free(recvbuf);

	LastMessageTime = time(0);
	CheckHang = 1;

}

void Command_SetSpeechColor(CmdInfoTag *CI)
{
	if(CI->Argc != 2)
	{
		LogPrint(1, "[SPEECHCOLOR] Uso: %s [num]\n", CI->Args[0]);
		return;
	}

	m_speechcolor = (unsigned short)ArgToInt(CI->Args[1])&0xFFFF;

	LogPrint(3, "[SPEECHCOLOR] Nova cor: [0x%X]\n", m_speechcolor);
}


void Command_GetSize(CmdInfoTag *CI)
{
	DWORD sc_min, sc_max;
	GetScrollRange(GetDlgItem(MainWnd, 1001), SB_VERT, &sc_min, &sc_max);
	LogPrint(3, "[DBG] %d\n", GetWindowTextLength(GetDlgItem(MainWnd, 1001)));
	LogPrint(3, "[DBG] min: %d | max: %d\n", sc_min, sc_max);
	Sleep(1000);
	SetScrollPos(GetDlgItem(MainWnd, 1001), SB_VERT, GetScrollPos(GetDlgItem(MainWnd, 1001), SB_VERT)+1, TRUE);
}

void Command_ReconnectTime(CmdInfoTag *CI)
{
	if(CI->Argc == 1)
	{
		if(ReconnectTime == 0)
			LogPrint(3, "[RECONNECT] ReconnectTime: desativado\n");
		else
			LogPrint(3, "[RECONNECT] ReconnectTime: %d minutos\n", ReconnectTime/60);

		LogPrint(3, "[RECONNECT] Uso: %s tempo\n", CI->Args[0]);
		LogPrint(3, "[RECONNECT] O tempo é em minutos, 10800 = 3h, 7200 = 2h\n");
		LogPrint(3, "[RECONNECT] Para desativar, use: %s 0\n", CI->Args[0]);
		return;
	}

	if(CI->Argc == 2)
	{
		ReconnectTime = ArgToInt(CI->Args[1])*60;
		LogPrint(3, "[RECONNECT] ReconnectTime atualizado para %d minutos\n", ReconnectTime/60);
		return;
	}
}

void Command_Resync(CmdInfoTag *CI)
{
	SendResync();
	LogPrint(1, "[RESYNC] Sincronizando\n");
}

void Command_Status(CmdInfoTag *CI)
{
	LogPrint(3, "[STATUS] No momento atual seu uobot esta [%s]\n", (hMainThread ? "ligado": "desligado"));
	if(hMainThread)
	{
		if(LoginTime)
			LogPrint(3, "[STATUS] Voce esta logado ha [%d] minutos\n", (time(0) - LoginTime)/60);

		if(ReconnectTime)
		{
			if(!LoginTime)
				LogPrint(3, "[STATUS] ReconnectTime esta ligado em [%d] minutos\n", ReconnectTime/60);
			else
				LogPrint(3, "[STATUS] ReconnectTime esta ligado em [%d] minutos, faltam [%d] minutos para desconectar\n", ReconnectTime/60, (ReconnectTime - (time(0) - LoginTime))/60);
		}
	}
	if(AvisoFile[0])
		LogPrint(3, "[STATUS] Arquivo de musica para aviso de staff: [%s]\n", AvisoFile);

}

void Command_Stats(CmdInfoTag *CI)
{
	GameObject *Player = FindSerial(mserial);
	if(Player==0)
		return;
	LogPrint(3, "[STATS] Dex: [%d]\n", Player->Character->DEX);
	LogPrint(3, "[STATS] Int: [%d]\n", Player->Character->INT);
	LogPrint(3, "[STATS] Str: [%d]\n", Player->Character->STR);
}

void Command_Walk(CmdInfoTag *CI)
{
	unsigned int x=0;
	unsigned int NumWalks = 0;

	if(CI->Argc != 3)
	{
		LogPrint(3, "[WALK] Uso: %s direcao passos\n", CI->Args[0]);
		return;
	}

	NumWalks = ArgToInt(CI->Args[2]);

	for(x=0; x < 8; x++)
	{
		if(!stricmp(DirNames[x].Name, CI->Args[1]))
		{
			unsigned int a;

			for(a=0; a < NumWalks; a++)
				Walk(CI->Args[1]);

			LogPrint(3, "[WALK] Andado na direcao [%s], [%d] vezes\n", DirNames[x].Name, NumWalks);
			return;
		}
	}
	LogPrint(3, "[WALK] Nao foi achado essa direcao\n");
	LogPrint(3, "[WALK] Direcoes: norte/nordeste/noroeste/sul/sudeste/sudoeste/leste/oeste\n");
}

void Command_SeeNews(CmdInfoTag *CI)
{
	int i = HttpCheckVers();
	if(!i)
		LogPrint(3, "[SEENEWS] Não há nova versão disponivel\n");
}

void Command_SetAviso(CmdInfoTag *CI)
{
	if(CI->Argc == 1)
	{
		LogPrint(3, "[SETAVISO] Música atual é: [%s]\n", AvisoFile);
		LogPrint(3, "[SETAVISO] Uso: %s musica\n", CI->Args[0]);
		return;
	}

	else if(CI->Argc == 2)
	{
		strncpy(AvisoFile, CI->Args[1], 49);
		LogPrint(3, "[SETAVISO] Música mudada para [%s]\n", AvisoFile);
		return;
	}

	else
		LogPrint(3, "[SETAVISO] Uso: %s musica\n", CI->Args[0]);
}

void Command_AceitarTrade(CmdInfoTag *CI)
{
	if(CI->Argc!=2)
	{
		LogPrint(3, "[ACCEPT] Uso: %s on/off\n", CI->Args[0]);
		return;
	}

	if(!stricmp(CI->Args[1], "on"))
	{
		AceitarCoisas = 1;
		LogPrint(3, "[ACCEPT] Responder trade/gate/aceitar animais/marcar itens excepcionais automaticamente\n");
		return;
	}
	else
	{
		AceitarCoisas = 0;
		LogPrint(3, "[ACCEPT] Responder gumps/gate/trade automatico off\n");
		return;
	}
}
void Command_SetRespAm(CmdInfoTag *CI)
{
	if(CI->Argc!=2)
	{
		LogPrint(3, "[RESPAM] Uso: %s on/off\n", CI->Args[0]);
		return;
	}

	if(!stricmp(CI->Args[1], "on"))
	{
		RespAm = 1;
		LogPrint(3, "[RESPAM] Respondedor de am on\n");
		return;
	}
	else
	{
		RespAm = 0;
		LogPrint(3, "[RESPAM] Respondedor de am off\n");
		return;
	}
}

void Command_SetTimeout(CmdInfoTag *CI)
{
	if(CI->Argc == 1)
	{
		LogPrint(3, "[SETTIMEOUT] Timeout atual é: [%d] segundos\n", PingTimeout);
		LogPrint(3, "[SETTIMEOUT] Uso: %s timeout\n", CI->Args[0]);
		return;
	}

	else if(CI->Argc == 2)
	{
		PingTimeout = ArgToInt(CI->Args[1]);
		LogPrint(3, "[SETTIMEOUT] Timeout mudado para [%d]\n", PingTimeout);
		return;
	}

	else
		LogPrint(3, "[SETTIMEOUT] Uso: %s timeout\n", CI->Args[0]);
}

void Command_SetShardIp(CmdInfoTag *CI)
{
	if(CI->Argc == 1)
	{
		LogPrint(3, "[SHARDIP] ShardIp atual é: [%s]\n", SHARDADDR);
		LogPrint(3, "[SHARDIP] Uso: %s ip\n", CI->Args[0]);
		return;
	}

	else if(CI->Argc == 2)
	{
		strcpy(SHARDADDR, CI->Args[1]);
		LogPrint(3, "[SHARDIP] ShardIp mudado para [%s]\n", SHARDADDR);
		return;
	}

	else
		LogPrint(3, "[SHARDIP] Uso: %s ip\n", CI->Args[0]);
}


void Command_HideWindow(CmdInfoTag *CI)
{
	memset(&Notify, 0, sizeof(NOTIFYICONDATA));
	Notify.cbSize = sizeof(NOTIFYICONDATA);
	Notify.hWnd = MainWnd;
	Notify.hIcon = LoadIcon(CurInstance,  MAKEINTRESOURCE(IDI_ICON4));
	strcpy(Notify.szTip, "UoBoT");
	Notify.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
	Notify.uCallbackMessage = WM_USER + 1;
	MBOut("HideWindow", "Hidando janela");
	Shell_NotifyIcon(NIM_ADD, &Notify);
	ShowWindow(MainWnd, SW_HIDE);
}

void Command_M2aCrypt(CmdInfoTag *CI)
{
	if(CI->Argc != 2)
	{
		LogPrint(3, "[M2ACRYPT] M2aCrypt esta: [%s]\n", (InsertM2aCrypt ? "on":"off"));
		LogPrint(3, "[M2ACRYPT] Uso: %s on/off\n", CI->Args[0]);
		return;
	}

	if(!stricmp(CI->Args[1], "on"))
	{
		InsertM2aCrypt = 1;
		LogPrint(3, "[M2ACRYPT] M2aCrypt esta on\n");
		return;
	}
	else
	{
		InsertM2aCrypt = 0;
		LogPrint(3, "[M2ACRYPT] M2aCrypt esta off\n");
		return;
	}
}

void Command_AntiStaff(CmdInfoTag *CI)
{
	if(CI->Argc!=2)
	{
		LogPrint(3, "[ANTISTAFF] Anti-Staff esta [%s]\n", (AntiStaff ? "on":"off"));
		LogPrint(3, "[ANTISTAFF] Uso: %s on/off\n", CI->Args[0]);
		return;
	}

	if(!stricmp(CI->Args[1], "on"))
	{
		AntiStaff = 1;
		LogPrint(3, "[ANTISTAFF] Anti-Staff on\n");
		return;
	}
	else
	{
		AntiStaff = 0;
		LogPrint(3, "[ANTISTAFF] Anti-Staff off\n");
		return;
	}
}

void Command_SeeStaff(CmdInfoTag *CI)
{
	unsigned char HelpPacket[258];

	memset(HelpPacket, 0, 258);
	HelpPacket[0] = 0x9b;

	SeeStaffType = 1;
	/* conselheiro */
	send_server(HelpPacket, 258);

	return;
}

void Command_CancelPage(CmdInfoTag *CI)
{
	unsigned char HelpPacket[258];

	memset(HelpPacket, 0, 258);
	HelpPacket[0] = 0x9b;

	/* cancelar page */
	CancelPage = 1;
	send_server(HelpPacket, 258);

	return;
}

int DetectStaff = 0;
HANDLE DetectStaffHandle = 0;
DWORD DetectStaffThread = 0;

/* thread que vai ficar enviando o help */
int MyWatchForStaff(LPVOID Inutil)
{
	unsigned char HelpPacket[258];

	/* help packet */
	memset(HelpPacket, 0, 258);
	HelpPacket[0] = 0x9b;

	do {
		SeeStaffType = 1;
		send_server(HelpPacket, 258);
		do{
			Sleep(2000);
		} while(SeeStaffType != 0);
	} while(1);

	return 0;
}

void Command_DetectStaff(CmdInfoTag *CI)
{
	if(CI->Argc!=2)
	{
		LogPrint(3, "[DETECT] Detect-Staff esta [%s]\n", (DetectStaff ? "on":"off"));
		LogPrint(3, "[DETECT] Uso: %s on/off\n", CI->Args[0]);
		return;
	}

	if(!stricmp(CI->Args[1], "on"))
	{
		if(DetectStaff)
		{
			LogPrint(3, "[DETECT] Detect staff ja esta on\n");
			return;
		}

		/* criar a thread */
		if((DetectStaffHandle = CreateThread(NULL, 4096, (LPTHREAD_START_ROUTINE)MyWatchForStaff, 0, 0, &DetectStaffThread)) == NULL)
		{
			LogPrint(3, "[DETECT] Nao foi possivel criar a thread\n");
			return;
		}

		DetectStaff = 1;

		LogPrint(3, "[DETECT] Detecting staffs on\n");
		LogPrint(3, "[DETECT] Se algum staff aparecer o uobot ira deslogar\n");

		return;
	}
	else
	{
		if(!DetectStaff)
		{
			LogPrint(3, "[DETECT] Detect-Staff ainda não esta ligado\n");
			return;
		}

		/* kilar processo */
		TerminateThread(DetectStaffHandle, 0);

		SeeStaffType = 0;

		DetectStaff = 0;
		LogPrint(3, "[DETECT] Detect-Staff off\n");
		return;
	}
}

void Command_DetectSpeech(CmdInfoTag *CI)
{
	if(CI->Argc!=2)
	{
		LogPrint(3, "[DETECT] Detect-Speech esta [%s]\n", (DetectSpeech ? "on":"off"));
		LogPrint(3, "[DETECT] Uso: %s on/off\n", CI->Args[0]);
		return;
	}

	if(!stricmp(CI->Args[1], "on"))
	{
		if(DetectSpeech)
		{
			LogPrint(3, "[DETECT] Detect speech ja esta on\n");
			return;
		}

		DetectSpeech = 1;

		LogPrint(3, "[DETECT] Detecting speech on\n");
		LogPrint(3, "[DETECT] Se alguem falar ingame algo perto de vc, ira acionar o anti-staff system\n");

		return;
	}
	else
	{
		if(!DetectSpeech)
		{
			LogPrint(3, "[DETECT] Detect-Spech ainda não esta ligado\n");
			return;
		}

		DetectSpeech = 0;
		LogPrint(3, "[DETECT] Detect-Speech off\n");
		return;
	}
}

void Command_Quit(CmdInfoTag *CI)
{
	LogPrint(3, "[QUIT] Fechando UOBOT\n");
	ExitProcess(1);
}

void Command_MassMove(CmdInfoTag *CI)
{
	unsigned int BagSerial, items;
	unsigned int BagDestino;

	if(CI->Argc!=2 && CI->Argc!=3)
	{
		LogPrint(3, "[MASSMOVE] Uso: %s bagorigem (bagdestino)\n", CI->Args[0]);
		return;
	}

	BagDestino = mbackpack;

	if(CI->Argc==3)
		BagDestino = ArgToInt(CI->Args[2]);

	BagSerial = ArgToInt(CI->Args[1]);

	items = MassMove(BagSerial, BagDestino);
	LogPrint(3, "[MASSMOVE] Movido %d items para bag 0x%X\n", items, BagDestino);
}

void InitCommands(void)
{
	AddCommand("loaddll", (void *)Command_LoadDll);
	AddCommand("listdlls", (void *)Command_ListDlls);
	AddCommand("listcmds", (void *)Command_ListCmds);
	AddCommand("listthreads", (void *)Command_ListThreads);
	AddCommand("killthread", (void *)Command_KillThread);
	AddCommand("allnames", (void *)Command_AllNames);
	AddCommand("unloaddll",(void *)Command_UnloadDll);
	AddCommand("listitens", (void *)Command_ListItens);
	AddCommand("listitensinlayer", (void *)Command_ListItensInLayer);
	AddCommand("seejournal", (void *)Command_SeeJournal);
	AddCommand("killall", (void *)Command_KillAllThreads);
	AddCommand("reqobjnames", (void *)Command_ReqObjNames);
	AddCommand("cleanjournal", (void *)Command_CleanJournal);
	AddCommand("clickobj", (void *)Command_ClickObj);
	AddCommand("disarm", (void *)Command_Disarm);
	AddCommand("setlogflag", (void *)Command_SetLogFlag);
	AddCommand("warmode", (void *)Command_Warmode);
	AddCommand("setdebug", (void *)Command_SetDebug);
	AddCommand("listskills", (void *)Command_ListSkills);
	AddCommand("findgraphic", (void *)Command_FindGraphic);
	AddCommand("reqallobjnames", (void *)Command_ReqAllObjNames);
	AddCommand("findname", (void *)Command_FindName);
	AddCommand("listallitens", (void *)Command_ListAllItens);
	AddCommand("viewmemo", (void *)Command_ViewMemoUse);
	AddCommand("setjornal", (void *)Command_SetJournal);
	AddCommand("teleto", (void *)Command_Teleto);
	AddCommand("listreags", (void *)Command_ListReags);
	AddCommand("findcontainer", (void *)Command_ContainerList);
	AddCommand("useobject", (void *)Command_UseObject);
	AddCommand("useobjecttype", (void *)Command_UseObjectType);
	AddCommand("waittarget", (void *)Command_WaitTarget);
	AddCommand("seeip", (void *)Command_SeeIp);
	AddCommand("setspeechcolor", (void *)Command_SetSpeechColor);
	AddCommand("resync", (void*)Command_Resync);
	AddCommand("reconnecttime", (void *)Command_ReconnectTime);
	AddCommand("status", (void *)Command_Status);
	AddCommand("walk", (void *)Command_Walk);
	AddCommand("stats", (void *)Command_Stats);
	AddCommand("seenews", (void *)Command_SeeNews);
	AddCommand("setaviso", (void *)Command_SetAviso);
	AddCommand("aceitatrade", (void *)Command_AceitarTrade);
	AddCommand("settimeout", (void *)Command_SetTimeout);
	AddCommand("setrespam", (void *)Command_SetRespAm);
	AddCommand("hidewindow", (void *)Command_HideWindow);
	AddCommand("shardip", (void *)Command_SetShardIp);
	AddCommand("m2acrypt", (void *)Command_M2aCrypt);
	AddCommand("antistaff", (void *)Command_AntiStaff);
	AddCommand("seestaff", (void *)Command_SeeStaff);
	AddCommand("cancelpage", (void *)Command_CancelPage);
	AddCommand("detectstaff", (void *)Command_DetectStaff);
	AddCommand("detectspeech", (void *)Command_DetectSpeech);
	AddCommand("quit", (void *)Command_Quit);
	AddCommand("massmove", (void *)Command_MassMove);
}
