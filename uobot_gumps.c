#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include "uobot_log.h"
#include "uobot_gumps.h"
#include "uobot_net.h"
#include "uobot_obj.h"

GumpDoneType *GumpDone = 0;

int WaitGump = 0;
int GumpSent = 0;

/* Handle do pacote 0xb1 */
int uo_handle_gump(unsigned char *buf, int len)
{
	GumpType gmr;
	LineType *Lines;

	char *Line=0;
	unsigned int x=0, LineSize=0;

	char *p1 = 0;
	unsigned int PageNow = 0;

	char Tipo[20]={0};

	ButtonType *Buttons = 0;
	unsigned int ButtonCount = 0;

	TextType *Texts = 0;
	unsigned int TextCount = 0;

	GumppicType *Gumppics = 0;
	unsigned int GumppicCount = 0;

	if(GumpDone) return 0;
	
	gmr.cmd = buf[0]&0xff;
	gmr.dtlen = unpack16(buf+1);
	gmr.serial = unpack32(buf+3);
	gmr.tipo = unpack32(buf+7);
	gmr.unk1 = unpack32(buf+11);
	gmr.unk2 = unpack32(buf+15);
	gmr.laylen = unpack16(buf+19);
	gmr.lay = (char *)buf+21;
	gmr.nlines = unpack16(buf+21+gmr.laylen);
	gmr.linelen = unpack16(buf+21+gmr.laylen+2);
	gmr.lines = (char *)buf+21+gmr.laylen+2;

	if(gmr.nlines == 0) return 0;

	Line = gmr.lines;

	/* Alocar espaco para gmr.nlines structs */
	Lines = malloc(sizeof(LineType)*gmr.nlines);

	/* Reinterpretar todas as linhas, transformando em ascii */
	for(x=0; x < (unsigned int)gmr.nlines; x++)
	{
		/* Tamanho da linha unicode */
		LineSize = unpack16(Line);
		/* Alocar espaco para o texto em ascii */
		Lines[x].Text = malloc(LineSize+1);
		/* Transformar o texto em ascii e guarda na struct */
		UnicodeToAscii(Line+2, LineSize*2, Lines[x].Text);
		/* Proxima linha */
		Line += 2+(LineSize*2);
	}

	gmr.lay[gmr.laylen] = 0;

	if(!WaitGump)
	{
		LogPrint(3, "[GUMP] S:[%#x] Id:[%#x]\n", gmr.serial, gmr.tipo);
		LogPrint(3, "[GUMP] NLinhas:[%d] LayLen:[%d]\n", gmr.nlines, gmr.laylen);
		LogPrint(3, "[GUMP] Lay:[%s]\n", gmr.lay);
	
		LogPrint(3, "[GUMP] Mostrando linhas\n");
		for(x=0; x < (unsigned int)gmr.nlines; x++)
			LogPrint(3, "%d: [%s]\n", x, Lines[x].Text);
		LogPrint(3, "[GUMP] [%d] Linhas mostradas\n", x);

		for(x=0; x < (unsigned int)gmr.nlines; x++)
			free(Lines[x].Text);

		free(Lines);

		return 0;
	}

	/* Handle do lay */

	p1 = gmr.lay;

	while(*p1 != 0)
	{
		while(*p1++ != '{');

		sscanf(p1, "%20s", Tipo);


		/* Button [x] [y] [released-id] [pressed-id] [quit] [page-id] [return-value] */
		if(!stricmp(Tipo, "button"))
		{
			ButtonCount++;
			Buttons = realloc(Buttons, sizeof(ButtonType)*ButtonCount);
			Buttons[ButtonCount - 1].Id = 0;
			sscanf(p1, "%*s %d %d %*d %*d %*d %*d %d",
				&Buttons[ButtonCount - 1].x, &Buttons[ButtonCount - 1].y, &Buttons[ButtonCount - 1].Id);
			Buttons[ButtonCount - 1].Page = PageNow;
		}

		/* Text [x] [y] [color] [text-id] */
		else if(!stricmp(Tipo, "text"))
		{
			TextCount++;
			Texts = realloc(Texts, sizeof(TextType)*TextCount);
			Texts[TextCount - 1].Id = 0;
			sscanf(p1, "%*s %d %d %*d %d",
				&Texts[TextCount - 1].x, &Texts[TextCount - 1].y, &Texts[TextCount - 1].Id);
			Texts[TextCount - 1].Page = PageNow;
		}

		/* GumpPic [x] [y] [id] */
		else if(!stricmp(Tipo, "gumppic"))
		{
			GumppicCount++;
			Gumppics = realloc(Gumppics, sizeof(GumppicType)*GumppicCount);
			Gumppics[GumppicCount - 1].Id = 0;
			sscanf(p1, "%*s %d %d %d",
				&Gumppics[GumppicCount - 1].x, &Gumppics[GumppicCount - 1].y, &Gumppics[GumppicCount - 1].Id);
			Gumppics[GumppicCount - 1].Page = PageNow;
		}

		/* Page [#] */
		else if(!stricmp(Tipo, "page"))
			sscanf(p1, "%*s %d", &PageNow);

		while(*p1++ != '}');
	}

	GumpDone = malloc(sizeof(GumpDoneType));
	
	GumpDone->Buttons = Buttons;
	GumpDone->ButtonCount = ButtonCount;
	GumpDone->Texts = Texts;
	GumpDone->TextCount = TextCount;
	GumpDone->Lines = Lines;
	GumpDone->LineCount = gmr.nlines;
	GumpDone->Gumppics = Gumppics;
	GumpDone->GumppicCount = GumppicCount;

	GumpDone->Serial = gmr.serial;
	GumpDone->Id = gmr.tipo;

	GumpSent = 1;

	return 0;
}


/* Libera todo o espaco alocado pelo gump */
void FreeGump(void)
{
	unsigned int x=0;

	if(GumpDone==0) return;

	free(GumpDone->Buttons);
	free(GumpDone->Texts);
	free(GumpDone->Gumppics);

	for(x=0; x < (unsigned int)GumpDone->LineCount; x++)
		free(GumpDone->Lines[x].Text);
	free(GumpDone->Lines);

	free(GumpDone);

	GumpDone = 0;

	WaitGump = 0;
}

/* Procura pelo x,y dentro de uma pagina e retorna o id */
unsigned int GumpSearchXY(unsigned int x, unsigned int y, unsigned int Page)
{
	unsigned int i = 0;

	if(GumpDone==0) return 0;

	for(i=0; i < GumpDone->ButtonCount; i++)
	{
		if(GumpDone->Buttons[i].x == x &&
			GumpDone->Buttons[i].y == y &&
			GumpDone->Buttons[i].Page == Page)
		{
			return GumpDone->Buttons[i].Id;
		}
	}

	return 0;
}

/* Procura por um texto dentro das linhas e retorna o x,y,page do Text  */
unsigned int GumpSearchText(char *Texto, unsigned int *X, unsigned int *Y, unsigned int *Page)
{
	unsigned int x=0,i=0;

	if(GumpDone==0) return 0;

	for(x=0; x < GumpDone->LineCount; x++)
	{
		if(!stricmp(GumpDone->Lines[x].Text, Texto))
		{
			for(i=0; i < GumpDone->TextCount; i++)
			{
				if(GumpDone->Texts[i].Id == x)
				{
					*X = GumpDone->Texts[i].x;
					*Y = GumpDone->Texts[i].y;
					*Page = GumpDone->Texts[i].Page;
					return 1;
				}
			}
		}
	}
	return 0;
}


/* Wait gump */
void Wait_Gump(void)
{
	if(GumpDone!=0)
	{
		LogPrint(2, "[WAITGUMP] Voce precisa primeiro usar FreeGump\n");
		return;
	}
	WaitGump = 1;
}


/* Send Gump Choice */
void SendGumpChoice(unsigned int ButtonId)
{
	unsigned char buf[0x17]={0};
	buf[0] = 0xb1;
	pack16(buf+1, 0x17);
	pack32(buf+3, GumpDone->Serial);
	pack32(buf+7, GumpDone->Id);
	pack32(buf+11, ButtonId);
	send_server(buf, 0x17);
}
