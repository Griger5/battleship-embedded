#include "LPC17xx.h"
#include "Driver_SPI.h"
#include "Driver_USART.h"
#include "lcd_lib/Open1768_LCD.h"
#include "lcd_lib/LCD_ILI9325.h"
#include "tp_lib/TP_Open1768.h"

#include "display.h"
#include "calibration.h"

void myUART_Thread(void const *argument);
extern ARM_DRIVER_USART Driver_USART1;

static ARM_DRIVER_USART * USARTdrv = &Driver_USART1;
char cmd[5];

void myUART_Thread(const void* args){
	USARTdrv->Send("\nHello my dear friend!", 22);
	//while (1) {
	//}
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

void TouchToLCD(int x, int y, int *x_lcd, int *y_lcd) {
	*x_lcd = x * 320 / 5596;
	*y_lcd = y * 240 / 3000;
}

bool insideTouchArea(int x, int y) {
	if (x > 20 && x < 2800 && y > 20 && y < 3500) {
		return true;
	}  
	return false;
}

volatile uint32_t msTicks = 0;

void SysTick_Handler(void) {
	//USARTdrv->Send("systick", 7);
	msTicks++;
}

void wait(uint32_t t) {
	uint32_t start = msTicks;

	while (msTicks < start + t);
}

void EINT3_IRQHandler(){
	//	if((LPC_GPIOINT->IO0IntEnF & (1 << 19) ) == (1 << 19))
	static int counter = 0;
	//USARTdrv->Send("handler", 7);
	switch (state) {
		//USARTdrv->Send("switch", 6);
		case CAL: {
				//int x;
				//int y;
				//touchpanelGetXY(&y, &x);
				//USARTdrv->Send("case", 4);
				/*if (insideTouchArea(x,y)) {
					USARTdrv->Send("inside", 6);
					touch[counter].x = x;
					touch[counter].y = y;
					counter++;
				}
				if (counter == 3) {
					USARTdrv->Send("counter", 7);				
					state = START;
					Point lcd_temp[3] = {lcd[0], lcd[1], lcd[2]};
					Point touch_temp[3] = {touch[0], touch[1], touch[2]};
					CalibrationMatrix calib_temp;
					
					calib_calculate(lcd_temp, touch_temp, &calib_temp);
					calib.a = calib_temp.a;
					calib.b = calib_temp.b;
					calib.c = calib_temp.c;
					calib.d = calib_temp.d;
					calib.e = calib_temp.e;
					calib.f = calib_temp.f;
				}*/
				//wait(1000);
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

int main() {
	int return_code = SysTick_Config(SystemCoreClock/1000);
	lcdConfiguration();
	init_ILI9325();
	touchpanelInit();
	// 37672 - ILI
	
	LPC_GPIO0->FIODIR &= ~(1 << 19 );
	LPC_GPIOINT->IO0IntEnF |= ( 1 << 19 );
	LPC_SC->EXTINT |= ( 1 << 3 );
	LPC_SC->EXTMODE |= ( 1 << 3 );
	LPC_SC->EXTPOLAR |= ( 0 << 3 );
	NVIC_EnableIRQ(EINT3_IRQn);

	USARTdrv->Initialize(NULL);
	USARTdrv->PowerControl(ARM_POWER_FULL); /*Configure the USART to 4800 Bits/sec */
	USARTdrv->Control(ARM_USART_MODE_ASYNCHRONOUS | ARM_USART_DATA_BITS_8 | ARM_USART_PARITY_NONE | ARM_USART_STOP_BITS_1 | ARM_USART_FLOW_CONTROL_NONE, 4800); /* Enable Receiver and Transmitter lines */
	USARTdrv->Control (ARM_USART_CONTROL_TX, 1);
	USARTdrv->Control (ARM_USART_CONTROL_RX, 1);
	myUART_Thread(NULL);
	
	fill(LCDWhite);
	
	CalibrationMatrix calib;
	Point lcd[3] = {{70, 30}, {120,180}, {250, 80}};
	Point touch[3] = {{}, {}, {}};
	
	/*draw_gameboard();
	// start board x = 30
	// start board y = 70
	// cell width = 15
	fill_rectangle(30, 70, LCDRed);
	fill_rectangle(30 + 4 * 15, 70 + 7 * 15, LCDGreen);
	
	draw_ship(30 + 2*15, 70 + 3 * 15, 4, LEFT, LCDCyan);
	*/
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
	
	while (1){
		switch (state) {
		//USARTdrv->Send("switch", 6);
		case CAL: {
				int x;
				int y;
				int correct_x[5];
				int correct_y[5];
				int correct_points = 0;
				while (correct_points < 5) {
					USARTdrv->Send("AAAAA", 5);
					touchpanelGetXY(&y, &x);
					//if (insideTouchArea(x,y)) {
						correct_x[correct_points] = x;
						correct_y[correct_points] = y;
						correct_points++;
					char str[1];
					sprintf(str, "%d", correct_points);
					USARTdrv->Send(str, 1);
					//}
				}
				
				avg_touch_xy(correct_x, correct_y, &x, &y);
				//USARTdrv->Send("case", 4);
				if (insideTouchArea(x,y)) {
					//USARTdrv->Send("inside", 6);
					touch[counter].x = x;
					touch[counter].y = y;
					counter++;
					USARTdrv->Send("counter", 7);
				}
				if (counter == 3) {
					USARTdrv->Send("counter done\n", 13);				
					state = START;
					Point lcd_temp[3] = {lcd[0], lcd[1], lcd[2]};
					Point touch_temp[3] = {touch[0], touch[1], touch[2]};
					CalibrationMatrix calib_temp;
					
					calib_calculate(lcd_temp, touch_temp, &calib_temp);
					calib.a = calib_temp.a;
					calib.b = calib_temp.b;
					calib.c = calib_temp.c;
					calib.d = calib_temp.d;
					calib.e = calib_temp.e;
					calib.f = calib_temp.f;
				}
				if (insideTouchArea(x,y)) {
					char str[3] = {0};
					sprintf(str, "%d", x);
					USARTdrv->Send(str, 3);
					USARTdrv->Send("   ", 3);
					str[0] = '\0';
					str[1] = '\0';
					str[2] = '\0';
					sprintf(str, "%d", y);
					USARTdrv->Send(str, 3);
					USARTdrv->Send("\n", 1);
					wait(1000);
				}
				//USARTdrv->Send("ABC", 3);
				touch_pending = 0;
				LPC_GPIOINT->IO0IntEnF |= (1 << 19);
				break;
			}
			case START: {
				/*fill(LCDWhite);
				int x;
				int y;
				int lcd_x;
				int lcd_y;
				touchpanelGetXY(&y, &x);
				if (insideTouchArea(x,y)) {
					calib_transform(&calib, y, x, &lcd_x, &lcd_y);
					fill_rectangle(lcd_x, lcd_y, LCDRed);
				}*/
				//USARTdrv->Send("start", 5);
				/*char str[10] = {0};
				sprintf(str, "%d", touch[0].y);
				USARTdrv->Send(str, 4);
				USARTdrv->Send(" ", 1);*/
				//sprintf(str, "%d", touch[0].y);
				//USARTdrv->Send("\n", 1);
				//USARTdrv->Send(str, 4);
				/*sprintf(str, "%d", touch[1].x);
				USARTdrv->Send(str, 4);
				USARTdrv->Send(" ", 1);
				sprintf(str, "%d", touch[1].y);
				USARTdrv->Send(str, 4);
				USARTdrv->Send("\n", 1);*/
				USARTdrv->Send("counter done", 12);
				char str[8] = {0};
/*				sprintf(str, "%f", calib.a);
				USARTdrv->Send(str, 8);
				USARTdrv->Send("   ", 3);
				sprintf(str, "%f", calib.b);
				USARTdrv->Send(str, 8);
				USARTdrv->Send("   ", 3);*/
				/*sprintf(str, "%f", calib.c);
				USARTdrv->Send(str, 8);
				USARTdrv->Send("   ", 3);
				sprintf(str, "%f", calib.d);
				USARTdrv->Send(str, 8);
				USARTdrv->Send("   ", 3);
				sprintf(str, "%f", calib.e);
				USARTdrv->Send(str, 8);
				USARTdrv->Send("   ", 3);
				sprintf(str, "%f", calib.f);
				USARTdrv->Send(str, 8);*/
				WriteChar(160 - 4, 120 - 6, 'X', LCDBlue, 1);
				state = SET;
				break;
			}
			case SET:
				draw_gameboard();
				int x;
				int y;
				int lcd_x;
				int lcd_y;
				touchpanelGetXY(&y, &x);
				if (insideTouchArea(x,y)) {
					calib_transform(&calib, y, x, &lcd_x, &lcd_y);
					//fill_rectangle(lcd_x, lcd_y, LCDRed);
					char str[3] = {0};
					sprintf(str, "%d", lcd_x);
					USARTdrv->Send(str, 4);
					USARTdrv->Send("   ", 3);
					sprintf(str, "%d", lcd_y);
					USARTdrv->Send(str, 4);
				}
				break;
			case MY_TURN:
				break;
			case OPPONENT_TURN:
				break;
			case FINISH:
				break;
		}
		//wait(2000);
		//USARTdrv->Send("main", 4);
	}
}
