#include "GeometryScreenSaver.h"
#include "resource.h"
#include <Shlwapi.h>
#include <Windows.h>
#include <ctime>
#include <cmath>
#include <ShlObj.h>

#define DegToRad(d) ((d)*3.141592653545/180)
#define KEY_USE_SCREEN "<SCREEN>"

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

#include <fstream>

bool isFileWritable(const wchar_t *path)
{
	BOOL e = PathFileExists(path);
	bool r = std::ofstream(path, std::ios::app).operator bool();
	if (!e)
		DeleteFile(path);
	return r;
}

GeometryScreenSaver::GeometryScreenSaver() :fullscreen(true), hDialog(nullptr),dxScreenShot(-1),bgBottom(0),bgCoverBottom(0),
bgCoverLeft(0), bgCoverRight(0), bgCoverTop(0), bgLeft(0), bgRight(0), bgTop(0), countOfShapes(), digit_index(0),dm(),dxBackgroundImage(-1),
dxCoverImage(-1), hImgWhiteFill(0), hScreenCover(0), screenHeight(0), screenWidth(0), temptimet(0), temptimet_last(0), temptm(),
timeX(0), timeY(0), vbuffer(), hScreenShot(NULL)
{
	if (global_pGSS)
		delete global_pGSS;
	global_pGSS = this;
	GetCurrentDirectory(sizeof(settingsFileName) / sizeof(*settingsFileName), settingsFileName);
	lstrcpy(appname, GetStringFromResource(IDS_STRING_APPNAME));
	PathCombine(settingsFileName, settingsFileName, GetStringFromResource(IDS_STRING_FILENAME_SETTINGS));
	if (!isFileWritable(settingsFileName))
	{
		GetModuleFileName(NULL, settingsFileName, MAX_PATH);
		PathCombine(settingsFileName, settingsFileName, TEXT(".."));
		PathCombine(settingsFileName, settingsFileName, global_stringBuf);
		if (!isFileWritable(settingsFileName))
		{
			SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, 0, settingsFileName);
			PathCombine(settingsFileName, settingsFileName, appname);
			PathCombine(settingsFileName, settingsFileName, global_stringBuf);
		}
	}
}

GeometryScreenSaver::~GeometryScreenSaver()
{
	global_pGSS = nullptr;
}

#define WinColorToDxColor(x) GetColor((x)&0xFF,((x)>>8)&0xFF,((x)>>16)&0xFF)

#include <map>

struct GraphMaps
{
	std::map<std::wstring, int>hGraphMaps;
	void Release()
	{
		for (typename std::map<std::wstring, int>::iterator it = hGraphMaps.begin(); it != hGraphMaps.end(); it++)
			DeleteGraph(it->second);
	}
	bool Exists(std::wstring key)
	{
		return hGraphMaps.find(key) != hGraphMaps.end();
	}
	void MakeDigitGraph(LPCTSTR str, const ScrSettings* st, int* pdw, int* pdh)
	{
		int dx, dy, dl;
		int fh = CreateFontToHandle(st->timeFontName, st->timeFontSize, st->timeFontThickness, DX_FONTTYPE_ANTIALIASING_EDGE,
			DX_CHARSET_DEFAULT, st->timeBorderWidth);
		GetDrawStringSizeToHandle(&dx, &dy, &dl, str, strlenDx(str), fh);
		int lastScreen = GetDrawScreen();
		SetDrawScreen(MakeScreen(dx, dy, TRUE));
		SetDrawBlendMode(DX_BLENDMODE_ALPHA, (st->timeColor >> 24) & 0xFF);
		DrawStringToHandle(0, 0, str, WinColorToDxColor(st->timeColor), fh, WinColorToDxColor(st->timeBorderColor));
		SetDrawBlendMode(DX_BLENDMODE_ALPHA, 255);
		int gr = MakeGraph(dx, dy);
		hGraphMaps.insert(std::make_pair(std::wstring(str), gr));
		GetDrawScreenGraph(0, 0, dx, dy, gr);
		DeleteFontToHandle(fh);
		SetDrawScreen(lastScreen);
		if (pdw && pdh)
			GetGraphSize(gr, pdw, pdh);
	}
	void GetSize(LPCTSTR str, int* pdw, int* pdh)
	{
		GetGraphSize(graphMaps[str], pdw, pdh);
	}
	int operator[](std::wstring key)
	{
		return hGraphMaps[key];
	}
}graphMaps;

// lpRect 为截取的区域
// by lymking@hotmail.com
HBITMAP CopyScrToBitmap(LPRECT lpRect)

{
	HDC hScrDC = 0, hMemDC;
	HBITMAP hBitmap, hOldBitmap;
	//int nX,nY,xX2,nY2;
	// bitmap width&height
	int nWidth, nHeight;
	// screen resolution
	int xScrn, yScrn;
	// makesure the rectangle not NULL
	if (IsRectEmpty(lpRect))
	{
		return NULL;
	}
	// create desktop screen dc 
	hScrDC = GetDC(GetDesktopWindow());/*CreateDC(_T("DISPLAY"), NULL, NULL, NULL);*/
	if (hScrDC == NULL)
		return NULL;
	// create mem dc
	hMemDC = CreateCompatibleDC(hScrDC);

	// get resolutions
	xScrn = GetDeviceCaps(hScrDC, HORZRES);
	yScrn = GetDeviceCaps(hScrDC, VERTRES);
	if (lpRect->left < 0)
		lpRect->left = 0;
	if (lpRect->top < 0)
		lpRect->top = 0;
	// 屏幕
	nWidth = (lpRect->right - lpRect->left);
	nHeight = (lpRect->bottom - lpRect->top);
	// create bitmap
	hBitmap = CreateCompatibleBitmap(hScrDC, nWidth, nHeight);
	// select new bitmap
	hOldBitmap = (HBITMAP)SelectObject(hMemDC, hBitmap);
	// copy scr dc to mem dc
	BitBlt(hMemDC, 0, 0, nWidth, nHeight, hScrDC, 0, 0, SRCCOPY);
	// get bitmap handle
	hBitmap = (HBITMAP)SelectObject(hMemDC, hOldBitmap);
	// clean 
	DeleteDC(hScrDC);
	DeleteDC(hMemDC);
	return hBitmap;
}

//https://dxlib.xsrv.jp/cgi/patiobbs/patio.cgi?mode=past&no=2068
int LoadGraphFromHBitmap(HBITMAP hBmp)
{
	// DIBセクションの取得
	BITMAP DDBInfo;
	BITMAPINFO DIBInfo;

	GetObject(hBmp, sizeof(BITMAP), &DDBInfo);
	DIBInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	DIBInfo.bmiHeader.biWidth = DDBInfo.bmWidth;
	DIBInfo.bmiHeader.biHeight = DDBInfo.bmHeight;
	DIBInfo.bmiHeader.biPlanes = 1;
	DIBInfo.bmiHeader.biBitCount = 32;
	DIBInfo.bmiHeader.biCompression = BI_RGB;

	BYTE* pData = new BYTE[DDBInfo.bmWidth * DDBInfo.bmHeight * 4];
	HDC hDC = GetDC(GetMainWindowHandle());
	GetDIBits(hDC, hBmp, 0, DDBInfo.bmHeight, (void*)pData, &DIBInfo, DIB_RGB_COLORS);
	ReleaseDC(GetMainWindowHandle(), hDC);
	DeleteObject(hBmp);

	// ソフトイメージに変換してみる
	int sh = MakeXRGB8ColorSoftImage(DDBInfo.bmWidth, DDBInfo.bmHeight);
	BYTE* Dots = pData;
	for (int y = DDBInfo.bmHeight - 1; y >= 0; y--) { // データは上下さかさまらしい
		for (int x = 0; x < DDBInfo.bmWidth; x++) {
			DrawPixelSoftImage(sh, x, y, *(Dots + 2), *(Dots + 1), *(Dots + 0), *(Dots + 3));
			Dots += 4;
		}
	}
	// ハンドルに変換してみる
	int gh = CreateGraphFromBmp(&DIBInfo, pData); // やってみたらうまくいきました。

	delete[] pData; // ReCreateGraphするときに必要かも。
	return gh;
}

int GeometryScreenSaver::Run(HWND hParent)
{
	if (hParent)
		return 0;
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
	if (lstrcmp(settings.backgroundSplash, TEXT(KEY_USE_SCREEN)) == 0)
		settings.backgroundColor = 0;
	if (lstrcmp(settings.coverSplash, TEXT(KEY_USE_SCREEN)) == 0)
		settings.coverColor = 0;

	dm.dmSize = sizeof(DEVMODE);
	EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &dm);
	ChangeFullscreenSettings(true);

	RECT rScreen = { 0,0,(LONG)dm.dmPelsWidth,(LONG)dm.dmPelsHeight };
	hScreenShot = CopyScrToBitmap(&rScreen);

	// ＤＸライブラリ初期化処理
	if (DxLib_Init() == -1) return -1;

	LoadSourceFromFiles();

	// 描画先画面を裏画面にする
	SetDrawScreen(DX_SCREEN_BACK);

	for (int i = 0; i < 10; i++)
	{
		sprintfDx(global_stringBuf, TEXT("%d"), i);
		if (!graphMaps.Exists(global_stringBuf))
			graphMaps.MakeDigitGraph(global_stringBuf, &settings, NULL, NULL);
	}
	for (int id : weekstrindex)
	{
		GetStringFromResource(id);
		if (!graphMaps.Exists(global_stringBuf))
			graphMaps.MakeDigitGraph(global_stringBuf, &settings, NULL, NULL);
	}

	while (1)
	{
		// Windows システムからくる情報を処理する
		if (ProcessMessage() == -1) break;

		// 画面に描かれているものをすべて消す
		ClearDrawScreen();

		// 待たないと処理が早すぎるのでここで２０ミリ秒待つ
		//WaitTimer(200);

			// x , y が示す画面座標にＢＭＰ画像 test1.bmp を描画する
		DrawBackgroundSplash();
		if (settings.enableCoverLayer)
			DrawCoverLayer();
		else
			DrawShapes(false);
		DrawDateTime();

		ShapesAct();

		// 上下左右のキー入力に対応して x, y の座標値を変更する
		if (CheckHitKey(KEY_INPUT_F11)) {
			OnPressF11();
			LoadSourceFromFiles();
		}
		// ＥＳＣキーが押されたらループから抜ける
		if (CheckHitKey(KEY_INPUT_ESCAPE))break;
		if (GetAsyncKeyState(VK_LBUTTON))break;
		if (GetAsyncKeyState(VK_RBUTTON))break;

		// 裏画面の内容を表画面に反映させる
		ScreenFlip();
	}
	for (int i = (int)shapes.size() - 1; i >= 0; i--)
		delete shapes[i];
	shapes.clear();
	graphMaps.Release();

	// ＤＸライブラリ使用の終了処理
	DxLib_End();

	// ソフトの終了
	return 0;
}

void GeometryScreenSaver::FitBorderRect(int borderW, int borderH, int imageW, int imageH, int*l, int*t, int*r, int*b)
{
	if ((float)borderH / borderW > (float)imageH / imageW)//较宽图
	{
		*l = 0;
		*t = (borderH - borderW*imageH / imageW) / 2;
		*r = borderW;
		*b = borderH - *t;
	}
	else//较高图
	{
		*t = 0;
		*l = (borderW - borderH*imageW / imageH) / 2;
		*b = borderH;
		*r = borderW - *l;
	}
}

void GeometryScreenSaver::LoadSourceFromFiles()
{
	GetDrawScreenSize(&screenWidth, &screenHeight);
	if (strcmpDx(settings.backgroundSplash, TEXT(KEY_USE_SCREEN)) == 0)
		dxBackgroundImage = dxScreenShot = LoadGraphFromHBitmap(hScreenShot);
	if (dxBackgroundImage == -1)
		dxBackgroundImage = LoadGraph(settings.backgroundSplash);
	GetGraphSize(dxBackgroundImage, &bgRight, &bgBottom);
	if (!bgRight)bgRight = 1;
	if (!bgBottom)bgBottom = 1;
	if (settings.fitBorder)
		FitBorderRect(screenWidth, screenHeight, bgRight, bgBottom, &bgLeft, &bgTop, &bgRight, &bgBottom);
	else
	{
		bgLeft = (screenWidth - bgRight) / 2;
		bgTop = (screenHeight - bgBottom) / 2;
		bgRight += bgLeft;
		bgBottom += bgTop;
	}
	timeX = screenWidth*settings.timePosX / 100;
	timeY = screenHeight*settings.timePosY / 100;
	SetBackgroundColor(settings.backgroundColor & 0x000000FF, (settings.backgroundColor & 0x0000FF00) >> 8,
		(settings.backgroundColor & 0x00FF0000) >> 16);
	SetFontSize(settings.timeFontSize);
	SetFontThickness(settings.timeFontThickness);
	ChangeFont(settings.timeFontName);
	ChangeFontType(DX_FONTTYPE_ANTIALIASING);
	if (settings.enableCoverLayer)
	{
		SetMaskReverseEffectFlag(TRUE);
		hScreenCover = MakeScreen(screenWidth, screenHeight, TRUE);
		SetMaskScreenGraph(hScreenCover);
		if (strcmpDx(settings.coverSplash, TEXT(KEY_USE_SCREEN)) == 0)
		{
			if (dxScreenShot == -1)
				dxScreenShot = LoadGraphFromHBitmap(hScreenShot);
			dxCoverImage = dxScreenShot;
		}
		if (dxCoverImage == -1)
			dxCoverImage = LoadGraph(settings.coverSplash);
		GetGraphSize(dxCoverImage, &bgCoverRight, &bgCoverBottom);
		if (!bgCoverRight)bgCoverRight = 1;
		if (!bgCoverBottom)bgCoverBottom = 1;
		if (settings.coverFitBorder)
			FitBorderRect(screenWidth, screenHeight, bgCoverRight, bgCoverBottom, &bgCoverLeft, &bgCoverTop, &bgCoverRight, &bgCoverBottom);
		else
		{
			bgCoverLeft = (screenWidth - bgCoverRight) / 2;
			bgCoverTop = (screenHeight - bgCoverBottom) / 2;
			bgCoverRight += bgCoverLeft;
			bgCoverBottom += bgCoverTop;
		}
		hImgWhiteFill = MakeGraph(screenWidth, screenHeight);
		FillGraph(hImgWhiteFill, 255, 255, 255);
	}
}

void GeometryScreenSaver::DrawBackgroundSplash()
{
	if (dxBackgroundImage != -1)
		DrawExtendGraph(bgLeft, bgTop, bgRight, bgBottom, dxBackgroundImage, TRUE);
}

RECT operator+(const RECT& a, const RECT& b)
{
	RECT r;
	r.bottom = a.bottom + b.bottom;
	r.left = a.left + b.left;
	r.right = a.right + b.right;
	r.top = a.top + b.top;
	return r;
}

RECT operator-(const RECT &a, const RECT &b)
{
	RECT r;
	r.bottom = a.bottom - b.bottom;
	r.left = a.left - b.left;
	r.right = a.right - b.right;
	r.top = a.top - b.top;
	return r;
}

RECT operator*(const RECT &a, int n)
{
	RECT r;
	r.bottom = a.bottom * n;
	r.left = a.left * n;
	r.right = a.right * n;
	r.top = a.top * n;
	return r;
}

RECT operator/(const RECT& a, int n)
{
	RECT r;
	r.bottom = a.bottom / n;
	r.left = a.left / n;
	r.right = a.right / n;
	r.top = a.top / n;
	return r;
}

struct DisplayUnit
{
public:
	TCHAR draw_str[4];
	int x_offset, y_offset,start_x,start_y,to_x, to_y,anim_time_ms,anim_time_ms_cur;
	RECT rect_start, rect_to, rect;
private:
	int dw,dh;

public:
	DisplayUnit():dw(0),dh(0),x_offset(0),y_offset(0),start_x(0),start_y(0),to_x(0),to_y(0),anim_time_ms(0),
		anim_time_ms_cur(0),draw_str(),rect_start(),rect_to(),rect()
	{
	}

	void SetDigit(LPCTSTR str,size_t slen, const ScrSettings* st)
	{
		if (strncmpDx(draw_str, str,(int)slen) == 0)
			return;
		ZeroMemory(draw_str, sizeof draw_str);
		strncpyDx(draw_str, str, slen);
		if (graphMaps.Exists(draw_str))
			graphMaps.GetSize(draw_str, &dw, &dh);
		else
			graphMaps.MakeDigitGraph(draw_str, st, &dw, &dh);
	}

	void Animate(int delta_time_ms)
	{
		int temp_atime = anim_time_ms;
		if (temp_atime < 0)
			temp_atime = -temp_atime;
		else if (temp_atime == 0 && y_offset != to_y)
		{
			x_offset = to_x;
			y_offset = to_y;
			rect = rect_to;
			return;
		}
		if (anim_time_ms_cur >= temp_atime)
			return;
		anim_time_ms_cur += delta_time_ms;
		if (anim_time_ms_cur > temp_atime)
			anim_time_ms_cur = temp_atime;
		x_offset = start_x + (to_x - start_x) * anim_time_ms_cur / temp_atime;
		y_offset = start_y + (to_y - start_y) * anim_time_ms_cur / temp_atime;
		rect = rect_start + (rect_to - rect_start) * anim_time_ms_cur / temp_atime;
	}
	void Draw(int x_base,int y_base)
	{
		if (graphMaps.Exists(draw_str) && rect.bottom > rect.top)
			DrawRectGraph(x_base + x_offset, y_base + y_offset, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top,
				graphMaps[draw_str], TRUE);
	}
	int GetGraphWidth()
	{
		return dw;
	}
	int GetGraphHeight()
	{
		return dh;
	}
	/*long CalcVisibleWidth()
	{
		return rect.right - rect.left;
	}
	long CalcVisibleHeight()
	{
		return rect.bottom - rect.top;
	}*/
};

struct DigitUnit
{
	DisplayUnit unit, unit_last;
	void Animate(int delta_time_ms)
	{
		unit.Animate(delta_time_ms);
		unit_last.Animate(delta_time_ms);
	}
	void Draw(int x_base,int y_base)
	{
		unit.Draw(x_base,y_base);
		unit_last.Draw(x_base,y_base);
	}
	long CalcWidth()
	{
		return max(unit.GetGraphWidth(),unit_last.GetGraphWidth());
	}
	long CalcHeight()
	{
		return unit.GetGraphHeight();
	}
	void Set(LPCTSTR str,size_t slen, const ScrSettings* st,int x1_offset, int y1_offset, int x2_offset, int y2_offset, RECT r1, RECT r2, int anim_time)
	{
		if (strncmpDx(unit.draw_str, str, slen) == 0)
			return;
		unit_last = unit;
		unit_last.start_x = unit_last.x_offset = 0;
		unit_last.start_y = unit_last.y_offset = 0;
		//GetGraphxxx与CalcVisiblexxx混用会造成矩形框不稳定，且用CalcVisiblexxx会使位置不准
		unit_last.rect_start = unit_last.rect = { 0,0,unit_last.GetGraphWidth(),unit_last.GetGraphHeight() };
		if (anim_time == 0)
		{
			//说明是不使用动画
			unit_last.to_y = unit_last.start_y = 0;
			unit_last.rect_to = unit_last.rect = { 0,0,0,0 };
		}
		else if (y2_offset >= y1_offset)
		{
			//说明是在由上向下滑动
			unit_last.to_y = unit_last.start_y + unit_last.GetGraphHeight();
			unit_last.rect_to = { 0,0,unit_last.GetGraphWidth(),0 };
		}
		else
		{
			unit_last.to_y = unit_last.start_y;
			unit_last.rect_to = { 0,unit_last.GetGraphHeight(),unit_last.GetGraphWidth(),unit_last.GetGraphHeight() };
		}
		unit_last.anim_time_ms = anim_time;
		unit_last.anim_time_ms_cur = 0;
		unit_last.SetDigit(unit.draw_str, strlenDx(unit.draw_str), st);
		unit.SetDigit(str,slen,st);
		unit.x_offset = unit.start_x = x1_offset;
		unit.to_x = x2_offset;
		unit.y_offset = unit.start_y = y1_offset;
		unit.to_y = y2_offset;
		unit.anim_time_ms = anim_time;
		unit.anim_time_ms_cur = 0;
		unit.rect = unit.rect_start = r1;
		unit.rect_to = r2;
	}
};

struct DigitsString
{
	std::vector<DigitUnit> digits;
	DigitUnit& operator[](size_t index)
	{
		while (digits.size() < index + 1)
			digits.push_back(DigitUnit());
		return digits[index];
	}
	void Release()
	{
		digits.clear();
	}
	size_t Size()
	{
		return digits.size();
	}
	long CalcWidth()
	{
		long w = 0;
		for (auto& e : digits)
			w += e.CalcWidth();
		return w;
	}
	long CalcHeight()
	{
		long h = 0;
		for (auto& e : digits)
		{
			long eh = e.CalcHeight();
			if (eh > h)
				h = eh;
		}
		return h;
	}
	void Animate(int delta_time_ms)
	{
		for (auto& e : digits)
			e.Animate(delta_time_ms);
	}
	void Draw(int x, int y)
	{
		int x_adv = x;
		for (size_t i=0;i<digits.size();i++)
		{
			digits[i].Draw(x_adv, y);
			x_adv += digits[i].CalcWidth();
		}
	}
}digitsString;

void GeometryScreenSaver::DrawDateTime()
{
	temptimet_last = temptimet;
	time(&temptimet);
	if (temptimet_last != temptimet)
	{
		digit_index = 0;
		localtime_s(&temptm, &temptimet);
		TCHAR* p;
		if (settings.GetScrTimeFormat(1))
		{
			p = GetStringOfValue(temptm.tm_year + 1900);
			for (int i = 0; p[i]; i++)
				SetDigit(p + i, 1);
			SetDigit(TEXT("-"), 1);
		}
		if (settings.GetScrTimeFormat(2))
		{
			p = GetStringOfValue(temptm.tm_mon + 1);
			for (int i = 0; p[i]; i++)
				SetDigit(p + i, 1);
			SetDigit(TEXT("-"), 1);
			p = GetStringOfValue(temptm.tm_mday);
			for (int i = 0; p[i]; i++)
				SetDigit(p + i, 1);
			SetDigit(TEXT(" "), 1);
		}
		if (settings.GetScrTimeFormat(3))
		{
			p = GetStringOfValue(temptm.tm_hour, TEXT("%2d"));
			for (int i = 0; p[i]; i++)
				SetDigit(p + i, 1);
			SetDigit(TEXT(":"), 1);
			p = GetStringOfValue(temptm.tm_min, TEXT("%02d"));
			for (int i = 0; p[i]; i++)
				SetDigit(p + i, 1);
		}
		if (settings.GetScrTimeFormat(4))
		{
			SetDigit(TEXT(":"),1);
			p = GetStringOfValue(temptm.tm_sec, TEXT("%02d"));
			for (int i = 0; p[i]; i++)
				SetDigit(p + i, 1);
		}
		if (settings.GetScrTimeFormat(5))
		{
			SetDigit(TEXT(" "), 1);
			p = GetStringFromResource(weekstrindex[temptm.tm_wday]);
			SetDigit(p, strlenDx(p));
		}
		while (digit_index < digitsString.Size())
		{
			if (digitsString[digit_index].unit.draw_str[0] == 0)
				digitsString.digits.erase(digitsString.digits.begin() + digit_index);
			else
				SetDigit(TEXT(""), 1);
		}
	}
	digitsString.Animate((int)(1000.0f / GetFPS()));
	int dx, dy;
	dx = digitsString.CalcWidth();
	dy = digitsString.CalcHeight();
	if (settings.timeShadowDistancePixelsH || settings.timeShadowDistancePixelsV)
	{
		SetDrawBlendMode(DX_BLENDMODE_ALPHA, settings.timeShadowAlpha);
		SetDrawBright(settings.timeShadowBright, settings.timeShadowBright, settings.timeShadowBright);
		digitsString.Draw(timeX - dx * settings.timeAlignAnchorX / 100 + settings.timeShadowDistancePixelsH,
			timeY - dy * settings.timeAlignAnchorY / 100 + settings.timeShadowDistancePixelsV);
		SetDrawBright(255, 255, 255);
	}
	SetDrawBlendMode(DX_BLENDMODE_ALPHA, settings.timeColor >> 24);
	digitsString.Draw(timeX - dx * settings.timeAlignAnchorX / 100, timeY - dy * settings.timeAlignAnchorY / 100);
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
}

void GeometryScreenSaver::SetDigit(LPCTSTR str, size_t slen)
{
	int dw, dh, dl;
	GetDrawStringSize(&dw, &dh, &dl, str, slen);
	if (settings.digitMovingSpeed > 0)//向下滑动
		digitsString[digit_index].Set(str, slen,&settings, 0, 0, 0, 0, { 0,dh,dw,dh }, { 0,0, dw ,dh }, settings.digitMovingSpeed);
	else
		digitsString[digit_index].Set(str, slen,&settings, 0, dh, 0, 0, { 0,0,dw,0 }, { 0,0, dw ,dh }, settings.digitMovingSpeed);
	digit_index++;
}

void GeometryScreenSaver::DrawTrianglePolygon(int cx, int cy, int radius, int rot_deg, int r, int g, int b, int num, int thick)
{
	for (int i = 0; i < num; i++)
	{
		vbuffer[i].x = (int)(radius*sin(DegToRad(rot_deg + 360.0 * i / num)) + cx);
		vbuffer[i].y = (int)(radius*cos(DegToRad(rot_deg + 360.0 * i / num)) + cy);
	}
	if (thick)
		for (int i = 0; i < num; i++)
			DrawLine(vbuffer[i].x, vbuffer[i].y, vbuffer[(i + 1) % num].x, vbuffer[(i + 1) % num].y, GetColor(r, g, b), thick);
	else
		for (int i = 2; i < num; i++)
			DrawTriangle(vbuffer[0].x, vbuffer[0].y, vbuffer[i - 1].x, vbuffer[i - 1].y, vbuffer[i].x, vbuffer[i].y,
				GetColor(r, g, b), TRUE);
}

void GeometryScreenSaver::DrawShapes(bool fill)
{
	for (int i = (int)shapes.size() - 1; i >= 0; i--)
		switch (shapes[i]->polygons)
		{
		case 0:
			if (fill)
				DrawCircle(shapes[i]->x, shapes[i]->y, shapes[i]->radius, 0xFFFFFFFF, TRUE);
			else
				DrawCircle(shapes[i]->x, shapes[i]->y, shapes[i]->radius, GetColor(shapes[i]->color[0],
					shapes[i]->color[1], shapes[i]->color[2]), 0, settings.borderWidth);
			break;
		default:
			DrawTrianglePolygon(shapes[i]->x, shapes[i]->y, shapes[i]->radius, shapes[i]->angle, shapes[i]->color[0],
				shapes[i]->color[1], shapes[i]->color[2], shapes[i]->polygons + 2, fill ? 0 : settings.borderWidth);
			break;
		}
}

void GeometryScreenSaver::DrawCoverLayer()
{
	SetDrawScreen(hScreenCover);
	ClearDrawScreen();
	DrawShapes(true);
	SetDrawScreen(DX_SCREEN_BACK);
	CreateMaskScreen();
	DrawBox(0, 0, screenWidth, screenHeight, settings.coverColor, TRUE);
	if (dxCoverImage != -1)
		DrawExtendGraph(bgCoverLeft, bgCoverTop, bgCoverRight, bgCoverBottom, dxCoverImage, TRUE);
	DeleteMaskScreen();
}

void GeometryScreenSaver::ShapesAct()
{
	for (int i = (int)shapes.size() - 1; i >= 0; i--)
	{
		shapes[i]->x += shapes[i]->vx;
		shapes[i]->y += shapes[i]->vy;
		shapes[i]->angle += shapes[i]->vrotate;
		if (settings.transformingColor)
		{
			if (shapes[i]->crF2)//增/减
			{
				if (shapes[i]->color[shapes[i]->crF3 % 3u] < shapes[i]->crTransSpeed)//到底端
				{
					shapes[i]->crF1 ? shapes[i]->crF3++ : shapes[i]->crF3--;
					shapes[i]->crF2 = false;
				}
				else shapes[i]->color[shapes[i]->crF3 % 3u] -= shapes[i]->crTransSpeed;
			}
			else
			{
				if (shapes[i]->color[shapes[i]->crF3 % 3u] > 255 - shapes[i]->crTransSpeed)//到顶端
				{
					shapes[i]->crF1 ? shapes[i]->crF3++ : shapes[i]->crF3--;
					shapes[i]->crF2 = true;
				}
				else shapes[i]->color[shapes[i]->crF3 % 3u] += shapes[i]->crTransSpeed;
			}
		}
		if (shapes[i]->y - (int)settings.maxRadius > (int)(fullscreen ? dm.dmPelsHeight : settings.windowHeight))
		{
			countOfShapes[shapes[i]->polygons]--;
			delete shapes[i];
			shapes.erase(shapes.begin() + i);
		}
		else if ((shapes[i]->x < 0 && shapes[i]->vx < 0) ||
			((unsigned)shapes[i]->x>(fullscreen ? dm.dmPelsWidth : settings.windowWidth) && shapes[i]->vx>0))
			shapes[i]->vx = -shapes[i]->vx;
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

int GeometryScreenSaver::Settings(HINSTANCE hInstance, HWND hWnd)
{
	LoadSettingsFromFile();
	if (DialogBox(hInstance, MAKEINTRESOURCE(IDD_DIALOG_SETTINGS), hWnd, DialogCallback) == IDOK)
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
	SendMessage(GetDlgItem(hDialog, IDC_SPIN_MINRADIUS), UDM_SETRANGE32, 0, 999);
	SendMessage(GetDlgItem(hDialog, IDC_SPIN_MAXRADIUS), UDM_SETRANGE32, 0, 999);
	SendMessage(GetDlgItem(hDialog, IDC_SPIN_TIMEANCHORX), UDM_SETRANGE32, 0, 100);
	SendMessage(GetDlgItem(hDialog, IDC_SPIN_TIMEANCHORY), UDM_SETRANGE32, 0, 100);
	SendMessage(GetDlgItem(hDialog, IDC_SPIN_DIGITMOVINGSPEED), UDM_SETRANGE32, -10000, 10000);
	SendMessage(GetDlgItem(hDialog, IDC_SPIN_TIMEBORDERWIDTH), UDM_SETRANGE32, 0, 1000);
	SendMessage(GetDlgItem(hDialog, IDC_SPIN_TIMESHADOWH), UDM_SETRANGE32, 0, 10000);
	SendMessage(GetDlgItem(hDialog, IDC_SPIN_TIMESHADOWV), UDM_SETRANGE32, 0, 10000);
	SendMessage(GetDlgItem(hDialog, IDC_SPIN_TIMESHADOWBRIGHT), UDM_SETRANGE32, 0, 255);
	SendMessage(GetDlgItem(hDialog, IDC_SPIN_TIMESHADOWALPHA), UDM_SETRANGE32, 0, 255);
	SendMessage(GetDlgItem(hDialog, IDC_SLIDER_PURECOLORBORDER_ALPHA), TBM_SETRANGE, FALSE, MAKELPARAM(0, 255));
	SendMessage(GetDlgItem(hDialog, IDC_SLIDER_BACKGROUNDCOLOR_ALPHA), TBM_SETRANGE, FALSE, MAKELPARAM(0, 255));
	SendMessage(GetDlgItem(hDialog, IDC_SLIDER_TIMECOLOR_ALPHA), TBM_SETRANGE, FALSE, MAKELPARAM(0, 255));
	SendMessage(GetDlgItem(hDialog, IDC_SLIDER_COVERCOLOR_ALPHA), TBM_SETRANGE, FALSE, MAKELPARAM(0, 255));
	SendMessage(GetDlgItem(hDialog, IDC_SLIDER_TIMEBORDERCOLOR), TBM_SETRANGE, FALSE, MAKELPARAM(0, 255));
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
	if (OpenFileDialog(settings.backgroundSplash))
		SetDlgItemText(hDialog, IDC_EDIT_BACKGROUNDSPLASH, settings.backgroundSplash);
}

void GeometryScreenSaver::OnCommandCoverColor()
{
	UINT newColor;
	if (ChooseColorDialog(&newColor, settings.coverColor))
	{
		TCHAR stringbuf[32];
		settings.coverColor = newColor;
		wsprintf(stringbuf, GetStringFromResource(IDS_STRING_BUTTON_COVERCOLOR), settings.coverColor);
		SetDlgItemText(hDialog, IDC_BUTTON_COVERCOLOR, stringbuf);
	}
}

void GeometryScreenSaver::OnCommandCoverSplashBrowse()
{
	if (OpenFileDialog(settings.coverSplash))
		SetDlgItemText(hDialog, IDC_EDIT_COVERSPLASH, settings.coverSplash);
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
	case IDC_SLIDER_COVERCOLOR_ALPHA:
		settings.coverColor = ((UINT)SendMessage(GetDlgItem(hDialog, IDC_SLIDER_COVERCOLOR_ALPHA),
			TBM_GETPOS, 0, 0) << 24) + (settings.coverColor & 0x00FFFFFF);
		break;
	case IDC_SLIDER_TIMEBORDERCOLOR:
		settings.timeBorderColor = ((UINT)SendMessage(GetDlgItem(hDialog, IDC_SLIDER_TIMEBORDERCOLOR),
			TBM_GETPOS, 0, 0) << 24) + (settings.timeBorderColor & 0x00FFFFFF);
	}
	CopySettingsToDialog();
}

void GeometryScreenSaver::OnCommandClearBackground()
{
	SetDlgItemText(hDialog, IDC_EDIT_BACKGROUNDSPLASH, TEXT(""));
}

void GeometryScreenSaver::OnCommandClearCover()
{
	SetDlgItemText(hDialog, IDC_EDIT_COVERSPLASH, TEXT(""));
}

void GeometryScreenSaver::OnCommandBackgroundUseScreen()
{
	SetDlgItemText(hDialog, IDC_EDIT_BACKGROUNDSPLASH, TEXT(KEY_USE_SCREEN));
}

void GeometryScreenSaver::OnCommandCoverUseScreen()
{
	SetDlgItemText(hDialog, IDC_EDIT_COVERSPLASH, TEXT(KEY_USE_SCREEN));
}

void GeometryScreenSaver::OnCommandTimeBorderColor()
{
	UINT newColor;
	if (ChooseColorDialog(&newColor, settings.timeBorderColor))
	{
		TCHAR stringbuf[32];
		settings.timeBorderColor = newColor;
		wsprintf(stringbuf, GetStringFromResource(IDS_STRING_BUTTON_TIMEBORDERCOLOR), settings.timeBorderColor);
		SetDlgItemText(hDialog, IDC_BUTTON_TIMEBORDERCOLOR, stringbuf);
	}
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
		case IDC_BUTTON_COVERCOLOR:OnCommandCoverColor(); break;
		case IDC_BUTTON_COVERSPLASH_BROWSE:OnCommandCoverSplashBrowse(); break;
		case IDC_BUTTON_PURECOLORBORDER:OnCommandPureColorBorder(); break;
		case IDC_BUTTON_TIMECOLOR:OnCommandTimeColor(); break;
		case IDC_BUTTON_TIMEFONT:OnCommandTimeFont(); break;
		case IDC_BUTTON_ABOUT: OnCommandAbout(); break;
		case IDC_BUTTON_LOADDEFAULTSETTINGS:OnCommandLoadDefaultSettings(); break;
		case IDC_BUTTON_VISIT:OnCommandVisit(); break;
		case IDC_BUTTON_CLEARBACKGROUND:OnCommandClearBackground(); break;
		case IDC_BUTTON_CLEARCOVER:OnCommandClearCover(); break;
		case IDC_BUTTON_BACKGROUNDUSESCREEN:OnCommandBackgroundUseScreen(); break;
		case IDC_BUTTON_COVERUSESCREEN:OnCommandCoverUseScreen(); break;
		case IDC_BUTTON_TIMEBORDERCOLOR:OnCommandTimeBorderColor(); break;
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
			case IDC_SLIDER_COVERCOLOR_ALPHA:
			case IDC_SLIDER_TIMEBORDERCOLOR:
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
	CheckDlgButton(hDialog, IDC_CHECK_USECOVER, settings.enableCoverLayer);
	CheckDlgButton(hDialog, IDC_CHECK_COVERFITBORDER, settings.coverFitBorder);
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
	wsprintf(global_stringBuf, TEXT("#%08X(&C)"), settings.coverColor);
	SetDlgItemText(hDialog, IDC_BUTTON_COVERCOLOR, global_stringBuf);
	SendMessage(GetDlgItem(hDialog, IDC_SLIDER_COVERCOLOR_ALPHA), TBM_SETPOS, TRUE, settings.coverColor >> 24);
	SetDlgItemText(hDialog, IDC_EDIT_COVERSPLASH, settings.coverSplash);
	SetDlgItemInt(hDialog, IDC_EDIT_TIMEANCHORX, settings.timeAlignAnchorX, FALSE);
	SetDlgItemInt(hDialog, IDC_EDIT_TIMEANCHORY, settings.timeAlignAnchorY, FALSE);
	SetDlgItemInt(hDialog, IDC_EDIT_DIGITMOVINGSPEED, settings.digitMovingSpeed, TRUE);
	SendMessage(GetDlgItem(hDialog, IDC_SLIDER_TIMEBORDERCOLOR), TBM_SETPOS, TRUE, settings.timeBorderColor >> 24);
	SetDlgItemInt(hDialog, IDC_EDIT_TIMEBORDERWIDTH, settings.timeBorderWidth, FALSE);
	SetDlgItemInt(hDialog, IDC_EDIT_TIMESHADOWH, settings.timeShadowDistancePixelsH, FALSE);
	SetDlgItemInt(hDialog, IDC_EDIT_TIMESHADOWV, settings.timeShadowDistancePixelsV, FALSE);
	SetDlgItemInt(hDialog, IDC_EDIT_TIMESHADOWBRIGHT, settings.timeShadowBright, FALSE);
	SetDlgItemInt(hDialog, IDC_EDIT_TIMESHADOWALPHA, settings.timeShadowAlpha, FALSE);
	wsprintf(global_stringBuf, TEXT("#%08X(&T)"), settings.timeBorderColor);
	SetDlgItemText(hDialog, IDC_BUTTON_TIMEBORDERCOLOR, global_stringBuf);
}

void GeometryScreenSaver::CopyDialogToSettings()
{
	GetDlgItemText(hDialog, IDC_EDIT_BACKGROUNDSPLASH, settings.backgroundSplash, ARRAYSIZE(settings.backgroundSplash));
	GetDlgItemText(hDialog, IDC_EDIT_COVERSPLASH, settings.coverSplash, ARRAYSIZE(settings.coverSplash));
	settings.fitBorder = IsDlgButtonChecked(hDialog, IDC_CHECK_FITBORDER);
	settings.enableCoverLayer = IsDlgButtonChecked(hDialog, IDC_CHECK_USECOVER);
	settings.coverFitBorder = IsDlgButtonChecked(hDialog, IDC_CHECK_COVERFITBORDER);
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
	settings.timeAlignAnchorX = GetDlgItemInt(hDialog, IDC_EDIT_TIMEANCHORX, FALSE, FALSE);
	settings.timeAlignAnchorY = GetDlgItemInt(hDialog, IDC_EDIT_TIMEANCHORY, FALSE, FALSE);
	settings.digitMovingSpeed = GetDlgItemInt(hDialog, IDC_EDIT_DIGITMOVINGSPEED, FALSE, TRUE);
	settings.timeBorderWidth = GetDlgItemInt(hDialog, IDC_EDIT_TIMEBORDERWIDTH, FALSE, FALSE);
	settings.timeShadowDistancePixelsH = GetDlgItemInt(hDialog, IDC_EDIT_TIMESHADOWH, FALSE, TRUE);
	settings.timeShadowDistancePixelsV = GetDlgItemInt(hDialog, IDC_EDIT_TIMESHADOWV, FALSE, TRUE);
	settings.timeShadowBright = GetDlgItemInt(hDialog, IDC_EDIT_TIMESHADOWBRIGHT, FALSE, FALSE);
	settings.timeShadowAlpha = GetDlgItemInt(hDialog, IDC_EDIT_TIMESHADOWALPHA, FALSE, FALSE);
}

BOOL GeometryScreenSaver::OpenFileDialog(wchar_t *pstr)
{
	TCHAR tempStr[128], title[8];
	const TCHAR filter[] = TEXT("\0*.bmp;*.jpg;*.jpeg;*.png;*.dds;*.argb;*.tga\0\0");
	OPENFILENAME openfile = { 0 };
	lstrcpy(title, GetStringFromResource(IDS_STRING_OPENIMAGE_TITLE));
	lstrcpy(tempStr, GetStringFromResource(IDS_STRING_OPENIMAGE_FILTER_DESC_IMAGE));
	int n = lstrlen(tempStr);
	for (int i = 0; i < ARRAYSIZE(filter); i++)
		tempStr[n + i] = filter[i];
	openfile.lStructSize = sizeof(OPENFILENAME);
	openfile.hwndOwner = hDialog;
	openfile.hInstance = NULL;
	openfile.lpstrFilter = tempStr;
	openfile.lpstrFile = pstr;
	openfile.lpstrTitle = title;
	openfile.nMaxFile = MAX_PATH;
	openfile.lpstrFileTitle = NULL;
	openfile.nMaxFileTitle = MAX_PATH;
	openfile.Flags = OFN_HIDEREADONLY | OFN_FILEMUSTEXIST;
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