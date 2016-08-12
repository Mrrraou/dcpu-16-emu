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


typedef struct {
	uint16_t interrupt_queue[0xFF];
	int16_t count;
} InterruptQueue;

typedef struct {
	uint8_t key_queue[0xFFFF];
	uint16_t count;
} KeyQueue;

void queue_interrupts_add(InterruptQueue *queue, uint16_t message);
uint16_t queue_interrupts_remove(InterruptQueue *queue);

void queue_keys_add(KeyQueue *queue, uint8_t key);
uint8_t queue_keys_remove(KeyQueue *queue);
