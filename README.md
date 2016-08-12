# dcpu-16-emu

## Description

A simple DCPU-16 emulator, written in C.

## Compliance

* This interpreter tries to be compliant to the 1.7 DCPU-16 specification that
	can be found
	[here](https://raw.githubusercontent.com/hakuch/Dcpu16Universe/master/doc/hw/dcpu16.txt).

* This emulator probably doesn't run at a "stable" clock rate. It probably
	runs faster than 100kHz, and can vary per system.

* Having the interrupt queue growing longer than 256 interrupts "crashes" the
	VM, instead of putting it "on fire".

* The interrupts implementation is maybe buggy. Not sure.

	For any weird, non-compliant, unexpected or buggy behavior, please create an
	issue with the code you are using, the result you get and the expected result.

* **The code can be very ugly or horrible. This is my very first C program.
	Sorry. Please feel free to file an issue on ways to make the code cleaner.**

## Usage

### Example

To run it, type this command in a terminal:

    ./d16-emu examples/hello_world.bin

This reads the program at the path supplied at the first argument, and copies it
to the virtual machine's memory at offset 0x0000.

Programs should be supplied in little-endian mode, and have to be assembled.

Using [Organic](https://github.com/SirCmpwn/Organic) with the `--little-endian`
parameter is highly recommended.
It is required to compile the test programs supplied.

## Building

You need libsdl2 and gcc.
You should also put [Organic](https://github.com/SirCmpwn/Organic) in your
PATH environment variable.

To build the programn and all the examples using Organic:

		make all

To build only the program:

		make

To build only the examples, using Organic:

		make examples

## Credits

* Mojang, Markus Persson for the DCPU-16
* TuxSH for helping me with C
* The cool guys (#CTRDev) for helping me with some C too and SDL2 (and for
	bearing my lack of sleep sometimes)
* SirCmpwn for Organic and Tomato (which source's was useful for implementing
	some stuff)
* Bisqwit for his C++ DCPU-16 emulator, which source's helped when implementing
	ticking, for the screen refresh rate and the hardware clock

## License

This program and its examples are published with the GNU GPLv3 license.
See LICENSE for more details.
