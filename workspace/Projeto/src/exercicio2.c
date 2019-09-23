#ifdef __USE_CMSIS
#include "LPC17xx.h"
#endif

#include <cr_section_macros.h>

#include "lpc17xx_pinsel.h"
#include "lpc17xx_gpio.h"
#include "lpc17xx_i2c.h"
#include "lpc17xx_ssp.h"
#include "lpc17xx_adc.h"
#include "lpc17xx_timer.h"

#include "led7seg.h"
#include "oled.h"
#include "rgb.h"
#include "light.h"
#include "temp.h"

#define NOTE_PIN_HIGH() GPIO_SetValue(0, 1 << 26);
#define NOTE_PIN_LOW() GPIO_ClearValue(0, 1 << 26);

uint8_t buf[10];
uint32_t msTicks = 0;

uint32_t luminosityValue = 0;
int32_t temp = 0;

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

static void init_adc(void)
{
    PINSEL_CFG_Type PinCfg;

    PinCfg.Funcnum = 1;
    PinCfg.OpenDrain = 0;
    PinCfg.Pinmode = 0;
    PinCfg.Pinnum = 23;
    PinCfg.Portnum = 0;
    PINSEL_ConfigPin(&PinCfg);
    ADC_Init(LPC_ADC, 1000000);
    ADC_IntConfig(LPC_ADC, ADC_CHANNEL_0, DISABLE);
    ADC_ChannelCmd(LPC_ADC, ADC_CHANNEL_0, ENABLE);
}

void SysTick_Handler(void)
{
    msTicks++;
}

static uint32_t getTicks(void)
{
    return msTicks;
}

static void playNote(uint32_t note, uint32_t durationMs)
{

    uint32_t t = 0;
    if (note > 0)
    {
        while (t < (durationMs * 1000))
        {
            NOTE_PIN_HIGH();
            Timer0_us_Wait(note / 2);
            NOTE_PIN_LOW();
            Timer0_us_Wait(note / 2);
            t += note;
        }
    }
    else
    {
        Timer0_Wait(durationMs);
    }
}

void intToString(int value, uint8_t *pBuf, uint32_t len, uint32_t base)
{
    static const char *pAscii = "0123456789abcdefghijklmnopqrstuvwxyz";
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
        value = -value;
        pBuf[pos++] = '-';
    }
    do
    {
        pos++;
        tmpValue /= base;
    } while (tmpValue > 0);

    if (pos > len)
    {
        return;
    }

    pBuf[pos] = '\0';

    do
    {
        pBuf[--pos] = pAscii[value % base];
        value /= base;
    } while (value > 0);

    return;
}

void tocarBuzzer()
{

    int i = 0;

    GPIO_SetDir(2, 1 << 0, 1);
    GPIO_SetDir(2, 1 << 1, 1);

    GPIO_SetDir(0, 1 << 27, 1);
    GPIO_SetDir(0, 1 << 28, 1);
    GPIO_SetDir(2, 1 << 13, 1);
    GPIO_SetDir(0, 1 << 26, 1);

    GPIO_ClearValue(0, 1 << 27); //LM4811-clk
    GPIO_ClearValue(0, 1 << 28); //LM4811-up/dn
    GPIO_ClearValue(2, 1 << 13); //LM4811-shutdn

    playNote(3030, 1000);
}

void displayCount()
{

    uint8_t caracter = '0', i = 0;

    for (i = 0; i < 10; i++)
    {

        led7seg_setChar(caracter, FALSE);

        Timer0_Wait(1000);

        caracter++;

        if (caracter > '9')
        {
            caracter = '0';
        }
    }

    tocarBuzzer();
}

void startDisplayOled()
{

    oled_init();

    oled_putString(1, 1, (uint8_t *)"Temp   : ", OLED_COLOR_BLACK, OLED_COLOR_WHITE);
    oled_putString(1, 9, (uint8_t *)"Light  : ", OLED_COLOR_BLACK, OLED_COLOR_WHITE);
    oled_putString(1, 17, (uint8_t *)"Umidade: ", OLED_COLOR_BLACK, OLED_COLOR_WHITE);
}

uint32_t getHumidity()
{
    uint32_t humidity = 0;
    ADC_StartCmd(LPC_ADC, ADC_START_NOW);
    //Wait conversion complete
    while (!(ADC_ChannelGetStatus(LPC_ADC, ADC_CHANNEL_0, ADC_DATA_DONE)))
        ;
    humidity = ADC_ChannelGetData(LPC_ADC, ADC_CHANNEL_0);
    return humidity;
}

void writeOnDisplay(uint32_t temperature, uint32_t luminosity, uint32_t humidity)
{

    intToString(temperature, buf, 10, 10);
    oled_fillRect((1 + 9 * 6), 1, 80, 8, OLED_COLOR_WHITE);
    oled_putString((1 + 9 * 6), 1, buf, OLED_COLOR_BLACK, OLED_COLOR_WHITE);

    intToString(luminosity, buf, 10, 10);
    oled_fillRect((1 + 9 * 6), 9, 80, 16, OLED_COLOR_WHITE);
    oled_putString((1 + 9 * 6), 9, buf, OLED_COLOR_BLACK, OLED_COLOR_WHITE);

    intToString(humidity, buf, 10, 10);
    oled_fillRect((1 + 9 * 6), 17, 80, 24, OLED_COLOR_WHITE);
    oled_putString((1 + 9 * 6), 17, buf, OLED_COLOR_BLACK, OLED_COLOR_WHITE);
}

void getTrimpot()
{

    unsigned int trim = 0;
    rgb_init();
    ADC_StartCmd(LPC_ADC, ADC_START_NOW);
    while (!(ADC_ChannelGetStatus(LPC_ADC, ADC_CHANNEL_0, ADC_DATA_DONE)))
        ;
    trim = ADC_ChannelGetData(LPC_ADC, ADC_CHANNEL_0);

    if (trim < 2700)
    {
        rgb_setLeds(RGB_RED);
    }
    else if (trim > 2960)
    {
        rgb_setLeds(RGB_BLUE);
        GPIO_SetValue(2, (1 << 1));
    }
    else
    {
        rgb_setLeds(RGB_GREEN);
    }
}

int main(void)
{

    //Initializing
    init_i2c();
    init_ssp();
    init_adc();
    temp_init(&getTicks);
    SysTick_Config(SystemCoreClock / 1000);
    led7seg_init();
    oled_init();
    rgb_init();
    light_init();
    light_enable();
    startDisplayOled();

    while (1)
    {
        getTrimpot();
        temp = temp_read();
        luminosityValue = light_read();
        uint32_t humidity = getHumidity();
        writeOnDisplay(luminosityValue, (uint32_t)temp / 10, humidity);
        if (luminosityValue < 15)
            displayCount();
    }
    return 0;
}
