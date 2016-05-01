#pragma once
#include <DxLib.h>
#include <vector>
#include "ScrSettings.h"
#include "Shapes.h"

class GeometryScreenSaver
{
public:
	GeometryScreenSaver();
	~GeometryScreenSaver();
	int Run();
	int Settings(HINSTANCE);

	void SaveSettingsToFile();
	void LoadSettingsFromFile();
	void ConfigureDialogControl();

	virtual void OnShowWindow(HWND);
	virtual void OnOK();
	virtual void OnCancel();
	virtual void OnDropFiles(HDROP);
	void OnPressF11();
	void ChangeFullscreenSettings(bool);
	void OnCheckShowTimeFormat(UINT nID);
	void OnEditChangeMinMaxRadius(int);
	void OnCommandBackgroundColor();
	void OnCommandBackgroundSplashBrowse();
	void OnCommandPureColorBorder();
	void OnCommandTimeColor();
	void OnCommandTimeFont();
	void OnCommandAbout();
	void OnCommandLoadDefaultSettings();
	void OnCommandVisit();
	void OnSliderCustomDraw(UINT_PTR);

	int CallbackFunction(HWND, UINT, WPARAM, LPARAM);
private:
	void CopySettingsToDialog();
	void CopyDialogToSettings();
	BOOL OpenFileDialog();
	BOOL ChooseColorDialog(UINT*, const UINT);
	BOOL ChooseFontDialog();
	HWND hDialog;
	ScrSettings settings;
	TCHAR settingsFileName[MAX_PATH], appname[32];
	DEVMODE dm;
	bool fullscreen;

	void LoadSourceFromFiles();
	void DrawBackgroundSplash();
	void DrawShapes();
	void DrawDateTime();
	void ShapesAct();
	UINT dxBackgroundImage;
	int bgLeft, bgTop, bgRight, bgBottom;//背景图相关变量
	int timeX, timeY;//时间相关变量
	time_t temptimet;
	tm temptm;
	std::vector<Shapes*>shapes;
	int countOfShapes[8];//图形的数量，0=圆，1=三角形，2=正方形，3=五边形，4～7=N边形最多数
#ifdef _UNICODE
	std::wstring timeFormatString;
#else
	std::string timeFormatString;
#endif
};

INT_PTR CALLBACK DialogCallback(HWND, UINT, WPARAM, LPARAM);
