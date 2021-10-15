/*
 * ccd_math.c
 *
 *  Created on: 06.05.2021
 *      Author: SP5WWP
 */

#include "ccd_math.h"
#include <string.h>

void lowpass(uint32_t *in, const uint16_t siz, const uint8_t len)
{
	uint32_t sum=0;

	for(uint16_t i=siz-1; i>=len-1; i--)
	{
		sum=0;

		for(uint8_t j=0; j<len; j++)
			sum+=in[i-j];

		in[i]=(uint32_t)(sum/len);
	}
}

void std_dev(uint32_t *arr, const uint16_t len) //std_dev dla serii, nadpisuje
{
	uint32_t sum=0;

	for(uint16_t j=NUM_PIX-1; j>len; j--)
	{
		sum=0;

		for(uint16_t i=j-len+1; i<=j; i++)
			sum+=arr[i];

		uint32_t avg=sum/len;
		sum=0;

		for(uint16_t i=j-len+1; i<=j; i++)
		{
			sum+=(arr[i]-avg)*(arr[i]-avg);
		}

		arr[j]=(uint32_t)sqrt(sum);
	}
}

void fir(uint32_t *inp, uint32_t *outp, const uint8_t len)
{
	memset((uint8_t*)outp, 0, 4*NUM_PIX);

	uint32_t tmp=0;
	for(uint8_t i=0; i<(len-1)/2; i++){
		tmp = 0;
		for(uint8_t j=0; j<=i+(len-1)/2; j++)
			tmp += inp[j];
		outp[i] = tmp/(i+1+(len-1)/2);
	}

	for(uint16_t i=(len-1)/2; i<=NUM_PIX-1-(len-1)/2; i++)
	{
		tmp = 0;
		for(int8_t j=-(len-1)/2; j<=(len-1)/2; j++)
			tmp+=inp[i+j];
		outp[i]=tmp/len;
	}

	for(uint16_t i=NUM_PIX-(len-1)/2; i<NUM_PIX; i++){
		tmp = 0;
		for(uint16_t j=i-(len-1)/2; j<NUM_PIX; j++)
			tmp += inp[j];
		outp[i] = tmp/(NUM_PIX-i+(len-1)/2);
	}
}

void fir16(uint16_t *inp, uint16_t *outp, const uint8_t len)
{
	memset((uint8_t*)outp, 0, 2*NUM_PIX);

	for(uint16_t i=len-1; i<REAL_PIX_NUM; i++)
	{
		uint32_t tmp=0;
		for(uint8_t j=0; j<len; j++)
			tmp+=inp[i-j];
		outp[i]=tmp/len;
	}
}

uint16_t findMax(uint32_t* input, uint16_t size){
	uint32_t max_value=0;
	uint16_t number=0;
	for (uint16_t i=0; i<size; i++){
		if(input[i]>max_value){
			max_value = input[i];
			number = i;
		}
	}
	return number;
}

uint16_t findMin(uint32_t* input, uint16_t size){
	uint32_t min_value = ~0;
	uint16_t number=0;
	for (uint16_t i=0; i<size; i++){
		if(input[i]<min_value){
			min_value = input[i];
			number = i;
		}
	}
	return number;
}


void saveCalData(void)
{
	eepromWriteInt(EEPROM_BASE_ADDR  +0, cal_data.serial);
	eepromWriteFloat(EEPROM_BASE_ADDR+2, cal_data.f);
	eepromWriteFloat(EEPROM_BASE_ADDR+6, cal_data.D);
	eepromWriteFloat(EEPROM_BASE_ADDR+10, cal_data.temp_corr);
	eepromWriteFloat(EEPROM_BASE_ADDR+14, cal_data.x_corr);
}

void readCalData(void)
{
	cal_data.serial=eepromReadInt(EEPROM_BASE_ADDR  +0);
	cal_data.f=eepromReadFloat(EEPROM_BASE_ADDR+2);
	cal_data.D=eepromReadFloat(EEPROM_BASE_ADDR+6);
	cal_data.temp_corr=eepromReadFloat(EEPROM_BASE_ADDR+10);
	cal_data.x_corr=eepromReadFloat(EEPROM_BASE_ADDR+14);
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
