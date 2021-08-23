#define G_BP 0xf7a
#define G_BM 0xf7b
#define G_GA 0xf84
#define G_GS 0xf85
#define G_MR 0xf86
#define G_NS 0xf88
#define G_SA 0xf8c
#define G_SS 0xf8d

typedef struct {
	char *Name;
	unsigned short Graphic;
	unsigned short Qtd;
} ReagTag;

unsigned short m_speechcolor;

unsigned int ReconnectTime;
unsigned int LoginTime;
unsigned int PingTimeout;

int SeeStaffType;
int CancelPage;
int DetectStaff;
HANDLE DetectStaffHandle;
DWORD DetectStaffThread;

void Command_KillThread(CmdInfoTag *CI);
void Command_LoadDll(CmdInfoTag *CI);
void Command_ListDlls(CmdInfoTag *CI);
void Command_ListCmds(CmdInfoTag *CI);
void Command_ListThreads(CmdInfoTag *CI);
void Command_UnloadDll(CmdInfoTag *CI);
void Command_AllNames(CmdInfoTag *CI);
void Command_ListItens(CmdInfoTag *CI);

void InitCommands(void);

char AvisoFile[50];

int AceitarCoisas;
int LastAm;
int RespAm;
NOTIFYICONDATA Notify;
int InsertM2aCrypt;
int AntiStaff;
int ErrNum;
int DetectSpeech;