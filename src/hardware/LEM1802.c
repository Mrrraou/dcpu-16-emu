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
#include <SDL2/SDL.h>

#include "interpreter.h"
#include "hardware.h"
#include "hardware/LEM1802.h"

#define PIXEL_SIZE		4
#define CELL_WIDTH		4
#define CELL_HEIGHT		8
#define SCREEN_WIDTH	128 * PIXEL_SIZE
#define SCREEN_HEIGHT	96	* PIXEL_SIZE
#define WINDOW_WIDTH	SCREEN_WIDTH	+ PIXEL_SIZE * 4
#define WINDOW_HEIGHT	SCREEN_HEIGHT + PIXEL_SIZE * 4


enum {
	INT_MEM_MAP_SCREEN,
	INT_MEM_MAP_FONT,
	INT_MEM_MAP_PALETTE,
	INT_SET_BORDER_COLOR,
	INT_MEM_DUMP_FONT,
	INT_MEM_DUMP_PALETTE
};

Device* LEM1802_attach(InterpreterState *state) {
	Device *d = calloc(1, sizeof(Device));
	LEM1802 *m = calloc(1, sizeof(LEM1802));

	// Create the SDL window
	m->window = SDL_CreateWindow(
		"LEM1802 - Low Energy Monitor",
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		WINDOW_WIDTH,
		WINDOW_HEIGHT,
		SDL_WINDOW_SHOWN
	);
	if(!m->window)
	{
		printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
		free(m);
		return 0;
	}

	m->screenSurface = SDL_GetWindowSurface(m->window);
	// Fills the window with black
	SDL_FillRect(
		m->screenSurface,
		NULL,
		SDL_MapRGB(m->screenSurface->format, 0, 0, 0)
	);

	SDL_UpdateWindowSurface(m->window);


	// Link to monitor data
	d->data = (void*) m;

	// Hardware info
	d->id = 0x7349F615;
	d->version = 0x1802;
	d->manufacturer = 0x1C6C8B36;

	// Setting up "events"
	d->fire_interrupt = &LEM1802_fire_interrupt;
	d->tick = &LEM1802_tick;
	d->detach = &LEM1802_destroy;

	m->startup_time = 120;

	// Setting up device in the virtual machine
	hardware_attach(state, d);
	return d;
}

void LEM1802_fire_interrupt(Device *device) {
	LEM1802 *m = (LEM1802*) device->data;
	switch(device->attached_state->registers[A])
	{
		case INT_MEM_MAP_SCREEN:
			m->screen_map_address = device->attached_state->registers[B];
			if(m->screen_map_address == 0)
				m->startup_time = 120;
			break;
		case INT_MEM_MAP_FONT:
			m->font_map_address = device->attached_state->registers[B];
			break;
		case INT_MEM_MAP_PALETTE:
			m->palette_map_address = device->attached_state->registers[B];
			break;
		case INT_SET_BORDER_COLOR:
			m->border_color_index = device->attached_state->registers[B] & 0xF;
			break;
		case INT_MEM_DUMP_FONT:
		{
			uint16_t i;
			for(i = 0; i < 256; i++)
			{
				device->attached_state->
					memory[((uint16_t) device->attached_state->registers[B] + i)]
					= LEM1802_default_font[i];
				interpreter_tick(device->attached_state, 1);
			}
			break;
		}
		case INT_MEM_DUMP_PALETTE:
		{
			uint8_t i;
			for(i = 0; i < 16; i++)
			{
				device->attached_state->
					memory[((uint16_t) device->attached_state->registers[B] + i)]
					= LEM1802_default_palette[i];
				interpreter_tick(device->attached_state, 1);
			}
			break;
		}
	}
}

uint32_t LEM1802_get_palette_color(Device *device, uint8_t index) {
	LEM1802 *m = (LEM1802*) device->data;

	// Only 16 colors in the palette
	index &= 0xF;
	uint16_t color; uint8_t r, g, b;

	if(m->palette_map_address == 0)
		color = LEM1802_default_palette[index];
	else
		color = device->attached_state->memory[((uint16_t) m->palette_map_address + index)];

	r = (color & 0xF00) >> 8; r |= r << 4;
	g = (color & 0x0F0) >> 4; g |= g << 4;
	b = (color & 0x00F) >> 0; b |= b << 4;

	return SDL_MapRGB(m->screenSurface->format, r, g, b);
}

uint32_t LEM1802_get_font_character(Device *device, uint8_t character) {
	LEM1802 *m = (LEM1802*) device->data;

	// Only take the last 7 bits
	character &= 0x7F;

	if(m->font_map_address == 0)
		return (LEM1802_default_font[character * 2] << 16) | (LEM1802_default_font[character * 2 + 1]);
	else
		return
			 (device->attached_state->memory[((uint16_t) m->font_map_address + (character * 2))] << 16)
			|	device->attached_state->memory[((uint16_t) m->font_map_address + (character * 2 + 1))];
}

void LEM1802_render(Device *device) {
	LEM1802 *m = (LEM1802*) device->data;

	if(m->screen_map_address == 0 || m->startup_time > 0)
		SDL_FillRect(
			m->screenSurface,
			NULL,
			SDL_MapRGB(m->screenSurface->format, 0, 0, 0)
		);
	else
	{
		// Borders
		SDL_FillRect(
			m->screenSurface,
			NULL,
			LEM1802_get_palette_color(device, m->border_color_index)
		);

		uint8_t x, y;
		for(x = 0; x < 32; x++)
		{
			for(y = 0; y < 12; y++)
			{
				uint16_t cell;
				union {
					uint32_t	l;
					uint8_t		s[4];
				} c;
				SDL_Rect cell_rect = {
					.x = PIXEL_SIZE * 2 + PIXEL_SIZE * x * CELL_WIDTH,
					.y = PIXEL_SIZE * 2 + PIXEL_SIZE * y * CELL_HEIGHT,
					.w = PIXEL_SIZE * 4,
					.h = PIXEL_SIZE * 8
				};
				cell = device->attached_state->memory[m->screen_map_address + x + y * 32];
				c.l = LEM1802_get_font_character(device, cell & 0x7F);

				SDL_FillRect(
					m->screenSurface,
					&cell_rect,
					LEM1802_get_palette_color(device, (cell & 0x0F00) >> 8)
				);

				if(((cell & 0x0F00) >> 8) == ((cell & 0xF000) >> 12)
				|| ((cell & 0x80) != 0 && (m->blink & (1 << 8))))
					continue;

				uint32_t fcolor = LEM1802_get_palette_color(device, (cell & 0xF000) >> 12);
				uint8_t cx, cy;
				for(cx = 0; cx < 4; cx++)
				{
					for(cy = 0; cy < 8; cy++)
					{
						if(!((c.s[3 - cx] >> cy) & 1))
							continue;

						SDL_Rect pixel_rect = {
							.x = cell_rect.x + PIXEL_SIZE * cx,
							.y = cell_rect.y + PIXEL_SIZE * cy,
							.w = PIXEL_SIZE,
							.h = PIXEL_SIZE
						};
						SDL_FillRect(
							m->screenSurface,
							&pixel_rect,
							fcolor
						);
					}
				}
			}
		}
	}

	SDL_UpdateWindowSurface(m->window);
}

void LEM1802_tick(Device *device) {
	LEM1802 *m = (LEM1802*) device->data;

	for(m->frameskip += 3; m->frameskip >= 5000; m->frameskip -= 5000)
	{
		m->blink++;
  	LEM1802_render(device);
		if(m->startup_time)
			m->startup_time--;
	}
}

void LEM1802_destroy(Device *device) {
	SDL_DestroyWindow(((LEM1802*) device->data)->window);
	free(device->data);
}
