/*
 * eeprom.c
 *
 *  Created on: 06.05.2021
 *      Author: SP5WWP
 */

#include "eeprom.h"
#include "spi.h"

void eepromWriteByte(uint32_t addr, uint8_t val)
{
	uint8_t wren=0b00000110;
	uint8_t seq[5]={0b00000010, addr>>16, addr>>8, addr, val}; //WRITE, addr, data

	HAL_GPIO_WritePin(EEPROM_CS_GPIO_Port, EEPROM_CS_Pin, 0);
	HAL_SPI_Transmit(&hspi4, &wren, 1, 5);
	HAL_GPIO_WritePin(EEPROM_CS_GPIO_Port, EEPROM_CS_Pin, 1);
	HAL_Delay(5);
	HAL_GPIO_WritePin(EEPROM_CS_GPIO_Port, EEPROM_CS_Pin, 0);
	HAL_SPI_Transmit(&hspi4, seq, 5, 5);
	HAL_GPIO_WritePin(EEPROM_CS_GPIO_Port, EEPROM_CS_Pin, 1);
}

uint8_t eepromReadByte(uint32_t addr)
{
	uint8_t tmp=0;
	uint8_t seq[4]={0b00000011, addr>>16, addr>>8, addr}; //READ, addr

	HAL_GPIO_WritePin(EEPROM_CS_GPIO_Port, EEPROM_CS_Pin, 0);
	HAL_SPI_Transmit(&hspi4, seq, 4, 5);
	HAL_SPI_Receive(&hspi4, &tmp, 1, 5);
	HAL_GPIO_WritePin(EEPROM_CS_GPIO_Port, EEPROM_CS_Pin, 1);

	return tmp;
}

void eepromWriteFloat(uint32_t addr, float val)
{
	uint8_t f_b[4];

	memcpy((uint8_t*)f_b, (uint8_t*)&val, 4);

	for(uint8_t i=0; i<4; i++)
	{
		eepromWriteByte(addr+i, f_b[i]);
		HAL_Delay(8);
	}
}

float eepromReadFloat(uint32_t addr)
{
	uint8_t f_b[4];
	float val;

	for(uint8_t i=0; i<4; i++)
		f_b[i]=eepromReadByte(addr+i);

	memcpy((uint8_t*)&val, (uint8_t*)f_b, 4);

	return val;
}

void eepromWriteInt(uint32_t addr, uint16_t val)
{
	uint8_t i_b[2];

	memcpy((uint8_t*)i_b, (uint8_t*)&val, 2);

	for(uint8_t i=0; i<2; i++)
	{
		eepromWriteByte(addr+i, i_b[i]);
		HAL_Delay(8);
	}
}

uint16_t eepromReadInt(uint32_t addr)
{
	uint8_t i_b[2];
	uint16_t val;

	for(uint8_t i=0; i<2; i++)
		i_b[i]=eepromReadByte(addr+i);

	memcpy((uint8_t*)&val, (uint8_t*)i_b, 2);

	return val;
}
