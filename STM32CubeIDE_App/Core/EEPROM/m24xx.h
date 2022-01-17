/**
 ******************************************************************************
 * @file    m24xx.h
 * @author  SRA-SAIL, STM32ODE
 * @version V2.0.0
 * @date    24-09-2020
 * @brief   header file of m24xx based eeprom driver 
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
 * All rights reserved.</center></h2>
 *
 * This software component is licensed by ST under BSD 3-Clause license,
 * the "License"; You may not use this file except in compliance with the
 * License. You may obtain a copy of the License at:
 *                        opensource.org/licenses/BSD-3-Clause
 *
 ******************************************************************************
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __M24_XX_H
#define __M24_XX_H

#ifdef __cplusplus
  extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/

#include <stdint.h>
#include "main.h"
/** @addtogroup BSP
  * @{
  */ 

/** @addtogroup Components
  * @{
  */ 
/** @addtogroup M24
  * @{
  */

/* Exported types ------------------------------------------------------------*/
/** @defgroup M24_Exported_Types
  * @{
  */



/* Exported constants --------------------------------------------------------*/
/** @defgroup M24_Exported_Constants
  * @{
  */
#ifndef NULL
#define NULL      (void *) 0
#endif
#define MIN_TRIALS              2
#define LOCKID                  0x02   /* Byte to be send to lock ID page*/    
#define ADDRLID_16              0x0400
  
#define ADDRLID_SPI             0x0480    
#define LOCKDATA_SPI            0x02
    
#define BITCOUNT                0x01  /* Initial count value */
#define IDMASK                  0x04  /* Mask fith bit of I2C mem device address */
    
#define EEPROMEX_WREN           0x06  /*!< Write enable instruction */
#define EEPROMEX_WRDI           0x04  /*!< Write disable instruction */
#define EEPROMEX_RDSR           0x05  /*!< Read Status Register instruction  */
#define EEPROMEX_WRSR           0x01  /*!< Write Status Register instruction */
#define EEPROMEX_WRITE          0x02  /*!< Lower Half Write to Memory instruction */
#define EEPROMEX_UPWRITE        0x0A  /*!< Upper Half Write to Memory instruction */
#define EEPROMEX_READ           0x03  /*!< Lower Half Read from Memory instruction */
#define EEPROMEX_UPREAD         0x0B  /*!< Upper Half Read from Memory instruction */
#define EEPROMEX_RDID           0x83  /*!< Read identifiction instruction */
#define EEPROMEX_WRID           0x82  /*!< Write identifiction instruction */
#define EEPROMEX_RDLS           0x83  /*!< Read ID page lock status instruction */
#define EEPROMEX_LID            0x82  /*!< Lock ID page in read only mode instruction */
#define EEPROMEX_FullProtect    0x0C  /*!< Whole memory protect from write - Write BP1 BP2 bits as 11 in status register */
#define EEPROMEX_UHalfProtect   0x08  /*!< Upper Half Protect from write - Write BP1 BP2 bits as 10 in status register */
#define EEPROMEX_UQuarterProtect 0x04  /*!< Upper Quarter Protect from write - Write BP1 BP2 bits as 01 in status register */
#define EEPROMEX_UnProtect      0x00  /*!< Un Protect from read only mode - Write BP1 BP2 bits as 10 in status register */


#define	MAX_PAGE_SIZE			256

/** @defgroup M24XX error codes
  * @}
  */
#define M24_I2C_POLL_TIMEOUT                 0x1000U

#define M24_OK              				 0
#define M24_ERROR   	   					-1
#define M24_BUSY    	   					-2
#define M24_TIMEOUT 	   					-3
#define M24_ADDR_OVERFLOW  					-4
#define M24_BUS_FAILURE	   					-5
#define M24_ERROR_BUS_ACKNOWLEDGE_FAILURE	-6
#define M24_ERROR_PERIPH_FAILURE			-7

int32_t M24_i2c_Init(I2C_HandleTypeDef* hi2c, uint8_t addr8bit, uint16_t pageSize, uint16_t pageCount);
int32_t M24_i2c_DeInit();
int32_t M24_i2c_IsDeviceReady(uint8_t devAddr, const uint32_t Trials);
int32_t M24_i2c_ReadByte(uint8_t * pData, const uint32_t TarAddr);
int32_t M24_i2c_ReadPage(uint8_t * pData, const uint32_t TarAddr);
int32_t M24_i2c_ReadData(uint8_t * pData, const uint32_t TarAddr, const uint16_t NbByte);
int32_t M24_i2c_WriteByte(uint8_t * pData, const uint32_t TarAddr);
int32_t M24_i2c_WritePage(uint8_t * pData, const uint32_t TarAddr, const uint16_t NByte);
int32_t M24_i2c_WriteData(uint8_t * pData, const uint32_t TarAddr, const uint16_t NbByte );
int32_t M24_i2c_WriteID(uint8_t * pData, const uint32_t TarAddr, const uint16_t NbByte );
int32_t M24_i2c_ReadID(uint8_t * pData, const uint32_t TarAddr, const uint16_t NbByte );
int32_t M24_i2c_LockID();
int32_t M24_i2c_LockStatus(uint8_t * pData );
 
#ifdef __cplusplus
  }
#endif
#endif /* __M_24xx_H */

/******************* (C) COPYRIGHT 2018 STMicroelectronics *****END OF FILE****/
