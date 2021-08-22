#include <windows.h>
#include <winsock.h>
#include "pluginirc.h"

// Criar o sock e retornar o numero
int net_connect(char *ip, short port)
{
	struct sockaddr_in *redsock = NULL;
	struct sockaddr stsock;
	struct hostent *redhost = NULL;
	int ret;
	SOCKET s;

	if(ip==NULL)
	{
		IF->LogPrint(3, "[ERROR] Ip = null\n");
		return SOCKET_ERROR;
	}
	memset(&stsock, 0x00, sizeof(stsock));
	redsock = (struct sockaddr_in *)&stsock;
	stsock.sa_family = AF_INET;
	redhost = gethostbyname(ip);
	if(redhost == NULL)
	{
		IF->LogPrint(3, "[ERROR] connect() Nao foi possivel descobrir o ip  do endereco %s\n", ip);
		return SOCKET_ERROR;
	}
	memcpy(&redsock->sin_addr.s_addr, redhost->h_addr, redhost->h_length);
	redsock->sin_port = htons(port);
	if((s=socket(AF_INET, SOCK_STREAM, IPPROTO_TCP))==INVALID_SOCKET)
	{
		IF->LogPrint(3, "[ERROR] socket()\n");
		return SOCKET_ERROR;
	}

	if((ret = connect(s, (struct sockaddr *)&stsock, sizeof(struct sockaddr))) == SOCKET_ERROR)
	{
		IF->LogPrint(1, "[ERROR] Connect() falhou. Error: %d\n", WSAGetLastError());
		return SOCKET_ERROR;
	}

	IF->LogPrint(3, "[NET] Conectado IP: %d.%d.%d.%d\n", (unsigned char)redhost->h_addr[0], (unsigned char)redhost->h_addr[1], (unsigned char)redhost->h_addr[2], (unsigned char)redhost->h_addr[3], ntohs(redsock->sin_port));
	return s;
}

int net_send(char *buf, int len)
{
	int ret=0;
	if(sockIrc == 0)
		return -1;
	if((ret=send(sockIrc, buf, len, 0))!=len){
//		IF->LogPrint(1, "[ERROR] send() != len (%d!=%d)\n", ret, len);
		return -1;
	}

	if(LogIrc)
	{
		if(buf[len] == '\n')
		{
			buf[len] = 0;
			IF->LogPrint(3, "[IRC-SEND] [%s]\n", buf);
			buf[len] = '\n';
		}
	}
	return ret;
}

// Receber pacotes
int net_recv(char *buf, unsigned int len)
{
	fd_set master;
	struct timeval timeout;
	int ret;

	FD_ZERO(&master);
	FD_SET(sockIrc, &master);
	timeout.tv_sec = 0; // Timeout sec
	timeout.tv_usec = 0;
	ret=select(sockIrc+1, &master, (fd_set *)0, (fd_set *)0, 0);

	if(ret == -1) // Problema no select()
	{ 
		IF->LogPrint(1, "[ERROR] select error\n"); 
		return 0; 
	}
	if(ret==0) return 0; // Timeout

	if(!FD_ISSET(sockIrc, &master))
	{
		IF->LogPrint(1, "[ERROR] Nao poderia chegar aqui\n");
		return 0;
	}
	if((ret = recv(sockIrc, buf, len, 0)) <= 0)
	{
		if(ret==0) // Disconnect
		{
			FD_CLR(sockIrc, &master);
			return 0;
		} else { // Ret = -1
			IF->LogPrint(1, "[ERROR] recv() error\n");
			return 0; // Tentar reconectar
		}
	}

	FD_CLR(sockIrc, &master);
	return ret;
}