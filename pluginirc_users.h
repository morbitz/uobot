typedef struct
{
	char NickName[30];
	unsigned char NickFlag;
} UserListTag;

typedef struct
{
	char ChanName[30];
	UserListTag *UserList;
	unsigned int UserListCount;
} ChanListTag;

ChanListTag *ChanList;
unsigned int ChanListCount;

ChanListTag *Chan_Search(char *ChanName);
ChanListTag *Chan_Create(char *ChanName);
int Chan_Del(char *ChanName);

UserListTag *User_Search(char *ChanName, char *NickName);
UserListTag *User_Add(char *ChanName, char *NickName, unsigned char NickFlag);
int User_Del(char *NickName, char *ChanName);


char *StrTok(char *Host);
int User_ChangeFlag(char *ChanName, char *Flags, char *Nicks);

int User_IsOp(char *ChanName, char *NickName);
int User_IsVoice(char *ChanName, char *NickName);

void User_ChangeNick(char *Old, char *New);