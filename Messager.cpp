//---------------------------------------------------------------------------
//#define WINVER 0x0400
//#define _WIN32_NT 0x0400
#define MB_SERVICE_NOTIFICATION 0x00040000L

#include <windows.h>
#include <commctrl.h>
#include <stdio.h>
#include <string.h>
#include "resource.h"
#include "Sound.h"

// Взято из нового Platform SDK
#define ICC_STANDARD_CLASSES   0x00004000

//---------------------------------------------------------------------------
#define MAX_TEXT      1024
#define MAX_TITLE       32
#define MAX_ICON         5

#define QUIT_TIMEOUT  1000

static int IconTable[] = { IDI_MESSAGER, IDI_ALARM, IDI_MOVIE };

static void __fastcall ErrorBox(void)
{
	char text[MAX_TEXT],
		title[MAX_TITLE];
	LoadString(0, ERR_FORMAT, text, sizeof(text));
	LoadString(0, IDS_ERRORTITLE, title, sizeof(title));
	MessageBox(0, text, title, MB_ICONSTOP);
}

static const    int  Sequence[] = { 200, 300, 400, 300, 700, 500 };
static          int  pos        = 0;
static volatile bool Terminate  = false;

DWORD WINAPI ThreadProc(void *)
{
	OSVERSIONINFO ver;
	ver.dwOSVersionInfoSize = sizeof(ver);
	GetVersionEx(&ver);
	register bool WinNT = ver.dwPlatformId == VER_PLATFORM_WIN32_NT;

	while (!Terminate)
	{
		if (Sequence[pos] != 0)
			if (WinNT)
				Beep(Sequence[pos], Sequence[pos + 1]);
			else // if (WinNT)
			{
				Sound(Sequence[pos]);
				Sleep(Sequence[pos + 1]);
				NoSound();
			} // if (WinNT) else
		else // if (Sequence[pos] != 0)
			Sleep(Sequence[pos + 1]);
		pos += 2;
		if (pos == 6)
			pos = 0;
	} // while (!Terminate)
	
	return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR CmdLine, int)
{
	INITCOMMONCONTROLSEX icc;
	icc.dwSize = sizeof(icc);
	icc.dwICC  = ICC_STANDARD_CLASSES;
	InitCommonControlsEx(&icc);

	// Ищем сообщение в командной строке
	char *Start = strchr(CmdLine, '\"');
	if (!Start)
	{
		ErrorBox();
		return 0xFF;
	} // if (!Start)
	char *pos = strchr(Start += 1, '\"');
	if (!pos)
	{
		ErrorBox();
		return 0xFF;
	} // if (!pos)
	*pos = 0;
	
	
	// Проверяем, есть ли заголовок
	char *TtlStart = 0;
	if (*(++pos))
	{
		TtlStart = strchr(pos, '\"');
		if (TtlStart)
		{
			pos = strchr(TtlStart += 1, '\"');
			if (!pos)
			{
				ErrorBox();
				return 0xFF;
			} // if (!pos)
			*pos = 0;
		} // if (TtlStart)
	} // if (*(++pos))

	// Проверяем наличие значка в командной строке
	int Icon = -1;
	if (*(++pos))
	{
		if (!sscanf(pos, "%i", &Icon))
			Icon = -1;
	}

	// Проверяем наличие параметра nosound
	bool NoSound = strstr(_strlwr(pos), "nosound") != NULL;
	
	if (Icon > MAX_ICON || Icon < 0)
		Icon = 0;
		

	DWORD  ThreadId;
	HANDLE hThread = NULL;

	if (!NoSound)
		hThread = CreateThread(0, 0, ThreadProc, 0, 0, &ThreadId);
		
		
	MSGBOXPARAMS Info;
	Info.cbSize = sizeof(MSGBOXPARAMS);
	Info.hwndOwner = GetDesktopWindow();
	Info.hInstance = hInstance;
	Info.lpszText = Start;
	Info.lpszCaption = TtlStart ? TtlStart : MAKEINTRESOURCE(IDS_TITLE);
	Info.dwStyle = MB_OK | MB_SETFOREGROUND | MB_TOPMOST | (Icon == 0 ?
		MB_ICONASTERISK : (Icon == 1 ? MB_ICONEXCLAMATION :
	(Icon == 2 ? MB_ICONHAND : MB_USERICON))) | MB_SERVICE_NOTIFICATION;
	Info.lpszIcon = MAKEINTRESOURCE(IconTable[Icon - 3]);
	Info.dwContextHelpId = 1;
	Info.lpfnMsgBoxCallback = 0;
	Info.dwLanguageId = MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT);
	if (Icon > 2)
		MessageBeep(MB_OK);
	MessageBoxIndirect(&Info);
		
	Terminate = true;
	
	if (hThread)
	{
		WaitForSingleObject(hThread, QUIT_TIMEOUT);
		CloseHandle(hThread);
	}
		
	return 0;
}
