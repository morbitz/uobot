#include <windows.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "uobot_net.h"
#include "uobot_log.h"


typedef struct
{
	unsigned int Id;
	unsigned short Graphic;
	unsigned short Color;
	unsigned short Qtd;
	unsigned short Preco;
	unsigned char *Name;
} VendListTag;

int WaitSellList = 0;
unsigned short SellWaitGraphic = 0;

int SellListSent = 0;

int SellItems(unsigned short Graphic)
{
	WaitSellList = 1;
	SellWaitGraphic = Graphic;
	SellListSent = 0;
	return 1;
}

int uo_handle_sell(char *buf,int len)
{
	unsigned int KeeperId = unpack32(buf+3);
	unsigned short NumItems = unpack16(buf+7), SentItems = 0;
	unsigned int x;
	unsigned short NameLen;
	unsigned char *SellPacket;
	VendListTag *VendList = 0;

	/* retornar caso nao esteja esperando pelo sell list */
	if(!WaitSellList)
		return 0;

	/* se o numero de items enviados pelo sell for = 0 */
	if(NumItems == 0)
	{
		LogPrint(3, "[SELL] Você não possui nada q interesse esse npc\n");
		SellListSent = 1;
		return 0;
	}

	/* criar pacote de resposta */
	SellPacket = malloc(9);
	SellPacket[0] = 0x9F;
	pack32(SellPacket+3, KeeperId);
	SentItems = 0;

	buf += 9;

	/* alocar espaco pro numero de items */
	VendList = malloc(NumItems * sizeof(VendListTag));

	/* gravar os items na lista */
	for(x=0; x < NumItems; x++)
	{
		VendList[x].Id = unpack32(buf);
		VendList[x].Graphic = unpack16(buf+4);
		VendList[x].Color = unpack16(buf+6);
		VendList[x].Qtd = unpack16(buf+8);
		VendList[x].Preco = unpack16(buf+10);
		NameLen = unpack16(buf+12);
		
		/* alocar espaco pro nome + '\0' */
		VendList[x].Name = malloc(NameLen+1);

		/* copiar nome e zerar o final */
		memcpy(VendList[x].Name, buf+14, NameLen);
		VendList[x].Name[NameLen] = 0;

		/* debug */
		//LogPrint(3, "[SELL] S[0x%X] G[0x%X] Qtd[%d] $[%d]\n",
		//	VendList[x].Id, VendList[x].Graphic, VendList[x].Qtd, VendList[x].Preco);

		/* pular buf pro proximo item */
		buf += NameLen + 14;

		/* verificar se o item corresponde ao grafico de espera */
		if(VendList[x].Graphic == SellWaitGraphic)
		{
			unsigned char *TmpSent;

			//LogPrint(3, "[SELL] Encontrado item [0x%X]\n", VendList[x].Id);
			SentItems++;
			SellPacket = realloc(SellPacket, (SentItems * 6) + 9);
			TmpSent = SellPacket + (9 + ((SentItems-1) * 6));
			pack32(TmpSent, VendList[x].Id);
			pack16(TmpSent+4, VendList[x].Qtd);
		}
	}

	pack16(SellPacket+1, (unsigned short)((SentItems * 6) + 9));
	pack16(SellPacket+7, SentItems);
	//LogDump(SellPacket, (SentItems*6)+9);

	send_server(SellPacket, (SentItems*6)+9);

	SellListSent = 1;
	WaitSellList = 0;

	/* liberar espaco alocado */
	for(x=0; x < NumItems; x++)
	{
		free(VendList[x].Name);
	}
	free(VendList);
	free(SellPacket);

	return 0;
}