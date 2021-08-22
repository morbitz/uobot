#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

unsigned char teste[]="\x6F\x70\x61";

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

void DesenCript(unsigned char *buf, int len, unsigned char key)
{
	int x;
	for(x=0; x<len; x++)
		buf[x] ^= key;
}


unsigned char *EnCript(unsigned char *buf, int len, char n)
{
	unsigned char *tmp=malloc(len+1);
	int x;
	strcpy(tmp, buf);

	for(x=0; x<len; x++)
	{
		buf[x] = tmp[len-x-1];
		buf[x] += n;
	}
	free(tmp);
	return buf;
}

int main(int argc, char **argv)
{
	unsigned char *HexText,*c,*CriptText;
	unsigned char cript=0x90;
	unsigned char *Test="fala serio meo";
	if(argc<2)
	{
		printf("Uso: %s \"texto\" cript\n", argv[0]);
		return 0;
	}
	//printf("%s\n", EnCript(Test, strlen(Test), -3));
	//printf("%s\n", EnCript(Test, strlen(Test), -3));
	printf("%s\n", EnCript(argv[1], strlen(argv[1]), -3));
	system("pause");
	exit(0);
	if(argc==3) cript=(unsigned char)ArgToInt(argv[2]);
	if((HexText=malloc((strlen(argv[1])*4)+1))==NULL)
	{
		printf("Nao foi possivel criar %d bytes\n", (strlen(argv[1])*4)+1);
		return 0;
	}
	if((CriptText=malloc((strlen(argv[1])*4)+1))==NULL)
	{
		printf("Nao foi possivel criar %d bytes\n", (strlen(argv[1])*4)+1);
		return 0;
	}
	*HexText = 0;
	*CriptText = 0;
	c=argv[1];
	while(*c!=0)
	{
		sprintf(HexText, "%s\\x%02X", HexText, *c);
		sprintf(CriptText, "%s\\x%02X", CriptText, *c^cript);
		c++;
	}
	printf("HexText: %s\n", HexText);
	printf("CriptText: %s\n", CriptText);
	printf("Teste: %s\n", teste);
	free(HexText);
	free(CriptText);
	return 0;
}


