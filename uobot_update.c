#include <windows.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "uobot_log.h"
#include "uobot_net.h"

/* default polynomial used for the table generation */
#define CRC_DEFAULT_POLYNOMIAL	0x04c11db7

/* builds the CRC32 table */
void CCrc(unsigned int polynomial);

/* Mirror a number of bits in a value */
unsigned int Reflect(unsigned int source, int c);

/* calculate the checksum of a given buffer (CRC32) */
unsigned int CalculateCRC32(unsigned char *Buf, unsigned int Len);


unsigned int CRCTable[256];
int TableBuilt = 0;


/******************************************************************************\
* 
* 	Construct a checksum instance from a given polynomial
* 
* 	PARAMETERS:
* 		unsigned int polynomial		Polynomial to use in checksum calculations
* 
* 	RETURNS:
* 		-none-
* 
\******************************************************************************/

void CCrc(unsigned int polynomial)
{
	int i = 0, j = 0;
	for(i = 0; i < 256; i++)
	{
		CRCTable[i] = Reflect(i, 8) << 24;

		for(j = 0; j < 8; j++)
			CRCTable[i] = (CRCTable[i] << 1) ^ ((CRCTable[i] & (1 << 31) ? polynomial : 0));

		CRCTable[i] = Reflect(CRCTable[i], 32);
	}
}

/******************************************************************************\
* 
* 	Mirror a number of bits in a value
* 
* 	PARAMETERS:
* 		unsigned int source		Source value
* 		int c					Number of bits to mirror
* 
* 	RETURNS:
* 		unsigned int			Mirrored value
* 
\******************************************************************************/

unsigned int Reflect(unsigned int source, int c) 
{
	unsigned int value = 0;

	int i;
	for(i = 1; i < (c + 1); i++)
	{
		if(source & 0x1)
		{
			value |= 1 << (c - i);
		}

		source >>= 1;
	}

	return value;
} 



unsigned int CalculateCRC32(unsigned char *Buf, unsigned int Len)
{
	unsigned int i = 0, crc = 0xffffffff;

	if(!TableBuilt)
	{
		CCrc(CRC_DEFAULT_POLYNOMIAL);
		TableBuilt = 1;
	}

	for(i = 0; i < Len; i++)
		crc = (crc >> 8) ^ CRCTable[(crc & 0xff) ^ Buf[i]];

	return crc ^ 0xffffffff;
}





unsigned int GetCheckSum(char *PathFile)
{
	HANDLE ClientHandle = 0;
	char *ClientBuf = 0;
	DWORD ClientSize = 0, BytesRead = 0;
	DWORD CheckSum = 0;
	
	/* copy the client to a buffer */
	ClientHandle = CreateFile(PathFile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_RANDOM_ACCESS, 0);
	if(ClientHandle == INVALID_HANDLE_VALUE)
	{
		LogPrint(3, "[UPDATE] Nao foi possivel abrir arquivo uobot.exe\n");
		return 0;
	}

	ClientSize = GetFileSize(ClientHandle, NULL);
	ClientBuf = (unsigned char *)malloc(ClientSize);
	if(ClientBuf == NULL)
	{
		LogPrint(3, "[UPDATE] Nao foi possivel alocar %d bytes\n", ClientSize);
		CloseHandle(ClientHandle);
		return 0;
	}


	ReadFile(ClientHandle, ClientBuf, ClientSize, &BytesRead, NULL);

	CloseHandle(ClientHandle);

	CheckSum = CalculateCRC32(ClientBuf, ClientSize);

	free(ClientBuf);

	return CheckSum;
}

int HttpCheckVers(void)
{
	SOCKET Http = 0;
	int ret = 0;
	DWORD CheckSumUoBot = 0, CheckSumPlugin = 0, CheckSumIrc = 0;
	DWORD SiteCheckUoBot = 0, SiteCheckPlugin = 0, SiteCheckIrc = 0;

	char s1[50] = {0}, s2[50]={0}, s3[50]={0};
	char CBot[50] = {0}, CPlu[50] = {0}, CIrc[50] = {0};

	char buf[5000] = {0}, *ponteiro = buf;

	char CheckPluginName[] = "pluginatck.dll";
	char CheckUobotName[] = "uobot.exe";
	char CheckPluginIrc[] = "pluginirc.dll";

	LogPrint(3, "[UPDATE] Fazendo checksum da versão atual\n");

	if((CheckSumIrc = GetCheckSum(CheckPluginIrc))==0)
	{
		/*LogPrint(3, "[UPDATE] Nao foi possivel fazer checksum do pluginirc\n");
		return 0;
		*/
	}
	else
		LogPrint(3, "[UPDATE] Checksum pluginirc: [0x%X]\n", CheckSumIrc);

	if((CheckSumUoBot = GetCheckSum(CheckUobotName))==0)
	{
		LogPrint(3, "[UPDATE] Nao foi possivel fazer checksum do core\n");
		return 0;
	}
	else
		LogPrint(3, "[UPDATE] Checksum uobot: [0x%X]\n", CheckSumUoBot);

	if((CheckSumPlugin = GetCheckSum(CheckPluginName))==0)
	{
		LogPrint(3, "[UPDATE] Nao foi possivel fazer checksum do plugin\n");
		return 0;
	}
	else
		LogPrint(3, "[UPDATE] Checksum pluginatck: [0x%X]\n", CheckSumPlugin);

	sprintf(buf,"GET /versions.txt HTTP/1.1\r\n"
				"Host: www.uobot.com.br\r\n"
				"Accept: */*\r\n"
				"User-Agent: Mozilla/4.0 (compatible; MSIE 4.01; Windows 98)\r\n"
				"Pragma: no-cache\r\n"
				"Cache-Control: no-cache\r\n"
				"Connection: close\r\n\r\n");
		

	if((Http=net_connect("www.uobot.com.br", 80))==SOCKET_ERROR)
	{
		LogPrint(3, "[UPDATE] Nao foi possivel conectar ao site do uobot. Site mudou?\n");
		return 0;
	}

	if((ret = send(Http, buf, strlen(buf), 0)) == SOCKET_ERROR)
	{
		LogPrint(3, "[UPDATE] Nao foi possivel enviar o query\n");
		closesocket(Http);
		return 0;
	}

	buf[0] = 0;
	while((ret = recv(Http, ponteiro, sizeof(buf), 0))>0)
	{
		ponteiro += ret;
		*ponteiro = 0;
	}


	closesocket(Http);

	if(!(ponteiro = strstr(buf, "uobot.exe")))
	{
		LogPrint(3, "[UPDATE] Nao foi possivel localizar uobot.exe\n");
		return 0;
	}
	
	sscanf(ponteiro, "%*s %20s %*s %20s %*s %20s", s1, s2, s3);

	sprintf(CBot, "0x%X", CheckSumUoBot);
	sprintf(CPlu, "0x%X", CheckSumPlugin);

	ret = 0;
	if(CheckSumIrc)
	{
		sprintf(CIrc, "0x%X", CheckSumIrc);
		if(s3[0] != 0 && stricmp(s3, CIrc))
		{
			LogPrint(3, "[UPDATE] Nova versao do pluginirc.dll disponivel\n");
			LogPrint(3, "[UPDATE] Minha versao: [%s], Nova: [%s]\n", CIrc, s3);
			ret = 1;
		}
	}

	if(stricmp(s1, CBot))
	{
		LogPrint(3, "[UPDATE] Nova versao do uobot.exe disponivel\n");
		LogPrint(3, "[UPDATE] Minha versao: [%s], Nova: [%s]\n", CBot, s1);
		ret = 1;
	}

	if(stricmp(s2, CPlu))
	{
		LogPrint(3, "[UPDATE] Nova versao do pluginatck.dll disponivel\n");
		LogPrint(3, "[UPDATE] Minha versao: [%s], Nova: [%s]\n", CPlu, s2);
		ret = 1;
	}
	
	return ret;
}
