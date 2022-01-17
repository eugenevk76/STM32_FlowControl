
#ifdef __cplusplus
 extern "C" {
#endif

#include "EEPROM.h"
#include "m24xx.h"
#include "string.h"

 extern I2C_HandleTypeDef hi2c1;


#define COUNTOF(a)      (sizeof(a) / sizeof(*(a)))

/*size of data to be read/write */
//#define SIZE10     (COUNTOF(tx10)-1)
//#define SIZE16     (COUNTOF(tx16)-1)
//#define SIZE43     (COUNTOF(tx43)-1)
//#define SIZE128    (COUNTOF(tx128)-1)
//#define SIZE256    (COUNTOF(tx256)-1)
//#define SIZE260    (COUNTOF(tx260)-1)
#define SIZE495    (COUNTOF(tx495)-1)
//#define SIZE512    (COUNTOF(tx512)-1)
//#define SIZE20     (COUNTOF(tx20)-1)

/*Sample data for R/W */
//uint8_t tx10[] = "STM32F4/L4";
uint8_t tx495[] = "abcdefghE-EEPROM- Expansion Firmware library EEPROM driver example : This firmware provides a basic example of how to use the X-Nucleo-eXpansion firmware library. This block of data is specially written to test the data write function of EEPROM (SPI/I2C)   abcdefghE-EEPROM-Expansion Firmware library EEPROM driver example : This firmware provides a basic example of how to use the X-Nucleo-eXpansion firmware library. This block of data is specially written to test the data write function of E";
/*Rx buffer*/
//uint8_t rx10[SIZE10];
uint8_t rx495[SIZE495];

 ////////////////////////////////
 // Testing prototype

bool M24_TestSingleByte(uint32_t  TargetAddress,int32_t TargetName,int32_t expWriteStatus,int32_t expReadStatus);
bool M24_TestPage(uint32_t TargetAddress,uint32_t TargetName,uint8_t tx[],uint8_t rx[],uint16_t Size,int32_t expWriteStatus,int32_t expReadStatus);
bool M24_TestData(uint32_t  TargetAddress,uint32_t TargetName, uint8_t tx[],uint8_t rx[],uint16_t Size,int32_t expWriteStatus,int32_t expReadStatus);


void EEPROM_Init(void)
{

  while(M24_i2c_Init(&hi2c1, I2C_ADDR, 256, 512) != M24_OK);

}


bool EEPROM_ReadData(uint32_t TargetAddress, uint8_t rx[], uint16_t Size) {

	  int32_t readstatus = M24_i2c_ReadData(rx, TargetAddress, Size);
	  HAL_Delay(5);

	  if (readstatus == M24_OK)
		return true;
	  else
		return false;

}

bool EEPROM_WriteData(uint32_t TargetAddress, uint8_t tx[], uint16_t Size) {

	  int32_t writestatus = M24_i2c_WriteData(tx, TargetAddress, Size);
	  HAL_Delay(5);

	  if (writestatus == M24_OK)
		return true;
	  else
		return false;

}

bool EEPROM_EraseChip() {

	  int32_t eraseStatus = M24_i2c_EraseEntireChip();
	  HAL_Delay(5);

	  if (eraseStatus == M24_OK)
		return true;
	  else
		return false;

}


bool EEPROM_WriteDataByBytes(uint32_t TargetAddress, uint8_t tx[], uint16_t Size) {

	  int cnt = 0;

	  while(Size > cnt) {

		int32_t writeStatus = M24_i2c_WriteByte(tx[cnt], TargetAddress++);
		HAL_Delay(10);

		cnt++;

		if(writeStatus == M24_OK)
			continue;
		else
			return false;



	  }

	  return true;
}

bool EEPROM_ReadDataByBytes(uint32_t TargetAddress, uint8_t rx[], uint16_t Size) {

	  int cnt = 0;

	  while(Size > cnt) {

		int32_t readStatus = M24_i2c_ReadByte(rx[cnt], TargetAddress++);
		HAL_Delay(10);

		cnt++;

		if(readStatus == M24_OK)
			continue;
		else
			return false;

	  }

	  return true;
}


bool EEPROM_ReadByte(uint32_t TargetAddress, uint8_t* rx) {

	int32_t readstatus = M24_i2c_ReadByte(rx, TargetAddress);
	HAL_Delay(10);

	if(readstatus == M24_OK)
		return true;
	else
		return false;

}
bool EEPROM_WriteByte(uint32_t TargetAddress, uint8_t* tx) {

	int32_t writeStatus = M24_i2c_WriteByte(tx, TargetAddress);
	HAL_Delay(10);

	if(writeStatus == M24_OK)
		return true;
	else
		return false;

}

////////////////////////////////
// Testing


bool EEPROM_Test(void) {

	//bool result = M24_TestSingleByte(0x05,EEPRMA1_M24M01_0,BSP_ERROR_NONE,BSP_ERROR_NONE);
	//while(!result);
	//result = M24_TestData(0x5B,EEPRMA1_M24M01_0,tx495,rx495,SIZE495,BSP_ERROR_NONE,BSP_ERROR_NONE);

//	bool result = M24_TestData(130000,EEPRMA1_M24M01_0,tx495,rx495,SIZE495,BSP_ERROR_NONE,BSP_ERROR_NONE);
//	while(!result);

	return true;

}
/**
  * @brief  Test Byte Write and Byte Read in the memory at specific address.
  * @param  TargetAddress : I2C/SPI data memory address to write.
  * @param  TargetName : memory name to write.
  * @param  expWriteStatus : Expected Write EEPROM enum Status according to parameter passed.
  * @param  expReadStatus : Expected Read EEPROM enum Status according to parameter passed.
  * @retval None
  */
bool M24_TestSingleByte(uint32_t  TargetAddress,int32_t TargetName,int32_t expWriteStatus,int32_t expReadStatus)
{
  uint8_t tx[] = "k";
  uint8_t rx[1];
  //EEPROMEX_CTRL_LOW(WC);
  int32_t writeStatus = EEPRMA1_M24_WriteByte(TargetName, tx, TargetAddress);
  //HAL_Delay(10);
  //EEPROMEX_CTRL_HIGH(WC);
  int32_t readstatus = EEPRMA1_M24_ReadByte(TargetName, rx, TargetAddress);
  HAL_Delay(10);

  if ( (writeStatus == expWriteStatus) && (readstatus == expReadStatus) )       /* to check whether roll over case */
	  return true;
      //printf("TestByte|target: %s|address: %lu|TX: %s|\t RX: %s|result: passed \r\n","M24C02",TargetAddress, tx, rx);
  else
      //printf("TestByte|target: %s|address: %lu|result: failed \r\n","M24C02",TargetAddress);
	  return false;

}


/**
  * @brief  Bytes Write/Read in the memory at specific address.
  * @param  TargetAddress : I2C/SPI data memory address to write.
  * @param  TargetName : memory name to write.
  * @param  tx : Input Buffer to Write.
  * @param  rx : Output Buffer to Read.
  * @param  Size : Size in bytes of the value to be written.
  * @param  expWriteStatus : Expected Write EEPROM enum Status according to parameter passed.
  * @param  expReadStatus : Expected Read EEPROM enum Status according to parameter passed.
  * @retval None
  */
bool M24_TestData(uint32_t  TargetAddress,uint32_t TargetName, uint8_t tx[],uint8_t rx[],uint16_t Size,int32_t expWriteStatus,int32_t expReadStatus)
{
  int32_t writeStatus = EEPRMA1_M24_WriteData(TargetName, tx, TargetAddress, Size);
  HAL_Delay(5);
  int32_t readstatus = EEPRMA1_M24_ReadData(TargetName, rx, TargetAddress, Size);
  HAL_Delay(5);

  if ( (writeStatus == expWriteStatus) && (readstatus == expReadStatus) )
    //printf("TestData|target: %s|address: %lu|data size: %d|result: passed \r\n","M24M01",TargetAddress,Size);
	return true;
  else
    //printf("TestData|target: %s|address: %lu|data size: %d|result: failed \r\n","M24M01",TargetAddress,Size);
	return false;
}

///////////////////////////////////////////////////////
// Tools

void float2Bytes(uint8_t * ftoa_bytes_temp,float float_variable)
{
    union {
      float a;
      uint8_t bytes[4];
    } thing;

    thing.a = float_variable;

    for (uint8_t i = 0; i < 4; i++) {
      ftoa_bytes_temp[i] = thing.bytes[i];
    }

}

float Bytes2float(uint8_t * ftoa_bytes_temp)
{
    union {
      float a;
      uint8_t bytes[4];
    } thing;

    for (uint8_t i = 0; i < 4; i++) {
    	thing.bytes[i] = ftoa_bytes_temp[i];
    }

   float float_variable =  thing.a;
   return float_variable;
}



#ifdef __cplusplus
}
#endif

