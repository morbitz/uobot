#include <windows.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "uobot_handles.h"
#include "uobot_net.h"
#include "uobot_log.h"
#include "uobot_obj.h"
#include "uobot_jornal.h"
#include "uobot_target.h"
#include "uobot_spells.h"
#include "uobot_threads.h"
#include "uobot.h"

JournalEntry Journal[JOURNAL_SIZE] = {0};
BOOL JournalReady = FALSE;
unsigned int JournalCount = 0;

void CleanJournal(void)
{
	int i = 0;
		
	for(i = 0; i < JOURNAL_SIZE; i++)
	{
		if(Journal[i].Text)
			free(Journal[i].Text);
		Journal[i].Text = NULL;
		Journal[i].Type = JOURNAL_NONE;
	}

	JournalCount = 0;
	JournalReady = TRUE;

	/* if the player is in the world say the log was cleared */
//	if(mserial != INVALID_SERIAL)
//		LogPrint(3, "[JORNAL] Jornal Clear\n");

	return;
}

void JournalAdd(char *Speaker, char *Text, int Type)
{ 
	int Len = 0;

	if(JournalReady == FALSE)
		return;

	while(JournalCount >= JOURNAL_SIZE - 1)
		JournalRemove(0); /* delete the oldest entry */

	if(Journal[JournalCount].Text)
	{
		free(Journal[JournalCount].Text);
		Journal[JournalCount].Text = NULL;
	}

	/* format is: "Speaker: Text" */
	Len = strlen(Text);
	if(Speaker) Len += strlen(Speaker);

	/* allocate text + space for ": " and the null char */
	Journal[JournalCount].Text = (char*)malloc(Len + 3);

	/* copy the text */
	if(Speaker) sprintf(Journal[JournalCount].Text, "%s: %s", Speaker, Text);
	else strncpy(Journal[JournalCount].Text, Text, Len);
	/* NULL on end of allocated area.. just for safety :) */
	Journal[JournalCount].Text[Len+2] = 0;
	Journal[JournalCount].Type = Type;
	JournalCount++;

	/* good for debugging but it spams players like hell ;) 
	if(mserial != INVALID_SERIAL)
		LogPrint(3, "[JORNAL] Journal lines: %d, added: %s\n", JournalCount, GetJournalLine(JournalCount-1)); */

	return;
}

void JournalRemove(unsigned int Line)
{ 
	if(JournalReady == FALSE)
		return;

	if(!JournalCount)
		return;

	if(Journal[Line].Text != NULL)
		free(Journal[Line].Text);
    
	memmove(Journal + Line, Journal + Line + 1, sizeof(JournalEntry) * (JournalCount - Line - 1));

	JournalCount--;
	Journal[JournalCount].Text = NULL;
	Journal[JournalCount].Type = JOURNAL_NONE;

	return;
}

char* GetJournalLine(unsigned int Line)
{
    if(JournalReady == FALSE)
		return NULL;

    if(Line >= JournalCount)
		return NULL; 
  
    return Journal[Line].Text; 
}

char* JournalGetLast(void)
{
	if(!JournalCount && !strcmp(GetJournalLine(JournalCount), ""))
		return NULL;
	else
		return GetJournalLine(JournalCount);
} 

void JournalDump(char *Filename)
{ 
	FILE *d = NULL;
	char DumpPath[4096];
	unsigned int i = 0;

	if(Filename == NULL)
		return;

	strcat(DumpPath, Filename);

	d = fopen(DumpPath, "at");

	if(!d)
		LogPrint(3,"[ERROR] Couldnt dump to %s", DumpPath);

	for(i = 0; i < JournalCount; i++) 
		fprintf(d, "\n%s", GetJournalLine(i));

	fclose(d);
	return;
}

void SetJournalLine(unsigned int Line, char *Replace)
{ 
	int Len = 0;

	if(JournalReady == FALSE)
		return;

	if(Line >= JOURNAL_SIZE)
		return;

	if(Replace == NULL)
		return;

	/* if its long its probably bad, and 512 is so nice number */ 
	Len = strlen(Replace);
	if(Len > 512 ) return; 

	if(Journal[Line].Text)
		free(Journal[Line].Text);

	Journal[Line].Text = (char *)malloc(sizeof(char)*(Len + 1)); 
	strncpy(Journal[Line].Text, Replace, Len + 1);
	/* in case it wasnt NULL terminated string */
	Journal[Line].Text[Len] = 0;

	return;
} 

int IsInJournal(char *Text, int Type)
{ 
	unsigned int i = 0;

	if(Text == NULL)
		return -1;

	for(i = 0; i < JournalCount; i++)
	{
		/* if we're looking for certain message type */
		if(Type != JOURNAL_ANY && Type != Journal[i].Type)
			continue;
		
		if(strstr(Journal[i].Text, Text) != NULL)
			return i;
	}

	return -1;
} 
