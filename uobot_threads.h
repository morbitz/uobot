
struct ThreadInfo
{
	unsigned char *buf;
	int len;
	int SleepTime;
	int (*send_server)(char *, int);
	void (*LogPrint)(int, const char *, ...);
};


int Thread_SendServer(char *buf, int len, int SleepTime);


typedef struct 
{
	char **Args;
	int Argc;
} CmdInfoTag;

typedef struct 
{
	int CmdIdx;
	HANDLE hHandle;
	CmdInfoTag *Info;
	int IsSuspended;
} CmdThreadsTag;

CmdThreadsTag *CmdThreads;
unsigned int CmdThreadsCount;


CmdInfoTag *GetThreadCI(HANDLE hHandle);
void CmdListThreads(void);
void ResumeAllPThreads(void);
unsigned int SuspendAllPThreads(void);
int MakeCmdThread(int Idx, char **Args, int Argc);
int CmdThreadExit(CmdInfoTag *CI);
int CmdThreadKill(CmdInfoTag *CI);
int GetIdxFromInfo(CmdInfoTag *CI);
unsigned int CmdKillAll(void);
unsigned int RemakeCmds(void);

