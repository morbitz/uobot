#include <windows.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define USE_DISTANCE 3
#define TARGET_OBJECT 0
#define TARGET_TILE 1

#define LAYER_NONE 0
#define LAYER_ONE_HANDED 1
#define LAYER_TWO_HANDED 2
#define LAYER_SHOES 3
#define LAYER_PANTS 4
#define LAYER_SHIRT 5
#define LAYER_HELM 6  /* hat */
#define LAYER_GLOVES 7
#define LAYER_RING 8
#define LAYER_9 9      /* unused */
#define LAYER_NECK 10
#define LAYER_HAIR 11
#define LAYER_WAIST 12 /* half apron */
#define LAYER_TORSO 13 /* chest armour */
#define LAYER_BRACELET 14
#define LAYER_15 15        /* unused */
#define LAYER_FACIAL_HAIR 16
#define LAYER_TUNIC 17 /* surcoat, tunic, full apron, sash */
#define LAYER_EARRINGS 18
#define LAYER_ARMS 19
#define LAYER_CLOAK 20
#define LAYER_BACKPACK 21
#define LAYER_ROBE 22
#define LAYER_SKIRT 23 /* skirt, kilt */
#define LAYER_LEGS 24  /* leg armour */
#define LAYER_MOUNT 25 /* horse, ostard, etc */
#define LAYER_VENDOR_BUY_RESTOCK 26
#define LAYER_VENDOR_BUY 27
#define LAYER_VENDOR_SELL 28
#define LAYER_BANK 29


#define INVALID_SERIAL 0xFFFFFFFF
#define INVALID_XY 0xFFFF
#define INVALID_GRAPHIC 0xFFFF
#define INVALID_COLOR 0xFFFF


#define JOURNAL_SIZE 100

#define JOURNAL_SPEECH 0
#define JOURNAL_SYSMSG 1
#define JOURNAL_EMOTE 2 /* adds *'s as part of text */
#define JOURNAL_YOUSEE 6 /* You see: */
#define JOURNAL_EMPHASIS 7 /* clears previous messages */
#define JOURNAL_WHISPER 8
#define JOURNAL_YELL 9
#define JOURNAL_SPELL 10
#define JOURNAL_NONE -1
#define JOURNAL_ANY -2

typedef struct tagCharacterInfo
{
	/* all players (npcs and pcs) have these attributes */
	unsigned int LastAttack;
	unsigned short MaxHitPoints;
	unsigned short HitPoints;
	char Name[30 + 1];

	/* character only attributes */
	unsigned short STR;
	unsigned short DEX;
	unsigned short INT;
	unsigned short MaxStamina;
	unsigned short Stamina;
	unsigned short MaxMana;
	unsigned short Mana;

	unsigned int Gold;
	unsigned short Armor;
	unsigned short Weight;
}CharacterObject;

typedef struct tagObjInfo
{
	unsigned int Container; /* serial of object which contains this item */
	BOOL IsContainer;
	unsigned int Serial;	
	unsigned short Graphic;
	unsigned short Color;
	unsigned short X, Y;
	char Direction;
	char Z;
	int Flags;
	int Layer;
	int Quantity;
	CharacterObject *Character; /* NULL if not a character (npc and pc) */
	struct tagObjInfo *Next;
}GameObject;

struct ExpFunc {
	//Send Server
	int (*send_server)(char *buf, int len);
	//LogDump/LogPrint
	void (*LogDump)(unsigned char *pBuffer, int length);
	void (*LogPrint)(int Type, const char *strFormat, ...);
	//Container Func
	int (*CountItemInContainer)(unsigned short Graphic, unsigned int Serial);
	unsigned int (*FindItemInContainer)(unsigned short Graphic, unsigned short Color, unsigned int ContainerSerial);
	GameObject *(*FindSerial)(unsigned int serial);
	unsigned int (*GetItemInLayer)(unsigned int ContainerSerial, int Layer);
	// Stuff
	void (*pack16)(unsigned char *Buf, unsigned short x);
	void (*pack32)(unsigned char *Buf, unsigned int x);
	int (*UnicodeToAscii)(const char *UnicodeText, int Len, char *AsciiText);
	unsigned short (*unpack16)(const unsigned char *Buf);
	unsigned int (*unpack32)(const unsigned char *Buf);
	// Jornal
	char*(*GetJournalLine)(unsigned int Line);
	char*(*JournalGetLast)(void);
	void (*SetJournalLine)(unsigned int Line, char *Replace);
	int (*IsInJournal)(char *Text, int Type);
	void (*CleanJournal)(void);
	//Say
	void (*Say)(char *Texto, ...);
	//Targeting
	void (*Target_WaitTarget)(unsigned int Serial);
	void (*Target_WaitTargetGraphic)(unsigned short Graphic);
	//Commands
	void (*UseObject)(unsigned int Serial);
	void (*ClickObject)(unsigned int Serial);
	int (*GetDistance)(int Sx, int Sy, int Tx, int Ty);
	void (*MoveToContainer)(unsigned int ItemSerial, unsigned short Quantity, unsigned int ContainerSerial);
	void (*PickupItem)(unsigned int ItemSerial, unsigned short Quantity);
	void (*DropItem)(unsigned int ItemSerial, unsigned short X, unsigned short Y, int Z);
	void (*EquipItem)(unsigned int ItemSerial, unsigned short Quantity, int Layer);
	void (*UnequipItem)(int Layer);
	void (*EquipItemType)(unsigned short Graphic, unsigned short Quantity, int Layer);
	void (*UseObjectType)(unsigned short Graphic);
	//Actions
	void (*CastSpell)(char *SpellName);
	void (*UseSkill)(char *SkillName);
	// Variaveis
	int *mbackpack;
	int *mserial;
};

struct ExpFunc *ImportFunc=0;

BOOL APIENTRY DllMain(HINSTANCE hDLLInst, DWORD fdwReason, LPVOID lpvReserved){
 switch(fdwReason){
  case DLL_PROCESS_ATTACH: break;
  case DLL_PROCESS_DETACH: break;
  case DLL_THREAD_ATTACH: break;
  case DLL_THREAD_DETACH: break;
 }
 return TRUE;
}

int PluginInit(struct ExpFunc *TmpFunc){
	ImportFunc = TmpFunc;
	if(ImportFunc==0) return 0;
	ImportFunc->Say("Plugin rodou!! Tudo ok!\n");
	return 0;
}
