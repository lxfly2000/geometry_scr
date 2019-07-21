#pragma once
#include <DxLib.h>
#include <vector>
#include <string>
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
	void OnCommandCoverColor();
	void OnCommandCoverSplashBrowse();
	void OnCommandPureColorBorder();
	void OnCommandTimeColor();
	void OnCommandTimeFont();
	void OnCommandAbout();
	void OnCommandLoadDefaultSettings();
	void OnCommandVisit();
	void OnSliderCustomDraw(UINT_PTR);
	void OnCommandClearBackground();
	void OnCommandClearCover();
	void OnCommandBackgroundUseScreen();
	void OnCommandCoverUseScreen();
	void OnCommandTimeBorderColor();

	int CallbackFunction(HWND, UINT, WPARAM, LPARAM);
private:
	void CopySettingsToDialog();
	void CopyDialogToSettings();
	BOOL OpenFileDialog(wchar_t*);
	BOOL ChooseColorDialog(UINT*, const UINT);
	BOOL ChooseFontDialog();
	void FitBorderRect(int borderW, int borderH, int imageW, int imageH, int*l, int*t, int*r, int*b);
	void DrawTrianglePolygon(int cx, int cy, int radius, int rot_rad, int r, int g, int b, int num, int thick);
	POINT vbuffer[5];
	HWND hDialog;
	ScrSettings settings;
	TCHAR settingsFileName[MAX_PATH], appname[32];
	DEVMODE dm;
	bool fullscreen;

	void LoadSourceFromFiles();
	void DrawBackgroundSplash();
	void DrawShapes(bool fill);
	void DrawCoverLayer();
	void DrawDateTime();
	void SetDigit(LPCTSTR str, size_t slen);
	void ShapesAct();
	UINT dxBackgroundImage;//����ͼ���
	int bgLeft, bgTop, bgRight, bgBottom;//����ͼ��ر���
	UINT dxCoverImage;//���ǲ�ͼ���
	int bgCoverLeft, bgCoverTop, bgCoverRight, bgCoverBottom;//���ǲ�ͼ��ر���
	int hScreenCover;//���ǲ���Ļ���
	int timeX, timeY;//ʱ����ر���
	int hImgWhiteFill;
	int screenWidth, screenHeight;
	time_t temptimet,temptimet_last;
	size_t digit_index;
	tm temptm;
	std::vector<Shapes*>shapes;
	int countOfShapes[8];//ͼ�ε�������0=Բ��1=�����Σ�2=�����Σ�3=����Σ�4��7=N���������
	int dxScreenShot;
	HBITMAP hScreenShot;
};

INT_PTR CALLBACK DialogCallback(HWND, UINT, WPARAM, LPARAM);
