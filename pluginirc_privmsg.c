#include <windows.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdio.h>
#include "pluginirc_users.h"
#include "pluginirc.h"
#include "pluginirc_skills.h"
#include "pluginirc_stuff.h"
#include "pluginirc_privmsg.h"
#include "pluginirc_irc.h"
#include "pluginirc_access.h"

void Process_PRIVMSG(char *ChanName, char *NickName, char *Texto)
{
	char Msgs[10][100] = {0};
	memset(Msgs, 0, sizeof(Msgs));


	sscanf(Texto, "%99s %99s %99s %99s %99s", Msgs[0], Msgs[1], Msgs[2], Msgs[3], Msgs[4]);

	if(!stricmp(Msgs[0], ".walk") && User_IsOp(ChanName, NickName)) 
	{
		IF->Walk(Msgs[1]);
		return;
	}

	if(!stricmp(Msgs[0], ".do") && User_IsOp(ChanName, NickName)) 
	{
		if(Msgs[1][0] != 0)
		{
			Texto[3] = '/';
			IF->HandleCmd(Texto+3);
			Irc_Say(ChanName, "Cmd [%s] done.", Texto+3);
		}
		else
			Irc_Say(ChanName, "Uso: .do cmd");
		return;
	}

	if(!stricmp(Msgs[0], ".raw") && User_IsOp(ChanName, NickName))
	{
		char *Resto = Texto;
		
		if(Resto[4] == 0)
			return;

		Resto = Texto+5;
		Irc_Raw("%s", Resto);
		return;
	}

	/* .online */
	if(!stricmp(Msgs[0], ".online") && !WS)
	{
		if(time(0) - LastOnlineReq > 20)
		{
			IF->Say(".online");
			LastOnlineReq = time(0);
		}
		return;
	}

	/* allnames */
	else if(!stricmp(Msgs[0], ".allnames") && User_IsOp(ChanName, NickName))
	{
		SeeAllPlayer(ChanName);
		return;
	}

	/* .set */
	else if(!stricmp(Msgs[0], ".set") && User_IsOp(ChanName, NickName))
	{
		if(!stricmp(Msgs[1], "verskillup"))
		{
			if(!strcmp(Msgs[2], "on"))
				SeeSkillUp = 1;
			else
				SeeSkillUp = 0;
			Irc_Say(ChanName, "verskillup: [%d]\n", SeeSkillUp);
		}

		else if(!strcmp(Msgs[1], "vermsgs"))
		{
			if(!strcmp(Msgs[2], "on"))
				SpeechIrc = 1;
			else
				SpeechIrc = 0;
			Irc_Say(ChanName, "vermsgs: [%d]\n", SpeechIrc);
		}
		return;
	}

	/* .sayingame */
	else if(!strcmp(Msgs[0], ".sayingame") && User_IsOp(ChanName, NickName))
	{
		char *Resto = Texto;

		if(Resto[10] == 0)
			return;
		
		Resto += 11;
		IF->Say(Resto);
		return;
	}
	else if(!strcmp(Msgs[0], ".reqskills") && User_IsOp(ChanName, NickName))
	{
		unsigned char buf[10] = { 0x34, 0xed, 0xed, 0xed, 0xed, 0x05, 0x00, 0x00, 0x00, 0x00};
		IF->pack32(buf+6, *IF->mserial);
		IF->send_server(buf, 10);
		Irc_Say(MainChan, "Requisitando skills");
	}

	/* .listskills */
	else if(!strcmp(Msgs[0], ".listskills") && User_IsOp(ChanName, NickName))
	{
		unsigned int x, count=0;

		for(x=0;x<50;x++)
		{
			if(SkillsName[x].LastValue != 0)
			{
				Irc_Say(ChanName, "[%s] -> [%.1f]", SkillsName[x].Name, (double)SkillsName[x].LastValue/10);
				count++;
			}
		}

		Irc_Say(ChanName, "Listado %d skills", count);

		return;
	}

	/* say */
	else if(!strcmp(Msgs[0], ".say") && User_IsOp(ChanName, NickName))
	{
		char *Resto = Texto;

		if(Resto[4] == 0)
			return;

		Resto += 5;
		Irc_Say(ChanName, Resto);
		return;
	}

	/* say .access */
	else if(!stricmp(Msgs[0], ".access") && User_IsOp(ChanName, NickName))
	{
		if(Msgs[1][0] == 0)
		{
			Irc_Say(ChanName, "Uso: .access add/del/list");
			return;
		}
	
		if(!strcmp(Msgs[1], "add"))
		{
			if(Msgs[2][0] == 0)
			{
				Irc_Say(ChanName, "Uso: .access add <nick>");
				return;
			}

			if(Access_Add(Msgs[2], 1))
				Irc_Say(ChanName, "Adicionado user %s a lista", Msgs[2]);
			else
				Irc_Say(ChanName, "Não foi possivel adicionar user %s na lista", Msgs[2]);
		}

		/* .access del */
		else if(!strcmp(Msgs[1], "del") && User_IsOp(ChanName, NickName))
		{
			if(Msgs[2][0] == 0)
			{
				Irc_Say(ChanName, "Uso: .access del <nick>");
				return;
			}
			if(Access_Del(Msgs[2], 0))
				Irc_Say(ChanName, "Deletado user %s", Msgs[2]);
			else
				Irc_Say(ChanName, "Nao foi possivel deletar user %s da lista", Msgs[5]);
		}

		/* access list */
		else if(!strcmp(Msgs[1], "list") && User_IsOp(ChanName, NickName))
		{
			unsigned int x;
			for(x=0; x < AccessListCount; x++)
				Irc_Say(ChanName, "%d: %s", x, AccessList[x].Name);
			Irc_Say(ChanName, "Listados %d users", x);
		}
		else
			Irc_Say(ChanName, "Uso .access add/del/list");
	
		return;
	}
}

