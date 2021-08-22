#include <windows.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdio.h>
#include "pluginirc_users.h"
#include "pluginirc_access.h"
#include "pluginirc.h"

ChanListTag *ChanList = 0;
unsigned int ChanListCount = 0;

char *StrTok(char *Host)
{
	unsigned int x = 0;
	if(Host[0] == ':')
		Host++;
	while(Host[x] !=0 && Host[x] !='!')
		x++;
	Host[x] = 0;

	return Host;
}

ChanListTag *Chan_Search(char *ChanName)
{
	unsigned int x;
	for(x = 0; x < ChanListCount; x++)
	{
		if(!stricmp(ChanList[x].ChanName, ChanName))
			return &ChanList[x];
	}

	return 0;
}

ChanListTag *Chan_Create(char *ChanName)
{
	ChanListTag *Chan;

	if((Chan = Chan_Search(ChanName)) != 0)
		return Chan;

	ChanListCount++;
	ChanList = realloc(ChanList, sizeof(ChanListTag) * ChanListCount);
	strcpy(ChanList[ChanListCount - 1].ChanName, ChanName);
	ChanList[ChanListCount - 1].UserList = 0;
	ChanList[ChanListCount - 1].UserListCount = 0;

	return (ChanListTag *)&ChanList[ChanListCount - 1];
}

int Chan_Del(char *ChanName)
{
	ChanListTag *TmpChan = 0;
	ChanListTag *Chan = 0;
	unsigned int TmpChanCount = 0;

	unsigned int x;


	if((Chan = Chan_Search(ChanName)) == 0)
		return -1;
	
	for(x = 0; x < ChanListCount; x++)
	{
		if(!stricmp(ChanList[x].ChanName, ChanName))
		{
			if(ChanList[x].UserList != 0)
				free(ChanList[x].UserList);
			ChanList[x].UserList = 0;

			continue;
		}

		TmpChanCount++;
		TmpChan = realloc(TmpChan, sizeof(ChanListTag) * TmpChanCount);

		strcpy(TmpChan[TmpChanCount - 1].ChanName, ChanList[x].ChanName);
		TmpChan[TmpChanCount - 1].UserList = ChanList[x].UserList;
		TmpChan[TmpChanCount - 1].UserListCount = ChanList[x].UserListCount;
	}

	ChanListCount = TmpChanCount;
	free(ChanList);
	ChanList = TmpChan;

	return 1;
}


UserListTag *User_Search(char *ChanName, char *NickName)
{
	unsigned int x;
	ChanListTag *Chan = 0;

	if((Chan = Chan_Search(ChanName)) == 0)
		return 0;

	for(x=0; x < Chan->UserListCount; x++)
	{
		if(!stricmp(Chan->UserList[x].NickName, NickName))
		{
			return &Chan->UserList[x];
		}
	}

	return 0;
}

UserListTag *User_Add(char *ChanName, char *NickName, unsigned char NickFlag)
{
	ChanListTag *Chan = 0;
	UserListTag *User = 0;

	Chan = Chan_Create(ChanName);

	if((User = User_Search(ChanName, NickName)) != 0)
	{
		User->NickFlag = NickFlag;
		return User;
	}

	Chan->UserListCount++;
	Chan->UserList = realloc(Chan->UserList, Chan->UserListCount * sizeof(UserListTag));

	strcpy(Chan->UserList[Chan->UserListCount - 1].NickName, NickName);
	Chan->UserList[Chan->UserListCount - 1].NickFlag = NickFlag;

	return &Chan->UserList[Chan->UserListCount - 1];
}

int User_Del(char *NickName, char *ChanName)
{
	unsigned int x, TmpUserCount = 0;
	UserListTag *TmpUser = 0;
	ChanListTag *Chan = 0;

	/* quit */
	if(ChanName == 0)
	{
		for(x=0; x < ChanListCount; x++)
		{
			if(User_Search(ChanList[x].ChanName, NickName) != 0)
				User_Del(NickName, ChanList[x].ChanName);
		}

		return 1;
	}

	if((Chan = Chan_Search(ChanName)) == 0)
		return -1;

	if(User_Search(ChanName, NickName) == 0)
		return -1;

	for(x=0; x < Chan->UserListCount; x++)
	{
		if(!stricmp(Chan->UserList[x].NickName, NickName))
		{
			continue;
		}
		
		TmpUserCount++;
		TmpUser = realloc(TmpUser, sizeof(UserListTag) * TmpUserCount);

		strcpy(TmpUser[TmpUserCount - 1].NickName, Chan->UserList[x].NickName);
		TmpUser[TmpUserCount - 1].NickFlag = Chan->UserList[x].NickFlag;

	}

	Chan->UserListCount = TmpUserCount;

	free(Chan->UserList);
	Chan->UserList = TmpUser;

	return 1;
}

int User_IsVoice(char *ChanName, char *NickName)
{
	UserListTag *User = 0;

	if(Access_Search(NickName) != -1)
		return 1;

	if((User=User_Search(ChanName, NickName)) == 0)
		return 0;

	if(User->NickFlag&1)
		return 1;

	return 0;
}

int User_IsOp(char *ChanName, char *NickName)
{
	UserListTag *User = 0;

	if(Access_Search(NickName) != -1)
		return 1;

	if((User=User_Search(ChanName, NickName)) == 0)
		return 0;

	if(User->NickFlag&2)
		return 1;

	return 0;
}

void User_ChangeNick(char *Old, char *New)
{
	UserListTag *User = 0;
	unsigned int x;

	for(x=0; x < ChanListCount; x++)
	{
		if((User = User_Search(ChanList[x].ChanName, Old)) != 0)
			strcpy(User->NickName, New);
	}

	return;
}


unsigned char User_GetFlag(char *ChanName, char *NickName)
{
	UserListTag *User = User_Search(ChanName, NickName);

	if(User == 0)
		User = User_Add(ChanName, NickName, 0);
	
	return User->NickFlag;
}

int User_ChangeFlag(char *ChanName, char *Flags, char *Nicks)
{
	char *TmpFlags = Flags;
	char *TmpNicks = Nicks;

	Nicks = strtok(Nicks, " ");
	//IF->LogPrint(3, "[IRC] c:[%s] f:[%s] n:[%s]\n", ChanName, Flags, Nicks);

	while(*TmpFlags!=0)
	{
		if(*TmpFlags== '+')
		{
			TmpFlags++;
			while(*TmpFlags!=0 && *TmpFlags != '-')
			{
				if(*TmpFlags=='o')
				{
					unsigned char flag;
					flag = User_GetFlag(ChanName, Nicks);
					flag |= 2;
					IF->LogPrint(1, "[IRC] Nick %s ganhou op no [%s]\n", Nicks, ChanName);
					User_Add(ChanName, Nicks, flag);
					Nicks = strtok(NULL, " ");
				}
				else if(*TmpFlags=='v')
				{
					unsigned char flag;
					flag = User_GetFlag(ChanName, Nicks);
					flag |= 1;
					IF->LogPrint(1, "[IRC] Nick %s ganhou voice no [%s]\n", Nicks, ChanName);
					User_Add(ChanName, Nicks, flag);
					Nicks = strtok(NULL, " ");
				}
				TmpFlags++;
			}
		}
		if(*TmpFlags== '-')
		{
			TmpFlags++;
			while(*TmpFlags!=0 && *TmpFlags != '+')
			{
				if(*TmpFlags=='o')
				{
					unsigned char flag;
					flag = User_GetFlag(ChanName, Nicks);
					flag &= 1;
					IF->LogPrint(1, "[IRC] Nick %s perdeu op no [%s]\n", Nicks, ChanName);
					User_Add(ChanName, Nicks, flag);
					Nicks = strtok(NULL, " ");
				}
				else if(*TmpFlags=='v')
				{
					unsigned char flag;
					flag = User_GetFlag(ChanName, Nicks);
					flag &= 2;
					IF->LogPrint(1, "[IRC] Nick %s perdeu voice no [%s]\n", Nicks, ChanName);
					User_Add(ChanName, Nicks, flag);
					Nicks = strtok(NULL, " ");
				}
				TmpFlags++;
			}
		}
	}
					
	return 1;
}

