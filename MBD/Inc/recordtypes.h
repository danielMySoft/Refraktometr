/*
 * recordtypes.h
 *
 *  Created on: 03.01.2019
 *      Author: W.K.
 */

#ifndef RECORDTYPES_H_
#define RECORDTYPES_H_

#define A_HEAD					0x00
#define A_PC					0xFF
#define A_MAINBOARD				0x80

#define R_TEST					0
#define R_STER_IN				1
#define R_WSPOLCZYNNIKI_IN		2
#define R_DIAGN_IN				3
#define R_RTC_IN				4
#define R_PAR_IN				5
#define R_WSPOLCZYNNIKI_OUT		6
#define R_POMIAR_OUT			7
#define R_STROJENIE_OUT			8
#define R_STATE_OUT				9
#define R_STRING				10
#define R_ROM_OUT				11
#define R_RAM_OUT				12
#define R_MEM_TEST_OUT			13
#define R_RTC_OUT				14
#define R_PAR_OUT				15
#define R_INFO_OUT				16
#define R_CCD_IN				17
#define R_CCD_MODE_IN			18
#define R_TAB_IN				19
#define R_TAB_OUT1				20
#define R_TAB_IN1				21
#define R_KAS_ARCH_IN			22
#define R_READ_ARCH_IN			23
#define R_OK					25
#define R_AUTOLED				26		//dodany przez nas rekord
#define R_CCD_DATA				0x80

#endif /* RECORDTYPES_H_ */
