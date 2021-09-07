/*
 * ccd_math.h
 *
 *  Created on: 06.05.2021
 *      Author: SP5WWP
 */

#ifndef CCD_MATH_H_
#define CCD_MATH_H_

#include "main.h"
#include <stdint.h>
#include <math.h>

void lowpass(uint32_t *in, const uint16_t siz, const uint8_t len);
void std_dev(uint32_t *arr, const uint16_t len);

#endif /* CCD_MATH_H_ */
