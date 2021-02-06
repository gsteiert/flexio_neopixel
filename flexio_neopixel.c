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
/*
void fiopix_int_handler(void){
  // get and check even flags
  uint32_t eventFlag = NEO_SCT->EVFLAG;
  // if data remaining
  if (_fiopix_count < _fiopix_size) {
    fiopix->flexioBase->SHIFTBUFBIS[fiopix->shifter+1] = _fiopix_data[_fiopix_count++];
  } else {
    _fiopix_busy = false;
  }
  // clear interrupt
}
*/
/*
void SCT0_DriverIRQHandler(void){
  fiopix_int_handler();
  SDK_ISR_EXIT_BARRIER;
}
*/
//--------------------------------------------------------------------+
// User Functions
//--------------------------------------------------------------------+
void fiopix_setPixel(FLEXIO_NEOPIXEL_Type *fiopix, uint32_t pixel, uint32_t color) {
  if (pixel < fiopix->pixelNum) {
//    while (_fiopix_busy) { __NOP(); } // for future non-blocking support
    fiopix->pixelBuf[pixel] = color;
  }
}

void fiopix_show(FLEXIO_NEOPIXEL_Type *fiopix) {
//  while (_fiopix_busy) {__NOP();} // for future non-blocking support
//  _fiopix_busy = true; // for future non-blocking support
  uint32_t pixelCount = 0;
  while (pixelCount < fiopix->pixelNum) {
    while (fiopix->flexioBase->SHIFTSTAT & (uint32_t)(1 << (fiopix->shifter+1))) {
      __NOP();
    }
    fiopix->flexioBase->SHIFTBUFBIS[fiopix->shifter+1] = fiopix->pixelBuf[pixelCount++];
  }
  // Make sure all the bits have shifted out and there is enough time for the reset pulse
  SDK_DelayAtLeastUs(128, SDK_DEVICE_MAXIMUM_CPU_CLOCK_FREQUENCY);
}

bool fiopix_canShow(FLEXIO_NEOPIXEL_Type *fiopix) {
//  return !_fiopix_busy; // for future non-blocking support
  return true;
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
      break;
    default:
      bitsPerPixel = 32;
  }

    /* Clear the shifterConfig & timerConfig struct. */
    (void)memset(&shifterConfig, 0, sizeof(shifterConfig));
    (void)memset(&timerConfig, 0, sizeof(timerConfig));

    /* Configure FLEXIO */
    fiopix->flexioBase->CTRL |= FLEXIO_CTRL_FLEXEN_MASK;

    /* Do hardware configuration. */

    /* 1. Configure the first shifter for logic. */
    shifterConfig.timerSelect  = fiopix->timer;  // shift on rise of short pulse
    shifterConfig.pinConfig    = kFLEXIO_PinConfigOutput;
    shifterConfig.pinSelect    = fiopix->shifter;  // does not apply when using next shifter
    shifterConfig.parallelWidth = 31;  // shift 32bits, one clock delay
    shifterConfig.pinPolarity  = kFLEXIO_PinActiveHigh; // does not apply when using next shifter
    shifterConfig.shifterMode  = kFLEXIO_ShifterModeLogic;  // use logic mode
    // get data from next shifter to free up FlexIO data lines
    shifterConfig.inputSource  = kFLEXIO_ShifterInputFromNextShifterOutput;
    // only use lower two FlexIO data lines for decode  
    shifterConfig.shifterStop  = (flexio_shifter_stop_bit_t)0x3;
    // enable lower two FlexIO data lines
    shifterConfig.shifterStart = (flexio_shifter_start_bit_t)0x0;
    shifterConfig.timerPolarity = kFLEXIO_ShifterTimerPolarityOnPositive;

    FLEXIO_SetShifterConfig(fiopix->flexioBase, fiopix->shifter, &shifterConfig);
    // write look-up table values to SHIFTBUF
    fiopix->flexioBase->SHIFTBUF[fiopix->shifter] = 0xCCCC8888;

    /* 2. Configure the second shifter for tx. */
    shifterConfig.timerSelect = fiopix->timer;
    shifterConfig.pinConfig   = kFLEXIO_PinConfigOutput;
    shifterConfig.pinSelect   = fiopix->shifter+5;
    shifterConfig.parallelWidth = 0;
    shifterConfig.pinPolarity = kFLEXIO_PinActiveHigh;
    shifterConfig.shifterMode = kFLEXIO_ShifterModeTransmit;
    shifterConfig.inputSource = kFLEXIO_ShifterInputFromPin;
    shifterConfig.timerPolarity = kFLEXIO_ShifterTimerPolarityOnNegitive;
    shifterConfig.shifterStop   = kFLEXIO_ShifterStopBitDisable;
    shifterConfig.shifterStart  = kFLEXIO_ShifterStartBitDisabledLoadDataOnEnable;

    FLEXIO_SetShifterConfig(fiopix->flexioBase, (fiopix->shifter+1), &shifterConfig);


    /* 3. Configure the first timer for bit clock. */
    timerConfig.triggerSelect   = FLEXIO_TIMER_TRIGGER_SEL_SHIFTnSTAT(fiopix->shifter+1);
    timerConfig.triggerPolarity = kFLEXIO_TimerTriggerPolarityActiveLow;
    timerConfig.triggerSource   = kFLEXIO_TimerTriggerSourceInternal;
    timerConfig.pinConfig       = kFLEXIO_PinConfigOutput;
    timerConfig.pinSelect       = fiopix->shifter+1;
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

    /* 4. Configure the second timer for Short Pulse. */
    timerConfig.triggerSelect   = FLEXIO_TIMER_TRIGGER_SEL_TIMn(fiopix->timer);
    timerConfig.triggerPolarity = kFLEXIO_TimerTriggerPolarityActiveHigh;
    timerConfig.triggerSource   = kFLEXIO_TimerTriggerSourceInternal;
    timerConfig.pinConfig       = kFLEXIO_PinConfigOutput;
    timerConfig.pinSelect       = fiopix->shifter;
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

}

