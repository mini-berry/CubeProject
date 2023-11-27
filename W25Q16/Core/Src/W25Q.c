#include "W25Q.h"

#define SPI_FLASH_CS_H() HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET)
#define SPI_FLASH_CS_L() HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET)

extern SPI_HandleTypeDef hspi2;

uint8_t SPI4_ReadByte(void)
{
	uint8_t RxData;
	HAL_SPI_Receive(&hspi2, &RxData, 1, 20);
	return RxData;
}

void SPI4_WriteByte(uint8_t TxData)
{
	HAL_SPI_Transmit(&hspi2, &TxData, 1, 20);
}

void SPI_FLASH_Write_Enable(void)
{
	SPI_FLASH_CS_L();
	SPI4_WriteByte(W25X_WriteEnable);
	SPI_FLASH_CS_H();
}

void SPI_FLASH_Write_Disable(void)
{
	SPI_FLASH_CS_L();
	SPI4_WriteByte(W25X_WriteDisable);
	SPI_FLASH_CS_H();
}

uint8_t SPI_Flash_ReadSR(void)
{
	uint8_t byte = 0;
	SPI_FLASH_CS_L();
	SPI4_WriteByte(W25X_ReadStatusReg);
	byte = SPI4_ReadByte();
	SPI_FLASH_CS_H();
	return byte;
}

void SPI_Flash_Wait_Busy(void)
{
	while ((SPI_Flash_ReadSR() & 0x01) == 0x01)
		;
}

uint16_t SPI_Flash_ReadID(void)
{
	uint16_t Temp = 0;
	SPI_FLASH_CS_L();
	SPI4_WriteByte(0x90);
	SPI4_WriteByte(0x00);
	SPI4_WriteByte(0x00);
	SPI4_WriteByte(0x00);

	Temp |= SPI4_ReadByte() << 8;
	Temp |= SPI4_ReadByte();
	SPI_FLASH_CS_H();

	return Temp;
}

void SPI_Flash_Erase_Chip(void)
{
	SPI_FLASH_Write_Enable();
	SPI_Flash_Wait_Busy();
	SPI_FLASH_CS_L();
	SPI4_WriteByte(W25X_ChipErase);
	SPI_FLASH_CS_H();
	SPI_Flash_Wait_Busy();
}

void SPI_Flash_Read(uint32_t ReadAddr, uint16_t NumByteToRead, uint8_t *pBuffer)
{
	uint16_t i;
	SPI_FLASH_CS_L();
	SPI4_WriteByte(W25X_ReadData);
	SPI4_WriteByte((uint8_t)((ReadAddr) >> 16));
	SPI4_WriteByte((uint8_t)((ReadAddr) >> 8));
	SPI4_WriteByte((uint8_t)ReadAddr);
	for (i = 0; i < NumByteToRead; i++)
	{
		pBuffer[i] = SPI4_ReadByte();
	}
	SPI_FLASH_CS_H();
}

void SPI_Flash_Write_Page(uint32_t WriteAddr, uint16_t NumByteToWrite, uint8_t *pBuffer)
{
	uint16_t i;
	SPI_FLASH_Write_Enable();
	SPI_FLASH_CS_L();
	SPI4_WriteByte(W25X_PageProgram);
	SPI4_WriteByte((uint8_t)((WriteAddr) >> 16));
	SPI4_WriteByte((uint8_t)((WriteAddr) >> 8));
	SPI4_WriteByte((uint8_t)WriteAddr);
	for (i = 0; i < NumByteToWrite; i++)
		SPI4_WriteByte(pBuffer[i]);
	SPI_FLASH_CS_H();
	SPI_Flash_Wait_Busy();
}