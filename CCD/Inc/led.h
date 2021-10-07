/*
 * led.h
 *
 *  Created on: 27.04.2021
 *      Author: SP5WWP
 */

#ifndef LED_H_
#define LED_H_

#include <stdint.h>

#define R1	5100.0f
#define R2	390.0f
#define RI	51.0f
#define REF	3.3f

void setLedCurrent(float current);
float getLedCurrent(void);
uint16_t getContrast(uint16_t *inp);

void autoLed(void);

#endif /* LED_H_ */
