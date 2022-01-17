/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32g0xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stdbool.h"
/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */
//typedef enum {
//	false = 0,
//	true = 1
//} bool;

typedef enum {

	KEY_NONE = 0,
	KEY_OK,
	KEY_CHANGE,
	KEY_NEXT,
	KEY_MENU

} Menu_Keys;

enum {
	LOW = 0,
	HIGH
};

typedef enum {

	APP_STATE_START = 1,
	APP_STATE_IDLE,
	APP_STATE_IDLE_LIGHTON,
	APP_STATE_EXT_VIEW_1,
	APP_STATE_EXT_VIEW_2,
	APP_STATE_MENU_SHOW,
	APP_STATE_SET_TIME,
	APP_STATE_SET_DATE,
	APP_STATE_SET_ALARM,
	APP_STATE_SET_LPP,
	APP_STATE_VIEW_LOG,
	APP_STATE_RESET,
	APP_STATE_FIRST_START

} AppStates;


AppStates CurrentAppState;

bool Key_press_event;
Menu_Keys Key_pressed;
Menu_Keys Key_preview;

uint32_t blinkCnt;
uint32_t pressedCnt;
uint32_t debounceCnt;
uint32_t viewCnt;
uint8_t	 secCnt;

bool data_idleBase_changed;
bool data_idle1_changed;
bool data_idle2_changed;
bool view_changed;

bool needMakeRecord;
bool calibration_mode;

void alarm();

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

#define bit(b) (1UL << (b))

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define EXTI_BTN_Pin GPIO_PIN_2
#define EXTI_BTN_GPIO_Port GPIOF
#define EXTI_BTN_EXTI_IRQn EXTI2_3_IRQn
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
