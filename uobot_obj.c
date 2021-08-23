#include <windows.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include "uobot_obj.h"
#include "uobot_handles.h"
#include "uobot_net.h"
#include "uobot_log.h"
#include "uobot_jornal.h"
#include "uobot_target.h"
#include "uobot_spells.h"
#include "uobot_threads.h"
#include "uobot.h"

// Numero de Objetos
unsigned int gobjcount=0;

// Onde os objetos serao guardados
GameObject gobjects={0};
GameObject pickupobj={0};

int IsCatchBagSet = 0;
unsigned int CatchBag = 0;
unsigned int LastCaught = 0;

unsigned int mserial=0;
unsigned int mbackpack=0;

int SetCatchBag(unsigned int Serial)
{
	IsCatchBagSet = 1;
	CatchBag = Serial;
	LastCaught = 0;
	return 1;
}

int UnSetCatchBag(void)
{
	IsCatchBagSet = 0;
	CatchBag = 0;
	LastCaught = 0;
	return 1;
}

GameObject *FindSerial(unsigned int serial)
{
	GameObject *TmpObj=&gobjects;
	unsigned int x=0;
	for(x=0; x < gobjcount; x++)
	{
		if(TmpObj->Serial==serial) return TmpObj;
		TmpObj = TmpObj->Next;
	}
	return 0;
}

GameObject *AddObj(unsigned int serial)
{
	GameObject *TmpObj=FindSerial(serial);
	unsigned int x=0;
	if(TmpObj!=0)
		return TmpObj;
	TmpObj = &gobjects;
	for(x=0;x<gobjcount;x++) TmpObj = TmpObj->Next;
	TmpObj->Character = 0;
	TmpObj->Flags = 0;
	TmpObj->Serial = serial;
	TmpObj->Container = INVALID_SERIAL;
	TmpObj->IsContainer = FALSE;
	TmpObj->Color = 0;
	TmpObj->Layer = LAYER_NONE;
	TmpObj->Quantity = 0;
	TmpObj->Graphic = 0;
	TmpObj->X = INVALID_XY;
	TmpObj->Y = INVALID_XY;
	TmpObj->Z = 0;
	strncpy(TmpObj->Name, "<not initialized>",30);
	if((TmpObj->Next=malloc(sizeof(GameObject)))==NULL){
		LogPrint(3, "[ERROR] Nao foi possivel alocar espaco pro novo obj\n");
		exit(-1);
	}
	gobjcount++;
	//LogPrint((PrintDebug ? 2 : 0), "[OBJ] Criado objeto[%#x] Count[%u]\n", serial, gobjcount);
	//ClickObject(serial);
	return TmpObj;
}

int RemoveObj(unsigned int serial)
{
	unsigned int x=0,i=0;
	GameObject *TmpObj=&gobjects,*TmpObj2=&gobjects;
	if(TmpObj==0) return 0;
	if(serial==mserial) return 0;
	for(x=0;x<gobjcount;x++)
	{
		if(TmpObj->Serial==serial)
		{
			TmpObj2->Next = TmpObj->Next;
			if(TmpObj->Character!=0) free(TmpObj->Character);
			free(TmpObj);
			gobjcount--;
			return 1;
		}
		TmpObj2 = TmpObj;
		TmpObj = TmpObj->Next;
	}
	return 0;
}

void RemoveAllObjs(void)
{
	unsigned int x=0;
	GameObject *TmpObj=gobjects.Next,*TmpObj2=gobjects.Next;
	for(x=0;x<gobjcount-1;x++)
	{
		TmpObj = TmpObj2;
		if(TmpObj==0){
			gobjcount = 0;
			gobjects.Next = 0;
			return;
		}
		TmpObj2 = TmpObj->Next;
		//LogPrint((PrintDebug ? 3 : 0), "[OBJ] Removido[%u] objeto[%#x]\n", x, TmpObj->Serial);
		if(TmpObj->Character!=0) free(TmpObj->Character);
		free(TmpObj);
	}
	gobjects.Next = 0;
	gobjcount = 0;
}

GameObject *AddCharacter(unsigned int serial)
{
	unsigned char GetInfo[10];
	int ObjIdx = 0;
	GameObject *TmpObj=AddObj(serial);

	/* only reset it if it doesnt exist */
	if(TmpObj->Character == 0)
	{
		/* helps to keep the memory usage to a minimum */
		if((TmpObj->Character=malloc(sizeof(CharacterObject)))==NULL)
		{
			LogPrint(3, "[ERROR] Impossivel alocar espaco para character\n");
			exit(-1);
		}

		TmpObj->Character->MaxHitPoints = 0;
		TmpObj->Character->HitPoints = 0;
		//strncpy(TmpObj->Name, "<not initialized>",30);

		TmpObj->Character->STR = 0;
		TmpObj->Character->DEX = 0;
		TmpObj->Character->INT = 0;
		TmpObj->Character->MaxStamina = 0;
		TmpObj->Character->Stamina = 0;
		TmpObj->Character->MaxMana = 0;
		TmpObj->Character->Mana = 0;

		TmpObj->Character->Gold = 0;
		TmpObj->Character->Armor = 0;
		TmpObj->Character->Weight = 0;

		/* send an object info request to the server */
		GetInfo[0] = 0x34;
		pack32(GetInfo + 1, 0xedededed);
		GetInfo[5] = 0x04; /* type: get basic stats */
		pack32(GetInfo + 6, TmpObj->Serial);

		//LogPrint((PrintDebug ? 3: 0), "[ADDCHARACTER] Requesting object info: 0x%08x\n", TmpObj->Serial);
		send_server(GetInfo, 10);
		ClickObject(TmpObj->Serial);
		return TmpObj;
	}
	return TmpObj;
}


int CountItemInContainer(unsigned short Graphic, unsigned int Serial)
{
	unsigned int i = 0, Count = 0;
	GameObject *TmpObj=&gobjects;
	for(i = 0; i < gobjcount; i++)
	{
		if(TmpObj==0) return Count;
		if(TmpObj->Container == Serial && TmpObj->Graphic == Graphic)
		{
			/* if this item is a container, add to the count the items in it */
			if(TmpObj->IsContainer)
				Count += CountItemInContainer(Graphic, TmpObj->Serial);

			/* add this item to the count */
			Count += TmpObj->Quantity;
		}
		TmpObj = TmpObj->Next;
	}
	
	return Count;
}

unsigned int FindItemInContainer(unsigned short Graphic, unsigned short Color, unsigned int ContainerSerial)
{
	unsigned int i = 0;
	unsigned int ObjSerial = INVALID_SERIAL;
	GameObject *TmpObj=&gobjects;

	for(i = 0; i < gobjcount; i++)
	{
		if(TmpObj==0) return INVALID_SERIAL;

		if(TmpObj->Container == ContainerSerial)
		{
			/* container inside container, search in it as well */
			if(TmpObj->IsContainer)
			{
				ObjSerial = FindItemInContainer(Graphic, Color, TmpObj->Serial);
				if(ObjSerial != INVALID_SERIAL)
					return ObjSerial;
			}

			/* by graphic only */
			if(Color == INVALID_COLOR && Graphic != INVALID_GRAPHIC && TmpObj->Graphic == Graphic)
					return TmpObj->Serial;

			/* by color only */
			if(Graphic == INVALID_GRAPHIC && Color != INVALID_COLOR && TmpObj->Color == Color)
				return TmpObj->Serial;

			/* by graphic and color */
			if( Graphic != INVALID_GRAPHIC && Color != INVALID_COLOR &&
				TmpObj->Color == Color && TmpObj->Graphic == Graphic )
				return TmpObj->Serial;
		}
		TmpObj = TmpObj->Next;
	}

	return INVALID_SERIAL;
}

unsigned int GetItemInLayer(unsigned int ContainerSerial, int Layer)
{
	unsigned int i = 0;
	GameObject *TmpObj=&gobjects;

	for(i = 0; i < gobjcount; i++)
	{
		if(TmpObj==0) return INVALID_SERIAL;

		if(TmpObj->Container == ContainerSerial && TmpObj->Layer == Layer)
			return TmpObj->Serial;
		TmpObj = TmpObj->Next;
	}

	return INVALID_SERIAL;
}

void UseObject(unsigned int Serial)
{
	unsigned char DoubleClick[5] = { 0x06, 0x00, 0x00, 0x00, 0x00 };

	//SetLastObject(Serial);
	pack32(DoubleClick + 1, Serial);
	send_server(DoubleClick, 5);

	return;
}

unsigned int UseObjectType(unsigned short Graphic)
{
	unsigned int Serial;
	unsigned char DoubleClick[5] = { 0x06, 0x00, 0x00, 0x00, 0x00 };
	Serial = FindItemInContainer(Graphic, INVALID_COLOR, mserial);

	if(Serial == INVALID_SERIAL)
	{
		LogPrint(3, "Could not find graphic 0x%04X in backpack to useobjecttype\n", Graphic);
		return 0;
	}

	//SetLastObject(Serial);
	pack32(DoubleClick + 1, Serial);
	send_server(DoubleClick, 5);

	return Serial;
}

void ClickObject(unsigned int Serial)
{
	unsigned char SingleClick[5] = { 0x09, 0x00, 0x00, 0x00, 0x00 };

	pack32(SingleClick + 1, Serial);
	send_server(SingleClick, 5);

	return;
}

int GetDistance(int Sx, int Sy, int Tx, int Ty)
{
	return (int)sqrt((Sx - Tx)*(Sx - Tx) + (Sy - Ty)*(Sy - Ty));
}

void MoveToContainer(unsigned int ItemSerial, unsigned short Quantity, unsigned int ContainerSerial)
{
	unsigned char PickupPacket[21];
	GameObject *Eu = FindSerial(mserial);

	PickupPacket[0] = 0x07;
	pack32(PickupPacket + 1, ItemSerial);
	pack16(PickupPacket + 5, Quantity);

	if(ContainerSerial == 0 && Eu)
	{
		PickupPacket[7] = 0x08;
		pack32(PickupPacket + 8, ItemSerial);
		pack16(PickupPacket + 12, Eu->X);
		pack16(PickupPacket + 14, Eu->Y);
		PickupPacket[16] = Eu->Z;
		pack32(PickupPacket + 17, 0xFFFFFFFF);
		send_server(PickupPacket, 21);
		return;
	}

	PickupPacket[7] = 0x08;
	pack32(PickupPacket + 8, ItemSerial);
	pack16(PickupPacket + 12, INVALID_XY);
	pack16(PickupPacket + 14, INVALID_XY);
	PickupPacket[16] = 0;
	pack32(PickupPacket + 17, ContainerSerial);

	send_server(PickupPacket, 21);
	return;
}

void PickupItem(unsigned int ItemSerial, unsigned short Quantity)
{
	unsigned char PickupPacket[7];

	PickupPacket[0] = 0x07;
	pack32(PickupPacket + 1, ItemSerial);
	pack16(PickupPacket + 5, Quantity);

	send_server(PickupPacket, 7);

	return;
}

void DropItem(unsigned int ItemSerial, unsigned short X, unsigned short Y, int Z)
{
	unsigned char DropPacket[14];

	DropPacket[0] = 0x08;
	pack32(DropPacket + 1, ItemSerial);
	pack16(DropPacket + 5, X);
	pack16(DropPacket + 7, Y);
	DropPacket[9] = Z & 0xff;
	pack32(DropPacket + 10, INVALID_SERIAL);

	send_server(DropPacket, 14);

	return;
}

int EquipItem(unsigned int ItemSerial, unsigned short Quantity, int Layer)
{
	unsigned char PickupPacket[20];

	if(GetItemInLayer(mserial, Layer)!=INVALID_SERIAL)
	{
		LogPrint(3, "[ERROR] Layer %u ja tem um objeto\n", Layer);
		return 0;
	}
	PickupPacket[0] = 0x07;
	pack32(PickupPacket + 1, ItemSerial);
	pack16(PickupPacket + 5, Quantity);

	PickupPacket[7] = 0x13;
	pack32(PickupPacket + 8, ItemSerial);
	PickupPacket[12] = Layer & 0xff;
	pack32(PickupPacket + 13, mserial);

	send_server((char*)PickupPacket, 17);

	return 1;
}

int EquipItemType(unsigned short Graphic, unsigned short Quantity, int Layer)
{
	unsigned char PickupPacket[17], *WearPacket=PickupPacket+7;
	unsigned int ItemSerial;
	if(GetItemInLayer(mserial, Layer)!=INVALID_SERIAL)
	{
		LogPrint(3, "[ERROR] Layer %u ja tem um objeto\n", Layer);
		return 0;
	}
	ItemSerial = FindItemInContainer(Graphic, INVALID_COLOR, mserial);

	if(ItemSerial == INVALID_SERIAL)
	{
		LogPrint(3, "Could not find graphic 0x%04X in backpack to equiptype\n", Graphic);
		return 0;
	}

	PickupPacket[0] = 0x07;
	pack32(PickupPacket + 1, ItemSerial);
	pack16(PickupPacket + 5, Quantity);

	WearPacket[0] = 0x13;
	pack32(WearPacket + 1, ItemSerial);
	WearPacket[5] = Layer & 0xff;
	pack32(WearPacket + 6, mserial);

	send_server(PickupPacket, 17);

	return 1;
}

void UnequipItem(int Layer)
{
	if(GetItemInLayer(mserial, Layer) != INVALID_SERIAL)
		MoveToContainer(GetItemInLayer(mserial, Layer), 1, mbackpack);

	return;
}

int IsItemInLayer(int Layer)
{
	if(GetItemInLayer(mserial, Layer)==INVALID_SERIAL)
		return 0;
	else
		return 1;
}

int Life(unsigned int Hit, unsigned int MaxHit)
{
	double Porcents=0.0;
	double Ret=0.0;
	Porcents = ((double)MaxHit)/100;
	Ret = ((double)Hit)/Porcents;
	if(MaxHit==0)
	{
		LogPrint(3, "[LIFE] Maxhit==0\n");
		return 0;
	}
	return (int)Ret;
}

void SendResync(void)
{
	char buf[3]={0};
	buf[0] = 0x22;
	send_server(buf, 3);
}


unsigned short GetQuantity(unsigned int Container, unsigned short Graphic)
{
	GameObject *Obj=&gobjects;
	unsigned short Total=0;
	unsigned int x=0;
	/*LogPrint(1, "[CONTAINER] C:[%#x] G:[%#x]\n", Container, Graphic);*/
	for(x=0; x < gobjcount; x++)
	{
		if(Obj->Container != Container)
		{
			Obj = Obj->Next;
			continue;
		}
		if(Obj->IsContainer)
		{
			/*LogPrint(1, "[CONTAINER] T:[%d] S:[%#x] G:[%#x]\n", Total, Obj->Serial, Obj->Graphic);*/
			Total += GetQuantity(Obj->Serial, Graphic);
			/*LogPrint(1, "[CONTAINER] T:[%d] S:[%#x] G:[%#x]\n", Total, Obj->Serial, Obj->Graphic);*/
		}
		else if(Obj->Graphic == Graphic)
			Total += Obj->Quantity;
		Obj = Obj->Next;
	}
	/* LogPrint(3, "[TOTAL] G:[%#x] T:[%d]\n", Graphic, Total);*/
	return Total;
}

unsigned int CheckCont(unsigned short Graphic)
{
	if(Graphic == 0xE76)
		return 1;
	if(Graphic == 0xE75)
		return 1;
	return 0;
}

void GetRefName(char **Where, short X, short Y)
{
	if(X==0 && Y==0)
		*Where = "mesmo local";
	else if(X==0 && Y>0)
		*Where = "sul";
	else if(X==0 && Y<0)
		*Where = "norte";
	else if(Y==0 && X>0)
		*Where = "leste";
    else if(Y==0 && X<0)
		*Where = "oeste";
	else if(X>0 && Y>0)
		*Where = "sudeste";
	else if(X>0 && Y<0)
		*Where = "nordeste";
	else if(X<0 && Y>0)
		*Where = "sudoeste";
	else if(X<0 && Y<0)
		*Where = "noroeste";
    else
        *Where = "nao sei";

    return;
}

unsigned int MassMove(unsigned int FromSerial, unsigned int ToSerial)
{
	GameObject *Obj;
	unsigned int x, count = 0;

	if((Obj=FindSerial(FromSerial))==0)
	{
		LogPrint(3, "[MASSMOVE] Nao foi possivel localizar objeto desse serial [0x%X]\n", FromSerial);
		return 0;
	}

	Obj = &gobjects;

	for(x=1; x < gobjcount; x++)
	{
		if(Obj->Next)
			Obj = Obj->Next;
		else
			break;

		/* checkzinho */
		if(Obj->Serial == FromSerial || Obj->Serial == ToSerial)
			continue;

		/* se nao estiver dentro da bag do fromserial continuar */
		if(Obj->Container != FromSerial)
			continue;

		/* se o item for um container, mover os itens dentro dele tb */
		if(Obj->IsContainer)
		{
			LogPrint(2, "[MASSMOVE] Localizado outra bag dentro dessa bag [0x%X]\n", Obj->Serial);
			count += MassMove(Obj->Serial, ToSerial);
		}
		
		//LogPrint(2, "[MASSMOVE] Localizado item [0x%X] [%s]\n", Obj->Serial, Obj->Name);
		/* caso o item estiver dentro do fromserial, mover o item para o container toserial */

		if(ToSerial == 0xFFFFFFFF)
		{
			PickupItem(Obj->Serial, Obj->Quantity);
			DropItem(Obj->Serial, gobjects.X, gobjects.Y, gobjects.Z);
		}
		else
			MoveToContainer(Obj->Serial, Obj->Quantity, ToSerial);

		count++;
	}

	return count;
}

unsigned int MassMoveGraphic(unsigned int FromSerial, unsigned int ToSerial, unsigned short Graphic)
{
	GameObject *Obj;
	unsigned int x, count = 0;

	if((Obj=FindSerial(FromSerial))==0)
	{
		LogPrint(3, "[MASSMOVE] Nao foi possivel localizar objeto desse serial [0x%X]\n", FromSerial);
		return 0;
	}

	Obj = &gobjects;


	for(x=0; x < gobjcount - 1; x++)
	{
		if(Obj->Next)
			Obj = Obj->Next;
		else
			break;

		/* checkzinho */
		if(Obj->Serial == FromSerial || Obj->Serial == ToSerial)
			continue;

		/* se nao estiver dentro da bag do fromserial continuar */
		if(Obj->Container != FromSerial)
			continue;

		/* se o item for um container, mover os itens dentro dele tb */
		if(Obj->IsContainer)
			count += MassMove(Obj->Serial, ToSerial);

		/* caso o item nao tiver o grafico procurado, continuar */
		if(Obj->Graphic != Graphic)
			continue;

		if(ToSerial == 0xFFFFFFFF)
		{
			PickupItem(Obj->Serial, Obj->Quantity);
			DropItem(Obj->Serial, gobjects.X, gobjects.Y, gobjects.Z);
		}
		else
			MoveToContainer(Obj->Serial, Obj->Quantity, ToSerial);

		count++;
	}

	return count;
}

