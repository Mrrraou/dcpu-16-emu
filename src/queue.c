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
#include <string.h>

#include "interpreter.h"
#include "queue.h"


void queue_interrupts_add(InterruptQueue* queue, uint16_t message) {
	queue->interrupt_queue[queue->count++] = message;
}

uint16_t queue_interrupts_remove(InterruptQueue* queue) {
	uint16_t message = queue->interrupt_queue[0];
	// Efficiency ? F*** that.
	// Copies items starting from index 1 to start from index 0
	memmove(
		&queue->interrupt_queue,
		((void*) &queue->interrupt_queue) + 2,
		--queue->count * 2
	);
	return message;
}


void queue_keys_add(KeyQueue* queue, uint8_t key) {
	if(queue->count < 0xFFFF)
		queue->key_queue[queue->count++] = key;
}

uint8_t queue_keys_remove(KeyQueue* queue) {
	if(queue->count < 1)
		return 0;

	uint8_t message = queue->key_queue[0];
	memmove(
		&queue->key_queue,
		((void*) &queue->key_queue) + 1,
		--queue->count
	);
	return message;
}
