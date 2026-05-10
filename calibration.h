#include <stdint.h>

typedef struct {
	int x;
	int y;
} Point;

typedef struct {
	float a, b, c;
	float d, e, f;
} CalibrationMatrix;

void calib_calculate(Point lcd[3], Point touch[3], CalibrationMatrix* Matrix);

void calib_transform(CalibrationMatrix* matrix, int tx, int ty, int* lcd_x, int* lcd_y);