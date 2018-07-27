#ifndef __WS2811_H
#define	__WS2811_H

#include "stm32f10x.h"
#include "delay.h"	


struct Pixel {
    uint8_t red;
    uint8_t green;
    uint8_t blue;
};

void LED_SPI_LowLevel_Init(void);
void LED_SPI_WriteByte(uint16_t Data);
void LED_SPI_SendBits(uint8_t bits);
void LED_SPI_SendPixel(uint32_t color);
void LED_SPI_Update(unsigned long buffer[], uint32_t length);
#endif /* __LED_H */
