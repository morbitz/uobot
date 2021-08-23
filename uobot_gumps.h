
typedef struct { /* Struct do gump */
	char cmd; /* 0xb0 */
	short dtlen; /* Tamanho do pacote */
	int serial; /* Serial do Gump */
	int tipo; /* Gumpid */
	int unk1; /* X */
	int unk2; /* Y */
	short laylen; /* Tamanho do lay */
	char *lay; /* Lay */
	short nlines; /* Numero de linhas */
	short linelen; /* Tamanho da primeira linha */
	char *lines; /* Linhas */
} GumpType;

typedef struct {
	char *Text;
} LineType;


typedef struct {
	unsigned int Id;
	unsigned int x;
	unsigned int y;
	unsigned int Page;
} TextType;

typedef struct {
	unsigned int Id;
	unsigned int x;
	unsigned int y;
	unsigned int Page;
} ButtonType;

typedef struct {
	unsigned int Id;
	unsigned int x;
	unsigned int y;
	unsigned int Page;
} GumppicType;


typedef struct {
	unsigned int Serial;
	unsigned int Id;
	ButtonType *Buttons;
	unsigned int ButtonCount;
	TextType *Texts;
	unsigned int TextCount;
	LineType *Lines;
	unsigned int LineCount;
	GumppicType *Gumppics;
	unsigned int GumppicCount;
} GumpDoneType;


/* Handle do pacote 0xb1 */
int uo_handle_gump(unsigned char *buf, int len);
/* Wait gump */
void Wait_Gump(void);
/* Procura por um texto dentro das linhas e retorna o x,y,page do Text  */
unsigned int GumpSearchText(char *Texto, unsigned int *X, unsigned int *Y, unsigned int *Page);
/* Procura pelo x,y dentro de uma pagina e retorna o id */
unsigned int GumpSearchXY(unsigned int x, unsigned int y, unsigned int Page);
/* Libera todo o espaco alocado pelo gump */
void FreeGump(void);
/* Send gump choice */
void SendGumpChoice(unsigned int ButtonId);
/* Qdo o gump eh enviado, o handle seta essa variavel true */
int GumpSent;

GumpDoneType *GumpDone;