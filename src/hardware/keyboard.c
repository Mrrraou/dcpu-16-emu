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
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <SDL2/SDL.h>

#include "interpreter.h"
#include "hardware.h"
#include "hardware/keyboard.h"
#include "queue.h"


enum {
	INT_CLEAR_KBD_BUFFER,
	INT_STORE_NEXT_KEY,
	INT_IS_KEY_PRESSED,
	INT_SET_INTERRUPTS
};

Device* Keyboard_attach(InterpreterState *state) {
	Device *d = calloc(1, sizeof(Device));
	Keyboard *k = calloc(1, sizeof(Keyboard));

	// Link to keyboard data
	d->data = (void*) k;

	// Hardware info
	d->id = 0x30CF7406;
	d->version = 0x0001;
	d->manufacturer = 0x00000000;

	// Setting up "events"
	d->fire_interrupt = &Keyboard_fire_interrupt;
	d->tick = &Keyboard_tick;
	d->detach = &Keyboard_detach;

	// Setting up device in the virtual machine
	hardware_attach(state, d);
	return d;
}

void Keyboard_fire_interrupt(Device *device) {
	Keyboard *k = (Keyboard*) device->data;
	switch(device->attached_state->registers[A])
	{
		case INT_CLEAR_KBD_BUFFER:	k->key_queue.count = 0; break;
		case INT_STORE_NEXT_KEY:		device->attached_state->registers[C] = queue_keys_remove(&k->key_queue); break;
		case INT_IS_KEY_PRESSED:
			device->attached_state->registers[C] = k->pressed_keys[device->attached_state->registers[B] & 0xFF];
			break;
		case INT_SET_INTERRUPTS:		k->interrupt_message = device->attached_state->registers[B]; break;
	}
}

uint8_t Keyboard_get_key(SDL_Event *e) {
	uint8_t key;
	switch(e->key.keysym.sym)
	{
		case SDLK_BACKSPACE: 	key = 0x10; break;
		case SDLK_RETURN: 		key = 0x11; break;
		case SDLK_INSERT: 		key = 0x12; break;
		case SDLK_DELETE: 		key = 0x13; break;
		case SDLK_UP: 				key = 0x80; break;
		case SDLK_DOWN: 			key = 0x81; break;
		case SDLK_LEFT: 			key = 0x82; break;
		case SDLK_RIGHT: 			key = 0x83; break;
		case SDLK_LSHIFT:
		case SDLK_RSHIFT:
			key = 0x90;
			break;
		case SDLK_LCTRL:
		case SDLK_RCTRL:
			key = 0x91;
			break;
		default:
		{
			char c = e->key.keysym.sym;
			if(e->key.keysym.mod & (KMOD_CAPS | KMOD_SHIFT))
				c = toupper(c);

			if(!(c >= 0x20 && c <= 0x7f))
				return 0;

			key = c;
			break;
		}
	}
	return key;
}

void Keyboard_tick(Device *device) {
	Keyboard *k = (Keyboard*) device->data;

	for(k->tick += 3; k->tick >= 10000; k->tick -= 10000)
	{
		SDL_Event e;
		while(SDL_PollEvent(&e))
		{
			switch(e.type)
			{
				case SDL_QUIT:
					// The easy solution
					device->attached_state->crashed = true;
					break;
				case SDL_KEYDOWN:
				{
					uint8_t key = Keyboard_get_key(&e);
					if(!key)
						return;

					k->pressed_keys[key] = true;
					queue_keys_add(&k->key_queue, key);

					if(k->interrupt_message)
						interpreter_fire_interrupt(device->attached_state, k->interrupt_message);
					break;
				}
				case SDL_KEYUP:
				{
					uint8_t key = Keyboard_get_key(&e);
					if(!key)
						return;
					k->pressed_keys[key] = false;
					break;
				}
			}
		}
	}
}

void Keyboard_detach(Device *device) {
	free(device->data);
}
