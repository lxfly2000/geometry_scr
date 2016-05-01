#pragma once
#include <Windows.h>

#define VAR_TO_STRING(varname)#varname

class ScrSettings
{
public:
	ScrSettings();
	~ScrSettings();
	UINT backgroundColor;//���������ɫ��0xAABBGGRR
	wchar_t backgroundSplash[MAX_PATH];//����ͼ�ļ���
	BOOL fitBorder;//����ͼ�Ƿ���������Ļ�߽�
	unsigned maxFalldownSpeed;//ͼ���ڴ�ֱ�������µ�����ٶ�
	unsigned maxHorizonalSpeed;//ͼ����ˮƽ�������µ�����ٶ�
	unsigned maxNTriangle;//�����ε��������
	unsigned maxNSquare;//�����ε��������
	unsigned maxNFivePolygons;//����ε��������
	unsigned maxNCircle;//Բ���������
	BOOL multiColoredBorder;//�Ƿ�ʹ�ò�ɫ�߿򣬼�ÿ��ͼ���Ƿ�ʹ�ò�ͬ����ɫ
	UINT pureColorBorder;//ʹ����ͬ��ɫ����ɫ���仯�Ļ���ʲô��ɫ��0xAABBGGRR
	BOOL transformingColor;//�߿���ɫ�Ƿ�仯
	int borderWidth;//�߿�Ŀ�ȣ�-1Ϊ��ȫ��䣬0Ϊ�ο�Ч��������ȫ��ԭ����
	unsigned char timePosX;//��ʾ����ʱ���λ��X��0Ϊ��100Ϊ��
	unsigned char timePosY;//��ʾ����ʱ���λ��Y��0Ϊ�ϣ�100Ϊ��
	unsigned maxRadius;//ͼ�ε����뾶
	unsigned minRadius;//ͼ�ε���С�뾶
	UINT timeColor;//����ʱ�����ɫ��0xAABBGGRR
	wchar_t timeFontName[MAX_PATH];//��ʱ��������
	unsigned timeFontSize;//��ʱ�������С
	unsigned timeFontThickness;//��ʱ���������أ�0��9���൱�� 100, 300, 400, 500, 900, 700
	int maxRotateSpeed;//�����ת�ٶȣ���Ϊ�������Ƕ���
	int minRotateSpeed;//��С��ת�ٶȣ���Ϊ�������Ƕ���
	unsigned windowWidth;//����ģʽʱ���
	unsigned windowHeight;//����ģʽʱ�߶�
	unsigned minFalldownSpeed;//ͼ����ˮƽ�������µ���С�ٶ�
	unsigned colorTransMaxSpeed;//��ɫ�仯������ٶ�
	//��������ʱ�����ʾ��ʽ���ֱ�Ϊ�Ƿ���ʾ�꣬���ڣ�ʱ�䣬�룬����
	void SetScrTimeFormat(BOOL, BOOL, BOOL, BOOL, BOOL);
	//��ȡ����ʱ�����ʾ��ʽ��0=ȫ������λ����1-5�ֱ�Ϊ�Ƿ���ʾ�꣬���ڣ�ʱ�䣬�룬����
	BOOL GetScrTimeFormat(int);

	//���ļ��ж�ȡ����
	void GetScrSettingsFromFile(wchar_t*, wchar_t*);
	//д�����õ��ļ�
	void SaveScrSettingsToFile(wchar_t*, wchar_t*);
	//����Ĭ������
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
