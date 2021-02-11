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

#ifndef _FLEXIO_NEOPIXEL_H_
#define _FLEXIO_NEOPIXEL_H_

#ifdef __cplusplus
 extern "C" {
#endif

#include "fsl_flexio.h"
#include <stdint.h>
#include <stdbool.h>


// RGB NeoPixel permutations; white and red offsets are always same
// Offset:         W        R        G        B
#define NEO_RGB  ((0<<6) | (0<<4) | (1<<2) | (2)) ///< Transmit as R,G,B
#define NEO_RBG  ((0<<6) | (0<<4) | (2<<2) | (1)) ///< Transmit as R,B,G
#define NEO_GRB  ((1<<6) | (1<<4) | (0<<2) | (2)) ///< Transmit as G,R,B
#define NEO_GBR  ((2<<6) | (2<<4) | (0<<2) | (1)) ///< Transmit as G,B,R
#define NEO_BRG  ((1<<6) | (1<<4) | (2<<2) | (0)) ///< Transmit as B,R,G
#define NEO_BGR  ((2<<6) | (2<<4) | (1<<2) | (0)) ///< Transmit as B,G,R

// RGBW NeoPixel permutations; all 4 offsets are distinct
// Offset:         W          R          G          B
#define NEO_WRGB ((0<<6) | (1<<4) | (2<<2) | (3)) ///< Transmit as W,R,G,B
#define NEO_WRBG ((0<<6) | (1<<4) | (3<<2) | (2)) ///< Transmit as W,R,B,G
#define NEO_WGRB ((0<<6) | (2<<4) | (1<<2) | (3)) ///< Transmit as W,G,R,B
#define NEO_WGBR ((0<<6) | (3<<4) | (1<<2) | (2)) ///< Transmit as W,G,B,R
#define NEO_WBRG ((0<<6) | (2<<4) | (3<<2) | (1)) ///< Transmit as W,B,R,G
#define NEO_WBGR ((0<<6) | (3<<4) | (2<<2) | (1)) ///< Transmit as W,B,G,R

#define NEO_RWGB ((1<<6) | (0<<4) | (2<<2) | (3)) ///< Transmit as R,W,G,B
#define NEO_RWBG ((1<<6) | (0<<4) | (3<<2) | (2)) ///< Transmit as R,W,B,G
#define NEO_RGWB ((2<<6) | (0<<4) | (1<<2) | (3)) ///< Transmit as R,G,W,B
#define NEO_RGBW ((3<<6) | (0<<4) | (1<<2) | (2)) ///< Transmit as R,G,B,W
#define NEO_RBWG ((2<<6) | (0<<4) | (3<<2) | (1)) ///< Transmit as R,B,W,G
#define NEO_RBGW ((3<<6) | (0<<4) | (2<<2) | (1)) ///< Transmit as R,B,G,W

#define NEO_GWRB ((1<<6) | (2<<4) | (0<<2) | (3)) ///< Transmit as G,W,R,B
#define NEO_GWBR ((1<<6) | (3<<4) | (0<<2) | (2)) ///< Transmit as G,W,B,R
#define NEO_GRWB ((2<<6) | (1<<4) | (0<<2) | (3)) ///< Transmit as G,R,W,B
#define NEO_GRBW ((3<<6) | (1<<4) | (0<<2) | (2)) ///< Transmit as G,R,B,W
#define NEO_GBWR ((2<<6) | (3<<4) | (0<<2) | (1)) ///< Transmit as G,B,W,R
#define NEO_GBRW ((3<<6) | (2<<4) | (0<<2) | (1)) ///< Transmit as G,B,R,W

#define NEO_BWRG ((1<<6) | (2<<4) | (3<<2) | (0)) ///< Transmit as B,W,R,G
#define NEO_BWGR ((1<<6) | (3<<4) | (2<<2) | (0)) ///< Transmit as B,W,G,R
#define NEO_BRWG ((2<<6) | (1<<4) | (3<<2) | (0)) ///< Transmit as B,R,W,G
#define NEO_BRGW ((3<<6) | (1<<4) | (2<<2) | (0)) ///< Transmit as B,R,G,W
#define NEO_BGWR ((2<<6) | (3<<4) | (1<<2) | (0)) ///< Transmit as B,G,W,R
#define NEO_BGRW ((3<<6) | (2<<4) | (1<<2) | (0)) ///< Transmit as B,G,R,W

#define NEO_BIT_FREQUENCY     800000

#define FIOPIX_LOGIC_PATTERN  0xCCCC8888


/*! @brief Handle for FlexIO NeoPixel Interrupts typedef */
typedef struct _flexio_neopixel_handle_type
{
  volatile uint32_t *dataReg;  /*!< Pointer to shifter data register */
  uint32_t shifterFlag;              /*!< INterrupt flags */
  uint32_t timerFlag;              /*!< INterrupt flags */
  uint32_t doneCnt;            /*!< Count at end of reset period */
  volatile uint32_t dataCnt;     /*!< Bytes transfered */
  volatile uint32_t timerCnt;     /*!< Bytes transfered */
  volatile bool     busy;      /*!< Busy indicator */
} FIOPIX_HANDLE_Type;

/*! @brief Define FlexIO NeoPixel structure typedef */
typedef struct _flexio_neopixel_type
{
    FLEXIO_Type *flexioBase; /*!< FlexIO base pointer */
    uint32_t *pixelBuf;      /*!< Pointer to pixel data buffer */
    uint32_t pixelNum;       /*!< Number of pixels */
    uint8_t pixelType;       /*!< Type of NeoPixel (GRB/WGRB/... etc) */
    // The FlexIO data for logic mode are fixed per shifter
    // For shifter index i, the following FlexIO Data lines are used:
    //   D(i) for first timer output (internal)
    //   D(i+1) for second timer output (internal)
    //   D(i+4) for NeoPixel data output (external)
    //   D(i+5) for internal shift register (no output)
    uint8_t shifter; /*!< Index of first FlexIO shifter resource */
    uint8_t timer;   /*!< Index of first FlexIO timer resource */
    FIOPIX_HANDLE_Type handle; /*!< Interrupt handle */
} FLEXIO_NEOPIXEL_Type;

/*!
  @brief  Initializes the SCTimer for driving NeoPixels
          This initializes all channels to inactive.  
          Channels must be added with sctpix_addCh
  @param  fiopix  Pointer to the FLEXIO_NEOPIXEL_Type structure
          Only 3byte per pixels are currently implemented.
*/
void fiopix_init(FLEXIO_NEOPIXEL_Type *fiopix, uint32_t srcClock_Hz);

/*!
  @brief  This initiates the transfer on the instance
*/
void fiopix_showBlocking(FLEXIO_NEOPIXEL_Type *fiopix);

/*!
  @brief  This initiates interrupt based transfer on the instance
*/
void fiopix_show(FLEXIO_NEOPIXEL_Type *fiopix);

/*!
  @brief  This indicates that no transaction is active so data can be updated 
          synchronously.
*/
bool fiopix_canShow(FLEXIO_NEOPIXEL_Type *fiopix);

/*!
  @brief  This sets the value of the respective pixel data
  @param  fiopix  Pointer to the FLEXIO_NEOPIXEL_Type structure
  @param  pixel  The pixel in the specified channel
  @param  color  32bit WRGB value to be converted and stored
*/
void fiopix_setPixel(FLEXIO_NEOPIXEL_Type *fiopix, uint32_t pixel, uint32_t color);

/*!
  @brief  This sets the value of the respective pixel data
  @param  fiopix  Pointer to the FLEXIO_NEOPIXEL_Type structure
  @param  pixel  The pixel in the specified channel
  @param  r  8 bit red value to be stored
  @param  g  8 bit green value to be stored
  @param  b  8 bit blue value to be stored
*/
void fiopix_setPixelRGB(FLEXIO_NEOPIXEL_Type *fiopix, uint32_t pixel, uint8_t r, uint8_t g, uint8_t b);

/*!
  @brief  This sets the value of the respective pixel data
  @param  fiopix  Pointer to the FLEXIO_NEOPIXEL_Type structure
  @param  pixel  The pixel in the specified channel
  @param  r  8 bit red value to be stored
  @param  g  8 bit green value to be stored
  @param  b  8 bit blue value to be stored
  @param  w  8 bit blue value to be stored
*/
void fiopix_setPixelRGBW(FLEXIO_NEOPIXEL_Type *fiopix, uint32_t pixel, uint8_t r, uint8_t g, uint8_t b, uint8_t w);


#ifdef __cplusplus
 }
#endif

#endif