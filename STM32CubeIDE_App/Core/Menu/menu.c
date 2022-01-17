
#include "menu.h"
#include "flow_control.h"
#include "string.h"

//MAIN MENU
//        NAME,      	NEXT,       	PREVIOUS,    	PARENT,      CHILD,      SELECT,     ENTER,	OPTIONS,                          				TEXT
MENU_ITEM(m_set_time,  	m_set_date,		m_reset,		NULL_MENU,   NULL_MENU,  opSetTime,  NULL, 	DIM_ON | TIMEOUT_ON | SEL_ENA | SET_TIME_ID,  	"SET TIME");
MENU_ITEM(m_set_date, 	m_set_alarm,   	m_set_time,     NULL_MENU,   NULL_MENU,  opSetDate,  NULL, 	DIM_ON | TIMEOUT_ON | SEL_ENA | SET_DATE_ID, 	"SET DATE");
MENU_ITEM(m_set_alarm, 	m_set_lpp,   	m_set_date,     NULL_MENU,   NULL_MENU,  opSetAlarm, NULL, 	DIM_ON | TIMEOUT_ON | SEL_ENA | SET_ALARM_ID, 	"SET ALARM");
MENU_ITEM(m_set_lpp,  	m_count_lpp,	m_set_alarm,   	NULL_MENU,   NULL_MENU,  opSetLPP,   NULL,  DIM_ON | TIMEOUT_ON | SEL_ENA | SET_LPP_ID,  	"SET L/PLS");
MENU_ITEM(m_count_lpp,  m_view_log,		m_set_lpp,   	NULL_MENU,   NULL_MENU,  opCountLPP, NULL,  DIM_ON | TIMEOUT_ON | SEL_ENA | COUNT_LPP_ID,  	"CALIBRATE L/PLS");
MENU_ITEM(m_view_log, 	m_reset_cnt,    m_count_lpp,    NULL_MENU,   NULL_MENU,  opViewLog,  NULL, 	DIM_ON | TIMEOUT_ON | SEL_ENA | VIEW_LOG_ID, 	"VIEW LOG");
MENU_ITEM(m_reset_cnt, 	m_reset,     	m_view_log,    	NULL_MENU,   NULL_MENU,  opCntReset, NULL, 	DIM_ON | TIMEOUT_ON | SEL_ENA | RESET_CNT_ID, 	"NEW FILTER RST");
MENU_ITEM(m_reset,  	m_set_time,   	m_reset_cnt,    NULL_MENU,   NULL_MENU,  opReset,    NULL, 	DIM_ON | TIMEOUT_ON | SEL_ENA | RESET_ID,  		"FULL RESET");

extern HAL_StatusTypeDef LCD_SendString(char *str);

static uint32_t menu_timeout = 0;
static uint8_t select_ena = 0;
static Menu_Item_t* CurrentMenuItem;

Menu_Item_t NULL_MENU = {0};

bool needToRefresh = true;

void MENU_Timeout();
void backHome();
void setMenuTimeOut();

void MENU_Navigate(Menu_Item_t* NewMenu)
{
    if ((NewMenu == &NULL_MENU) || (NewMenu == 0))
	return;

    CurrentMenuItem = NewMenu;

//    menu_timeout = 0;
//    if (CurrentMenuItem->Options & TIMEOUT_ON)
//	    menu_timeout = HAL_GetTick();
//    setMenuTimeOut();

//    if (CurrentMenuItem->Options & SEL_ENA)
//        select_ena = 1;
//    else
//    	select_ena = 0;

    //enter_idx = 0;

    MENU_Update();

}

void MENU_Update() {

	if(CurrentAppState != APP_STATE_MENU_SHOW) {
		return;
	}

	LCD_Cls();

    if (CurrentMenuItem != 0 && CurrentMenuItem != &NULL_MENU) {

    	LCD_SetCursor(0, 0);
    	LCD_SendString(">");
    	LCD_SetCursor(0, 1);
    	LCD_SendString(CurrentMenuItem->Text);

    }

    if (CurrentMenuItem->Next != 0 && CurrentMenuItem->Next != &NULL_MENU) {

    	LCD_SetCursor(1, 1);
    	LCD_SendString(CurrentMenuItem->Next->Text);

    }

    if (CurrentMenuItem->Options & SEL_ENA)
         select_ena = 1;
     else
     	select_ena = 0;

    setMenuTimeOut();

}

void MENU_Run()
{
    if (Key_pressed > 0)
    {
    	//key_pressed = key_val;

        switch(Key_pressed)
		{
			case KEY_NEXT:
				Key_pressed = KEY_NONE;
				MENU_Navigate(MENU_NEXT);
			break;

			case KEY_CHANGE:
				Key_pressed = KEY_NONE;
				MENU_Navigate(MENU_PREVIOUS);
			break;

			case KEY_MENU:
				Key_pressed = KEY_NONE;
				backHome();
			break;

			case KEY_OK:

				if (CurrentMenuItem->SelectCallback != NULL)
				{
					if (select_ena)
					{
						needToRefresh = true;
						CurrentMenuItem->SelectCallback();
					}
				}
			break;


		}
    } else {

    	if(CurrentMenuItem == NULL) {
    		CurrentMenuItem = (Menu_Item_t*)&m_set_time;

    	}

    	if(needToRefresh) {
    		MENU_Update();
    		needToRefresh = false;
    	}

    }

//    //SELECT Function runs in background
//    if (CurrentMenuItem->SelectCallback != NULL_FUNC)
//    {
//    	if (select_ena)
//    	{
//    	    CurrentMenuItem->SelectCallback();
//    	}
//    }


    MENU_Timeout();

}


void MENU_Timeout() {

	if(!menu_timeout) return;

	if((HAL_GetTick() - menu_timeout) > 10000) {
		backHome();
	}

}

void setMenuTimeOut() {

	menu_timeout = 0;
  	if(CurrentMenuItem != NULL && (CurrentMenuItem->Options & TIMEOUT_ON)) {
  		menu_timeout = HAL_GetTick();
  	}

}

void backHome() {

	viewCnt = 0;
	data_idleBase_changed = true;
	CurrentAppState = APP_STATE_IDLE_LIGHTON;
	needToRefresh = true;

}

