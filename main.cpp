#include <tchar.h>
#include "GeometryScreenSaver.h"
#include "E:\Codes\UseVisualStyle.h"
// WinMain関数
int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE hPvI, LPTSTR lpCmdLine, int nCmdShow)
{
	SetThreadUILanguage(GetUserDefaultUILanguage());
	GeometryScreenSaver saver;
	if (lstrcmp(lpCmdLine, TEXT("/S")) == 0 || lstrcmp(lpCmdLine, TEXT("/s")) == 0)
		return saver.Run();
	else return saver.Settings(hInstance);
}