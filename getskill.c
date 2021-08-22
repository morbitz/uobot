#include <string.h>
#include <stdlib.h>
#include <stdio.h>

typedef struct
{
	char *SkillName;
	unsigned int fromvalue;
	unsigned int tovalue;
	unsigned int tempo;
} SkillTag;


char *SkillsName[] = 
{
	{"Alchemy"},
	{"Anatomy"},
	{"Animal Lore"},
	{"Item Identification"},
	{"Arms Lore"},
	{"Parrying"},
	{"Begging"},
	{"Blacksmithy"},
	{"Bowcraft"},
	{"Peacemaking"},
	{"Camping"},
	{"Carpentry"},
	{"Cartography"},
	{"Cooking"},
	{"Detecting Hidden"},
	{"Enticement"},
	{"Evaluating Intelligence"},
	{"Healing"},
	{"Fishing"},
	{"Forensic Evaluation"},
	{"Herding"},
	{"Hiding"},
	{"Provocation"},
	{"Inscription"},
	{"Lockpicking"},
	{"Magery"},
	{"Resisting Spells"},
	{"Tactics"},
	{"Snooping"},
	{"Musicianship"},
	{"Poisoning"},
	{"Archery"},
	{"Spirit Speak"},
	{"Stealing"},
	{"Tailoring"},
	{"Animal Taming"},
	{"Taste Identification"},
	{"Tinkering"},
	{"Tracking"},
	{"Veterinary"},
	{"Swordsmanship"},
	{"Mace Fighting"},
	{"Fencing"},
	{"Wrestling"},
	{"Lumberjacking"},
	{"Mining"},
	{"Meditation"},
	{"Stealth"},
	{"Remove Trap"},
	{"Necromancy"}
};

int cmpfunc_skill(const void *tmpc1, const void *tmpc2)
{
	SkillTag *s1 = (SkillTag *)tmpc1, *s2 = (SkillTag *)tmpc2;
	
	if(s1->fromvalue > s2->fromvalue)
		return 1;
	if(s1->fromvalue < s2->fromvalue)
		return -1;
	if(s1->fromvalue == s2->fromvalue)
	{
		if(s2->tempo > s2->tempo)
			return -1;
		if(s2->tempo < s2->tempo)
			return 1;
		return 0;
	}
	
	return 0;
}

void ordena_skill(SkillTag *skills, unsigned int qtd)
{
	qsort(skills, qtd, sizeof(SkillTag), cmpfunc_skill);
}

void formatvalue(char *str)
{
	char *ptr = str;
	
	while(*str != 0)
	{
		if(*str != ')' && *str != '(' && *str != '.')
		{
			*ptr = *str;
			ptr++;
		}
		str++;
	}
	
	*ptr = 0;
}

char *SkillSearch(char *skill_name)
{
	unsigned int x;
	for(x=0; x < 50; x++)
	{
		if(!strcmp(SkillsName[x], skill_name))
		{
			return SkillsName[x];
		}
	}
	
	return "nao achei";
}

SkillTag *GetSkills(char *file_name, char *skill_name, unsigned int *count)
{
	FILE *fh;
	char linha[201];
	
	char skill[100];
	char *tmpskill;
	
	char fvalue[100];
	unsigned int fromvalue;
	
	char tvalue[100];
	unsigned int tovalue;
	
	char ttempo[50];
	unsigned int tempo;
	
	SkillTag *skills = 0;
	unsigned int skillcount = 0;
	
	*count = 0;
	
	if((fh=fopen(file_name, "r"))==NULL)
	{
		printf("nao foi possivel abrir %s\n", file_name);
		return 0;
	}
	
	linha[0] = 0;
	skill[0] = 0;
	fvalue[0] = 0;
	tvalue[0] = 0;
	ttempo[0] = 0;
	
	while(fgets(linha, 200, fh))
	{
		sscanf(linha, "%*s %*s %*s %*s %[^)] %*s %*s %*s %10s %*s %10s %*s %10s",
			skill, fvalue, tvalue, ttempo);
		
		if(skill[0] != 0 && fvalue[0] != 0 && tvalue[0] != 0 && ttempo[0] != 0)
		{
			formatvalue(skill);
			formatvalue(fvalue);
			formatvalue(tvalue);
			formatvalue(ttempo);
			
			memset(&fromvalue, 0, 4);
			memset(&tovalue, 0, 4);
			tempo = 3000;
			
			sscanf(ttempo, "%u", &tempo);
			sscanf(fvalue, "%u", &fromvalue);
			sscanf(tvalue, "%u", &tovalue);
			
			tmpskill = SkillSearch(skill);
			
			//printf("Skill [%s]\n", tmpskill);
			
			if(fromvalue != 0 && tovalue <= 1000 && tempo < 10000 && strstr(tmpskill, skill_name))
			{
				skillcount++;
				skills = realloc(skills, sizeof(SkillTag) * skillcount);
				skills[skillcount - 1].SkillName = tmpskill;
				skills[skillcount - 1].fromvalue = (unsigned int)(fromvalue);
				skills[skillcount - 1].tovalue = (unsigned int)(tovalue);
				skills[skillcount - 1].tempo = tempo;
				
				//printf("[%s] de [%.1f] para [%.1f] em [%d] segundos\n",
				//	skills[skillcount - 1].SkillName, (float)skills[skillcount - 1].fromvalue/10, (float)skills[skillcount - 1].tovalue/10, skills[skillcount - 1].tempo);
				
				
				*count = *count + 1;
			}
			
		}
		
		linha[0] = 0;
		skill[0] = 0;
		fvalue[0] = 0;
		tvalue[0] = 0;
		ttempo[0] = 0;
	}
	
	fclose(fh);
	
	return skills;
}

int main(int argc, char *argv[])
{
	SkillTag *skills = 0;
	unsigned int qtdade, x;
	unsigned int fromvalue = 0, tovalue=0;
	unsigned int tempomedio, num_encontrada;
	unsigned int maiortempo;
	unsigned int menortempo;
	
	unsigned int lastvalue;
	unsigned int searchvalue;
	unsigned int demora;
	unsigned int i;
	int axei;
	unsigned int qtdskills;
	
	unsigned int lastdez;
	unsigned int demora2;
	
	if(argc != 4)
	{
		printf("Uso: %s [skillname] [from] [where]\n", argv[0]);
		return 0;
	}
	
	fromvalue = atoi(argv[2]);
	tovalue = atoi(argv[3]);
	
	if((skills = GetSkills("logskills.txt", argv[1], &qtdade)) == 0)
	{
		printf("nao foi encontrado nenhuma skill %s\n", argv[1]);
		return 0;
	}
	
	tempomedio = 0;
	num_encontrada = 0;
	maiortempo = 0;
	menortempo = 0;
	
	printf("Encontrado %u entradas da skill \"%s\", comecando a analizar\n", qtdade, skills[0].SkillName);
	
	for(x=0; x < qtdade; x++)
	{
		if(skills[x].fromvalue >= fromvalue && skills[x].tovalue <= tovalue && skills[x].tempo != 0)
		{
			num_encontrada++;
			
			if(maiortempo == 0)
				maiortempo = skills[x].tempo;
			else
				if(skills[x].tempo > maiortempo)
					maiortempo = skills[x].tempo;
				
				if(menortempo == 0)
					menortempo = skills[x].tempo;
				else
					if(skills[x].tempo < menortempo)
						menortempo = skills[x].tempo;
					
					if(tempomedio != 0)
						tempomedio = (tempomedio + skills[x].tempo)/2;
					else
						tempomedio = skills[x].tempo;
		}
	}
	
	ordena_skill(skills, qtdade);
	
	/*for(i=0; i < qtdade; i++)
	{
	printf("[%s] de [%.1f] para [%.1f] em [%d] segundos\n",
	skills[i].SkillName, (float)skills[i].fromvalue/10, (float)skills[i].tovalue/10, skills[i].tempo);
}*/
	
	
	lastvalue = menortempo;
	searchvalue = fromvalue;
	demora = 0;
	
	qtdskills = (unsigned int)(tovalue - fromvalue);
	
	lastdez = fromvalue;
	demora2 = 0;
	
	for(i=0; i < qtdskills; i++)
	{
		axei = 0;
		for(x=0; x < qtdade; x++)
		{
			if(skills[x].fromvalue == searchvalue)
			{
				axei = 1;
				break;
			}
			if(skills[x].fromvalue > searchvalue && skills[x].fromvalue < (searchvalue+2))
			{
				//printf("[%s] de [%u != %u] para [%u] em [%d] segundos\n",
				//	skills[x].SkillName, skills[x].fromvalue, searchvalue, skills[x].tovalue, skills[x].tempo);
			}
		}
		if(axei)
		{
			demora += skills[x].tempo;
			demora2 += skills[x].tempo;
			
			lastvalue = skills[x].tempo;
			//printf("%.1f ate %.1f: %u\n",
			//	(float)searchvalue/10, (float)(searchvalue + 1)/10, skills[x].tempo);
		}
		else
		{
			demora += lastvalue;
			demora2 += lastvalue;
			//printf("%.1f ate %.1f: %u(lasttime)\n",
			//	(float)searchvalue/10, (float)(searchvalue + 1)/10, lastvalue);
		}
		
		if(searchvalue - lastdez >= 50)
		{
			printf("De %.1f ate %.1f: %uh %02um - %um %02us por 0.1\n",
				(float)lastdez/10, (float)searchvalue/10, demora2/60/60, demora2/60%60, demora2/60/50, demora2/50%60);
			demora2 = 0;
			lastdez = searchvalue;
		}
		
		searchvalue = searchvalue + 1;
	}
	
	printf("De %.1f ate %.1f: %uh %02um - %um %02us por 0.1\n",
		(float)lastdez/10, (float)searchvalue/10, demora2/60/60, demora2/60%60, demora2/60/50, demora2/50%60);
	
	
	printf("Foi encontrado %u entradas de %s entre %.1f e %.1f\nMenor tempo: %u segundos\nMaior tempo: %u segundos\nTempo medio: %u segundos\n",
		num_encontrada, skills[0].SkillName, (float)fromvalue/10, (float)tovalue/10, menortempo, maiortempo, tempomedio);
	
	printf("Tempo provavel que demora pra subir %s de %.1f a %.1f: %.2f horas(%u minutos)\n",
		skills[0].SkillName, (float)fromvalue/10, (float)tovalue/10, (float)demora/60/60, demora/60);
	
	return 0;
}