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


typedef struct tagCharacterInfo
{
	/* all players (npcs and pcs) have these attributes */
	unsigned int LastAttack;
	unsigned short MaxHitPoints;
	unsigned short HitPoints;

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
	char HighLight;
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
	unsigned short Quantity;
	CharacterObject *Character; /* NULL if not a character (npc and pc) */
	//Edited - UOBOT
	char Name[30 + 1];
	struct tagObjInfo *Next;
}GameObject;


// Numero de Objetos
unsigned int gobjcount;


GameObject gobjects;
GameObject pickupobj;

GameObject *FindSerial(unsigned int serial);
int RemoveObj(unsigned int serial);
GameObject *AddObj(unsigned int serial);
void RemoveAllObjs(void);
GameObject *AddCharacter(unsigned int serial);

// Serial de mim
unsigned int mserial;
unsigned int mbackpack;


int CountItemInContainer(unsigned short Graphic, unsigned int Serial);
unsigned int FindItemInContainer(unsigned short Graphic, unsigned short Color, unsigned int ContainerSerial);
unsigned int GetItemInLayer(unsigned int ContainerSerial, int Layer);


void UseObject(unsigned int Serial);
void ClickObject(unsigned int Serial);
int GetDistance(int Sx, int Sy, int Tx, int Ty);
void MoveToContainer(unsigned int ItemSerial, unsigned short Quantity, unsigned int ContainerSerial);
void PickupItem(unsigned int ItemSerial, unsigned short Quantity);
void DropItem(unsigned int ItemSerial, unsigned short X, unsigned short Y, int Z);
int EquipItem(unsigned int ItemSerial, unsigned short Quantity, int Layer);
void UnequipItem(int Layer);
int EquipItemType(unsigned short Graphic, unsigned short Quantity, int Layer);
unsigned int UseObjectType(unsigned short Graphic);
int Life(unsigned int Hit, unsigned int MaxHit);
int IsItemInLayer(int Layer);
void SendResync(void);
unsigned short GetQuantity(unsigned int Container, unsigned short Graphic);
unsigned int CheckCont(unsigned short Graphic);

int SetCatchBag(unsigned int Serial);
int UnSetCatchBag(void);
BOOL IsCatchBagSet;
unsigned int CatchBag;
unsigned int LastCaught;

void GetRefName(char **Where, short X, short Y);
unsigned int MassMove(unsigned int FromSerial, unsigned int ToSerial);
unsigned int MassMoveGraphic(unsigned int FromSerial, unsigned int ToSerial, unsigned short Graphic);