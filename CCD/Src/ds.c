/*
 * ds.c
 *
 *  Created on: 30.04.2021
 *      Author: SP5WWP
 */

#include "ds.h"
#include "ds_crc.h"
#include "usart.h"

uint8_t DS_CheckCksum(uint8_t *inp, uint8_t numBytes)
{
	uint8_t local[9];
	uint8_t crc=0;

	for(uint8_t byte=0; byte<numBytes+1; byte++)
	{
		local[byte]=0;

		for(uint8_t bit=0; bit<8; bit++)
		{
			if(inp[byte*8+bit]==0xFF)
				local[byte]|=(1<<bit);
		}
	}

	for(uint8_t i=0; i<numBytes-1; i++)
		crc = CodeCRC[crc^local[i]];

	if(crc==local[numBytes-1])
		return 0;
	else
		return 1;
}

void DS_Reset(void)
{
	uint8_t val=0xF0;
	huart4.Init.BaudRate=9600;
	HAL_UART_Init(&huart4);
	//HAL_UART_Receive_DMA(&huart4, &uart_rcv_val, 1);
	HAL_UART_Transmit(&huart4, &val, 1, 1);
}

void DS_Write(uint8_t val)
{
	uint8_t ds_snd[8];

	for(uint8_t i=0; i<8; i++)
		if((val>>i)&1)
			ds_snd[i]=0xFF;
		else
			ds_snd[i]=0;

	huart4.Init.BaudRate=115200;
	HAL_UART_Init(&huart4);
	HAL_UART_Transmit(&huart4, ds_snd, 8, 5);
}

void DS_Read(uint8_t num_bits, uint8_t *dest)
{
	uint8_t ds_snd[9*8];

	for(uint8_t i=0; i<num_bits; i++)
		ds_snd[i]=0xFF;

	huart4.Init.BaudRate=115200;
	HAL_UART_Init(&huart4);
	HAL_UART_AbortReceive_IT(&huart4);
	HAL_UART_Receive_IT(&huart4, dest, num_bits);
	HAL_UART_Transmit(&huart4, ds_snd, num_bits, 10);
}

float DS_GetTemp(void)
{
	uint8_t local_ds[9*8];

	DS_Reset();
	HAL_Delay(1);
	DS_Write(0xCC);
	DS_Write(0x44);
	DS_Reset();
	HAL_Delay(1);
	DS_Write(0xCC);
	DS_Write(0xBE);
	DS_Read(9*8, local_ds);

	uint16_t temp=0;
	for(uint8_t i=0; i<16; i++)
	{
		temp<<=1;
		if(local_ds[15-i]==0xFF)
			temp|=1;
	}

	if(!DS_CheckCksum(local_ds, 9))
		return (float)temp/16.0;
	else
		return -2137.0;	//niedobrze :)
}
