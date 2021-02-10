/*
 * Copyright (c) 2013 - 2015, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_device_registers.h"
#include "fsl_debug_console.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "flexio_neopixel.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define EXAMPLE_LED_GPIO     BOARD_USER_LED_GPIO
#define EXAMPLE_LED_GPIO_PIN BOARD_USER_LED_GPIO_PIN

#define NEOPIXEL_TYPE          NEO_GRB
#define NEOPIXEL_NUMBER        40
#define NEOPIXEL_SHIFTER       2
#define NEOPIXEL_TIMER         2
#define NEOPIXEL_FLEXIO_BASE   (FLEXIO3)

/* Select USB1 PLL (480 MHz) as flexio clock source */
#define NEOPIXEL_FLEXIO_CLOCK_SELECT (3U)
/* Clock pre divider for flexio clock source */
#define NEOPIXEL_FLEXIO_CLOCK_PRE_DIVIDER (5U)
/* Clock divider for flexio clock source */
#define NEOPIXEL_FLEXIO_CLOCK_DIVIDER (3U)
#define NEOPIXEL_FLEXIO_CLOCK_FREQUENCY                                            \
    (CLOCK_GetFreq(kCLOCK_Usb1PllClk) / (NEOPIXEL_FLEXIO_CLOCK_PRE_DIVIDER + 1U) / \
     (NEOPIXEL_FLEXIO_CLOCK_DIVIDER + 1U))


/*******************************************************************************
 * Variables
 ******************************************************************************/
volatile uint32_t g_systickCounter;

/*******************************************************************************
 * Code
 ******************************************************************************/
void SysTick_Handler(void)
{
    if (g_systickCounter != 0U)
    {
        g_systickCounter--;
    }
}

void SysTick_DelayTicks(uint32_t n)
{
    g_systickCounter = n;
    while (g_systickCounter != 0U)
    {
    }
}

/*!
 * @brief Main function
 */
int main(void)
{
    /* Define the init structure for the output LED pin*/
    gpio_pin_config_t led_config = {kGPIO_DigitalOutput, 0, kGPIO_NoIntmode};
    FLEXIO_NEOPIXEL_Type fiopix;
    uint32_t pixelData[NEOPIXEL_NUMBER];
    uint32_t i;
    uint32_t color;

    /* Init board hardware. */
    BOARD_ConfigMPU();
    BOARD_InitPins();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();

    /* Init output LED GPIO. */
    GPIO_PinInit(EXAMPLE_LED_GPIO, EXAMPLE_LED_GPIO_PIN, &led_config);
    GPIO_PinWrite(EXAMPLE_LED_GPIO, EXAMPLE_LED_GPIO_PIN, 0U);

    /* Clock setting for Flexio */
    CLOCK_SetMux(kCLOCK_Flexio2Mux, NEOPIXEL_FLEXIO_CLOCK_SELECT);
    CLOCK_SetDiv(kCLOCK_Flexio2PreDiv, NEOPIXEL_FLEXIO_CLOCK_PRE_DIVIDER);
    CLOCK_SetDiv(kCLOCK_Flexio2Div, NEOPIXEL_FLEXIO_CLOCK_DIVIDER);

    /* Initialize NeoPixel Interface */
    fiopix.flexioBase = NEOPIXEL_FLEXIO_BASE;
    fiopix.pixelBuf   = pixelData;
    fiopix.pixelNum   = NEOPIXEL_NUMBER;
    fiopix.pixelType  = NEOPIXEL_TYPE;
    fiopix.shifter    = NEOPIXEL_SHIFTER;
    fiopix.timer      = NEOPIXEL_TIMER;
    fiopix_init(&fiopix, NEOPIXEL_FLEXIO_CLOCK_FREQUENCY);
    for (i=0; i<NEOPIXEL_NUMBER; i++){
        pixelData[i]=0x00010100;
    }
    fiopix_show(&fiopix);

    PRINTF("FlexIO NeoPixel Demo\r\n");

    /* Set systick reload value to generate 1ms interrupt */
    if (SysTick_Config(SystemCoreClock / 1000U))
    {
        while (1)
        {
        }
    }

    GPIO_PinWrite(EXAMPLE_LED_GPIO, EXAMPLE_LED_GPIO_PIN, 1U);
    
    i = 0;
    while (1)
    {
        if (i & 0x20) { GPIO_PinWrite(EXAMPLE_LED_GPIO, EXAMPLE_LED_GPIO_PIN, 0U); }
        else { GPIO_PinWrite(EXAMPLE_LED_GPIO, EXAMPLE_LED_GPIO_PIN, 1U); }
        SysTick_DelayTicks(50U);
        color = i/NEOPIXEL_NUMBER;
        color = (1&color) + ((2&color)<<8) + ((4&color)<<16);
        fiopix_setPixel(&fiopix, (i % NEOPIXEL_NUMBER), 0);
        fiopix_setPixel(&fiopix, ((i+1) % NEOPIXEL_NUMBER), (color <<0));
        fiopix_setPixel(&fiopix, ((i+2) % NEOPIXEL_NUMBER), (color <<1));
        fiopix_setPixel(&fiopix, ((i+3) % NEOPIXEL_NUMBER), (color <<2));
        fiopix_setPixel(&fiopix, ((i+4) % NEOPIXEL_NUMBER), (color <<3));
        fiopix_setPixel(&fiopix, ((i+5) % NEOPIXEL_NUMBER), (color <<4));
        fiopix_setPixel(&fiopix, ((i+6) % NEOPIXEL_NUMBER), (color <<2));
        fiopix_setPixel(&fiopix, ((i+7) % NEOPIXEL_NUMBER), (color <<0));
        fiopix_show(&fiopix);
        i +=1;
    }
}
