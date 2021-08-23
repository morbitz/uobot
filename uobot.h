
struct ExpFunc {
	/* Send Server */
	int (*send_server)(char *buf, int len);
	/* LogDump/LogPrint */
	void (*LogDump)(unsigned char *pBuffer, int length);
	void (*LogPrint)(int Type, const char *strFormat, ...);
	void (*MBOut)(const char *title, const char *msg, ...);
	/* Container Func */
	int (*CountItemInContainer)(unsigned short Graphic, unsigned int Serial);
	unsigned int (*FindItemInContainer)(unsigned short Graphic, unsigned short Color, unsigned int ContainerSerial);
	GameObject *(*FindSerial)(unsigned int serial);
	unsigned int (*GetItemInLayer)(unsigned int ContainerSerial, int Layer);
	/*  Stuff */
	void (*pack16)(unsigned char *Buf, unsigned short x);
	void (*pack32)(unsigned char *Buf, unsigned int x);
	int (*UnicodeToAscii)(const char *UnicodeText, int Len, char *AsciiText);
	unsigned short (*unpack16)(const unsigned char *Buf);
	unsigned int (*unpack32)(const unsigned char *Buf);
	/*  Jornal */
	char*(*GetJournalLine)(unsigned int Line);
	char*(*JournalGetLast)(void);
	void (*SetJournalLine)(unsigned int Line, char *Replace);
	int (*IsInJournal)(char *Text, int Type);
	void (*CleanJournal)(void);
	/* Say */
	void (*Say)(char *Texto, ...);
	/* Targeting */
	void (*Target_WaitTarget)(unsigned int Serial);
	void (*Target_WaitTargetGraphic)(unsigned short Graphic);
	int *TargetSent;
	/* Gaphic Effect */
	void (*Effect_WaitGraphEffect)(unsigned int FromSerial, unsigned int ToSerial, unsigned short Type);
	int *GraphicEffectSent;
	/* Commands & Objects */
	void (*UseObject)(unsigned int Serial);
	void (*ClickObject)(unsigned int Serial);
	int (*GetDistance)(int Sx, int Sy, int Tx, int Ty);
	void (*MoveToContainer)(unsigned int ItemSerial, unsigned short Quantity, unsigned int ContainerSerial);
	void (*PickupItem)(unsigned int ItemSerial, unsigned short Quantity);
	void (*DropItem)(unsigned int ItemSerial, unsigned short X, unsigned short Y, int Z);
	int (*EquipItem)(unsigned int ItemSerial, unsigned short Quantity, int Layer);
	void (*UnequipItem)(int Layer);
	int (*EquipItemType)(unsigned short Graphic, unsigned short Quantity, int Layer);
	unsigned int (*UseObjectType)(unsigned short Graphic);
	int (*Life)(unsigned int Hit, unsigned int MaxHit);
	int (*IsItemInLayer)(int Layer);
	unsigned long (*ArgToInt)(char *Arg);
	/* Actions */
	void (*CastSpell)(char *SpellName);
	void (*UseSkill)(char *SkillName);
	/* Attack */
	void (*Attack)(unsigned int serial);
	void (*DropHere)(unsigned int Serial);
	int (*IsDead)(unsigned int Serial);
	/* Socket */
	int (*CloseShardSocket)(void);
	/* Packet Handler */
	int (*AddPacketHandler)(int PacketID, void *Handler);
	/*  AntiMacro */
	int *AntiMacro;
	/*  Variaveis */
	unsigned int *mbackpack;
	unsigned int *mserial;
	/* Plugin AddCommand */
	int (*CmdThreadExit)(CmdInfoTag *CI);
	int (*PluginAddCommand)(const char *Name, void *Handler, HMODULE hModule);
	/* Skills */
	unsigned int (*ReqSkillValue)(char *SkillName);
	/* Warmode */
	void (*WarMode)(unsigned int Flag);
	/* Menu & Diaglog */
	int *WDlg;
	void (*Send_Reply)(char *Text);
	int *WMenu;
	int (*Send_Choice)(char *ChoiceText);
	void (*HookConsoleTo)(void *HF);
	/* Gump */
	void (*Wait_Gump)(void);
	unsigned int (*GumpSearchText)(char *Texto, unsigned int *X, unsigned int *Y, unsigned int *Page);
	unsigned int (*GumpSearchXY)(unsigned int x, unsigned int y, unsigned int Page);
	void (*FreeGump)(void);
	void (*SendGumpChoice)(unsigned int ButtonId);
	int *GumpSent;
	unsigned short (*GetQuantity)(unsigned int Container, unsigned short Graphic);
	/* CatchBag */
	int (*SetCatchBag)(unsigned int Serial);
	int (*UnSetCatchBag)(void);
	/* Numero de objetos */
	unsigned int *QtdObjects;
	/* Walk */
	void (*Walk)(char *Direction);
	/* Menu */
	void *MenuOptions;
	/* Target */
	void (*Target_WaitTargetTile)(unsigned short x, unsigned short y, unsigned char z);
	/* Teleto */
	int (*Teleto_Rew)(char *FileName);
	int (*Teleto_Go)(char *FileName);
    /* Calculo */
    void (*GetRefName)(char **Where, short X, short Y);
	/* Target */
	void (*Target_SendTarget)(unsigned char TargetType, unsigned short X, unsigned short Y, unsigned char Z, unsigned int Serial, unsigned short Graphic);
	void (*Target_RecvTarget)(void);
	/* Sell */
	int *SellListSent;
	int (*SellItems)(unsigned short Graphic);
	/* handle cmd */
	void (*HandleCmd)(char *Text);
	/* mass move */
	unsigned int (*MassMove)(unsigned int FromSerial, unsigned int ToSerial);
	unsigned int (*MassMoveGraphic)(unsigned int FromSerial, unsigned int ToSerial, unsigned short Graphic);
};

BOOL PrintDebug;

struct ExpFunc DllExpFunc;
typedef int (*PlInit)(struct ExpFunc *exp);
PlInit PluginInit;

char SHARDADDR[100];
short SHARDPORT;
char LOGUIN[30];
char SENHA[30];
char *VERSION;
char CHARNAME[30];

int Start(char*, char *,char *);
