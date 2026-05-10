#include "TP_Open1768.h"

static void touchpanelDelayUS(uint32_t cnt) {
	volatile uint32_t i;
	for (i = 0; i < cnt; i++) {
		volatile uint8_t us = 12; /*  */
		while (us--) /*  */
		{
			;
		}
	}
}

static uint8_t touchpanelWriteCmd(uint8_t cmd) {
	uint8_t byte_r;

	while (LPC_SSP1->SR & (1 << SSPSR_BSY))
		; /* Wait for transfer to finish */
	LPC_SSP1->DR = cmd;
	while (LPC_SSP1->SR & (1 << SSPSR_BSY))
		; /* Wait for transfer to finish */
	while (!(LPC_SSP1->SR & (1 << SSPSR_RNE)))
		; /* Wait until the Rx FIFO is not empty */
	byte_r = LPC_SSP1->DR;

	return byte_r; /* Return received value */
}

static int touchpanelReadAddress(void) {
	unsigned short buf, temp;

	temp = touchpanelWriteCmd(0x00);
	buf = temp << 8;
	touchpanelDelayUS(1);
	temp = touchpanelWriteCmd(0x00);
	;
	buf |= temp;
	buf >>= 3;
	buf &= 0xfff;
	return buf;
}

int touchpanelReadX(void) {
	int i;
	TP_CS(0)
		;
	touchpanelDelayUS(1);
	touchpanelWriteCmd(CHX);
	touchpanelDelayUS(1);
	i = touchpanelReadAddress();
	TP_CS(1)
		;
	return i;
}

int touchpanelReadY(void) {
	int i;
	TP_CS(0)
		;
	touchpanelDelayUS(1);
	touchpanelWriteCmd(CHY);
	touchpanelDelayUS(1);
	i = touchpanelReadAddress();
	TP_CS(1)
		;
	return i;
}

void touchpanelGetXY(int *x, int *y) {
	int adx, ady;
	adx = touchpanelReadX();
	touchpanelDelayUS(1);
	ady = touchpanelReadY();
	*x = adx;
	*y = ady;
}

__INLINE void LPC17xx_SPI_SetSpeed(uint8_t speed) {
	speed &= 0xFE;
	if (speed < 2) {
		speed = 2;
	}
	LPC_SSP1->CPSR = speed;	//APB clock divider 2-254
}

static void touchpanelDriverDataBusInit(void) {
	volatile uint32_t dummy;

	/* Initialize and enable the SSP1 Interface module. */
	LPC_SC->PCONP |= (1	<< 10); /* Enable power to SSPI1 block  */

	/* P0.7 SCK, P0.8 MISO, P0.9 MOSI are SSP pins. */
	LPC_PINCON->PINSEL0	&= ~((3UL << 14) | (3UL << 16) | (3UL << 18)); /* P0.7,P0.8,P0.9 cleared */
	LPC_PINCON->PINSEL0 |= (2UL << 14) | (2UL << 16) | (2UL << 18); /* P0.7 SCK1,P0.8 MISO1,P0.9 MOSI1 */

	/* PCLK_SSP1=CCLK */
	LPC_SC->PCLKSEL0 &= ~(3 << 20); /* PCLKSP0 = CCLK/4 (18MHz) */
	LPC_SC->PCLKSEL0 |= (1 << 20); /* PCLKSP0 = CCLK   (72MHz) */

	LPC_SSP1->CR0 = 0x0007; /* 8Bit, CPOL=0, CPHA=0         */
	LPC_SSP1->CR1 = 0x0002; /* SSP1 enable, master          */

	LPC17xx_SPI_SetSpeed(SPI_SPEED_500kHz);

	/* wait for busy gone */
	while (LPC_SSP1->SR & (1 << SSPSR_BSY))
		;

	/* drain SPI RX FIFO */
	while (LPC_SSP1->SR & (1 << SSPSR_RNE)) {
		dummy = LPC_SSP1->DR;
	}
}

void touchpanelInit(void) {
	LPC_GPIO0->FIODIR |= PIN_TP_CS; /* P0.6 CS is output */
	LPC_GPIO0->FIODIR &= ~PIN_TP_INT; /* P0.19 TP_INT is input */
	TP_CS(1);
	touchpanelDriverDataBusInit();
}

