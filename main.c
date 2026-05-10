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

volatile Point lcd[3] = {{70, 30}, {70,210}, {250, 30}};
volatile Point touch[3] = {{}, {}, {}};
volatile CalibrationMatrix calib;

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
	if (x > 20 && x < 1900 && y > 20 && y < 3500) {
		return true;
	}  
	return false;
}


volatile uint32_t msTicks = 0;

void SysTick_Handler(void) {
	USARTdrv->Send("systick", 7);
	msTicks++;
}

void wait(uint32_t t) {
	uint32_t start = msTicks;

	while (msTicks < start + t);
}

void EINT3_IRQHandler(){
	//	if((LPC_GPIOINT->IO0IntEnF & (1 << 19) ) == (1 << 19))
	static int counter = 0;
	USARTdrv->Send("handler", 7);
	switch (state) {
		USARTdrv->Send("switch", 6);
		case CAL: {
				int x;
				int y;
				touchpanelGetXY(&y, &x);
				USARTdrv->Send("case", 4);
				if (insideTouchArea(x,y)) {
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
				}
				wait(1);
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
	
	WriteChar(lcd[0].x - 4, lcd[0].y - 6, 'X', LCDRed, 1);
	WriteChar(lcd[1].x - 4, lcd[1].y - 6, 'X', LCDRed, 1);
	WriteChar(lcd[2].x - 4, lcd[2].y - 6, 'X', LCDRed, 1);
	
	while (1){
		switch (state) {
		USARTdrv->Send("switch", 6);
		case CAL: {
				int x;
				int y;
				touchpanelGetXY(&y, &x);
				USARTdrv->Send("case", 4);
				if (insideTouchArea(x,y)) {
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
				}
				wait(1);
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
		USARTdrv->Send("main", 4);

	}
}
