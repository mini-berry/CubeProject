#ifndef __W25Q_H__
#define __W25Q_H__
#include "stm32f1xx_hal.h"
#include "spi.h"

#define FLASH_ID 0XEF14
#define W25X_WriteEnable		0x06 
#define W25X_WriteDisable		0x04 
#define W25X_ReadStatusReg		0x05 
#define W25X_WriteStatusReg		0x01 
#define W25X_ReadData			0x03 
#define W25X_FastReadData		0x0B 
#define W25X_FastReadDual		0x3B 
#define W25X_PageProgram		0x02 
#define W25X_BlockErase			0xD8 
#define W25X_SectorErase		0x20 
#define W25X_ChipErase			0xC7 
#define W25X_PowerDown			0xB9 
#define W25X_ReleasePowerDown	0xAB 
#define W25X_DeviceID			0xAB 
#define W25X_ManufactDeviceID	0x90 
#define W25X_JedecDeviceID		0x9F 

uint16_t SPI_Flash_ReadID(void);
void SPI_Flash_Erase_Chip(void);
void SPI_Flash_Read(uint32_t ReadAddr,uint16_t NumByteToRead,uint8_t* pBuffer);
void SPI_Flash_Write_Page(uint32_t WriteAddr,uint16_t NumByteToWrite,uint8_t* pBuffer);
 
#endif