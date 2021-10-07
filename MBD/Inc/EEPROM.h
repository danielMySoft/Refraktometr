/*
 * EEPROM.h
 *	EEPROM i Archiwum
 *
 *  Created on: 17 мая 2021 г.
 *      Author: Admin
 */

#ifndef EEPROM_H_
#define EEPROM_H_

#include "main.h"
#include <string.h>

//EEPROM COMMANDS
#define EEPROM_READ 0x03
#define EEPROM_WRITE 0x02
#define EEPROM_WREN 0x06
#define EEPROM_WRDI 0x04
#define EEPROM_RDSR 0x05
#define EEPROM_WRSR 0x01
#define EEPROM_PE 0x42
#define EEPROM_SE 0xD8
#define EEPROM_CE 0xC7
#define EEPROM_RDID 0xAB
#define EEPROM_DPD 0xB9

#define EEPROM_PAGE_SIZE 128

//EEPROM settings addresses
#define EEPROM_SETTINGS_ADDRESS 0
#define EEPROM_MEASUREMENT_TYPE_ADDR 0
#define EEPROM_MEASUREMENT_UNIT_ADDR &settings.measurementUnit - &settings.measurementType + EEPROM_SETTINGS_ADDRESS
#define EEPROM_THERMAL_COMP_ADDR &settings.thermalCompensation - &settings.measurementType + EEPROM_SETTINGS_ADDRESS
#define EEPROM_SINGLE_MEASUREMENT_TIME_ADDR &settings.singleMeasurementTime - &settings.measurementType + EEPROM_SETTINGS_ADDRESS
#define EEPROM_LANGUAGE_ADDR &settings.language - &settings.measurementType + EEPROM_SETTINGS_ADDRESS

#define EEPROM_CONTRAST_ADDR &settings.contrast - &settings.measurementType + EEPROM_SETTINGS_ADDRESS

#define EEPROM_ARCHIVE_STORAGE_MODE_ADDR &settings.storageMode - &settings.measurementType + EEPROM_SETTINGS_ADDRESS
#define EEPROM_ARCHIVE_FIRST_ENTRY_OFFSET_ADR &settings.archiveFirstEntryOffset - &settings.measurementType + EEPROM_SETTINGS_ADDRESS
#define EEPROM_ARCHIVE_ENTRIES_TOTAL_ADR &settings.archiveEntriesTotal - &settings.measurementType + EEPROM_SETTINGS_ADDRESS

#define EEPROM_SERIAL_NUMBER_ADDR (uint8_t*)&settings.serialNumber - &settings.measurementType + EEPROM_SETTINGS_ADDRESS

//ARCHIWUM
#define EEPROM_ARCHIVE_BASE_ADDRESS 128
#define ARCHIVE_SIZE 250
#define ARCHIVE_END_PTR ARCHIVE_SIZE - 1



uint8_t spiTradeByte(uint8_t byte);
uint8_t EEPROM_read(uint16_t address);
void EEPROM_read_multiple(uint16_t address, uint8_t* rData, uint8_t byteCount);
void EEPROM_readSettings(uint16_t address, struct SETTINGS* settingsStruct, uint8_t byteCount);
void EEPROM_write(uint16_t address, uint8_t *byte);
void EEPROM_write_multiple(uint16_t address, uint8_t* bytes, uint8_t byteCount);
void EEPROM_add_archive_entry(struct ARCHIVE* ArchiveEntry, uint8_t byteCount);
void EEPROM_read_archive_entry(uint8_t entryNO, struct ARCHIVE* ArchiveEntry, uint8_t byteCount);
void erase_archive(void);
void erase_settings(void);

extern SPI_HandleTypeDef hspi2;

#endif /* EEPROM_H_ */
