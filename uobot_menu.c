#include <windows.h>
#include <string.h>
#include <stdio.h>
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
#include "uobot_commands.h"
#include "uobot_windows.h"
#include "uobot_teleto.h"
#include "uobot_menu.h"
#include "uobot.h"


MenuOptionsTag MenuOptions = {0};

int WMenu=0;

unsigned int t_id=0;
int WDlg=0;


int uo_handle_textdlg(char *buf, int len)
{
	t_id = unpack32(buf + 3);
	WDlg = 1;
	return 0;
}

void Send_Reply(char *Text)
{
	unsigned char buf[100]={0};
	buf[0] = 0xac;
	buf[2] = strlen(Text)+1+12;
	pack32(buf+3, t_id);
	buf[7] = 0;
	buf[8] = 0;
	buf[9] = 1;
	buf[10] = 0;
	buf[11] = strlen(Text)+1;
	memcpy(buf+12, Text, strlen(Text)+1);
	send_server(buf, 12+buf[11]);
}

int uo_handle_menu(char *buf, int len)
{
	unsigned int perglen=buf[9]&0xff;
	unsigned int x=0;

	/* Id e GumpId */
	MenuOptions.m_id = unpack32(buf + 3);
	MenuOptions.m_gump = unpack16(buf + 7);

	/* Pergunta */
	MenuOptions.m_question = malloc(buf[9]+1);
	memcpy(MenuOptions.m_question, buf+10, buf[9]);
	MenuOptions.m_question[buf[9]] = 0;

	buf += 10+perglen;
	MenuOptions.m_num_choices = buf[0];
	buf++;
	//LogPrint(3, "[CHOICES] %d choices\n", MenuOptions.m_num_choices);
	MenuOptions.m_options = malloc(sizeof(MenuOption)*MenuOptions.m_num_choices);
	for(x=0; x < MenuOptions.m_num_choices; x++)
	{
		MenuOptions.m_options[x].Graphic = unpack16(buf);
		strncpy(MenuOptions.m_options[x].Desc, buf+5, buf[4]);
		MenuOptions.m_options[x].Desc[buf[4]] = 0;
		//LogPrint(3, "[CHOICES] Choice[%d] = [%#X] [%s]\n",
			//x, MenuOptions.m_options[x].Graphic, MenuOptions.m_options[x].Desc);
		perglen = buf[4];
		buf += 5+perglen;
	}
	WMenu = 1;
	return 0;
}


int Send_Choice(char *ChoiceText)
{
    unsigned char buf[13];
	unsigned int x=0;
	short index=-1;

    buf[0] = 0x7D;
    pack32(buf + 1, MenuOptions.m_id);
    pack16(buf + 5, MenuOptions.m_gump);
	if(ChoiceText)
	{
		for(x=0; x < MenuOptions.m_num_choices; x++)
		{
			if(strstr(MenuOptions.m_options[x].Desc, ChoiceText)!=0)
			{
				index = x;
			}
		}
	}
	
    pack16(buf + 7, (unsigned short)(index + 1));
    if(index == -1)
        pack16(buf + 9, 0);
    else
        pack16(buf + 9, MenuOptions.m_options[index].Graphic);
    pack16(buf + 11, 0);
	free(MenuOptions.m_options);
	free(MenuOptions.m_question);
	MenuOptions.m_question = 0;
	MenuOptions.m_options = 0;
    send_server(buf, sizeof(buf));
	return index;
}


