/*
 * OLED.c
 *
 *  Created on: 17 мая 2021 г.
 *      Author: Admin
 */
#include "OLED.h"
#include "font.h"

void OLED_Power(uint8_t enable)
{
    if(enable)
        HAL_GPIO_WritePin(OLED_OFF_GPIO_Port, OLED_OFF_Pin, 1);
    else
        HAL_GPIO_WritePin(OLED_OFF_GPIO_Port, OLED_OFF_Pin, 0);
}

void OLED_Reset(void)
{
    HAL_GPIO_WritePin(OLED_RESET_GPIO_Port, OLED_RESET_Pin, 1);
    HAL_Delay(100);
    HAL_GPIO_WritePin(OLED_RESET_GPIO_Port, OLED_RESET_Pin, 0);
    HAL_Delay(100);
    HAL_GPIO_WritePin(OLED_RESET_GPIO_Port, OLED_RESET_Pin, 1);
}

void OLED_WriteData(uint8_t data)
{
	//for(uint8_t a=0; a<200; a++);
    DC_PORT->BSRR|=(1<<DC_PIN);         //D/C high
    CS_PORT->BSRR|=(1<<(CS_PIN+16));    //CS low

    /*for(uint8_t a=0; a<200; a++);
    *(uint8_t *)&SPI1->DR=data;
    while(SPI1->SR & SPI_SR_BSY);
    for(uint8_t a=0; a<200; a++);*/
    HAL_SPI_Transmit(&hspi1, &data, 1, 100);

    CS_PORT->BSRR|=(1<<CS_PIN);         //CS high
}

void OLED_WriteCmd(uint8_t cmd)
{
	//for(uint8_t a=0; a<200; a++);

    DC_PORT->BSRR|=(1<<(DC_PIN+16));    //D/C low
    CS_PORT->BSRR|=(1<<(CS_PIN+16));    //CS low

    /*for(uint8_t a=0; a<200; a++);
    *(uint8_t *)&SPI1->DR=cmd;
    while(SPI1->SR & SPI_SR_BSY);
    for(uint8_t a=0; a<200; a++);*/
    HAL_SPI_Transmit(&hspi1, &cmd, 1, 100);

    CS_PORT->BSRR|=(1<<CS_PIN);         //CS high
}

void OLED_Init(void)
{
    OLED_Reset();
    HAL_Delay(200);
    OLED_Power(1);
    HAL_Delay(200);

    OLED_WriteCmd(OLED_COLUMNS);
    OLED_WriteCmd(0x00);
    OLED_WriteCmd(0x3f);

    OLED_WriteCmd(OLED_ROWS);
    OLED_WriteCmd(0x00);
    OLED_WriteCmd(0x3f);

    OLED_WriteCmd(OLED_CONTRAST);
    OLED_WriteCmd(0x3F);

    OLED_WriteCmd(OLED_CURRENT_FULL);
    //OLED_WriteCmd(OLED_CURRENT_QUARTER);

    OLED_WriteCmd(OLED_RE_MAP);
    OLED_WriteCmd(0x52);

    OLED_WriteCmd(OLED_START_LINE);
    OLED_WriteCmd(0);

    OLED_WriteCmd(OLED_OFFSET);
    OLED_WriteCmd(76);

    OLED_WriteCmd(OLED_MODE_NORMAL);

    OLED_WriteCmd(OLED_MULTIPLEKSER_RATIO);
    OLED_WriteCmd(0x3f);

    OLED_WriteCmd(OLED_PHASE_LENGHT);
    OLED_WriteCmd(0x35);

    OLED_WriteCmd(OLED_ROW_PERIOD);
    OLED_WriteCmd(0x46);

    /*
    OLED_WriteCmd(0xb4);
    OLED_WriteCmd(3);

    OLED_WriteCmd(0xad);
    OLED_WriteCmd(3);
    */

    OLED_WriteCmd(OLED_DISPLAY_CLOCK);
    OLED_WriteCmd(0x01);    // 00

    OLED_WriteCmd(OLED_SEG_LOW_VOLTAGE);    //OLED_SEG_LOW_VOLTAGE
    OLED_WriteCmd(0x0);

    OLED_WriteCmd(OLED_SEG_VCOMH);
    OLED_WriteCmd(11);      // 11

    //OLED_WriteCmd(OLED_PRE_VOLTAGE);
    //OLED_WriteCmd(0x0B);


    OLED_WriteCmd(OLED_GREY_SCALE_TABLE);
    OLED_WriteCmd(0x01);
    OLED_WriteCmd(0x11);
    OLED_WriteCmd(0x22);
    OLED_WriteCmd(0x32);
    OLED_WriteCmd(0x43);
    OLED_WriteCmd(0x54);
    OLED_WriteCmd(0x65);
    OLED_WriteCmd(0x76);


    OLED_WriteCmd(OLED_MASTER_CONF);
    OLED_WriteCmd(0x02);

    OLED_WriteCmd(OLED_COLUMNS);
    OLED_WriteCmd(0x00);
    OLED_WriteCmd(0x3f);
    OLED_WriteCmd(OLED_ROWS);
    OLED_WriteCmd(0x00);
    OLED_WriteCmd(0x3f);
    for(uint16_t i=0; i<4096; i++)
        OLED_WriteData(0x00);

    OLED_WriteCmd(OLED_ON);
}

void OLED_CLEAR(void)
{
int i;
	OLED_WriteCmd(OLED_COLUMNS);
	OLED_WriteCmd(0x00);
	OLED_WriteCmd(0x3f);
	OLED_WriteCmd(OLED_ROWS);
	OLED_WriteCmd(0x00);
	OLED_WriteCmd(0x3f);
		for(i=0;i<4096;i++)
		{
			OLED_WriteData(0x00);
		}
}

void OLED_SetPixel(uint8_t x, uint8_t y, uint8_t color)
{
    OLED_WriteCmd(OLED_COLUMNS);
    OLED_WriteCmd(x);
    OLED_WriteCmd(x);
    OLED_WriteCmd(OLED_ROWS);
    OLED_WriteCmd(y);
    OLED_WriteCmd(y);

    OLED_WriteData(color);
}

void OLED_font_5x7(uint8_t x, uint8_t y, uint8_t color, uint8_t znak)
{
	uint8_t dwa_pixle;

	znak&=0x7f;

	OLED_WriteCmd(OLED_COLUMNS);
	OLED_WriteCmd(x);
	OLED_WriteCmd(x+2);
	OLED_WriteCmd(OLED_ROWS);
	OLED_WriteCmd(y);
	OLED_WriteCmd(y+8);

	for(uint8_t a=0; a<8; a++)
	{
		uint8_t c=gzn1[znak][a]<<2;

		for(uint8_t b=0; b<3;b++)
		{
			dwa_pixle=0;

			if(c & 0x80)
				dwa_pixle=color << 4;
			if(c & 0x40)
				dwa_pixle+=color;

			OLED_WriteData(dwa_pixle);
			c=c<<2;
		}
	}
	OLED_WriteData(0);
	OLED_WriteData(0);
	OLED_WriteData(0);
}

void OLED_font_10x14 (uint8_t x, uint8_t y, uint8_t color, uint8_t znak)
{
unsigned char a,b;
unsigned short int c;
unsigned char dwa_pixle;
OLED_WriteCmd(OLED_COLUMNS);
OLED_WriteCmd(x);
OLED_WriteCmd(x+5);
OLED_WriteCmd(OLED_ROWS);
OLED_WriteCmd(y);
OLED_WriteCmd(y+16);

	for(a=0;a<16;a++){
		c=gzn2[znak][a]<<4;
		for(b=0;b<6;b++){
			dwa_pixle=0;
			if(c & 0x8000){
				dwa_pixle=color << 4;
			}
			if(c & 0x4000){
				dwa_pixle+=color;
			}
			OLED_WriteData(dwa_pixle);
			c=c<<2;
		}
	}
	OLED_WriteData(0);
	OLED_WriteData(0);
	OLED_WriteData(0);
	OLED_WriteData(0);
	OLED_WriteData(0);
	OLED_WriteData(0);
}

void OLED_font_15x24 (uint8_t x, uint8_t y, uint8_t color, uint8_t znak)
{
unsigned char a,b;
unsigned short int c;
unsigned char dwa_pixle;
	switch(znak){
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			znak=znak-'0';
		break;
		case '-':
			znak=0xb;
		break;
		case 0xa:
			znak=0xa;
		break;
		case '%':
			znak=0xc;
		break;
		case '.':
			znak=0xf;
		break;
		default:
			znak=0xe;
		break;
	}
	OLED_WriteCmd(OLED_COLUMNS);
	OLED_WriteCmd(x);
	OLED_WriteCmd(x+7);
	OLED_WriteCmd(OLED_ROWS);
	OLED_WriteCmd(y);
	OLED_WriteCmd(y+24);

	for(a=0;a<24;a++){
		c=gzn3[znak][a]<<1;
		for(b=0;b<8;b++){
			dwa_pixle=0;
			if(c & 0x8000){
				dwa_pixle=color << 4;
			}
			if(c & 0x4000){
				dwa_pixle+=color;
			}
			OLED_WriteData(dwa_pixle);
			c=c<<2;
		}
	}
	OLED_WriteData(0);
	OLED_WriteData(0);
	OLED_WriteData(0);
	OLED_WriteData(0);

	OLED_WriteData(0);
	OLED_WriteData(0);
	OLED_WriteData(0);
	OLED_WriteData(0);
}

void OLED_font_20x32 (uint8_t x, uint8_t y, uint8_t color, uint8_t znak)
// -----------------------------------------------
{
unsigned char a,b;
unsigned int c;
unsigned char dwa_pixle;

	switch(znak){
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			znak=znak-'0';
		break;
		case '-':
			znak=0xb;
		break;
		case 0xa:
			znak=0xa;
		break;
		case '%':
			znak=0xc;
		break;
		case '.':
			znak=0xf;
		break;
		default:
			znak=0xe;
		break;
	}
	OLED_WriteCmd(OLED_COLUMNS);
	OLED_WriteCmd(x);
	OLED_WriteCmd(x+12);
	OLED_WriteCmd(OLED_ROWS);
	OLED_WriteCmd(y);
	OLED_WriteCmd(y+32);

	for(a=0;a<32;a++){
		c=gzn4[znak][a];
		for(b=0;b<13;b++){
			dwa_pixle=0;
			if(c & 0x80000l){
				dwa_pixle=color << 4;
			}
			if(c & 0x40000l){
				dwa_pixle+=color;
			}
			OLED_WriteData(dwa_pixle);
			c=c<<2;
		}
	}
}

// 0 - czcionka 5x7
// 1 - czcionka 10x14
// 2 - czcionka 15x24
// 3 - czcionka 20x32

void OLED_PutStr(uint8_t x, uint8_t y, const char *str, uint8_t font, uint8_t color)
{
	switch(font)
	{
		case 0:
			for(uint8_t i=0; i<strlen(str); i++)
				OLED_font_5x7(x+3*i , y, color, str[i]);

		break;

		case 1:
			for(uint8_t i=0; i<strlen(str); i++)
				OLED_font_10x14(x+6*i , y, color, str[i]);
		break;

		case 2:
			for(uint8_t i=0; i<strlen(str); i++)
				OLED_font_15x24(x+8*i , y, color, str[i]);
		break;

		case 3:
			for(uint8_t i=0; i<strlen(str); i++)
				OLED_font_20x32(x+11*i , y, color, str[i]);
		break;
	}
}

void OLED_bitmap (unsigned char x, unsigned char y, const unsigned char s[])
{
int	a,lb;
	lb=(unsigned short int)s[0]*(unsigned short int)s[1]/2;
	OLED_WriteCmd(OLED_COLUMNS);
	OLED_WriteCmd(x);
	OLED_WriteCmd(x+s[0]/2-1);
	OLED_WriteCmd(OLED_ROWS);
	OLED_WriteCmd(y);
	OLED_WriteCmd(y+s[1]);
	for(a=2;a<lb+2;a++){
		OLED_WriteData(s[a] * settings.contrast);		//dodany kontrast żeby kreske nad O w BRAK PROBKI wyswietlac
	}													//zeby normalnie wyswietlic obrazek to kontrast trzeba wywalic albo tuz przed wyswietleniem obrazka zmienic na 0x01
}

//=============================	OLED END ======================================= //
