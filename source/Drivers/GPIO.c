/*
 * GPIO.c
 *
 *  Created on: 23 Jun 2025
 *      Author: kenneth
 */

#include "GPIO.h"
#include "fsl_gpio.h"
#include "fsl_io_mux.h"
#include "board.h"

void GPIO_PIN_Init(void) {
    gpio_pin_config_t output_config = {
        kGPIO_DigitalOutput,
        LOGIC_PIN_HIGH, /* Initial state: off */
    };

    GPIO_PortInit(GPIO, GPIO_OUTPUT_PORT0);

	IO_MUX_SetPinMux(IO_MUX_GPIO10);
	IO_MUX_SetPinMux(IO_MUX_GPIO9);

	GPIO_PinInit(GPIO, GPIO_OUTPUT_PORT0, GPIO10, &output_config);
	GPIO_PinInit(GPIO, GPIO_OUTPUT_PORT0, GPIO9, &output_config);
}

void GPIO_PIN_Set(gpio_output_pins pin){
	GPIO->CLR[GPIO_OUTPUT_PORT0] |= 1 << pin;
}

void GPIO_PIN_Clear(gpio_output_pins pin){
	GPIO->SET[GPIO_OUTPUT_PORT0] |= 1 << pin;
}
void GPIO_PIN_Toggle(gpio_output_pins pin){
	GPIO->NOT[GPIO_OUTPUT_PORT0] |= 1 << pin;
}
