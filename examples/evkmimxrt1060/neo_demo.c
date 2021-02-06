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
    FLEXIO_NEOPIXEL_Type fiopix;
    uint32_t pixelData[NEOPIXEL_NUMBER];
    uint32_t i;
    uint32_t color;

    /* Init board hardware. */
    BOARD_InitPins();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();

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
        pixelData[i]=0x080001;
    }
    fiopix_show(&fiopix);

    PRINTF("SCT NeoPixel Demo\r\n");

    /* Set systick reload value to generate 1ms interrupt */
    if (SysTick_Config(SystemCoreClock / 1000U))
    {
        while (1)
        {
        }
    }

    i = 0;
    while (1)
    {
        SysTick_DelayTicks(50U);
        color = i/NEOPIXEL_NUMBER;
        color = (1&color) + ((2&color)<<16) + ((4&color)<<8);
        pixelData[i % NEOPIXEL_NUMBER] = 0;
        pixelData[(i+1) % NEOPIXEL_NUMBER] = color;
        pixelData[(i+2) % NEOPIXEL_NUMBER] = color <<1;
        pixelData[(i+3) % NEOPIXEL_NUMBER] = color <<2;
        pixelData[(i+4) % NEOPIXEL_NUMBER] = color <<3;
        pixelData[(i+5) % NEOPIXEL_NUMBER] = color <<4;
        pixelData[(i+6) % NEOPIXEL_NUMBER] = color <<2;
        pixelData[(i+7) % NEOPIXEL_NUMBER] = color ;
        fiopix_show(&fiopix);
        i +=1;
    }
}
