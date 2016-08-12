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
#include "hardware/clock.h"


enum {
	INT_SET_CLOCK_RATE,
	INT_GET_ELAPSED_TICKS,
	INT_SETUP_INTERRUPTS
};

Device* Clock_attach(InterpreterState *state) {
	Device *d = calloc(1, sizeof(Device));
 	Clock *c = calloc(1, sizeof(Clock));

	// Link to clock data
	d->data = (void*) c;

	// Hardware info
	d->id = 0x12D0B402;
	d->version = 0x0001;
	d->manufacturer = 0x00000000;

	// Setting up "events"
	d->fire_interrupt = &Clock_fire_interrupt;
	d->tick = &Clock_tick;
	d->detach = &Clock_detach;

	// Setting up device in the virtual machine
	hardware_attach(state, d);
	return d;
}

void Clock_fire_interrupt(Device *device) {
	Clock *c = (Clock*) device->data;
	switch(device->attached_state->registers[A])
	{
		case INT_SET_CLOCK_RATE:
			c->ticks = c->counter = 0;
			c->divider = device->attached_state->registers[B] * 5000;
			break;
		case INT_GET_ELAPSED_TICKS:
			device->attached_state->registers[C] = c->counter;
			break;
		case INT_SETUP_INTERRUPTS:
			c->interrupt_message = device->attached_state->registers[B];
			break;
	}
}

void Clock_tick(Device *device) {
	Clock *c = (Clock*) device->data;

	for(c->ticks += 3; c->divider && c->ticks >= c->divider; c->ticks -= c->divider)
	{
		c->counter++;
		if(c->interrupt_message)
			interpreter_fire_interrupt(device->attached_state, c->interrupt_message);
	}
}

void Clock_detach(Device *device) {
	free(device->data);
}
