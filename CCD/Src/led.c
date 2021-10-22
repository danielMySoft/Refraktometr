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
#include <string.h>
#include "tim.h"
#include "global.h"

void getCCD(void)
{
	for(uint16_t i=0; i<NUM_PIX; i++)
		ccd[i] = 0;
	ccd_data_ready = 0;
	ccd_read_req=1;
	while(ccd[NUM_PIX-1] == 0){
		HAL_Delay(1);
	};
	ccd_data_ready = 1;
	ccd_read_req=0;
	HAL_TIM_Base_Stop(&htim1);
}

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

void autoLed(void)
{
	volatile uint16_t c_max=0;
	float curr_max=0.0;
	for(uint8_t i=20; i<100; i++) //bylo 0..200
	{
		setLedCurrent((float)i/20000.0);	//40000
		HAL_Delay(10);	//bylo 25
		getCCD();

		fir16(ccd, ccd_fir16, 3);
		memcpy(ccd, ccd_fir16, sizeof(ccd_fir16));
		fir16(ccd, ccd_fir16, 5);
		memcpy(ccd, ccd_fir16, sizeof(ccd_fir16));
		fir16(ccd, ccd_fir16, 7);
		memcpy(ccd, ccd_fir16, sizeof(ccd_fir16));
		fir16(ccd, ccd_fir16, 9);
		memcpy(ccd, ccd_fir16, sizeof(ccd_fir16));
		contrast=getContrast(ccd_fir16);
		if(i == 30){
			volatile uint8_t aaa = 0;
			aaa = 5;
			pix_num = aaa;
		}
		if(contrast>c_max)
		{
			c_max=contrast;
			curr_max=(float)i/20000.0;
			//if(contrast>21850)
			if(contrast>19560)
				break;
		}
	}

	setLedCurrent(curr_max);	//powinno byc *0.8, ale nikt nie wie czemu to nie dziala

	//memset((uint8_t*)ccd, 0, NUM_PIX*2);
	ccd_read_req=1;
	while(!ccd_data_ready);
	contrast=getContrast(ccd);
}
