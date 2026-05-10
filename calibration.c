#include "calibration.h"

void calib_calculate(Point lcd[3], Point touch[3], CalibrationMatrix* m){
	float x1 = touch[0].x;
	float y1 = touch[0].y;
	float x2 = touch[1].x;
	float y2 = touch[1].y;
	float x3 = touch[2].x;
	float y3 = touch[2].y;
	
	float X1 = lcd[0].x;
	float Y1 = lcd[0].y;
	float X2 = lcd[1].x;
	float Y2 = lcd[1].y;
	float X3 = lcd[2].x;
	float Y3 = lcd[2].y;
	
	float W = x1*y2 + y1*x3 + x2*y3 - y2*x3-x1 * y3-x2*y1;
	float Wa = X1*y1 + y1*X3 + X2*y3 - y2*X3 - X1*y3 - X2*y1;
	float Wb = x1*X2 + X1*x3 + x2*X3 - X2*x3 - x1*X3 - x2 * X1;
	float Wc = x1*y2*X3 + y1*X2 * x3 + X1*x2*y3 - X1*y2*x3 - y3*X2*x1 - X3 *x2*y1;
	float Wd = Y1*y1 + y1*Y3 + Y2*y3 - y2*Y3 - Y1*y3 - Y2*y1;
	float We = x1*Y2 + Y1*x3 + x2*Y3 - Y2*x3 - x1*Y3 - x2 * Y1;
	float Wf = x1*y2*Y3 + y1*Y2 * x3 + Y1*x2*y3 - Y1*y2*x3 - y3*Y2*x1 - Y3 *x2*y1;
	
	m->a = Wa/W;
	m->b = Wb/W;
	m->c = Wc/W;
	m->d = Wd/W;
	m->e = We/W;
	m->f = Wf/W;
}

void calib_transform(CalibrationMatrix* m, int tx, int ty, int* lcd_x, int* lcd_y){
	*lcd_x = (int)(m->a * tx + m->b * ty + m->c);
	*lcd_y = (int)(m->c * tx + m->d * ty + m->e);
}