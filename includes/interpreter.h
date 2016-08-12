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

typedef struct InterpreterState InterpreterState;

#include "queue.h"
#include "hardware.h"


#define PROGRAM_TOO_LARGE 0x1

#define NON_IMPLEMENTED(name,state) printf("Non implemented opcode: %s\n%x:\t%x\n", name, state->program_counter - 1, state->memory[state->program_counter - 1]);


enum Registers {
	A, B, C,
	X, Y, Z,
	I, J
};

struct InterpreterState {
	// Runtime
	bool crashed;
	bool interrupt_queueing_enabled;
	InterruptQueue interrupt_queue;
	bool _skip_instruction;

	// Hardware devices
	Device *devices[0xFFFF];
	uint16_t connected_devices;

	/* A word is 16 bits in DCPU-16 */
	// Registers
	uint16_t registers[8];

	// "System registers"
	uint16_t program_counter;
	uint16_t stack_pointer;
	uint16_t extra;
	uint16_t interrupt_address;

	// Memory
	uint16_t memory[0x10000];
};


InterpreterState* interpreter_init_state();
void interpreter_reset(InterpreterState *state);

uint32_t interpreter_load_program_buffer(InterpreterState *state, void *program, uint32_t size);

uint16_t* interpreter_read_operand(InterpreterState *state, uint8_t operand, bool isA, bool skip);

void interpreter_fire_interrupt(InterpreterState *state, uint16_t message);
void interpreter_handle_interrupts(InterpreterState *state);

void interpreter_tick(InterpreterState *state, uint16_t ticks);
void interpreter_run_program_step(InterpreterState *state);
void interpreter_run_program(InterpreterState *state);
