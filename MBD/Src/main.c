/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include "graph.h"
//#include "font.h"
#include "bitmaps.h"
#include "recordtypes.h"
#include "constants.h"
#include "kbd.h"
#include "OLED.h"
#include "EEPROM.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

//#define PIX_NUM				3694
//#define	PIX_BUFF_LEN		3700
//#define PIX_START			32
//#define PIX_TAIL			14

#define SOFTWARE_VERSION "v1.0"
#define SERIAL_NUMBER 1
#define POLISH 0
#define ENGLISH 1


//typy ekranow
#define FRAME_BP 0
#define FRAME_MAIN_MENU 1
#define FRAME_POMIAR 2
#define FRAME_SET_VALUE 3
#define FRAME_ARCHIVE_BEING_ERASED 4
#define FRAME_ARCHIVE_BROWSE 5
#define FRAME_2_BIG_LINES 6
#define FRAME_1_BIG_LINE 7

#define FRAME_EXIT 255

//ustawienia
#define CCD_RX_BUFFER_SIZE 1000
#define USB_RX_BUFFER_SIZE 1000
#define SAMPLE_DETECT_TIMEOUT 2//4
#define BP_FLOATING_DELAY 60
#define SCREEN_SAVER_DELAY 300
#define SCREEN_SAVER_BP_DELAY 300		//Delay dla BRAK PROBKI
#define SS_CONTRAST 0x07
#define DEFAULT_CONTRAST 0x09
#define MAX_CONTRAST 0x09
#define ARCHIVE_INFO_SCREEN_DELAY 2000
#define KEYBOARD_REPEAT_RATE 20
#define MEASUREMENT_SINGLE 0
#define MEASUREMENT_CONTINUOUS 1
#define UNIT_BRIX 0
#define UNIT_RI 1
#define OFF 0
#define ON 1
#define STORAGE_PROHIBITED 0
#define STORAGE_BY_KEYBOARD 1

#define POMIAR_ARRAY_SIZE 5//50		//wielkosc bufora pomiarow do usredniania. 8 -> pomiary usrednione z 1 sek. 16 -> 2 sek itd


//POPRAWNE USTAWIENIA
#define M_TYPES 2
#define UNIT_TYPES 2
#define T_COMP_OPTIONS 2
#define S_M_TIME_MIN 15
#define S_M_TIME_MAX 40
#define NO_LANGUAGES 2
#define STORAGE_MODES 2

//INNE
#define DEVICE_BRAND_NEW settings.language == 0xFF && settings.measurementType == 0xFF && settings.measurementUnit == 0xFF && settings.singleMeasurementTime == 0xFF && settings.singleMeasurementTime == 0xFF && settings.storageMode == 0xFF && settings.archiveEntriesTotal == 0xFF && settings.archiveFirstEntryOffset == 0xFF
#define SETTINGS_INCORRECT settings.language >= NO_LANGUAGES || settings.measurementType >= M_TYPES || settings.measurementUnit >= UNIT_TYPES || settings.thermalCompensation >= T_COMP_OPTIONS || settings.singleMeasurementTime < S_M_TIME_MIN || settings.singleMeasurementTime > S_M_TIME_MAX || settings.storageMode >= STORAGE_MODES || settings.archiveEntriesTotal > ARCHIVE_SIZE || settings.archiveFirstEntryOffset >= ARCHIVE_END_PTR
//STANY
#define DEVICE_IDLE ssTimer == 0xFF
#define POMIAR_STOP 0
#define POMIAR_START 1
#define POMIAR_PENDING 2
#define POMIAR_DISP 3
#define POMIAR_DONE 4
#define POMIAR_NOP 5


#define NO_SAMPLE (pomiar.nc == 0 || pomiar.nc == 1)
//#define NO_SAMPLE (pomiarStatus == POMIAR_NOP)



//KEYBOARD CONTEXTS
#define CONTEXT_MAIN_SCREEN 0
#define CONTEXT_MAIN_MENU 1

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
RTC_HandleTypeDef hrtc;

SPI_HandleTypeDef hspi1;
SPI_HandleTypeDef hspi2;

TIM_HandleTypeDef htim6;

UART_HandleTypeDef huart2;
UART_HandleTypeDef huart6;
DMA_HandleTypeDef hdma_usart2_rx;
DMA_HandleTypeDef hdma_usart2_tx;
DMA_HandleTypeDef hdma_usart6_rx;
DMA_HandleTypeDef hdma_usart6_tx;

/* USER CODE BEGIN PV */
struct POMIARBUFFER {
	volatile float temperatura;		//temperatura zmierzona
	volatile float nc;				//wspolczynnik zalamania zmierzony w temperaturze pomiaru
	volatile float nck;				//wspolczynnik zalamania obliczony dla temp. 20*C
	volatile float brix;			//brixy zmierzone
	volatile float brixk;			//brixy po korekcji temperaturowej
} PomiarBuffer;

struct FRAME {
	uint8_t type;			//frame type - normal w/ 1 or 2 lines at top/mid, Main menu etc.
	char firstLine[24];
	char secondLine[24];
	char thirdLine[24];
	char bottomLine[24];
	uint8_t level;
	//uint8_t root;
	uint8_t position[4];	// current position at level 0,1,2 (coordinates to current position)
	uint8_t totalPositions;	// total available positions on screen
	uint16_t value;
	uint16_t valueMin;
	uint16_t valueMax;
	char buffer[16];
} frame;

struct POMIAR {						//finalny pomiar po usrednianiu
	volatile float temperatura;		//temperatura zmierzona
	volatile float nc;				//wspolczynnik zalamania zmierzony w temperaturze pomiaru
	volatile float nck;				//wspolczynnik zalamania obliczony dla temp. 20*C
	volatile float brix;			//brixy zmierzone
	volatile float brixk;			//brixy po korekcji temperaturowej
	volatile float I_LED;			//prad LEDa w jednostkach arbitralnych
	volatile float num_pix;			//numer granicznego piksela obliczony
} pomiar;

struct CURRENT_POMIAR		//pojedynczy pomiar. finalny pomiar (POMIAR - patrz wyzej) bedzie zawieral kilka takich usrednionych.
{
	volatile float brix;
	volatile float brixk;
	volatile float I_LED;
	volatile float nc;
	volatile float nck;
	volatile float temperatura;
};

struct KOREKCJA {
	volatile float f;
	volatile float D;
	volatile float tempKor;
	volatile float xKor;
} korekcja;

struct KOREKCJA_PAKIET {
	uint8_t header[2];
	uint16_t serialNumber;
	volatile float f;
	volatile float D;
	volatile float tempKor;
	volatile float xKor;
} korekcja_pakiet;


RTC_TimeTypeDef gTime = {0};
RTC_DateTypeDef gDate = {0};

uint8_t floatingTextDelay;
int8_t xShift;
int8_t yShift;
uint8_t dirY;
uint8_t dirX;
volatile uint8_t CCD_rxBuffer[CCD_RX_BUFFER_SIZE];
volatile uint8_t USB_rxBuffer[USB_RX_BUFFER_SIZE];
volatile uint32_t TicksSinceLastRecordArrived = 0;
volatile uint16_t ssTimer;
volatile uint8_t initDone;
volatile uint8_t pomiarTimer;
volatile uint8_t pomiarStatus;
volatile uint8_t newArchiveEntryAdded;
volatile uint8_t ssStateChanged;
volatile uint8_t pcConnectionTimeout = 3;
volatile uint8_t nacisnijStart = 0;				//flaga specjalnego wyjątku żeby w ladny sposob wyswietlic napis "NACISNIJ START "z polskimi literkami w trybie pomiaru pojedynczego.
volatile uint8_t sampleDetected = 0;
volatile uint8_t ledTimer = 0;
volatile uint8_t onesecondTick = 0;
volatile uint8_t pomiarArrayFull = 0;
volatile uint8_t newPacketCount = 0;
volatile uint8_t packetIsNew = 1;
volatile uint8_t serialNumberModified = 0;


struct CURRENT_POMIAR pomiarArray[POMIAR_ARRAY_SIZE];			//zebrane pomiary do usrednienia
volatile uint8_t packetCounter = 0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_SPI1_Init(void);
static void MX_SPI2_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_USART6_UART_Init(void);
static void MX_TIM6_Init(void);
static void MX_RTC_Init(void);
/* USER CODE BEGIN PFP */
void key_down_callback(uint8_t key, uint8_t context);
void USER_UART6_IDLE_Callback(UART_HandleTypeDef *huart);
void USER_UART2_IDLE_Callback(UART_HandleTypeDef *huart);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

uint8_t CalcChecksum(volatile uint8_t *arr, uint8_t checkSumPositionInPacket)
{
	uint8_t sum=0;

	for(uint8_t i=0; i<checkSumPositionInPacket; i++)
		sum+=arr[i];

	return sum;
}

//moj uart do testow
//void UART_print(char* str)
//{
//	HAL_UART_Transmit(&huart2, (uint8_t*)str, strlen(str), 100);
//}


//funkcje od daty
uint8_t No_of_days_a_month(uint8_t month)
{
	if(gDate.Year % 4 == 0)
	{
		if (month == 2) return 29;
	}
	else
	{
		if (month == 2) return 28;
	}

	switch(month)
	{
		case 1:
			return 31;
			break;

		case 3:
			return 31;
			break;

		case 4:
			return 30;
			break;

		case 5:
			return 31;
			break;

		case 6:
			return 30;
			break;

		case 7:
			return 31;
			break;

		case 8:
			return 31;
			break;

		case 9:
			return 30;
			break;

		case 10:
			return 31;
			break;

		case 11:
			return 30;
			break;

		case 12:
			return 31;
			break;
	}
	return 0;
}

void verify_date(void)
{
	if(gDate.Year % 4 == 0)
	{
		switch(gDate.Month)
		{
			case 1:
				if(gDate.Date > 31) gDate.Date = 31;
				break;

			case 2:
				if(gDate.Date > 29) gDate.Date = 29;
				break;

			case 3:
				if(gDate.Date > 31) gDate.Date = 31;
				break;

			case 4:
				if(gDate.Date > 30) gDate.Date = 30;
				break;

			case 5:
				if(gDate.Date > 31) gDate.Date = 31;
				break;

			case 6:
				if(gDate.Date > 30) gDate.Date = 30;
				break;

			case 7:
				if(gDate.Date > 31) gDate.Date = 31;
				break;

			case 8:
				if(gDate.Date > 31) gDate.Date = 31;
				break;

			case 9:
				if(gDate.Date > 30) gDate.Date = 30;
				break;

			case 10:
				if(gDate.Date > 31) gDate.Date = 31;
				break;

			case 11:
				if(gDate.Date > 30) gDate.Date = 30;
				break;

			case 12:
				if(gDate.Date > 31) gDate.Date = 31;
				break;
		}
	}
	else
	{
		switch(gDate.Month)
		{
			case 1:
				if(gDate.Date > 31) gDate.Date = 31;
				break;

			case 2:
				if(gDate.Date > 28) gDate.Date = 28;
				break;

			case 3:
				if(gDate.Date > 31) gDate.Date = 31;
				break;

			case 4:
				if(gDate.Date > 30) gDate.Date = 30;
				break;

			case 5:
				if(gDate.Date > 31) gDate.Date = 31;
				break;

			case 6:
				if(gDate.Date > 30) gDate.Date = 30;
				break;

			case 7:
				if(gDate.Date > 31) gDate.Date = 31;
				break;

			case 8:
				if(gDate.Date > 31) gDate.Date = 31;
				break;

			case 9:
				if(gDate.Date > 30) gDate.Date = 30;
				break;

			case 10:
				if(gDate.Date > 31) gDate.Date = 31;
				break;

			case 11:
				if(gDate.Date > 30) gDate.Date = 30;
				break;

			case 12:
				if(gDate.Date > 31) gDate.Date = 31;
				break;
		}
	}
}

void fetch_date_time(void)
{
	 HAL_RTC_GetTime(&hrtc, &gTime, RTC_FORMAT_BIN);
	 HAL_RTC_GetDate(&hrtc, &gDate, RTC_FORMAT_BIN);
	 if (gDate.Year == 0) gDate.Year = 20;
	 if (gDate.Month == 0) gDate.Year = 1;
	 if (gDate.Date == 0) gDate.Date = 1;
}

//weryfikacja ustawien i debug
void verify_settings(void)
{
	if (DEVICE_BRAND_NEW)	// if eeprom never programmed before -> load default settings
	{

		settings.language = POLISH;
		settings.measurementType = MEASUREMENT_SINGLE;
		settings.measurementUnit = UNIT_BRIX;
		settings.thermalCompensation = ON;
		settings.singleMeasurementTime = 15;
		settings.contrast = DEFAULT_CONTRAST;
		settings.storageMode = STORAGE_BY_KEYBOARD;
		settings.archiveEntriesTotal = 0;
		settings.archiveFirstEntryOffset = 0;
		settings.serialNumber = 0;

		OLED_CLEAR();
		OLED_PutStr(2, 24, "EEPROM uninitialized", 0, 0x09);
		OLED_PutStr(2, 33, "initializing EEPROM", 0, 0x09);
		EEPROM_write_multiple(EEPROM_SETTINGS_ADDRESS, (uint8_t*)&settings, sizeof(settings));		//zapisz domyslne ustawienia
		HAL_Delay(4000);
		OLED_PutStr(2, 42, "DONE!", 0, 0x09);
		HAL_Delay(1000);
		OLED_CLEAR();

	}

	if (SETTINGS_INCORRECT)
	{
		settings.language = POLISH;
		settings.measurementType = MEASUREMENT_SINGLE;
		settings.measurementUnit = UNIT_BRIX;
		settings.thermalCompensation = ON;
		settings.singleMeasurementTime = 15;
		settings.contrast = DEFAULT_CONTRAST;
		settings.storageMode = ARCHIVE_STORAGE_MODE_BY_KEYBOARD;
		erase_archive();
		OLED_CLEAR();


		OLED_PutStr(0, 24, "MEMORY ERROR", 0, 0x09);
		OLED_PutStr(0, 33, "LOADING DEFAULT", 0, 0x09);
		OLED_PutStr(0, 42, "SETTINGS", 0, 0x09);
		HAL_Delay(5000);
		OLED_CLEAR();
	}
}

void settings_debug(void)
{
	OLED_CLEAR();
	char a[20];
	sprintf(a,"lang = %d", settings.language);
	OLED_PutStr(0, 0, a, 0, 0x09);
	sprintf(a,"type = %d", settings.measurementType);
	OLED_PutStr(0, 9, a, 0, 0x09);
	sprintf(a,"unit = %d", settings.measurementUnit);
	OLED_PutStr(0, 18, a, 0, 0x09);
	sprintf(a,"tcomp = %d", settings.thermalCompensation);
	OLED_PutStr(0, 27, a, 0, 0x09);
	sprintf(a,"sMeasTim = %d", settings.singleMeasurementTime);
	OLED_PutStr(0, 36, a, 0, 0x09);
	sprintf(a,"contrast = %d", settings.contrast);
	OLED_PutStr(0, 45, a, 0, 0x09);
	sprintf(a,"storMode = %d", settings.storageMode);
	OLED_PutStr(0, 54, a, 0, 0x09);
	HAL_Delay(10000);
	OLED_CLEAR();
	sprintf(a,"archTotEnt = %d", settings.archiveEntriesTotal);
	OLED_PutStr(0, 0, a, 0, 0x09);
	sprintf(a,"aFirstEntOfs = %d", settings.archiveFirstEntryOffset);
	OLED_PutStr(0, 9, a, 0, 0x09);
	HAL_Delay(5000);
	OLED_CLEAR();
}
//Funkcje od wygaszacza

void reset_screen_saver(void)
{
	ssTimer = 0;
	ssStateChanged = 1;
	//settings.contrast = DEFAULT_CONTRAST;
	settings.contrast = EEPROM_read(EEPROM_CONTRAST_ADDR);
	//EEPROM_readSettings(EEPROM_CONTRAST_ADDR, &settings.contrast, 1);
}

// Menu i wyswietlanie
void exit_main_menu()
{
	if(settings.measurementType != MEASUREMENT_SINGLE) nacisnijStart = 0;		// dodane aby usunac buga wyswietlania przy zmianie trybu pomiaru. nie kasowal sie napis NACISNIJ

	OLED_CLEAR();
	frame.position[0] = 0;
	frame.position[1] = 0;
	frame.position[2] = 0;
	frame.level = 0;
	frame.type = FRAME_EXIT;
}

void MM_up_one_level(void)
{
	 frame.type = FRAME_MAIN_MENU;
	 frame.position[frame.level] = 0;
	 frame.level--;
}

void drawFrame(void)
{
	//get current time
	 fetch_date_time();
	 verify_date();
	// wyswietlanie

	switch (frame.type)
	{
		case FRAME_1_BIG_LINE:
			if (settings.thermalCompensation == OFF) sprintf(frame.firstLine, Main_Screen[3][settings.language]);
			else sprintf(frame.firstLine, Main_Screen[0][settings.language]);
			OLED_PutStr(40, 1, frame.firstLine, 0, settings.contrast);
			OLED_PutStr(6, 24, frame.secondLine, 1, settings.contrast);
			sprintf(frame.bottomLine,"%02u-%02u-%u   %02u:%02u:%02u", gDate.Date, gDate.Month, gDate.Year+2000, gTime.Hours,gTime.Minutes,gTime.Seconds);
			OLED_PutStr(0, 56, frame.bottomLine, 0, settings.contrast);
			break;



		case FRAME_2_BIG_LINES:
			if (settings.thermalCompensation == OFF) sprintf(frame.firstLine, Main_Screen[3][settings.language]);
			else sprintf(frame.firstLine, Main_Screen[0][settings.language]);
			OLED_PutStr(40, 1, frame.firstLine, 0, settings.contrast);

			if (newArchiveEntryAdded && settings.archiveEntriesTotal != 0)
			{
				sprintf(frame.buffer,"A%03u", settings.archiveEntriesTotal);
				OLED_PutStr(10, 1, frame.buffer, 0, settings.contrast);

			}
			else
			{
				OLED_PutStr(10, 1, "    ", 0, settings.contrast);
			}
//	Specjalny warunek zeby ladnie wyswietlic NACISNIJ w napisie "NACISNIJ_START"
			if (nacisnijStart == 1)	//dla polskiej wersji ten bajzel
			{
				uint8_t tmpVar = 12;	//Pozycja x napisu NACISNIJ
				//OLED_PutStr(nacPos + 18, 15, "I", 1, settings.contrast);
				OLED_PutStr(tmpVar, 15, "NAC", 1, settings.contrast);
				OLED_PutStr(tmpVar + 21, 15, "SN", 1, settings.contrast);
				OLED_PutStr(tmpVar + 36, 15, "J", 1, settings.contrast);
				OLED_PutStr(18, 34, frame.thirdLine, 1, settings.contrast);
				OLED_bitmap(tmpVar + 18, 15, b8);		//pierwsza litera I
				OLED_bitmap(tmpVar + 33, 15, b8);		//druga
				OLED_bitmap(tmpVar + 22, 11, b1);		// kreska nad litera s
			}
			else	//dla całej reszty normalnie
			{
				OLED_PutStr(3, 15, frame.secondLine, 1, settings.contrast);
				OLED_PutStr(3, 34, frame.thirdLine, 1, settings.contrast);
			}
// Koniec specjalnego warunku
			sprintf(frame.bottomLine,"%02u-%02u-%u   %02u:%02u:%02u", gDate.Date, gDate.Month, gDate.Year+2000, gTime.Hours,gTime.Minutes,gTime.Seconds);
			OLED_PutStr(0, 56, frame.bottomLine, 0, settings.contrast);
			break;



		case FRAME_ARCHIVE_BROWSE:
			if (frame.value > frame.valueMax) frame.value = frame.valueMin;
			if (frame.value < frame.valueMin) frame.value = frame.valueMax;

			if (settings.archiveEntriesTotal == 0)		//jak archiwum puste
			{
				if (settings.language == POLISH)
				{
					OLED_PutStr(20, 15, Archive[3][settings.language], 1, settings.contrast);
					OLED_PutStr(8, 34, Archive[7][settings.language], 1, settings.contrast);
				}
				if (settings.language == ENGLISH)
				{
					OLED_PutStr(25, 15, Archive[3][settings.language], 1, settings.contrast);
					OLED_PutStr(11, 34, Archive[7][settings.language], 1, settings.contrast);
				}
			 	HAL_Delay(ARCHIVE_INFO_SCREEN_DELAY);
				OLED_CLEAR();
				frame.type = FRAME_MAIN_MENU;
			}
			else
			{
				if (settings.measurementUnit == UNIT_BRIX)
				{
					if (settings.thermalCompensation == OFF)
					{
						if (frame.value < 10) sprintf(frame.firstLine, "%s %s %d %s %d  " , Archive[7][settings.language], Archive[9][settings.language], frame.value + 1, Archive[10][settings.language], settings.archiveEntriesTotal);
						else sprintf(frame.firstLine, "%s %s %d %s %d " , Archive[7][settings.language], Archive[9][settings.language], frame.value + 1, Archive[10][settings.language], settings.archiveEntriesTotal);

						OLED_PutStr(0, 1, frame.firstLine, 0, settings.contrast);

						if (ArchiveEntry.brix >= 10) sprintf(frame.secondLine,"  %2.2f %% ", ArchiveEntry.brix);
						else if(ArchiveEntry.brix >= 0) sprintf(frame.secondLine,"   %2.2f %% ", ArchiveEntry.brix);
						else if(ArchiveEntry.brix > -10) sprintf(frame.secondLine,"  %2.2f %% ", ArchiveEntry.brix);
						else if(ArchiveEntry.brix <= -10) sprintf(frame.secondLine," %2.2f %% ", ArchiveEntry.brix);

						OLED_PutStr(3, 15, frame.secondLine, 1, settings.contrast);
						sprintf(frame.thirdLine,"  %2.2f \x7B\x43 ", ArchiveEntry.temperature);
						OLED_PutStr(3, 34, frame.thirdLine, 1, settings.contrast);
						sprintf(frame.bottomLine,"%02u-%02u-%u   %02u:%02u:%02u", ArchiveEntry.day, ArchiveEntry.month, ArchiveEntry.year+2000, ArchiveEntry.hour, ArchiveEntry.minute, ArchiveEntry.second);
						OLED_PutStr(0, 56, frame.bottomLine, 0, settings.contrast);
					}
					else
					{
						if (frame.value < 10) sprintf(frame.firstLine, "%s %s %d %s %d  " , Archive[7][settings.language], Archive[9][settings.language], frame.value + 1, Archive[10][settings.language], settings.archiveEntriesTotal);
						else sprintf(frame.firstLine, "%s %s %d %s %d " , Archive[7][settings.language], Archive[9][settings.language], frame.value + 1, Archive[10][settings.language], settings.archiveEntriesTotal);

						OLED_PutStr(0, 1, frame.firstLine, 0, settings.contrast);

						if (ArchiveEntry.brixk >= 10) sprintf(frame.secondLine,"  %2.2f %% ", ArchiveEntry.brixk);
						else if(ArchiveEntry.brixk >= 0) sprintf(frame.secondLine,"   %2.2f %% ", ArchiveEntry.brixk);
						else if(ArchiveEntry.brixk > -10) sprintf(frame.secondLine,"  %2.2f %% ", ArchiveEntry.brixk);
						else if(ArchiveEntry.brixk <= -10) sprintf(frame.secondLine," %2.2f %% ", ArchiveEntry.brixk);

						OLED_PutStr(3, 15, frame.secondLine, 1, settings.contrast);
						sprintf(frame.thirdLine,"  %d.00 \x7B\x43 ", 20);
						OLED_PutStr(3, 34, frame.thirdLine, 1, settings.contrast);
						sprintf(frame.bottomLine,"%02u-%02u-%u   %02u:%02u:%02u", ArchiveEntry.day, ArchiveEntry.month, ArchiveEntry.year+2000, ArchiveEntry.hour, ArchiveEntry.minute, ArchiveEntry.second);
						OLED_PutStr(0, 56, frame.bottomLine, 0, settings.contrast);
					}
				}
				if(settings.measurementUnit == UNIT_RI)
				{
					if (settings.thermalCompensation == OFF)
					{
						if (frame.value < 10) sprintf(frame.firstLine, "%s %s %d %s %d  " , Archive[7][settings.language], Archive[9][settings.language], frame.value + 1, Archive[10][settings.language], settings.archiveEntriesTotal);
						else sprintf(frame.firstLine, "%s %s %d %s %d " , Archive[7][settings.language], Archive[9][settings.language], frame.value + 1, Archive[10][settings.language], settings.archiveEntriesTotal);

						OLED_PutStr(0, 1, frame.firstLine, 0, settings.contrast);
						sprintf(frame.secondLine,"%1.5f nd", ArchiveEntry.nc);
						OLED_PutStr(3, 15, frame.secondLine, 1, settings.contrast);
						sprintf(frame.thirdLine,"  %2.2f \x7B\x43", ArchiveEntry.temperature);
						OLED_PutStr(3, 34, frame.thirdLine, 1, settings.contrast);
						sprintf(frame.bottomLine,"%02u-%02u-%u   %02u:%02u:%02u", ArchiveEntry.day, ArchiveEntry.month, ArchiveEntry.year+2000, ArchiveEntry.hour, ArchiveEntry.minute, ArchiveEntry.second);
						OLED_PutStr(0, 56, frame.bottomLine, 0, settings.contrast);
					}
					else
					{
						if (frame.value < 10) sprintf(frame.firstLine, "%s %s %d %s %d  " , Archive[7][settings.language], Archive[9][settings.language], frame.value + 1, Archive[10][settings.language], settings.archiveEntriesTotal);
						else sprintf(frame.firstLine, "%s %s %d %s %d " , Archive[7][settings.language], Archive[9][settings.language], frame.value + 1, Archive[10][settings.language], settings.archiveEntriesTotal);
						OLED_PutStr(0, 1, frame.firstLine, 0, settings.contrast);
						sprintf(frame.secondLine,"%1.5f nd", ArchiveEntry.nck);
						OLED_PutStr(3, 15, frame.secondLine, 1, settings.contrast);
						sprintf(frame.thirdLine,"  %d.00 \x7B\x43", 20);
						OLED_PutStr(3, 34, frame.thirdLine, 1, settings.contrast);
						sprintf(frame.bottomLine,"%02u-%02u-%u   %02u:%02u:%02u", ArchiveEntry.day, ArchiveEntry.month, ArchiveEntry.year+2000, ArchiveEntry.hour, ArchiveEntry.minute, ArchiveEntry.second);
						OLED_PutStr(0, 56, frame.bottomLine, 0, settings.contrast);
					}
				}

			}
			break;

		case FRAME_ARCHIVE_BEING_ERASED:
			OLED_CLEAR();
			OLED_PutStr(5 + settings.language * 7, 15, Archive[6][settings.language], 1, settings.contrast);	//kasowanie archiwum
			OLED_PutStr(8 + settings.language * 4, 34, Archive[7][settings.language], 1, settings.contrast);
			erase_archive();
			newArchiveEntryAdded = 0;
			HAL_Delay(ARCHIVE_INFO_SCREEN_DELAY);
			OLED_CLEAR();
			OLED_PutStr(9 + settings.language * 3, 15, Archive[7][settings.language], 1, settings.contrast);	//archiwum skasowane
			OLED_PutStr(6 + settings.language * 9, 34, Archive[8][settings.language], 1, settings.contrast);
			HAL_Delay(ARCHIVE_INFO_SCREEN_DELAY);
			OLED_CLEAR();
			frame.type = FRAME_MAIN_MENU;
			break;

		case FRAME_BP:						//BRAK PROBKI
			floatingTextDelay++;

			if (settings.thermalCompensation == OFF) sprintf(frame.firstLine, Main_Screen[3][settings.language]);
			else sprintf(frame.firstLine, Main_Screen[0][settings.language]);
			sprintf(frame.secondLine, Main_Screen[1][settings.language]);
			sprintf(frame.thirdLine, Main_Screen[2][settings.language]);

			if (floatingTextDelay == BP_FLOATING_DELAY || ssStateChanged)	//jezeli trzeba przesunac BRAK PROBKI lub nastapi moment wlaczenia/wylaczenia się wygaszacza ekranu
			{
				ssStateChanged = 0;
				OLED_PutStr(21 + settings.language * 6 + xShift, 15 + yShift, "    ", 1, settings.contrast);
				OLED_PutStr(15 + xShift, 34 + yShift, "      ", 1, settings.contrast);

				floatingTextDelay = 0;

				if (yShift > 4) dirY = 1;
				if (yShift < -4) dirY = 0;
				if(dirY == 0)
					yShift++;
				else yShift--;

				if (xShift > 12) dirX = 1;
				if (xShift < -12) dirX = 0;
				if(dirX == 0)
					xShift++;
				else xShift--;

				OLED_PutStr(21 + settings.language * 6 + xShift, 15 + yShift, frame.secondLine, 1, settings.contrast);
				OLED_PutStr(15 + xShift, 34 + yShift, frame.thirdLine, 1, settings.contrast);
				if(settings.language == POLISH) OLED_bitmap(14 + xShift + 14, 34 + yShift - 4, b1);		//kreska nad O w BRAK PROBKI, kontrast zmieniany wewnatrz funkcji OLED_BITMAP
				if(settings.language == POLISH) OLED_bitmap(31 + xShift + 14, 34 + yShift, b8);		//poprawiony wyglad litery I
			}

			OLED_PutStr(40, 1, frame.firstLine, 0, settings.contrast);
			sprintf(frame.bottomLine,"%02u-%02u-%u   %02u:%02u:%02u", gDate.Date, gDate.Month, gDate.Year+2000, gTime.Hours,gTime.Minutes,gTime.Seconds);
			OLED_PutStr(0, 56, frame.bottomLine, 0, settings.contrast);
			break;

		case FRAME_SET_VALUE:
			if (frame.value > frame.valueMax) frame.value = frame.valueMin;
			if (frame.value < frame.valueMin) frame.value = frame.valueMax;

			OLED_PutStr(10, 0, frame.firstLine, 0, settings.contrast);
			OLED_PutStr(5, 10, frame.secondLine, 0, settings.contrast);
			OLED_PutStr(5, 19, frame.thirdLine, 0, settings.contrast);
			sprintf(frame.buffer,"%d  ",frame.value);
			OLED_PutStr(10, 33, frame.buffer, 1, settings.contrast);
			OLED_PutStr(5, 55, frame.bottomLine, 0, settings.contrast);
			break;

		case FRAME_MAIN_MENU:
			if (frame.level == 0)			//jezeli w glownym menu
			{
				frame.totalPositions = 6;
				OLED_PutStr(10, 0, Main_Menu[0][settings.language], 0, settings.contrast);			//tytul okna (menu glowne)
				OLED_PutStr(5, 10, Main_Menu[1][settings.language], 0, settings.contrast);			//opcje do wyboru
				OLED_PutStr(5, 19, Main_Menu[2][settings.language], 0, settings.contrast);
				OLED_PutStr(5, 28, Main_Menu[3][settings.language], 0, settings.contrast);
				OLED_PutStr(5, 37, Main_Menu[4][settings.language], 0, settings.contrast);
				OLED_PutStr(5, 46, Main_Menu[5][settings.language], 0, settings.contrast);
				OLED_PutStr(5, 55, Main_Menu[6][settings.language], 0, settings.contrast);
				OLED_PutStr(1, frame.position[frame.level]*9 + 10, ">", 0, settings.contrast);
			}

			if (frame.level == 1)			//jezeli w podmenu menu glownego
			{
				switch (frame.position[0])	//sprawdz ktore podmenu menu glownego zostalo wybrane
				{
					case 0: 				//Rodzaj pomiaru
						frame.totalPositions = 3;
						OLED_PutStr(10, 0, Main_Menu[1][settings.language], 0, settings.contrast);			//tytul okna (rodzaj pomiaru)
						OLED_PutStr(5, 10, Measurement_Type[0][settings.language], 0, settings.contrast);
						OLED_PutStr(5, 19, Measurement_Type[1][settings.language], 0, settings.contrast);

//test - zakomentowac 2 linijki powyzej. dodaje gwiazdke na koncu obecnie uzywanego parametru
//						if(settings.measurementType == MEASUREMENT_SINGLE)
//						{
//							sprintf(frame.buffer,"%s*",Measurement_Type[0][settings.language]);
//							OLED_PutStr(5, 10, frame.buffer, 0, settings.contrast);
//						}
//						else OLED_PutStr(5, 10, Measurement_Type[0][settings.language], 0, settings.contrast);
//
//						if(settings.measurementType == MEASUREMENT_CONTINUOUS)
//						{
//							sprintf(frame.buffer,"%s*",Measurement_Type[1][settings.language]);
//							OLED_PutStr(5, 19, frame.buffer, 0, settings.contrast);
//						}
//						else OLED_PutStr(5, 19, Measurement_Type[1][settings.language], 0, settings.contrast);
//test end


						OLED_PutStr(5, 28, Main_Menu[7][settings.language], 0, settings.contrast);
						OLED_PutStr(1, frame.position[frame.level]*9 + 10, ">", 0, settings.contrast);
					break;

					case 1:					//sposob wyswietlania (jednostka)
						frame.totalPositions = 3;
						OLED_PutStr(10, 0, Main_Menu[2][settings.language], 0, settings.contrast);			//tytul okna (sposob wyswietlania)
						OLED_PutStr(5, 10, "BRIX", 0, settings.contrast);
						OLED_PutStr(5, 19, Measurement_Unit[0][settings.language], 0, settings.contrast);
						OLED_PutStr(5, 28, Main_Menu[7][settings.language], 0, settings.contrast);
						OLED_PutStr(1, frame.position[frame.level]*9 + 10, ">", 0, settings.contrast);
					break;

					case 2:					//kompensacja temperatury
						frame.totalPositions = 3;
						OLED_PutStr(10, 0, Thermal_Compensation[0][settings.language], 0, settings.contrast);			//tytul okna (komensacja temperatury)
						OLED_PutStr(5, 10, Thermal_Compensation[1][settings.language], 0, settings.contrast);
						OLED_PutStr(5, 19, Thermal_Compensation[2][settings.language], 0, settings.contrast);
						OLED_PutStr(5, 28, Main_Menu[7][settings.language], 0, settings.contrast);
						OLED_PutStr(1, frame.position[frame.level]*9 + 10, ">", 0, settings.contrast);
					break;

					case 3:					//ustawienia
						frame.totalPositions = 5;
						OLED_PutStr(10, 0, Settings[0][settings.language], 0, settings.contrast);			//tytul okna (ustawienia)
						OLED_PutStr(5, 10, Settings[1][settings.language], 0, settings.contrast);
						OLED_PutStr(5, 19, Settings[2][settings.language], 0, settings.contrast);
						OLED_PutStr(5, 28, Settings[3][settings.language], 0, settings.contrast);
						OLED_PutStr(5, 37, Settings[4][settings.language], 0, settings.contrast);
						OLED_PutStr(5, 46, Main_Menu[7][settings.language], 0, settings.contrast);
						OLED_PutStr(1, frame.position[frame.level]*9 + 10, ">", 0, settings.contrast);
					break;

					case 4:
											// archiwum
						frame.totalPositions = 4;
						OLED_PutStr(10, 0, Archive[7][settings.language], 0, settings.contrast);			//tytul okna (ustawienia)
						OLED_PutStr(5, 10, Archive[0][settings.language], 0, settings.contrast);
						OLED_PutStr(5, 19, Archive[1][settings.language], 0, settings.contrast);
						OLED_PutStr(5, 28, Archive[2][settings.language], 0, settings.contrast);
						OLED_PutStr(5, 37, Main_Menu[7][settings.language], 0, settings.contrast);
						OLED_PutStr(1, frame.position[frame.level]*9 + 10, ">", 0, settings.contrast);
						break;

					case 5:					//wyjscie
						exit_main_menu();
					break;
				}
			}

			if (frame.level == 2)			//jezeli w podmenu ustawienia/Archiwum
			{
				if (frame.position[0] == MAIN_MENU_SETTINGS)	//jesli w podmenu ustawienia
				{
					switch (frame.position[1])	//sprawdz ktore podmenu menu ustawienia zostalo wybrane
					{
						 case SETTINGS_SUBMENU_SINGLE_MEASUREMENT_TIME:
							 frame.valueMin = 15;
							 frame.valueMax = 40;
							 sprintf(frame.firstLine,Settings[0][settings.language]);
							 sprintf(frame.secondLine, Settings[1][settings.language]);
							 sprintf(frame.thirdLine, "MIN   %d", frame.valueMin);
							 frame.value = settings.singleMeasurementTime;
							 sprintf(frame.bottomLine, "MAX   %d", frame.valueMax);
							 OLED_CLEAR();
							 frame.type = FRAME_SET_VALUE;
							 break;

						case SETTINGS_SUBMENU_DATE:				//data
							frame.totalPositions = 4;
							OLED_PutStr(10, 0, Settings[5][settings.language], 0, settings.contrast);			//tytul okna (ustaw date)
							OLED_PutStr(5, 10, Settings[9][settings.language], 0, settings.contrast);
							OLED_PutStr(5, 19, Settings[8][settings.language], 0, settings.contrast);
							OLED_PutStr(5, 28, Settings[7][settings.language], 0, settings.contrast);
							OLED_PutStr(5, 37, Main_Menu[7][settings.language], 0, settings.contrast);
							OLED_PutStr(1, frame.position[frame.level]*9 + 10, ">", 0, settings.contrast);
							break;

						case SETTINGS_SUBMENU_TIME:				//czas
							frame.totalPositions = 4;
							OLED_PutStr(10, 0, Settings[6][settings.language], 0, settings.contrast);			//tytul okna (ustaw czas)
							OLED_PutStr(5, 10, Settings[10][settings.language], 0, settings.contrast);
							OLED_PutStr(5, 19, Settings[11][settings.language], 0, settings.contrast);
							OLED_PutStr(5, 28, Settings[12][settings.language], 0, settings.contrast);
							OLED_PutStr(5, 37, Main_Menu[7][settings.language], 0, settings.contrast);
							OLED_PutStr(1, frame.position[frame.level]*9 + 10, ">", 0, settings.contrast);
							break;

						case SETTINGS_SUBMENU_LANGUAGE:				//jezyk
							frame.totalPositions = 3;
							OLED_PutStr(10, 0, Settings[4][settings.language], 0, settings.contrast);			//tytul okna (sposob wyswietlania)
							OLED_PutStr(5, 10, Languages[0], 0, settings.contrast);
							OLED_PutStr(5, 19, Languages[1], 0, settings.contrast);
							OLED_PutStr(5, 28, Main_Menu[7][settings.language], 0, settings.contrast);
							OLED_PutStr(1, frame.position[frame.level]*9 + 10, ">", 0, settings.contrast);
							break;
					}
				}

				if (frame.position[0] == MAIN_MENU_ARCHIVE) //jesli w podmenu archiwum
				{
					switch (frame.position[1])	//sprawdz ktore podmenu menu archiwum zostalo wybrane
					{
						case ARCHIVE_SUBMENU_BROWSE:
							//exit_main_menu();
							break;

						case ARCHIVE_SUBMENU_STORAGE_MODE:
							frame.totalPositions = 3;
							OLED_PutStr(10, 0, Archive[1][settings.language], 0, settings.contrast);			//tytul okna (sposob wyswietlania)
							OLED_PutStr(5, 10, Archive[4][settings.language], 0, settings.contrast);
							OLED_PutStr(5, 19, Archive[5][settings.language], 0, settings.contrast);
							OLED_PutStr(5, 28, Main_Menu[7][settings.language], 0, settings.contrast);
							OLED_PutStr(1, frame.position[frame.level]*9 + 10, ">", 0, settings.contrast);
							break;

						case ARCHIVE_SUBMENU_ERASE:
							//exit_main_menu();
							break;
					}
				}

			}
			if (frame.level == 3)			//jezeli w podmenu daty/czasu
			{
				if (frame.position[1] == SETTINGS_SUBMENU_DATE)	//jezeli w podmenu zmiany daty
				{
					switch (frame.position[2])	//sprawdz zmiana ktorego parametru zostala wybrana (dnia/miesiaca/roku)
					{
						case DATE_YEAR_INPUT_SCREEN:				//rok
							 frame.valueMin = 2020;
							 frame.valueMax = 2099;
							 sprintf(frame.firstLine, Settings[5][settings.language]);
							 sprintf(frame.secondLine, Settings[9][settings.language]);
							 sprintf(frame.thirdLine, "MIN   %d", frame.valueMin);
							 frame.value = gDate.Year + 2000;
							 sprintf(frame.bottomLine, "MAX   %d", frame.valueMax);
							 OLED_CLEAR();
							 frame.type = FRAME_SET_VALUE;
							break;

						case DATE_MONTH_INPUT_SCREEN:				//miesiac
							 frame.valueMin = 1;
							 frame.valueMax = 12;
							 sprintf(frame.firstLine, Settings[5][settings.language]);
							 sprintf(frame.secondLine, Settings[8][settings.language]);
						     sprintf(frame.thirdLine, "MIN   %d", frame.valueMin);
							 frame.value = gDate.Month;
							 sprintf(frame.bottomLine, "MAX   %d", frame.valueMax);
							 OLED_CLEAR();
							 frame.type = FRAME_SET_VALUE;

							break;

						case DATE_DAY_INPUT_SCREEN:				//dzien
							 frame.valueMin = 1;
							 frame.valueMax = No_of_days_a_month(gDate.Month);
							 sprintf(frame.firstLine, Settings[5][settings.language]);
							 sprintf(frame.secondLine, Settings[7][settings.language]);
						     sprintf(frame.thirdLine, "MIN   %d", frame.valueMin);
							 frame.value = gDate.Date;
							 sprintf(frame.bottomLine, "MAX   %d", frame.valueMax);
							 OLED_CLEAR();
							 frame.type = FRAME_SET_VALUE;
							break;

					}
				}

				if (frame.position[1] == SETTINGS_SUBMENU_TIME)	//jezeli w podmenu zmiany czasu
				{
					switch (frame.position[2])	//sprawdz zmiana ktorego parametru zostala wybrana (godziny/minuty/sekundy)
					{
						case TIME_HOUR_INPUT_SCREEN:				//godzina
							 frame.valueMin = 0;
							 frame.valueMax = 23;
							 sprintf(frame.firstLine, Settings[6][settings.language]);
							 sprintf(frame.secondLine, Settings[10][settings.language]);
						     sprintf(frame.thirdLine, "MIN   %d", frame.valueMin);
							 frame.value = gTime.Hours;
							 sprintf(frame.bottomLine, "MAX   %d", frame.valueMax);
							 OLED_CLEAR();
							 frame.type = FRAME_SET_VALUE;
							break;

						case TIME_MINUTE_INPUT_SCREEN:				//minuta
							 frame.valueMin = 0;
							 frame.valueMax = 59;
							 sprintf(frame.firstLine,Settings[6][settings.language]);
							 sprintf(frame.secondLine, Settings[11][settings.language]);
						     sprintf(frame.thirdLine, "MIN   %d", frame.valueMin);
							 frame.value = gTime.Minutes;
							 sprintf(frame.bottomLine, "MAX   %d", frame.valueMax);
							 OLED_CLEAR();
							 frame.type = FRAME_SET_VALUE;

							break;

						case TIME_SECOND_INPUT_SCREEN:				//sekunda
							 frame.valueMin = 0;
							 frame.valueMax = 59;
							 sprintf(frame.firstLine,Settings[6][settings.language]);
							 sprintf(frame.secondLine, Settings[12][settings.language]);
						     sprintf(frame.thirdLine, "MIN   %d", frame.valueMin);
							 frame.value = gTime.Seconds;
							 sprintf(frame.bottomLine, "MAX   %d", frame.valueMax);
							 OLED_CLEAR();
							 frame.type = FRAME_SET_VALUE;
							break;

					}
				}


			}
	}
}

void main_menu()
{
	frame.level = 0;
	while(1)
	{
		kbd_event_handler(CONTEXT_MAIN_MENU);
		drawFrame();
		if (frame.type == FRAME_EXIT) break;
	}
}

void key_down_callback(uint8_t key, uint8_t context)
{
	//if (DEVICE_IDLE) reset_screen_saver();
	reset_screen_saver();

	if (key == KEY_OFF)
	{
		key = KEY_NONE;
		//UART_print("aa\r\n");
		HAL_GPIO_WritePin(GPIOB, POWER_OFF_Pin, GPIO_PIN_SET);

	}

	switch (context)
	{
		case CONTEXT_MAIN_SCREEN:
			 if (key == KEY_PROGRAM)
			 {
				 key = KEY_NONE;

				 if (pomiarStatus == POMIAR_DISP || pomiarStatus == POMIAR_STOP)
				 {
					 OLED_CLEAR();
					 frame.type = FRAME_MAIN_MENU;
					 reset_screen_saver();
					 main_menu();
				 }
			 }

			 if (key == KEY_LEFT)
			 {
				 key = KEY_NONE;
				 if (settings.contrast > SS_CONTRAST)
				 {
					 settings.contrast--;
					 EEPROM_write(EEPROM_CONTRAST_ADDR, &settings.contrast);
				 }
			 }

			 if (key == KEY_RIGHT)
			 {
				 key = KEY_NONE;
				 if (settings.contrast < MAX_CONTRAST)
				 {
					 settings.contrast++;
					 EEPROM_write(EEPROM_CONTRAST_ADDR, &settings.contrast);
				 }
			 }

			 if (key == KEY_UP)
			 {
				 key = KEY_NONE;

				 if (settings.measurementType == MEASUREMENT_SINGLE && !NO_SAMPLE)
				 {
					 if (pomiarStatus == POMIAR_DISP || pomiarStatus == POMIAR_STOP)
					 {
						 newArchiveEntryAdded = 0;
						 pomiarStatus = POMIAR_START;
					 }
				 }
			 }

			 if (key == KEY_DOWN)
			 {
				 key = KEY_NONE;

				 if (!NO_SAMPLE)		//dodaj wpis do archiwum
				 {
					 if (settings.measurementType == MEASUREMENT_SINGLE)
					 {
						 if (pomiarStatus == POMIAR_DISP && newArchiveEntryAdded == 0 && settings.storageMode == STORAGE_BY_KEYBOARD)
						 {
							 newArchiveEntryAdded = 1;
							 ArchiveEntry.second = gTime.Seconds;
							 ArchiveEntry.minute = gTime.Minutes;
							 ArchiveEntry.hour = gTime.Hours;
							 ArchiveEntry.day = gDate.Date;
							 ArchiveEntry.month = gDate.Month;
							 ArchiveEntry.year = gDate.Year;
							 ArchiveEntry.nc = PomiarBuffer.nc;
							 ArchiveEntry.nck = PomiarBuffer.nck;
							 ArchiveEntry.brix = PomiarBuffer.brix;
							 ArchiveEntry.brixk = PomiarBuffer.brixk;
							 ArchiveEntry.temperature = PomiarBuffer.temperatura;
							 EEPROM_add_archive_entry(&ArchiveEntry, sizeof(ArchiveEntry));
						 }
					 }
					 if (settings.measurementType == MEASUREMENT_CONTINUOUS && newArchiveEntryAdded == 0 && settings.storageMode == STORAGE_BY_KEYBOARD)
					 {
						 newArchiveEntryAdded = 1;
						 ArchiveEntry.second = gTime.Seconds;
						 ArchiveEntry.minute = gTime.Minutes;
						 ArchiveEntry.hour = gTime.Hours;
						 ArchiveEntry.day = gDate.Date;
						 ArchiveEntry.month = gDate.Month;
						 ArchiveEntry.year = gDate.Year;
						 ArchiveEntry.nc = pomiar.nc;
						 ArchiveEntry.nck = pomiar.nck;
						 ArchiveEntry.brix = pomiar.brix;
						 ArchiveEntry.brixk = pomiar.brixk;
						 ArchiveEntry.temperature = pomiar.temperatura;
						 EEPROM_add_archive_entry(&ArchiveEntry, sizeof(ArchiveEntry));
					 }
				 }
			 }
			break;


		case CONTEXT_MAIN_MENU:
			 if(frame.type == FRAME_ARCHIVE_BROWSE)
			 {
				 if (key == KEY_LEFT)
				 {
					 key = KEY_NONE;

					 if (frame.value == 0) frame.value = frame.valueMax;
					 else frame.value--;
					 EEPROM_read_archive_entry(frame.value, &ArchiveEntry, sizeof(ArchiveEntry));

				 }

				 if (key == KEY_RIGHT)
				 {
					 key = KEY_NONE;

					 if (frame.value == frame.valueMax) frame.value = frame.valueMin;
					 else frame.value++;
					 EEPROM_read_archive_entry(frame.value, &ArchiveEntry, sizeof(ArchiveEntry));
				 }

				 if (key == KEY_PROGRAM)
				 {
					 key = KEY_NONE;

					 reset_screen_saver();
					 frame.type = FRAME_MAIN_MENU;
					 OLED_CLEAR();
				 }
			 }

			 if(frame.type == FRAME_SET_VALUE)
			 {
				 if (key == KEY_LEFT)
				 {
					 key = KEY_NONE;

					 if (frame.value == 0) frame.value = frame.valueMax;
					 else frame.value--;
				 }

				 if (key == KEY_RIGHT)
				 {
					 key = KEY_NONE;

					 if (frame.value == 0xFF) frame.value = frame.valueMin;
					 else frame.value++;
				 }
			 }

			 if (key == KEY_UP)
			 {
				 key = KEY_NONE;

				 OLED_PutStr(1, frame.position[frame.level]*9 + 10, " ", 0, settings.contrast);
				 frame.position[frame.level]--;
				 if (frame.position[frame.level] == 255) frame.position[frame.level] = frame.totalPositions - 1;
			 }

			 if (key == KEY_DOWN)
			 {
				 key = KEY_NONE;

				 OLED_PutStr(1, frame.position[frame.level]*9 + 10, " ", 0, settings.contrast);
				 frame.position[frame.level]++;
				 if (frame.position[frame.level] > frame.totalPositions - 1) frame.position[frame.level] = 0;
			 }

			 if (key == KEY_PROGRAM)
			 {
				 key = KEY_NONE;

				 OLED_PutStr(1, frame.position[frame.level]*9 + 10, " ", 0, settings.contrast);


				 if (frame.level == 0) frame.level = 1;	//jezeli otwarte menu glowne i zaznaczony element to go wybierz
				 else if (frame.level == 1)			//sprawdz czy otwarte podmenu menu glownego
				 {

					 switch (MAIN_MENU_POSITION)		//sprawdz ktore
					 {
						 case MAIN_MENU_MEASUREMENT_TYPE:					//otwarte podmenu rodzaj pomiaru

							 switch (frame.position[1])							//obsluz wybor
							 {
								 case MEASUREMENT_SINGLE:
									 settings.measurementType = MEASUREMENT_SINGLE;
									 EEPROM_write(EEPROM_MEASUREMENT_TYPE_ADDR, &settings.measurementType);
									 exit_main_menu();
									 break;

								 case MEASUREMENT_CONTINUOUS:
									 settings.measurementType = MEASUREMENT_CONTINUOUS;
									 EEPROM_write(EEPROM_MEASUREMENT_TYPE_ADDR, &settings.measurementType);
									 pomiarStatus = POMIAR_STOP;
									 exit_main_menu();
									 break;

								 case 2:		//wstecz
									 MM_up_one_level();
									 break;
							 }
							 break;

						 case MAIN_MENU_MEASUREMENT_UNIT:					//sposob wyswietlania - jednostka pomiaru

								 switch (frame.position[1])							//obsluz wybor
								 {
									 case UNIT_BRIX:
										 settings.measurementUnit = UNIT_BRIX;
										 EEPROM_write(EEPROM_MEASUREMENT_UNIT_ADDR, &settings.measurementUnit);
										 exit_main_menu();
										 break;

									 case UNIT_RI:
										 settings.measurementUnit = UNIT_RI;
										 EEPROM_write(EEPROM_MEASUREMENT_UNIT_ADDR, &settings.measurementUnit);
										 exit_main_menu();
										 break;

									 case 2:		//wstecz
										 MM_up_one_level();
										 break;
								 }
								 break;

							 case MAIN_MENU_THERMAL_COMPENSATION:					//kompensacja temperatury

								 switch (frame.position[1])							//obsluz wybor
								 {
									 case OFF:
										 settings.thermalCompensation = OFF;
										 EEPROM_write(EEPROM_THERMAL_COMP_ADDR, &settings.thermalCompensation);
										 exit_main_menu();
										 break;

									 case ON:
										 settings.thermalCompensation = ON;
										 EEPROM_write(EEPROM_THERMAL_COMP_ADDR, &settings.thermalCompensation);
										 exit_main_menu();
										 break;

									 case 2:		//wstecz
										 MM_up_one_level();
										 break;
								 }
								 break;

								 case MAIN_MENU_SETTINGS:					//ustawienia

									 switch (frame.position[1])							//obsluz wybor
									 {
										 case SETTINGS_SUBMENU_SINGLE_MEASUREMENT_TIME:
											 frame.level = 2;
											 break;

										 case SETTINGS_SUBMENU_DATE:
											 frame.level = 2;
											 break;

										 case SETTINGS_SUBMENU_TIME:
											 frame.level = 2;
											 break;

										 case SETTINGS_SUBMENU_LANGUAGE:
											 frame.level = 2;
											 break;

										 case 4:		//wstecz
											 MM_up_one_level();
											 break;
									 }
									 break;

								 case MAIN_MENU_ARCHIVE:					//archiwum

									switch (frame.position[1])							//obsluz wybor
									{
										 case ARCHIVE_SUBMENU_BROWSE:
											 EEPROM_read_archive_entry(0,&ArchiveEntry,sizeof(ArchiveEntry));
											 frame.valueMin = 0;
											 frame.valueMax = settings.archiveEntriesTotal - 1;
											 frame.value = 0;
											 frame.type = FRAME_ARCHIVE_BROWSE;
											 OLED_CLEAR();
											 break;

										 case ARCHIVE_SUBMENU_STORAGE_MODE:
											 frame.level = 2;
											 break;

										 case ARCHIVE_SUBMENU_ERASE:
											 frame.type = FRAME_ARCHIVE_BEING_ERASED;
											 //exit_main_menu();
											 break;

										 case 3:		//wstecz
											 MM_up_one_level();
											 break;
									}
									break;

								 case MAIN_MENU_EXIT:					//wyjscie
									exit_main_menu();
									break;
					 }



				 }
				 else if (frame.level == 2)			//sprawdz czy otwarte podmenu ustawienia/archiwum
				 {
					 if (frame.position[0] == MAIN_MENU_SETTINGS)	//jesli otwarte ustawienia
					 {
						 switch (frame.position[1])		//sprawdz ktore podmenu ustawien otwarte
						 {
							 case SETTINGS_SUBMENU_SINGLE_MEASUREMENT_TIME:
								 settings.singleMeasurementTime = frame.value;
								 EEPROM_write(EEPROM_SINGLE_MEASUREMENT_TIME_ADDR, &settings.singleMeasurementTime);
								 exit_main_menu();
								 break;

							 case SETTINGS_SUBMENU_DATE:
								 switch (frame.position[2])
								 {
									 case DATE_YEAR_INPUT_SCREEN:
										 frame.level = 3;
										 break;

									 case DATE_MONTH_INPUT_SCREEN:
										 frame.level = 3;
										 break;

									 case DATE_DAY_INPUT_SCREEN:
										 frame.level = 3;
										 break;

									 case 3:		//wstecz
										 MM_up_one_level();
										 break;
								 }

								 break;

							 case SETTINGS_SUBMENU_TIME:
								 switch (frame.position[2])
								 {
									 case TIME_HOUR_INPUT_SCREEN:
										 frame.level = 3;
										 break;

									 case TIME_MINUTE_INPUT_SCREEN:
										 frame.level = 3;
										 break;

									 case TIME_SECOND_INPUT_SCREEN:
										 frame.level = 3;
										 break;

									 case 3:		//wstecz
										 MM_up_one_level();
										 break;
								 }
								 break;

							 case SETTINGS_SUBMENU_LANGUAGE:
								 switch (frame.position[2])
								 {
									 case POLISH:
										 settings.language = POLISH;
										 EEPROM_write(EEPROM_LANGUAGE_ADDR, &settings.language);
										 exit_main_menu();
										 break;

									 case ENGLISH:
										 settings.language = ENGLISH;
										 EEPROM_write(EEPROM_LANGUAGE_ADDR, &settings.language);
										 exit_main_menu();
										 break;

									 case 2:		//wstecz
										 MM_up_one_level();
										 break;
								 }

								 break;
						 }
					 }

					 if (frame.position[0] == MAIN_MENU_ARCHIVE)	//jesli otwarte archiwum
					 {
						 switch (frame.position[2])		//sprawdz ktory tryb zapisu do archiwum wybrany
						 {
							 case ARCHIVE_STORAGE_MODE_PROHIBITED:
								 settings.storageMode = ARCHIVE_STORAGE_MODE_PROHIBITED;
								 EEPROM_write(EEPROM_ARCHIVE_STORAGE_MODE_ADDR, &settings.storageMode);
								 exit_main_menu();
								 break;

							 case ARCHIVE_STORAGE_MODE_BY_KEYBOARD:
								 settings.storageMode = ARCHIVE_STORAGE_MODE_BY_KEYBOARD;
								 EEPROM_write(EEPROM_ARCHIVE_STORAGE_MODE_ADDR, &settings.storageMode);
								 exit_main_menu();
								 break;

							 case 2:		//wstecz
								 MM_up_one_level();
								 break;
						 }
					 }
				 }

				 else if (frame.level == 3)			//sprawdz czy otwarte podmenu ustawiania daty / godziny
				 {
					 if(frame.position[0] == MAIN_MENU_SETTINGS)
					 {
							if (frame.position[1] == SETTINGS_SUBMENU_DATE)	//jezeli w podmenu zmiany daty
							{
								 switch (frame.position[2])		//sprawdz zmiana ktorego parametru zostala wybrana (dnia/miesiaca/roku)
								 {

									 case DATE_DAY_INPUT_SCREEN:			//dzien
										 gDate.Date = frame.value;
										 HAL_RTC_SetDate(&hrtc, &gDate, RTC_FORMAT_BIN);
										 MM_up_one_level();
										 break;

									 case DATE_MONTH_INPUT_SCREEN:			//miesiac
										 gDate.Month = frame.value;
										 HAL_RTC_SetDate(&hrtc, &gDate, RTC_FORMAT_BIN);
										 MM_up_one_level();
										 break;

									 case DATE_YEAR_INPUT_SCREEN:			//rok
										 gDate.Year = frame.value - 2000;
										 HAL_RTC_SetDate(&hrtc, &gDate, RTC_FORMAT_BIN);
										 MM_up_one_level();
										 break;

								 }
							}

							if (frame.position[1] == SETTINGS_SUBMENU_TIME)	//jezeli w podmenu zmiany czasu
							{
								 switch (frame.position[2])		//sprawdz zmiana ktorego parametru zostala wybrana (godziny/minuty/sekundy)
								 {

									 case TIME_HOUR_INPUT_SCREEN:			//godzina
										 gTime.Hours = frame.value;
										 HAL_RTC_SetTime(&hrtc, &gTime, RTC_FORMAT_BIN);
										 MM_up_one_level();
										 break;

									 case TIME_MINUTE_INPUT_SCREEN:			//minuta
										 gTime.Minutes = frame.value;
										 HAL_RTC_SetTime(&hrtc, &gTime, RTC_FORMAT_BIN);
										 MM_up_one_level();
										 break;

									 case TIME_SECOND_INPUT_SCREEN:			//sekunda
										 gTime.Seconds = frame.value;
										 HAL_RTC_SetTime(&hrtc, &gTime, RTC_FORMAT_BIN);
										 MM_up_one_level();
										 break;
								 }
							}

					 }

				 }
				 OLED_CLEAR();
			 }
			break;
	}
}


void USER_UART2_IDLE_Callback(UART_HandleTypeDef *huart)			// PC -> MBD
{
	HAL_UART_DMAStop(&huart2);
	huart6.RxXferCount = 0;
	uint8_t buffer[27] = {0};

	uint16_t data_length  = CCD_RX_BUFFER_SIZE - __HAL_DMA_GET_COUNTER(&hdma_usart2_rx);

	if (__HAL_UART_GET_FLAG(&huart2, UART_FLAG_ORE))
	{
		  __HAL_UART_CLEAR_OREFLAG(&huart2);
		  data_length = USB_RX_BUFFER_SIZE;
		  memset((uint8_t*)USB_rxBuffer, 0, data_length);  //passing argument 1 of 'memset' discards 'volatile' qualifier from pointer target type
	}													   //dma wyłączone więc nie powinno byc problemu dlatego wygaszony warning

	if (USB_rxBuffer[0] == '!')		//jesli przyjdzie zapytanie o pomiar wyslij ostatni pomiar
	{
		if (USB_rxBuffer[1] == 'P')
		{
			HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, 0);
			if (packetIsNew)
			{
				buffer[0] = '!';
				buffer[1] = 'P';
				memcpy(&buffer[2], (uint8_t*)&pomiar.brix, 4);
				memcpy(&buffer[6], (uint8_t*)&pomiar.brixk, 4);
				memcpy(&buffer[10], (uint8_t*)&pomiar.I_LED, 4);
				memcpy(&buffer[14], (uint8_t*)&pomiar.nc, 4);
				memcpy(&buffer[18], (uint8_t*)&pomiar.nck, 4);
				memcpy(&buffer[22], (uint8_t*)&pomiar.temperatura, 4);
				buffer[26] = CalcChecksum(buffer, 26);
				HAL_UART_Transmit_DMA(&huart2, buffer, sizeof(buffer));
				packetIsNew = 0;										//zapobiega wysylaniu tego samego pomiaru ponownie
			}
		}
		else if (USB_rxBuffer[1] == 'C')
		{
			if (USB_rxBuffer[20] == CalcChecksum(USB_rxBuffer, 20))
			{
				HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, 0);
				memcpy(&settings.serialNumber, (uint8_t*)&USB_rxBuffer[2], 2);
				serialNumberModified = 1;
				memcpy(&korekcja, (uint8_t*)&USB_rxBuffer[4], sizeof(korekcja));

				HAL_UART_Transmit_DMA(&huart6, (uint8_t*)USB_rxBuffer, data_length);	// przeslij do ccd
			}																			//passing argument discards 'volatile' qualifier from pointer target type
																						//dma wyłączone więc nie powinno byc problemu dlatego wygaszony warning
		}
	}

	//Zero Receiving Buffer
	//memset(USB_rxBuffer, 0, data_length);
	data_length = 0;

	//Restart to start DMA transmission of 255 bytes of data at a time
	HAL_UART_Receive_DMA(&huart2, (uint8_t*) USB_rxBuffer, USB_RX_BUFFER_SIZE);
}

void USER_UART6_IDLE_Callback(UART_HandleTypeDef *huart)		//CCD -> MBD
{
	//Stop this DMA transmission
	HAL_UART_DMAStop(&huart6);
	huart6.RxXferCount = 0;

	//Calculate the length of the received data
	uint16_t data_length  = CCD_RX_BUFFER_SIZE - __HAL_DMA_GET_COUNTER(&hdma_usart6_rx);

	if (__HAL_UART_GET_FLAG(&huart6, UART_FLAG_ORE))
	{
		  __HAL_UART_CLEAR_OREFLAG(&huart6);
		  data_length = CCD_RX_BUFFER_SIZE;
		  memset((uint8_t*)CCD_rxBuffer, 0, data_length);   //passing argument 1 of 'memset' discards 'volatile' qualifier from pointer target type
	}														//dma wyłączone więc nie powinno byc problemu dlatego wygaszony warning
															//dma wyłączone więc nie powinno byc problemu dlatego wygaszony warning

	if (CCD_rxBuffer[0] == '!' && CCD_rxBuffer[1] == 'M' && data_length == 45)	//only consider packets that start with proper header and are correct length
	{
		HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, 0);

		if (CCD_rxBuffer[44] == CalcChecksum(CCD_rxBuffer, 44))	//if CRC correct
		{
			if (TicksSinceLastRecordArrived < 50)	//if sample present
			{

																										//sample detection:
				memcpy((uint8_t*)&pomiarArray[packetCounter].brix, (uint8_t*)&CCD_rxBuffer[2], 24);		//copy received packet to array for later averaging
				packetCounter++;
				if (packetCounter == 2 && !pomiarArrayFull)												//assume that 2 packets in a row means that everything is going fine
				{																						//all measurements are copied to final pomiar struct before averaging
					pomiar.temperatura = pomiarArray[packetCounter].temperatura;						//it is done so in order to trigger stripes animation without changing old code.
					pomiar.nc = pomiarArray[packetCounter].nc;											//stripes animation takes 4+ seconds so by the time it is finished
					pomiar.nck = pomiarArray[packetCounter].nck;										//the proper averaged value will have been displayed anyway
					pomiar.brix = pomiarArray[packetCounter].brix;
					pomiar.brixk = pomiarArray[packetCounter].brixk;
					reset_screen_saver();
					if (settings.measurementType == MEASUREMENT_SINGLE) pomiarStatus = POMIAR_START;	//po wykryciu probki od razu uruchom pomiar pojedynczy
				}
				if (pomiarArrayFull && newPacketCount >= 4)	// jesli zebrane pomiary z ostatnich 3 sekund, co 8 pakiet (1 sekunde) licz srednia
				{
					//HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, 0);	//zielony led pokaze jak czesto obliczana jest srednia
					newPacketCount = 0;
					memset(&pomiar, 0, sizeof(pomiar));
					for(uint8_t i = 0; i < POMIAR_ARRAY_SIZE; i++)						//final averaged values calculation
					{
						pomiar.nc += pomiarArray[i].nc;
						pomiar.nck += pomiarArray[i].nck;
						pomiar.brix += pomiarArray[i].brix;
						pomiar.brixk += pomiarArray[i].brixk;
						pomiar.temperatura += pomiarArray[i].temperatura;
						pomiar.I_LED += pomiarArray[i].I_LED;
					}
					//pomiar.I_LED = pomiarArray[packetCounter].I_LED;
					pomiar.I_LED /= POMIAR_ARRAY_SIZE;
					pomiar.temperatura /= POMIAR_ARRAY_SIZE;
					pomiar.nc /= POMIAR_ARRAY_SIZE;
					pomiar.nck /= POMIAR_ARRAY_SIZE;
					pomiar.brix /= POMIAR_ARRAY_SIZE;
					pomiar.brixk /= POMIAR_ARRAY_SIZE;
					packetIsNew = 1;
				}
				if (packetCounter >= POMIAR_ARRAY_SIZE)
				{
					packetCounter = 0;
					pomiarArrayFull = 1;
				}
				newPacketCount++;	//counts new packets received
			}
		}

	}
	else	// USB <-> CCD bridge
	{
		HAL_UART_Transmit_DMA(&huart2, (uint8_t*)CCD_rxBuffer, data_length);	//passing argument 1 of 'memset' discards 'volatile' qualifier from pointer target type
	}																			//dma wyłączone więc nie powinno byc problemu dlatego wygaszony warning

	//Zero Receiving Buffer
	//memset(rxBuffer, 0, data_length);
	data_length = 0;

	//Restart to start DMA transmission of 255 bytes of data at a time
	HAL_UART_Receive_DMA(&huart6, (uint8_t*) CCD_rxBuffer, CCD_RX_BUFFER_SIZE);
	TicksSinceLastRecordArrived = 0;
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_SPI1_Init();
  MX_SPI2_Init();
  MX_USART2_UART_Init();
  MX_USART6_UART_Init();
  MX_TIM6_Init();
  MX_RTC_Init();
  /* USER CODE BEGIN 2 */

  HAL_GPIO_WritePin(OLED_OFF_GPIO_Port, OLED_OFF_Pin, 0);
  HAL_GPIO_WritePin(POWER_OFF_GPIO_Port, POWER_OFF_Pin, 0);
  HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, 1);
  HAL_GPIO_WritePin(K1_OUT_GPIO_Port, K1_OUT_Pin, 1);	// wlacza obsluge klawiszy
  HAL_GPIO_WritePin(K2_OUT_GPIO_Port, K2_OUT_Pin, 0);	// wlacza obsluge klawiszy
  HAL_TIM_Base_Start_IT(&htim6);						//aktywuje przerwanie od timera. wylaczone bo wykorzystane przerwanie z alarmu RTC
														 //w ustawieniach alarmu trzeba zmienic -> sAlarm.AlarmMask = RTC_ALARMMASK_ALL;

  //erase_settings();

  OLED_CLEAR();
  OLED_Init();
  OLED_CLEAR();

//Logo firmy
  settings.contrast = 0x01;		//zmodyfikowałem wyswietlanie bitmap zeby zeby mozna bylo regulowac kontrast przy wyswietlaniu kreski nad O w BRAK PROBKI
  OLED_bitmap(0, 5, gr_00);
  settings.contrast = DEFAULT_CONTRAST;
  HAL_Delay(1000);

//zczytanie ustawien
  EEPROM_readSettings(EEPROM_SETTINGS_ADDRESS, &settings, sizeof(settings));
  register_on_key_press_callback(key_down_callback);
  //settings_debug();

  verify_settings();
  fetch_date_time();
  __HAL_UART_ENABLE_IT(&huart6, UART_IT_IDLE);

  ssStateChanged = 1;

  //HAL_Delay(2000);

  OLED_CLEAR();

// Nr seryjny i wersja softu
  OLED_PutStr(3, 14, "MS REF090L", 1, 0x09);
  sprintf(frame.buffer,"SN: %d", settings.serialNumber);
  OLED_PutStr(3, 32, frame.buffer, 0, 0x09);
  sprintf(frame.buffer,"SW: %s", SOFTWARE_VERSION);
  OLED_PutStr(3, 42, frame.buffer, 0, 0x09);
  HAL_Delay(1000);

  OLED_CLEAR();
  OLED_PutStr(5, 24, "AUTO TEST", 1, 0x09);

  __HAL_DMA_DISABLE_IT(&hdma_usart6_rx, DMA_IT_HT);
  __HAL_DMA_DISABLE_IT(&hdma_usart2_rx, DMA_IT_HT);

  HAL_UART_Receive_DMA(&huart6, (uint8_t*)CCD_rxBuffer, CCD_RX_BUFFER_SIZE);
  HAL_UART_Receive_DMA(&huart2, (uint8_t*)USB_rxBuffer, USB_RX_BUFFER_SIZE);
  TicksSinceLastRecordArrived = 0;
  //HAL_Delay(5000);
  OLED_CLEAR();
  char kreski[10] = {0};	//kreski po wykryciu próbki
  __HAL_UART_ENABLE_IT(&huart2, UART_IT_IDLE);
  initDone = 1;

  /////////////////////////
//  korekcja.D = 0.000040;
//  korekcja.f = 77.820;
//  korekcja.tempKor = 0.10;
//  korekcja.xKor = 109.60;
////  serialNumberModified = 1
//  volatile uint8_t test[sizeof(korekcja_pakiet) + 1];
//
//  korekcja_pakiet.header[0] = '!';
//  korekcja_pakiet.header[1] = 'C';
//  korekcja_pakiet.D = 0.000040;
//  korekcja_pakiet.f = 77.820;
//  korekcja_pakiet.tempKor = 0.10;
//  korekcja_pakiet.xKor = 109.60;
//  korekcja_pakiet.serialNumber = 1;
//  memcpy(&test, (uint8_t*)&korekcja_pakiet, sizeof(korekcja_pakiet));
//  test[sizeof(korekcja_pakiet)] = CalcChecksum(test,sizeof(test)-1);


//memcpy(&test, (uint8_t*)&korekcja, sizeof(korekcja));

  ////////////////////////


  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
	  if (serialNumberModified)
	  {
		  EEPROM_write_multiple(EEPROM_SERIAL_NUMBER_ADDR, (uint8_t*)&settings.serialNumber, sizeof(settings.serialNumber));
		  serialNumberModified = 0;
		  ///////////////
//		  char buf[25];
//		  OLED_CLEAR();
//		  sprintf(buf, "SN: %d", settings.serialNumber);
//		  OLED_PutStr(15, 10, buf, 0, 0x09);
//		  sprintf(buf, "f=%f", korekcja.f);
//		  OLED_PutStr(15, 20, buf, 0, 0x09);
//		  sprintf(buf, "D=%f", korekcja.D);
//		  OLED_PutStr(15, 30, buf, 0, 0x09);
//		  sprintf(buf, "t=%f", korekcja.tempKor);
//		  OLED_PutStr(15, 40, buf, 0, 0x09);
//		  sprintf(buf, "x=%f", korekcja.xKor);
//		  OLED_PutStr(15, 50, buf, 0, 0x09);
//		  HAL_Delay(5000);
//		  OLED_CLEAR();

		  ///////////////
	  }


	 if (NO_SAMPLE)
	 {
		 //if (settings.measurementType == MEASUREMENT_SINGLE) pomiarStatus = POMIAR_STOP;
		 if (settings.measurementType == MEASUREMENT_SINGLE) pomiarStatus = POMIAR_START;

		 sampleDetected = 0;						//wykyrwanie probki w celu odpalenia naswietlania i odczekania az pomiar bedzie gotowy
		 if (frame.type != FRAME_BP)
		 {
			 reset_screen_saver();
			 //UART_print("BP\r\n");
			 memset(&kreski, 0, sizeof(kreski));

			 OLED_CLEAR();
			 xShift = 0;
			 yShift = 0;
			 frame.type = FRAME_BP;
		 }
	 }
	 else
	 {
		 if (settings.measurementType == MEASUREMENT_CONTINUOUS)		//wyswietlaj kreski tylko dla pomiaru ciaglego
		 {
			 if (sampleDetected == 0) sampleDetected = 1;
			 if (sampleDetected == 1)
			 {
				 //SendRecord(R_AUTOLED, A_HEAD, A_MAINBOARD, NULL, 0);       // naświetlanie
				 //UART_print("naswietlanie\r\n");
				 reset_screen_saver();
				 ledTimer = SAMPLE_DETECT_TIMEOUT;
				 sampleDetected = 2;
				 OLED_CLEAR();
			 }
			 if (sampleDetected == 2 && onesecondTick == 1)			//wyswietlanie kreseczek jak oczekiwanie na pomiar
			 {
				 //UART_print("tick\r\n");
				 onesecondTick = 0;
				 if (frame.type != FRAME_2_BIG_LINES)
				 {
					 //char a[10];
					 sprintf(kreski,"%sIII",kreski);
					 sprintf(frame.secondLine, "%s", kreski);
					 frame.type = FRAME_1_BIG_LINE;
				 }
			 }
		 }
		 else
		 {
			 sampleDetected = 3;
			 //pomiarStatus = POMIAR_STOP;
		 }

		 if (sampleDetected == 3)								//wyswietlenie pomiaru po kreskach
		 {
			 if (settings.measurementType == MEASUREMENT_CONTINUOUS)
			 {

					 if (settings.measurementUnit == UNIT_BRIX)
					 {
						 if (settings.thermalCompensation == OFF)
						 {
							if (pomiar.brix >= 10) sprintf(frame.secondLine,"  %2.2f %% ", pomiar.brix);
							else if(pomiar.brix >= 0) sprintf(frame.secondLine,"   %2.2f %% ", pomiar.brix);
							else if(pomiar.brix > -10) sprintf(frame.secondLine,"  %2.2f %% ", pomiar.brix);
							else if(pomiar.brix <= -10) sprintf(frame.secondLine," %2.2f %% ", pomiar.brix);
							sprintf(frame.thirdLine,"  %2.2f \x7B\x43", pomiar.temperatura);
						 }
						 else
						 {
							if (pomiar.brixk >= 10) sprintf(frame.secondLine,"  %2.2f %% ", pomiar.brixk);
							else if(pomiar.brixk >= 0) sprintf(frame.secondLine,"   %2.2f %% ", pomiar.brixk);
							else if(pomiar.brixk > -10) sprintf(frame.secondLine,"  %2.2f %% ", pomiar.brixk);
							else if(pomiar.brixk <= -10) sprintf(frame.secondLine," %2.2f %% ", pomiar.brixk);
							//sprintf(frame.thirdLine,"  %d.00 \x7B\x43", 20);
							sprintf(frame.thirdLine,"  %2.2f \x7B\x43", pomiar.temperatura);
						 }

						 if (frame.type != FRAME_2_BIG_LINES)
						 {
							OLED_CLEAR();
							frame.type = FRAME_2_BIG_LINES;
						 }
					 }

					 if (settings.measurementUnit == UNIT_RI)
					 {
						 if (settings.thermalCompensation == OFF)
						 {
							 sprintf(frame.secondLine,"%1.5f nd", pomiar.nc);
							 sprintf(frame.thirdLine,"  %2.2f \x7B\x43", pomiar.temperatura);
						 }
						 else
						 {
							 sprintf(frame.secondLine,"%1.5f nd", pomiar.nck);
							 //sprintf(frame.thirdLine,"  %d.00 \x7B\x43", 20);
							 sprintf(frame.thirdLine,"  %2.2f \x7B\x43", pomiar.temperatura);
						 }
						 if (frame.type != FRAME_2_BIG_LINES)
						 {
						  OLED_CLEAR();
						  frame.type = FRAME_2_BIG_LINES;
						 }
					 }

			 }
			 if (settings.measurementType == MEASUREMENT_SINGLE)
			 {

				 if (NO_SAMPLE)
				 {
					 if (frame.type != FRAME_BP)
					 {
						 reset_screen_saver();
						 OLED_CLEAR();
						 xShift = 0;
						 yShift = 0;
						 pomiarStatus = POMIAR_STOP;
						 frame.type = FRAME_BP;
					 }
				 }
				 else if (frame.type != FRAME_2_BIG_LINES && pomiarStatus == POMIAR_STOP)
				 {
					OLED_CLEAR();
					if (settings.language == ENGLISH)		//warunek zeby ladnie wyswietlac napis "NACISNIJ START"
					{
						sprintf(frame.secondLine, "   %s",  Main_Screen[6][settings.language]);
						sprintf(frame.thirdLine,"   START");
						//nacisnijStart = 0;
					}
					else
					{
						//sprintf(frame.secondLine, " %s",  Main_Screen[6][settings.language]);
						nacisnijStart = 1;
						sprintf(frame.thirdLine,"START");
					}

					frame.type = FRAME_2_BIG_LINES;
				 }


				 if (pomiarStatus == POMIAR_START && pomiarTimer == 0)
				 {
					OLED_CLEAR();
					nacisnijStart = 0;
					OLED_PutStr(10, 1, "    ", 0, settings.contrast);	//usun napis AXXX jesli poprzedni pomiar byl zapisywany
					pomiarTimer = settings.singleMeasurementTime;
					pomiarStatus = POMIAR_PENDING;
				 }

				 if (pomiarStatus == POMIAR_PENDING)
				 {
					reset_screen_saver();
					sprintf(frame.secondLine,"  %s", Main_Screen[4][settings.language]);
					sprintf(frame.thirdLine,"  %d %s ",pomiarTimer, Main_Screen[5][settings.language]);
					if (frame.type != FRAME_2_BIG_LINES)
					{
					OLED_CLEAR();
					frame.type = FRAME_2_BIG_LINES;
					}

				 }

				 if (pomiarStatus == POMIAR_DONE)
				 {
					 newArchiveEntryAdded = 0;
					 PomiarBuffer.nc = pomiar.nc;					// bufor dodany w razie jakby ktos chcial zmieniac ustawienia/lazic po archiwum gdy wyswietlany jest wynik.
					 PomiarBuffer.nck = pomiar.nck;
					 PomiarBuffer.brix = pomiar.brix;
					 PomiarBuffer.brixk = pomiar.brixk;
					 PomiarBuffer.temperatura = pomiar.temperatura;
					 pomiarTimer = 0;
					 pomiarStatus = POMIAR_DISP;
					 OLED_CLEAR();
				 }

				 if (pomiarStatus == POMIAR_DISP)
				 {
					 if (settings.measurementUnit == UNIT_BRIX)
					 {
						 if (settings.thermalCompensation == OFF)
						 {
							if (PomiarBuffer.brix >= 10) sprintf(frame.secondLine,"  %2.2f %% ", PomiarBuffer.brix);
							else if(PomiarBuffer.brix >= 0) sprintf(frame.secondLine,"   %2.2f %% ", PomiarBuffer.brix);
							else if(PomiarBuffer.brix > -10) sprintf(frame.secondLine,"  %2.2f %% ", PomiarBuffer.brix);
							else if(PomiarBuffer.brix <= -10) sprintf(frame.secondLine," %2.2f %% ", PomiarBuffer.brix);

							sprintf(frame.thirdLine,"  %2.2f \x7B\x43", PomiarBuffer.temperatura);
						 }
						 else
						 {
							if (PomiarBuffer.brixk >= 10) sprintf(frame.secondLine,"  %2.2f %% ", PomiarBuffer.brixk);
							else if(PomiarBuffer.brixk >= 0) sprintf(frame.secondLine,"   %2.2f %% ", PomiarBuffer.brixk);
							else if(PomiarBuffer.brixk > -10) sprintf(frame.secondLine,"  %2.2f %% ", PomiarBuffer.brixk);
							else if(PomiarBuffer.brixk <= -10) sprintf(frame.secondLine," %2.2f %% ", PomiarBuffer.brixk);

							//sprintf(frame.thirdLine,"  %d.00 \x7B\x43", 20);
							sprintf(frame.thirdLine,"  %2.2f \x7B\x43", PomiarBuffer.temperatura);
						 }

						 if (frame.type != FRAME_2_BIG_LINES)
						 {
							OLED_CLEAR();
							frame.type = FRAME_2_BIG_LINES;
						 }
					 }

					 if (settings.measurementUnit == UNIT_RI)
					 {
						 if (settings.thermalCompensation == OFF)
						 {
							 sprintf(frame.secondLine,"%1.5f nd", PomiarBuffer.nc);
							 sprintf(frame.thirdLine,"  %2.2f \x7B\x43", PomiarBuffer.temperatura);
						 }
						 else
						 {
							 sprintf(frame.secondLine,"%1.5f nd", PomiarBuffer.nck);
							 //sprintf(frame.thirdLine,"  %d.00 \x7B\x43", 20);
							 sprintf(frame.thirdLine,"  %2.2f \x7B\x43", PomiarBuffer.temperatura);
						 }
						 if (frame.type != FRAME_2_BIG_LINES)
						 {
						  OLED_CLEAR();
						  frame.type = FRAME_2_BIG_LINES;
						 }
					 }
				 }
			 }

		 }
	 }
	 kbd_event_handler(CONTEXT_MAIN_SCREEN);
	 drawFrame();
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

  /** Configure the main internal regulator output voltage 
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE|RCC_OSCILLATORTYPE_LSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 6;
  RCC_OscInitStruct.PLL.PLLN = 96;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  RCC_OscInitStruct.PLL.PLLR = 2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV2;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_RTC;
  PeriphClkInitStruct.RTCClockSelection = RCC_RTCCLKSOURCE_LSE;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief RTC Initialization Function
  * @param None
  * @retval None
  */
static void MX_RTC_Init(void)
{

  /* USER CODE BEGIN RTC_Init 0 */

  /* USER CODE END RTC_Init 0 */

//  RTC_TimeTypeDef sTime = {0};
//  RTC_DateTypeDef sDate = {0};
  RTC_AlarmTypeDef sAlarm = {0};

  /* USER CODE BEGIN RTC_Init 1 */

  /* USER CODE END RTC_Init 1 */
  /** Initialize RTC Only 
  */
  hrtc.Instance = RTC;
  hrtc.Init.HourFormat = RTC_HOURFORMAT_24;
  hrtc.Init.AsynchPrediv = 127;
  hrtc.Init.SynchPrediv = 255;
  hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
  hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
  hrtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
  if (HAL_RTC_Init(&hrtc) != HAL_OK)
  {
    Error_Handler();
  }

  /* USER CODE BEGIN Check_RTC_BKUP */
    
  /* USER CODE END Check_RTC_BKUP */

  /** Initialize RTC and set the Time and Date 
  */
//  sTime.Hours = 14;
//  sTime.Minutes = 29;
//  sTime.Seconds = 0;
//  sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
//  sTime.StoreOperation = RTC_STOREOPERATION_RESET;
//  if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN) != HAL_OK)
//  {
//    Error_Handler();
//  }
//  sDate.WeekDay = RTC_WEEKDAY_MONDAY;
//  sDate.Month = RTC_MONTH_JANUARY;
//  sDate.Date = 1;
//  sDate.Year = 0;
//
//  if (HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BIN) != HAL_OK)
//  {
//    Error_Handler();
//  }
  /** Enable the Alarm A 
  */
  sAlarm.AlarmTime.Hours = 0;
  sAlarm.AlarmTime.Minutes = 0;
  sAlarm.AlarmTime.Seconds = 0;
  sAlarm.AlarmTime.SubSeconds = 0;
  sAlarm.AlarmTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
  sAlarm.AlarmTime.StoreOperation = RTC_STOREOPERATION_RESET;
  sAlarm.AlarmMask = RTC_ALARMMASK_NONE;
  sAlarm.AlarmSubSecondMask = RTC_ALARMSUBSECONDMASK_ALL;
  sAlarm.AlarmDateWeekDaySel = RTC_ALARMDATEWEEKDAYSEL_DATE;
  sAlarm.AlarmDateWeekDay = 1;
  sAlarm.Alarm = RTC_ALARM_A;
  if (HAL_RTC_SetAlarm_IT(&hrtc, &sAlarm, RTC_FORMAT_BIN) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN RTC_Init 2 */
  sAlarm.AlarmMask = RTC_ALARMMASK_ALL;
  if (HAL_RTC_SetAlarm_IT(&hrtc, &sAlarm, RTC_FORMAT_BIN) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE END RTC_Init 2 */

}

/**
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_4;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

}

/**
  * @brief SPI2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI2_Init(void)
{

  /* USER CODE BEGIN SPI2_Init 0 */

  /* USER CODE END SPI2_Init 0 */

  /* USER CODE BEGIN SPI2_Init 1 */

  /* USER CODE END SPI2_Init 1 */
  /* SPI2 parameter configuration*/
  hspi2.Instance = SPI2;
  hspi2.Init.Mode = SPI_MODE_MASTER;
  hspi2.Init.Direction = SPI_DIRECTION_2LINES;
  hspi2.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi2.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi2.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi2.Init.NSS = SPI_NSS_SOFT;
  hspi2.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_128;
  hspi2.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi2.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi2.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi2.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI2_Init 2 */

  /* USER CODE END SPI2_Init 2 */

}

/**
  * @brief TIM6 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM6_Init(void)
{

  /* USER CODE BEGIN TIM6_Init 0 */

  /* USER CODE END TIM6_Init 0 */

  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM6_Init 1 */

  /* USER CODE END TIM6_Init 1 */
  htim6.Instance = TIM6;
  htim6.Init.Prescaler = 48000-1;
  htim6.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim6.Init.Period = 20-1;
  htim6.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim6) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim6, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM6_Init 2 */

  /* USER CODE END TIM6_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * @brief USART6 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART6_UART_Init(void)
{

  /* USER CODE BEGIN USART6_Init 0 */

  /* USER CODE END USART6_Init 0 */

  /* USER CODE BEGIN USART6_Init 1 */

  /* USER CODE END USART6_Init 1 */
  huart6.Instance = USART6;
  huart6.Init.BaudRate = 115200;
  huart6.Init.WordLength = UART_WORDLENGTH_8B;
  huart6.Init.StopBits = UART_STOPBITS_1;
  huart6.Init.Parity = UART_PARITY_NONE;
  huart6.Init.Mode = UART_MODE_TX_RX;
  huart6.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart6.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart6) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART6_Init 2 */

  /* USER CODE END USART6_Init 2 */

}

/** 
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void) 
{
  /* DMA controller clock enable */
  __HAL_RCC_DMA2_CLK_ENABLE();
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Stream5_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Stream5_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Stream5_IRQn);
  /* DMA1_Stream6_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Stream6_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Stream6_IRQn);
  /* DMA2_Stream1_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA2_Stream1_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA2_Stream1_IRQn);
  /* DMA2_Stream6_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA2_Stream6_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA2_Stream6_IRQn);

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOE, K1_OUT_Pin|K2_OUT_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, OLED_OFF_Pin|OLED_D_C_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, OLED_RESET_Pin|OLED_CS_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, LED1_Pin|LED2_Pin|SPI_CS_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(POWER_OFF_GPIO_Port, POWER_OFF_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(SPI_WP_GPIO_Port, SPI_WP_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : K1_OUT_Pin K2_OUT_Pin */
  GPIO_InitStruct.Pin = K1_OUT_Pin|K2_OUT_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  /*Configure GPIO pins : K3_Pin K4_Pin K5_Pin */
  GPIO_InitStruct.Pin = K3_Pin|K4_Pin|K5_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  /*Configure GPIO pin : OLED_OFF_Pin */
  GPIO_InitStruct.Pin = OLED_OFF_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(OLED_OFF_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : OLED_RESET_Pin OLED_D_C_Pin OLED_CS_Pin */
  GPIO_InitStruct.Pin = OLED_RESET_Pin|OLED_D_C_Pin|OLED_CS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : LED1_Pin LED2_Pin POWER_OFF_Pin */
  GPIO_InitStruct.Pin = LED1_Pin|LED2_Pin|POWER_OFF_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : SPI_CS_Pin */
  GPIO_InitStruct.Pin = SPI_CS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  HAL_GPIO_Init(SPI_CS_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : SPI_WP_Pin */
  GPIO_InitStruct.Pin = SPI_WP_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  HAL_GPIO_Init(SPI_WP_GPIO_Port, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim6)	//50Hz refresh rate for keyboard - Runs every 20ms
{

		HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, 1);
		HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, 1);


	TicksSinceLastRecordArrived++;
	if(TicksSinceLastRecordArrived > 50)
	{
		memset(pomiarArray, 0, sizeof(pomiarArray));
		packetCounter = 0;
		pomiarArrayFull = 0;
		newPacketCount = 0;
		memset(&pomiar, 0, sizeof(pomiar));

		TicksSinceLastRecordArrived = 0;
	}
	isr_kbd();
}

void HAL_RTC_AlarmAEventCallback(RTC_HandleTypeDef *hrtc)
{
	onesecondTick = 1;
	if (frame.type == FRAME_BP)
	{
		if (ssTimer == SCREEN_SAVER_BP_DELAY - 1)
		{
			settings.contrast = SS_CONTRAST;
			ssStateChanged = 1;
			ssTimer = 0xFF;
		}
		else if (ssTimer < SCREEN_SAVER_BP_DELAY - 1) ssTimer++;
	}
	else
	{
		if (ssTimer == SCREEN_SAVER_DELAY - 1)
		{
			settings.contrast = SS_CONTRAST;
			ssStateChanged = 1;
			ssTimer = 0xFF;
		}
		else if (ssTimer < SCREEN_SAVER_DELAY - 1) ssTimer++;
	}

	if (sampleDetected != 0 && sampleDetected != 3)
	{
		if (ledTimer == 0)
		{
			sampleDetected = 3;
			//UART_print("ledTimer=0\r\n");
		}
		else
		{
			//UART_print("ledTimer-\r\n");
			ledTimer--;
		}
	}


	if (initDone)
	{
// 		HAL_UART_Receive_IT(&huart3, rx_buff, 1);
// 		HAL_UART_Receive_IT(&huart1, usbrx_buff, 1);
// 		if(pcConnectionTimeout >= 2)	//PC connection timed out..
// 		{
// 			HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, 1);
// 			SendRecord(R_POMIAR_OUT, A_HEAD, A_MAINBOARD, NULL, 0);
// 		}
// 		else {pcConnectionTimeout++; HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, 0);}
//
		if (settings.measurementType == MEASUREMENT_CONTINUOUS)			//umożliwia ponowny zapis do archiwum tej samej probki. sekunde po zapisaniu w archiwum napis AXXX znika i można dokonac kolejnego
		{
			newArchiveEntryAdded = 0;
		}
	}
	if (pomiarTimer > 0)
	{
		pomiarTimer--;
		if (pomiarTimer == 0) pomiarStatus = POMIAR_DONE;
	}


}
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */

  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{ 
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
