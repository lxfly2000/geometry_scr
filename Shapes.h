#pragma once
struct Shapes
{
	Shapes(int _x, int _y, int _radi, int _a, int _vx, int _vy, int _vrot, int _cr, int _poly, int _crt, int flag) :
		x(_x), y(_y), radius(_radi), angle(_a), vx(_vx), vy(_vy), vrotate(_vrot), polygons(_poly), crTransSpeed(_crt)
	{
		color[0] = (_cr & 0x00FF0000) >> 16;
		color[1] = (_cr & 0x0000FF00) >> 8;
		color[2] = _cr & 0x000000FF;
		color[3] = (_cr & 0xFF000000) >> 24;

		crF1 = ((flag >> 16) & 0x01) == 0 ? false : true;
		crF2 = ((flag >> 8) & 0x01) == 0 ? false : true;
		crF3 = (unsigned char)(flag & 0xFF);
	}
	int x, y;//����
	int radius;//ͼ�δ�С
	int vx, vy;//ˮƽ/��ֱ�ƶ��ٶ�
	int vrotate;//��ת�ٶȣ��Ƕ���
	unsigned char color[4];//��ɫ��0=R, 1=G, 2=B, 3=A
	int polygons;//0=Բ,1=������,2=������,3=�����
	bool crF1;//��ɫ����λ���ƶ�����T=R->G->B->R, F=R->B->G->R
	bool crF2;//��ɫ F=����T=��
	unsigned char crF3;//������ɫ����λ�ã�0=R, 1=G, 2=B, 3=A
	int crTransSpeed;
	int angle;//��ת�Ƕȣ��Ƕ���
};