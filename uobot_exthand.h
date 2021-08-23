

typedef int (*PacketHandler) (unsigned char *, int);

void InitPacketHandler(void);
int AddPacketHandler(int PacketID, void *Handler);

PacketHandler ExternalHandlers[256];