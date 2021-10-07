/*
 * OLED.h
 *
 *  Created on: 17 мая 2021 г.
 *      Author: Admin
 */

#ifndef OLED_H_
#define OLED_H_

#include <string.h>
#include "main.h"
#include "graph.h"


#define     DC_PORT     GPIOA
#define     DC_PIN      4

#define     CS_PORT     GPIOA
#define     CS_PIN      6


void OLED_Power(uint8_t enable);
void OLED_Reset(void);
void OLED_WriteData(uint8_t data);
void OLED_WriteCmd(uint8_t cmd);
void OLED_Init(void);
void OLED_CLEAR(void);
void OLED_SetPixel(uint8_t x, uint8_t y, uint8_t color);
void OLED_font_5x7(uint8_t x, uint8_t y, uint8_t color, uint8_t znak);
void OLED_font_10x14 (uint8_t x, uint8_t y, uint8_t color, uint8_t znak);
void OLED_font_15x24 (uint8_t x, uint8_t y, uint8_t color, uint8_t znak);
void OLED_font_20x32 (uint8_t x, uint8_t y, uint8_t color, uint8_t znak);
void OLED_PutStr(uint8_t x, uint8_t y, const char *str, uint8_t font, uint8_t color);
void OLED_bitmap (unsigned char x, unsigned char y, const unsigned char s[]);

extern SPI_HandleTypeDef hspi1;



#endif /* OLED_H_ */
