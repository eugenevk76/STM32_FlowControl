
#include "main.h"
#include "menu.h"
#include "flow_control.h"
#include "string.h"
#include "EEPROM.h"
#include "stdio.h"

extern RTC_HandleTypeDef hrtc;

char time[11] = {0,};
char date[11] = {0,};
char disp_String[16] = {0, };

static uint32_t logCurrentRecordAddr;
static uint16_t logCurrentNumber;
static uint16_t _lastSessionConsumption;

//////////////////////////////////////////////
// PRIVATE PROTOTYPES

//Working with date
void getDateTime();
void setTime(uint8_t hour, uint8_t minute, uint8_t second);
void setDate(uint8_t day, uint8_t month, uint8_t year);
uint8_t maxDaysInMonth(uint8_t month, uint8_t year);

//Settings and log
void readSettings();
void saveSettings();
void setDataFromLog();
void putRecordToLog();

//Main views
void show_idle_baseview();
void show_ext_view_1();
void show_ext_view_2();
void updateView(AppStates toState);

//Helpers
void setFlowParameters(FC_RecordInfo* recordData);
void resetByType(bool hardReset);

//------------------------------------------------------------

void processIdleState() {

	if(Key_pressed != KEY_NONE) {

		if(CurrentAppState == APP_STATE_IDLE) {

			LCD_Set_LED(LED_ON);
			CurrentAppState = APP_STATE_IDLE_LIGHTON;
			viewCnt = 0;
			Key_pressed = KEY_NONE;

		} else {

			switch (Key_pressed) {
				case KEY_MENU:
					updateView(APP_STATE_MENU_SHOW);
					MENU_Run(KEY_NONE);
					return;
					break;
				case KEY_NEXT:
					switch (CurrentAppState) {
						case APP_STATE_IDLE_LIGHTON:
							updateView(APP_STATE_EXT_VIEW_1);
							break;
						case APP_STATE_EXT_VIEW_1:
							updateView(APP_STATE_EXT_VIEW_2);
							break;
						case APP_STATE_EXT_VIEW_2:
							updateView(APP_STATE_IDLE_LIGHTON);
						default:
							break;
					}

				default:
					break;
			}

		}

	}

	showIdleScreen();


}

//Show one of three main view
void showIdleScreen() {


	switch (CurrentAppState) {

		case APP_STATE_IDLE_LIGHTON:
		case APP_STATE_IDLE:
			if(viewCnt > TIME_LED_IS_ON_MSEC) { //Light off

				 LCD_Set_LED(LED_OFF);
				 CurrentAppState = APP_STATE_IDLE;
				 viewCnt = 0;
			}
			show_idle_baseview();
			break;


		case APP_STATE_EXT_VIEW_1:
			show_ext_view_1();
			break;

		case APP_STATE_EXT_VIEW_2:
			show_ext_view_2();
			break;

		default:
			break;

	}

}

//This screen shows current water consumption and
//average consumption l/min
//
void show_idle_baseview() {

	if(viewCnt%1000 != 0) {
		return;
	}

	if(!data_idleBase_changed) return;
	data_idleBase_changed = false;

	if(view_changed) LCD_Cls();
	view_changed = false;

	LCD_SetCursor(0, 0);
	sprintf((char*) disp_String, "CUR %06d L.", currentWaterConsumption);
	LCD_SendString(disp_String);
	LCD_SetCursor(1, 0);

	int tmpInt1 = averageWaterConsumption; // Get the integer (678).
	float tmpFrac = averageWaterConsumption - tmpInt1;      // Get fraction (0.0123).
	int tmpInt2 = (uint8_t)(tmpFrac * 100);  // Turn into integer (123).

	// Print as parts, note that you need 0-padding for fractional bit.
	sprintf ((char*) disp_String, "AVG %03d.%02d L/MIN", tmpInt1, tmpInt2);

	//sprintf((char*) disp_String, "AVG %06f L/MIN", averageWaterConsumption);

	LCD_SendString(disp_String);

}

//Here we will show last water consumption
//and how many liters we have to alarm
void show_ext_view_1() {

	if(viewCnt%1000 != 0) {
		return;
	}

	if(viewCnt > TIME_LED_IS_ON_MSEC) {

		updateView(APP_STATE_IDLE_LIGHTON);
		return;
	}

	if(!data_idle1_changed) return;
	data_idle1_changed = false;

	if(view_changed) LCD_Cls();
	view_changed = false;

	LCD_SetCursor(0, 0);
	sprintf((char*) disp_String, "LAST %05d L.", _lastSessionConsumption);
	LCD_SendString(disp_String);

	LCD_SetCursor(1, 0);
	sprintf((char*) disp_String, "REST %05d L.",
			currentWaterConsumption > fc_settings.alarmLitres ? 0 : fc_settings.alarmLitres - currentWaterConsumption);
	LCD_SendString(disp_String);

}

//This screen shows current date and time
//
void show_ext_view_2() {

	if(viewCnt%1000 != 0) {
		return;
	}

	if(viewCnt > TIME_LED_IS_ON_MSEC) {

		updateView(APP_STATE_IDLE_LIGHTON);
		return;
	}

	if(!data_idle2_changed) return;
	data_idle2_changed = false;

	if(view_changed) LCD_Cls();
	view_changed = false;

	getDateTime();
	LCD_SetCursor(0, 3);
	LCD_SendString(date);
	LCD_SetCursor(1, 6);
	LCD_SendString(time);

}



void flowControlSetup() {

	LCD_Cls();
	LCD_SetCursor(0, 0);
	LCD_SendString("Initializing...");

	CurrentAppState = APP_STATE_START;

	if(HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_DR1) != TIME_IS_SET) {

		getDateTime();
		setDate(01, 01, 21);
		setTime(12, 0, 0);
		opSetDate();
		opSetTime();
	}

	readSettings();
	setDataFromLog();

	CurrentAppState = APP_STATE_IDLE;
	data_idleBase_changed = true;
	view_changed = true;

	//needMakeRecord = true;

	sessionLitres = 0;

	//averageWaterConsumption = 0;

	HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR1, TIME_IS_SET);
}

void readSettings() {

	//EEPROM_Init();
	//EEPROM_Test();

	while(!EEPROM_ReadData(0, (uint8_t*)&fc_settings, sizeof(fc_settings)-1));

	if(fc_settings.settingsKey != SETTINGS_KEY) {

		//Set first start
		fc_settings.settingsKey = SETTINGS_KEY;
		fc_settings.keysBeep = false;
		fc_settings.pulsesPerLitre = 80;
		fc_settings.alarmLitres = 10000;

		opSetLPP();
		opSetAlarm();

		fc_settings.reserve[0] = 1;
		fc_settings.reserve[1] = 2;
		fc_settings.reserve[2] = 3;

		saveSettings();

	}

}

void saveSettings() {

	while(!EEPROM_WriteData(0, (uint8_t*)&fc_settings, sizeof(fc_settings)-1));
	//while(!EEPROM_WriteDataByBytes(0, (uint8_t*)&fc_settings, sizeof(fc_settings)));
}


void setDataFromLog() {


	EEPROM_Init();

	FC_RecordInfo recordData = {0};
	uint16_t recordSize = sizeof(recordData);

	uint32_t curAddr = LOG_START_ADDR;
	bool parametersIsSet = false;

	while((curAddr + recordSize - 1) < LOG_END_ADDR) {

		while(!EEPROM_ReadData(curAddr, (uint8_t*)&recordData, recordSize - 1));

		if(recordData.headRecordKey == LOG_ACTIVE_HEAD_RECORD_KEY
				&& recordData.tailRecordKey == LOG_TAIL_RECORD_KEY) {

			logCurrentRecordAddr = curAddr;
			parametersIsSet = true;
			setFlowParameters(&recordData);

			break;
		}

		if(recordData.headRecordKey == LOG_REGULAR_HEAD_RECORD_KEY
				&& recordData.tailRecordKey == LOG_TAIL_RECORD_KEY) {

			curAddr += (recordSize - 1);
			setFlowParameters(&recordData);
			parametersIsSet = true;
			logCurrentRecordAddr = curAddr;
			continue;

		}

		//no active records, create new with current data (probably first start)
		if(!parametersIsSet) {
			averageWaterConsumption = 0;
			totalWaterConsumption = 0;
			sessionLitres = 0;
		}
		putRecordToLog();
		break;

	}

}

void setFlowParameters(FC_RecordInfo* recordData) {

	averageWaterConsumption = Bytes2float(&(recordData->avgConsumption));
	totalWaterConsumption = recordData->totalConsumption;
	currentWaterConsumption = recordData->recordConsumption;
	logCurrentNumber = recordData->recordNumber;

}

void putRecordToLog() {

	RTC_DateTypeDef gDate;
	RTC_TimeTypeDef gTime;
	DateTimeType logDateTime;

	FC_RecordInfo recordData = {0};
	uint16_t recordSize = sizeof(recordData);

	if(!logCurrentRecordAddr)
		logCurrentRecordAddr = LOG_START_ADDR;
	else {

		//deactivate previous record
		while(!EEPROM_ReadData(logCurrentRecordAddr, (uint8_t*)&recordData, recordSize-1));
		if(recordData.headRecordKey == LOG_ACTIVE_HEAD_RECORD_KEY) {
			recordData.headRecordKey = LOG_REGULAR_HEAD_RECORD_KEY;
			while(!EEPROM_WriteData(logCurrentRecordAddr, (uint8_t*)&recordData, recordSize-1));
			//while(!EEPROM_WriteDataByBytes(logCurrentRecordAddr, (uint8_t*)&recordData, recordSize));
		}

		logCurrentRecordAddr += (recordSize - 1);

		logCurrentNumber++;
	}

	if(logCurrentRecordAddr + recordSize - 1 > LOG_END_ADDR) {
		logCurrentRecordAddr = LOG_START_ADDR;
	}

	HAL_RTC_GetTime(&hrtc, &gTime, RTC_FORMAT_BIN);
	HAL_RTC_GetDate(&hrtc, &gDate, RTC_FORMAT_BIN);

	logDateTime.day 	= gDate.Date;
	logDateTime.month 	= gDate.Month;
	logDateTime.year 	= gDate.Year;
	logDateTime.hour 	= gTime.Hours;
	logDateTime.minute 	= gTime.Minutes;
	logDateTime.second 	= gTime.Seconds;

	recordData.headRecordKey = LOG_ACTIVE_HEAD_RECORD_KEY;

	recordData.recordNumber = logCurrentNumber;
	recordData.recordDate = logDateTime;

	recordData.sessionConsumption = sessionLitres;
	recordData.recordConsumption = currentWaterConsumption;
	float2Bytes(&recordData.avgConsumption, averageWaterConsumption);
	recordData.totalConsumption = totalWaterConsumption;

	recordData.tailRecordKey = LOG_TAIL_RECORD_KEY;

	while(!EEPROM_WriteData(logCurrentRecordAddr, (uint8_t*)&recordData, recordSize-1));
	//while(!EEPROM_WriteDataByBytes(logCurrentRecordAddr, (uint8_t*)&recordData, recordSize));

	_lastSessionConsumption = sessionLitres;
	sessionLitres = 0;
	needMakeRecord = false;

}

void opSetTime() {

	uint8_t curPos;
	uint8_t prevCnt = 0;

	RTC_DateTypeDef gDate;
	RTC_TimeTypeDef gTime;

	uint8_t hour;
	uint8_t minute;

	Key_pressed = KEY_NONE;

	AppStates OldState = CurrentAppState;
	CurrentAppState = APP_STATE_SET_TIME;
	LCD_Cls();
	LCD_SetCursor(0, 1);
	LCD_SendString("SET TIME");

	HAL_RTC_GetTime(&hrtc, &gTime, RTC_FORMAT_BIN);
	HAL_RTC_GetDate(&hrtc, &gDate, RTC_FORMAT_BIN);

	hour = gTime.Hours;
	minute = gTime.Minutes;

	curPos = 0;
	bool changed = false;
	uint8_t elapse = 0;

	while(1) {

		if(Key_pressed == KEY_CHANGE) {
			elapse = 0;
			Key_pressed = KEY_NONE;
			changed = true;
			if(curPos == 0) {
				hour++;
				if(hour > 23) hour = 0;
			} else {
				minute++;
				if(minute > 59) minute = 0;
			}
		}

		if(Key_pressed == KEY_NEXT) {

			Key_pressed = KEY_NONE;
			curPos++;
			if(curPos > 1) curPos = 0;

		}

		if(Key_pressed == KEY_MENU) {
			Key_pressed = KEY_NONE;
			break;
		}

		if(Key_pressed == KEY_OK) {
			Key_pressed = KEY_NONE;

			setTime(hour, minute, 0);

			break;

		}

		if(!changed && prevCnt == secCnt) continue;

		prevCnt = secCnt;
		elapse++;

		if(OldState != APP_STATE_START && elapse > DELAY_OUT) break;

		if (changed || secCnt%2 == 0) {
			sprintf((char*) time, "%02d:%02d", hour, minute);
			changed = false;
		} else {
			if(curPos == 0)
				sprintf((char*) time, "  :%02d", minute);
			else
				sprintf((char*) time, "%02d:  ", hour);
		}

		LCD_SetCursor(1, 6);
		LCD_SendString(time);

	}

	CurrentAppState = OldState;
	//MENU_Update();


}

void opSetDate() {

	uint8_t curPos;
	uint8_t prevCnt = 0;

	RTC_DateTypeDef gDate;
	RTC_TimeTypeDef gTime;

	uint8_t day;
	uint8_t month;
	uint8_t year;

	Key_pressed = KEY_NONE;

	AppStates OldState = CurrentAppState;
	CurrentAppState = APP_STATE_SET_DATE;
	LCD_Cls();
	LCD_SetCursor(0, 1);
	LCD_SendString("SET DATE");

	HAL_RTC_GetTime(&hrtc, &gTime, RTC_FORMAT_BIN);
	HAL_RTC_GetDate(&hrtc, &gDate, RTC_FORMAT_BIN);

	day = gDate.Date;
	month = gDate.Month;
	year = gDate.Year;

	uint8_t maxDays;

	curPos = 0;
	bool changed = false;
	uint8_t elapse = 0;

	maxDays = maxDaysInMonth(month, year);

	while(1) {

		if(Key_pressed == KEY_CHANGE) {
			elapse = 0;
			Key_pressed = KEY_NONE;
			changed = true;
			if(curPos == 0) { //change day
				day++;
				if(day > maxDays) day = 1;
			}
			if(curPos == 1) { // change month
				month++;
				if(month > 12) month = 1;
				maxDays = maxDaysInMonth(month, year);
				if(day > maxDays) day = maxDays;
			}
			if(curPos == 2) { // change year

				year++;
				if(year > 99) year = 0;
				maxDays = maxDaysInMonth(month, year);
				if(day > maxDays) day = maxDays;

			}

		}

		if(Key_pressed == KEY_NEXT) {

			Key_pressed = KEY_NONE;
			curPos++;
			if(curPos > 2) curPos = 0;

		}

		if(Key_pressed == KEY_MENU) {
			Key_pressed = KEY_NONE;
			break;
		}

		if(Key_pressed == KEY_OK) {
			Key_pressed = KEY_NONE;

			setDate(day, month, year);

			break;

		}

		if(!changed && prevCnt == secCnt) continue;

		prevCnt = secCnt;
		elapse++;

		if(OldState != APP_STATE_START && elapse > DELAY_OUT) break;

		if (changed || secCnt%2 == 0) {
			sprintf((char*) date, "%02d-%02d-%04d", day, month, 2000 + year);
			changed = false;
		} else {
			if(curPos == 0)
				sprintf((char*) date, "  -%02d-%04d", month, 2000 + year);
			if(curPos == 1)
				sprintf((char*) date, "%02d-  -%04d", day, 2000 + year);
			if(curPos == 2)
				sprintf((char*) date, "%02d-%02d-    ", day, month);
		}

		LCD_SetCursor(1, 3);
		LCD_SendString(date);

	}

	CurrentAppState = OldState;
	//MENU_Update();


}


void opSetAlarm() {

	AppStates OldState = CurrentAppState;
	CurrentAppState = APP_STATE_SET_ALARM;
	LCD_Cls();
	LCD_Cursor_On();
	LCD_SetCursor(0, 1);
	LCD_SendString("SET ALARM COUNT");

	uint8_t cursorPosition = 7;

	uint8_t prevCnt = 0;
	Key_pressed = KEY_NONE;
	bool changed = false;
	uint8_t elapse = 0;
	uint16_t delta = 1;

	uint32_t 	alarmLitres = fc_settings.alarmLitres;

	while(1) {

		if(Key_pressed == KEY_CHANGE) {
			elapse = 0;
			Key_pressed = KEY_NONE;
			changed = true;
			alarmLitres += delta;
			if(alarmLitres > 65535) alarmLitres = 0;
		}

		if(Key_pressed == KEY_NEXT) {
			elapse = 0;
			Key_pressed = KEY_NONE;
			changed = true;
			cursorPosition++;

			if(cursorPosition > 7) cursorPosition = 3;

			delta = 1;
			for(uint8_t i = 0; i < (7 - cursorPosition); i++)
				delta = 10 * delta;
		}

		if(Key_pressed == KEY_MENU) {
			Key_pressed = KEY_NONE;
			break;
		}

		if(Key_pressed == KEY_OK) {
			Key_pressed = KEY_NONE;

			fc_settings.alarmLitres = alarmLitres;
			saveSettings();

			break;

		}

		if(!changed && prevCnt == secCnt) continue;

		prevCnt = secCnt;
		elapse++;

		if(elapse > DELAY_OUT) break;

		sprintf((char*) disp_String, "%05d L", alarmLitres);
		changed = false;

		LCD_SetCursor(1, 3);
		LCD_SendString(disp_String);

		LCD_SetCursor(1, cursorPosition);


	}

	CurrentAppState = OldState;
	LCD_Cursor_Off();
	//MENU_Update();

}

////////////////////////////////////
//explicit setting pulses per liter
void opSetLPP() {

	AppStates OldState = CurrentAppState;
	CurrentAppState = APP_STATE_SET_LPP;
	LCD_Cls();
	LCD_SetCursor(0, 1);
	LCD_SendString("SET PULSES/LITRE");
	LCD_Cursor_On();


	uint8_t cursorPosition = 4;
	uint16_t delta = 1;
	uint8_t prevCnt = 0;
	Key_pressed = KEY_NONE;
	bool changed = false;
	uint8_t elapse = 0;

	uint16_t pulsesPerLitre = fc_settings.pulsesPerLitre;

	while(1) {

		if(Key_pressed == KEY_CHANGE) {
			elapse = 0;
			Key_pressed = KEY_NONE;
			changed = true;
			pulsesPerLitre += delta;
			if(pulsesPerLitre > 9999) pulsesPerLitre = 0;
		}

		if(Key_pressed == KEY_NEXT) {
			elapse = 0;
			Key_pressed = KEY_NONE;
			changed = true;
			cursorPosition++;
			if(cursorPosition > 4) cursorPosition = 1;
			delta = 1;
			for(uint8_t i = 0; i < (4 - cursorPosition); i++)
				delta = 10 * delta;
		}

		if(Key_pressed == KEY_MENU) {
			Key_pressed = KEY_NONE;
			break;
		}

		if(Key_pressed == KEY_OK) {
			Key_pressed = KEY_NONE;

			fc_settings.pulsesPerLitre = pulsesPerLitre;
			saveSettings();

			break;

		}

		if(!changed && prevCnt == secCnt) continue;

		prevCnt = secCnt;
		elapse++;

		if(elapse > DELAY_OUT) break;

		sprintf((char*) disp_String, "%04d PPL", pulsesPerLitre);
		changed = false;

		LCD_SetCursor(1, 1);
		LCD_SendString(disp_String);
		LCD_SetCursor(1, cursorPosition);

	}

	LCD_Cursor_Off();
	CurrentAppState = OldState;
	//MENU_Update();

}

////////////////////////////////////
//setting pulses per liter by metering
void opCountLPP() {

	AppStates OldState = CurrentAppState;
	CurrentAppState = APP_STATE_SET_LPP;
	LCD_Cls();
	LCD_SetCursor(0, 1);
	LCD_SendString("COUNT, OK-ACCEPT");

	uint8_t prevCnt = 0;
	Key_pressed = KEY_NONE;
	uint8_t elapse = 0;

	pulses_calibration = 0;
	//uint32_t 	alarmLitres;
	calibration_mode = true;
	uint16_t prev_pulses = 0;

	while(1) {

		if(Key_pressed == KEY_CHANGE) {
			elapse = 0;
			Key_pressed = KEY_NONE;
		}

		if(Key_pressed == KEY_NEXT) {
			elapse = 0;
			Key_pressed = KEY_NONE;
		}

		if(Key_pressed == KEY_MENU) {
			Key_pressed = KEY_NONE;
			CurrentAppState = OldState;
			calibration_mode = false;
			return;
			break;
		}

		if(Key_pressed == KEY_OK) {

			Key_pressed = KEY_NONE;

			if(pulses_calibration > 0) {

				fc_settings.pulsesPerLitre = pulses_calibration;
				saveSettings();
				calibration_mode = false;

			}
			CurrentAppState = OldState;
			return;

			break;

		}

		if(prevCnt == secCnt) continue;

		prevCnt = secCnt;
		if(prev_pulses == pulses_calibration)
			elapse++;
		else
			elapse = 0;

		prev_pulses = pulses_calibration;

		if(elapse > DELAY_OUT) {
			calibration_mode = false;
			break;
		}

		sprintf((char*) disp_String, "PULSES %05d", pulses_calibration);

		LCD_SetCursor(1, 1);
		LCD_SendString(disp_String);

	}

	//CurrentAppState = OldState;

}


void opViewLog() {

	AppStates OldState = CurrentAppState;
	FC_RecordInfo recordData = {0};
	uint16_t recordSize = sizeof(recordData);


	CurrentAppState = APP_STATE_VIEW_LOG;
	LCD_Cls();
	LCD_SetCursor(0, 0);
	LCD_SendString("VIEW LOG:");
	LCD_SetCursor(1, 0);
	LCD_SendString("<<=CHG; NEXT=>");

	uint8_t prevCnt = 0;
	Key_pressed = KEY_NONE;
	bool changed = false;
	bool move_forward = false;
	bool last_prev = false;
	bool last_next = true;
	uint8_t elapse = 0;

	uint16_t curViewLogAddr = logCurrentRecordAddr;
	uint32_t numOfRecords = (LOG_END_ADDR - LOG_START_ADDR) / recordSize;

	//uint32_t 	alarmLitres;

	while(1) {

		if(Key_pressed == KEY_CHANGE) { //Previous record
			elapse = 0;
			Key_pressed = KEY_NONE;
			move_forward = false;
			if(!last_prev) {
				changed = true;
				curViewLogAddr -= (recordSize - 1);
				if(curViewLogAddr < LOG_START_ADDR)
					curViewLogAddr = LOG_START_ADDR + (numOfRecords - 1) * recordSize;
			}

		}

		if(Key_pressed == KEY_NEXT) {
			elapse = 0;
			Key_pressed = KEY_NONE;
			//changed = true;
			move_forward = true;
			if(!last_next) {
				changed = true;
				curViewLogAddr += (recordSize - 1);
				if(curViewLogAddr > LOG_END_ADDR) curViewLogAddr = LOG_START_ADDR;
			}
		}

		if(Key_pressed == KEY_MENU) {
			Key_pressed = KEY_NONE;
			CurrentAppState = OldState;
			return;
			break;
		}

		if(Key_pressed == KEY_OK) {
			Key_pressed = KEY_NONE;
			CurrentAppState = OldState;
			return;
			break;
		}

		if(!changed && prevCnt == secCnt) continue;

		prevCnt = secCnt;
		elapse++;

		if(elapse > DELAY_OUT) break;

		if(changed) {

			last_next = false;
			last_prev = false;
			bool show_record = true;

			while(!EEPROM_ReadData(curViewLogAddr, (uint8_t*)&recordData, recordSize - 1));

			if(recordData.headRecordKey == LOG_ACTIVE_HEAD_RECORD_KEY
				&& recordData.tailRecordKey == LOG_TAIL_RECORD_KEY) {

				if(move_forward) last_next = true;
					else last_prev = true;

			}

			if(recordData.headRecordKey != LOG_REGULAR_HEAD_RECORD_KEY
					|| recordData.tailRecordKey != LOG_TAIL_RECORD_KEY) {

				if(move_forward) last_next = true;
					else last_prev = true;
				show_record = false;
				changed = false;

			}

			if(show_record) {

				//1st row
				sprintf((char*) disp_String, "%02d-%02d-%02d %02d:%02d",
						recordData.recordDate.day,
						recordData.recordDate.month,
						recordData.recordDate.year,
						recordData.recordDate.hour,
						recordData.recordDate.minute);
				LCD_SetCursor(0, 0);
				LCD_SendString(disp_String);

				//2nd row
				sprintf((char*) disp_String, "RC:%02dL TOT:%04dL",
						recordData.sessionConsumption,
						recordData.recordConsumption);
				LCD_SetCursor(1, 0);
				LCD_SendString(disp_String);
				changed = false;

			}

		}

	}

	CurrentAppState = OldState;

}


void opCntReset() {


	resetByType(false);

}

void opReset() {

	resetByType(true);

}

void resetByType(bool hardReset) {

	AppStates OldState = CurrentAppState;
	CurrentAppState = APP_STATE_RESET;


	LCD_SetCursor(0, 0);
	if(hardReset)
		LCD_SendString("FULL RESET!     ");
	else
		LCD_SendString("NEW FILTER RESET");
	LCD_SetCursor(1, 0);
	LCD_SendString("OK TO CONTINUE  ");

	uint8_t prevCnt = 0;
	Key_pressed = KEY_NONE;
	bool changed = false;
	uint8_t elapse = 0;

	while(1) {

		if(Key_pressed == KEY_CHANGE) {
			elapse = 0;
			Key_pressed = KEY_NONE;
		}

		if(Key_pressed == KEY_NEXT) {
			elapse = 0;
			Key_pressed = KEY_NONE;
		}

		if(Key_pressed == KEY_MENU) {
			Key_pressed = KEY_NONE;
			CurrentAppState = OldState;
			return;
			break;
		}

		if(Key_pressed == KEY_OK) {
			Key_pressed = KEY_NONE;
			changed = true;
			//break;

		}

		if(!changed && prevCnt == secCnt) continue;

		prevCnt = secCnt;
		elapse++;

		if(elapse > DELAY_OUT) break;

		if(!changed) continue;

		if(hardReset) {

			//temp
			EEPROM_EraseChip();
			//__set_FAULTMASK(1);// Запрещаем все маскируемые прерывания
			NVIC_SystemReset();// Программный сброс
			break;

		} else {

			currentWaterConsumption = 0;
			putRecordToLog();
			break;

		}

	}

	CurrentAppState = OldState;
}



void setTime(uint8_t hour, uint8_t minute, uint8_t second) {

	RTC_TimeTypeDef sTime = {0};


	sTime.Hours = hour;
	sTime.Minutes = minute;
	sTime.Seconds = second;
	sTime.SubSeconds = 0x0;
	sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
	sTime.StoreOperation = RTC_STOREOPERATION_RESET;
	if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN) != HAL_OK)
	{
		Error_Handler();
	}



}

void setDate(uint8_t day, uint8_t month, uint8_t year) {

	RTC_DateTypeDef sDate = {0};
	sDate.WeekDay = RTC_WEEKDAY_MONDAY;
	sDate.Month = month;
	sDate.Date = day;
	sDate.Year = year;

	if (HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BIN) != HAL_OK) {
		Error_Handler();
	}

}

void getDateTime() {

	RTC_DateTypeDef gDate;
	RTC_TimeTypeDef gTime;

	HAL_RTC_GetTime(&hrtc, &gTime, RTC_FORMAT_BIN);
	HAL_RTC_GetDate(&hrtc, &gDate, RTC_FORMAT_BIN);



	if(secCnt%2 == 0)
		sprintf((char*) time, "%02d:%02d", gTime.Hours, gTime.Minutes);
	else
		sprintf((char*) time, "%02d %02d", gTime.Hours, gTime.Minutes);

	//sprintf((char*) time, "%02d:%02d:%02d", gTime.Hours, gTime.Minutes);


	sprintf((char*) date, "%02d-%02d-%02d", gDate.Date, gDate.Month, 2000 + gDate.Year);
	//sprintf((char*) date, "%02d-%02d-20%02d", gDate.Date, gDate.Month, gDate.Year);

}

void updateView(AppStates toState) {


	viewCnt = 0;
	view_changed = true;
	CurrentAppState = toState;
	Key_pressed = KEY_NONE;

	switch (toState) {
		case APP_STATE_IDLE:
		case APP_STATE_IDLE_LIGHTON:
			data_idleBase_changed = true;
			break;
		case APP_STATE_EXT_VIEW_1:
			data_idle1_changed = true;
			break;
		case APP_STATE_EXT_VIEW_2:
			data_idle2_changed = true;
			break;
		default:
			data_idleBase_changed = true;
			break;
	}


}


uint8_t maxDaysInMonth(uint8_t month, uint8_t year) {

	uint16_t longMonths = 0b0000101011010101;
	uint8_t maxDays;

	if(longMonths & (1<<month-1))
		maxDays = 31;
	else
		maxDays = 30;

	if(month == 2) {

		bool isLeapYear = ((year%4) == 0);

		if(isLeapYear)
			maxDays = 29;
		else
			maxDays = 28;
	}

	return maxDays;

}

