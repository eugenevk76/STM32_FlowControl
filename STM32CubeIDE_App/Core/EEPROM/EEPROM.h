
#ifndef EEPROM_EEPROM_H_
#define EEPROM_EEPROM_H_

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

#define I2C_ADDR  80 << 1

/* Exported Functions --------------------------------------------------------*/
void EEPROM_Init(void);

/////////////////////////////////
// Public interface
bool EEPROM_ReadData(uint32_t TargetAddress, uint8_t rx[], uint16_t Size);
bool EEPROM_WriteData(uint32_t TargetAddress, uint8_t tx[], uint16_t Size);
bool EEPROM_ReadByte(uint32_t TargetAddress, uint8_t* rx);
bool EEPROM_WriteByte(uint32_t TargetAddress, uint8_t* tx);
bool EEPROM_WriteDataByBytes(uint32_t TargetAddress, uint8_t tx[], uint16_t Size);
bool EEPROM_ReadDataByBytes(uint32_t TargetAddress, uint8_t rx[], uint16_t Size);
bool EEPROM_EraseChip();


//////////////////
//Tools
void float2Bytes(uint8_t * ftoa_bytes_temp,float float_variable);
float Bytes2float(uint8_t * ftoa_bytes_temp);

/////////////////////////////////
// Testing interface

bool EEPROM_Test(void);
//void MX_EEPROM_Process(void);

#ifdef __cplusplus
}
#endif



#endif /* EEPROM_EEPROM_H_ */
