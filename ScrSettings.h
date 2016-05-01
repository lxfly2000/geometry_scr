#pragma once
#include <Windows.h>

#define VAR_TO_STRING(varname)#varname

class ScrSettings
{
public:
	ScrSettings();
	~ScrSettings();
	UINT backgroundColor;//背景清除颜色，0xAABBGGRR
	wchar_t backgroundSplash[MAX_PATH];//背景图文件名
	BOOL fitBorder;//背景图是否缩放至屏幕边界
	unsigned maxFalldownSpeed;//图形在垂直方向落下的最大速度
	unsigned maxHorizonalSpeed;//图形在水平方向落下的最大速度
	unsigned maxNTriangle;//三角形的最大数量
	unsigned maxNSquare;//正方形的最大数量
	unsigned maxNFivePolygons;//五边形的最大数量
	unsigned maxNCircle;//圆的最大数量
	BOOL multiColoredBorder;//是否使用彩色边框，即每个图形是否使用不同的颜色
	UINT pureColorBorder;//使用相同颜色且颜色不变化的话用什么颜色，0xAABBGGRR
	BOOL transformingColor;//边框颜色是否变化
	int borderWidth;//边框的宽度，-1为完全填充，0为镂空效果（即完全仿原作）
	unsigned char timePosX;//显示日期时间的位置X，0为左，100为右
	unsigned char timePosY;//显示日期时间的位置Y，0为上，100为下
	unsigned maxRadius;//图形的最大半径
	unsigned minRadius;//图形的最小半径
	UINT timeColor;//日期时间的颜色，0xAABBGGRR
	wchar_t timeFontName[MAX_PATH];//日时的字体名
	unsigned timeFontSize;//日时的字体大小
	unsigned timeFontThickness;//日时的字体字重，0～9，相当于 100, 300, 400, 500, 900, 700
	int maxRotateSpeed;//最大旋转速度，可为负数，角度制
	int minRotateSpeed;//最小旋转速度，可为负数，角度制
	unsigned windowWidth;//窗口模式时宽度
	unsigned windowHeight;//窗口模式时高度
	unsigned minFalldownSpeed;//图形在水平方向落下的最小速度
	unsigned colorTransMaxSpeed;//颜色变化的最大速度
	//设置日期时间的显示格式，分别为是否显示年，日期，时间，秒，星期
	void SetScrTimeFormat(BOOL, BOOL, BOOL, BOOL, BOOL);
	//获取日期时间的显示格式，0=全部（按位），1-5分别为是否显示年，日期，时间，秒，星期
	BOOL GetScrTimeFormat(int);

	//从文件中读取设置
	void GetScrSettingsFromFile(wchar_t*, wchar_t*);
	//写入设置到文件
	void SaveScrSettingsToFile(wchar_t*, wchar_t*);
	//加载默认设置
	void LoadDefaultScrSettings();
private:
	BOOL timeShowYear;
	BOOL timeShowDate;
	BOOL timeShowTime;
	BOOL timeShowSec;
	BOOL timeShowWeek;
	wchar_t stringbuf[256];
	wchar_t *ntostr(int, wchar_t*, bool = false);
};
