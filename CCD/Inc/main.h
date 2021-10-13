/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  ** This notice applies to any and all portions of this file
  * that are not between comment pairs USER CODE BEGIN and
  * USER CODE END. Other portions of this file, whether 
  * inserted by the user or by software development tools
  * are owned by their respective copyright owners.
  *
  * COPYRIGHT(c) 2021 STMicroelectronics
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32h7xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "korekty_icumsa.h"
#include "tables.h"
/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */
#define NUM_PIX				3700					//ile wszystkich pikseli z CCD idzie, razem z tymi "skrajnymi"
#define REAL_PIX_NUM		3648					//ile prawdziwych pikseli obrazu jest w CCD
#define EEPROM_BASE_ADDR	2137					//adres bazowy danych przechowywanych w eepromie
#define	AVG_LINES			10						//usrednianie tylu linijek CCD
#define	AVG_FILTER			8						//parametr filtra wygladzajacego
#define	STD_DEV_LEN			64						//z ilu probek liczymy odchylenie standardowe
#define PIX_NUM_AVG			60						//z ilu wartosci pix_num liczymy srednia
/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */
#define ICG_H		(GPIOA->BSRRL=1<<5)
#define ICG_L		(GPIOA->BSRRH=1<<5)
#define SH_H		(GPIOA->BSRRL=1<<3)
#define SH_L		(GPIOA->BSRRH=1<<3)
/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define SCLK_Pin GPIO_PIN_2
#define SCLK_GPIO_Port GPIOE
#define EEPROM_WP_Pin GPIO_PIN_3
#define EEPROM_WP_GPIO_Port GPIOE
#define EEPROM_CS_Pin GPIO_PIN_4
#define EEPROM_CS_GPIO_Port GPIOE
#define SDO_Pin GPIO_PIN_5
#define SDO_GPIO_Port GPIOE
#define SDI_Pin GPIO_PIN_6
#define SDI_GPIO_Port GPIOE
#define EEPROM_HOLD_Pin GPIO_PIN_13
#define EEPROM_HOLD_GPIO_Port GPIOC
#define CCD_OS_Pin GPIO_PIN_1
#define CCD_OS_GPIO_Port GPIOC
#define DS18B20_TX_Pin GPIO_PIN_0
#define DS18B20_TX_GPIO_Port GPIOA
#define DS18B20_RX_Pin GPIO_PIN_1
#define DS18B20_RX_GPIO_Port GPIOA
#define CCD_SH_Pin GPIO_PIN_3
#define CCD_SH_GPIO_Port GPIOA
#define CCD_ICG_Pin GPIO_PIN_5
#define CCD_ICG_GPIO_Port GPIOA
#define I_LED_Pin GPIO_PIN_1
#define I_LED_GPIO_Port GPIOB
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/