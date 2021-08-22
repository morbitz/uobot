typedef struct
{
	char Name[30];
	unsigned char CmdLevel;
} AccessListTag;

AccessListTag *AccessList;
unsigned int AccessListCount;

int Access_Search(char *Name);
int Access_Add(char *Name, unsigned char CmdLevel);
int Access_Del(char *Name, unsigned char Idx);
