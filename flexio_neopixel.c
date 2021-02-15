/* 
 * The MIT License (MIT)
 *
 * Copyright (c) 2021 Greg Steiert
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#include "flexio_neopixel.h"
#include "board.h"
#include "fsl_device_registers.h"
#include "fsl_flexio.h"


//--------------------------------------------------------------------+
// Variables
//--------------------------------------------------------------------+
/* for future non-blocking support 
volatile uint32_t _fiopix_count = 0;
volatile bool     _fiopix_busy = false;
*/

//--------------------------------------------------------------------+
// Interrupt
//--------------------------------------------------------------------+

void fiopix_int_handler(void *fiopixType, void *fiopixHandle) {
  FLEXIO_NEOPIXEL_Type *fiopix = (FLEXIO_NEOPIXEL_Type *)fiopixType;
  FIOPIX_HANDLE_Type *handle = (FIOPIX_HANDLE_Type *)fiopixHandle;
  
  if (handle->dataCnt < fiopix->pixelNum) {
    if (fiopix->flexioBase->SHIFTSTAT & handle->shifterFlag) {
      *handle->dataReg = fiopix->pixelBuf[handle->dataCnt++];
    }
  }

  if (fiopix->flexioBase->TIMSTAT & handle->timerFlag) {
    handle->timerCnt +=1;
    fiopix->flexioBase->TIMSTAT = handle->timerFlag;
    if (handle->timerCnt == fiopix->pixelNum) {
      // Turn off the output and run a few more pixels to 
      // enforce the reset period before clearing busy
      fiopix->flexioBase->TIMCTL[fiopix->timer] &= ~FLEXIO_TIMCTL_PINCFG_MASK;
      *handle->dataReg = 0;
    }
    if ((handle->timerCnt > fiopix->pixelNum) && (handle->timerCnt < handle->doneCnt)) {
      *handle->dataReg = 0;
    }
    if (handle->timerCnt >= handle->doneCnt) {
      FLEXIO_DisableTimerStatusInterrupts(fiopix->flexioBase, handle->timerFlag);
      fiopix->flexioBase->TIMCTL[fiopix->timer] |= FLEXIO_TIMCTL_PINCFG(kFLEXIO_PinConfigOpenDrainOrBidirection);
      handle->dataCnt = 0;
      handle->timerCnt = 0;
      handle->busy = false;
//      FLEXIO_DisableTimerStatusInterrupts(fiopix->flexioBase, handle->timerFlag);
    }
  }
}

//--------------------------------------------------------------------+
// User Functions
//--------------------------------------------------------------------+

void fiopix_setPixelRGB(FLEXIO_NEOPIXEL_Type *fiopix, uint32_t pixel, uint8_t r, uint8_t g, uint8_t b) {
  if (pixel < fiopix->pixelNum) {
    switch (fiopix->pixelType)
    {
    case NEO_GRB:
      fiopix->pixelBuf[pixel] = (r << 16) | (g << 24) | (b << 8);
      break;
    case NEO_GBR:
      fiopix->pixelBuf[pixel] = (r << 8) | (g << 24) | (b << 16);
      break;
    case NEO_RGB:
      fiopix->pixelBuf[pixel] = (r << 24) | (g << 16) | (b << 8);
      break;
    case NEO_RBG:
      fiopix->pixelBuf[pixel] = (r << 24) | (g << 8) | (b << 16);
      break;
    case NEO_BRG:
      fiopix->pixelBuf[pixel] = (r << 16) | (g << 8) | (b << 24);
      break;
    case NEO_BGR:
      fiopix->pixelBuf[pixel] = (r << 8) | (g << 16) | (b << 24);
      break;    
    default:
      fiopix->pixelBuf[pixel] = 0;
      break;
    }
  }
}

void fiopix_setPixelRGBW(FLEXIO_NEOPIXEL_Type *fiopix, uint32_t pixel, uint8_t r, uint8_t g, uint8_t b, uint8_t w) {
  if (pixel < fiopix->pixelNum) {
    switch (fiopix->pixelType)
    {
    case NEO_RGBW:
      fiopix->pixelBuf[pixel] = (r << 24) | (g << 16) | (b << 8) | (w);
      break;
    default:
      fiopix->pixelBuf[pixel] = 0;
      break;
    }
  }
}

void fiopix_setPixel(FLEXIO_NEOPIXEL_Type *fiopix, uint32_t pixel, uint32_t color) {
  uint8_t w, r, g, b;
  if (pixel < fiopix->pixelNum) {
    w = 0xFF & (color >> 24);
    r = 0xFF & (color >> 16);
    g = 0xFF & (color >> 8);
    b = 0xFF & color;
    switch (fiopix->pixelType) {
      case NEO_RGB:
      case NEO_RBG:
      case NEO_GRB:
      case NEO_GBR:
      case NEO_BRG:
      case NEO_BGR:
        fiopix_setPixelRGB(fiopix, pixel, r, g, b);
        break;
      default:
        fiopix_setPixelRGBW(fiopix, pixel, r, g, b, w);
    }
  }
}

void fiopix_showBlocking(FLEXIO_NEOPIXEL_Type *fiopix) {
  while (fiopix->handle.busy) {
    __NOP();
  }
  uint32_t pixelCount = 0;
  while (pixelCount < fiopix->pixelNum) {
    while (!(fiopix->flexioBase->SHIFTSTAT & (uint32_t)(1 << (fiopix->shifter)))) {
      __NOP();
    }
    fiopix->flexioBase->SHIFTBUFBIS[fiopix->shifter] = fiopix->pixelBuf[pixelCount++];
  }
  // Make sure all the bits have shifted out and there is enough time for the reset pulse
  SDK_DelayAtLeastUs(128, SDK_DEVICE_MAXIMUM_CPU_CLOCK_FREQUENCY);
}

void fiopix_show(FLEXIO_NEOPIXEL_Type *fiopix) {
  while (fiopix->handle.busy) {
    __NOP();
  }
  fiopix->handle.busy = true;
  fiopix->handle.dataCnt = 0;
  fiopix->handle.timerCnt = 0;
  fiopix->flexioBase->TIMSTAT = fiopix->handle.timerFlag;
  FLEXIO_EnableTimerStatusInterrupts(fiopix->flexioBase, fiopix->handle.timerFlag);
  *fiopix->handle.dataReg = fiopix->pixelBuf[fiopix->handle.dataCnt++];
  // if (fiopix->handle.dataCnt < fiopix->pixelNum) {
  //   while (!(fiopix->flexioBase->SHIFTSTAT & fiopix->handle.shifterFlag)) {
  //     __NOP();
  //   }
  //   *fiopix->handle.dataReg = fiopix->pixelBuf[fiopix->handle.dataCnt++];
  // } 
  // FLEXIO_EnableTimerStatusInterrupts(fiopix->flexioBase, fiopix->handle.timerFlag);

}


bool fiopix_canShow(FLEXIO_NEOPIXEL_Type *fiopix) {
  return !fiopix->handle.busy;
}

void fiopix_init(FLEXIO_NEOPIXEL_Type *fiopix, uint32_t srcClock_Hz) {

    assert(fiopix != NULL);

    flexio_shifter_config_t shifterConfig;
    flexio_timer_config_t timerConfig;
    // Round up time base 
    uint16_t quarterPeriod = (uint16_t)((srcClock_Hz + 2*NEO_BIT_FREQUENCY)/(4*NEO_BIT_FREQUENCY));
    assert(quarterPeriod <= 0x80); // (2x quarterPeriod)-1 must fit in 8 bits
    uint16_t bitsPerPixel = 32;

  switch (fiopix->pixelType) {
    case NEO_RGB:
    case NEO_RBG:
    case NEO_GRB:
    case NEO_GBR:
    case NEO_BRG:
    case NEO_BGR:
      bitsPerPixel = 24;
      fiopix->handle.doneCnt = fiopix->pixelNum +3;
      break;
    default:
      bitsPerPixel = 32;
      fiopix->handle.doneCnt = fiopix->pixelNum + 2;
  }

    /* Clear the shifterConfig & timerConfig struct. */
    (void)memset(&shifterConfig, 0, sizeof(shifterConfig));
    (void)memset(&timerConfig, 0, sizeof(timerConfig));

    CLOCK_EnableClock(s_flexioClocks[FLEXIO_GetInstance(fiopix->flexioBase)]);

    /* Configure FLEXIO */
    fiopix->flexioBase->CTRL |= FLEXIO_CTRL_FLEXEN_MASK;

    /* Do hardware configuration. */

    /* 1. Configure the shifter for tx. */
    shifterConfig.timerSelect = fiopix->timer;
    shifterConfig.pinConfig   = kFLEXIO_PinConfigBidirectionOutputData;
    shifterConfig.pinSelect   = fiopix->pixelPin;
    shifterConfig.parallelWidth = 0;
    shifterConfig.pinPolarity = kFLEXIO_PinActiveHigh;
    shifterConfig.shifterMode = kFLEXIO_ShifterModeTransmit;
    shifterConfig.inputSource = kFLEXIO_ShifterInputFromPin;
    shifterConfig.timerPolarity = kFLEXIO_ShifterTimerPolarityOnNegitive;
    shifterConfig.shifterStop   = kFLEXIO_ShifterStopBitDisable;
    shifterConfig.shifterStart  = kFLEXIO_ShifterStartBitDisabledLoadDataOnEnable;

    FLEXIO_SetShifterConfig(fiopix->flexioBase, (fiopix->shifter), &shifterConfig);

    /* 2. Configure the first timer for bit clock. */
    timerConfig.triggerSelect   = FLEXIO_TIMER_TRIGGER_SEL_SHIFTnSTAT(fiopix->shifter);
    timerConfig.triggerPolarity = kFLEXIO_TimerTriggerPolarityActiveLow;
    timerConfig.triggerSource   = kFLEXIO_TimerTriggerSourceInternal;
    timerConfig.pinConfig       = kFLEXIO_PinConfigOpenDrainOrBidirection;
    timerConfig.pinSelect       = fiopix->pixelPin;
    timerConfig.pinPolarity     = kFLEXIO_PinActiveHigh;
    timerConfig.timerMode       = kFLEXIO_TimerModeDual8BitBaudBit;
    timerConfig.timerOutput     = kFLEXIO_TimerOutputZeroNotAffectedByReset;
    timerConfig.timerDecrement  = kFLEXIO_TimerDecSrcOnFlexIOClockShiftTimerOutput;
    timerConfig.timerReset      = kFLEXIO_TimerResetNever;
    timerConfig.timerDisable    = kFLEXIO_TimerDisableOnTimerCompareTriggerLow;
    timerConfig.timerEnable     = kFLEXIO_TimerEnableOnTriggerHigh;
    timerConfig.timerStop       = kFLEXIO_TimerStopBitDisabled;
    timerConfig.timerStart      = kFLEXIO_TimerStartBitDisabled;

    timerConfig.timerCompare = (((uint16_t)(bitsPerPixel * 2U) - 1U) << 8U)
                             | (0xFF & ((quarterPeriod <<1) -1U));

    FLEXIO_SetTimerConfig(fiopix->flexioBase, fiopix->timer, &timerConfig);

    /* 3. Configure the second timer for Short Pulse. */
    timerConfig.triggerSelect   = FLEXIO_TIMER_TRIGGER_SEL_TIMn(fiopix->timer);
    timerConfig.triggerPolarity = kFLEXIO_TimerTriggerPolarityActiveHigh;
    timerConfig.triggerSource   = kFLEXIO_TimerTriggerSourceInternal;
    timerConfig.pinConfig       = kFLEXIO_PinConfigBidirectionOutputData;
    timerConfig.pinSelect       = fiopix->pixelPin;
    timerConfig.pinPolarity     = kFLEXIO_PinActiveHigh;
    timerConfig.timerMode       = kFLEXIO_TimerModeDual8BitPWM;
    timerConfig.timerOutput     = kFLEXIO_TimerOutputOneNotAffectedByReset;
    timerConfig.timerDecrement  = kFLEXIO_TimerDecSrcOnFlexIOClockShiftTimerOutput;
    timerConfig.timerReset      = kFLEXIO_TimerResetNever;
    timerConfig.timerDisable    = kFLEXIO_TimerDisableOnPreTimerDisable;
    timerConfig.timerEnable     = kFLEXIO_TimerEnableOnPrevTimerEnable;
    timerConfig.timerStop       = kFLEXIO_TimerStopBitDisabled;
    timerConfig.timerStart      = kFLEXIO_TimerStartBitDisabled;

    timerConfig.timerCompare = ((quarterPeriod-1U)<<8) | (quarterPeriod -1U);

    FLEXIO_SetTimerConfig(fiopix->flexioBase, fiopix->timer+1, &timerConfig);

    fiopix->handle.dataReg = &fiopix->flexioBase->SHIFTBUFBIS[fiopix->shifter];
    fiopix->handle.shifterFlag = 1U << (fiopix->shifter);
    fiopix->handle.timerFlag = 1U << (fiopix->timer);
    fiopix->handle.dataCnt = 0;
    fiopix->handle.timerCnt = 0;
    fiopix->handle.busy = false;

    IRQn_Type flexio_irqs[] = FLEXIO_IRQS;

    /* Clear pending NVIC IRQ before enable NVIC IRQ. */
    NVIC_ClearPendingIRQ(flexio_irqs[FLEXIO_GetInstance(fiopix->flexioBase)]);
    /* Enable interrupt in NVIC. */
    (void)EnableIRQ(flexio_irqs[FLEXIO_GetInstance(fiopix->flexioBase)]);

    /* Save the context in global variables to support the double weak mechanism. */
    FLEXIO_RegisterHandleIRQ(fiopix, &fiopix->handle, fiopix_int_handler);
}

