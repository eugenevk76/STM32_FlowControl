
#ifndef MENU_MENU_H_
#define MENU_MENU_H_

#include "main.h"

#define DIM_ON 		1
#define TIMEOUT_ON 	2
#define SEL_ENA 	4

#define SET_TIME_ID 	8
#define SET_DATE_ID 	16
#define SET_ALARM_ID 	32
#define SET_LPP_ID		64
#define VIEW_LOG_ID		128
#define RESET_ID		256
#define COUNT_LPP_ID	512
#define RESET_CNT_ID	1024


typedef const struct Menu_Item
{
    const struct Menu_Item *Next;     /**< Pointer to the next menu item of this menu item */
    const struct Menu_Item *Previous; /**< Pointer to the previous menu item of this menu item */
    const struct Menu_Item *Parent;   /**< Pointer to the parent menu item of this menu item */
    const struct Menu_Item *Child;    /**< Pointer to the child menu item of this menu item */
    void (*SelectCallback)(void);     /**< Pointer to the optional menu-specific select callback of this menu item */
    void (*EnterCallback)(void);      /**< Pointer to the optional menu-specific enter callback of this menu item */
    const uint32_t Options;
    const char Text[];                /**< Menu item text to pass to the menu display callback function */
} Menu_Item_t;


#define MENU_ITEM(Name, Next, Previous, Parent, Child, SelectFunc, EnterFunc, Options, Text) \
    extern Menu_Item_t Next;     \
    extern Menu_Item_t Previous; \
    extern Menu_Item_t Parent;   \
    extern Menu_Item_t Child;    \
    Menu_Item_t Name = {&Next, &Previous, &Parent, &Child, SelectFunc, EnterFunc, Options, Text}


//#define MENU_PARENT     (&Menu_GetCurrentMenu()->Parent)
#define MENU_PARENT     (CurrentMenuItem->Parent)

/** Relative navigational menu entry for \ref Menu_Navigate(), to move to the menu child. */
//#define MENU_CHILD      (&Menu_GetCurrentMenu()->Child)
#define MENU_CHILD      (CurrentMenuItem->Child)

/** Relative navigational menu entry for \ref Menu_Navigate(), to move to the next linked menu item. */
//#define MENU_NEXT           (&Menu_GetCurrentMenu()->Next)
#define MENU_NEXT       (CurrentMenuItem->Next)
/** Relative navigational menu entry for \ref Menu_Navigate(), to move to the previous linked menu item. */
//#define MENU_PREVIOUS       (&Menu_GetCurrentMenu()->Previous)
#define MENU_PREVIOUS   (CurrentMenuItem->Previous)

extern Menu_Item_t NULL_MENU;
extern void NULL_FUNC;

//void MENU_Run(Menu_Keys key_val);
void MENU_Run();
void MENU_Update();

#endif /* MENU_MENU_H_ */
