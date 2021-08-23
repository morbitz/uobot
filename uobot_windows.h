#define MAXTEXTSIZE 10000

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, INT nCmdShow);
BOOL CALLBACK MainDlgProc(HWND hDlg, UINT wMsg, WPARAM wParam, LPARAM lParam);

HANDLE hMainProc;
DWORD LastMessageTime;
HANDLE CurInstance;
HWND CurWnd;
HWND MainWnd;
void TextBoxCat(HWND Dlg, int DlgItem, const char *Text, ...);
void ConnectBot();

DWORD WINAPI StartThread(LPVOID lpParameter);

typedef void (*CommandHandler)(CmdInfoTag *);

typedef struct tagIRWCmd
{
	HMODULE Place;
	char Command[30];
	CommandHandler Handler;
}Cmd;

HANDLE hMainThread;
int AddCommand(const char *Name, void *Handler);
void HandleCommand(char *Text, int Flag);
Cmd *Commands;
unsigned int CmdListSize;
void HookConsoleTo(void *HF);
