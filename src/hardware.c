/* dcpu-16-emu - A simple DCPU-16 emulator written in C.
 * Copyright (C) 2016  Mrrraou
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include "interpreter.h"
#include "hardware.h"


void hardware_attach(InterpreterState *state, Device *device) {
	device->attached_state = state;
	state->devices[state->connected_devices++] = device;
	printf(
		"Attached hardware %u: %08x ver.%04x\n",
		state->connected_devices,
		device->id, device->version
	);
}

// This does not remove the device from the device array !!
// This should only be used when execution terminates !!
void hardware_detach(InterpreterState *state, Device *device) {
	if(!device->detach)
		device->detach(device);
	free(device);
}
