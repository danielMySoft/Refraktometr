/*
 * ds.h
 *
 *  Created on: 30.04.2021
 *      Author: Programista
 */

#ifndef DS_H_
#define DS_H_

#include <stdint.h>

uint8_t DS_CheckCksum(uint8_t *inp, uint8_t numBytes);
void DS_Reset(void);
void DS_Write(uint8_t val);
void DS_Read(uint8_t num_bits, uint8_t *dest);

float DS_GetTemp(void);

#endif /* DS_H_ */
