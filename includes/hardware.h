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


#pragma once

#include <stdint.h>
#include <stdbool.h>

typedef struct Device Device;

#include "interpreter.h"


struct Device {
	// Pointer to device data
	void *data;

	// Runtime
	InterpreterState *attached_state;

	// Methods

	// This fires an interrupt to the device.
	// This method is called when HWI is used.
	void (*fire_interrupt)(Device*);

	// This method is called per instruction, right after interrupt triggering.
	void (*tick)(Device*);

	// This method is called when a device is detached.
	// This should be used to free memory, notably.
	void (*detach)(Device*);

	// Device information
	uint32_t id;
	uint16_t version;
	uint32_t manufacturer;
};

void hardware_attach(InterpreterState *state, Device *device);
void hardware_detach(InterpreterState *state, Device *device);
