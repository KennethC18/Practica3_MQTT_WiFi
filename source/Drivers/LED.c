/*
 * LED.c
 *
 *  Created on: 15 Jun 2025
 *      Author: kenneth
 */

#include "LED.h"

#include "fsl_gpio.h"
#include "fsl_io_mux.h"
#include "board.h"

void LED_Init(void) {
    gpio_pin_config_t led_config = {
        kGPIO_DigitalOutput,
        LOGIC_LED_OFF, /* Initial state: off */
    };

    /* Initialize GPIO port */
    GPIO_PortInit(GPIO, LED_GPIO_PORT);

    IO_MUX_SetPinMux(IO_MUX_GPIO0);
	IO_MUX_SetPinMux(IO_MUX_GPIO1);
	IO_MUX_SetPinMux(IO_MUX_GPIO12);

    /* Initialize LED pins */
    GPIO_PinInit(GPIO, LED_GPIO_PORT, LED_RED_GPIO_PIN, &led_config);
    GPIO_PinInit(GPIO, LED_GPIO_PORT, LED_GREEN_GPIO_PIN, &led_config);
    GPIO_PinInit(GPIO, LED_GPIO_PORT, LED_BLUE_GPIO_PIN, &led_config);
}

void LED_Set(uint8_t RED, uint8_t GREEN, uint8_t BLUE){
	GPIO->B[LED_GPIO_PORT][LED_RED_GPIO_PIN] = RED;
	GPIO->B[LED_GPIO_PORT][LED_GREEN_GPIO_PIN] = GREEN;
	GPIO->B[LED_GPIO_PORT][LED_BLUE_GPIO_PIN] = BLUE;
}

void LED_Clear(uint8_t RED, uint8_t GREEN, uint8_t BLUE){
    GPIO->SET[LED_GPIO_PORT] |= (!RED << LED_RED_GPIO_PIN) +
    						   (!GREEN << LED_GREEN_GPIO_PIN) +
							   (!BLUE << LED_BLUE_GPIO_PIN);
}
void LED_Toggle(uint8_t RED, uint8_t GREEN, uint8_t BLUE){
	GPIO->NOT[LED_GPIO_PORT] = (!RED << LED_RED_GPIO_PIN) +
							   (!GREEN << LED_GREEN_GPIO_PIN) +
							   (!BLUE << LED_BLUE_GPIO_PIN);
}
