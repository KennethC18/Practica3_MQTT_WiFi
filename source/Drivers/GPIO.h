/*
 * LED.h
 *
 *  Created on: 23 Jun 2025
 *      Author: kenneth
 */

#include <stdint.h>

#ifndef GPIO_H_
#define GPIO_H_

/* GPIO pin definitions*/
#define GPIO_OUTPUT_PORT0   	0U

typedef enum gpio_output_pins_t{
	GPIO10 = 10U,
	GPIO9 = 9U,
} gpio_output_pins;

/* GPIO pin output definitions */
#define LOGIC_PIN_LOW  			1U
#define LOGIC_PIN_HIGH 			0U

/**
 * @brief Initialize the GPIO output pins
 */
void GPIO_PIN_Init(void);

void GPIO_PIN_Set(gpio_output_pins pin);

void GPIO_PIN_Clear(gpio_output_pins pin);

void GPIO_PIN_Toggle(gpio_output_pins pin);

#endif /* GPIO_H_ */
