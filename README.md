# flexio_neopixel
FlexIO based NeoPixel Driver

This is currently functional, but still a work in progress.

## Theory of Operation

This implementation utilizes two timers and two shifters.  One timer is in baud rate generator mode to set the bit rate and count bits, the other operates at double the bit rate to generate timing for the short pulse.  One shifter advances bits and the other shifter is used in logic mode to generate the waveform based on the clocks and bit data.

The inputs and outputs for logic mode are hard coded, so the shifter selection will determine the output pin.  For shifter i, when i is 0-3 the output will be on FlexIO Di+4, when i is 4-7 the output will be on FlexIO Di+8.

The library only lets you specify the first timer and the first shifter.  The other timer and shifter are sequential.

When shifting MSB, the first data starts at bit 31, so in 24bit mode the data is shifted by 8 bits.

The timer status interrupt on the baud rate timer is used to service the transfer because it is activated at the end of a pixels worth of bits being shifted out.  After all the pixels have been shifted out, the interrupt handler clears the logic look up table to turn off the output and shifts two or three more pixels worth of data depending on if the LEDs are 32 or 24 bit.  These extra pixels with the output off ensure the reset pulse duration before the busy flag is cleared. 

## Building

Demos are in the examples directory.  The NXP MCUX-SDK is huge so it is not included.  There is a build script in the example that expects the github.com/nxpmicro/mcux-sdk repo to be in a parallel directory at the same level as this repo.  The instructions are the same as for the examples in the mcux-sdk.  Make sure you have an ARMGCC_DIR environment variable pointing to your gcc install directory.  See https://github.com/nxpmicro/mcux-sdk/blob/main/docs/run_a_project_using_armgcc.md for more information.  
