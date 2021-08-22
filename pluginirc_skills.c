#include <windows.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdio.h>
#include "pluginirc.h"
#include "pluginirc_irc.h"

typedef struct ActionT {
	char *Name;
	char *Value;
	unsigned short LastValue;
} ActionsType;


unsigned int SpellsSize=64;
unsigned int SkillsSize=50;
unsigned int SkillsTimeList[50] = {0}; 

ActionsType SkillsName[50] = {
	{"Alchemy",				 "0 0",	0},
	{"Anatomy",				 "1 0",	0},
	{"Animal Lore",			 "2 0",	0},
	{"Item Identification",	 "3 0",	0},
	{"Arms Lore",			 "4 0",	0},
	{"Parrying",			 "5 0",	0},
	{"Begging",				 "6 0",	0},
	{"Blacksmithy",			 "7 0",	0},
	{"Bowcraft",			 "8 0",	0},
	{"Peacemaking",			 "9 0",	0},
	{"Camping",				"10 0",	0},
	{"Carpentry",			"11 0",	0},
	{"Cartography",			"12 0",	0},
	{"Cooking",				"13 0",	0},
	{"Detecting Hidden",	"14 0",	0},
	{"Enticement",			"15 0",	0},
	{"Evaluating Intelligence","16 0",	0},
	{"Healing",				"17 0",	0},
	{"Fishing",				"18 0",	0},
	{"Forensic Evaluation",	"19 0",	0},
	{"Herding",				"20 0",	0},
	{"Hiding",				"21 0",	0},
	{"Provocation",			"22 0",	0},
	{"Inscription",			"23 0",	0},
	{"Lockpicking",			"24 0",	0},
	{"Magery",				"25 0",	0},
	{"Resisting Spells",	"26 0",	0},
	{"Tactics",				"27 0",	0},
	{"Snooping",			"28 0",	0},
	{"Musicianship",		"29 0",	0},
	{"Poisoning",			"30 0",	0},
	{"Archery",				"31 0",	0},
	{"Spirit Speak",		"32 0",	0},
	{"Stealing",			"33 0",	0},
	{"Tailoring",			"34 0",	0},
	{"Animal Taming",		"35 0",	0},
	{"Taste Identification","36 0",	0},
	{"Tinkering",			"37 0",	0},
	{"Tracking",			"38 0",	0},
	{"Veterinary",			"39 0",	0},
	{"Swordsmanship",		"40 0",	0},
	{"Mace Fighting",		"41 0",	0},
	{"Fencing",				"42 0",	0},
	{"Wrestling",			"43 0",	0},
	{"Lumberjacking",		"44 0",	0},
	{"Mining",				"45 0",	0},
	{"Meditation",			"46 0",	0},
	{"Stealth",				"47 0",	0},
	{"Remove Trap",			"48 0",	0},
	{"Necromancy",			"49 0",	0}};


int uo_handle_updateskill(char *buf, int len) // 0x3A
{
	unsigned int TimeNow = time(0),count=0;
	unsigned char flag=buf[3],*p=buf+4;

	if(!IrcConnected)
		return 0;

	if(flag == 0)
	{
		while(*((unsigned short *)p)!=0x0000)
		{
			SkillsName[p[1]-1].LastValue = IF->unpack16(p+2);
			p += 7;
			count++;
		}
		//LogPrint(3, "[SKILLS] Updated %d skills\n", count);
	}
	else if(flag == 0xFF)
	{
		if(SeeSkillUp)
		{
			if(SkillsTimeList[buf[5]] != 0)
			{
				Irc_Say(MainChan, "Skill (%s) mudou de (%.1f) para (%.1f) em (%d) segundos\n",
					SkillsName[buf[5]].Name, (double)SkillsName[buf[5]].LastValue/10, (double)IF->unpack16(buf+6)/10, TimeNow - SkillsTimeList[buf[5]]);
			}
			else
			{
				Irc_Say(MainChan, "Skill (%s) mudou de (%.1f) para (%.1f)\n",
					SkillsName[buf[5]].Name, (double)SkillsName[buf[5]].LastValue/10, (double)IF->unpack16(buf+6)/10);
			}
		}
		SkillsTimeList[buf[5]] = TimeNow;
		SkillsName[buf[5]].LastValue = IF->unpack16(buf+6);
	}
	return 0;
}
