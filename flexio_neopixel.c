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
volatile uint32_t _fiopix_count = 0;
volatile bool     _fiopix_busy = false;


//--------------------------------------------------------------------+
// Interrupt
//--------------------------------------------------------------------+
void fiopix_int_handler(void){
  // get and check even flags
  uint32_t eventFlag = NEO_SCT->EVFLAG;
  // if data remaining
  if (_fiopix_count < _fiopix_size) {
    base->flexioBase->SHIFTBUFBIS[base->firstShifterIndex+1] = _fiopix_data[_fiopix_count++];
  } else {
    _fiopix_busy = false;
  }
  // clear interrupt

}

void SCT0_DriverIRQHandler(void){
  fiopix_int_handler();
  SDK_ISR_EXIT_BARRIER;
}

//--------------------------------------------------------------------+
// User Functions
//--------------------------------------------------------------------+
void fiopix_setPixel(uint32_t pixel, uint32_t color){
  if (pixel < _fiopix_size) {
    if (_fiopix_syncUpdate) {
      while (_fiopix_busy) { __NOP(); }
    }
    _fiopix_data[pixel] = color;
  }
}

void fiopix_show(void){
  while (_fiopix_busy) {__NOP();}
  _fiopix_busy = true;
  _fiopix_count = 0;
  base->flexioBase->SHIFTBUFBIS[base->firstShifterIndex+1] = _fiopix_data[_fiopix_count++];
}

bool fiopix_canShow(void) {
  return !_fiopix_busy;
}

void fiopix_init(FLEXIO_NEOPIXEL_Type *base, uint32_t neoPixelType, uint32_t *data, uint32_t size) {

  EnableIRQ(SCT0_IRQn);

}

