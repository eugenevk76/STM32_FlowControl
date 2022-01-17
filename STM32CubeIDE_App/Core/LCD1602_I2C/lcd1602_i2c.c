
#ifndef LCD1602_I2C_LCD1602_I2C_C_
#define LCD1602_I2C_LCD1602_I2C_C_

#include "lcd1602_i2c.h"


#define PIN_RS    (1 << 0)
#define PIN_EN    (1 << 2)
#define BACKLIGHT (1 << 3)

#define LCD_DELAY_MS 5

LCD lcdConfig;
uint8_t currentBacklight = BACKLIGHT;

//Service
//uint8_t I2C_Scan(I2C_HandleTypeDef* i2c_handle, uint8_t start_addr);


HAL_StatusTypeDef LCD_Init(I2C_HandleTypeDef* hi2c_lcd, uint8_t addr) {

	HAL_StatusTypeDef result;

	uint8_t lcd_addr = 0;
	if(addr == 0) {
		lcd_addr = I2C_Scan(hi2c_lcd, 0);
		if(lcd_addr == 128)
			return HAL_ERROR;
	} else {

		lcd_addr = addr;
	}

	lcdConfig.hi2c_lcd = hi2c_lcd;
	lcdConfig.address = lcd_addr;

	// 4-bit mode, 2 lines, 5x7 format
    result = LCD_SendCommand(0b00110000);
    // display & cursor home (keep this!)
    result = LCD_SendCommand(0b00000010);
    // display on, right shift, underline off, blink off
    result = LCD_SendCommand(0b00001100);
    // clear display (optional here)
    result = LCD_SendCommand(0b00000001);

    return result;

}

HAL_StatusTypeDef LCD_SendInternal(uint8_t data, uint8_t flags) {
    HAL_StatusTypeDef res;
    for(;;) {
        res = HAL_I2C_IsDeviceReady(lcdConfig.hi2c_lcd, lcdConfig.address << 1, 1,
                                    HAL_MAX_DELAY);
        if(res == HAL_OK)
            break;
    }

    uint8_t up = data & 0xF0;
    uint8_t lo = (data << 4) & 0xF0;

    uint8_t data_arr[4];
//    data_arr[0] = up|flags|BACKLIGHT|PIN_EN;
//    data_arr[1] = up|flags|BACKLIGHT;
//    data_arr[2] = lo|flags|BACKLIGHT|PIN_EN;
//    data_arr[3] = lo|flags|BACKLIGHT;

    data_arr[0] = up|flags|currentBacklight|PIN_EN;
    data_arr[1] = up|flags|currentBacklight;
    data_arr[2] = lo|flags|currentBacklight|PIN_EN;
    data_arr[3] = lo|flags|currentBacklight;

    res = HAL_I2C_Master_Transmit(lcdConfig.hi2c_lcd, lcdConfig.address << 1, data_arr,
                                  sizeof(data_arr), HAL_MAX_DELAY);
    HAL_Delay(LCD_DELAY_MS);
    return res;
}

HAL_StatusTypeDef LCD_SendCommand(uint8_t cmd) {
    return LCD_SendInternal(cmd, 0);
}

HAL_StatusTypeDef LCD_SendData(uint8_t data) {
    return LCD_SendInternal(data, PIN_RS);
}


HAL_StatusTypeDef LCD_SendString(char *str) {

	HAL_StatusTypeDef res = HAL_OK;

	while(*str && res == HAL_OK) {
        res = LCD_SendData((uint8_t)(*str));
        str++;
    }

	return res;

}

//Commands

HAL_StatusTypeDef LCD_Cls() {

	return LCD_SendCommand(0b00000001);

}

HAL_StatusTypeDef LCD_Set_LED(LED_StateType state) {

	if(state == LED_ON) {
		currentBacklight = 8;
		return LCD_SendCommand(0b00000000);
	} else {
		currentBacklight = 0;
		return LCD_SendCommand(0b00000000);
	}

}

LED_StateType LCD_Get_LED() {

	if(currentBacklight > 0)
		return LED_ON;

	return LED_OFF;

}

HAL_StatusTypeDef LCD_SetAddr(uint8_t addr) {

	lcdConfig.address = addr;
	return HAL_OK;

}

uint8_t LCD_GetAddr() {

	return lcdConfig.address;

}

// Puts the cursor at the desired row and column. The row can be either 0 or 1, and the column can vary from 0 to 15.
void LCD_SetCursor(int row, int col)
{
    switch (row)
    {
        case 0:
            col |= 0x80;
            break;
        case 1:
            col |= 0xC0;
            break;
    }
    LCD_SendCommand(col);
}

void LCD_Off() {

	LCD_SendCommand(0b00001000);
}

void LCD_On() {

	LCD_SendCommand(0b00001100);

}

void LCD_Cursor_On() {

	LCD_SendCommand(0b00001111);

}

void LCD_Cursor_Off() {

	LCD_SendCommand(0b00001100);

}


//Service

uint8_t I2C_Scan(I2C_HandleTypeDef* i2c_handle, uint8_t start_addr)
{

	for(uint8_t i = start_addr; i < 128; i++)
	{
		if (HAL_I2C_IsDeviceReady(i2c_handle, i << 1, 1, HAL_MAX_DELAY) == HAL_OK) return i;
 	}

	return 128;

}

#endif /* LCD1602_I2C_LCD1602_I2C_C_ */
