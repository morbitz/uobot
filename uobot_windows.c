#include <windows.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "uobot_log.h"
#include "uobot_net.h"
#include "uobot_handles.h"
#include "uobot_obj.h"
#include <time.h>
#include "uobot_jornal.h"
#include "uobot_target.h"
#include "uobot_spells.h"
#include "uobot_exthand.h"
#include "uobot_threads.h"
#include "uobot_plugins.h"
#include "uobot_commands.h"
#include "uobot.h"
#include <commctrl.h>
#include "resource.h"
#include "uobot_hexcp.h"
#include <richedit.h> //DAMN MOTHERFUCKER!!!
#include "uobot_windows.h"

#define WM_ENTER (WM_USER + 2) 

DWORD LastMessageTime=0;
DWORD WatchGuard=0;

HANDLE CurInstance = NULL;
HWND CurWnd = NULL;
HWND MainWnd = NULL;
HANDLE hMainThread=0;
HANDLE hMainProc=0;

char login[30]={0};
char senha[30]={0};
char charname[30]={0};
int CheckCmd=FALSE;
unsigned char CmdParam[2000];

BOOL AutoVScroll = 1;

Cmd *Commands = NULL;
unsigned int CmdListSize = 0;

BOOL HookConsole=FALSE;
typedef void (*HookFunc)(unsigned char *);
HookFunc HookConsFunc=0;

void HookConsoleTo(void *HF)
{
	if(HF==0)
	{
		HookConsole = FALSE;
		return;
	}
	HookConsFunc = (HookFunc)HF;
	HookConsole = TRUE;
	LogPrint(1, "[HOOK] Console hookiado para %#X\n", HF);
}


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, INT nCmdShow)
{
	HANDLE hRichEdit;
	CurInstance = hInstance;

	if(*lpCmdLine!=0){
		strcpy(CmdParam, lpCmdLine);
		CheckCmd = TRUE;
	}
	hMainProc = GetCurrentProcess();
	// Now all exceptions would be handled in my filter
	SetUnhandledExceptionFilter(MyUnhandledExceptionFilter);

	if((hRichEdit = LoadLibrary("RICHED32.DLL"))==NULL)
	{
		MBOut("Erro", "Nao foi possivel localizar RICHED32.DLL");
		return TRUE;
	}

	if(DialogBox(hInstance, MAKEINTRESOURCE(DIALOG_MAIN), 0, (DLGPROC)MainDlgProc) == -1)
		MBOut("Erro","Erro ao iniciar resource");

	FreeLibrary(hRichEdit);
	return TRUE;
}

HANDLE GetCurInstance(void)
{
	return CurInstance;
}

BOOL CALLBACK MainDlgProc(HWND hDlg, UINT wMsg, WPARAM wParam, LPARAM lParam)
{
	char console[2000];
	DWORD iResult=0;
	WSADATA wsaData;
	
	CurWnd = hDlg;
	LastMessageTime = GetTickCount();

	switch(wMsg)
	{
	case WM_INITDIALOG:
		iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
		if(iResult != NO_ERROR)
		{
			LogPrint(1, "[ERROR] Erro no WSAStartup()\n");
			exit(-1);
		}
		SendMessage(CurWnd, WM_SETICON, ICON_SMALL, (LPARAM)LoadIcon(CurInstance, MAKEINTRESOURCE(IDI_ICON2)));
		SendMessage(GetDlgItem(CurWnd, IDASCROLL), BM_SETCHECK, BST_CHECKED, 0);
		SetFocus(GetDlgItem(CurWnd, ID_CONSOLE));
		MainWnd = CurWnd;
		InitCommands();
		LogPrint(3, "[INIT] Handling exceptions\n");
		LogPrint(3, "[INIT] Watching for uobot hang\n");
		if(CheckCmd)
		{
			LogPrint(1, "[CMDLINE] %s", CmdParam);
			HandleCommand(CmdParam, 1);
			CheckCmd = FALSE;
		}
		CheckHang = 1;
		CreateThread(NULL,4096,(LPTHREAD_START_ROUTINE)MyWatchForHangClientThread,0,0,&WatchGuard);
		break;
	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDSEND:
			GetDlgItemText(CurWnd, ID_CONSOLE, console, sizeof(console));
			SetDlgItemText(CurWnd, ID_CONSOLE, (char*)0);
			SetFocus(GetDlgItem(CurWnd, ID_CONSOLE));
			if(console[0] == '/') HandleCommand(console, 0);
			else if(HookConsole) HookConsFunc(console);
			else if(hMainThread) Say(console);
			return FALSE;
		case IDASCROLL:
			if(SendMessage(GetDlgItem(CurWnd, IDASCROLL), BM_GETCHECK, 0, 0) == BST_CHECKED)
				AutoVScroll = 1;
			else 
				AutoVScroll = 0;
			return FALSE;
		break;
		case IDOK:
			if(GetFocus() == GetDlgItem(CurWnd, ID_CONSOLE)) 
			{
				GetDlgItemText(CurWnd, ID_CONSOLE, console, sizeof(console));
				SetDlgItemText(CurWnd, ID_CONSOLE, (char*)0);
				if(console[0] == '/') HandleCommand(console, 0);
				else if(HookConsole) HookConsFunc(console);
				else if(hMainThread) Say(console);
			}
			return FALSE;
		break;
		case ID_CONNECT:
			GetDlgItemText(CurWnd, EDIT_LOGIN, login, sizeof(login));					
			GetDlgItemText(CurWnd, EDIT_SENHA, senha, sizeof(senha));
			GetDlgItemText(CurWnd, EDIT_CHARNAME, charname, sizeof(charname));
			SetFocus(GetDlgItem(CurWnd, ID_CONSOLE));
			ConnectBot();
			return FALSE;
		break;
		case IDSEENEWS:
			HandleCommand("/seenews", 0);
			SetFocus(GetDlgItem(CurWnd, ID_CONSOLE));
			break;
    	case ID_DISCONNECT:
			if(!hMainThread)
			{
				MBOut("Erro", "Voce nao esta conectado");
				return FALSE;
			}
			ErrNum = 3;
			Plugin_Done(ErrNum);
			CloseShardSocket();
			SuspendThread(hMainThread);
			CleanJournal();
			CmdKillAll();
			RemoveAllObjs();
			if(hThread)
			{
				TerminateThread(hThread, 0);
				hThread = 0;
			}
			LogClose();
			TerminateThread(hMainThread, 0);
			hMainThread = 0;
			LogPrint(3, "[THREAD] Main thread finalizada\n");
			IsConnected = FALSE;
			return FALSE;
		break;
		}
		return FALSE;
		break;
	case WM_USER+1:
		switch(lParam)
		{
			case WM_LBUTTONDOWN:
				Shell_NotifyIcon(NIM_DELETE, &Notify);
				ShowWindow(MainWnd, SW_SHOW);
				return FALSE;
			break;
		}
		return FALSE;
	break;
	case WM_CLOSE:
		LogPrint(3, "[UOBOT] Closing\n");
		EndDialog(hDlg, 0);
		return FALSE;
	break;
	case WM_VSCROLL:
		return DefWindowProc(hDlg, wMsg, wParam, lParam);
	break;/*
	case WM_PAINT:
		return DefWindowProc(hDlg, wMsg, wParam, lParam);
	break;
	case WM_NCPAINT:
		return DefWindowProc(hDlg, wMsg, wParam, lParam);
	break;
	case WM_CAPTURECHANGED:
		return DefWindowProc(hDlg, wMsg, wParam, lParam);
	break;
	case WM_SYSCOMMAND:
		return DefWindowProc(hDlg, wMsg, wParam, lParam);
	break;*/
	default:
		break;
	}
	return FALSE;
}

//int NumLines = 0;
void TextBoxCat(HWND Dlg, int DlgItem, const char *Text, ...)
{
	char Final[8000];
	va_list List;
	char *NewText = NULL;
	unsigned int len=0;
	unsigned int NewTextSize = 0;
	DWORD sc_min=0,sc_max=0,counter=0;
	int lastPos; /*first visible line of control*/ 
	HWND hwLog = GetDlgItem(MainWnd, ID_LOG);

	if(Dlg == 0)
	{
		Dlg = MainWnd;
		DlgItem = ID_LOG;
	}

	va_start(List, Text);
	vsprintf((char*)Final, Text, List);
	va_end(List);

   /* save current position before editing */ 
   lastPos = (int) SendDlgItemMessage(Dlg, DlgItem, (UINT) EM_GETFIRSTVISIBLELINE, (WPARAM) 0, (LPARAM) 0); 

	/* get the dlg text, make sure we have enough space for it */
/*	NewText = malloc(len);
	memcpy(NewText, Text, len);
	NewTextSize += 100;
	NewText = (char*)realloc(NewText, NewTextSize+len);
	
	while(GetDlgItemText(Dlg, DlgItem, NewText+len, NewTextSize) == (unsigned int)NewTextSize - 1)
	{
		if(NewTextSize > MAXTEXTSIZE) break;
		NewTextSize += 100;
		NewText = (char*)realloc(NewText, NewTextSize+len);
	}*/
	
	len=strlen(Text);
	NewTextSize = GetWindowTextLength(hwLog);
	NewText = malloc(NewTextSize+len+1);
	GetDlgItemText(Dlg, DlgItem, NewText, NewTextSize+1);
	memcpy(NewText+NewTextSize, Text, len+1);

	if(NewTextSize+len+1 > 7000)
		SetDlgItemText(Dlg, ID_LOG, NewText+((NewTextSize+len+1) - 7000));
	else
		SetDlgItemText(Dlg, ID_LOG, NewText);

	/*if(!AutoVScroll)
	{
		//x= GetScrollPos*NumLines/GetScrollRange()
		counter = GetScrollPos(Dlg, SB_VERT);
		counter = counter * SendDlgItemMessage(Dlg, DlgItem, (UINT) EM_GETLINECOUNT,(WPARAM) 0, (LPARAM) 0);
		GetScrollRange(Dlg, SB_VERT, &sc_min, &sc_max);
		counter = counter / sc_max;
	}*/

	SetDlgItemText(Dlg, ID_LOG, NewText);

	
	if(!AutoVScroll)
	{
		SendDlgItemMessage(Dlg, DlgItem, (UINT) EM_LINESCROLL, (WPARAM) 0, (LPARAM) lastPos);
	}
	else
	{
		counter = (int) SendDlgItemMessage(Dlg, DlgItem, (UINT) EM_GETLINECOUNT,(WPARAM) 0, (LPARAM) 0);
		SendDlgItemMessage(Dlg, DlgItem, (UINT) EM_LINESCROLL, (WPARAM) 0, (LPARAM) counter);
	}

	free(NewText);

	return;
}

void TextBoxCat3(HWND Dlg, int DlgItem, const char *Text, ...) 
{ 
	char Final[4096]; 
	va_list List; 
	
	char *NewText = NULL; 
    unsigned int NewTextSize = 0; 
    unsigned int len=0;
	HWND hwLog = GetDlgItem(MainWnd, ID_LOG);
	int counter; /*line counter*/ 
	int lastPos; /*first visible line of control*/ 
	
	va_start(List, Text); 
   vsprintf((char*)Final, Text, List); 
   va_end(List); 
  
   /* save current position before editing */ 
   lastPos = (int) SendDlgItemMessage(Dlg, DlgItem, (UINT) EM_GETFIRSTVISIBLELINE, (WPARAM) 0, (LPARAM) 0); 
    
   /* get the dlg text, make sure we have enough space for it */ 
	len=strlen(Text);
	NewTextSize = GetWindowTextLength(hwLog);
	NewText = malloc(NewTextSize+len+1);
	GetDlgItemText(Dlg, DlgItem, NewText, NewTextSize+1);
	memcpy(NewText+NewTextSize, Text, len+1);

	if(NewTextSize+len+1 > 7000)
		SetDlgItemText(Dlg, ID_LOG, NewText+((NewTextSize+len+1) - 7000));
	else
		SetDlgItemText(Dlg, ID_LOG, NewText);

   if (AutoVScroll) 
   { 
      counter = (int) SendDlgItemMessage(Dlg, DlgItem, (UINT) EM_GETLINECOUNT,(WPARAM) 0, (LPARAM) 0); 
      SendDlgItemMessage(Dlg, DlgItem, (UINT) EM_LINESCROLL, (WPARAM) 0, (LPARAM) counter); 
   } 
   else 
   { 
      SendDlgItemMessage(Dlg, DlgItem, (UINT) EM_LINESCROLL, (WPARAM) 0, (LPARAM) lastPos); 
   } 
    
   free(NewText); 
   return; 
}

void ConnectBot()
{
	if(login[0]==0||senha[0]==0||charname[0]==0)
	{
		MBOut("Erro", "Senha/Loguin/Charname == 0");
		return;
	}
	if(hMainThread!=0)
	{
		MBOut("Erro", "Voce precisa primeiro desconectar");
		return;
	}
	if((hMainThread = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)StartThread, 0, 0, (LPDWORD)&hMainThread))==NULL)
	{
		MBOut("Erro", "Nao foi possivel iniciar a thread do uobot");
		return;
	}
	//MBOut("OK","L:%s -=- P:%s -=- C:%s",login,senha,charname);
}

DWORD WINAPI StartThread(LPVOID lpParameter)
{
	Start(login, senha, charname);
	return 0;
}

int AddCommand(const char *Name, void *Handler)
{
	unsigned int i = 0;
	char *NewName = malloc(strlen(Name)+2);
	sprintf(NewName, "/%s", Name);
	/* can't redefine a command */
	for(i = 0; i < CmdListSize; i++)
	{
		if(!strcmp(Commands[i].Command, Name))
		{
			LogPrint(1, "[ERROR] Comando %s ja existe\n", NewName);
			return 0;
		}
	}

	/* name must always be present. and either alias or callback */
	if(Name == NULL || Handler == NULL)
		return 0;

	CmdListSize++;
    Commands = (Cmd*)realloc(Commands, CmdListSize*sizeof(Cmd));
	Commands[CmdListSize - 1].Place = NULL;
	strcpy(Commands[CmdListSize - 1].Command, NewName);
	Commands[CmdListSize - 1].Handler = (CommandHandler)Handler;
	LogPrint(3, "[CMD] Comando %s adicionado\n", Commands[CmdListSize - 1].Command);
	return 1;
}

void HandleCmd(char *Text)
{
	HandleCommand(Text, 0);
}

void HandleCommand(char *Text, int Flag)
{
	char *TokenBegin = NULL, *TokenEnd = NULL;
	unsigned int Idx = -1, i = 0;
	int InQuote = 0, Len = 0;
	char **Args = NULL;
	unsigned int WordCount=0,x=0;
	char buff[200];

	/*
	* separate the command into words. the first being the command
	* ' ' is a word separator and '\'' unifies a word
	*/
	if(strlen(Text) == 0)
		return;

	/* parse the command and it's arguments */
	while(1)
	{
		if(TokenBegin && Text[i] == '\0')
		{
			TokenEnd = Text + i;
			WordCount++;
			Args = (char**)realloc(Args, WordCount*sizeof(char**));
			Len = (int)(TokenEnd - TokenBegin);
			Args[WordCount-1] = (char*)malloc(Len + 1);
            strncpy(Args[WordCount-1], TokenBegin, Len);
			Args[WordCount-1][Len] = '\0';
			break;
		}
		else if(Text[i] == '\0') /* if the text is simply over, stop */
			break;
        if(TokenBegin == NULL)
		{
			while(1)
			{
				if(Text[i] == '\'' || Text[i] == ' ' || Text[i] == '\"')
					i++;
				else /* found a non-token (text) */
					break;
			}

			/*
			* set the start of the text and
			* check if its a quote (allow spaces in the text)
			*/
			TokenBegin = Text + i;
			if(i != 0 && (Text[i-1] == '\'' || Text[i-1] == '\"'))
				InQuote = 1;
			else
				InQuote = 0;
		}
		if(TokenBegin && InQuote == 0 && Text[i] == ' ')
		{
			TokenEnd = Text + i;
			WordCount++;
			/* allocate one more pointer */
			Args = (char**)realloc(Args, WordCount*sizeof(char**));
			/* allocate space for the text (the word) */
			Len = (int)(TokenEnd - TokenBegin);
			Args[WordCount-1] = (char*)malloc(Len + 1);
            /* copy the text */
            strncpy(Args[WordCount-1], TokenBegin, Len);
			Args[WordCount-1][Len] = '\0';

			TokenBegin = NULL;
		}

		/* if we are expecting quotes and this is one, the space between them is a word */
		if(TokenBegin && InQuote == 1 && (Text[i] == '\'' || Text[i] == '\"'))
		{
			TokenEnd = Text + i;
			WordCount++;
			/* allocate one more pointer */
			Args = (char**)realloc(Args, WordCount*sizeof(char**));
			/* allocate space for the text (the word) */
			Len = (int)(TokenEnd - TokenBegin);
			Args[WordCount-1] = (char*)malloc(Len + 1);
            /* copy the text */
            strncpy(Args[WordCount-1], TokenBegin, Len);
			Args[WordCount-1][Len] = '\0';

			TokenBegin = NULL;
		}

		/* check the next character */
		i++;
	}

	if(Flag)
	{
		if(WordCount<5)
		{
			//LogPrint(3, "Count = %d\n", WordCount);
			LogPrint(3, "Uso: loguin senha charname plugin.dll macro macroargs\n");
			for(x=0; x < WordCount; x++)
			{
				free(Args[x]);
			}
			free(Args);
			return;
		}
		strcpy(login, Args[0]);
		strcpy(senha, Args[1]);
		strcpy(charname, Args[2]);
		sprintf(buff, "/loaddll %s", Args[3]);
		HandleCommand(buff, 0);
		sprintf(buff, "/%s %s", Args[4], strstr(Text, Args[4])+strlen(Args[4]));
		HandleCommand(buff, 0);
		SetDlgItemText(CurWnd, EDIT_LOGIN, login);					
		SetDlgItemText(CurWnd, EDIT_SENHA, senha);
		SetDlgItemText(CurWnd, EDIT_CHARNAME, charname);
		ConnectBot();
		return;
	}
    for(i = 0; i < CmdListSize; i++)
	{
		if(!strcmp(Commands[i].Command, Args[0]))
            Idx = i;
	}

    if(Idx == -1)
		LogPrint(1,"[ERROR] Commando %s nao encontrado\n", Args[0]);
	else
		MakeCmdThread(Idx, Args, WordCount);

	return;
}








