#include <tchar.h>
#include "GeometryScreenSaver.h"
#include "UseVisualStyle.h"
// WinMain関数
int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE hPvI, LPTSTR lpCmdLine, int nCmdShow)
{
	SetThreadUILanguage(GetUserDefaultUILanguage());
	GeometryScreenSaver saver;
	if (lstrlen(lpCmdLine) == 0)
		saver.Settings(hInstance);
	else switch (lpCmdLine[1])
	{
	case TEXT('c'):case TEXT('C'):saver.Settings(hInstance); break;
	case TEXT('t'):case TEXT('T'):saver.Run(); break;
	case TEXT('p'):case TEXT('P'):break;
	case TEXT('a'):case TEXT('A'):break;
	case TEXT('s'):case TEXT('S'):saver.Run(); break;
	}
	return 0;
}
