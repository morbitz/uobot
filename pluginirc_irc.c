#include <windows.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "pluginirc.h"
#include "pluginirc_net.h"


void Irc_Say(char *Where, char *Texto, ...)
{
	char final[2000]={0};
	va_list args;

	sprintf(final, "PRIVMSG %s :", Where);

	va_start(args, Texto);
	vsprintf( (&final[0] + strlen(final)) , Texto, args);
	va_end(args);

	strcat(final, "\n");

	net_send(final, strlen(final));
}

void Irc_Raw(char *Texto, ...)
{
	char final[2000]={0};
	va_list args;

	memset(final, 0, sizeof(final));

	va_start(args, Texto);
	vsprintf(final, Texto, args);
	va_end(args);
	strcat(final, "\n");

	net_send(final, strlen(final));
}

void Irc_Join(char *Chan)
{
	char *ChanText = malloc(strlen(Chan)+10);
	sprintf(ChanText, "JOIN %s\n", Chan);
	net_send(ChanText, strlen(ChanText));

	/* Setar o canal como Last Channel */
	strcpy(LastChan, Chan);

	free(ChanText);
}

void Irc_Part(char *Chan)
{
	char *ChanText = malloc(strlen(Chan)+10);
	sprintf(ChanText, "PART %s\n", Chan);
	net_send(ChanText, strlen(ChanText));
	free(ChanText);
}

