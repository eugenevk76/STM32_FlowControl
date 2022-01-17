
#ifndef INC_FLOW_CONTROL_H_
#define INC_FLOW_CONTROL_H_

#include "main.h"
#include "stdbool.h"
#include "lcd1602_i2c.h"

#define SIZEOF(a)      (sizeof(a))

#define SETTINGS_KEY 		0x32F1BBCC
#define TIME_IS_SET 		0x32F1
#define TIME_LED_IS_ON_MSEC 20000 //How many msec will on the light without actions
#define DELAY_OUT 			20 //How many seconds we wait reaction in sub-menu

typedef struct {

	uint8_t year;
	uint8_t month;
	uint8_t day;
	uint8_t hour;
	uint8_t minute;
	uint8_t second;

} DateTimeType;

typedef struct {

	uint8_t			headRecordKey;
	uint16_t		recordNumber;
	DateTimeType 	recordDate;
	uint32_t 		recordConsumption;
	uint32_t 		totalConsumption;
	uint16_t 		sessionConsumption;
	uint8_t			avgConsumption[4];
	uint8_t			tailRecordKey;

} FC_RecordInfo;

typedef struct {

	uint32_t	settingsKey;
	bool 		keysBeep;
	uint16_t	pulsesPerLitre;
	uint32_t 	alarmLitres;
	uint32_t	reserve[3];

} FC_Settings;

FC_Settings fc_settings;

#define LOG_ACTIVE_HEAD_RECORD_KEY	0xA4
#define LOG_REGULAR_HEAD_RECORD_KEY	0x24
#define LOG_TAIL_RECORD_KEY			0x25
#define LOG_START_ADDR				SIZEOF(FC_Settings) + 1
#define LOG_END_ADDR				131072

//typedef enum {
//
//	UNDEFINED_VIEW_ID = 0,
//	BASE_VIEW_ID,
//	EXT1_VIEW_ID,
//	EXT2_VIEW_ID
//
//} ViewID;



float averageWaterConsumption;
uint16_t currentWaterConsumption;
uint32_t totalWaterConsumption;
uint16_t sessionLitres;
uint16_t pulses_calibration;

bool needMakeRecord;


///////////////////////////////
// Menu actions

void processIdleState();
void showIdleScreen();
void opSetTime();
void opSetDate();
void opSetAlarm();
void opSetLPP();
void opViewLog();
void opReset();
void opCountLPP();
void opCntReset();
void flowControlSetup();

#endif /* INC_FLOW_CONTROL_H_ */
