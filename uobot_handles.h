
#define SIZE_VARIABLE 0


HANDLE hThread;
HANDLE hPluginThread;

// Tamanho do pacote variavel

char *PacketNames[256];
int PacketLengths[256];

int AntiMacro;

// Tipo de funcao handler
typedef int (*handle_msg)(char *buf, int len); 
struct hmsg { // Estrutura dos handles
	char *name;
	int size;
	handle_msg uo_hand;
};
struct hmsg hnmsg[0xcd];


struct gumprecv { // Struct do gump
	char cmd;
	short dtlen; // Tamanho do pacote
	int serial; // Serial do Gump
	int tipo;
	int unk1;
	int unk2;
	short laylen; 
	char *lay;
	short nlines;
	short linelen;
	char *lines;
};


struct gmrecv { // Botao do am
	int x; 
	int y;
	int id;
	int num;
};


// Uo Handles
int uo_handle_servers_list(char *buf, int len); //0xA8
int uo_handle_server_speech(char *buf, int len); // 0x1C
int uo_handle_relay(char *buf, int len); //0x8C
int uo_handle_list_chars(char *buf, int len); // 0xA9
int uo_handle_loguin_confirm(char *buf, int len); // 0x1B
int uo_handle_loguin_complete(char *buf, int len); //0x55
int uo_handle_ping(char *buf, int len); // 0x73
int uo_handle_open_gump(char *buf, int len); //0xB0
int uo_handle_draw_player(char *buf, int len); //0x20
int uo_handle_error_code(char *buf, int len); //0x53
int uo_handle_draw_object(char *buf, int len); // 0x78
int uo_handle_delete(char *buf, int len); //0x1D
int uo_handle_update_item(char *buf, int len); // 0x1A
int uo_handle_updatestam(char *buf, int len); //0xA3
int uo_handle_updatemana(char *buf, int len); //0xA2
int uo_handle_updatehp(char *buf, int len); //0xA1
int uo_handle_char_stats(char *buf, int len); // 0x11
int uo_handle_update_playerpos(char *buf, int len); // 0x77
int uo_handle_pickup(char *buf, int len); // 0x07
int uo_handle_dropitem(char *buf, int len); // 0x08
int uo_handle_equipitem(char *buf, int len); // 0x2E
int uo_handle_addtocontainer(char *buf, int len); // 0x25
int uo_handle_opencontainer(char *buf, int len); // 0x24
int uo_handle_updatecontainer(char *buf, int len); // 0x3C
int uo_handle_paperdoll(char *buf, int len); // 0x88
int uo_handle_trading(char *buf, int len); // 0x6F
int uo_handle_unispeech(char *buf, int len); // 0xAE
int uo_handle_target(char *buf, int len); // 0x6C
int uo_handle_updateskill(char *buf, int len); // 0x3A
int uo_handle_login_denied(char *buf, int len); //0x82

void uo_processpkt(unsigned char *buf, int len);
void uo_processclientmsg(char *buf, int len);

void uo_sendloguin(char *loguin, char *senha);
unsigned int GetPacketLen(unsigned char *Buf, unsigned int Size);

typedef int (*send_shard)(char *, int);
DWORD WINAPI uo_thread_ping(LPVOID lpParameter);

int IsConnected;
int DebugClientMsg;

void HandleCmd(char *Text);