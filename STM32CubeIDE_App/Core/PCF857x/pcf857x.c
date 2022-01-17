/*
 * pcf857x.c
 *
 *  Created on: 19 квіт. 2018 р.
 *      Author: Andriy
 */
#include <pcf857x.h>

static bool 				_is8575;
static uint16_t 			_bufferPins;
static uint8_t 				_data[2];
static PCF857x_TypeDef		_error;

PCF857x_TypeDef pcf857x_Init(uint16_t value_init, bool is8575)
{
	 _is8575 = is8575;

	if(PCF857X_IS_READY() != HAL_OK)
	{
		_error = PCF857x_I2C_ERROR;
		return PCF857x_I2C_ERROR;
	}
	else
	{
		if(_is8575)
		{
			if(pcf857x_Write16(value_init) != PCF857x_OK)
			{
				return _error;
			}
		}
		else
		{
			if(pcf857x_Write8(value_init) != PCF857x_OK)
			{
				return _error;
			}
		}

		_error = PCF857x_OK;
		return PCF857x_OK;
	}
}

uint8_t pcf857x_Read8(void)
{
	if(_is8575)
	{
		return (uint8_t) pcf857x_Read16();
	}

	if( PCF857X_READ(&_data[0], 1) != HAL_OK)
	{
		_error = PCF857x_I2C_ERROR;
	}
	else
	{
		_error = PCF857x_OK;
	}

	return _data[0];
}

uint16_t pcf857x_Read16(void)
{
	if(!_is8575)
	{
		return (uint16_t) pcf857x_Read8();
	}

	if(PCF857X_READ(_data, 2) != HAL_OK)
	{
		_error = PCF857x_I2C_ERROR;
	}
	else
	{
		_error = PCF857x_OK;
	}

	return ((uint16_t)_data[1] << 8) | _data[0];
}

bool pcf857x_Read(uint8_t pin)
{
	if(_is8575)
	{
		if(pin > 15)
		{
			_error = PCF857x_PIN_ERROR;
			return 0;
		}

		return _BIT_IS_SET(pcf857x_Read16(), pin);
	}
	else
	{
		if(pin > 7)
		{
			_error = PCF857x_PIN_ERROR;
			return 0;
		}

		return _BIT_IS_SET(pcf857x_Read8(), pin);
	}
}

PCF857x_TypeDef pcf857x_Write8(uint8_t value)
{
	_bufferPins &= 0xFF00;
	_bufferPins |= value;
	_data[0] = _bufferPins & 0x00FF;
	_data[1] = (_bufferPins & 0xFF00) >> 8;

	if(PCF857X_WRITE(_data, 1) != HAL_OK)
	{
		_error = PCF857x_I2C_ERROR;
		return PCF857x_I2C_ERROR;
	}
	else
	{
		_error = PCF857x_OK;
		return PCF857x_OK;
	}
}

PCF857x_TypeDef pcf857x_Write16(uint16_t value)
{
	if(!_is8575)
	{
		_error = PCF857x_FUN_ERROR;
		return PCF857x_FUN_ERROR;
	}

	_bufferPins = value;
	_data[0] = _bufferPins & 0x00FF;
	_data[1] = (_bufferPins & 0xFF00) >> 8;

	if(PCF857X_WRITE(_data, 2) != HAL_OK)
	{
		_error = PCF857x_I2C_ERROR;
		return PCF857x_I2C_ERROR;
	}
	else
	{
		_error = PCF857x_OK;
		return PCF857x_OK;
	}
}

PCF857x_TypeDef pcf857x_Write(uint8_t pin, bool value)
{
	if(_is8575)
	{
		if(pin > 15)
	    {
	      return PCF857x_PIN_ERROR;
	    }
	}
	else
	{
		if(pin > 7)
		{
			return PCF857x_PIN_ERROR;
		}
	}

	if(value)
	{
		_BIT_SET(_bufferPins, pin);
	}
	else
	{
		_BIT_CLEAR(_bufferPins, pin);
	}

	if(_is8575)
	{
		if(pcf857x_Write16(_bufferPins) != PCF857x_OK)
		{
			_error = PCF857x_I2C_ERROR;
			return PCF857x_I2C_ERROR;
		}
	}
	else
	{
		if(pcf857x_Write8(_bufferPins) != PCF857x_OK)
		{
			_error = PCF857x_I2C_ERROR;
			return PCF857x_I2C_ERROR;
		}
	}

	_error = PCF857x_OK;
	return PCF857x_OK;
}

PCF857x_TypeDef pcf857x_Toggle(uint8_t pin)
{
	if(_is8575)
	{
		if(pin > 15)
		{
			_error = PCF857x_PIN_ERROR;
			return PCF857x_PIN_ERROR;
		}
	}
	else
	{
		if(pin > 7)
		{
			_error = PCF857x_PIN_ERROR;
			return PCF857x_PIN_ERROR;
		}
	}

	_BIT_TOGGLE(_bufferPins, pin);

	if(_is8575)
	{
		if(pcf857x_Write16(_bufferPins) != PCF857x_OK)
		{
			_error = PCF857x_I2C_ERROR;
			return PCF857x_I2C_ERROR;
		}
	}
	else
	{
		if(pcf857x_Write8(_bufferPins) != PCF857x_OK)
		{
			_error = PCF857x_I2C_ERROR;
			return PCF857x_I2C_ERROR;
		}
	}

	return PCF857x_OK;
}

PCF857x_TypeDef pcf857x_ToggleAll(void)
{
	_bufferPins = ~_bufferPins;

	if(_is8575)
	{
		if(pcf857x_Write16(_bufferPins) != PCF857x_OK)
		{
			_error = PCF857x_I2C_ERROR;
			return PCF857x_I2C_ERROR;
		}
	}
	else
	{
		if(pcf857x_Write8(_bufferPins) != PCF857x_OK)
		{
			_error = PCF857x_I2C_ERROR;
			return PCF857x_I2C_ERROR;
		}
	}

	return PCF857x_OK;
}

PCF857x_TypeDef pcf857x_ShiftRight(uint8_t n)
{
	if(_is8575)
	{
		if (n == 0 || n > 15 )
		{
			_error = PCF857x_VAL_ERROR;
			return PCF857x_VAL_ERROR;
		}
	}
	else
	{
		if (n == 0 || n > 7 )
		{
			_error = PCF857x_VAL_ERROR;
			return PCF857x_VAL_ERROR;
		}
	}

	_bufferPins >>= n;

	if(_is8575)
	{
		if(pcf857x_Write16(_bufferPins) != PCF857x_OK)
		{
			_error = PCF857x_I2C_ERROR;
			return PCF857x_I2C_ERROR;
		}
	}
	else
	{
		if(pcf857x_Write8(_bufferPins) != PCF857x_OK)
		{
			_error = PCF857x_I2C_ERROR;
			return PCF857x_I2C_ERROR;
		}
	}

	return PCF857x_OK;
}

PCF857x_TypeDef pcf857x_ShiftLeft(uint8_t n)
{
	if(_is8575)
	{
		if (n == 0 || n > 15 )
		{
			_error = PCF857x_VAL_ERROR;
			return PCF857x_VAL_ERROR;
		}
	}
	else
	{
		if (n == 0 || n > 7 )
		{
			_error = PCF857x_VAL_ERROR;
			return PCF857x_VAL_ERROR;
		}
	}

	_bufferPins <<= n;

	if(_is8575)
	{
		if(pcf857x_Write16(_bufferPins) != PCF857x_OK)
		{
			_error = PCF857x_I2C_ERROR;
			return PCF857x_I2C_ERROR;
		}
	}
	else
	{
		if(pcf857x_Write8(_bufferPins) != PCF857x_OK)
		{
			_error = PCF857x_I2C_ERROR;
			return PCF857x_I2C_ERROR;
		}
	}

	return PCF857x_OK;
}

PCF857x_TypeDef pcf857x_RotateRight(uint8_t n)
{
	if(_is8575)
	{
		uint8_t r = n & 15;

		_bufferPins = (_bufferPins >> r) | (_bufferPins << (16 - r));

	    if(pcf857x_Write16(_bufferPins) != PCF857x_OK)
		{
	    	_error = PCF857x_I2C_ERROR;
	    	return PCF857x_I2C_ERROR;
		}
	}
	else
	{
	    uint8_t r = n & 7;

	    _bufferPins = (_bufferPins >> r) | (_bufferPins << (8 - r));

	    if(pcf857x_Write8(_bufferPins) != PCF857x_OK)
		{
	    	_error = PCF857x_I2C_ERROR;
	    	return PCF857x_I2C_ERROR;
		}
	  }

	return PCF857x_OK;
}

PCF857x_TypeDef pcf857x_RotateLeft(uint8_t n)
{
	if(_is8575)
	{
		if(pcf857x_RotateRight(16 - (n & 15)) != PCF857x_OK)
		{
			_error = PCF857x_I2C_ERROR;
			return PCF857x_I2C_ERROR;
		}
	}
	else
	{
		if(pcf857x_RotateRight(8 - (n & 7)) != PCF857x_OK)
		{
			_error = PCF857x_I2C_ERROR;
			return PCF857x_I2C_ERROR;
		}
	}

	return PCF857x_OK;
}

PCF857x_TypeDef pcf857x_ResetInterruptPin(void)
{
	if(_is8575)
	{
		pcf857x_Read16();
	}
	else
	{
		pcf857x_Read8();
	}

	return _error;
}

PCF857x_TypeDef pcf857x_GetLastError(void)
{
  int err = _error;
  _error = 0;
  return err;
}
