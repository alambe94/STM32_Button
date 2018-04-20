/*
 * button.h
 *
 *  Created on: 17-Apr-2018
 *      Author: medprime
 */

#ifndef BUTTON_H_
#define BUTTON_H_

#include"stm32f1xx_hal.h"


typedef enum Button_Event_t
{
	Button_Idle=0,
	Button_Pressed,  //still pressed
	Button_Repressed,//at least one clicked and still pressed
	Button_Long_Pressed
}Button_Event_t;


void Button_Scan();
uint8_t Button_Get_Clicked_Count(uint8_t Button);
Button_Event_t Button_Get_Status(uint8_t Button);
void Button_Reset(uint8_t Button);
uint8_t Button_Attach(uint16_t Button_Pin, GPIO_TypeDef* Button_Pin_Port,uint8_t Button_Pressed_Logic,
		void (*Callback)(uint8_t Button_Clicked_Count));





#endif /* BUTTON_H_ */
