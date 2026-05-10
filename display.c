#include "display.h"

void draw_line(uint16_t x, uint16_t y, uint16_t len, Direction direction, uint16_t color) {
	switch (direction) {
		case UP:
			for (int i = x; i < x + len; i++) {
				lcdSetCursor(i, y);
				lcdWriteReg(0x22, color);
			}
			break;
		case RIGHT:
			for (int i = y; i < y + len; i++) {
				lcdSetCursor(x, i);
				lcdWriteReg(0x22, color);
			}
			break;
		case LEFT:
			for (int i = y; i > y - len; i--) {
				lcdSetCursor(x, i);
				lcdWriteReg(0x22, color);
			}
			break;
		case DOWN:
			for (int i = x; i > x - len; i--) {
				lcdSetCursor(i, y);
				lcdWriteReg(0x22, color);
			}
			break;
	}
}
	
void draw_gameboard() {
	for (int i = 0; i < BOARD_SIZE + 1; i++) {
		draw_line(START_X, START_Y + CELL_WIDTH * i, 180, UP, LCDBlack);
	}
	
	for (int j = 0; j < BOARD_SIZE + 1; j++) {
		draw_line(START_X + CELL_WIDTH * j, START_Y, 180, RIGHT, LCDBlack);
	}
}

void fill_rectangle(uint16_t x, uint16_t y, uint16_t color) {
	lcdWriteReg(0x0003, 0);
	for(int i = y + 1; i < y + CELL_WIDTH; i++ ){
		lcdSetCursor(x + CELL_WIDTH - 1, i);
		for(int j = x; j > x - CELL_WIDTH - 1; j-- ){
			lcdWriteIndex(0x0022);
			lcdWriteData(color);
		}
	}
}

void draw_ship(uint16_t x, uint16_t y, uint8_t size, Direction direction, uint16_t color) {
	lcdWriteReg(0, 0);
	switch (direction) {
		case UP:
			for (int i = 0; i < size; i++) {
				fill_rectangle(x + CELL_WIDTH * i, y, color);
			}
			break;
		case RIGHT:
			for (int i = 0; i < size; i++) {
				fill_rectangle(x, y + CELL_WIDTH * i, color);
			}
			break;
		case LEFT:
			for (int i = 0; i < size; i++) {
				fill_rectangle(x, y - CELL_WIDTH * i, color);
			}
			break;
		case DOWN:
			for (int i = 0; i < size; i++) {
				fill_rectangle(x - CELL_WIDTH * i, y, color);
			}
			break;
	}
}

void WriteChar(int x, int y, char c, uint16_t color, int scale) {
	unsigned char pixels[16];
	GetASCIICode(ASCII_8X16_System, pixels, c);

	for (int i = 0; i < 16; i++) {
		for (int j = 0; j < 8; j++) {
			if (pixels[i] & ( 0x01 << j )) {
				for (int k = 0; k < scale; k++) {
					lcdSetCursor(y + i * scale + k, x + j * scale + k); // minus bo ekran do gory nogami
					lcdWriteIndex(0x0022);
					lcdWriteData(color);
				}
			}
		}
	}
}