#include <windows.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "pluginirc_access.h"

AccessListTag *AccessList = 0;
unsigned int AccessListCount = 0;

int Access_Search(char *Name)
{
	unsigned int x;

	for(x=0; x < AccessListCount; x++)
	{
		if(strstr(Name, AccessList[x].Name))
			return (int)x;
	}

	return -1;
}

int Access_Add(char *Name, unsigned char CmdLevel)
{
	if(Access_Search(Name) != -1)
		return 0;

	AccessListCount++;
	AccessList = realloc(AccessList, sizeof(AccessListTag) * AccessListCount);

	AccessList[AccessListCount - 1].CmdLevel = CmdLevel;
	strcpy(AccessList[AccessListCount - 1].Name, Name);

	return 1;
}

int Access_Del(char *Name, unsigned char Idx)
{
	unsigned int x, TmpAccessCount = 0;
	AccessListTag *TmpAccess = 0;

	if(Name == 0)
	{
		if(Idx > AccessListCount)
			return 0;
	}

	for(x=0; x < AccessListCount; x++)
	{
		if(Name == 0)
		{
			if(x == Idx)
				continue;
		}
		else
		{
			if(!strcmp(Name, AccessList[x].Name))
				continue;
		}

		TmpAccessCount++;
		TmpAccess = realloc(TmpAccess, sizeof(AccessListTag) * TmpAccessCount);

		TmpAccess[TmpAccessCount - 1].CmdLevel = AccessList[x].CmdLevel;
		strcpy(TmpAccess[TmpAccessCount - 1].Name, AccessList[x].Name);
	}
	
	if(AccessList != 0)
		free(AccessList);

	x = AccessList - TmpAccess;

	AccessList = TmpAccess;
	AccessListCount = TmpAccessCount;

	return (int)x;
}	