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
	int bgLeft, bgTop, bgRight, bgBottom;//����ͼ��ر���
	int timeX, timeY;//ʱ����ر���
	time_t temptimet;
	tm temptm;
	std::vector<Shapes*>shapes;
	int countOfShapes[8];//ͼ�ε�������0=Բ��1=�����Σ�2=�����Σ�3=����Σ�4��7=N���������
#ifdef _UNICODE
	std::wstring timeFormatString;
#else
	std::string timeFormatString;
#endif
};

INT_PTR CALLBACK DialogCallback(HWND, UINT, WPARAM, LPARAM);
