
#ifndef LCD1602_I2C_LCD1602_I2C_H_
#define LCD1602_I2C_LCD1602_I2C_H_

#include "main.h"

typedef struct {

	I2C_HandleTypeDef* hi2c_lcd;
	uint8_t address;

} LCD;

typedef enum {

	LED_ON = 0,
	LED_OFF = 1

} LED_StateType;

HAL_StatusTypeDef LCD_Init(I2C_HandleTypeDef* hi2c_lcd, uint8_t addr);
HAL_StatusTypeDef LCD_SendCommand(uint8_t cmd);
HAL_StatusTypeDef LCD_SendData(uint8_t data);
HAL_StatusTypeDef LCD_SendString(char *str);

//Comands
HAL_StatusTypeDef LCD_Cls();
HAL_StatusTypeDef LCD_Set_LED(LED_StateType state);
LED_StateType LCD_Get_LED();
void LCD_SetCursor(int row, int col);
void LCD_Off();
void LCD_On();
void LCD_Cursor_On();
void LCD_Cursor_Off();

//Properties
HAL_StatusTypeDef LCD_SetAddr(uint8_t addr);
uint8_t LCD_GetAddr();

uint8_t I2C_Scan(I2C_HandleTypeDef* i2c_handle, uint8_t start_addr);

#endif /* LCD1602_I2C_LCD1602_I2C_H_ */
