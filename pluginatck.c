#include <windows.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "pluginatck.h"

/* MUITO IMPORTANTE
 * COLEQUE O NOME EXATO DO ARQUIVO .DLL
 */
#define DLLNAME "pluginatck.dll"

struct ExpFunc *ImportFunc=0;

/* main() */
BOOL APIENTRY DllMain(HINSTANCE hDLLInst, DWORD fdwReason, LPVOID lpvReserved)
{
 return TRUE;
}

/* Serial do player e layer para ser usado no macro de combat */
unsigned int AniSerial;
unsigned ArmLayer=2;
/* Algumas flags pro macro de combat */
int LifeDown = 0;
int FixRec = 1;

int uo_handle_updatehp(unsigned char *buf, int len)
{
	unsigned int serial=ImportFunc->unpack32(buf+1);
	unsigned int Hits=ImportFunc->unpack16(buf+7),MaxHits=ImportFunc->unpack16(buf+5);
	if(serial!=AniSerial) return 0;
	if(FixRec) return 0;
	if(ImportFunc->Life(Hits, MaxHits) < 40)
	{
		LifeDown = 1;
		ImportFunc->LogPrint(1, "[PLUGIN] Life<40 %d/100\n", ImportFunc->Life(Hits, MaxHits));
		if(ImportFunc->GetItemInLayer(*ImportFunc->mserial, ArmLayer) != INVALID_SERIAL)
			ImportFunc->MoveToContainer(ImportFunc->GetItemInLayer(*ImportFunc->mserial, ArmLayer), 1, *ImportFunc->mbackpack);
		ImportFunc->Say("todos pare");
		ImportFunc->WarMode(0);
	}
	return 0;
}

void Command_Combat(CmdInfoTag *CI){
	/* Graphics das bands, ars e barril */
	unsigned short Barril=0x154D;
	unsigned short BandSuja=0x0E20;
	unsigned short ArmType=0x0F45, BandType=0x0E21;
	unsigned short CapuzType=0x13BB,TunicaType=0x13BF;
	unsigned short LuvaType=0x13C6,CalcaType=0x13C3;
	unsigned short ShieldType = 0x1B76;
	GameObject *IniObj = 0;
	unsigned char Macro = 0;
	unsigned int delay=500;


	if(CI->Argc != 4 && CI->Argc != 5)
	{
		ImportFunc->LogPrint(3, "[PLUGIN] Uso: %s serialdoplayer armtype armlayer (delay)\n", CI->Args[0]);
		ImportFunc->LogPrint(3, "[PLUGIN] Para combat parry, utilize armlayer = 0\n");
		ImportFunc->LogPrint(3, "[PLUGIN] Para combat wrest, utilize armtype E armlayer = 0\n");
		ImportFunc->CmdThreadExit(CI);
		return;
	}

	if(CI->Argc==5)
	{
		delay = ImportFunc->ArgToInt(CI->Args[4]);
		ImportFunc->LogPrint(3, "[PLUGIN] Delay: %d\n", delay);
	}

	/* Serial pessoa para atacar/curar */
	AniSerial = ImportFunc->ArgToInt(CI->Args[1]);
	/* Graphic da arma, lumber = 0x0F45 */
	ArmType = (unsigned short)ImportFunc->ArgToInt(CI->Args[2]);
	/* Layer da arma, escudo = 2, lumber = 2 */
	ArmLayer = ImportFunc->ArgToInt(CI->Args[3]);

	/* Macro = mace/sw/fenc/lumber */
	Macro = 1;

	/* Verificar se ArmType e ArmLayer == 0, se for, entao o macro eh wrest */
	if(!ArmType && !ArmLayer)
	{
		Macro = 0;
		ImportFunc->LogPrint(3, "[PLUGIN] Script wrest loaded!\n");
	}
	/* Se ArmType != 0 e ArmLayer == 0, macro = parry */
	else if(ArmType && !ArmLayer)
	{
		Macro = 2;
		ArmLayer = 1;
		ImportFunc->LogPrint(3, "[PLUGIN] Script parry loaded!\n");
	}
	/* Macro = sw/mace/fenc/lumber */
	else
		ImportFunc->LogPrint(3, "[PLUGIN] Script mace/sw/fenc/lumber loaded!\n");

	ImportFunc->LogPrint(3, "[PLUGIN] AniSerial %#X ArmType %#X ArmLayer %d\n", AniSerial, ArmType, ArmLayer);

	/* Adicionar handler do pacote 0xA1 = updatehp */
	if(Macro)
		ImportFunc->AddPacketHandler(0xA1, (void *)uo_handle_updatehp);
	else
		goto combatloop;

lifebaixo:

	/* Nao comecar enquanto nao localizar a pessoa para curar/atacar */
	while((IniObj=ImportFunc->FindSerial(AniSerial)) == 0) Sleep(10000);
	/* Nao comecar enquanto a pessoa estiver morta */
	while(IniObj->Character->HitPoints == 0) Sleep(1000);
	/* Curar a pessoa caso n esteja com life full */
	while(IniObj->Character->HitPoints != IniObj->Character->MaxHitPoints)
	{
		/* Target da pessoa */
		ImportFunc->Target_WaitTarget(AniSerial);
		do {
			/* Se nao localizar as bands, eh pq acabou */
			if(!ImportFunc->UseObjectType(BandType))
			{
				/* Limpar as bands sujas */
				ImportFunc->LogPrint(3, "[PLUGIN] Bands down\n");
				ImportFunc->Target_WaitTargetGraphic(Barril);
				Sleep(1000);
				ImportFunc->UseObjectType(BandSuja);
			}
			Sleep(2000);
		} while(*ImportFunc->TargetSent==0);
		*ImportFunc->TargetSent = 0;
	}

	ImportFunc->LogPrint(1, "[PLUGIN] [%s] Life: [%d/100]\n",
		IniObj->Name, ImportFunc->Life(IniObj->Character->HitPoints, IniObj->Character->MaxHitPoints));

combatloop:
	/* Avisar para o handler do updatehp que o macro comecou */
	FixRec = 0;
	/* Se o life estiver abaixo de 40 voltar para o inicio para curar a pessoa */
	if(Macro && LifeDown)
	{
		LifeDown = 0;
		goto lifebaixo;
	}
	/* Nao comecar enquanto nao achar a pessoa */
	while((IniObj=ImportFunc->FindSerial(AniSerial)) == 0) Sleep(1000);

	/*Atacar animal */
	ImportFunc->Attack(AniSerial);

	/* Caso o macro nao seja o wrest, verificar se tem uma arma na mao */
	if(Macro)
	{
		if(!ImportFunc->IsItemInLayer(ArmLayer))
			ImportFunc->EquipItemType(ArmType, 1, ArmLayer);
		if(Macro == 2)
		{
			if(!ImportFunc->IsItemInLayer(2))
				ImportFunc->EquipItemType(ShieldType, 1, 2);
		}
	}

	/*Se algo na armadura se partiu, por outra q esteja na bag*/
	if(!ImportFunc->IsItemInLayer(6))
		ImportFunc->EquipItemType(CapuzType, 1, 6);
	if(!ImportFunc->IsItemInLayer(13))
		ImportFunc->EquipItemType(TunicaType, 1, 13);
	if(!ImportFunc->IsItemInLayer(7))
		ImportFunc->EquipItemType(LuvaType, 1, 7);
	if(!ImportFunc->IsItemInLayer(4))
		ImportFunc->EquipItemType(CalcaType, 1, 4);
	
	/* Se o macro for wrest, dormir um tempo e retornar para o inicio */
	if(!Macro)
	{
		Sleep(30000);
		goto combatloop;
	}

	/* Curar a pessoa */
	ImportFunc->Target_WaitTarget(AniSerial);
	do {
		Sleep(delay);
		if(!ImportFunc->UseObjectType(BandType))
		{
			ImportFunc->LogPrint(3, "[PLUGIN] Bands down\n");
			ImportFunc->Target_WaitTargetGraphic(Barril);
			ImportFunc->UseObjectType(BandSuja);
		}
	} while(*ImportFunc->TargetSent==0);
	*ImportFunc->TargetSent = 0;
	Sleep(delay);
	goto combatloop;
	ImportFunc->LogPrint(3, "[PLUGIN] ERRO: Impossivel chegar aki\n");
	ImportFunc->CmdThreadExit(CI);
}



void Command_Heal(CmdInfoTag *CI)
{
	/*Graphic das bands*/
	unsigned short BandType=0x0E21;
	unsigned int Serial;
	unsigned int delay = 500;
	if(CI->Argc != 2 && CI->Argc != 3)
	{
		ImportFunc->LogPrint(3, "[PLUGIN] Uso: %s serial (delay)\n", CI->Args[0]);
		ImportFunc->CmdThreadExit(CI);
		return;
	}

	if(CI->Argc == 3)
	{
		delay = ImportFunc->ArgToInt(CI->Args[2]);
	}
	Serial = ImportFunc->ArgToInt(CI->Args[1]);
	ImportFunc->LogPrint(3, "[PLUGIN] Script heal loaded! Delay: %d\n", delay);
healloop:
	/*Qdo aparecer um target, escolher esse serial*/
	ImportFunc->Target_WaitTarget(Serial);
	/*Dois cliques nas bands*/
	ImportFunc->UseObjectType(BandType);
	Sleep(delay);
	goto healloop;
	ImportFunc->CmdThreadExit(CI);
}

void Command_Test(CmdInfoTag *CI) 
{
	ImportFunc->LogPrint(3, "[PLUGIN] Script test loaded\n");
	/* Primeiro limpa o jornal */
	ImportFunc->CleanJournal();
	do{
		Sleep(2000);
	/* Se alguem tiver digitado 'ola' ingame, ira ficar no jornal */
	} while(ImportFunc->IsInJournal("ola", JOURNAL_ANY)==-1);
	ImportFunc->Say("Opa, iae, sussa?\n");
	ImportFunc->CmdThreadExit(CI);
}

void Command_CastFS(CmdInfoTag *CI)
{
	unsigned int x = 0, SpellTimes = 0;
	unsigned int Delay = 0;

	unsigned char *SpellName = "Flame Strike";

	/* MyMana/MaxMan */
	unsigned short *MaxMana = 0;
	unsigned short *MyMana = 0;

	unsigned int Serial = 0;

	GameObject *mobj = ImportFunc->FindSerial(*ImportFunc->mserial);
	GameObject *CastObj = 0;

	if(CI->Argc != 4)
	{
		ImportFunc->LogPrint(3, "[PLUGIN] Uso: %s serial num_fs delay\n", CI->Args[0]);
		ImportFunc->CmdThreadExit(CI);
        return;
	}

	Sleep(10000);

	/* Setar as variaveis */

	/* Serial da pessoa que ira receber os fs */
	Serial = ImportFunc->ArgToInt(CI->Args[1]);
	/* Qtos fs esse char ira castar */
	SpellTimes = ImportFunc->ArgToInt(CI->Args[2]);
	/* Delay entre os fs, recomendado = 2000, proxy=4000 */
	Delay = ImportFunc->ArgToInt(CI->Args[3]);

	/* Se ainda n foi criado meu serial */
	if(mobj==0){
		ImportFunc->LogPrint(3, "[PLUGIN] Meu objeto n foi encontrado\n");
		ImportFunc->CmdThreadExit(CI);
		return;
	}

	/* Pegar endereco da mana atual e maximo de mana */
	MyMana = &mobj->Character->Mana;
	MaxMana = &mobj->Character->MaxMana;

	ImportFunc->LogPrint(3, "[PLUGIN] Script castfs loaded!!\n");
	ImportFunc->LogPrint(3, "[PLUGIN] SpellTimes[%d] Delay[%d]\n", SpellTimes, Delay);
	ImportFunc->LogPrint(3, "[PLUGIN] Mymana[%d] MaxMana[%d] Serial[%#x]\n", *MyMana, *MaxMana, Serial);

	/* Limpar jornal = Evitar bugs */
	ImportFunc->CleanJournal();

	/* Inutil */
	//ImportFunc->Say("Vou castar!!");

castar:
	/* Procurar pelo player com o Serial */
	CastObj = ImportFunc->FindSerial(Serial);

	/* Se nao achar, voltar pro inicio */
	if(CastObj==0)
	{
		ImportFunc->LogPrint(3, "[PLUGIN] Serial %#x nao encontrado\n", Serial);
		ImportFunc->LogPrint(3, "[PLUGIN] Esperando 10s para procurar novamente\n");
		Sleep(10000);
		goto castar;
	}

	/* Somente comecar a castar se a mana estiver full */
	if(*MyMana == *MaxMana)
	{
		for(x=0; x < SpellTimes; x++)
		{
			/* Se alguem mandou parar o macro, parar tudo */
			if(ImportFunc->IsInJournal("para tudo", -2)!=-1)
			{
				ImportFunc->LogPrint(3, "[PLUGIN] Plugin terminado\n");
				ImportFunc->CmdThreadExit(CI);
				return;
			}

			/* Verificar se o graphic != 0x190 */
			if(ImportFunc->IsDead(Serial))
			{
				ImportFunc->LogPrint(3, "[PLUGIN] Mataram o cara\n");
				ImportFunc->MBOut("Plugin Dead", "CastObj morreu");
				ImportFunc->CmdThreadExit(CI);
				return;
			}

			/* Enquanto o hp dele n estiver full, nao caste */
			while(CastObj->Character->HitPoints != CastObj->Character->MaxHitPoints) Sleep(1000);

			/* Setar o target */
			ImportFunc->Target_WaitTarget(Serial);
            do{
                ImportFunc->CastSpell(SpellName);
                Sleep(2000);
			} while(*ImportFunc->TargetSent==0);
			*ImportFunc->TargetSent = 0;

            Sleep(Delay);
		}
	}

    if(*MyMana == *MaxMana)
        goto castar;

	/* Meditar ate encher a mana */
	ImportFunc->LogPrint(1, "[PLUGIN] Esperar mana subir ate [%d]\n", *MaxMana);

	/* Meditar */
	do{
		ImportFunc->UseSkill("Meditation");
		Sleep(2000);
	} while(*MyMana < *MaxMana);

	Sleep(1000);

	/* Repetir o macro */
	goto castar;

	ImportFunc->CmdThreadExit(CI);
}

void Command_CastEb(CmdInfoTag *CI)
{
	unsigned int x=0,SpellTimes=0;

	unsigned short *MaxMana=0;
	unsigned short *Mana=0;
	unsigned short PlayerMana=0;

	unsigned char *SpellName="Energy Bolt";

	unsigned int Serial=0;

	GameObject *mobj=ImportFunc->FindSerial(*ImportFunc->mserial);
	GameObject *CastObj=0;

	if(CI->Argc != 2)
	{
		ImportFunc->LogPrint(3, "[PLUGIN] Uso: %s serial\n", CI->Args[0]);
		ImportFunc->CmdThreadExit(CI);
		return;
	}

	/* Pegar serial */
	Serial = ImportFunc->ArgToInt(CI->Args[1]);

	/* Se nao foi encontrado meu serial, sair fora */
	if(mobj==0)
	{
		ImportFunc->CmdThreadExit(CI);
		return;
	}


	if((CastObj=ImportFunc->FindSerial(Serial))==0)
	{
		ImportFunc->LogPrint(3, "[PLUGIN] Nao foi possivel achar serial [0x%X]\n", Serial);
		ImportFunc->CmdThreadExit(CI);
		return;
	}

	/* Pegar endereco da minha mana e maxmana */
	Mana = &mobj->Character->Mana;
	MaxMana = &mobj->Character->MaxMana;

	/* Verificar atraves da mana qtas vezes o player pode castar eb */
	PlayerMana = *MaxMana;
	while(PlayerMana>0){ PlayerMana -= 20; SpellTimes++; }
	PlayerMana += 20;
	if(PlayerMana<10) SpellTimes--;
	PlayerMana = mobj->Character->MaxMana;

	ImportFunc->LogPrint(3, "[PLUGIN] Script cast loaded!!\n");
	ImportFunc->LogPrint(3, "[PLUGIN] SpellTimes[%d] Mana[%d] MaxMana[%d]\n", SpellTimes, *Mana, *MaxMana);

tryagain:

	/* Limpar jornal */
	ImportFunc->CleanJournal();

castar:

	/* Se o castobj nao estiver logado, tentar novamente em 10s */
	if((CastObj=ImportFunc->FindSerial(Serial))==0)
	{
		ImportFunc->LogPrint(3, "[PLUGIN] Serial [0x%X] invalido\n", Serial);
		ImportFunc->LogPrint(3, "[PLUGIN] Esperando 10s para procurar novamente\n");
		Sleep(10000);
		goto castar;
	}

	if(ImportFunc->IsDead(Serial))
	{
		ImportFunc->LogPrint(3, "[PLUGIN] Serial [0x%X] esta morto\n", Serial);
		ImportFunc->LogPrint(3, "[PLUGIN] Esperando 10s para comecar novamente\n");
		Sleep(10000);
		goto castar;
	}

	/* Minha mana >= MaxMana*/
	if(*Mana >= *MaxMana) 
	{
		Sleep(3000);
		ImportFunc->Target_WaitTarget(Serial);
		for(x=0;x<SpellTimes;x++)
		{
			/* Castar eb */
			ImportFunc->Target_WaitTarget(Serial);

			do{
				ImportFunc->CastSpell("Energy Bolt");
				Sleep(1000);
			} while(*ImportFunc->TargetSent==0); /*Esperar receber um pacote target*/
			*ImportFunc->TargetSent = 0;

			/* Se alguem mandou parar.. */
			if(ImportFunc->IsInJournal("para tudo", -2) != -1)
			{
				ImportFunc->LogPrint(3, "[PLUGIN] Plugin parado\n");
				goto tryagain;
			}

			Sleep(5000);
		}
	}

	ImportFunc->LogPrint(1, "[PLUGIN] waiting mana %d\n", *MaxMana);

	/* Meditar */
	do{
		ImportFunc->UseSkill("Meditation");
		Sleep(2000);
	} while(*Mana < *MaxMana);

	Sleep(1500);
	goto castar;

	ImportFunc->CmdThreadExit(CI);
}

void Command_Anatomy(CmdInfoTag *CI)
{
	unsigned int SerialDoNpc;
	if(CI->Argc != 2)
	{
		ImportFunc->LogPrint(1, "[PLUGIN] Uso: %s serial\n", CI->Args[0]);
		ImportFunc->CmdThreadExit(CI);
		return;
	}

	/* Serial do npc para macroar */
	SerialDoNpc = ImportFunc->ArgToInt(CI->Args[1]);
	Sleep(10000);
	ImportFunc->LogPrint(3, "[PLUGIN] Plugin anatomy loaded!\n");
	ImportFunc->LogPrint(3, "[PLUGIN] Serial: %#x\n", SerialDoNpc);

AnatoLoop:
	/* Setar target no npc */
	ImportFunc->Target_WaitTarget(SerialDoNpc);
	do{
		ImportFunc->UseSkill("Anatomy");
		Sleep(1000);
	} while(!*ImportFunc->TargetSent);
	*ImportFunc->TargetSent = 0;

	Sleep(5000);
	goto AnatoLoop;

	ImportFunc->CmdThreadExit(CI);
}



void Command_Eval(CmdInfoTag *CI)
{
	unsigned int SerialDoNpc;

	if(CI->Argc != 2)
	{
		ImportFunc->LogPrint(1, "[PLUGIN] Uso: %s serial\n", CI->Args[0]);
		ImportFunc->CmdThreadExit(CI);
		return;
	}
	/* Serial do npc para macroar */
	SerialDoNpc = ImportFunc->ArgToInt(CI->Args[1]);
	Sleep(10000);
	ImportFunc->LogPrint(3, "[PLUGIN] Plugin eval loaded!\n");
	ImportFunc->LogPrint(3, "[PLUGIN] Serial: %#x\n", SerialDoNpc);
evalloop:
	/* Setar target no npc */
	ImportFunc->Target_WaitTarget(SerialDoNpc);
	do{
		ImportFunc->UseSkill("Eval");
		Sleep(1000);
	} while(!*ImportFunc->TargetSent);
	*ImportFunc->TargetSent = 0;
	Sleep(5000);
	goto evalloop;
	ImportFunc->CmdThreadExit(CI);
}

void Command_Hide(CmdInfoTag *CI)
{
	ImportFunc->LogPrint(3, "[PLUGIN] Macro de hide loaded\n");
HidingLoop:
	ImportFunc->UseSkill("Hiding");
	Sleep(5000);
	goto HidingLoop;
	ImportFunc->CmdThreadExit(CI);
}

void Command_TailorAll(CmdInfoTag *CI)
{
	const unsigned short	KitG=0x0, AlgodaoG=0x0,
							LinhaG=0x0, BolaG=0x0,
							TecidoG=0x0, RoloG=0x0,
							BandG=0x0;

	unsigned short Qtd, ObjG;

	unsigned int TailorSerial, SecureSerial;
	unsigned char *Choice1, *Choice2;

	GameObject *TailorObj;

	if(CI->Argc != 5)
	{
		ImportFunc->LogPrint(1, "[PLUGIN] Uso: %s choice1 choice2 objgrafico secureserial\n", CI->Args[0]);
		ImportFunc->CmdThreadExit(CI);
		return;
	}

	Choice1 = CI->Args[1];
	Choice2 = CI->Args[2];
	ObjG = (unsigned short)ImportFunc->ArgToInt(CI->Args[3]);
	SecureSerial = ImportFunc->ArgToInt(CI->Args[4]);

	ImportFunc->LogPrint(3, "[PLUGIN] Tailor loaded, c1:[%s] c2:[%s] ObjG:[0x%X] SSerial:[0x%X]\n",
		Choice1, Choice2, ObjG, SecureSerial);

	while(1)
	{
		ImportFunc->LogPrint(3, "[PLUGIN] Transformando todos os itens tailor dentro da bag\n");
		
		while(ImportFunc->FindItemInContainer(AlgodaoG, INVALID_COLOR, *ImportFunc->mbackpack) != INVALID_SERIAL)
		{
			ImportFunc->UseObjectType(AlgodaoG);
			Sleep(2000);
		}
		
		while(ImportFunc->FindItemInContainer(BolaG, INVALID_COLOR, *ImportFunc->mbackpack)!=INVALID_SERIAL) 
		{
			ImportFunc->UseObjectType(BolaG);
			Sleep(2000);
		} 
		
		while(ImportFunc->FindItemInContainer(LinhaG, INVALID_COLOR, *ImportFunc->mbackpack)!=INVALID_SERIAL) 
		{ 
			ImportFunc->UseObjectType(LinhaG);
			Sleep(2000);
		} 
		
		ImportFunc->LogPrint(3, "[PLUGIN] Nao ha mais oque transformar\n"); 
		
		ImportFunc->LogPrint(3, "[PLUGIN] Procurando rolo/tecido\n");
		
		while( ((TailorSerial = ImportFunc->FindItemInContainer(RoloG, INVALID_COLOR, *ImportFunc->mbackpack))!=INVALID_SERIAL) || ((TailorSerial = ImportFunc->FindItemInContainer(TecidoG, INVALID_COLOR, *ImportFunc->mbackpack))!=INVALID_SERIAL) )
		{
			if(ImportFunc->FindItemInContainer(ObjG, INVALID_COLOR, *ImportFunc->mbackpack) != INVALID_SERIAL)
			{
				ImportFunc->LogPrint(3, "[PLUGIN] Movendo todos os itens criados pro chao\n");
				ImportFunc->MassMoveGraphic(*ImportFunc->mbackpack, 0xFFFFFFFF, ObjG);
				Sleep(10000);
			}

			TailorObj = ImportFunc->FindSerial(TailorSerial);
			
			ImportFunc->LogPrint(3, "[PLUGIN] Tailorando s:[0x%X] g:[0x%X] qtd:[0x%X] n:[%s]\n",
				TailorObj->Serial, TailorObj->Graphic, TailorObj->Quantity, TailorObj->Name);
			
			do{
				ImportFunc->LogPrint(1, "[PLUGIN] Dando 2 cliques no kit\n");
				ImportFunc->UseObjectType(KitG);
				Sleep(2000);
			} while(*ImportFunc->WMenu==0);
			*ImportFunc->WMenu = 0;
			
			ImportFunc->LogPrint(1, "[PLUGIN] Enviando escolha [%s]\n", Choice1);
			/* Choice 1 */
			ImportFunc->Send_Choice(Choice1);
			
			while(*ImportFunc->WMenu==0) Sleep(1000);
			*ImportFunc->WMenu = 0;
			
			ImportFunc->LogPrint(1, "[PLUGIN] Enviando escolha [%s]\n", Choice2);
			/* Choice 2 */
			ImportFunc->Send_Choice(Choice2);
			
			ImportFunc->LogPrint(1, "[PLUGIN] Selecionando target [0x%X]\n", TailorSerial);
			do{
				ImportFunc->Target_WaitTarget(TailorSerial);
				Sleep(2000);
			} while(*ImportFunc->TargetSent==0);
			*ImportFunc->TargetSent = 0;
			
			/* TextReply */
			while(*ImportFunc->WDlg==0) Sleep(1000);
			*ImportFunc->WDlg = 0;
			
			ImportFunc->LogPrint(1, "[PLUGIN] Criando 10 items\n");
			ImportFunc->Send_Reply("10");
			
			ImportFunc->LogPrint(3, "[PLUGIN] Criando itens, aguardar check com bands\n");
			
			Sleep(10000);
			
			/* aguardar terminar de criar os itens, check com bands */
			ImportFunc->Target_WaitTarget(*ImportFunc->mserial);
			do{
				ImportFunc->UseObjectType(BandG);
				Sleep(2000);
			} while(*ImportFunc->TargetSent==0);
			*ImportFunc->TargetSent = 0;
			
			/* itens criados, voltando pro inicio */
		}
		
		ImportFunc->LogPrint(3, "[PLUGIN] Pano/Tecido nao encontrado dentro da bag, abrindo secure\n");
		
		/* abrir secure e procurar itens */

		ImportFunc->UseObject(SecureSerial);
		Sleep(2000);

		if( ((TailorSerial = ImportFunc->FindItemInContainer(AlgodaoG, INVALID_COLOR, SecureSerial)) == INVALID_SERIAL) &&
			((TailorSerial = ImportFunc->FindItemInContainer(LinhaG, INVALID_COLOR, SecureSerial)) == INVALID_SERIAL) &&
			((TailorSerial = ImportFunc->FindItemInContainer(BolaG, INVALID_COLOR, SecureSerial)) == INVALID_SERIAL) &&
			((TailorSerial = ImportFunc->FindItemInContainer(TecidoG, INVALID_COLOR, SecureSerial)) == INVALID_SERIAL) &&
			((TailorSerial = ImportFunc->FindItemInContainer(RoloG, INVALID_COLOR, SecureSerial)) == INVALID_SERIAL) )
		{
			ImportFunc->LogPrint(3, "[PLUGIN] Nao foi encontrado mais itens taylor dentro do secure\n");
			continue;
		}

		TailorObj = ImportFunc->FindSerial(TailorSerial);

		Qtd = 200;
		if(TailorObj->Graphic == AlgodaoG)
			Qtd = 200;
		else if(TailorObj->Graphic == LinhaG)
			Qtd = 600;
		else if(TailorObj->Graphic == BolaG)
			Qtd = 1200;
		else if(TailorObj->Graphic == TecidoG)
			Qtd = 6000;
		else if(TailorObj->Graphic == RoloG)
			Qtd = 120;
		
		ImportFunc->MoveToContainer(TailorObj->Serial, Qtd, *ImportFunc->mbackpack);
		Sleep(5000);
	}

	ImportFunc->CmdThreadExit(CI);
}

void Command_Tailor(CmdInfoTag *CI)
{
	unsigned short Kit=0;
	unsigned int Tecido;
	unsigned short Roupa;

	if(CI->Argc != 6 && CI->Argc != 7)
	{
		ImportFunc->LogPrint(3, "[PLUGIN] Uso: %s grapkit sertecido choice1 choice2 textreply [grafico_vender]\n", CI->Args[0]);
		ImportFunc->CmdThreadExit(CI);
		return;
	}
	Kit = (unsigned short)ImportFunc->ArgToInt(CI->Args[1]);
	Tecido = ImportFunc->ArgToInt(CI->Args[2]);

	ImportFunc->LogPrint(3, "[PLUGIN] Macro de tailor loaded\n");
	
	if(CI->Argc == 7)
	{
		Roupa = (unsigned short)ImportFunc->ArgToInt(CI->Args[6]);
		ImportFunc->LogPrint(3, "[PLUGIN] Vender[0x%X]\n", Roupa);
	}

	ImportFunc->LogPrint(3, "[PLUGIN] Kit[%#X] Tecido[%#X] C1[%s] C2[%s] Qtd[%s]\n",
		Kit, Tecido, CI->Args[3], CI->Args[4], CI->Args[5]);

tailorloop:
	if(CI->Argc == 7 && ImportFunc->GetQuantity(*ImportFunc->mbackpack, Roupa) > 10)
	{
		/* vender items */
		ImportFunc->SellItems(Roupa);

		ImportFunc->Say("vender");
	}

	do{
		ImportFunc->UseObjectType(Kit);
		Sleep(2000);
	} while(*ImportFunc->WMenu==0);
	*ImportFunc->WMenu = 0;
	/* Choice 1 */
	ImportFunc->Send_Choice(CI->Args[3]);
	/* Choice 2 */
	while(*ImportFunc->WMenu==0) Sleep(1000);
	*ImportFunc->WMenu = 0;
	ImportFunc->Send_Choice(CI->Args[4]);
	do{
		ImportFunc->Target_WaitTarget(Tecido);
		Sleep(2000);
	} while(*ImportFunc->TargetSent==0);
	*ImportFunc->TargetSent = 0;
	/* TextReply */
	while(*ImportFunc->WDlg==0) Sleep(1000);
	*ImportFunc->WDlg = 0;
	ImportFunc->Send_Reply(CI->Args[5]);
	ImportFunc->LogPrint(3, "[PLUGIN] Criando itens, aguardar 10s\n");
	Sleep(10000);
	goto tailorloop;
	ImportFunc->CmdThreadExit(CI);
}

void Command_Bs(CmdInfoTag *CI)
{
	unsigned short Martelo=0;
	unsigned int Serial;
	unsigned int Lings;
	int FlagOk=0;
	GameObject *obj;
	if(CI->Argc != 7)
	{
		ImportFunc->LogPrint(3, "[PLUGIN] Uso: %s grapmartelo serlings choice1 choice2 choice3 textreply\n", CI->Args[0]);
		ImportFunc->CmdThreadExit(CI);
		return;
	}
	/* Grafico do martelo */
	Martelo = (unsigned short)ImportFunc->ArgToInt(CI->Args[1]);
	/* Serial dos lings */
	Lings = ImportFunc->ArgToInt(CI->Args[2]);

	ImportFunc->LogPrint(3, "[PLUGIN] Macro de bs loaded\n");
	ImportFunc->LogPrint(3, "[PLUGIN] Mart[%#X] Lin[%#X] C1[%s] C2[%s] C3[%s] Qtd[%s]\n",
		Martelo, Lings, CI->Args[3], CI->Args[4], CI->Args[5], CI->Args[6]);

bsloop:
	do{
		ImportFunc->Target_WaitTarget(Lings);
		FlagOk = 0;
		/* Verificar se ja tem algum item no layer do martelo */
		if((Serial=ImportFunc->GetItemInLayer(*ImportFunc->mserial, 1))!=INVALID_SERIAL)
		{
			/* Se nao for um martelo, jogar o item na mochila */
			obj=ImportFunc->FindSerial(Serial);
			if(obj->Graphic!=Martelo)
				ImportFunc->MoveToContainer(Serial, 1, *ImportFunc->mbackpack);
			else
				FlagOk = 1;
		}
		/* Se ja houver um martelo na mao, usa-lo */
		if(FlagOk)
			ImportFunc->UseObject(Serial);
		/* Caso nao haja, procurar na bag e usar */
		else
			ImportFunc->UseObjectType(Martelo);
		Sleep(5000);
	} while(*ImportFunc->TargetSent==0);
	*ImportFunc->TargetSent = 0;
	/* Enviar choice 1 */
	while(*ImportFunc->WMenu==0) Sleep(1000);
	*ImportFunc->WMenu = 0;
	ImportFunc->Send_Choice(CI->Args[3]);
	/* Enviar choice 2 */
	while(*ImportFunc->WMenu==0) Sleep(1000);
	*ImportFunc->WMenu = 0;
	ImportFunc->Send_Choice(CI->Args[4]);
	/* Enviar choice 3 */
	while(*ImportFunc->WMenu==0) Sleep(1000);
	*ImportFunc->WMenu = 0;
	ImportFunc->Send_Choice(CI->Args[5]);
	/* TextReply = digitar 10 */
	while(*ImportFunc->WDlg==0) Sleep(1000);
	*ImportFunc->WDlg = 0;
	/* Enviar TextReply */
	ImportFunc->Send_Reply(CI->Args[6]);

	ImportFunc->LogPrint(3, "[PLUGIN] Criando itens, aguardar 10s\n");
	Sleep(10000);
	goto bsloop;
	ImportFunc->CmdThreadExit(CI);
}

/* Isso era pra ser um check para mana 
 * mas, apesar dos esforcos, nao tem como
 * fazer alguns macros de cast baseados na
 * mana, pois voce acaba caindo em outro buraco
 */
unsigned short LastMana = 0;
unsigned int WaitManaUpdate = 0;

int uo_handle_updatemana(unsigned char *buf, int len)
{
	unsigned int Serial = ImportFunc->unpack32(buf+1);
	unsigned short Mana = ImportFunc->unpack16(buf+5);

	/* Se o serial updated nao for = ao meu retornar */
	if(Serial != *ImportFunc->mserial)
		return 0;

	/* Se a mana abaixou com relacao ao ultimo update, eh pq castou?!?! algo */
	if(LastMana > Mana)
		WaitManaUpdate = 0;

	/* Setar o ultimo update mana para a mana atual */
	LastMana = Mana;
	return 0;
}

void Command_MageMacro(CmdInfoTag *CI)
{	/* Magias de 30 a 100 */
	typedef struct
	{
		unsigned char *SpellName;
		unsigned char Mana;
	} SpellsTag;
	/*	Tabela 
	 *	 0 a  47 = "Wall of Stone"
	 *	47 a  61 = "Greater Heal"
	 *	61 a  70 = "Invisibility"
	 *	70 a  80 = "Polymorph"
	 *	80 a 100 = "Air Elemental"
	 */
	SpellsTag Spells[]=
	{
		{"Greater Heal",	11},
		{"Invisibility",	20},
		{"Polymorph",		40},
		{"Air Elemental",	50},
		{"Poison",			 9},
		{"Wall of Stone",	 9}
	};
	/* Mana atual e MaxMana */
	unsigned short *MyMana=0,*MaxMana;
	/* Bands */
	unsigned short Bands = 0x0E21;
	/* Skill atual */
	unsigned int Magery=0;
	GameObject *Obj=ImportFunc->FindSerial(*ImportFunc->mserial);

	Sleep(5000);

	/* Mana atual, maximo de mana */
	MyMana = &Obj->Character->Mana;
	MaxMana = &Obj->Character->MaxMana;
	Magery = ImportFunc->ReqSkillValue("Magery");

	ImportFunc->LogPrint(1, "[PLUGIN] Macro de mage loaded\n");
	ImportFunc->LogPrint(1, "[PLUGIN] Mana: %d/%d Skills: %.1f\n",
		*MyMana, *MaxMana, (double)Magery/10);


mageloop:
	/* Pegar o valor atual da skill Magery */
	do {
		Magery = ImportFunc->ReqSkillValue("Magery");
		Sleep(2000);
	} while(Magery==0);

	/* Meditar ate full mana */
	while(*MyMana!=*MaxMana)
	{
		ImportFunc->UseSkill("Meditation");
		Sleep(5000);
	}

	if(Magery <= 470)
	{
		/* Antigo macro de poison + heal
		do{
			ImportFunc->Target_WaitTarget(*ImportFunc->mserial);
			do{
				ImportFunc->CastSpell(Spells[4].SpellName);
				Sleep(2000);
			} while(*ImportFunc->TargetSent==0);
			*ImportFunc->TargetSent = 0;
			Sleep(2000);
		} while(*MyMana >= Spells[4].Mana);

		do{
			ImportFunc->Target_WaitTarget(*ImportFunc->mserial);
			do{
				ImportFunc->UseObjectType(Bands);
				Sleep(2000);
			} while(*ImportFunc->TargetSent==0);
			*ImportFunc->TargetSent = 0;
			Sleep(2000);
		} while(Obj->Character->HitPoints != Obj->Character->MaxHitPoints);*/

		do{
			ImportFunc->Target_WaitTarget(*ImportFunc->mserial);
			do{
				ImportFunc->CastSpell(Spells[5].SpellName);
				Sleep(2000);
			} while(*ImportFunc->TargetSent==0);
			*ImportFunc->TargetSent = 0;
			Sleep(2000);
		} while(*MyMana >= Spells[5].Mana);

		goto magemedit;
	}

	else if(Magery > 470 && Magery <= 610)
	{
		do{
			ImportFunc->Target_WaitTarget(*ImportFunc->mserial);
			do{
				ImportFunc->CastSpell(Spells[0].SpellName);
				Sleep(2000);
			} while(*ImportFunc->TargetSent==0);
			*ImportFunc->TargetSent = 0;
			Sleep(2000);
		} while(*MyMana >= Spells[0].Mana);
		goto magemedit;
	}
	else if(Magery > 610 && Magery <= 700)
	{
		do{
			ImportFunc->Target_WaitTarget(*ImportFunc->mserial);
			do{
				ImportFunc->CastSpell(Spells[1].SpellName);
				Sleep(2000);
			} while(*ImportFunc->TargetSent==0);
			*ImportFunc->TargetSent = 0;
			Sleep(2000);
		} while(*MyMana >= Spells[1].Mana);
		goto magemedit;
	}
	else if(Magery > 700 && Magery <= 800)
	{
		TryFindBandAgain:
		if(ImportFunc->FindItemInContainer(0x0E21, INVALID_COLOR, *ImportFunc->mbackpack)==INVALID_SERIAL)
		{
			ImportFunc->LogPrint(3, "[PLUGIN] Voce precisa ter pelo menos uma band dentro da bag para usar esse macro\n");
			Sleep(10000);
			goto TryFindBandAgain;
		}

		do{
			ImportFunc->CastSpell(Spells[2].SpellName);
			/* Pequeno check para nao gastar mana */
			ImportFunc->Target_WaitTarget(*ImportFunc->mserial);
			do{
				ImportFunc->UseObjectType(Bands);
				Sleep(1000);
			} while(*ImportFunc->TargetSent==0);
			*ImportFunc->TargetSent = 0;
		} while(*MyMana >= Spells[2].Mana);
		goto magemedit;
	}
	else if(Magery > 800 && Magery < 1000)
	{
		do {
			ImportFunc->Target_WaitTarget(*ImportFunc->mserial);
			do{
				ImportFunc->CastSpell(Spells[3].SpellName);
				Sleep(2000);
			} while(*ImportFunc->TargetSent==0);
			*ImportFunc->TargetSent = 0;
			/* Pequeno check para nao gastar mana */
			ImportFunc->Target_WaitTarget(*ImportFunc->mserial);
			do{
				ImportFunc->UseObjectType(Bands);
				Sleep(1000);
			} while(*ImportFunc->TargetSent==0);
			*ImportFunc->TargetSent = 0;
			Sleep(1000);

			/* Se tiver mais de 90 de mana, castar 2 elementais */
			if(*MaxMana >= 90)
			{
				/* Esperar subir mana ate 50 */
				while(*MyMana<Spells[3].Mana) Sleep(1000);

				ImportFunc->Target_WaitTarget(*ImportFunc->mserial);
				do{
					ImportFunc->CastSpell(Spells[3].SpellName);
					Sleep(2000);
				} while(*ImportFunc->TargetSent==0);
				*ImportFunc->TargetSent = 0;

				/* Pequeno check para nao gastar mana */
				ImportFunc->Target_WaitTarget(*ImportFunc->mserial);
				do{
					ImportFunc->UseObjectType(Bands);
					Sleep(1000);
				} while(*ImportFunc->TargetSent==0);
				*ImportFunc->TargetSent = 0;
				Sleep(1000);

				/* Meditar */
				break;
			}

		} while (*MyMana >= Spells[3].Mana); 
		goto magemedit;
	}
	/* Se gmzou, parar o macro */
	else if(Magery == 1000)
	{
		ImportFunc->LogPrint(1, "[PLUGIN] Macro finalizado, mage gmzado\n");
		ImportFunc->CmdThreadExit(CI);
		return;
	}
magemedit:
	ImportFunc->LogPrint(1, "[PLUGIN] Meditando\n");
	/* Meditar */
	do{
		ImportFunc->UseSkill("Meditation");
		Sleep(5000);
	} while(*MyMana!=*MaxMana);
	ImportFunc->LogPrint(1, "[PLUGIN] Mana cheia, repetindo o macro\n");

	goto mageloop;
	ImportFunc->CmdThreadExit(CI);
}


int AlchGetXY(unsigned int *x, unsigned int *y)
{
	/* Pagina esquerda */
	if(*x == 180)
	{
		*x=290;
		*y=275;
	}

	/* Pagina direita */
	else if(*x == 340)
	{
		*x=445;
		*y=275;
	}
	/* Nao achei a pagina */
	else
		return 0;
	return 1;
}

int uo_handle_update_player(char *buf, int len)
{
	unsigned int serial = ImportFunc->unpack32(buf+1);
	if(serial == *ImportFunc->mserial && buf[10]&4)
	{
		ImportFunc->LogPrint(3, "[ALCH] Voce foi poisonado\n");
		ImportFunc->CloseShardSocket();
	}
	return 0;
}

unsigned short LastHp = 0;

int uo_handle_update_hp(char *buf, int len)
{
	unsigned int serial = ImportFunc->unpack32(buf+1);
	unsigned short MaxHp = ImportFunc->unpack16(buf+5);
	unsigned short MyHp = ImportFunc->unpack16(buf+7);
	GameObject *Eu;

	if(serial != *ImportFunc->mserial)
		return 0;

	if(!LastHp)
	{
		if((Eu = ImportFunc->FindSerial(*ImportFunc->mserial))!=0)
			LastHp = Eu->Character->HitPoints;
		return 0;
	}

	if(MyHp < LastHp)
	{
		ImportFunc->LogPrint(3, "[ALCH] A poção explodiu, usando pot de gh\n");
		ImportFunc->UseObjectType(0x0f0c);
	}

	LastHp = MyHp;

	return 0;
}

void Command_AlchMacro(CmdInfoTag *CI)
{
	unsigned short BookAlch=0x1C13;
	unsigned int KegSerial=0;
	unsigned int x=0,y=0,Page=0,ButtonId=0;
	int count = 0;
	GameObject *Eu;

	if(CI->Argc != 3)
	{
		ImportFunc->LogPrint(1, "[PLUGIN] Uso: %s 'tipo de potion' serial_da_keg\n", CI->Args[0]);
		ImportFunc->CmdThreadExit(CI);
		return;
	}

	Sleep(5000);

	ImportFunc->AddPacketHandler(0x20, (void *)uo_handle_update_player);
	ImportFunc->AddPacketHandler(0xa1, (void *)uo_handle_update_hp);

	if((Eu = ImportFunc->FindSerial(*ImportFunc->mserial))!=0)
	{
		LastHp = Eu->Character->HitPoints;
		if(Eu->Character->HitPoints!=Eu->Character->MaxHitPoints)
		{
			ImportFunc->LogPrint(3, "[ALCH] Usando pot de gh para encher life\n");
			ImportFunc->UseObjectType(0x0f0c);
		}
	}

	/* Utilizar a keg para guardar as pocoes */
	KegSerial = ImportFunc->ArgToInt(CI->Args[2]);

	ImportFunc->LogPrint(3, "[PLUGIN] Alchemy load, potion[%s] keg[%#x]\n", CI->Args[1], KegSerial);
AlchLoop:
	
	*ImportFunc->GumpSent = 0;

	/* Dizer ao core para gravar os gumps enviados */
	ImportFunc->Wait_Gump();

	/* Unset catchbag */
	ImportFunc->UnSetCatchBag();

	/* Dois cliques no book */
	do{
		ImportFunc->UseObjectType(BookAlch);
		Sleep(2000);
	} while(*ImportFunc->GumpSent==0);
	*ImportFunc->GumpSent = 0;

	/* Procurar pelo CI->Args[1] no gump */
	if(ImportFunc->GumpSearchText(CI->Args[1], &x,&y,&Page)==0)
	{
		ImportFunc->LogPrint(3, "[PLUGIN] Nao encontrado texto '%s'\n", CI->Args[1]);
		ImportFunc->FreeGump();
		ImportFunc->CmdThreadExit(CI);
		return;
	}
	/* Verificar x,y dos textos */
	if(!AlchGetXY(&x, &y))
	{
		ImportFunc->LogPrint(3, "[PLUGIN] x[%d] y[%d] invalidos\n", x, y);
		ImportFunc->FreeGump();
		ImportFunc->CmdThreadExit(CI);
		return;
	}

	/* Pegar id do botao correspondente */
	if((ButtonId = ImportFunc->GumpSearchXY(x, y, Page))==0)
	{
		ImportFunc->LogPrint(3, "[PLUGIN] x[%d] y[%d] page[%d] nao bateram\n", x, y, Page);
		ImportFunc->FreeGump();
		ImportFunc->CmdThreadExit(CI);
		return;
	}

	/* Colocar pocoes na keg */
	ImportFunc->SetCatchBag(KegSerial);

	/* Send button id */
	ImportFunc->SendGumpChoice(ButtonId);

	/* TextReply */
	while(*ImportFunc->WDlg==0) Sleep(1000);
	*ImportFunc->WDlg = 0;
	/* 10 pocoes */
	ImportFunc->Send_Reply("10");
	
	ImportFunc->LogPrint(3, "[PLUGIN] Comecando a fazer as pots\n");

	/* Liberar o espaco alocado pelo gump */
	ImportFunc->FreeGump();

	count = 80000;

	ImportFunc->CleanJournal();

	do{
		Sleep(2000);
		count -= 2000;
		if(ImportFunc->IsInJournal("mistura fica instavel e explode", JOURNAL_ANY) != -1)
			goto AlchLoop;
		if(ImportFunc->IsInJournal("nao tem reagentes sufientes", JOURNAL_ANY) != -1)
		{
			ImportFunc->LogPrint(3, "[ALCH] Cabou reagentes\n");
			ImportFunc->CmdThreadExit(CI);
			return;
		}
	} while(count > 0);

	/* Aguardar enquanto as pocoes estao sendo feitas */

	goto AlchLoop;
	ImportFunc->CmdThreadExit(CI);

}

void Command_InscrMacro(CmdInfoTag *CI)
{
	/* Graphic dos pergaminhos na bag */
	unsigned short Scrolls = 0x0E34;
	/* Circulo da magia com relacao a skill */
	unsigned char Circulo = 0;

	unsigned short Bands = 0x0E21;

	typedef struct {
		char *SpellName;
		unsigned char *Circulo;
		unsigned char Mana;
	} InscrType;

	InscrType Inscr[] = {
		{"Cure",				"2",  6},
		{"Teleport",			"3",  9},
		{"Recall",				"4", 11},
		{"Magic Reflection",	"5", 15},
		{"Mark",				"6", 20},
		{"Gate Travel",			"7", 40},
		{"Resurrection",		"8", 50}
	};
	unsigned int Inscription = 0;

	unsigned short *MyMana = 0, *MaxMana = 0;
	GameObject *Obj;
    Sleep(2000);

    while((Obj=ImportFunc->FindSerial(*ImportFunc->mserial))==0) Sleep(1000);

	/* Pegar mana e maxmana */
	MyMana = &Obj->Character->Mana;
	MaxMana = &Obj->Character->MaxMana;
	Inscription = ImportFunc->ReqSkillValue("Inscript");

	ImportFunc->LogPrint(1, "[PLUGIN] Inscript macro loaded!\n");
	ImportFunc->LogPrint(1, "[PLUGIN] Mana[%d/%d] Skill[%d]\n",	*MyMana, *MaxMana, Inscription);

InscrLoop:

	/* Verificar se eu ainda tenho pergaminhos em branco */
	if(ImportFunc->FindItemInContainer(Scrolls, INVALID_COLOR, *ImportFunc->mbackpack) == INVALID_SERIAL)
	{
		ImportFunc->LogPrint(1, "[PLUGIN] Acabou os pergaminhos em branco\n");
		ImportFunc->CmdThreadExit(CI);
		return;
	}

	/* Pegar o valor atual da skill Inscript */
	do {
		Inscription = ImportFunc->ReqSkillValue("Inscript");
		Sleep(2000);
	} while(Inscription==0);

	/* Meditar ate full mana */
	while(*MyMana!=*MaxMana)
	{
		ImportFunc->UseSkill("Meditation");
		Sleep(5000);
	}

	if(Inscription <= 400)
		Circulo = 0;
	else if(Inscription > 400 && Inscription <= 500)
		Circulo = 1;
	else if(Inscription > 500 && Inscription <= 600)
		Circulo = 2;
	else if(Inscription > 600 && Inscription <= 700)
		Circulo = 3;
	else if(Inscription > 700 && Inscription <= 800)
		Circulo = 4;
	else if(Inscription > 800 && Inscription <= 900)
		Circulo = 5;
	else if(Inscription > 900 && Inscription < 1000)
		Circulo = 6;
	else if(Inscription == 1000)
	{
		ImportFunc->LogPrint(1, "[PLUGIN] Inscription gmzado\n");
		ImportFunc->CmdThreadExit(CI);
		return;
	}


	do{
		do {
			ImportFunc->UseSkill("Inscript");
			Sleep(1000);
		} while(*ImportFunc->WMenu==0);
		*ImportFunc->WMenu = 0;

		/* Enviar circulo */
		ImportFunc->Send_Choice(Inscr[Circulo].Circulo);

		/* Enviar nome da magia */
		while(*ImportFunc->WMenu==0) Sleep(1000);
		*ImportFunc->WMenu = 0;
		ImportFunc->Send_Choice(Inscr[Circulo].SpellName);

		/* Aguardar pelo dialog de texto */
		while(*ImportFunc->WDlg==0) Sleep(1000);
		*ImportFunc->WDlg = 0;
		/* Enviar resposta = 10 */
		ImportFunc->Send_Reply("10");
		ImportFunc->LogPrint(1, "[PLUGIN] Criando scrolls\n");
		Sleep(3000);
		/* Pequeno macro para verificar qdo ele acabou de criar os scrolls */
		ImportFunc->Target_WaitTarget(*ImportFunc->mserial);
		do{
			ImportFunc->UseObjectType(Bands);
			Sleep(1000);
		} while(*ImportFunc->TargetSent==0);
		*ImportFunc->TargetSent = 0;

		ImportFunc->LogPrint(1, "[PLUGIN] Acabou de criar os scrolls\n");
	} while(*MyMana >= Inscr[Circulo].Mana);

	/* Meditar */
	ImportFunc->LogPrint(1, "[PLUGIN] Meditando\n");
	do {
		ImportFunc->UseSkill("Meditation");
		Sleep(2000);
	} while(*MyMana != *MaxMana);

	goto InscrLoop;
	ImportFunc->CmdThreadExit(CI);
}

void Command_ArmsLore(CmdInfoTag *CI) 
{ 
	unsigned int ArmSerial;

	if(CI->Argc != 2)
	{
		ImportFunc->LogPrint(3, "[PLUGIN] Uso: %s serial\n", CI->Args[0]);
		ImportFunc->CmdThreadExit(CI);
        return;
	}

	/* Setar serial da arma */
	ArmSerial = ImportFunc->ArgToInt(CI->Args[1]);

	Sleep(10000);

	ImportFunc->LogPrint(3, "[PLUGIN] Arms Lore loaded!\n"); 
	ImportFunc->LogPrint(3, "[PLUGIN] Item Serial: [0x%X]\n", ArmSerial);

ArmsLoreLoop: 
	ImportFunc->Target_WaitTarget(ArmSerial);
	do {
		ImportFunc->UseSkill("Arms Lore"); 
		Sleep(2000);
	} while (!*ImportFunc->TargetSent);
	*ImportFunc->TargetSent = 0; 
	Sleep(5000); 
	goto ArmsLoreLoop; 
	ImportFunc->CmdThreadExit(CI);
}

int uo_handle_menu(unsigned char *buf, int len)
{
	unsigned char Question[] = "O que voce deseja rastrear";
	MenuOptionsTag *MenuOptions = (MenuOptionsTag*)ImportFunc->MenuOptions;

	if(strstr(MenuOptions->m_question, Question))
		ImportFunc->Send_Choice("Animais");
	else
		ImportFunc->Send_Choice(NULL);

	return 0;
}


void Command_Track(CmdInfoTag *CI)
{
	Sleep(5000);
	ImportFunc->LogPrint(3, "[PLUGIN] Macro de track loaded\n");

	ImportFunc->AddPacketHandler(0x7C, (void *)uo_handle_menu);

	do {
		ImportFunc->UseSkill("Tracking");
		Sleep(5000);
	} while(1);

	ImportFunc->CmdThreadExit(CI);
}

void Command_TransTailor(CmdInfoTag *CI) 
{ 
	unsigned short Algodao=0X1A9C; 
	unsigned short Boladela=0XE1D; 
	unsigned short Linhadecozer=0x0FA0;

	ImportFunc->LogPrint(3, "[PLUGIN] Plugin de transformar tailor loaded\n");
	Sleep(10000);

AlgodaoLoop: 
	if(ImportFunc->FindItemInContainer(Algodao, INVALID_COLOR, *ImportFunc->mbackpack) != INVALID_SERIAL)
	{ 
		ImportFunc->UseObjectType(Algodao);
		Sleep(2000);
		goto AlgodaoLoop; 
	} 

BoladelaLoop: 
	if(ImportFunc->FindItemInContainer(Boladela, INVALID_COLOR, *ImportFunc->mbackpack)!=INVALID_SERIAL) 
	{ 
		ImportFunc->UseObjectType(Boladela);
		Sleep(2000);
		goto BoladelaLoop; 
	} 

LinhadecozerLoop: 
	if(ImportFunc->FindItemInContainer(Linhadecozer, INVALID_COLOR, *ImportFunc->mbackpack)!=INVALID_SERIAL) 
	{ 
		ImportFunc->UseObjectType(Linhadecozer);
		Sleep(2000);
		goto LinhadecozerLoop; 
	} 
	ImportFunc->LogPrint(3, "[PLUGIN] Finalizado: Nao há mais oque transformar\n"); 
	ImportFunc->CmdThreadExit(CI); 
}

void Command_Stealth(CmdInfoTag *CI)
{
	GameObject *Eu=0;
	unsigned short HideSkill;
	
	Sleep(2000);

	/* Pegar meu objeto para utilizar o Obj->Flags */
	if((Eu = ImportFunc->FindSerial(*ImportFunc->mserial))==0)
	{
		ImportFunc->LogPrint(3, "[PLUGIN] Nao foi possivel achar meu serial\n");
		ImportFunc->CmdThreadExit(CI);
		return;
	}

	ImportFunc->LogPrint(3, "[PLUGIN] Macro de stealth iniciado\n");
	
	while(1)
	{
		/* Hidar */
		ImportFunc->UseSkill("Hiding");
		Sleep(10000);

		/* Verificar se ja tenho +80 de hide para usar stealth */
		HideSkill = ImportFunc->ReqSkillValue("Hiding");

		if(HideSkill < 800)
		{
			ImportFunc->LogPrint(1, "[PLUGIN] Voce ainda nao tem(%d) hide sufiente(80) para usar stealth\n",
				ImportFunc->Life(HideSkill, 1000));
			continue;
		}

		/* Se conseguir hidar, usar stealth ate aparecer novamente */
		while(Eu->Flags&0x80)
		{
			ImportFunc->UseSkill("Stealth");
			Sleep(10000);
		}
	}

	ImportFunc->CmdThreadExit(CI);
}

void Command_AnimalLore(CmdInfoTag *CI)
{
	unsigned int SerialDoNpc;

	if(CI->Argc != 2)
	{
		ImportFunc->LogPrint(1, "[PLUGIN] Uso: %s serial\n", CI->Args[0]);
		ImportFunc->CmdThreadExit(CI);
		return;
	}
	/* Serial do npc para macroar */
	SerialDoNpc = ImportFunc->ArgToInt(CI->Args[1]);
	Sleep(10000);

	ImportFunc->LogPrint(3, "[PLUGIN] Plugin animal lore loaded!\n");
	ImportFunc->LogPrint(3, "[PLUGIN] Serial: %#x\n", SerialDoNpc);
	
	while(1)
	{
		/* Setar target no npc */
		ImportFunc->Target_WaitTarget(SerialDoNpc);
		do{
			ImportFunc->UseSkill("Animal Lor");
			Sleep(1000);
		} while(!*ImportFunc->TargetSent);
		*ImportFunc->TargetSent = 0;

		Sleep(5000);
	}
	ImportFunc->CmdThreadExit(CI);
}

void Command_Steal(CmdInfoTag *CI)
{
	unsigned int SerialDoNpc;

	if(CI->Argc != 2)
	{
		ImportFunc->LogPrint(1, "[PLUGIN] Uso: %s serial\n", CI->Args[0]);
		ImportFunc->CmdThreadExit(CI);
		return;
	}
	/* Serial do npc para macroar */
	SerialDoNpc = ImportFunc->ArgToInt(CI->Args[1]);
	Sleep(10000);

	ImportFunc->LogPrint(3, "[PLUGIN] Plugin steal lore loaded!\n");
	ImportFunc->LogPrint(3, "[PLUGIN] Serial: %#x\n", SerialDoNpc);
	
	while(1)
	{
		/* Setar target no npc */
		ImportFunc->Target_WaitTarget(SerialDoNpc);
		do{
			ImportFunc->UseSkill("Steal");
			Sleep(1000);
		} while(!*ImportFunc->TargetSent);
		*ImportFunc->TargetSent = 0;

		Sleep(5000);
	}
	ImportFunc->CmdThreadExit(CI);
}

void Command_DuploClick(CmdInfoTag *CI)
{
	unsigned int SerialDoObj;

	if(CI->Argc != 2)
	{
		ImportFunc->LogPrint(1, "[PLUGIN] Uso: %s serial\n", CI->Args[0]);
		ImportFunc->CmdThreadExit(CI);
		return;
	}
	/* Serial do obj para macroar */
	SerialDoObj = ImportFunc->ArgToInt(CI->Args[1]);
	Sleep(10000);

	ImportFunc->LogPrint(3, "[PLUGIN] Plugin duplo clique loaded!\n");
	ImportFunc->LogPrint(3, "[PLUGIN] Serial: %#x\n", SerialDoObj);
	
	while(1)
	{
		ImportFunc->UseObject(SerialDoObj);
		Sleep(2000);
	}

	ImportFunc->CmdThreadExit(CI);
}

void Command_Snoop(CmdInfoTag *CI)
{
	GameObject *Eu;
	GameObject *Obj;
	unsigned int x, LastSnoopSerial = 0, Count=0;

	ImportFunc->LogPrint(3, "[PLUGIN] Plugin snoop loaded!\n");
	Sleep(2000);

	if((Eu = ImportFunc->FindSerial(*ImportFunc->mserial))==0)
	{
		ImportFunc->LogPrint(3, "[PLUGIN] Não foi possivel localizar meu objeto\n");
		ImportFunc->CmdThreadExit(CI);
		return;
	}

SnoopAction:

	/* checkzinho pro core nao ir cancelando logo o target */
	ImportFunc->Target_RecvTarget();

	/* aguardar ate aparecer o target */
	do {
		ImportFunc->UseSkill("Steal");
		Sleep(2000);
	} while (!*ImportFunc->TargetSent);
	*ImportFunc->TargetSent = 0;

	Count = 0;

SnoopSearch:
	/* procurar por um player ao redor */
	Obj = Eu;
	for(x=0; x < *ImportFunc->QtdObjects; x++)
	{
		/* se for o antigo npc que vc usou snoop, continuar a procurar */
		if(LastSnoopSerial == Obj->Serial)
			continue;

		/* se for um player masc/fem */
		if(Obj->Graphic == 0x190 || Obj->Graphic == 0x191)
		{
			/* verificar se a distancia entre nos 2 for <= 1 */
			if(ImportFunc->GetDistance(Eu->X, Eu->Y, Obj->X, Obj->Y) <= 1)
			{
				/* usar snoop nele */
				ImportFunc->Target_SendTarget(TARGET_OBJECT, Obj->X, Obj->Y, Obj->Z, Obj->Serial, Obj->Graphic);

				/* setar serial como lastsnoop */
				LastSnoopSerial = Obj->Serial;

				/* parar de procurar */
				Sleep(10000);
				goto SnoopAction;
			}
		}
	}

	Count++;

	if(Count >= 6)
	{
		/* cancelar target */
		ImportFunc->Target_SendTarget(TARGET_OBJECT, INVALID_XY, INVALID_XY, 0, 0, 0);
		goto SnoopAction;
	}

	Sleep(3000);
	goto SnoopSearch; 
	
	ImportFunc->CmdThreadExit(CI);
}

void Command_Vender(CmdInfoTag *CI)
{
	unsigned short Graphic;

	if(CI->Argc != 2)
	{
		ImportFunc->LogPrint(3, "[PLUGI] Uso: %s graphic\n", CI->Args[0]);
		ImportFunc->CmdThreadExit(CI);
		return;
	}

	Graphic = (unsigned short)ImportFunc->ArgToInt(CI->Args[1]);

	ImportFunc->LogPrint(3, "[PLUGIN] Vendendo itens com gráfico 0x%X\n", Graphic);

	ImportFunc->SellItems(Graphic);

	ImportFunc->Say("vender");

	do {
		Sleep(2000);
	} while(!*ImportFunc->SellListSent);
	*ImportFunc->SellListSent = 0;


	ImportFunc->LogPrint(3, "[PLUGIN] Items vendidos\n");

	ImportFunc->CmdThreadExit(CI);
}

unsigned int *FundirMemUsed = 0;

void Command_Fundir(CmdInfoTag *CI)
{
	unsigned short Bag1G=0xE76, Bag2G=0xE75;
	unsigned short AlicateG=0xFBB;
	unsigned int AlicateS;

	unsigned int i,x,count;
	GameObject *Obj;

	unsigned int *Items = 0;
	unsigned int ItemsCount;

	if(CI->Argc != 2)
	{
		ImportFunc->LogPrint(3, "[FUNDIR] Uso: %s [alicate_serial]\n", CI->Args[0]);
		ImportFunc->CmdThreadExit(CI);
		return;
	}

	AlicateS = ImportFunc->ArgToInt(CI->Args[1]);

	ImportFunc->LogPrint(3, "[FUNDIR] Macro de fundir items loaded\n");

	Sleep(3000);

	while((Obj=ImportFunc->FindSerial(*ImportFunc->mserial))==0)
	{
		ImportFunc->LogPrint(3, "[FUNDIR] Nao foi possivel localizar meu objecto, tentando novamente em 10s\n");
		Sleep(10000);
	}

	Obj = Obj->Next;

	/* esvaziar as bags q estiverem dentro do backpack */
	ImportFunc->LogPrint(3, "[FUNDIR] Esvaziando as bags dentro do backpack\n");

	count = 0;
	for(x=1; x < *ImportFunc->QtdObjects; x++)
	{
		if((Obj->Graphic == Bag1G || Obj->Graphic == Bag2G) &&
			Obj->IsContainer &&
			Obj->Serial != *ImportFunc->mbackpack &&
			Obj->Container == *ImportFunc->mbackpack &&
			Obj->Serial != AlicateS)
		{
			ImportFunc->LogPrint(3, "[FUNDIR] Esvaziando a bag [0x%X]\n", Obj->Serial);
			i = ImportFunc->MassMove(Obj->Serial, *ImportFunc->mbackpack);
			count += i;
			ImportFunc->LogPrint(3, "[FUNDIR] Retirado %d itens da bag [0x%X]\n", i, Obj->Serial);
		}

		if(Obj->Next)
			Obj = Obj->Next;
		else
			break;
	}

	ImportFunc->LogPrint(3, "[FUNDIR] Esperar 10s para os %d itens serem transferidos\n", count);
	Sleep(10000);

	Obj=ImportFunc->FindSerial(*ImportFunc->mserial);

	Obj->Next;

	ItemsCount = 0;

	for(x=1; x < *ImportFunc->QtdObjects; x++)
	{
		if(Obj->Container == *ImportFunc->mbackpack && !Obj->IsContainer && Obj->Graphic != AlicateG)
		{
			ItemsCount++;
			Items = realloc(Items, sizeof(unsigned int) * ItemsCount);
			Items[ItemsCount - 1] = Obj->Serial;

			//ImportFunc->LogPrint(3, "[FUNDIR] Localizado item [0x%X] [%s]\n", Obj->Serial, Obj->Name);
		}

		if(Obj->Next)
			Obj = Obj->Next;
		else
			break;
	}

	FundirMemUsed = Items;

	ImportFunc->LogPrint(3, "[FUNDIR] Localizados %d items, tentando fundi-los\n", ItemsCount);

	for(x=0; x < ItemsCount; x++)
	{
		ImportFunc->Target_WaitTarget(Items[x]);

		do {
			ImportFunc->UseObject(AlicateS);
			Sleep(2000);
		} while(!*ImportFunc->TargetSent);
		*ImportFunc->TargetSent = 0;
		ImportFunc->LogPrint(1, "[FUNDIR] Falta fundir %d items\n", ItemsCount - x);
	}

	/* liberar espaco alocado */
	if(Items)
		free(Items);

	ImportFunc->CmdThreadExit(CI);
}

void PluginEnd(int ErrNum)
{
	if(FundirMemUsed)
	{
		free(FundirMemUsed);
		FundirMemUsed = 0;
	}
}

int __declspec(dllexport) PluginInit(struct ExpFunc *TmpFunc)
{
	HMODULE hModule=GetModuleHandle(DLLNAME);
	ImportFunc = TmpFunc;
	ImportFunc->PluginAddCommand("teste", (void *)Command_Test, hModule);
	ImportFunc->PluginAddCommand("eval", (void *)Command_Eval, hModule);
	ImportFunc->PluginAddCommand("casteb", (void *)Command_CastEb, hModule);
	ImportFunc->PluginAddCommand("castfs", (void *)Command_CastFS, hModule);
	ImportFunc->PluginAddCommand("heal", (void *)Command_Heal, hModule);
	ImportFunc->PluginAddCommand("combat", (void *)Command_Combat, hModule);
	ImportFunc->PluginAddCommand("hide", (void *)Command_Hide, hModule);
	ImportFunc->PluginAddCommand("bsmacro", (void *)Command_Bs, hModule);
	ImportFunc->PluginAddCommand("tailormacro", (void *)Command_Tailor, hModule);
	ImportFunc->PluginAddCommand("magemacro", (void *)Command_MageMacro, hModule);
	ImportFunc->PluginAddCommand("alchmacro", (void *)Command_AlchMacro, hModule);
	ImportFunc->PluginAddCommand("inscrmacro", (void *)Command_InscrMacro, hModule);
	ImportFunc->PluginAddCommand("armslore", (void *)Command_ArmsLore, hModule);
	ImportFunc->PluginAddCommand("transftailor", (void *)Command_TransTailor, hModule);
	ImportFunc->PluginAddCommand("anatomy", (void *)Command_Anatomy, hModule);
	ImportFunc->PluginAddCommand("track", (void *)Command_Track, hModule);
	ImportFunc->PluginAddCommand("stealth", (void *)Command_Stealth, hModule);
	ImportFunc->PluginAddCommand("animallore", (void *)Command_AnimalLore, hModule);
	ImportFunc->PluginAddCommand("steal", (void *)Command_Steal, hModule);
	ImportFunc->PluginAddCommand("duploclick", (void *)Command_DuploClick, hModule);
	ImportFunc->PluginAddCommand("vender", (void *)Command_Vender, hModule);
	ImportFunc->PluginAddCommand("fundir", (void *)Command_Fundir, hModule);
	ImportFunc->PluginAddCommand("snoop", (void *)Command_Snoop, hModule);
	ImportFunc->PluginAddCommand("tailorall", (void *)Command_TailorAll, hModule);
	ImportFunc->LogPrint(3, "[PLUGIN] Plugins Inicializado\n");
	return 1;
}
