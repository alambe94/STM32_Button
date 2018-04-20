/*
 * button.c
 *
 *  Created on: 17-Apr-2018
 *      Author: medprime
 */

#include "main.h"
#include "button.h"

#define MAX_BUTTONS 3

#define BUTTON_SCAN_TICK 20         // Button_Scan() called every BUTTON_SCAN_TICK

#define BUTTON_DEBOUNCE_DELAY      (50/BUTTON_SCAN_TICK)   //ticks
#define BUTTON_CLICKED_DELAY       (400/BUTTON_SCAN_TICK)  // confirm clicked in foreground after released for CLICKED_DELAY
#define BUTTON_REPRESSED_DELAY     (300/BUTTON_SCAN_TICK)  //ticks
#define BUTTON_LONG_PRESSED_DELAY  (1000/BUTTON_SCAN_TICK) //ticks

typedef struct Button_Struct_t
{
	uint16_t _Button_Pin;  //gpio pin to which button is attached
	GPIO_TypeDef* _Button_Pin_Port;  //gpio port to which button is attached
	uint8_t _Button_Pressed_Logic; //gpio logic state after gpio is pressed HIGH /  LOW
	Button_Event_t _Button_Event;
	uint32_t _Button_Pressed_Ticks;
	uint32_t _Button_Released_Ticks;
	uint8_t _Button_Clicked_Count;
	void (*_Callback)(uint8_t Button_Clicked_Count); //callback function

} Button_Struct_t;

static Button_Struct_t Button_Array[MAX_BUTTONS];

static uint8_t Attached_Buttons = 0;

/*********************************************************
 *
 * called every tick (BUTTON_SCAN_TICK ?)
 *
 * should be called at least every 20ms?
 *
 *
 */
void Button_Scan()
{
	static volatile uint32_t Button_Scan_Time_Stamp = 0;

	if (HAL_GetTick() - Button_Scan_Time_Stamp > (BUTTON_SCAN_TICK - 1)) // excute this loop on every BUTTON_SCAN_TICK
	{
		for (uint8_t Index = 0; Index < Attached_Buttons; Index++)

		{
			if (HAL_GPIO_ReadPin(Button_Array[Index]._Button_Pin_Port,
					Button_Array[Index]._Button_Pin)
					== Button_Array[Index]._Button_Pressed_Logic) //pressed detected

			{
				Button_Array[Index]._Button_Pressed_Ticks++;	//

				if (Button_Array[Index]._Button_Pressed_Ticks
						< BUTTON_LONG_PRESSED_DELAY)
				{

					if (Button_Array[Index]._Button_Released_Ticks //repressed detected
					< BUTTON_REPRESSED_DELAY)
					{
						Button_Array[Index]._Button_Event = Button_Repressed;
					}
					else
					{
						Button_Array[Index]._Button_Event = Button_Pressed;
					}

					Button_Array[Index]._Button_Released_Ticks = 0;

				}
				else
				{   //long pressed detected
					Button_Array[Index]._Button_Event = Button_Long_Pressed;
					Button_Array[Index]._Button_Clicked_Count = 0xFF; //0xFF for long press
					if (Button_Array[Index]._Callback != NULL)
					{
						Button_Array[Index]._Callback(
								Button_Array[Index]._Button_Clicked_Count);
						Button_Array[Index]._Button_Clicked_Count = 0;
						Button_Array[Index]._Button_Event = Button_Idle;
					}

				}
			}
			else   //released detected
			{
				Button_Array[Index]._Button_Released_Ticks++;

				if (Button_Array[Index]._Button_Pressed_Ticks
						> BUTTON_DEBOUNCE_DELAY) //if button was pressed for BUTTON_DEBOUNCE_DELAY
				{
					Button_Array[Index]._Button_Pressed_Ticks = 0;
					if (Button_Array[Index]._Button_Event == Button_Repressed)
					{
						Button_Array[Index]._Button_Clicked_Count++;
					}
					else if (Button_Array[Index]._Button_Event
							== Button_Pressed)
					{
						Button_Array[Index]._Button_Clicked_Count = 1;
					}

				}

				if (Button_Array[Index]._Button_Released_Ticks
						> BUTTON_CLICKED_DELAY)
				{
					if (Button_Array[Index]._Callback != NULL
							&& Button_Array[Index]._Button_Event != Button_Idle)
					{
						Button_Array[Index]._Callback(
								Button_Array[Index]._Button_Clicked_Count);
						Button_Array[Index]._Button_Clicked_Count = 0;
						Button_Array[Index]._Button_Event = Button_Idle;

					}
				}

			}

		}

		Button_Scan_Time_Stamp = HAL_GetTick();
	}
}

void Button_Init()
{
	//init GPIOs
}

uint8_t Button_Attach(uint16_t Button_Pin, GPIO_TypeDef* Button_Pin_Port,
		uint8_t Button_Pressed_Logic,
		void (*Callback)(uint8_t Button_Clicked_Count))
{
	//init GPIOs as input

	GPIO_InitTypeDef GPIO_InitStruct;

	/* GPIO Ports Clock Enable */
	__HAL_RCC_GPIOC_CLK_ENABLE()
	;
	__HAL_RCC_GPIOD_CLK_ENABLE()
	;
	__HAL_RCC_GPIOA_CLK_ENABLE()
	;
	__HAL_RCC_GPIOB_CLK_ENABLE()
	;

	GPIO_InitStruct.Pin = Button_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	if (Button_Pressed_Logic == 0)
	{
		GPIO_InitStruct.Pull = GPIO_PULLUP;
	}
	else
	{
		GPIO_InitStruct.Pull = GPIO_PULLDOWN;
	}
	HAL_GPIO_Init(Button_Pin_Port, &GPIO_InitStruct);


	Button_Array[Attached_Buttons]._Button_Pin = Button_Pin;
	Button_Array[Attached_Buttons]._Button_Pin_Port = Button_Pin_Port;
	Button_Array[Attached_Buttons]._Button_Pressed_Logic = Button_Pressed_Logic;
	Button_Array[Attached_Buttons]._Callback = Callback;
	Button_Array[Attached_Buttons]._Button_Clicked_Count = 0;
	Button_Array[Attached_Buttons]._Button_Event = Button_Idle;
	Button_Array[Attached_Buttons]._Button_Pressed_Ticks = 0;
	Button_Array[Attached_Buttons]._Button_Released_Ticks = 0;
	Attached_Buttons++;
	if (Attached_Buttons > MAX_BUTTONS)
	{
		_Error_Handler(__FILE__, __LINE__);
	}
	return (Attached_Buttons - 1);
}

Button_Event_t Button_Get_Status(uint8_t Button)
{
	return Button_Array[Button]._Button_Event;
}

uint8_t Button_Get_Clicked_Count(uint8_t Button)
{
	return Button_Array[Button]._Button_Clicked_Count;
}

void Button_Reset(uint8_t Button)
{
	Button_Array[Button]._Button_Clicked_Count = 0;
	Button_Array[Button]._Button_Event = Button_Idle;
}

