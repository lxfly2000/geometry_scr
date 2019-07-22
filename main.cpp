#include <tchar.h>
#include<sal.h>
#include "GeometryScreenSaver.h"
#include "UseVisualStyle.h"
// WinMain関数
int WINAPI _tWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPvI, _In_ LPTSTR lpCmdLine, _In_ int nCmdShow)
{
	SetThreadUILanguage(GetUserDefaultUILanguage());
	GeometryScreenSaver saver;
	HWND hParent = NULL;
	if (lstrlen(lpCmdLine) > 3)
		hParent = (HWND)_ttoi(lpCmdLine + 3);
	if (lstrlen(lpCmdLine) == 0)
		saver.Settings(hInstance, hParent);//在文件的右键菜单上选择配置屏保
	else switch (lpCmdLine[1])
	{//https://support.microsoft.com/en-us/help/182383/info-screen-saver-command-line-arguments
	case TEXT('c')://在屏保设置对话框里的配置屏保
	case TEXT('C'):saver.Settings(hInstance, hParent); break;
	case TEXT('p')://在屏保设置对话框里的屏保预览
	case TEXT('P'):saver.Run(hParent); break;
	case TEXT('a'):case TEXT('A'):break;//退出屏保时要求输入密码（仅Win95）
	case TEXT('s')://在屏保设置对话框里点击预览按钮后的预览，系统空闲超时后的屏保运行
	case TEXT('S')://在文件上预览
		saver.Run(hParent);
		break;
	}
	return 0;
}
