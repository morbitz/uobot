#include <windows.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "pluginirc.h"

unsigned int HDSerial = 0;

SOCKET Make_Connection(char *ip, short port)
{
	struct sockaddr_in *redsock = NULL;
	struct sockaddr stsock;
	struct hostent *redhost = NULL;
	int ret;
	SOCKET s;

	if(ip==NULL)
		return SOCKET_ERROR;

    memset(&stsock, 0x00, sizeof(stsock));
	redsock = (struct sockaddr_in *)&stsock;
	stsock.sa_family = AF_INET;

	if((redhost = gethostbyname(ip)) == NULL)
		return SOCKET_ERROR;

    memcpy(&redsock->sin_addr.s_addr, redhost->h_addr, redhost->h_length);
    redsock->sin_port = htons(port);
	
    if((s=socket(AF_INET, SOCK_STREAM, IPPROTO_TCP))==INVALID_SOCKET)
		return SOCKET_ERROR;

	if((ret = connect(s, (struct sockaddr *)&stsock, sizeof(struct sockaddr))) == SOCKET_ERROR)
		return SOCKET_ERROR;

	return s;
}

BOOL GetVolumeSerial(unsigned int *dwVolSerial)
{
	BOOL ret = GetVolumeInformation((LPCTSTR)"C:\\",(LPTSTR)NULL,(DWORD)NULL,(LPDWORD)dwVolSerial,(LPDWORD)NULL,(LPDWORD)NULL,(LPTSTR)NULL,(DWORD)NULL);
	if(!ret)
		return GetVolumeInformation((LPCTSTR)"E:\\",(LPTSTR)NULL,(DWORD)NULL,(LPDWORD)dwVolSerial,(LPDWORD)NULL,(LPDWORD)NULL,(LPTSTR)NULL,(DWORD)NULL);
	return ret;
}

int CheckPass(void)
{
	SOCKET Http;
	int ret = 0;
	char buf[500] = {0};
   	char *recvbuf = 0, *point = 0;
	int recvbufsize = 0;

    if(GetVolumeSerial(&HDSerial)==FALSE)
	{
		IF->LogPrint(3, "[AUTH] Nao foi possivel achar o serial do hd\n");
        return 0;
	}

	sprintf(buf,	"GET /auth.php?key=%X HTTP/1.1\r\n"
					"Host: www.uobot.com.br\r\n"
					"Accept: */*\r\n"
					"User-Agent: Mozilla/4.0 (compatible; MSIE 4.01; Windows 98)\r\n"
					"Pragma: no-cache\r\n"
					"Cache-Control: no-cache\r\n"
					"Connection: close\r\n\r\n",
                    HDSerial);

	if((Http=Make_Connection("www.uobot.com.br", 80))==SOCKET_ERROR)
		return 0;

	if((ret = send(Http, buf, strlen(buf), 0)) <= 0)
        return 0;

	ret = 0;
	do {
		recvbufsize += ret;
		recvbuf = realloc(recvbuf, recvbufsize+200);
	} while((ret = recv(Http, recvbuf+recvbufsize, 199, 0)) > 0);

	recvbuf[recvbufsize] = 0;

	closesocket(Http);

    if(strstr(recvbuf, "KEYOK"))
        return 1;

	return 0;
}
