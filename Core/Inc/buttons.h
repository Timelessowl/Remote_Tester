#ifndef __BUTTON_H_
#define __BUTTON_H_

//#include "stm8s.h"
#include "stm32f1xx_hal.h"
#include "stm32f1xx.h"
#define BUTTONS_COUNT   (3)
#define BUTTONS_PORT        GPIOB
#define BUTTON1_PIN         GPIO_PIN_12
#define BUTTON2_PIN         GPIO_PIN_13
#define BUTTON3_PIN         GPIO_PIN_14

typedef union
{
	uint8_t val;
   struct{
	  uint8_t click: 1;
      uint8_t longPress: 1;
      uint8_t longPressEnd: 1; 
   };
} BUTTON_STATE;

typedef union
{
   BUTTON_STATE btns[BUTTONS_COUNT];
   struct{
       BUTTON_STATE btnOk;
       BUTTON_STATE btnTestedButton;
       BUTTON_STATE btnFinger;
   };
} BUTTONS;

extern BUTTONS gButtons;

extern void Buttons_Init(void);
extern void Buttons_Task(void);

//bool Button_IsPressed(uint16_t button_pin);
//unsigned int ButtonOk_IsLongPress();
//unsigned int Check_Change_Page(void);

#endif //__BUTTON_H_
