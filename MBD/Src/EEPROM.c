/*
 * EEPROM.c
 *
 *  Created on: 17 мая 2021 г.
 *      Author: Admin
 */

#include "EEPROM.h"
//Obsluga SPI i eepromu
uint8_t spiTradeByte(uint8_t byte)
{
 uint8_t answer;

 HAL_SPI_TransmitReceive(&hspi2, &byte, &answer, 1, 5000);

 return answer;
}

uint8_t EEPROM_read(uint16_t address)
{
	uint8_t result;
	HAL_GPIO_WritePin(SPI_CS_GPIO_Port, SPI_CS_Pin, GPIO_PIN_RESET);	//select eeprom ic
	spiTradeByte(EEPROM_READ);
	spiTradeByte((uint8_t)(address >> 8));
	spiTradeByte((uint8_t)(address & 0x00FF));
	result=spiTradeByte(0);
	HAL_GPIO_WritePin(SPI_CS_GPIO_Port, SPI_CS_Pin, GPIO_PIN_SET);	//select eeprom ic
	return result;
}

void EEPROM_read_multiple(uint16_t address, uint8_t* rData, uint8_t byteCount)
{
	HAL_GPIO_WritePin(SPI_CS_GPIO_Port, SPI_CS_Pin, GPIO_PIN_RESET);	//select eeprom ic
	spiTradeByte(EEPROM_READ);
	spiTradeByte((uint8_t)(address >> 8));
	spiTradeByte((uint8_t)(address & 0x00FF));

	HAL_SPI_Receive(&hspi2, (uint8_t *)rData, byteCount, 5000);

	HAL_GPIO_WritePin(SPI_CS_GPIO_Port, SPI_CS_Pin, GPIO_PIN_SET);	//select eeprom ic
}

void EEPROM_readSettings(uint16_t address, struct SETTINGS* settingsStruct, uint8_t byteCount)
{
	HAL_GPIO_WritePin(SPI_CS_GPIO_Port, SPI_CS_Pin, GPIO_PIN_RESET);	//select eeprom ic
	spiTradeByte(EEPROM_READ);
	spiTradeByte((uint8_t)(address >> 8));
	spiTradeByte((uint8_t)(address & 0x00FF));

	HAL_SPI_Receive(&hspi2, (uint8_t*)((struct SETTINGS *)settingsStruct), byteCount, 5000);

	HAL_GPIO_WritePin(SPI_CS_GPIO_Port, SPI_CS_Pin, GPIO_PIN_SET);	//select eeprom ic
}

void EEPROM_write(uint16_t address, uint8_t *byte)
{
	HAL_GPIO_WritePin(SPI_CS_GPIO_Port, SPI_CS_Pin, GPIO_PIN_RESET);	//select eeprom ic
	spiTradeByte(EEPROM_WREN);
	HAL_GPIO_WritePin(SPI_CS_GPIO_Port, SPI_CS_Pin, GPIO_PIN_SET);	//deselect eeprom ic
	HAL_GPIO_WritePin(SPI_CS_GPIO_Port, SPI_CS_Pin, GPIO_PIN_RESET);
	spiTradeByte(EEPROM_WRITE);
	spiTradeByte((uint8_t)(address >> 8));
	spiTradeByte((uint8_t)(address & 0x00FF));
	spiTradeByte(*byte);
	HAL_GPIO_WritePin(SPI_CS_GPIO_Port, SPI_CS_Pin, GPIO_PIN_SET);	//deselect eeprom ic
	HAL_Delay(6);
}

void EEPROM_write_multiple(uint16_t address, uint8_t* bytes, uint8_t byteCount)
{
	HAL_GPIO_WritePin(SPI_CS_GPIO_Port, SPI_CS_Pin, GPIO_PIN_RESET);	//select eeprom ic
	spiTradeByte(EEPROM_WREN);
	HAL_GPIO_WritePin(SPI_CS_GPIO_Port, SPI_CS_Pin, GPIO_PIN_SET);	//deselect eeprom ic
	HAL_GPIO_WritePin(SPI_CS_GPIO_Port, SPI_CS_Pin, GPIO_PIN_RESET);
	spiTradeByte(EEPROM_WRITE);
	spiTradeByte((uint8_t)(address >> 8));
	spiTradeByte((uint8_t)(address & 0x00FF));
	HAL_SPI_Transmit(&hspi2, bytes, byteCount, 5000);
	HAL_GPIO_WritePin(SPI_CS_GPIO_Port, SPI_CS_Pin, GPIO_PIN_SET);	//deselect eeprom ic
	HAL_Delay(10);
}

//funkcje od archiwum

void EEPROM_add_archive_entry(struct ARCHIVE* ArchiveEntry, uint8_t byteCount)
{
	uint16_t address;
	uint8_t currentPage;
	uint8_t diff;
	settings.archiveFirstEntryOffset = EEPROM_read(EEPROM_ARCHIVE_FIRST_ENTRY_OFFSET_ADR);
	settings.archiveEntriesTotal = EEPROM_read(EEPROM_ARCHIVE_ENTRIES_TOTAL_ADR);

	if (settings.archiveEntriesTotal == ARCHIVE_SIZE)								//jezeli archiwum pelne
	{
		address = EEPROM_ARCHIVE_BASE_ADDRESS + settings.archiveFirstEntryOffset * byteCount;
	}
	else
	{
		address = EEPROM_ARCHIVE_BASE_ADDRESS + settings.archiveEntriesTotal * byteCount;
	}


	for (currentPage = 1; currentPage <= 255; currentPage++)
	{
		if (address < EEPROM_ARCHIVE_BASE_ADDRESS + currentPage * EEPROM_PAGE_SIZE) break;
	}
	diff = (currentPage + 1) * EEPROM_PAGE_SIZE - address;

	if (diff >= sizeof(*ArchiveEntry))
	{
		HAL_GPIO_WritePin(SPI_CS_GPIO_Port, SPI_CS_Pin, GPIO_PIN_RESET);	//select eeprom ic
		spiTradeByte(EEPROM_WREN);
		HAL_GPIO_WritePin(SPI_CS_GPIO_Port, SPI_CS_Pin, GPIO_PIN_SET);	//deselect eeprom ic
		//HAL_Delay(100);
		HAL_GPIO_WritePin(SPI_CS_GPIO_Port, SPI_CS_Pin, GPIO_PIN_RESET);
		spiTradeByte(EEPROM_WRITE);
		spiTradeByte((uint8_t)(address>>8));
		spiTradeByte((uint8_t)(address&0x00FF));
		HAL_SPI_Transmit(&hspi2,(uint8_t*) ArchiveEntry, byteCount, 5000);			//bylo bez (uint8_t*) ale dawalo warning. jakby cos swirowalo to trzeba poprawic
		HAL_GPIO_WritePin(SPI_CS_GPIO_Port, SPI_CS_Pin, GPIO_PIN_SET);	//deselect eeprom ic
		HAL_Delay(10);
	}
	else
	{
		EEPROM_write_multiple(address, (uint8_t*)(ArchiveEntry), diff);
		EEPROM_write_multiple((currentPage + 1) * EEPROM_PAGE_SIZE, (uint8_t*)(ArchiveEntry)+diff, byteCount - diff);
	}

	if(settings.archiveEntriesTotal < ARCHIVE_SIZE)				//jezeli nie pelne zapamietaj ile zostalo zuzyte miejsca
	{
		settings.archiveEntriesTotal++;
		EEPROM_write(EEPROM_ARCHIVE_ENTRIES_TOTAL_ADR, &settings.archiveEntriesTotal);
	}
	else	//jezeli archiwum pelne
	{
		if(settings.archiveFirstEntryOffset < ARCHIVE_END_PTR)	//i pierwszy wpis byl przed koncowym adresem archiwum w eepromie
		{														//dodawaj kolejne wpisy
			settings.archiveFirstEntryOffset++;
		}
		else
		{
			settings.archiveFirstEntryOffset = 0;				//jezeli wpis byl na ostatniej pozycji w pamieci archiwum to nadpisuj od najstarszej pozycji
		}
		EEPROM_write(EEPROM_ARCHIVE_FIRST_ENTRY_OFFSET_ADR, &settings.archiveFirstEntryOffset);
	}


}

void EEPROM_read_archive_entry(uint8_t entryNO, struct ARCHIVE* ArchiveEntry, uint8_t byteCount)
{
	uint16_t address;
	settings.archiveFirstEntryOffset = EEPROM_read(EEPROM_ARCHIVE_FIRST_ENTRY_OFFSET_ADR);
	settings.archiveEntriesTotal = EEPROM_read(EEPROM_ARCHIVE_ENTRIES_TOTAL_ADR);

	if (entryNO > settings.archiveEntriesTotal - 1)	//jesli wybrano wpis o numerze wiekszym jak pojemnosc archiwum
	{
		ArchiveEntry->day = 255;	//TODO: to dodane aby nie zapomniec ze taki przypadek sie moze teoretycznie zdazyc i bedzie zczytywac jakies smieci. trza tu cos bedzie powstawiac moze
		ArchiveEntry->month = 255;
	}
	else	//jesli wszytko wywolane bylo ok
	{
		if (entryNO + settings.archiveFirstEntryOffset > ARCHIVE_END_PTR)	//jezeli numer wpisu archiwum wychodzi poza przestrzen adresowa archiwum w eepromie(np. bo pierwszy wpis jest gdzies blizej konca)
		{
			address = EEPROM_ARCHIVE_BASE_ADDRESS + (entryNO + settings.archiveFirstEntryOffset - ARCHIVE_SIZE) * byteCount; //to wylicz pozycje patrzac od poczatku przestrzeni adresowej
		}
		else if (settings.archiveFirstEntryOffset == ARCHIVE_END_PTR && entryNO > 0)	//jezeli pierwszy wpis w ostatniej pozycji i odczytywany jest wlasnie on
		{
			address = EEPROM_ARCHIVE_BASE_ADDRESS + (entryNO - 1) * byteCount;
		}
		else	//pozostale wpisy odczytywane normalnie
		{
			address = EEPROM_ARCHIVE_BASE_ADDRESS + entryNO * byteCount + settings.archiveFirstEntryOffset * byteCount;
		}

		for (uint8_t i=0; i<byteCount; i++)
		{
			*((uint8_t*)(ArchiveEntry) + i) = EEPROM_read(address+i);
		}
	}
}

void erase_archive(void)
{
	uint16_t archiveUsedPages;
	uint8_t a[128] = {0xFF};
	archiveUsedPages = ARCHIVE_SIZE * sizeof(ArchiveEntry);
	if (archiveUsedPages % 128 == 0)
	{
		archiveUsedPages /= 128;
	}
	else
	{
		archiveUsedPages /= 128;
		archiveUsedPages += 1;
	}

	for (uint16_t i = 0; i < archiveUsedPages; i++) EEPROM_write_multiple(EEPROM_ARCHIVE_BASE_ADDRESS + i * EEPROM_PAGE_SIZE, a, sizeof(a));
	memset(&ArchiveEntry, 0, sizeof(ArchiveEntry));
	settings.archiveFirstEntryOffset = 0;
	settings.archiveEntriesTotal = 0;
	EEPROM_write(EEPROM_ARCHIVE_FIRST_ENTRY_OFFSET_ADR, &settings.archiveFirstEntryOffset);
	EEPROM_write(EEPROM_ARCHIVE_ENTRIES_TOTAL_ADR, &settings.archiveEntriesTotal);
}

void erase_settings(void)
{
	uint8_t a[sizeof(settings)] = {[0 ... sizeof(settings)-1] = 0xFF};
	EEPROM_write_multiple(EEPROM_SETTINGS_ADDRESS, a, sizeof(settings));
}
