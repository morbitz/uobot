#include <windows.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "uobot_handles.h"
#include "uobot_net.h"
#include "uobot_log.h"
#include "uobot_obj.h"
#include "uobot_jornal.h"
#include "uobot_target.h"
#include "uobot_spells.h"
#include "uobot_threads.h"
#include "uobot_commands.h"
#include "uobot.h"

// Socket do shard
SOCKET sockshard=0;
int firstpacket = 1;
int LoguinSeed = 0;
HuffmanObj HuffObj;

int send_server(char *buf, int len);
int net_recv(SOCKET s, char *buf, unsigned int tmout, unsigned int len);
int net_connect(char *ip, short port);

// Funcao de envio pro shard
int send_server(char *buf, int len)
{
	int ret=len,x;
	unsigned int PacketLen=0;
	char *cryptbuf=buf;
	
	// Primeiro pacote?
	if(firstpacket)
	{
		LogPrint(3, "[NET] Sending LoginSeed\n");
		if(PrintDebug) LogDump(buf, len);
		firstpacket = 0;
	}
	else {
		PacketLen = GetPacketLen(buf, len);

		if(PacketLen<(unsigned int)len){
			while(ret!=0)
			{
				PacketLen = GetPacketLen(cryptbuf, ret);
				//LogPrint((PrintDebug ? 3 : 0), "[C->S] [%#x] %s [%u]\n", *cryptbuf&0xff, PacketNames[*cryptbuf&0xff], PacketLen);
				//if(PrintDebug) LogDump(cryptbuf, PacketLen);
				uo_processclientmsg(cryptbuf, PacketLen);
				cryptbuf += PacketLen;
				ret -= PacketLen;
			}
		} else {
			//LogPrint((PrintDebug ? 3 : 0), "[C->S] [%#x] %s [%u]\n", *buf&0xff, PacketNames[*buf&0xff], len);
			//if(PrintDebug) LogDump(buf, len);
			uo_processclientmsg(cryptbuf, len);
		}
	}


	// Adionando a encryptacao da mystic
	if(InsertM2aCrypt)
	{
		cryptbuf = buf;
		for(x=0;x<len;x++)
			cryptbuf[x] ^= 0x90;
	}

	// Enviar o pacote para o shard
	if((ret=send(sockshard, buf, len, 0))!=len)
	{
		// Tamanho do pacote enviado for diferente doq deveria ser
		//LogPrint(3, "[ERROR] send() != len (%d!=%d)\n", ret, len);
	}
	return ret;
}

// Receber pacotes
int net_recv(SOCKET s, char *buf, unsigned int tmout, unsigned int len)
{
	fd_set master;
	struct timeval timeout;
	int ret=0;
	unsigned int outbyt=0;
	char *dcbuf=0;

	FD_ZERO(&master);
	FD_SET(s, &master);
	timeout.tv_sec = 0; // Timeout sec
	timeout.tv_usec = 0;
	ret=select(s+1, &master, (fd_set *)0, (fd_set *)0, 0);

	if(ret == -1) // Problema no select()
	{ 
		LogPrint(3, "[ERROR] select error\n"); 
		return 0; 
	}
	if(ret==0) return 0; // Timeout
	if(!FD_ISSET(s, &master))
	{
		LogPrint(3, "[ERROR] Nao poderia chegar aqui\n");
		return -1;
	}
	if((ret = recv(s, buf, 8000, 0)) <= 0)
	{
		if(ret==0) // Disconnect
		{
			LogPrint(3, "[DISCONNECT] Desconectado\n");
			FD_CLR(s, &master);
			return -1;
		} else if(ret == -1){ // Ret = -1
			LogPrint(3, "[ERROR] Recv error code[%d]\n", WSAGetLastError());
			return -1; // Tentar reconectar
		}
	}
//	if(LoguinSeed) return ret;
	//Alocar espaco pra descompressao
	if((dcbuf = (char *)malloc(MIN_DECBUF_SIZE(ret)))==NULL)
	{
		LogPrint(3, "[ERROR] Nao foi possivel malocar espaco\n");
		return -1;
	}
	Decompress(dcbuf, buf, &outbyt, &ret, &HuffObj); // Descomprimir
	memcpy(buf, dcbuf, outbyt); // Copiar buf descomprimido
	free(dcbuf); // Liberar buf descomprimido
	LogPrint((PrintDebug ? 2 : 0), "Decompressed %u bytes into %u bytes\n", ret, outbyt);
	FD_CLR(s, &master);
	return outbyt;
}


// Criar o sock e retornar o numero
int net_connect(char *ip, short port)
{
	struct sockaddr_in *redsock = NULL;
	struct sockaddr stsock;
	struct hostent *redhost = NULL;
	int ret;
	SOCKET s;
	int sendbuf,recvbuf;
	/*int tmp;*/

	if(ip==NULL)
	{
		LogPrint(3, "[ERROR] Ip = null\n");
		return SOCKET_ERROR;
	}
	memset(&stsock, 0x00, sizeof(stsock));
	redsock = (struct sockaddr_in *)&stsock;
	stsock.sa_family = AF_INET;
	redhost = gethostbyname(ip);
	if(redhost == NULL)
	{
		LogPrint(3, "[ERROR] connect() Nao foi possivel descobrir o ip  do endereco %s\n", ip);
		return SOCKET_ERROR;
	}
	memcpy(&redsock->sin_addr.s_addr, redhost->h_addr, redhost->h_length);
	redsock->sin_port = htons(port);
	if((s=socket(AF_INET, SOCK_STREAM, IPPROTO_TCP))==INVALID_SOCKET)
	{
		LogPrint(3, "[ERROR] socket()\n");
		return SOCKET_ERROR;
	}
/*	getsockopt(s, SOL_SOCKET, SO_RCVBUF, (char *)&recvbuf, &tmp);
	getsockopt(s, SOL_SOCKET, SO_SNDBUF, (char *)&sendbuf, &tmp);

	LogPrint(3, "[BUF] Recv(%d) Send(%d)\n", recvbuf, sendbuf);
*/

	sendbuf = 65000;
	recvbuf = 65000;

	/*setsockopt(s, SOL_SOCKET, SO_RCVBUF, (char *)&sendbuf, sizeof(sendbuf));
	setsockopt(s, SOL_SOCKET, SO_SNDBUF, (char *)&recvbuf, sizeof(recvbuf)); */

/*	getsockopt(s, SOL_SOCKET, SO_RCVBUF, (char *)&recvbuf, &tmp);
	getsockopt(s, SOL_SOCKET, SO_SNDBUF, (char *)&sendbuf, &tmp);

	LogPrint(3, "[BUF] Recv(%d) Send(%d)\n", recvbuf, sendbuf);
*/

	if((ret = connect(s, (struct sockaddr *)&stsock, sizeof(struct sockaddr))) == SOCKET_ERROR)
	{
		LogPrint(1, "[ERROR] Connect() falhou. Error: %d\n", WSAGetLastError());
		return SOCKET_ERROR;
	}

	//LogPrint(3, "[NET] Conectado IP: %d.%d.%d.%d\n", (unsigned char)redhost->h_addr[0], (unsigned char)redhost->h_addr[1], (unsigned char)redhost->h_addr[2], (unsigned char)redhost->h_addr[3], ntohs(redsock->sin_port));
	return s;
}

int CloseShardSocket(void)
{
	closesocket(sockshard);
	return 0;
}