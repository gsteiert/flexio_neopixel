# flexio_neopixel
FlexIO based NeoPixel Driver

This is currently functional, but still a work in progress.

## Theory of Operation

My original implementation required two shifters, one to shift data, and a second configured as logic to generate the waveform.  While thinking of ways to work around the logic shifter (that is not available in all FlexIO implementations) I discovered that outputs sharing a pin are OR'd together, thanks to [an undocumented feature that Eric Styger used in his implementation](https://mcuoneclipse.com/2016/05/22/nxp-flexio-generator-for-the-ws2812b-led-stripe-protocol/).  With this knowledge, I was able to replace the logic shifter with a hefty pull-down resistor.

This implementation utilizes two timers and a shifter.  One timer is in baud rate generator mode to set the bit rate and count bits, the other operates at double the bit rate to generate timing for the short pulse.  The shifter advances bits just like in the SPI implementation.  The fast timer and data are OR'd together by connecting them to the same data line, and the baud timer is connected to the output enable as an effective AND gate.  This requires a reasonably strong pull-down to get the output low quickly, but it uses fewer shifters and is not restricted to the hard coded logic mode outputs.

The library lets you specify the output pin, the shifter and the first timer.  The second timer must be the next timer after the one specified.

When shifting MSB, the first data starts at bit 31, so in 24bit mode the data is shifted by 8 bits.

The timer status interrupt on the baud rate timer is used to service the transfer because it is activated at the end of a pixels worth of bits being shifted out.  After all the pixels have been shifted out, the interrupt handler turns off the output and shifts two or three more pixels worth of data depending on if the LEDs are 32 or 24 bit.  These extra pixels with the output off ensure the reset pulse duration before the busy flag is cleared. 

## Building

Demos are in the examples directory.  The NXP MCUX-SDK is huge so it is not included.  There is a build script in the example that expects the github.com/nxpmicro/mcux-sdk repo to be in a parallel directory at the same level as this repo.  The instructions are the same as for the examples in the mcux-sdk.  Make sure you have an ARMGCC_DIR environment variable pointing to your gcc install directory.  See https://github.com/nxpmicro/mcux-sdk/blob/main/docs/run_a_project_using_armgcc.md for more information.  
