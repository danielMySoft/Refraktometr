/*
 * eeprom.h
 *
 *  Created on: 06.05.2021
 *      Author: SP5WWP
 */

#ifndef EEPROM_H_
#define EEPROM_H_

#include <stdint.h>

void eepromWriteByte(uint32_t addr, uint8_t val);
uint8_t eepromReadByte(uint32_t addr);
void eepromWriteFloat(uint32_t addr, float val);
float eepromReadFloat(uint32_t addr);
void eepromWriteInt(uint32_t addr, uint16_t val);
uint16_t eepromReadInt(uint32_t addr);

#endif /* EEPROM_H_ */
