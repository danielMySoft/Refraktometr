/*
 * led.c
 *
 *  Created on: 27.04.2021
 *      Author: SP5WWP
 */

#include "main.h"
#include "led.h"
#include "dac.h"
#include "adc.h"
#include "ccd_math.h"

extern volatile uint16_t ccd[NUM_PIX];
extern volatile uint8_t ccd_read_req;
extern volatile uint8_t ccd_data_ready;
extern volatile uint16_t contrast;

void setLedCurrent(float current)
{
	uint16_t val=0;

	if(current>(REF*(R2/(R1+R2)))/RI)	//max_i
		val=4095;
	else
		val=(((current*RI*((R1+R2)/R2)))/REF)*4095.0;

	HAL_DAC_SetValue(&hdac1, DAC_CHANNEL_1, DAC_ALIGN_12B_R, val);
	HAL_DAC_Start(&hdac1, DAC_CHANNEL_1);
}

float getLedCurrent(void)
{
	uint16_t val;

	HAL_ADC_Start(&hadc1);
	val=HAL_ADC_GetValue(&hadc1);

	return (val/65535.0*REF)/RI;
}

uint16_t getContrast(uint16_t *inp)
{
	uint16_t min=65535, max=0;

	for(uint16_t i=150; i<3550; i++)
	{
		if(inp[i]>max)
			max=inp[i];
		if(inp[i]<min)
			min=inp[i];
	}

	return max-min;
}

void autoLed(void)
{
	volatile uint16_t c_max=0;
	float curr_max=0.0;
	uint16_t ccd_copy[NUM_PIX];
	for(uint8_t i=0; i<50; i++) //bylo 0..200
	{
		setLedCurrent((float)i/10000.0);	//40000
		HAL_Delay(300);	//bylo 25

		//memset((uint8_t*)ccd, 0, NUM_PIX*2);

		ccd_read_req=1;
		while(!ccd_data_ready);
		fir16(ccd, ccd_copy, 3);
		memcpy(ccd, ccd_copy, sizeof(ccd));
		fir16(ccd, ccd_copy, 5);
		memcpy(ccd, ccd_copy, sizeof(ccd));
		fir16(ccd, ccd_copy, 7);
		memcpy(ccd, ccd_copy, sizeof(ccd));
		fir16(ccd, ccd_copy, 9);
		contrast=getContrast(ccd_copy);
		if(contrast>c_max)
		{
			c_max=contrast;
			curr_max=(float)i/10000.0;
		}
	}

	setLedCurrent(curr_max*0.8);	//powinno byc *0.8, ale nikt nie wie czemu to nie dziala

	//memset((uint8_t*)ccd, 0, NUM_PIX*2);
	ccd_read_req=1;
	while(!ccd_data_ready);
	contrast=getContrast(ccd);
}
