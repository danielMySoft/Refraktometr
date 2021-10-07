/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
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
#include "stm32f4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */
struct SETTINGS {
	uint8_t measurementType;
	uint8_t measurementUnit;
	uint8_t thermalCompensation;
	uint8_t singleMeasurementTime;
	uint8_t language;
	uint8_t contrast;
	uint8_t storageMode;
	uint8_t archiveFirstEntryOffset;
	uint8_t archiveEntriesTotal;
	uint16_t serialNumber;
} settings;

struct ARCHIVE {
	uint8_t day;
	uint8_t month;
	uint8_t year;
	uint8_t hour;
	uint8_t minute;
	uint8_t second;
	float temperature;
	float nc;
	float nck;
	float brix;
	float brixk;
} ArchiveEntry;
/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define K1_OUT_Pin GPIO_PIN_2
#define K1_OUT_GPIO_Port GPIOE
#define K2_OUT_Pin GPIO_PIN_3
#define K2_OUT_GPIO_Port GPIOE
#define K3_Pin GPIO_PIN_4
#define K3_GPIO_Port GPIOE
#define K4_Pin GPIO_PIN_5
#define K4_GPIO_Port GPIOE
#define K5_Pin GPIO_PIN_6
#define K5_GPIO_Port GPIOE
#define LSE_XTAL1_Pin GPIO_PIN_14
#define LSE_XTAL1_GPIO_Port GPIOC
#define LSE_XTAL2_Pin GPIO_PIN_15
#define LSE_XTAL2_GPIO_Port GPIOC
#define HSE_XTAL1_Pin GPIO_PIN_0
#define HSE_XTAL1_GPIO_Port GPIOH
#define HSE_XTAL2_Pin GPIO_PIN_1
#define HSE_XTAL2_GPIO_Port GPIOH
#define OLED_OFF_Pin GPIO_PIN_1
#define OLED_OFF_GPIO_Port GPIOA
#define OLED_RESET_Pin GPIO_PIN_2
#define OLED_RESET_GPIO_Port GPIOA
#define OLED_D_C_Pin GPIO_PIN_4
#define OLED_D_C_GPIO_Port GPIOA
#define OLED_CLK_Pin GPIO_PIN_5
#define OLED_CLK_GPIO_Port GPIOA
#define OLED_CS_Pin GPIO_PIN_6
#define OLED_CS_GPIO_Port GPIOA
#define OLED_DATA_Pin GPIO_PIN_7
#define OLED_DATA_GPIO_Port GPIOA
#define LED1_Pin GPIO_PIN_0
#define LED1_GPIO_Port GPIOB
#define LED2_Pin GPIO_PIN_1
#define LED2_GPIO_Port GPIOB
#define POWER_OFF_Pin GPIO_PIN_2
#define POWER_OFF_GPIO_Port GPIOB
#define SPI_CS_Pin GPIO_PIN_12
#define SPI_CS_GPIO_Port GPIOB
#define SPI_SCK_Pin GPIO_PIN_13
#define SPI_SCK_GPIO_Port GPIOB
#define SPI_MISO_Pin GPIO_PIN_14
#define SPI_MISO_GPIO_Port GPIOB
#define SPI_MOSI_Pin GPIO_PIN_15
#define SPI_MOSI_GPIO_Port GPIOB
#define SPI_WP_Pin GPIO_PIN_8
#define SPI_WP_GPIO_Port GPIOD
#define TXD_CCD_Pin GPIO_PIN_6
#define TXD_CCD_GPIO_Port GPIOC
#define RXD_CCD_Pin GPIO_PIN_7
#define RXD_CCD_GPIO_Port GPIOC
#define TXD_USB_Pin GPIO_PIN_5
#define TXD_USB_GPIO_Port GPIOD
#define RXD_USB_Pin GPIO_PIN_6
#define RXD_USB_GPIO_Port GPIOD
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
