#include "buttons.h"
//#include "hardware.h"


BUTTONS gButtons;

#define BTN_LONG_PRESS_MAX_COUNT  ((uint8_t)(20))
#define BTN_CLICK_MIN_COUNT       (1)
static uint8_t _btnIsDownCount[BUTTONS_COUNT];

//static const uint16_t _btnPins[] = {BUTTON1_PIN, BUTTON2_PIN, BUTTON3_PIN};
static const uint16_t _btnPins[] = { BUTTON1_PIN };
static uint8_t _BtnIsPressed(uint16_t btnPin);

void Buttons_Init(void)
{
//	GPIO_Init(BUTTONS_PORT, BUTTON1_PIN,  GPIO_MODE_IN_PU_NO_IT);
//	GPIO_Init(BUTTONS_PORT, BUTTON2_PIN,  GPIO_MODE_IN_PU_NO_IT);
//	GPIO_Init(BUTTONS_PORT, BUTTON3_PIN,  GPIO_MODE_IN_PU_NO_IT);
	
   for(uint8_t i=0; i<BUTTONS_COUNT; i++)
   {
      gButtons.btns[i].val = 0;
      _btnIsDownCount[i] = 0;
   }
}

void Buttons_Task(void)
{
   for(uint8_t i=0; i<BUTTONS_COUNT; i++)
   {
      gButtons.btns[i].click = 0;
      gButtons.btns[i].longPressEnd = 0;
      
   	  if( _BtnIsPressed(_btnPins[i]) )
   	  {
   		  gButtons.btns[i].longPress = (_btnIsDownCount[i] == BTN_LONG_PRESS_MAX_COUNT);
   		  if( !gButtons.btns[i].longPress )
   			  _btnIsDownCount[i]++;
   	  }
   	  else
      {  
   		  if( gButtons.btns[i].longPress )
   		  {
   			  gButtons.btns[i].longPressEnd = 1;
   			  gButtons.btns[i].longPress = 0;
   		  }
   		  else
   		  {
   			  gButtons.btns[i].click = (_btnIsDownCount[i] >= BTN_CLICK_MIN_COUNT);
   		  }
   		  _btnIsDownCount[i] = 0;
      }
   }
}

uint8_t _BtnIsPressed(uint16_t btnPin)
{
#define max_count (10)
   int8_t average = 0;
	while(1)
	{
		if( HAL_GPIO_ReadPin(BUTTONS_PORT, btnPin) != RESET )
		{
         if( --average == -max_count )
            return 0;
		}
		else
		{
			if( ++average == max_count )
				return 1;
		}
		HAL_Delay(1);
	}	
}


