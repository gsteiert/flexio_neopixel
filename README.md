# flexio_neopixel
FlexIO based NeoPixel Driver

This is currently functional, but still a work in progress.

## Theory of Operation

This implementation utilizes two timers and two shifters.  One timer is in baud rate generator mode to set the bit rate and count bits, the other operates at double the bit rate to generate timing for the short pulse.  One shifter advances bits and the other shifter is used in logic mode to generate the waveform based on the clocks and bit data.

The inputs and outputs for logic mode are hard coded, so the shifter selection will determine the output pin.  For shifter i, when i is 0-3 the output will be on FlexIO Di+4, when i is 4-7 the output will be on FlexIO Di+8.

The library only lets you specify the first timer and the first shifter.  The other timer and shifter are sequential.

When shifting MSB, the first data starts at bit 31, so in 24bit mode the data is shifted by 8 bits.
