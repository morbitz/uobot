#include <string.h>
#include <windows.h>
#include "pluginirc.h"

unsigned long ArgToInt(char *Arg)
{
	unsigned long Value = 0;

	if(!strlen(Arg))
		return 0;

	/* check if its a serial (hexa or decimal) */
	if((!strncmp(Arg, "0x", 2) || !strncmp(Arg, "0X", 2)) && (strlen(Arg) > 2 && isxdigit(Arg[2])))
		return strtol(Arg, NULL, 16);
	else if(isdigit(Arg[0]))
		return strtol(Arg, NULL, 10);

	return Value;
}

int LineSearch(LineType *Lines, unsigned int LineCount, char *Texto)
{
	unsigned int x = 0;
	for(x=0; x < LineCount; x++)
	{
		if(strstr(Lines[x].Text, Texto)!=NULL)
			return x;
	}
	return -1;
}

char *StrChrNum(char *Texto, char Word, int Num)
{
	while(*Texto!=0 && Num)
	{
		if(*Texto == Word)
			Num--;

		Texto++;
	}

	return Texto;
}
