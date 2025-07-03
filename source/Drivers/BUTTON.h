/*
 * BUTTON.h
 *
 *  Created on: 19 jun. 2025
 *      Author: kenne
 */

#ifndef BUTTON_H_
#define BUTTON_H_

#include "fsl_common.h"
#include "fsl_gpio.h"

#define BUTTON_SW_PORT            0
#define BUTTON_SW_PIN             11
#define BUTTON_IRQ                GPIO_INTA_IRQn
#define BUTTON_CONNECTED_LEVEL    0U

#define BTN_GPIO_19				  19
#define BTN_GPIO_7				  7
#define BTN_BOARD_SW2			  11

/*! @brief Button interrupt callback function type */
typedef void (*button_callback_t)(void);

/*!
 * @brief Initialize the button hardware and interrupt.
 *
 * @param callback Optional callback function to be called when the button is pressed.
 */
void BUTTON_Init(button_callback_t callback);

/*!
 * @brief Check if the button is pressed.
 *
 * @param 	uint8_t Specifies the desired pin to read.
 * @return 	bool True if the button is pressed, false otherwise.
 */
bool BUTTON_IsPressed(uint8_t button_pin);

/*!
 * @brief Get the current state of the button interrupt flag.
 *
 * @return bool True if the button was pressed (interrupt triggered), false otherwise.
 */
bool BUTTON_GetInterruptFlag(void);

/*!
 * @brief Clear the button interrupt flag.
 */
void BUTTON_ClearInterruptFlag(void);

#endif /* BUTTON_H_ */
