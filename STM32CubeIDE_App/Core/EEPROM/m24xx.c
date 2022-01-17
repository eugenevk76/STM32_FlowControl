/**
 ******************************************************************************
 * @file    m24xx.c
 * @author  SRA-SAIL, STM32ODE
 * @version V2.0.0
 * @date    24-09-2020
 * @brief   file provides functionality of driver functions and to manage communication 
             between BSP and M24xx chip
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

/* Includes ------------------------------------------------------------------*/
#include "m24xx.h"
//#include "math.h"
#include "string.h"



static I2C_HandleTypeDef* HI2C;
static uint8_t 	_DevAddr;
static uint16_t 	_PageSize;
static uint16_t 	_PageCount;
static uint32_t 	_EndOfMemory;


/////////////////////////////////////////////
// Private prototypes

int32_t BSP_I2C_ReadReg(uint8_t devAddr, uint16_t Reg, uint8_t *pData, uint16_t Length);
int32_t BSP_I2C_ReadReg16(uint8_t devAddr, uint16_t Reg, uint8_t *pData, uint16_t Length);
int32_t BSP_I2C_WriteReg(uint16_t DevAddr, uint16_t Reg, uint8_t *pData, uint16_t Length);
int32_t BSP_I2C_WriteReg16(uint8_t devAddr, uint16_t Reg, uint8_t *pData, uint16_t Length);
int32_t BSP_I2C_Send(uint8_t devAddr, uint8_t *pData, uint16_t Length);

int32_t M24_i2c_Init(I2C_HandleTypeDef* hi2c,
		uint8_t addr8bit,
		uint16_t pageSize,
		uint16_t pageCount)
{ 

	HI2C = hi2c;
	int32_t ret = M24_OK;

	if (HAL_I2C_GetState(HI2C) == HAL_I2C_STATE_RESET)
	{
		ret = M24_BUS_FAILURE;
	}

	_DevAddr = addr8bit;
	_PageSize = pageSize;
	_PageCount = pageCount;

	_EndOfMemory = pageSize * pageCount;

	return ret;

}

int32_t M24_i2c_DeInit()
{ 

	return M24_OK;

}

int32_t M24_i2c_IsDeviceReady(uint8_t devAddr, const uint32_t Trials)
{
	 int32_t ret = M24_OK;

	  if (HAL_I2C_IsDeviceReady(HI2C, devAddr, Trials, M24_I2C_POLL_TIMEOUT) != HAL_OK)
	  {
	    ret = M24_BUSY;
	  }

	  return ret;

}


int32_t M24_i2c_ReadByte(uint8_t * pData, const uint32_t TarAddr)
{  
  /* Before calling this function M24xx must be ready, here is a check to detect an issue */
  if (M24_i2c_IsDeviceReady(_DevAddr, MIN_TRIALS) != M24_OK )
    {
      return M24_TIMEOUT;
    }  
  
  if (_DevAddr == 0xA8)
    return BSP_I2C_ReadReg(_DevAddr, TarAddr, pData, 1);
  else
    return BSP_I2C_ReadReg16(_DevAddr, TarAddr, pData, 1);
}

/**
  * @brief  Read N bytes starting from specified I2C address
  * @param  pData : pointer of the data to store
  * @param  TarAddr : I2C data memory address to read
  * @param  NbByte : number of bytes to read
  * @retval EEPROMEX enum status
  */
int32_t M24_i2c_ReadPage(uint8_t * pData, const uint32_t TarAddr)
{  
  /* Before calling this function M24xx must be ready, here is a check to detect an issue */
  if (M24_i2c_IsDeviceReady(_DevAddr, MIN_TRIALS ) != M24_OK)
    {
      return M24_TIMEOUT;
    }
  
  if (_DevAddr == 0xA8)
    return BSP_I2C_ReadReg(_DevAddr, TarAddr, pData, _PageSize);
  else
    return BSP_I2C_ReadReg16(_DevAddr, TarAddr, pData, _PageSize);
}

/**
  * @brief  Read N bytes starting from specified I2C address
  * @param  pData : pointer of the data to store
  * @param  TarAddr : I2C data memory address to read
  * @param  DeviceAddr : Device Address of selected memory
  * @param  NbByte : number of bytes to read
  * @retval EEPROMEX enum status
  */
int32_t M24_i2c_ReadData(uint8_t * pData, const uint32_t TarAddr,
                           const uint16_t NbByte )
{  
 /* Before calling this function M24xx must be ready, here is a check to detect an issue */
  if (M24_i2c_IsDeviceReady(_DevAddr, MIN_TRIALS ) != M24_OK)
	{
	  return M24_TIMEOUT;
	}


  if(_DevAddr != 0xA8 && TarAddr + NbByte > 65535) {

	  if (M24_i2c_IsDeviceReady(_DevAddr + 2, MIN_TRIALS) != M24_OK )
	  {
		 return M24_TIMEOUT;
	  }
  }

  if (_DevAddr == 0xA8)
    return BSP_I2C_ReadReg(_DevAddr, TarAddr, pData, NbByte );
  else {

  if(TarAddr + NbByte > 65535) {


	  if(TarAddr > 65535) {

		  uint8_t addr = _DevAddr + 2;
		  uint32_t readAddr = TarAddr - 65536;

		  return BSP_I2C_ReadReg16(addr, readAddr, pData, NbByte );

	  } else {

		  uint32_t bytesToReadOnCurSection = 65535 - TarAddr;
		  uint32_t bytesToReadOnNextSection = NbByte - bytesToReadOnCurSection;
		  uint32_t result = BSP_I2C_ReadReg16(_DevAddr, TarAddr, pData, bytesToReadOnCurSection );
		  if(result != 0) return result;

		  return BSP_I2C_ReadReg16(_DevAddr + 2,
				  0,
				  pData + bytesToReadOnCurSection + 1,
				  bytesToReadOnNextSection
		  );

	  }

  } else {

	  return BSP_I2C_ReadReg16(_DevAddr, TarAddr, pData, NbByte);

  }

  }

}

/**
  * @brief  Write a single byte to a specified address of I2C memory
  * @param  pData : pointer of the data to write
  * @param  TarAddr : I2C data memory address to write
  * @param  DeviceAddr : Device Address of selected memory
  * @retval EEPROMEX enum status
*/
int32_t M24_i2c_WriteByte(uint8_t * pData, const uint32_t TarAddr)
{
  int32_t status;
  
  /* Before calling this function M24xx must be ready, here is a check to detect an issue */
  if (M24_i2c_IsDeviceReady(_DevAddr, MIN_TRIALS ) != M24_OK)
 	{
 	  return M24_TIMEOUT;
 	}

  if (_DevAddr == 0xA8)
    status = BSP_I2C_WriteReg(_DevAddr, TarAddr, pData, 1 );
  else
    status = BSP_I2C_WriteReg16(_DevAddr, TarAddr, pData, 1 );

  while(M24_i2c_IsDeviceReady(_DevAddr, MIN_TRIALS ) != M24_OK);
  
  return status;
}

/**
  * @brief  Write Page
  * @param  pData : pointer of the data to write
  * @param  TarAddr : I2C data memory address to write
  * @param  DeviceAddr : Device Address of selected memory
  * @param  PageSize : Size of the page of selected memory
  * @param  NbByte : number of bytes to write
  * @retval EEPROMEX enum status
*/
int32_t M24_i2c_WritePage(uint8_t * pData, const uint32_t TarAddr, const uint16_t NByte)
{
	int32_t status;
	if (M24_i2c_IsDeviceReady(_DevAddr, MIN_TRIALS) != M24_OK)
	{
		return M24_TIMEOUT;
	}


	if (_DevAddr == 0xA8)
		status = BSP_I2C_WriteReg(_DevAddr, TarAddr, pData, _PageSize);
	else
		status = BSP_I2C_WriteReg16(_DevAddr, TarAddr, pData, _PageSize);

	while(M24_i2c_IsDeviceReady(_DevAddr, MIN_TRIALS) != M24_OK);

	return status;
}

/**
  * @brief  Write N data bytes starting from specified I2C Address
  * @param  pData : pointer of the data to write
  * @param  TarAddr : I2C data memory address to write
  * @param  DeviceAddr : Device Address of selected memory
  * @param  PageSize : Size of the page of selected memory
  * @param  NbByte : number of bytes to write
  * @retval EEPROMEX enum status
  */

int32_t M24_i2c_WriteData(uint8_t * pData, const uint32_t TarAddr, const uint16_t Size )
{

  uint32_t 	iNumberOfPage;
  int32_t 	status = M24_OK;
  uint32_t 	targetAddress = TarAddr;

  uint8_t 	device_addr = _DevAddr;
  uint32_t 	writeAddr = TarAddr;
  uint16_t	pageCount;
  uint16_t	pageOffset;
  uint16_t	startPage;
  uint32_t 	restOfData = Size;
  uint32_t	bytesToWrite;


  //Do we have device online?
  if (M24_i2c_IsDeviceReady(_DevAddr, MIN_TRIALS) != M24_OK)
  {
 	return M24_TIMEOUT;
  }

  if(_DevAddr != 0xA8 && targetAddress + Size > 65535) {

 	  if (M24_i2c_IsDeviceReady(_DevAddr + 2, MIN_TRIALS) != M24_OK)
 	  	  {
 	  		return M24_TIMEOUT;
 	  	  }

   }

  pageCount = 0;
  startPage = 0;
  if(Size > _PageSize) {

	 startPage = Size / _PageSize;

  }
  iNumberOfPage = targetAddress / _PageSize;
  pageOffset = targetAddress % _PageSize;

  uint8_t* pBuf = pData;

  while(restOfData) {

	  if(pageCount == 0) {

		  bytesToWrite = (_PageSize - pageOffset) > Size ? Size : _PageSize - pageOffset;

	  } else {

		  pageOffset = 0;
		  bytesToWrite = (_PageSize - restOfData) > 0 ? restOfData : (_PageSize - restOfData);

	  }


      if (_DevAddr == 0xA8)

    	  status = BSP_I2C_WriteReg(_DevAddr, targetAddress,  pBuf, bytesToWrite);

      else {

    	  writeAddr = targetAddress;
    	  if(writeAddr > 65535) {

    		  writeAddr = writeAddr - 65536;
    		  device_addr = _DevAddr + 2;
    	  }

    	  status = BSP_I2C_WriteReg16(device_addr, writeAddr,  pBuf, bytesToWrite);
      }

      if(status != M24_OK) break;

      HAL_Delay(10);
      targetAddress += bytesToWrite;
      pBuf += bytesToWrite;
      pageCount++;
      restOfData -= bytesToWrite;

      while(M24_i2c_IsDeviceReady(_DevAddr, MIN_TRIALS) != M24_OK);

  }

  return status;

}



//int32_t __M24_i2c_WriteData(M24_Object_t *pObj, uint8_t * pData, const uint32_t TarAddr,
//                                          const uint16_t PageSize, const uint16_t Size )
//{
//
//  uint32_t iNumberOfPage;
//  int32_t status = M24_OK;
//  uint32_t targetAddress = TarAddr;
//
//  uint8_t device_addr = pObj->IO.Address;
//  uint32_t writeAddr = TarAddr;
//
//
//
//  /*to handle dynamically start writing address*/
//  if (targetAddress >= PageSize)
//    {
//     iNumberOfPage =  Size / PageSize;
//    if ((targetAddress % PageSize) > 0)
//      {
//      iNumberOfPage += 1;
//      }
//    }
//  else  {
//
//	  iNumberOfPage = ( targetAddress + Size ) / PageSize;
//
//  }
//
//  uint32_t iRemainder = ( targetAddress + Size ) % PageSize;
//  uint8_t * pageIndex = pData;
//
// if (iRemainder>0)
//  {
//    iNumberOfPage += 1;
//  }
//
// if (  pObj->IO.IsReady( pObj->IO.Address, MIN_TRIALS ) != M24_OK )
// {
//	return M24_TIMEOUT;
// }
//
// if(pObj->IO.Address != 0xA8 && targetAddress + Size > 65535) {
//
//	  if (  pObj->IO.IsReady( pObj->IO.Address + 1, MIN_TRIALS ) != M24_OK )
//	  	  {
//	  		return M24_TIMEOUT;
//	  	  }
//
//  }
//
//  if (targetAddress == 0)       /*If target address from which read/write will be done starts from 0*/
//  {
//    for (int index = 0;index < iNumberOfPage;index++)
//    {
//       uint32_t iSize = PageSize;
//       if (index+1 == iNumberOfPage)     /*For aligning last page of eeprom*/
//        iSize = iRemainder;
//
//
//       if (pObj->IO.Address == 0xA8)
//        status = pObj->IO.WriteReg(pObj->IO.Address, targetAddress,  pageIndex, iSize );
//       else {
//
//    	   status = pObj->IO.WriteReg16(pObj->IO.Address, targetAddress,  pageIndex, iSize );
//       }
//
//
//        pObj->IO.Delay(6);
//        targetAddress += iSize;
//        pageIndex += iSize;
//        while (  pObj->IO.IsReady( pObj->IO.Address, MIN_TRIALS ) != M24_OK ) {};
//
//     }
//     return status;
//  }
//  else
//  {
//    for(int index = 0;index < iNumberOfPage;index++)
//    {
//
//       uint32_t iSize = PageSize;
//       if (index == 0) /*For aligning first page*/
//        {
//          if (targetAddress <= PageSize)
//            iSize = (PageSize - targetAddress)>0? (PageSize - targetAddress) : PageSize;
//          else
//            iSize = PageSize - (targetAddress % PageSize);
//
//          	//if(iSize > Size) iSize = Size;
//
//        }
//
//        if (index+1 == iNumberOfPage) /*For aligning last page of eeprom*/
//          iSize = iRemainder;
//
//       if (pObj->IO.Address == 0xA8)
//
//    	   status = pObj->IO.WriteReg(pObj->IO.Address, targetAddress,  pageIndex, iSize );
//       else {
//
//    	   if(targetAddress > 65535) {
//    		   writeAddr = targetAddress - 65535;
//    		   device_addr = pObj->IO.Address + 1;
//    	   }
//    	   status = pObj->IO.WriteReg16(device_addr, writeAddr,  pageIndex, iSize );
//       }
//
//        pObj->IO.Delay(6);
//        targetAddress += iSize;
//        pageIndex += iSize;
//
//        while (  pObj->IO.IsReady( pObj->IO.Address, MIN_TRIALS ) != M24_OK ) {};
//
//     }
//     return status;
//  }
//}


/**
  * @brief  Write Identification Page
  * @param  pData : pointer of the data to write
  * @param  TarAddr : I2C data memory address to write
  * @param  PageSize : Size of the page of selected memory
  * @param  DeviceAddr : Device Address of selected memory
  * @param  NbByte : number of bytes to write
  * @retval EEPROMEX enum status
  */
int32_t M24_i2c_WriteID(uint8_t * pData, const uint32_t TarAddr, const uint16_t NbByte ){
  int32_t 	status;
  uint16_t  Devaddr;
  uint16_t 	mem_addr;
  uint16_t  count;
  uint16_t 	temp;
  uint16_t  bitcount;
  uint16_t 	mask = 0 ;

  if (M24_i2c_IsDeviceReady(_DevAddr, MIN_TRIALS) != M24_OK)
  {
    return M24_TIMEOUT;
  }

  uint8_t *pdata_index = (uint8_t *)pData;
  Devaddr = _DevAddr;
  bitcount = BITCOUNT;
  temp = _PageSize;
  mem_addr = ( uint16_t )TarAddr;
  Devaddr |= 1 << IDMASK;                  /* Set fifth bit of Device address */
  while ( temp / ( 1 << bitcount ) != 0 ){  /* Generate mask for address*/
   mask |= ( 1 << (bitcount - 1) );
   bitcount++;
  }
  mem_addr &= mask;                        /* Mask address address according to pagesize*/  
  count = _PageSize - mem_addr % _PageSize;  /* Calculate available space in the ID page */
  if ( NbByte <= count ){  /* Address byte is not aligned with page and no byte must be less than available byte in ID page*/
    if (_DevAddr == 0xA8)
        status = BSP_I2C_WriteReg(Devaddr, mem_addr,  pdata_index, NbByte );
    else
        status = BSP_I2C_WriteReg16(Devaddr, mem_addr,  pdata_index, NbByte );
  }
  else 
    return M24_ERROR;   /* Return error if above two condtions does'nt met */

  while(M24_i2c_IsDeviceReady(_DevAddr, MIN_TRIALS) != M24_OK);

  return status; 
}

/**
  * @brief  Read Identification Page 
  * @param  pData : pointer of the data to store
  * @param  TarAddr : I2C data memory address to write
  * @param  PageSize : Size of the page of selected memory
  * @param  DeviceAddr : Device Address of selected memory
  * @param  NbByte : number of bytes to read
  * @retval EEPROMEX enum status
  */
int32_t M24_i2c_ReadID(uint8_t * pData, const uint32_t TarAddr, const uint16_t NbByte )
{
  uint16_t 	Devaddr;
  uint16_t 	mem_addr;
  uint16_t 	count;
  uint16_t	temp;
  uint8_t 	bitcount;
  uint16_t 	mask;

  if (M24_i2c_IsDeviceReady(_DevAddr, MIN_TRIALS) != M24_OK)
    return M24_TIMEOUT;

  mem_addr = ( uint16_t )TarAddr;
  Devaddr = _DevAddr;
  mask = 0;
  bitcount = BITCOUNT;
  temp = _PageSize;
  Devaddr |= 1 << IDMASK ;
  while ( temp / ( 1 << bitcount ) != 0 )
  {
   mask |= ( 1 << (bitcount - 1) );
   bitcount++;
  }
  mem_addr &= mask;
  count = _PageSize - mem_addr % _PageSize;
  if (NbByte <= count)
    if (_DevAddr == 0xA8)
      return BSP_I2C_ReadReg( Devaddr, TarAddr, pData, NbByte);
    else
      return BSP_I2C_ReadReg16( Devaddr, TarAddr, pData, NbByte);
  else
    if (_DevAddr == 0xA8)
      return BSP_I2C_ReadReg(Devaddr, TarAddr,  pData, count);
    else
      return BSP_I2C_ReadReg16(Devaddr, TarAddr,  pData, count);
}

/**
  * @brief  Permanentaly lock identification page of memory 
  * @param  DeviceAddr : Device Address of selected memory
  * @retval BSP status
  */
int32_t M24_i2c_LockID()
{  
  int32_t status = M24_OK;
  uint8_t  Devaddr = _DevAddr | 1 << IDMASK;
  uint8_t lid_cmd = LOCKID;

  if (M24_i2c_IsDeviceReady(_DevAddr, MIN_TRIALS) != M24_OK)
  {
    return M24_TIMEOUT;
  }
  
  if (_DevAddr == 0xA8)
    status = BSP_I2C_WriteReg(Devaddr, ADDRLID_16, &lid_cmd, 1 );
  else
    status = BSP_I2C_WriteReg16(Devaddr, ADDRLID_16, &lid_cmd, 1 );

  while(M24_i2c_IsDeviceReady(_DevAddr, MIN_TRIALS) != M24_OK);
  
  return status;
  
}

/**
  * @brief  Reads the lock status
  * @param  pData : pointer of the data to read
  * @param  DeviceAddr : DeviceAddress of the selected memory
  * @retval EEPROMEX enum status
  */
int32_t M24_i2c_LockStatus(uint8_t * pData )
{
  uint8_t dummybyte = 0xAA;
  uint8_t  Devaddr = _DevAddr | 1 << IDMASK;

  if (M24_i2c_IsDeviceReady(_DevAddr, MIN_TRIALS) != M24_OK)
  {
    return M24_TIMEOUT;

  }
  if (BSP_I2C_Send(Devaddr, &dummybyte , 1) == M24_OK)
  {
    *pData = 0;
  }
  else 
  {
    *pData = 1;  
  } 

  while(M24_i2c_IsDeviceReady(_DevAddr, MIN_TRIALS) != M24_OK);

  return M24_OK;
}

/* Erase a page in the EEPROM Memory
 * @page is the number of page to erase
 * In order to erase multiple pages, just use this function in the for loop
 */
uint32_t M24_i2c_PageErase(uint16_t page)
{

	// create a buffer to store the reset values
	uint8_t 	pBuf[MAX_PAGE_SIZE];
	uint32_t 	targetAddress;
	uint8_t 	device_addr = _DevAddr;
	uint32_t 	writeAddr;

	int32_t status = M24_OK;

//	// calculate the memory address based on the page number
//	int paddrposition = log(_PageSize)/log(2);
//	targetAddress = page << paddrposition;

	targetAddress = page * _PageSize;

	//Do we have device online?
	if (M24_i2c_IsDeviceReady(_DevAddr, MIN_TRIALS) != M24_OK)
	{
		return M24_TIMEOUT;
	}

	if(_DevAddr != 0xA8 && targetAddress + _PageSize > 65535) {

	  if (M24_i2c_IsDeviceReady(_DevAddr + 2, MIN_TRIALS) != M24_OK)
		  {
			return M24_TIMEOUT;
		  }

	}

	memset(pBuf, 0xff, _PageSize);

	if (_DevAddr == 0xA8)

	  status = BSP_I2C_WriteReg(_DevAddr, targetAddress,  pBuf, _PageSize);

	else {

	  writeAddr = targetAddress;
	  if(writeAddr > 65535) {

		  writeAddr = writeAddr - 65536;
		  device_addr = _DevAddr + 2;
	  }

	  status = BSP_I2C_WriteReg16(device_addr, writeAddr,  pBuf, _PageSize);
	}

	HAL_Delay (5);  // write cycle delay


	return status;

	// write the data to the EEPROM
	//HAL_I2C_Mem_Write(EEPROM_I2C, EEPROM_ADDR, MemAddress, 2, data, _PageSize, 1000);

}

uint32_t M24_i2c_EraseEntireChip() {

	int32_t status = M24_OK;
	uint16_t curPage = 0;
	while(status == M24_OK
			&& curPage <= _PageCount) {

		status = M24_i2c_PageErase(curPage);
		curPage++;

	}

	return status;

}


////////////////////////////////////////////////////////////////////////
// LOW LEVEL

/**
  * @brief  Write a value in a register of the device through BUS.
  * @param  DevAddr Device address on Bus.
  * @param  Reg    The target register address to write
  * @param  pData  Pointer to data buffer to write
  * @param  Length Data Length
  * @retval BSP status
  */

int32_t BSP_I2C_WriteReg(uint16_t DevAddr, uint16_t Reg, uint8_t *pData, uint16_t Length)
{
  int32_t ret = M24_OK;

  if (HAL_I2C_Mem_Write(HI2C, DevAddr,Reg, I2C_MEMADD_SIZE_8BIT,pData, Length, M24_I2C_POLL_TIMEOUT) != HAL_OK)
  {
    if (HAL_I2C_GetError(HI2C) == HAL_I2C_ERROR_AF)
    {
      ret = M24_ERROR_BUS_ACKNOWLEDGE_FAILURE;
    }
    else
    {
      ret =  M24_ERROR_PERIPH_FAILURE;
    }
  }
  return ret;
}

/**
  * @brief  Read a register of the device through BUS
  * @param  DevAddr Device address on Bus.
  * @param  Reg    The target register address to read
  * @param  pData  Pointer to data buffer to read
  * @param  Length Data Length
  * @retval BSP status
  */
int32_t BSP_I2C_ReadReg(uint8_t devAddr, uint16_t Reg, uint8_t *pData, uint16_t Length)
{
  int32_t ret = M24_OK;

  if (HAL_I2C_Mem_Read(HI2C, devAddr, Reg, I2C_MEMADD_SIZE_8BIT, pData, Length, M24_I2C_POLL_TIMEOUT) != HAL_OK)
  {
    if (HAL_I2C_GetError(HI2C) == HAL_I2C_ERROR_AF)
    {
      ret = M24_ERROR_BUS_ACKNOWLEDGE_FAILURE;
    }
    else
    {
      ret = M24_ERROR_PERIPH_FAILURE;
    }
  }
  return ret;
}

/**

  * @brief  Write a value in a register of the device through BUS.
  * @param  DevAddr Device address on Bus.
  * @param  Reg    The target register address to write

  * @param  pData  Pointer to data buffer to write
  * @param  Length Data Length
  * @retval BSP statu
  */
int32_t BSP_I2C_WriteReg16(uint8_t devAddr, uint16_t Reg, uint8_t *pData, uint16_t Length)
{
  int32_t ret = M24_OK;

  if (HAL_I2C_Mem_Write(HI2C, devAddr, Reg, I2C_MEMADD_SIZE_16BIT, pData, Length, M24_I2C_POLL_TIMEOUT) != HAL_OK)
  {
    if (HAL_I2C_GetError(HI2C) == HAL_I2C_ERROR_AF)
    {
      ret = M24_ERROR_BUS_ACKNOWLEDGE_FAILURE;
    }
    else
    {
      ret = M24_ERROR_PERIPH_FAILURE;
    }
  }
  return ret;
}

/**
  * @brief  Read registers through a bus (16 bits)
  * @param  DevAddr: Device address on BUS
  * @param  Reg: The target register address to read
  * @param  Length Data Length
  * @retval BSP status
  */
int32_t BSP_I2C_ReadReg16(uint8_t devAddr, uint16_t Reg, uint8_t *pData, uint16_t Length)
{
  int32_t ret = M24_OK;

  if (HAL_I2C_Mem_Read(HI2C, devAddr, Reg, I2C_MEMADD_SIZE_16BIT, pData, Length, M24_I2C_POLL_TIMEOUT) != HAL_OK)
  {
    if (HAL_I2C_GetError(HI2C) != HAL_I2C_ERROR_AF)
    {
      ret =  M24_ERROR_BUS_ACKNOWLEDGE_FAILURE;
    }
    else
    {
      ret =  M24_ERROR_PERIPH_FAILURE;
    }
  }
  return ret;
}

/**
  * @brief  Send an amount width data through bus (Simplex)
  * @param  DevAddr: Device address on Bus.
  * @param  pData: Data pointer
  * @param  Length: Data length
  * @retval BSP status
  */
int32_t BSP_I2C_Send(uint8_t devAddr, uint8_t *pData, uint16_t Length) {

  int32_t ret = M24_OK;

  if (HAL_I2C_Master_Transmit(HI2C, devAddr, pData, Length, M24_I2C_POLL_TIMEOUT) != HAL_OK)
  {
    if (HAL_I2C_GetError(HI2C) != HAL_I2C_ERROR_AF)
    {
      ret = M24_ERROR_BUS_ACKNOWLEDGE_FAILURE;
    }
    else
    {
      ret =  M24_ERROR_PERIPH_FAILURE;
    }
  }

  return ret;
}

/**
  * @brief  Receive an amount of data through a bus (Simplex)
  * @param  DevAddr: Device address on Bus.
  * @param  pData: Data pointer
  * @param  Length: Data length
  * @retval BSP status
  */
int32_t BSP_I2C_Recv(uint8_t devAddr, uint8_t *pData, uint16_t Length) {

	int32_t ret = M24_OK;

  if (HAL_I2C_Master_Receive(HI2C, devAddr, pData, Length, M24_I2C_POLL_TIMEOUT) != HAL_OK)
  {
    if (HAL_I2C_GetError(HI2C) != HAL_I2C_ERROR_AF)
    {
      ret = M24_ERROR_BUS_ACKNOWLEDGE_FAILURE;
    }
    else
    {
      ret = M24_ERROR_PERIPH_FAILURE;
    }
  }
  return ret;
}



/**
  * @brief  Return system tick in ms
  * @retval Current HAL time base time stamp
  */
int32_t BSP_GetTick(void) {
  return HAL_GetTick();
}


