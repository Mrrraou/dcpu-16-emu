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
#include <SDL2/SDL.h>

#include "interpreter.h"
#include "hardware.h"
#include "queue.h"

typedef struct {
	uint16_t 	interrupt_message;
	bool			pressed_keys[0xFF];
	KeyQueue	key_queue;
	uint16_t	tick;
} Keyboard;


Device* Keyboard_attach(InterpreterState *state);

uint8_t Keyboard_get_key(SDL_Event *e);

void Keyboard_fire_interrupt(Device *device);
void Keyboard_tick(Device *device);
void Keyboard_detach(Device *device);
