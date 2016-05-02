#include "GeometryScreenSaver.h"
#include "resource.h"
#include <Shlwapi.h>
#include <Windows.h>
#include <ctime>
#include <cmath>

#define DegToRad(d) ((d)*3.141592653545/180)

static GeometryScreenSaver *global_pGSS = nullptr;
static COLORREF global_colorRef[16] = { 0 };
static LOGFONT global_logfont = { 0 };
TCHAR global_stringBuf[256];
const int weekstrindex[7] = { IDS_STRING_SUNDAY,IDS_STRING_MONDAY,IDS_STRING_TUESDAY,
IDS_STRING_WEDNESDAY,IDS_STRING_THURSDAY,IDS_STRING_FRIDAY,IDS_STRING_SATURDAY };

TCHAR *GetStringFromResource(UINT nID)
{
	if (!LoadString(GetModuleHandle(NULL), nID, global_stringBuf, sizeof(global_stringBuf) / sizeof(*global_stringBuf)))
		wsprintf(global_stringBuf, GetStringFromResource(IDS_STRING_RESOURCEID_NOTFOUND), nID);
	return global_stringBuf;
}

TCHAR *GetStringOfValue(int value, LPCTSTR formatStr = TEXT("%d"))
{
	wsprintf(global_stringBuf, formatStr, value);
	return global_stringBuf;
}

GeometryScreenSaver::GeometryScreenSaver() :fullscreen(true), hDialog(nullptr)
{
	if (global_pGSS)
		delete global_pGSS;
	global_pGSS = this;
	GetCurrentDirectory(sizeof(settingsFileName) / sizeof(*settingsFileName), settingsFileName);
	PathCombine(settingsFileName, settingsFileName, GetStringFromResource(IDS_STRING_FILENAME_SETTINGS));
	lstrcpy(appname, GetStringFromResource(IDS_STRING_APPNAME));
}

GeometryScreenSaver::~GeometryScreenSaver()
{
	global_pGSS = nullptr;
}

int GeometryScreenSaver::Run()
{
#ifndef _DEBUG
	SetOutApplicationLogValidFlag(FALSE);
#endif
	ZeroMemory(countOfShapes, sizeof(countOfShapes));
	LoadSettingsFromFile();
	if (settings.randSeed == 0)
	{
		settings.randSeed = (int)time(NULL);
		SetWindowText(GetStringOfValue(settings.randSeed));
	}
	SRand(settings.randSeed);

	dm.dmSize = sizeof(DEVMODE);
	EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &dm);
	ChangeFullscreenSettings(true);

	// ＤＸライブラリ初期化処理
	if (DxLib_Init() == -1) return -1;

	LoadSourceFromFiles();

	// 描画先画面を裏画面にする
	SetDrawScreen(DX_SCREEN_BACK);

	while (1)
	{
		// x , y が示す画面座標にＢＭＰ画像 test1.bmp を描画する
		DrawBackgroundSplash();
		DrawShapes();
		DrawDateTime();

		// 裏画面の内容を表画面に反映させる
		ScreenFlip();

		// 上下左右のキー入力に対応して x, y の座標値を変更する
		if (CheckHitKey(KEY_INPUT_F11)) {
			OnPressF11();
			LoadSourceFromFiles();
		}
		// ＥＳＣキーが押されたらループから抜ける
		if (CheckHitKey(KEY_INPUT_ESCAPE))break;
		if (GetAsyncKeyState(VK_LBUTTON))break;
		if (GetAsyncKeyState(VK_RBUTTON))break;

		// Windows システムからくる情報を処理する
		if (ProcessMessage() == -1) break;

		// 画面に描かれているものをすべて消す
		ClearDrawScreen();

		// 待たないと処理が早すぎるのでここで２０ミリ秒待つ
		//WaitTimer(200);

		ShapesAct();
	}
	for (int i = (int)shapes.size() - 1; i >= 0; i--)
		delete shapes.at(i);
	shapes.clear();

	// ＤＸライブラリ使用の終了処理
	DxLib_End();

	// ソフトの終了
	return 0;
}

void GeometryScreenSaver::LoadSourceFromFiles()
{
	dxBackgroundImage = LoadGraph(settings.backgroundSplash);
	GetGraphSize(dxBackgroundImage, &bgRight, &bgBottom);
	if (!bgRight)bgRight = 1;
	if (!bgBottom)bgBottom = 1;
	if (fullscreen)
	{
		if (settings.fitBorder)
		{
			if (dm.dmPelsHeight / dm.dmPelsWidth > (unsigned)(bgRight / bgBottom))
			{
				bgLeft = 0;
				bgTop = (dm.dmPelsHeight - dm.dmPelsWidth*bgBottom / bgRight) / 2;
				bgRight = dm.dmPelsWidth;
				bgBottom = dm.dmPelsHeight - bgTop;
			}
			else
			{
				bgTop = 0;
				bgLeft = (dm.dmPelsWidth - dm.dmPelsHeight*bgRight / bgBottom) / 2;
				bgBottom = dm.dmPelsHeight;
				bgRight = dm.dmPelsWidth - bgLeft;
			}
		}
		else
		{
			bgLeft = (dm.dmPelsWidth - bgRight) / 2;
			bgTop = (dm.dmPelsHeight - bgBottom) / 2;
			bgRight += bgLeft;
			bgBottom += bgTop;
		}
		timeX = dm.dmPelsWidth*settings.timePosX / 100;
		timeY = dm.dmPelsHeight*settings.timePosY / 100;
	}
	else
	{
		if (settings.fitBorder)
		{
			if (settings.windowHeight / settings.windowWidth > (unsigned)(bgRight / bgBottom))
			{
				bgLeft = 0;
				bgTop = (settings.windowHeight - settings.windowWidth*bgBottom / bgRight) / 2;
				bgRight = settings.windowWidth;
				bgBottom = settings.windowHeight - bgTop;
			}
			else
			{
				bgTop = 0;
				bgLeft = (settings.windowWidth - settings.windowHeight*bgRight / bgBottom) / 2;
				bgBottom = settings.windowHeight;
				bgRight = settings.windowWidth - bgLeft;
			}
		}
		else
		{
			bgLeft = (settings.windowWidth - bgRight) / 2;
			bgTop = (settings.windowHeight - bgBottom) / 2;
			bgRight += bgLeft;
			bgBottom += bgTop;
		}
		timeX = settings.windowWidth*settings.timePosX / 100;
		timeY = settings.windowHeight*settings.timePosY / 100;
	}
	SetBackgroundColor(settings.backgroundColor & 0x000000FF, (settings.backgroundColor & 0x0000FF00) >> 8,
		(settings.backgroundColor & 0x00FF0000) >> 16);
	SetFontSize(settings.timeFontSize);
	SetFontThickness(settings.timeFontThickness);
	ChangeFont(settings.timeFontName);
	ChangeFontType(DX_FONTTYPE_ANTIALIASING);
}

void GeometryScreenSaver::DrawBackgroundSplash()
{
	if (dxBackgroundImage != -1)
		DrawExtendGraph(bgLeft, bgTop, bgRight, bgBottom, dxBackgroundImage, TRUE);
}

void GeometryScreenSaver::DrawDateTime()
{
	time(&temptimet);
	localtime_s(&temptm, &temptimet);
	timeFormatString.clear();
	if (settings.GetScrTimeFormat(1))
	{
		timeFormatString.append(GetStringOfValue(temptm.tm_year + 1900));
		timeFormatString.append(TEXT("-"));
	}
	if (settings.GetScrTimeFormat(2))
	{
		timeFormatString.append(GetStringOfValue(temptm.tm_mon + 1));
		timeFormatString.append(TEXT("-"));
		timeFormatString.append(GetStringOfValue(temptm.tm_mday));
		timeFormatString.append(TEXT(" "));
	}
	if (settings.GetScrTimeFormat(3))
	{
		timeFormatString.append(GetStringOfValue(temptm.tm_hour, TEXT("%2d")));
		timeFormatString.append(TEXT(":"));
		timeFormatString.append(GetStringOfValue(temptm.tm_min, TEXT("%02d")));
	}
	if (settings.GetScrTimeFormat(4))
	{
		timeFormatString.append(TEXT(":"));
		timeFormatString.append(GetStringOfValue(temptm.tm_sec, TEXT("%02d")));
	}
	if (settings.GetScrTimeFormat(5))
	{
		timeFormatString.append(TEXT(" "));
		timeFormatString.append(GetStringFromResource(weekstrindex[temptm.tm_wday]));
	}
	DrawFormatString(timeX, timeY, GetColor(settings.timeColor & 0x000000FF, (settings.timeColor & 0x0000FF00) >> 8,
		(settings.timeColor & 0x00FF0000) >> 16), timeFormatString.c_str());
}

void GeometryScreenSaver::DrawShapes()
{
	for (int i = (int)shapes.size() - 1; i >= 0; i--)
		switch (shapes.at(i)->polygons)
		{
		case 0://圆
			DrawCircle(shapes.at(i)->x, shapes.at(i)->y, shapes.at(i)->radius, GetColor(shapes.at(i)->color[0],
				shapes.at(i)->color[1], shapes.at(i)->color[2]), 0, settings.borderWidth);
			break;
		case 1://三角形
			DrawTriangle((int)(shapes.at(i)->radius*sin(DegToRad(shapes.at(i)->angle)) + shapes.at(i)->x),
				(int)(shapes.at(i)->radius*cos(DegToRad(shapes.at(i)->angle)) + shapes.at(i)->y),
				(int)(shapes.at(i)->radius*sin(DegToRad(shapes.at(i)->angle + 120)) + shapes.at(i)->x),
				(int)(shapes.at(i)->radius*cos(DegToRad(shapes.at(i)->angle + 120)) + shapes.at(i)->y),
				(int)(shapes.at(i)->radius*sin(DegToRad(shapes.at(i)->angle + 240)) + shapes.at(i)->x),
				(int)(shapes.at(i)->radius*cos(DegToRad(shapes.at(i)->angle + 240)) + shapes.at(i)->y),
				GetColor(shapes.at(i)->color[0], shapes.at(i)->color[1], shapes.at(i)->color[2]), 0);
			break;
		case 2://正方形
			DrawLine((int)(shapes.at(i)->radius*sin(DegToRad(shapes.at(i)->angle)) + shapes.at(i)->x),
				(int)(shapes.at(i)->radius*cos(DegToRad(shapes.at(i)->angle)) + shapes.at(i)->y),
				(int)(shapes.at(i)->radius*sin(DegToRad(shapes.at(i)->angle + 90)) + shapes.at(i)->x),
				(int)(shapes.at(i)->radius*cos(DegToRad(shapes.at(i)->angle + 90)) + shapes.at(i)->y),
				GetColor(shapes.at(i)->color[0], shapes.at(i)->color[1], shapes.at(i)->color[2]), settings.borderWidth);
			DrawLine((int)(shapes.at(i)->radius*sin(DegToRad(shapes.at(i)->angle + 90)) + shapes.at(i)->x),
				(int)(shapes.at(i)->radius*cos(DegToRad(shapes.at(i)->angle + 90)) + shapes.at(i)->y),
				(int)(shapes.at(i)->radius*sin(DegToRad(shapes.at(i)->angle + 180)) + shapes.at(i)->x),
				(int)(shapes.at(i)->radius*cos(DegToRad(shapes.at(i)->angle + 180)) + shapes.at(i)->y),
				GetColor(shapes.at(i)->color[0], shapes.at(i)->color[1], shapes.at(i)->color[2]), settings.borderWidth);
			DrawLine((int)(shapes.at(i)->radius*sin(DegToRad(shapes.at(i)->angle + 180)) + shapes.at(i)->x),
				(int)(shapes.at(i)->radius*cos(DegToRad(shapes.at(i)->angle + 180)) + shapes.at(i)->y),
				(int)(shapes.at(i)->radius*sin(DegToRad(shapes.at(i)->angle + 270)) + shapes.at(i)->x),
				(int)(shapes.at(i)->radius*cos(DegToRad(shapes.at(i)->angle + 270)) + shapes.at(i)->y),
				GetColor(shapes.at(i)->color[0], shapes.at(i)->color[1], shapes.at(i)->color[2]), settings.borderWidth);
			DrawLine((int)(shapes.at(i)->radius*sin(DegToRad(shapes.at(i)->angle + 270)) + shapes.at(i)->x),
				(int)(shapes.at(i)->radius*cos(DegToRad(shapes.at(i)->angle + 270)) + shapes.at(i)->y),
				(int)(shapes.at(i)->radius*sin(DegToRad(shapes.at(i)->angle)) + shapes.at(i)->x),
				(int)(shapes.at(i)->radius*cos(DegToRad(shapes.at(i)->angle)) + shapes.at(i)->y),
				GetColor(shapes.at(i)->color[0], shapes.at(i)->color[1], shapes.at(i)->color[2]), settings.borderWidth);
			break;
		case 3://五边形
			DrawLine((int)(shapes.at(i)->radius*sin(DegToRad(shapes.at(i)->angle)) + shapes.at(i)->x),
				(int)(shapes.at(i)->radius*cos(DegToRad(shapes.at(i)->angle)) + shapes.at(i)->y),
				(int)(shapes.at(i)->radius*sin(DegToRad(shapes.at(i)->angle + 72)) + shapes.at(i)->x),
				(int)(shapes.at(i)->radius*cos(DegToRad(shapes.at(i)->angle + 72)) + shapes.at(i)->y),
				GetColor(shapes.at(i)->color[0], shapes.at(i)->color[1], shapes.at(i)->color[2]), settings.borderWidth);
			DrawLine((int)(shapes.at(i)->radius*sin(DegToRad(shapes.at(i)->angle + 72)) + shapes.at(i)->x),
				(int)(shapes.at(i)->radius*cos(DegToRad(shapes.at(i)->angle + 72)) + shapes.at(i)->y),
				(int)(shapes.at(i)->radius*sin(DegToRad(shapes.at(i)->angle + 144)) + shapes.at(i)->x),
				(int)(shapes.at(i)->radius*cos(DegToRad(shapes.at(i)->angle + 144)) + shapes.at(i)->y),
				GetColor(shapes.at(i)->color[0], shapes.at(i)->color[1], shapes.at(i)->color[2]), settings.borderWidth);
			DrawLine((int)(shapes.at(i)->radius*sin(DegToRad(shapes.at(i)->angle + 144)) + shapes.at(i)->x),
				(int)(shapes.at(i)->radius*cos(DegToRad(shapes.at(i)->angle + 144)) + shapes.at(i)->y),
				(int)(shapes.at(i)->radius*sin(DegToRad(shapes.at(i)->angle + 216)) + shapes.at(i)->x),
				(int)(shapes.at(i)->radius*cos(DegToRad(shapes.at(i)->angle + 216)) + shapes.at(i)->y),
				GetColor(shapes.at(i)->color[0], shapes.at(i)->color[1], shapes.at(i)->color[2]), settings.borderWidth);
			DrawLine((int)(shapes.at(i)->radius*sin(DegToRad(shapes.at(i)->angle + 216)) + shapes.at(i)->x),
				(int)(shapes.at(i)->radius*cos(DegToRad(shapes.at(i)->angle + 216)) + shapes.at(i)->y),
				(int)(shapes.at(i)->radius*sin(DegToRad(shapes.at(i)->angle + 288)) + shapes.at(i)->x),
				(int)(shapes.at(i)->radius*cos(DegToRad(shapes.at(i)->angle + 288)) + shapes.at(i)->y),
				GetColor(shapes.at(i)->color[0], shapes.at(i)->color[1], shapes.at(i)->color[2]), settings.borderWidth);
			DrawLine((int)(shapes.at(i)->radius*sin(DegToRad(shapes.at(i)->angle + 288)) + shapes.at(i)->x),
				(int)(shapes.at(i)->radius*cos(DegToRad(shapes.at(i)->angle + 288)) + shapes.at(i)->y),
				(int)(shapes.at(i)->radius*sin(DegToRad(shapes.at(i)->angle)) + shapes.at(i)->x),
				(int)(shapes.at(i)->radius*cos(DegToRad(shapes.at(i)->angle)) + shapes.at(i)->y),
				GetColor(shapes.at(i)->color[0], shapes.at(i)->color[1], shapes.at(i)->color[2]), settings.borderWidth);
			break;
		}
}

void GeometryScreenSaver::ShapesAct()
{
	for (int i = (int)shapes.size() - 1; i >= 0; i--)
	{
		shapes.at(i)->x += shapes.at(i)->vx;
		shapes.at(i)->y += shapes.at(i)->vy;
		shapes.at(i)->angle += shapes.at(i)->vrotate;
		if (settings.transformingColor)
		{
			if (shapes.at(i)->crF2)//增/减
			{
				if (shapes.at(i)->color[shapes.at(i)->crF3 % 3u] < shapes.at(i)->crTransSpeed)//到底端
				{
					shapes.at(i)->crF1 ? shapes.at(i)->crF3++ : shapes.at(i)->crF3--;
					shapes.at(i)->crF2 = false;
				}
				else shapes.at(i)->color[shapes.at(i)->crF3 % 3u] -= shapes.at(i)->crTransSpeed;
			}
			else
			{
				if (shapes.at(i)->color[shapes.at(i)->crF3 % 3u] > 255 - shapes.at(i)->crTransSpeed)//到顶端
				{
					shapes.at(i)->crF1 ? shapes.at(i)->crF3++ : shapes.at(i)->crF3--;
					shapes.at(i)->crF2 = true;
				}
				else shapes.at(i)->color[shapes.at(i)->crF3 % 3u] += shapes.at(i)->crTransSpeed;
			}
		}
		if (shapes.at(i)->y - (int)settings.maxRadius > (int)(fullscreen ? dm.dmPelsHeight : settings.windowHeight))
		{
			countOfShapes[shapes.at(i)->polygons]--;
			delete shapes.at(i);
			shapes.erase(shapes.begin() + i);
		}
		else if ((shapes.at(i)->x < 0 && shapes.at(i)->vx < 0) ||
			((unsigned)shapes.at(i)->x>(fullscreen ? dm.dmPelsWidth : settings.windowWidth) && shapes.at(i)->vx>0))
			shapes.at(i)->vx = -shapes.at(i)->vx;
	}
	for (int i = 0; i < 4; i++)
		if (countOfShapes[i] < countOfShapes[4 + i] && GetRand(5) == 0)
		{
			Shapes *s = new Shapes(GetRand(fullscreen ? dm.dmPelsWidth : settings.windowWidth), 0 - settings.maxRadius,
				GetRand(settings.maxRadius - settings.minRadius) + settings.minRadius, GetRand(360),
				(int)(settings.maxHorizonalSpeed - GetRand(settings.maxHorizonalSpeed * 2)),
				GetRand(settings.maxFalldownSpeed - settings.minFalldownSpeed) + settings.minFalldownSpeed,
				GetRand(settings.maxRotateSpeed - settings.minRotateSpeed) + settings.minRotateSpeed,
				settings.multiColoredBorder ? GetRand(0x00FFFFFF) : GetColor(settings.pureColorBorder & 0x000000FF,
					(settings.pureColorBorder & 0x0000FF00) >> 8, (settings.pureColorBorder & 0x00FF0000) >> 16),
				i, GetRand(settings.colorTransMaxSpeed), GetRand(0x00FFFFFF));
			shapes.push_back(s);
			countOfShapes[i]++;
		}
}

int GeometryScreenSaver::Settings(HINSTANCE hInstance)
{
	LoadSettingsFromFile();
	if (DialogBox(hInstance, MAKEINTRESOURCE(IDD_DIALOG_SETTINGS), NULL, DialogCallback) == IDOK)
		SaveSettingsToFile();
	return 0;
}

void GeometryScreenSaver::SaveSettingsToFile()
{
	settings.SaveScrSettingsToFile(settingsFileName, appname);
}

void GeometryScreenSaver::LoadSettingsFromFile()
{
	settings.GetScrSettingsFromFile(settingsFileName, appname);
	countOfShapes[4] = settings.maxNCircle;
	countOfShapes[5] = settings.maxNTriangle;
	countOfShapes[6] = settings.maxNSquare;
	countOfShapes[7] = settings.maxNFivePolygons;
}

void GeometryScreenSaver::ConfigureDialogControl()
{
	SendMessage(GetDlgItem(hDialog, IDC_SPIN_MAXFALLDOWNSPEED), UDM_SETRANGE32, 0, 100);
	SendMessage(GetDlgItem(hDialog, IDC_SPIN_MAXHORIZONALSPEED), UDM_SETRANGE32, 0, 100);
	SendMessage(GetDlgItem(hDialog, IDC_SPIN_MAXNTRIANGLE), UDM_SETRANGE32, 0, 100);
	SendMessage(GetDlgItem(hDialog, IDC_SPIN_MAXNSQUARE), UDM_SETRANGE32, 0, 100);
	SendMessage(GetDlgItem(hDialog, IDC_SPIN_MAXNFIVEPOLYGONS), UDM_SETRANGE32, 0, 100);
	SendMessage(GetDlgItem(hDialog, IDC_SPIN_MAXNCIRCLE), UDM_SETRANGE32, 0, 100);
	SendMessage(GetDlgItem(hDialog, IDC_SPIN_TIMEPOSX), UDM_SETRANGE32, 0, 100);
	SendMessage(GetDlgItem(hDialog, IDC_SPIN_TIMEPOSY), UDM_SETRANGE32, 0, 100);
	SendMessage(GetDlgItem(hDialog, IDC_SPIN_BORDERWIDTH), UDM_SETRANGE32, -1, 100);
	SendMessage(GetDlgItem(hDialog, IDC_SPIN_MINRADIUS), UDM_SETRANGE32, 0, 1000);
	SendMessage(GetDlgItem(hDialog, IDC_SPIN_MAXRADIUS), UDM_SETRANGE32, 0, 1000);
	SendMessage(GetDlgItem(hDialog, IDC_SLIDER_PURECOLORBORDER_ALPHA), TBM_SETRANGE, FALSE, MAKELPARAM(0, 255));
	SendMessage(GetDlgItem(hDialog, IDC_SLIDER_BACKGROUNDCOLOR_ALPHA), TBM_SETRANGE, FALSE, MAKELPARAM(0, 255));
	SendMessage(GetDlgItem(hDialog, IDC_SLIDER_TIMECOLOR_ALPHA), TBM_SETRANGE, FALSE, MAKELPARAM(0, 255));
}

void GeometryScreenSaver::OnShowWindow(HWND hwnd)
{
	hDialog = hwnd;
	ConfigureDialogControl();
	CopySettingsToDialog();
}

void GeometryScreenSaver::OnOK()
{
	CopyDialogToSettings();
	EndDialog(hDialog, IDOK);
}

void GeometryScreenSaver::OnCancel()
{
	EndDialog(hDialog, IDCANCEL);
}

void GeometryScreenSaver::OnDropFiles(HDROP hd)
{
	DragQueryFile(hd, 0, settings.backgroundSplash, sizeof(settings.backgroundSplash) / sizeof(*settings.backgroundSplash));
	DragFinish(hd);
	SetDlgItemText(hDialog, IDC_EDIT_BACKGROUNDSPLASH, settings.backgroundSplash);
}

void GeometryScreenSaver::OnPressF11()
{
	fullscreen = !fullscreen;
	ChangeFullscreenSettings(fullscreen);
}

void GeometryScreenSaver::ChangeFullscreenSettings(bool bF)
{
	fullscreen = bF;
	if (fullscreen)
	{
		ChangeWindowMode(FALSE);
		SetGraphMode(dm.dmPelsWidth, dm.dmPelsHeight, dm.dmBitsPerPel);
	}
	else
	{
		ChangeWindowMode(TRUE);
		SetGraphMode(settings.windowWidth, settings.windowHeight, dm.dmBitsPerPel);
	}
}

void GeometryScreenSaver::OnCheckShowTimeFormat(UINT nID)
{
	settings.SetScrTimeFormat(IsDlgButtonChecked(hDialog, IDC_CHECK_SHOWYEAR),
		IsDlgButtonChecked(hDialog, IDC_CHECK_SHOWDATE),
		IsDlgButtonChecked(hDialog, IDC_CHECK_SHOWTIME),
		IsDlgButtonChecked(hDialog, IDC_CHECK_SHOWSEC),
		IsDlgButtonChecked(hDialog, IDC_CHECK_SHOWWEEK));
	CheckDlgButton(hDialog, IDC_CHECK_SHOWYEAR, settings.GetScrTimeFormat(1));
	CheckDlgButton(hDialog, IDC_CHECK_SHOWDATE, settings.GetScrTimeFormat(2));
	CheckDlgButton(hDialog, IDC_CHECK_SHOWTIME, settings.GetScrTimeFormat(3));
	CheckDlgButton(hDialog, IDC_CHECK_SHOWSEC, settings.GetScrTimeFormat(4));
	CheckDlgButton(hDialog, IDC_CHECK_SHOWWEEK, settings.GetScrTimeFormat(5));
}

void GeometryScreenSaver::OnEditChangeMinMaxRadius(int nID)
{
	if (GetDlgItemInt(hDialog, IDC_EDIT_MINRADIUS, FALSE, FALSE) > GetDlgItemInt(hDialog, IDC_EDIT_MAXRADIUS, FALSE, FALSE))
	{
		if (nID == IDC_EDIT_MINRADIUS)
			SetDlgItemInt(hDialog, IDC_EDIT_MAXRADIUS, GetDlgItemInt(hDialog, IDC_EDIT_MINRADIUS, FALSE, FALSE), FALSE);
		else
			SetDlgItemInt(hDialog, IDC_EDIT_MINRADIUS, GetDlgItemInt(hDialog, IDC_EDIT_MAXRADIUS, FALSE, FALSE), FALSE);
	}
}

void GeometryScreenSaver::OnCommandBackgroundColor()
{
	UINT newColor;
	if (ChooseColorDialog(&newColor, settings.backgroundColor))
	{
		TCHAR stringbuf[32];
		settings.backgroundColor = newColor;
		wsprintf(stringbuf, GetStringFromResource(IDS_STRING_BUTTON_BACKGROUNDCOLOR), settings.backgroundColor);
		SetDlgItemText(hDialog, IDC_BUTTON_BACKGROUNDCOLOR, stringbuf);
	}
}

void GeometryScreenSaver::OnCommandBackgroundSplashBrowse()
{
	if (OpenFileDialog())
		SetDlgItemText(hDialog, IDC_EDIT_BACKGROUNDSPLASH, settings.backgroundSplash);
}

void GeometryScreenSaver::OnCommandPureColorBorder()
{
	UINT newColor;
	if (ChooseColorDialog(&newColor, settings.pureColorBorder))
	{
		TCHAR stringbuf[32];
		settings.pureColorBorder = newColor;
		wsprintf(stringbuf, GetStringFromResource(IDS_STRING_BUTTON_PURECOLORBORDER), settings.pureColorBorder);
		SetDlgItemText(hDialog, IDC_BUTTON_PURECOLORBORDER, stringbuf);
	}
}

void GeometryScreenSaver::OnCommandTimeColor()
{
	UINT newColor;
	if (ChooseColorDialog(&newColor, settings.timeColor))
	{
		TCHAR stringbuf[32];
		settings.timeColor = newColor;
		wsprintf(stringbuf, GetStringFromResource(IDS_STRING_BUTTON_TIMECOLOR), settings.timeColor);
		SetDlgItemText(hDialog, IDC_BUTTON_TIMECOLOR, stringbuf);
	}
}

void GeometryScreenSaver::OnCommandTimeFont()
{
	lstrcpy(global_logfont.lfFaceName, settings.timeFontName);
	global_logfont.lfHeight = -MulDiv(settings.timeFontSize, GetDeviceCaps(GetDC(hDialog), LOGPIXELSY), 72);
	if (ChooseFontDialog())
	{
		TCHAR stringbuf[32];
		lstrcpy(settings.timeFontName, global_logfont.lfFaceName);
		settings.timeFontSize = MulDiv(-global_logfont.lfHeight, 72, GetDeviceCaps(GetDC(hDialog), LOGPIXELSY));
		wsprintf(stringbuf, TEXT("%s, %d(&D)"), settings.timeFontName, settings.timeFontSize);
		SetDlgItemText(hDialog, IDC_BUTTON_TIMEFONT, stringbuf);
	}
}

void GeometryScreenSaver::OnCommandAbout()
{
	TCHAR tempStr[32];
	lstrcpy(tempStr, GetStringFromResource(IDS_STRING_ABOUT_TITLE));
	MessageBox(hDialog, GetStringFromResource(IDS_STRING_ABOUT_INFORMATION), tempStr, MB_ICONINFORMATION);
}

void GeometryScreenSaver::OnCommandLoadDefaultSettings()
{
	TCHAR tempStr[32];
	lstrcpy(tempStr, GetStringFromResource(IDS_STRING_LOADDEFAULT_TITLE));
	if (MessageBox(hDialog, GetStringFromResource(IDS_STRING_LOADDEFAULT_INFORMATION), tempStr, MB_ICONQUESTION | MB_YESNO) == IDYES)
	{
		settings.LoadDefaultScrSettings();
		CopySettingsToDialog();
	}
}

void GeometryScreenSaver::OnCommandVisit()
{
	ShellExecute(hDialog, TEXT("open"), GetStringFromResource(IDS_STRING_MYURL), NULL, NULL, SW_SHOWNORMAL);
}

void GeometryScreenSaver::OnSliderCustomDraw(UINT_PTR nID)
{
	switch (nID)
	{
	case IDC_SLIDER_BACKGROUNDCOLOR_ALPHA:
		settings.backgroundColor = ((UINT)SendMessage(GetDlgItem(hDialog, IDC_SLIDER_BACKGROUNDCOLOR_ALPHA),
			TBM_GETPOS, 0, 0) << 24) + (settings.backgroundColor & 0x00FFFFFF);
		break;
	case IDC_SLIDER_PURECOLORBORDER_ALPHA:
		settings.pureColorBorder = ((UINT)SendMessage(GetDlgItem(hDialog, IDC_SLIDER_PURECOLORBORDER_ALPHA),
			TBM_GETPOS, 0, 0) << 24) + (settings.pureColorBorder & 0x00FFFFFF);
		break;
	case IDC_SLIDER_TIMECOLOR_ALPHA:
		settings.timeColor = ((UINT)SendMessage(GetDlgItem(hDialog, IDC_SLIDER_TIMECOLOR_ALPHA),
			TBM_GETPOS, 0, 0) << 24) + (settings.timeColor & 0x00FFFFFF);
		break;
	}
	CopySettingsToDialog();
}

int GeometryScreenSaver::CallbackFunction(HWND hW, UINT uM, WPARAM wP, LPARAM lP)
{
	switch (uM)
	{
	case WM_SHOWWINDOW:OnShowWindow(hW); break;
	case WM_COMMAND:
		switch (LOWORD(wP))
		{
		case IDOK:OnOK(); break;
		case IDCANCEL:OnCancel(); break;
		case IDC_CHECK_SHOWYEAR:
		case IDC_CHECK_SHOWDATE:
		case IDC_CHECK_SHOWTIME:
		case IDC_CHECK_SHOWSEC:
		case IDC_CHECK_SHOWWEEK:
			OnCheckShowTimeFormat(LOWORD(wP));
			break;
		case IDC_EDIT_MAXRADIUS:
		case IDC_EDIT_MINRADIUS:
			switch (HIWORD(wP))
			{
			case EN_CHANGE:OnEditChangeMinMaxRadius(LOWORD(wP)); break;
			}
			break;
		case IDC_BUTTON_BACKGROUNDCOLOR:OnCommandBackgroundColor(); break;
		case IDC_BUTTON_BACKGROUNDSPLASH_BROWSE:OnCommandBackgroundSplashBrowse(); break;
		case IDC_BUTTON_PURECOLORBORDER:OnCommandPureColorBorder(); break;
		case IDC_BUTTON_TIMECOLOR:OnCommandTimeColor(); break;
		case IDC_BUTTON_TIMEFONT:OnCommandTimeFont(); break;
		case IDC_BUTTON_ABOUT: OnCommandAbout(); break;
		case IDC_BUTTON_LOADDEFAULTSETTINGS:OnCommandLoadDefaultSettings(); break;
		case IDC_BUTTON_VISIT:OnCommandVisit(); break;
		}
		break;
	case WM_DROPFILES:OnDropFiles((HDROP)wP); break;
	case WM_NOTIFY:
		switch (((LPNMHDR)lP)->code)
		{
		case UDN_DELTAPOS:
			switch (((LPNMUPDOWN)lP)->hdr.idFrom)
			{
			case IDC_SPIN_MAXRADIUS:
			case IDC_SPIN_MINRADIUS:
				break;
			}
			break;
		case NM_CUSTOMDRAW:
			switch (((LPNMHDR)lP)->idFrom)
			{
			case IDC_SLIDER_BACKGROUNDCOLOR_ALPHA:
			case IDC_SLIDER_PURECOLORBORDER_ALPHA:
			case IDC_SLIDER_TIMECOLOR_ALPHA:
				OnSliderCustomDraw(((NMTRBTHUMBPOSCHANGING*)lP)->hdr.idFrom);
				break;
			}
			break;
		}
		break;
	}
	return 0;
}

void GeometryScreenSaver::CopySettingsToDialog()
{
	wsprintf(global_stringBuf, TEXT("#%08X(&B)"), settings.backgroundColor);
	SetDlgItemText(hDialog, IDC_BUTTON_BACKGROUNDCOLOR, global_stringBuf);
	SetDlgItemText(hDialog, IDC_EDIT_BACKGROUNDSPLASH, settings.backgroundSplash);
	CheckDlgButton(hDialog, IDC_CHECK_FITBORDER, settings.fitBorder);
	SetDlgItemInt(hDialog, IDC_EDIT_MAXFALLDOWNSPEED, settings.maxFalldownSpeed, FALSE);
	SetDlgItemInt(hDialog, IDC_EDIT_MAXHORIZONALSPEED, settings.maxHorizonalSpeed, FALSE);
	SetDlgItemInt(hDialog, IDC_EDIT_MAXNTRIANGLE, settings.maxNTriangle, FALSE);
	SetDlgItemInt(hDialog, IDC_EDIT_MAXNSQUARE, settings.maxNSquare, FALSE);
	SetDlgItemInt(hDialog, IDC_EDIT_MAXNFIVEPOLYGONS, settings.maxNFivePolygons, FALSE);
	SetDlgItemInt(hDialog, IDC_EDIT_MAXNCIRCLE, settings.maxNCircle, FALSE);
	CheckDlgButton(hDialog, IDC_CHECK_MULTICOLOREDBORDER, settings.multiColoredBorder);
	wsprintf(global_stringBuf, TEXT("#%08X(&D)"), settings.pureColorBorder);
	SetDlgItemText(hDialog, IDC_BUTTON_PURECOLORBORDER, global_stringBuf);
	CheckDlgButton(hDialog, IDC_CHECK_TRANSFORMINGCOLOR, settings.transformingColor);
	SetDlgItemInt(hDialog, IDC_EDIT_BORDERWIDTH, settings.borderWidth, TRUE);
	SetDlgItemInt(hDialog, IDC_EDIT_TIMEPOSX, settings.timePosX, FALSE);
	SetDlgItemInt(hDialog, IDC_EDIT_TIMEPOSY, settings.timePosY, FALSE);
	SetDlgItemInt(hDialog, IDC_EDIT_MAXRADIUS, settings.maxRadius, FALSE);
	SetDlgItemInt(hDialog, IDC_EDIT_MINRADIUS, settings.minRadius, FALSE);
	wsprintf(global_stringBuf, TEXT("#%08X(&E)"), settings.timeColor);
	SetDlgItemText(hDialog, IDC_BUTTON_TIMECOLOR, global_stringBuf);
	wsprintf(global_stringBuf, TEXT("%s, %d(&D)"), settings.timeFontName, settings.timeFontSize);
	SetDlgItemText(hDialog, IDC_BUTTON_TIMEFONT, global_stringBuf);
	CheckDlgButton(hDialog, IDC_CHECK_SHOWYEAR, settings.GetScrTimeFormat(1));
	CheckDlgButton(hDialog, IDC_CHECK_SHOWDATE, settings.GetScrTimeFormat(2));
	CheckDlgButton(hDialog, IDC_CHECK_SHOWTIME, settings.GetScrTimeFormat(3));
	CheckDlgButton(hDialog, IDC_CHECK_SHOWSEC, settings.GetScrTimeFormat(4));
	CheckDlgButton(hDialog, IDC_CHECK_SHOWWEEK, settings.GetScrTimeFormat(5));
	SendMessage(GetDlgItem(hDialog, IDC_SLIDER_BACKGROUNDCOLOR_ALPHA), TBM_SETPOS, TRUE, settings.backgroundColor >> 24);
	SendMessage(GetDlgItem(hDialog, IDC_SLIDER_PURECOLORBORDER_ALPHA), TBM_SETPOS, TRUE, settings.pureColorBorder >> 24);
	SendMessage(GetDlgItem(hDialog, IDC_SLIDER_TIMECOLOR_ALPHA), TBM_SETPOS, TRUE, settings.timeColor >> 24);
}

void GeometryScreenSaver::CopyDialogToSettings()
{
	GetDlgItemText(hDialog, IDC_EDIT_BACKGROUNDSPLASH, settings.backgroundSplash, sizeof(settings.backgroundSplash) / sizeof(*settings.backgroundSplash));
	settings.fitBorder = IsDlgButtonChecked(hDialog, IDC_CHECK_FITBORDER);
	settings.maxFalldownSpeed = GetDlgItemInt(hDialog, IDC_EDIT_MAXFALLDOWNSPEED, FALSE, FALSE);
	settings.maxHorizonalSpeed = GetDlgItemInt(hDialog, IDC_EDIT_MAXHORIZONALSPEED, FALSE, FALSE);
	settings.maxNTriangle = GetDlgItemInt(hDialog, IDC_EDIT_MAXNTRIANGLE, FALSE, FALSE);
	settings.maxNSquare = GetDlgItemInt(hDialog, IDC_EDIT_MAXNSQUARE, FALSE, FALSE);
	settings.maxNFivePolygons = GetDlgItemInt(hDialog, IDC_EDIT_MAXNFIVEPOLYGONS, FALSE, FALSE);
	settings.maxNCircle = GetDlgItemInt(hDialog, IDC_EDIT_MAXNCIRCLE, FALSE, FALSE);
	settings.multiColoredBorder = IsDlgButtonChecked(hDialog, IDC_CHECK_MULTICOLOREDBORDER);
	settings.transformingColor = IsDlgButtonChecked(hDialog, IDC_CHECK_TRANSFORMINGCOLOR);
	settings.borderWidth = GetDlgItemInt(hDialog, IDC_EDIT_BORDERWIDTH, FALSE, TRUE);
	settings.timePosX = GetDlgItemInt(hDialog, IDC_EDIT_TIMEPOSX, FALSE, FALSE);
	settings.timePosY = GetDlgItemInt(hDialog, IDC_EDIT_TIMEPOSY, FALSE, FALSE);
	settings.maxRadius = GetDlgItemInt(hDialog, IDC_EDIT_MAXRADIUS, FALSE, FALSE);
	settings.minRadius = GetDlgItemInt(hDialog, IDC_EDIT_MINRADIUS, FALSE, FALSE);
}

BOOL GeometryScreenSaver::OpenFileDialog()
{
	TCHAR tempStr[128], title[8];
	const TCHAR filter[] = TEXT("\0*.bmp;*.jpg;*.jpeg;*.png;*.dds;*.argb;*.tga\0\0");
	OPENFILENAME openfile = { 0 };
	lstrcpy(title, GetStringFromResource(IDS_STRING_OPENIMAGE_TITLE));
	lstrcpy(tempStr, GetStringFromResource(IDS_STRING_OPENIMAGE_FILTER_DESC_IMAGE));
	int n = lstrlen(tempStr);
	for (int i = 0; i < sizeof(filter); i++)
		tempStr[n + i] = filter[i];
	openfile.lStructSize = sizeof(OPENFILENAME);
	openfile.hwndOwner = hDialog;
	openfile.hInstance = NULL;
	openfile.lpstrFilter = tempStr;
	openfile.lpstrFile = settings.backgroundSplash;
	openfile.lpstrTitle = title;
	openfile.nMaxFile = MAX_PATH;
	openfile.lpstrFileTitle = NULL;
	openfile.nMaxFileTitle = MAX_PATH;
	openfile.Flags = OFN_HIDEREADONLY;
	openfile.lpstrDefExt = TEXT("png");
	return GetOpenFileName(&openfile);
}

BOOL GeometryScreenSaver::ChooseColorDialog(UINT *outColor, const UINT oldColor)
{
	CHOOSECOLOR color = { 0 };
	color.lStructSize = sizeof(CHOOSECOLOR);
	color.hwndOwner = hDialog;
	color.hInstance = NULL;
	color.rgbResult = oldColor & 0x00FFFFFF;
	if (color.rgbResult == 0)color.rgbResult = RGB(255, 255, 255);
	color.lpCustColors = global_colorRef;
	color.Flags = CC_RGBINIT | CC_FULLOPEN;
	if (ChooseColor(&color))
	{
		*outColor = (color.rgbResult & 0x00FFFFFF) | (oldColor & 0xFF000000);
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

BOOL GeometryScreenSaver::ChooseFontDialog()
{
	CHOOSEFONT cf = { 0 };
	cf.lStructSize = sizeof(CHOOSEFONT);
	cf.hwndOwner = hDialog;
	cf.hDC = NULL;
	cf.lpLogFont = &global_logfont;
	cf.iPointSize = 0;
	cf.Flags = CF_INITTOLOGFONTSTRUCT | CF_EFFECTS | CF_SCREENFONTS;
	cf.rgbColors = 0;
	cf.lCustData = 0;
	cf.lpfnHook = NULL;
	cf.lpTemplateName = NULL;
	cf.hInstance = NULL;
	cf.lpszStyle = NULL;
	cf.nFontType = 0;
	cf.nSizeMin = 0;
	cf.nSizeMax = 0;
	return ChooseFont(&cf);
}

INT_PTR CALLBACK DialogCallback(HWND hW, UINT uM, WPARAM wP, LPARAM lP)
{
	return global_pGSS->CallbackFunction(hW, uM, wP, lP);
}