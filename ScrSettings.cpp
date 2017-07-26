#include "ScrSettings.h"
#include "resource.h"

ScrSettings::ScrSettings()
{
	LoadDefaultScrSettings();
}

ScrSettings::~ScrSettings()
{
}

void ScrSettings::SetScrTimeFormat(BOOL bY, BOOL bD, BOOL bT, BOOL bS, BOOL bW)
{
	timeShowDate = bD;
	timeShowYear = bD ? bY : FALSE;
	timeShowTime = bT;
	timeShowSec = bT ? bS : FALSE;
	timeShowWeek = bD ? bW : FALSE;
}

BOOL ScrSettings::GetScrTimeFormat(int ydtsw)
{
	int ret = 0;
	switch (ydtsw)
	{
	case 0:
	case 1:
		ret += (int)timeShowYear;
		if (ydtsw == 1)return ret;
		ret <<= 1;
	case 2:
		ret += (int)timeShowDate;
		if (ydtsw == 2)return ret;
		ret <<= 1;
	case 3:
		ret += (int)timeShowTime;
		if (ydtsw == 3)return ret;
		ret <<= 1;
	case 4:
		ret += (int)timeShowSec;
		if (ydtsw == 4)return ret;
		ret <<= 1;
	case 5:
		ret += (int)timeShowWeek;
	}
	return ret;
}

void ScrSettings::SaveScrSettingsToFile(wchar_t *filename, wchar_t *appname)
{
	WritePrivateProfileString(appname, TEXT(VAR_TO_STRING(backgroundColor)), ntostr(backgroundColor, stringbuf), filename);
	WritePrivateProfileString(appname, TEXT(VAR_TO_STRING(backgroundSplash)), backgroundSplash, filename);
	WritePrivateProfileString(appname, TEXT(VAR_TO_STRING(fitBorder)), ntostr(fitBorder, stringbuf), filename);
	WritePrivateProfileString(appname, TEXT(VAR_TO_STRING(enableCoverLayer)), ntostr(enableCoverLayer, stringbuf), filename);
	WritePrivateProfileString(appname, TEXT(VAR_TO_STRING(coverColor)), ntostr(coverColor, stringbuf), filename);
	WritePrivateProfileString(appname, TEXT(VAR_TO_STRING(coverSplash)), coverSplash, filename);
	WritePrivateProfileString(appname, TEXT(VAR_TO_STRING(coverFitBorder)), ntostr(coverFitBorder, stringbuf), filename);
	WritePrivateProfileString(appname, TEXT(VAR_TO_STRING(maxFalldownSpeed)), ntostr(maxFalldownSpeed, stringbuf), filename);
	WritePrivateProfileString(appname, TEXT(VAR_TO_STRING(minFalldownSpeed)), ntostr(minFalldownSpeed, stringbuf), filename);
	WritePrivateProfileString(appname, TEXT(VAR_TO_STRING(maxHorizonalSpeed)), ntostr(maxHorizonalSpeed, stringbuf), filename);
	WritePrivateProfileString(appname, TEXT(VAR_TO_STRING(maxNTriangle)), ntostr(maxNTriangle, stringbuf), filename);
	WritePrivateProfileString(appname, TEXT(VAR_TO_STRING(maxNSquare)), ntostr(maxNSquare, stringbuf), filename);
	WritePrivateProfileString(appname, TEXT(VAR_TO_STRING(maxNFivePolygons)), ntostr(maxNFivePolygons, stringbuf), filename);
	WritePrivateProfileString(appname, TEXT(VAR_TO_STRING(maxNCircle)), ntostr(maxNCircle, stringbuf), filename);
	WritePrivateProfileString(appname, TEXT(VAR_TO_STRING(multiColoredBorder)), ntostr(multiColoredBorder, stringbuf), filename);
	WritePrivateProfileString(appname, TEXT(VAR_TO_STRING(pureColorBorder)), ntostr(pureColorBorder, stringbuf), filename);
	WritePrivateProfileString(appname, TEXT(VAR_TO_STRING(transformingColor)), ntostr(transformingColor, stringbuf), filename);
	WritePrivateProfileString(appname, TEXT(VAR_TO_STRING(colorTransMaxSpeed)), ntostr(colorTransMaxSpeed, stringbuf), filename);
	WritePrivateProfileString(appname, TEXT(VAR_TO_STRING(borderWidth)), ntostr(borderWidth, stringbuf), filename);
	WritePrivateProfileString(appname, TEXT(VAR_TO_STRING(timePosX)), ntostr(timePosX, stringbuf), filename);
	WritePrivateProfileString(appname, TEXT(VAR_TO_STRING(timePosY)), ntostr(timePosY, stringbuf), filename);
	WritePrivateProfileString(appname, TEXT(VAR_TO_STRING(maxRadius)), ntostr(maxRadius, stringbuf), filename);
	WritePrivateProfileString(appname, TEXT(VAR_TO_STRING(minRadius)), ntostr(minRadius, stringbuf), filename);
	WritePrivateProfileString(appname, TEXT(VAR_TO_STRING(timeColor)), ntostr(timeColor, stringbuf), filename);
	WritePrivateProfileString(appname, TEXT(VAR_TO_STRING(timeFontName)), timeFontName, filename);
	WritePrivateProfileString(appname, TEXT(VAR_TO_STRING(timeFontSize)), ntostr(timeFontSize, stringbuf), filename);
	WritePrivateProfileString(appname, TEXT(VAR_TO_STRING(timeFontThickness)), ntostr(timeFontThickness, stringbuf), filename);
	WritePrivateProfileString(appname, TEXT(VAR_TO_STRING(timeShowYear)), ntostr(timeShowYear, stringbuf), filename);
	WritePrivateProfileString(appname, TEXT(VAR_TO_STRING(timeShowDate)), ntostr(timeShowDate, stringbuf), filename);
	WritePrivateProfileString(appname, TEXT(VAR_TO_STRING(timeShowTime)), ntostr(timeShowTime, stringbuf), filename);
	WritePrivateProfileString(appname, TEXT(VAR_TO_STRING(timeShowSec)), ntostr(timeShowSec, stringbuf), filename);
	WritePrivateProfileString(appname, TEXT(VAR_TO_STRING(timeShowWeek)), ntostr(timeShowWeek, stringbuf), filename);
	WritePrivateProfileString(appname, TEXT(VAR_TO_STRING(maxRotateSpeed)), ntostr(maxRotateSpeed, stringbuf), filename);
	WritePrivateProfileString(appname, TEXT(VAR_TO_STRING(minRotateSpeed)), ntostr(minRotateSpeed, stringbuf), filename);
	WritePrivateProfileString(appname, TEXT(VAR_TO_STRING(windowWidth)), ntostr(windowWidth, stringbuf), filename);
	WritePrivateProfileString(appname, TEXT(VAR_TO_STRING(windowHeight)), ntostr(windowHeight, stringbuf), filename);
	WritePrivateProfileString(appname, TEXT(VAR_TO_STRING(randSeed)), ntostr(randSeed, stringbuf), filename);
}

void ScrSettings::LoadDefaultScrSettings()
{
	backgroundColor = 0xFF000000;
	lstrcpy(backgroundSplash, TEXT(""));
	fitBorder = FALSE;
	enableCoverLayer = FALSE;
	coverColor = 0xFF000000;
	lstrcpy(coverSplash, TEXT(""));
	coverFitBorder = FALSE;
	maxFalldownSpeed = 8;
	maxHorizonalSpeed = 10;
	maxNTriangle = 6;
	maxNSquare = 6;
	maxNFivePolygons = 6;
	maxNCircle = 2;
	multiColoredBorder = TRUE;
	pureColorBorder = 0xFF320167;
	transformingColor = TRUE;
	borderWidth = 3;
	timePosX = 10;
	timePosY = 80;
	maxRadius = 200;
	minRadius = 100;
	timeColor = 0xFFFFFFFF;
	LoadString(GetModuleHandle(NULL), IDS_STRING_FONTNAME_DEFAULT, timeFontName, sizeof(timeFontName) / sizeof(*timeFontName));
	timeFontSize = 48;
	SetScrTimeFormat(TRUE, TRUE, TRUE, FALSE, FALSE);
	maxRotateSpeed = 5;
	minRotateSpeed = -5;
	windowWidth = 640;
	windowHeight = 400;
	timeFontThickness = 4;
	minFalldownSpeed = 1;
	colorTransMaxSpeed = 1;
	randSeed = 0;
}

void ScrSettings::GetScrSettingsFromFile(wchar_t *filename, wchar_t *appname)
{
	GetPrivateProfileString(appname, TEXT(VAR_TO_STRING(backgroundSplash)), backgroundSplash, backgroundSplash, sizeof(backgroundSplash) / sizeof(*backgroundSplash), filename);
	GetPrivateProfileString(appname, TEXT(VAR_TO_STRING(timeFontName)), timeFontName, timeFontName, sizeof(timeFontName) / sizeof(*timeFontName), filename);
	backgroundColor = GetPrivateProfileInt(appname, TEXT(VAR_TO_STRING(backgroundColor)), backgroundColor, filename);
	enableCoverLayer = GetPrivateProfileInt(appname, TEXT(VAR_TO_STRING(enableCoverLayer)), enableCoverLayer, filename);
	GetPrivateProfileString(appname, TEXT(VAR_TO_STRING(coverSplash)), coverSplash, coverSplash, sizeof(coverSplash) / sizeof(*coverSplash), filename);
	coverColor = GetPrivateProfileInt(appname, TEXT(VAR_TO_STRING(coverColor)), coverColor, filename);
	fitBorder = GetPrivateProfileInt(appname, TEXT(VAR_TO_STRING(fitBorder)), fitBorder, filename);
	coverFitBorder = GetPrivateProfileInt(appname, TEXT(VAR_TO_STRING(coverFitBorder)), coverFitBorder, filename);
	maxFalldownSpeed = GetPrivateProfileInt(appname, TEXT(VAR_TO_STRING(maxFalldownSpeed)), maxFalldownSpeed, filename);
	minFalldownSpeed = GetPrivateProfileInt(appname, TEXT(VAR_TO_STRING(minFalldownSpeed)), minFalldownSpeed, filename);
	maxHorizonalSpeed = GetPrivateProfileInt(appname, TEXT(VAR_TO_STRING(maxHorizonalSpeed)), maxHorizonalSpeed, filename);
	maxNTriangle = GetPrivateProfileInt(appname, TEXT(VAR_TO_STRING(maxNTriangle)), maxNTriangle, filename);
	maxNSquare = GetPrivateProfileInt(appname, TEXT(VAR_TO_STRING(maxNSquare)), maxNSquare, filename);
	maxNFivePolygons = GetPrivateProfileInt(appname, TEXT(VAR_TO_STRING(maxNFivePolygons)), maxNFivePolygons, filename);
	maxNCircle = GetPrivateProfileInt(appname, TEXT(VAR_TO_STRING(maxNCircle)), maxNCircle, filename);
	multiColoredBorder = GetPrivateProfileInt(appname, TEXT(VAR_TO_STRING(multiColoredBorder)), multiColoredBorder, filename);
	pureColorBorder = GetPrivateProfileInt(appname, TEXT(VAR_TO_STRING(pureColorBorder)), pureColorBorder, filename);
	transformingColor = GetPrivateProfileInt(appname, TEXT(VAR_TO_STRING(transformingColor)), transformingColor, filename);
	borderWidth = GetPrivateProfileInt(appname, TEXT(VAR_TO_STRING(borderWidth)), borderWidth, filename);
	timePosX = GetPrivateProfileInt(appname, TEXT(VAR_TO_STRING(timePosX)), timePosX, filename);
	timePosY = GetPrivateProfileInt(appname, TEXT(VAR_TO_STRING(timePosY)), timePosY, filename);
	maxRadius = GetPrivateProfileInt(appname, TEXT(VAR_TO_STRING(maxRadius)), maxRadius, filename);
	minRadius = GetPrivateProfileInt(appname, TEXT(VAR_TO_STRING(minRadius)), minRadius, filename);
	timeColor = GetPrivateProfileInt(appname, TEXT(VAR_TO_STRING(timeColor)), timeColor, filename);
	timeFontSize = GetPrivateProfileInt(appname, TEXT(VAR_TO_STRING(timeFontSize)), timeFontSize, filename);
	SetScrTimeFormat(GetPrivateProfileInt(appname, TEXT(VAR_TO_STRING(timeShowYear)), timeShowYear, filename),
		GetPrivateProfileInt(appname, TEXT(VAR_TO_STRING(timeShowDate)), timeShowDate, filename),
		GetPrivateProfileInt(appname, TEXT(VAR_TO_STRING(timeShowTime)), timeShowTime, filename),
		GetPrivateProfileInt(appname, TEXT(VAR_TO_STRING(timeShowSec)), timeShowSec, filename),
		GetPrivateProfileInt(appname, TEXT(VAR_TO_STRING(timeShowWeek)), timeShowWeek, filename));
	maxRotateSpeed = GetPrivateProfileInt(appname, TEXT(VAR_TO_STRING(maxRotateSpeed)), maxRotateSpeed, filename);
	minRotateSpeed = GetPrivateProfileInt(appname, TEXT(VAR_TO_STRING(minRotateSpeed)), minRotateSpeed, filename);
	windowWidth = GetPrivateProfileInt(appname, TEXT(VAR_TO_STRING(windowWidth)), windowWidth, filename);
	windowHeight = GetPrivateProfileInt(appname, TEXT(VAR_TO_STRING(windowHeight)), windowHeight, filename);
	timeFontThickness = GetPrivateProfileInt(appname, TEXT(VAR_TO_STRING(timeFontThickness)), timeFontThickness, filename);
	colorTransMaxSpeed = GetPrivateProfileInt(appname, TEXT(VAR_TO_STRING(colorTransMaxSpeed)), colorTransMaxSpeed, filename);
	randSeed = GetPrivateProfileInt(appname, TEXT(VAR_TO_STRING(randSeed)), randSeed, filename);
}

wchar_t * ScrSettings::ntostr(int source, wchar_t *buf, bool inhex)
{
	wsprintf(buf, inhex ? TEXT("%x") : TEXT("%d"), source);
	return buf;
}