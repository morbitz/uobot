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
	char buf[1024]={0};
	unsigned int size=0,count=0;
	char *point=0;
	FILE *fh=0;


	if(argc<2)
	{
		printf("Uso: %s senhas.txt\n", argv[0]);
		return 0;
	}
	if((fh=fopen(argv[1], "r"))==NULL)
	{
		printf("Nao foi possivel abrir %s\n", argv[1]);
		return 0;
	}
	fseek(fh, 0, SEEK_END);
	size = ftell(fh);
	fseek(fh, 0, SEEK_SET);
	//printf("Tamanho do arquivo: %d\n", size);

	while(fgets(buf, 1020, fh)!=NULL)
	{
		
		if(buf[strlen(buf)-2] == ' ')
			buf[strlen(buf)-2] = 0;

		if(buf[0] == '\n')
			continue;

		if((point = strstr(buf, " "))==NULL)
			continue;

		*point = 0;
		printf("%s ", EnCript(buf, strlen(buf), -3));
		printf("%s\n", EnCript(point+1, strlen(point+1), -3));
		count++;
	}
	//printf("Lido %d senhas\n", count);
	//system("pause");
	return 0;
}


