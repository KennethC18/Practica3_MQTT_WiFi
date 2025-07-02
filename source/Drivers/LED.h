/*
 * LED.h
 *
 *  Created on: 15 Jun 2025
 *      Author: kenneth
 */

#include <stdint.h>

#ifndef LED_H_
#define LED_H_

/* LED pin definitions*/
#define LED_GPIO_PORT   	0U

#define LED_RED_GPIO_PIN    1U
#define LED_GREEN_GPIO_PIN  12U
#define LED_BLUE_GPIO_PIN   0U

/* LED colour macros*/
#define LED_RED_COLOUR 		LOGIC_LED_ON, LOGIC_LED_OFF, LOGIC_LED_OFF
#define LED_GREEN_COLOUR 	LOGIC_LED_OFF, LOGIC_LED_ON, LOGIC_LED_OFF
#define LED_BLUE_COLOUR 	LOGIC_LED_OFF, LOGIC_LED_OFF, LOGIC_LED_ON

#define LED_YELLOW_COLOUR 	LOGIC_LED_ON, LOGIC_LED_ON, LOGIC_LED_OFF
#define LED_CYAN_COLOUR 	LOGIC_LED_OFF, LOGIC_LED_ON, LOGIC_LED_ON
#define LED_MAGENTA_COLOUR 	LOGIC_LED_ON, LOGIC_LED_OFF, LOGIC_LED_ON

#define LED_WHITE_COLOUR 	LOGIC_LED_ON, LOGIC_LED_ON, LOGIC_LED_ON

/**
 * @brief Initialize the LEDs
 * @return 0 on success, 1 on failure
 */
void LED_Init(void);

void LED_Set(uint8_t RED, uint8_t GREEN, uint8_t BLUE);

void LED_Clear(uint8_t RED, uint8_t GREEN, uint8_t BLUE);

void LED_Toggle(uint8_t RED, uint8_t GREEN, uint8_t BLUE);

#endif /* LED_H_ */
