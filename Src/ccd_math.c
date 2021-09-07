/*
 * ccd_math.c
 *
 *  Created on: 06.05.2021
 *      Author: SP5WWP
 */

#include "ccd_math.h"

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

	for(uint16_t i=len-1; i<REAL_PIX_NUM; i++)
	{
		uint32_t tmp=0;
		for(uint8_t j=0; j<len; j++)
			tmp+=inp[i-j];
		outp[i]=tmp/len;
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
	uint16_t number;
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
	uint16_t number;
	for (uint16_t i=0; i<size; i++){
		if(input[i]<min_value){
			min_value = input[i];
			number = i;
		}
	}
	return number;
}
