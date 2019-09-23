#ifdef __USE_CMSIS
#include "LPC17xx.h"
#endif

#include "oled.h"
#include "rgb.h"
#include "lpc17xx_gpio.h"
#include "lpc17xx_ssp.h"
#include "lpc17xx_pinsel.h"
#include "lpc17xx_timer.h"
#include "lpc17xx_i2c.h"

#define NOTE_PIN_HIGH() GPIO_SetValue(0, 1<<26);
#define NOTE_PIN_LOW()  GPIO_ClearValue(0, 1<<26);

uint8_t buffer[10];

void delay(int n) {
	Timer0_Wait(n);
}

static void playNote(uint32_t note, uint32_t durationMs) {

    uint32_t t = 0;
    if (note > 0) {
        while (t < (durationMs*1000)) {
            NOTE_PIN_HIGH();
            Timer0_us_Wait(note / 2);
            NOTE_PIN_LOW();
            Timer0_us_Wait(note / 2);
            t += note;
        }
    }
    else {
    	Timer0_Wait(durationMs);
    }
}

void callBuzzer() {

	int i = 0;
	GPIO_SetDir(2, 1<<0, 1);
	GPIO_SetDir(2, 1<<1, 1);
	GPIO_SetDir(0, 1<<27, 1);
	GPIO_SetDir(0, 1<<28, 1);
	GPIO_SetDir(2, 1<<13, 1);
	GPIO_SetDir(0, 1<<26, 1);
	GPIO_ClearValue(0, 1<<27);
	GPIO_ClearValue(0, 1<<28);
	GPIO_ClearValue(2, 1<<13);
	playNote(3030, 1000);
}

static void init_i2c(void)
{
	PINSEL_CFG_Type PinCfg;

	PinCfg.Funcnum = 2;
	PinCfg.Pinnum = 10;
	PinCfg.Portnum = 0;
	PINSEL_ConfigPin(&PinCfg);
	PinCfg.Pinnum = 11;
	PINSEL_ConfigPin(&PinCfg);

	I2C_Init(LPC_I2C2, 100000);
	I2C_Cmd(LPC_I2C2, ENABLE);
}

static void init_ssp(void)
{
	SSP_CFG_Type SSP_ConfigStruct;
	PINSEL_CFG_Type PinCfg;

	PinCfg.Funcnum = 2;
	PinCfg.OpenDrain = 0;
	PinCfg.Pinmode = 0;
	PinCfg.Portnum = 0;
	PinCfg.Pinnum = 7;
	PINSEL_ConfigPin(&PinCfg);
	PinCfg.Pinnum = 8;
	PINSEL_ConfigPin(&PinCfg);
	PinCfg.Pinnum = 9;
	PINSEL_ConfigPin(&PinCfg);
	PinCfg.Funcnum = 0;
	PinCfg.Portnum = 2;
	PinCfg.Pinnum = 2;
	PINSEL_ConfigPin(&PinCfg);

	SSP_ConfigStructInit(&SSP_ConfigStruct);
	SSP_Init(LPC_SSP1, &SSP_ConfigStruct);
	SSP_Cmd(LPC_SSP1, ENABLE);

}

void intToString(int value, uint8_t* pBuf, uint32_t len, uint32_t base)
{
	static const char* pAscii = "0123456789abcdefghijklmnopqrstuvwxyz";
	int pos = 0;
	int tmpValue = value;

	if (pBuf == NULL || len < 2)
	{
		return;
	}

	if (base < 2 || base > 36)
	{
		return;
	}

	if (value < 0)
	{
		tmpValue = -tmpValue;
		value    = -value;
		pBuf[pos++] = '-';
	}

	do {
		pos++;
		tmpValue /= base;
	} while(tmpValue > 0);


	if (pos > len)
	{
		return;
	}

	pBuf[pos] = '\0';

	do {
		pBuf[--pos] = pAscii[value % base];
		value /= base;
	} while(value > 0);

	return;
}

void startDisplayOled() {

	uint8_t* texto = "TEMPO: ";
	uint8_t X = 0;
    uint8_t Y = 0;

	oled_init();

	oled_clearScreen(OLED_COLOR_WHITE);
	X = 1;
	Y = 10;
	oled_putString(X, Y, texto, OLED_COLOR_BLACK, OLED_COLOR_WHITE);

}

void writeDisplay(uint32_t cont) {

	uint8_t X = 0;
	uint8_t Y = 0;
	startDisplayOled();
	intToString(cont, buffer, 10, 10);

	X = 1;
	Y = 30;
	oled_putString(X, Y, buffer, OLED_COLOR_BLACK, OLED_COLOR_WHITE);
	X = 1;
	Y = 50;
	if(cont <= 50) {
		oled_putString(X, Y,"ACENDE AZUL", OLED_COLOR_BLACK, OLED_COLOR_WHITE);
	}
	else if(cont > 50 && cont <= 100) {
		oled_putString(X, Y,"ACENDE VERDE", OLED_COLOR_BLACK, OLED_COLOR_WHITE);
	}
	else if(cont > 100) {
		oled_putString(X, Y,"ACENDE VERMELHO", OLED_COLOR_BLACK, OLED_COLOR_WHITE);
	}
}


int main(void) {
	rgb_init();
	init_ssp();
	init_i2c();
	oled_init();

	uint32_t countLedTime = 0;
	uint8_t countBuzz = 0;



	while(1) {
		if(countLedTime <= 50) {
			rgb_setLeds(RGB_BLUE);
		}
		else if(countLedTime > 50 && countLedTime <= 100) {
			rgb_setLeds(RGB_GREEN);
		}
		else {
			rgb_setLeds(RGB_RED);
			if(countBuzz == 0) {
				callBuzzer();
				countBuzz = 1;
			}
		}

		writeDisplay(countLedTime);
		delay(500);

		countLedTime++;
		if(countLedTime > 150) {
			countLedTime = 0;
			countBuzz = 0;
		}
	}

}
