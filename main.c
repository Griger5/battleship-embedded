#include "LPC17xx.h"
#include "Driver_SPI.h"
#include "Driver_USART.h"
#include "lcd_lib/Open1768_LCD.h"
#include "lcd_lib/LCD_ILI9325.h"
#include "tp_lib/TP_Open1768.h"
#include "Board_Buttons.h"

#include "display.h"
#include "calibration.h"
#include "game_logic/battleship.h"

#include <stdlib.h>

void myUART_Thread(void const *argument);
extern ARM_DRIVER_USART Driver_USART1;

static ARM_DRIVER_USART * USARTdrv = &Driver_USART1;
char cmd[5];

void myUART_Thread(const void* args){
	USARTdrv->Send("\nHello my dear friend!", 22);
}

typedef enum {
	CAL,
	START,
	SET,
	MY_TURN,
	OPPONENT_TURN,
	FINISH
} State;

static State state = CAL;

volatile uint8_t touch_pending = 0;

void fill( uint16_t color ){
	lcdWriteReg(0x0003, 0);
	for(int i = 0; i < 320; i++ ){
		lcdSetCursor(0, i);
		lcdWriteIndex(0x0022);
		for(int j = 0; j < 240; j++ ){
			lcdWriteData(color);
		}
	}
}

bool insideTouchArea(int x, int y) {
	if (x > 10 && x < 2800 && y > 10 && y < 3500) {
		return true;
	}
	return false;
}

volatile uint32_t msTicks = 0;

void wait(uint32_t t) {
	uint32_t start = msTicks;
	while (msTicks < start + t);
}

void EINT3_IRQHandler(){
	switch (state) {
		case CAL: {
			LPC_GPIOINT->IO0IntEnF &= ~(1 << 19);
			LPC_GPIOINT->IO0IntClr =  (1 << 19);
			touch_pending = 1;
			break;
		}
		case START:
			break;
		case SET:
			break;
		case MY_TURN:
			break;
		case OPPONENT_TURN:
			break;
		case FINISH:
			break;
	}
	LPC_GPIOINT->IO0IntClr = ( 1 << 19 );
}

volatile Direction dir = RIGHT;
Direction directions[4] = {DOWN, LEFT, UP, RIGHT};

volatile uint32_t buttonClicks = 0;
uint32_t debouncerTimer = 0;

volatile uint8_t confirm_ship_pending = 0;
volatile uint8_t confirm_shot_pending = 0;

void SysTick_Handler(void) {
    msTicks++;
    uint32_t button = Buttons_GetState();
    static uint8_t i = 0;

    //key2 - obrót statku
    if (button & 1) {
        if (msTicks >= debouncerTimer + 200) {
            if (state == SET) {
                dir = directions[i];
                i = (i + 1) % 4;
            }
            debouncerTimer = msTicks;
        }
    }

    //key1 — zatwierdzanie 
    if (button & 2) {
        if (msTicks >= debouncerTimer + 200) {
            if (state == SET) {
                confirm_ship_pending = 1;
            } 
            else if (state == MY_TURN) {
                confirm_shot_pending = 1;
            }
            debouncerTimer = msTicks;
        }
    }
}

static void anchor_to_place(uint16_t draw_x, uint16_t draw_y, uint8_t size, Direction d, uint8_t *out_row, uint8_t *out_col, uint8_t *out_horizontal) {
	uint16_t bound_x = START_X / CELL_WIDTH;
	uint16_t bound_y = START_Y / CELL_WIDTH;
	switch (d) {
		case UP:
			*out_row        = (uint8_t)(draw_y - bound_y);
			*out_col        = (uint8_t)(draw_x - bound_x);
			*out_horizontal = 1;
			break;
		case DOWN:
			*out_row        = (uint8_t)(draw_y - bound_y);
			*out_col        = (uint8_t)(draw_x - (size - 1) - bound_x);
			*out_horizontal = 1;
			break;
		case RIGHT:
			*out_row        = (uint8_t)(draw_y - bound_y);
			*out_col        = (uint8_t)(draw_x - bound_x);
			*out_horizontal = 0;
			break;
		case LEFT:
			*out_row        = (uint8_t)(draw_y - (size - 1) - bound_y);
			*out_col        = (uint8_t)(draw_x - bound_x);
			*out_horizontal = 0;
			break;
	}
}

static uint8_t ship_fits_on_board(uint16_t draw_x, uint16_t draw_y, uint8_t size, Direction d) {
	int16_t bx_min = START_X / CELL_WIDTH;
	int16_t by_min = START_Y / CELL_WIDTH;
	int16_t bx_max = bx_min + BOARD_SIZE - 1;
	int16_t by_max = by_min + BOARD_SIZE - 1;

	for (uint8_t s = 0; s < size; s++) {
		int16_t cx = (int16_t)draw_x;
		int16_t cy = (int16_t)draw_y;
		switch (d) {
			case UP:    
				cx = (int16_t)draw_x + s; 
				break;
			case DOWN:  
				cx = (int16_t)draw_x - s; 
				break;
			case RIGHT: 
				cy = (int16_t)draw_y + s; 
				break;
			case LEFT:  
				cy = (int16_t)draw_y - s; 
				break;
		}
		if (cx < bx_min || cx > bx_max || cy < by_min || cy > by_max)
			return 0;
	}
	return 1;
}

static void erase_preview(const Player *p, uint16_t draw_x, uint16_t draw_y, uint8_t size, Direction d) {
	uint16_t bound_x = START_X / CELL_WIDTH;
	uint16_t bound_y = START_Y / CELL_WIDTH;
	for (uint8_t s = 0; s < size; s++) {
		int16_t cx = (int16_t)draw_x;
		int16_t cy = (int16_t)draw_y;
		switch (d) {
			case UP:    cx = (int16_t)draw_x + s; break;
			case DOWN:  cx = (int16_t)draw_x - s; break;
			case RIGHT: cy = (int16_t)draw_y + s; break;
			case LEFT:  cy = (int16_t)draw_y - s; break;
		}
		if (cx < 0 || cy < 0) continue;
		uint8_t row = (uint8_t)((uint16_t)cy - bound_y);
		uint8_t col = (uint8_t)((uint16_t)cx - bound_x);
		if (row < BOARD_SIZE && col < BOARD_SIZE) {
			if (board_get(p->own, row, col) == WATER) {
				fill_rectangle((uint16_t)cx * CELL_WIDTH, (uint16_t)cy * CELL_WIDTH, LCDWhite);
			}
		}
	}
	draw_gameboard();
}

static char rx_buf[16];
static uint8_t rx_idx = 0;

//nieblokujacy UART
bool UART_ReadCommand(char *out_cmd) {
    if (USARTdrv->GetRxCount() > 0) {
        char c;
        USARTdrv->Receive(&c, 1);
        
        if (c == '\n' || c == '\r') {
            if (rx_idx > 0) {
                rx_buf[rx_idx] = '\0';
                strcpy(out_cmd, rx_buf);
                rx_idx = 0;
                return true;
            }
        } else if (rx_idx < sizeof(rx_buf) - 1) {
            rx_buf[rx_idx++] = c;
        }
    }
    return false;
}

int main() {
	int return_code = SysTick_Config(SystemCoreClock/1000);
	lcdConfiguration();
	init_ILI9325();
	touchpanelInit();

	LPC_GPIO0->FIODIR &= ~(1 << 19 );
	LPC_GPIOINT->IO0IntEnF |= ( 1 << 19 );
	LPC_SC->EXTINT |= ( 1 << 3 );
	LPC_SC->EXTMODE |= ( 1 << 3 );
	LPC_SC->EXTPOLAR |= ( 0 << 3 );
	NVIC_EnableIRQ(EINT3_IRQn);

	USARTdrv->Initialize(NULL);
	USARTdrv->PowerControl(ARM_POWER_FULL);
	USARTdrv->Control(ARM_USART_MODE_ASYNCHRONOUS | ARM_USART_DATA_BITS_8 |
	                  ARM_USART_PARITY_NONE | ARM_USART_STOP_BITS_1 |
	                  ARM_USART_FLOW_CONTROL_NONE, 4800);
	USARTdrv->Control(ARM_USART_CONTROL_TX, 1);
	USARTdrv->Control(ARM_USART_CONTROL_RX, 1);
	myUART_Thread(NULL);

	fill(LCDWhite);

	CalibrationMatrix calib;
	Point lcd[3]   = {{70, 30}, {120, 150}, {250, 80}};
	Point touch[3] = {{}, {}, {}};

	int x;
	int y;
	int x_lcd;
	int y_lcd;
	int counter = 0;

	WriteChar(lcd[0].x - 4, lcd[0].y - 6, 'X', LCDRed, 1);
	WriteChar(lcd[1].x - 4, lcd[1].y - 6, 'X', LCDRed, 1);
	WriteChar(lcd[2].x - 4, lcd[2].y - 6, 'X', LCDRed, 1);

	wait(2);
	USARTdrv->Send("\nZZZZZZZ\n", 10);

	Player player;
	player_init(&player);

	const ShipDef *ships = ship_defs();
	uint8_t curr_ship_idx = 0;

	uint8_t prev_draw_x = 0;
	uint8_t prev_draw_y = 0;
	Direction prev_dir = RIGHT;

	while (1){
		char received_cmd[16];
		bool has_cmd = UART_ReadCommand(received_cmd);
		
		switch (state) {

			case CAL: {
				#define SAMPLES 50
				if (!touch_pending) break;
				touch_pending = 0;

				int x, y;
				int sum_x = 0, sum_y = 0;

				for (int i = 0; i < SAMPLES; i++) {
					touchpanelGetXY(&y, &x);
					if (insideTouchArea(x, y)) {
						sum_x += x;
						sum_y += y;
					}
					wait(20);
				}

				int avg_x = sum_x / SAMPLES;
				int avg_y = sum_y / SAMPLES;

				if (insideTouchArea(avg_x, avg_y)) {
					touch[counter].x = avg_x;
					touch[counter].y = avg_y;
					counter++;

					char str[20] = {0};
					sprintf(str, "P%d: %d %d\n", counter, avg_x, avg_y);
					USARTdrv->Send(str, 12);

					WriteChar(lcd[counter-1].x - 4, lcd[counter-1].y - 6, 'O', LCDGreen, 1);
					wait(2000);
				}

				if (counter == 3) {
					calib_calculate(lcd, touch, &calib);
					USARTdrv->Send("git\n", 4);
					state = START;
					fill(LCDWhite);
					counter = 0;
				} else {
					wait(500);
					LPC_GPIOINT->IO0IntEnF |= (1 << 19);
				}
				break;
			}

			case START: {
				USARTdrv->Send("counter done", 12);
				draw_gameboard();
				state = SET;
				break;
			}

			case SET: {
                int x, y, lcd_x, lcd_y;
                
                
                if (prev_dir != dir && prev_draw_x != 0 && prev_draw_y != 0) {
                    erase_preview(&player, prev_draw_x, prev_draw_y,
                                  ships[curr_ship_idx].size, prev_dir);
                    
                    if (ship_fits_on_board(prev_draw_x, prev_draw_y, ships[curr_ship_idx].size, dir)) {
                        draw_ship(prev_draw_x * CELL_WIDTH, prev_draw_y * CELL_WIDTH, ships[curr_ship_idx].size, dir, LCDBlue);
                        prev_dir = dir; 
                    } else {
                        dir = prev_dir;
                        draw_ship(prev_draw_x * CELL_WIDTH, prev_draw_y * CELL_WIDTH, ships[curr_ship_idx].size, dir, LCDBlue);
                    }
                }

                touchpanelGetXY(&y, &x);

                if (insideTouchArea(x, y)) {
                    calib_transform(&calib, y, x, &lcd_x, &lcd_y);

                    uint16_t draw_x = lcd_x / CELL_WIDTH;
                    uint16_t draw_y = (lcd_y + 45) / CELL_WIDTH;
                    uint16_t bound_x = START_X / CELL_WIDTH;
                    uint16_t bound_y = START_Y / CELL_WIDTH;

                    if (draw_x < bound_x) {
                        ;
                    } else if (draw_x > bound_x + BOARD_SIZE) {
                        ;
                    } else if (draw_y < bound_y) {
                        ;
                    } else if (draw_y > bound_y + BOARD_SIZE) {
                        ;
                    } else if (!ship_fits_on_board(draw_x, draw_y, ships[curr_ship_idx].size, dir)) {
                        ;
                    } else {
                        if (prev_draw_x != draw_x || prev_draw_y != draw_y || prev_dir != dir) {
                            erase_preview(&player, prev_draw_x, prev_draw_y, ships[curr_ship_idx].size, prev_dir);
                            prev_draw_x = draw_x;
                            prev_dir    = dir;
                            prev_draw_y = draw_y;
                        }
                        
                        draw_ship(draw_x * CELL_WIDTH, draw_y * CELL_WIDTH, ships[curr_ship_idx].size, dir, LCDBlue);
                    }

                    char str[3] = {0};
                    sprintf(str, "%d", lcd_x);
                    USARTdrv->Send(str, 4);
                    USARTdrv->Send("   ", 3);
                    sprintf(str, "%d", lcd_y);
                    USARTdrv->Send(str, 4);
                }

                //key1 zatwierdza
                if (confirm_ship_pending) {
                    confirm_ship_pending = 0;

                    uint16_t bound_x = START_X / CELL_WIDTH;
                    uint16_t bound_y = START_Y / CELL_WIDTH;

                    if (prev_draw_x >= bound_x && prev_draw_x <= bound_x + BOARD_SIZE &&
                        prev_draw_y >= bound_y && prev_draw_y <= bound_y + BOARD_SIZE &&
                        ship_fits_on_board(prev_draw_x, prev_draw_y, ships[curr_ship_idx].size, prev_dir))
                    {
                        uint8_t row, col, horizontal;
                        anchor_to_place(prev_draw_x, prev_draw_y,
                                        ships[curr_ship_idx].size, prev_dir,
                                        &row, &col, &horizontal);

                        PlaceResult result = place_ship(&player, curr_ship_idx,
                                                        row, col, horizontal);

                        if (result == PLACE_OK) {
                            char info[20] = {0};
                            sprintf(info, "Ship%d OK\n", curr_ship_idx);
                            USARTdrv->Send(info, 10);

                            curr_ship_idx++;

                            if (curr_ship_idx >= NUM_SHIPS) {
                                USARTdrv->Send("ALL PLACED\n", 11);
                                state = MY_TURN;
                            } else {
                                prev_draw_x = 0;
                                prev_draw_y = 0;
                                prev_dir    = dir;
                            }
                        } else {
                            USARTdrv->Send("INVALID\n", 8);
                        }
                    }
                }
                break;
            }

			case MY_TURN: {
                /*int x, y, lcd_x, lcd_y;
                static int8_t aim_row = -1;
                static int8_t aim_col = -1;
                
                touchpanelGetXY(&y, &x);

                if (insideTouchArea(x, y)) {
                    calib_transform(&calib, y, x, &lcd_x, &lcd_y);
                    
                    int8_t shot_col = (lcd_x / CELL_WIDTH) - (START_X / CELL_WIDTH);
                    int8_t shot_row = ((lcd_y + 45) / CELL_WIDTH) - (START_Y / CELL_WIDTH);

                    if (shot_col >= 0 && shot_col < BOARD_SIZE && shot_row >= 0 && shot_row < BOARD_SIZE) {
                        if (board_get(player.track, shot_row, shot_col) == WATER) {
                            
                            if (aim_row != shot_row || aim_col != shot_col) {

                                if (aim_row != -1 && board_get(player.track, aim_row, aim_col) == WATER) {
                                    fill_rectangle((aim_col + START_X/CELL_WIDTH) * CELL_WIDTH, 
                                                   (aim_row + START_Y/CELL_WIDTH) * CELL_WIDTH, LCDWhite);
                                }
                                
                                aim_row = shot_row;
                                aim_col = shot_col;
                                
                                fill_rectangle((aim_col + START_X/CELL_WIDTH) * CELL_WIDTH, (aim_row + START_Y/CELL_WIDTH) * CELL_WIDTH, 0xF7BE);
                            }
                        }
                    }
                }

                //zatwierdzenie key1
                if (confirm_shot_pending) {
                    confirm_shot_pending = 0; 
					
                    if (aim_row >= 0 && aim_col >= 0) {
                        char shot_msg[10];
                        sprintf(shot_msg, "S:%d%d\n", aim_row, aim_col);
                        USARTdrv->Send(shot_msg, strlen(shot_msg));
                        
                        aim_row = -1;
                        aim_col = -1;
                        
                        state = OPPONENT_TURN;
                        wait(300); 
                    }
                }*/
                break;
            }

            case OPPONENT_TURN: {
                /*if (has_cmd) {
                    
                    if (strcmp(received_cmd, "RDY") == 0) {
                        state = MY_TURN;
                        USARTdrv->Send("START_GAME\n", 11);
                    }
                    
                    else if (received_cmd[0] == 'S') {
                        uint8_t enemy_row = received_cmd[2] - '0';
                        uint8_t enemy_col = received_cmd[3] - '0';
                        
                        ShotResult res = receive_shot(&player, enemy_row, enemy_col);
                        
                        if (res == SHOT_HIT || res == SHOT_SUNK || res == SHOT_WIN) {
                            fill_rectangle((enemy_col + START_X/CELL_WIDTH) * CELL_WIDTH, 
                                           (enemy_row + START_Y/CELL_WIDTH) * CELL_WIDTH, LCDRed);
                        } else if (res == SHOT_MISS) {
                            fill_rectangle((enemy_col + START_X/CELL_WIDTH) * CELL_WIDTH, 
                                           (enemy_row + START_Y/CELL_WIDTH) * CELL_WIDTH, LCDGrey);
                        }

                        char response[10];
                        sprintf(response, "R:%d%d%d\n", res, enemy_row, enemy_col);
                        USARTdrv->Send(response, strlen(response));
                        
                        if (res == SHOT_WIN) {
                            USARTdrv->Send("LOSE\n", 5);
                            state = FINISH;
                        } else {
                            state = MY_TURN;
                        }
                        
                        draw_gameboard();
                    }
                    
                    else if (received_cmd[0] == 'R') {
                        ShotResult my_res = (ShotResult)(received_cmd[2] - '0');
                        uint8_t my_row    = received_cmd[3] - '0';
                        uint8_t my_col    = received_cmd[4] - '0';
                        
                        record_shot(&player, my_row, my_col, my_res);
                        
                        if (my_res == SHOT_HIT || my_res == SHOT_SUNK || my_res == SHOT_WIN) {
                            fill_rectangle((my_col + START_X/CELL_WIDTH) * CELL_WIDTH, 
                                           (my_row + START_Y/CELL_WIDTH) * CELL_WIDTH, LCDRed);
                        } else {
                            fill_rectangle((my_col + START_X/CELL_WIDTH) * CELL_WIDTH, 
                                           (my_row + START_Y/CELL_WIDTH) * CELL_WIDTH, LCDGrey);
                        }

                        if (my_res == SHOT_WIN) {
                            USARTdrv->Send("WIN\n", 4);
                            state = FINISH;
                        }
                        
                        draw_gameboard();
                    }
                }*/
                break;
            }
			case FINISH:
				break;
		}
	}
}