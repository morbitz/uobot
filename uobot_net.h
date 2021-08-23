
#include "UOHuffman.h"
// Tamanho minimo do buf descomprimido
#define MIN_DECBUF_SIZE(in) ((in * 4) + 4)


SOCKET sockshard;
int firstpacket;

int LoguinSeed;
HuffmanObj HuffObj;

int send_server(char *buf, int len);
int net_recv(SOCKET s, char *buf, unsigned int tmout, unsigned int len);
int net_connect(char *ip, short port);
int CloseShardSocket(void);