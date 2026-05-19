#ifndef DISPLAY_H
#define DISPLAY_H

#include "LPC17xx.h"
#include "lcd_lib/Open1768_LCD.h"
#include "lcd_lib/LCD_ILI9325.h"
#include "lcd_lib/asciiLib.h"

#define START_X 30
#define START_Y 70
#define CELL_WIDTH 15
#define BOARD_SIZE 12

typedef enum {
	UP = 0,
	RIGHT = 1,
	DOWN = 2,
	LEFT = 3
} Direction;

void draw_line(uint16_t x, uint16_t y, uint16_t len, Direction direction, uint16_t color);
	
void draw_gameboard();

void fill_rectangle(uint16_t x, uint16_t y, uint16_t color);

void draw_ship(uint16_t x, uint16_t y, uint8_t size, Direction direction, uint16_t color);

void which_cell(int x, int y, int *idx_x, int *idx_y);

void WriteChar(int x, int y, char c, uint16_t color, int scale);

#endif