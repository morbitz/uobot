
typedef struct {
	unsigned short Graphic;
	unsigned char Desc[50];
} MenuOption;

typedef struct {
	MenuOption *m_options;
	char *m_question;
	unsigned int m_id;
	unsigned short m_gump;
	unsigned int m_num_choices;
} MenuOptionsTag;

MenuOptionsTag MenuOptions;

int WMenu;
int WDlg;

int uo_handle_textdlg(char *buf, int len);
int uo_handle_menu(char *buf, int len);
void Send_Reply(char *Text);
int Send_Choice(char *ChoiceText);