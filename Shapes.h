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
	int x, y;//坐标
	int radius;//图形大小
	int vx, vy;//水平/垂直移动速度
	int vrotate;//旋转速度，角度制
	unsigned char color[4];//颜色，0=R, 1=G, 2=B, 3=A
	int polygons;//0=圆,1=三角形,2=正方形,3=五边形
	bool crF1;//颜色分量位置移动方向，T=R->G->B->R, F=R->B->G->R
	bool crF2;//颜色 F=增，T=减
	unsigned char crF3;//处理颜色分量位置，0=R, 1=G, 2=B, 3=A
	int crTransSpeed;
	int angle;//旋转角度，角度制
};