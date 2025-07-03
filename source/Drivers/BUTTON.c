/*
 * BUTTON.c
 *
 *  Created on: 19 jun. 2025
 *      Author: kenne
 */

#include "BUTTON.h"
#include "board.h"
#include "fsl_io_mux.h"

static volatile bool s_buttonInterruptFlag = false;
static button_callback_t s_buttonCallback = NULL;

void BUTTON_Init(button_callback_t callback)
{
    gpio_pin_config_t sw_config = {kGPIO_DigitalInput, 0};
    gpio_interrupt_config_t config = {kGPIO_PinIntEnableEdge, kGPIO_PinIntEnableLowOrFall};

    s_buttonCallback = callback;

    IO_MUX_SetPinMux(IO_MUX_GPIO11);
    IO_MUX_SetPinMux(IO_MUX_GPIO19);
    IO_MUX_SetPinMux(IO_MUX_GPIO7);

    GPIO_PortInit(GPIO, BUTTON_SW_PORT);
    GPIO_PinInit(GPIO, BUTTON_SW_PORT, BUTTON_SW_PIN, &sw_config);
    GPIO_PinInit(GPIO, BUTTON_SW_PORT, BTN_GPIO_19, &sw_config);
    GPIO_PinInit(GPIO, BUTTON_SW_PORT, BTN_GPIO_7, &sw_config);

    EnableIRQ(BUTTON_IRQ);
    GPIO_SetPinInterruptConfig(GPIO, BUTTON_SW_PORT, BUTTON_SW_PIN, &config);
    GPIO_PinEnableInterrupt(GPIO, BUTTON_SW_PORT, BUTTON_SW_PIN, 0);
}

bool BUTTON_IsPressed(uint8_t button_pin)
{
    return (BUTTON_CONNECTED_LEVEL == GPIO_PinRead(GPIO, BUTTON_SW_PORT, button_pin));
}

bool BUTTON_GetInterruptFlag(void)
{
    return s_buttonInterruptFlag;
}

void BUTTON_ClearInterruptFlag(void)
{
    s_buttonInterruptFlag = false;
    GPIO_PinClearInterruptFlag(GPIO, BUTTON_SW_PORT, BUTTON_SW_PIN, 0);
}

void GPIO_INTA_IRQHandler(void)
{
	GPIO_PinClearInterruptFlag(GPIO, BUTTON_SW_PORT, BUTTON_SW_PIN, 0);
	s_buttonInterruptFlag = true;

	if (s_buttonCallback != NULL)
	{
		s_buttonCallback();
	}

	SDK_ISR_EXIT_BARRIER;
}
