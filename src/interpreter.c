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
#include <string.h>

#include "interpreter.h"
#include "opcodes.h"
#include "queue.h"


static uint16_t literal_value;

InterpreterState* interpreter_init_state() {
	InterpreterState* state = calloc(1, sizeof(InterpreterState));
	return state;
}

void interpreter_reset(InterpreterState *state) {
	state->crashed = false;

	uint8_t i;
	for(i = 0; i < 8; i++)
		state->registers[i] = 0;

	state->program_counter = state->stack_pointer = state->extra = state->interrupt_address = 0;
}

uint32_t interpreter_load_program_buffer(InterpreterState *state, void *program, uint32_t size) {
	if(size >= 0x10000)
		return PROGRAM_TOO_LARGE;
	memcpy(state->memory, program, size);
	return 0;
}

uint16_t* interpreter_read_operand(InterpreterState *state, uint8_t operand, bool isA, bool skip) {
	uint16_t *opvalue;
	switch(operand)
	{
		// register
		case 0x00:
		case 0x01:
		case 0x02:
		case 0x03:
		case 0x04:
		case 0x05:
		case 0x06:
		case 0x07:
			#ifdef DEBUG
			printf("[R]\t%p: %04x\n", &state->registers[operand], state->registers[operand]);
			#endif
			opvalue = &state->registers[operand];
			interpreter_tick(state, 0);
			break;
		// [register]
		case 0x08:
		case 0x09:
		case 0x0a:
		case 0x0b:
		case 0x0c:
		case 0x0d:
		case 0x0e:
		case 0x0f:
			opvalue = &state->memory[state->registers[operand - 0x08]];
			interpreter_tick(state, 0);
			break;
		// [register + next word]
		case 0x10:
		case 0x11:
		case 0x12:
		case 0x13:
		case 0x14:
		case 0x15:
		case 0x16:
		case 0x17:
			#ifdef DEBUG
			printf(
				"[R+NW]\t%p: %04x+%04x=%04x=>%04x\n",
				&state->memory[(uint16_t)(state->registers[operand - 0x10] + state->memory[state->program_counter])],
				state->registers[operand - 0x10],
				state->memory[state->program_counter],
				(uint16_t)(state->registers[operand - 0x10] + state->memory[state->program_counter]),
				state->memory[(uint16_t)(state->registers[operand - 0x10] + state->memory[state->program_counter])]
			);
			#endif
			opvalue = &state->memory[(uint16_t)(state->registers[operand - 0x10] + state->memory[state->program_counter++])];
			interpreter_tick(state, 1);
			break;
		// (PUSH / [--SP]) or (POP / [SP++])
		case 0x18:
			if(isA)
				// POP
				if(skip)
					opvalue = &state->memory[state->stack_pointer + 1];
				else
					opvalue = &state->memory[state->stack_pointer++];
			else
				// PUSH
				if(skip)
					opvalue = &state->memory[state->stack_pointer - 1];
				else
					opvalue = &state->memory[--state->stack_pointer];
			interpreter_tick(state, 0);
			break;
		// [SP] / PEEK
		case 0x19:
			opvalue = &state->memory[state->stack_pointer];
			interpreter_tick(state, 0);
			break;
		// [SP + next word] / PICK n
		case 0x1a:
			opvalue = &state->memory[(uint16_t)(state->stack_pointer + state->memory[state->program_counter++])];
			interpreter_tick(state, 1);
			break;
		// SP
		case 0x1b:
			opvalue = &state->stack_pointer;
			interpreter_tick(state, 0);
			break;
		// PC
		case 0x1c:
			opvalue = &state->program_counter;
			interpreter_tick(state, 0);
			break;
		// EX
		case 0x1d:
			opvalue = &state->extra;
			interpreter_tick(state, 0);
			break;
		// [next word]
		case 0x1e:
			#ifdef DEBUG
			printf(
				"[NW]\t%p: %04x = %04x (PC: %04x)\n",
				&state->memory[state->memory[state->program_counter]],
				state->memory[state->program_counter],
				state->memory[state->memory[state->program_counter]],
				state->program_counter
			);
			#endif
			opvalue = &state->memory[state->memory[state->program_counter++]];
			interpreter_tick(state, 1);
			break;
		// next word (literal)
		case 0x1f:
			literal_value = state->memory[state->program_counter++];
			#ifdef DEBUG
			printf("[NW;L]\t%p: %04x\n", &literal_value, literal_value);
			#endif
			opvalue = &literal_value;
			interpreter_tick(state, 1);
			break;
		// literal value
		default:
			literal_value = operand - 0x21;
			opvalue = &literal_value;
			interpreter_tick(state, 0);
			break;
	}
	return opvalue;
}

void interpreter_fire_interrupt(InterpreterState *state, uint16_t message) {
	if(state->interrupt_address == 0)
		return;

	#ifdef DEBUG
	printf("Got interrupt with message: %04x\n", message);
	#endif

	// No more than 256 interrupts in the queue
	// It's supposed to "take fire", but I just crash it.
	if(state->interrupt_queue.count + 1 > 0xFF) {
		state->crashed = true;
		return;
	}

	queue_interrupts_add(&state->interrupt_queue, message);
}

void interpreter_handle_interrupts(InterpreterState *state) {
	// Nothing to care about if queueing is enabled or the queue length is
	// inferior or equal to zero
	if(state->interrupt_queueing_enabled || state->interrupt_queue.count <= 0)
		return;

	uint16_t message = queue_interrupts_remove(&state->interrupt_queue);
	// Should be the right behavior if IA is 0
	if(state->interrupt_address == 0)
		return;

	// Enable interrupt queueing, as we trigger an interrupt
	state->interrupt_queueing_enabled = true;

	// Let's push PC and A to the stack
	state->memory[--state->stack_pointer] = state->program_counter;
	state->memory[--state->stack_pointer] = state->registers[A];

	// Now let's setup the registers
	state->program_counter = state->interrupt_address;
	state->registers[A] = message;
}

void interpreter_tick(InterpreterState *state, uint16_t ticks) {
	while(ticks-- > 0)
	{
		uint32_t i;
		for(i = 0; i < state->connected_devices; i++)
			if(state->devices[i]->tick)
				state->devices[i]->tick(state->devices[i]);
	}
}

void interpreter_run_program_step(InterpreterState *state) {
	if(!state->_skip_instruction)
		interpreter_handle_interrupts(state);

	#ifdef DEBUG
	uint16_t dbg_opc = state->program_counter;
	#endif
	uint16_t word = state->memory[state->program_counter++];
	uint8_t opcode = word & 0x1F;
	uint8_t b_val = (word & 0x3E0) >> 5;
	uint8_t a_val = (word & 0xFC00) >> 10;

	#ifdef DEBUG
	printf(
		"A: %04x \tB: %04x \tC: %04x \tX: %04x \tY: %04x \tZ: %04x \tI: %04x \tJ: %04x\n",
		state->registers[A], state->registers[B], state->registers[C],
		state->registers[X], state->registers[Y], state->registers[Z],
		state->registers[I], state->registers[J]
	);
	printf(
		"PC: %04x \tSP: %04x \tEX: %04x \tIA: %04x\n",
		state->program_counter, state->stack_pointer, state->extra,
		state->interrupt_address
	);
	printf(
		"Interrupt queueing: %u\tIn queue: %d\n",
		state->interrupt_queueing_enabled,
		state->interrupt_queue.count
	);
	#endif

	interpreter_tick(state, 1);

	uint16_t *a = interpreter_read_operand(state, a_val, true, state->_skip_instruction);

	#ifdef DEBUG
	printf("opcode:\t%02x\n", opcode);
	printf("%04x:\t%04x\n", dbg_opc, word);
	printf("%04x:\t%04x\n", state->program_counter - 1, state->memory[state->program_counter - 1]);
	printf("a:\t%02x\t([%p] %p: %04x)\n", a_val, &state->memory, a, *a);
	#endif

	// Special opcodes
	if(opcode == 0) {
		#ifdef DEBUG
		printf("special opcode:\t%02x\n", b_val);
		#endif
		if(state->_skip_instruction)
		{
			state->_skip_instruction = false;
			return;
		}
		switch(b_val)
		{
			case SPECIAL_OPCODE_JSR:
				state->memory[--state->stack_pointer] = state->program_counter;
				state->program_counter = *a;
				interpreter_tick(state, 2);
				break;
			case SPECIAL_OPCODE_INT:
				interpreter_fire_interrupt(state, *a);
				interpreter_tick(state, 3);
				break;
			case SPECIAL_OPCODE_IAG:
				*a = state->interrupt_address;
				interpreter_tick(state, 0);
				break;
			case SPECIAL_OPCODE_IAS:
				state->interrupt_address = *a;
				interpreter_tick(state, 0);
				break;
			case SPECIAL_OPCODE_RFI:
				state->interrupt_queueing_enabled = false;
				state->registers[A] = state->memory[state->stack_pointer++];
				state->program_counter = state->memory[state->stack_pointer++];
				interpreter_tick(state, 1);
				break;
			case SPECIAL_OPCODE_IAQ:
				state->interrupt_queueing_enabled = *a != 0;
				interpreter_tick(state, 1);
				break;
			case SPECIAL_OPCODE_HWN:
				*a = state->connected_devices;
				interpreter_tick(state, 1);
				break;
			case SPECIAL_OPCODE_HWQ:
			{
				Device *d = state->devices[*a];
				state->registers[A] = d->id & 0xFFFF;
				state->registers[B] = d->id >> 16;
				state->registers[C] = d->version;
				state->registers[X] = d->manufacturer & 0xFFFF;
				state->registers[Y] = d->manufacturer >> 16;
				interpreter_tick(state, 3);
				break;
			}
			case SPECIAL_OPCODE_HWI:
			{
				if(state->devices[*a]->fire_interrupt != 0)
					state->devices[*a]->fire_interrupt(state->devices[*a]);
				interpreter_tick(state, 3);
				break;
			}
			default:
				#ifdef DEBUG
				printf("Invalid special opcode\n");
				#endif
				state->crashed = true;
				return;
		}
	}
	// Basic opcodes
	else
	{
		uint16_t *b = interpreter_read_operand(state, b_val, false, state->_skip_instruction);
		#ifdef DEBUG
		printf("b:\t%02x\t([%p] %p: %04x)\n", b_val, &state->memory, b, *b);
		#endif
		if(state->_skip_instruction)
		{
			switch(opcode)
			{
				case BASIC_OPCODE_IFB:
				case BASIC_OPCODE_IFC:
				case BASIC_OPCODE_IFE:
				case BASIC_OPCODE_IFN:
				case BASIC_OPCODE_IFG:
				case BASIC_OPCODE_IFA:
				case BASIC_OPCODE_IFL:
				case BASIC_OPCODE_IFU:
					// All conditionals should be skipped if one is skipped.
					break;
				default: state->_skip_instruction = false; break;
			}
			return;
		}
		switch(opcode)
		{
			case BASIC_OPCODE_SET: *b = *a; interpreter_tick(state, 0); break;
			case BASIC_OPCODE_ADD:
				// If there is an overflow
				if(*b > 0xFFFF - *a)
					state->extra = 0x0001;
				else
					state->extra = 0x0000;
				*b += *a;
				interpreter_tick(state, 1);
				break;
			case BASIC_OPCODE_SUB:
				// If there is an underflow
				if(*b < *a)
					state->extra = 0xFFFF;
				else
					state->extra = 0x0000;
				*b -= *a;
				interpreter_tick(state, 1);
				break;
			case BASIC_OPCODE_MUL:
				state->extra = ((*b * *a) >> 16) & 0xFFFF;
				*b *= *a;
				interpreter_tick(state, 1);
				break;
			case BASIC_OPCODE_MLI:
				state->extra = ((*((int16_t*) b) * *((int16_t*) a)) >> 16) & 0xFFFF;
				*((int16_t*) b) *= *((int16_t*) a);
				interpreter_tick(state, 1);
				break;
			case BASIC_OPCODE_DIV:
				if(a == 0)
				{
					*b = 0;
					state->extra = 0;
				}
				else
				{
					state->extra = ((*b << 16) / *a) & 0xFFFF;
					*b /= *a;
				}
				interpreter_tick(state, 2);
				break;
			case BASIC_OPCODE_DVI:
				if(a == 0)
				{
					*b = 0;
					state->extra = 0;
				}
				else
				{
					state->extra = ((*((int16_t*) b) << 16) / *((int16_t*) a)) & 0xFFFF;
					*((int16_t*) b) /= *((int16_t*) a);
				}
				interpreter_tick(state, 2);
				break;
			case BASIC_OPCODE_MOD:
				state->extra = ((*b << 16) / *a) & 0xFFFF;
				*b /= *a;
				interpreter_tick(state, 2);
				break;
			case BASIC_OPCODE_MDI:
				state->extra = ((*((int16_t*) b) << 16) / *((int16_t*) a)) & 0xFFFF;
				*((int16_t*) b) /= *((int16_t*) a);
				interpreter_tick(state, 2);
				break;
			case BASIC_OPCODE_AND: *b &= *a; interpreter_tick(state, 0); break;
			case BASIC_OPCODE_BOR: *b |= *a; interpreter_tick(state, 0); break;
			case BASIC_OPCODE_XOR: *b ^= *a; interpreter_tick(state, 0); break;
			case BASIC_OPCODE_SHR:
				state->extra = ((*b << 16) >> *a) & 0xFFFF;
				*b >>= *a;
				interpreter_tick(state, 0);
				break;
			case BASIC_OPCODE_ASR:
				state->extra = ((*((int16_t*) b) << 16) >> *((int16_t*) a)) & 0xFFFF;
				*((int16_t*) b) >>= *((int16_t*) a);
				interpreter_tick(state, 0);
				break;
			case BASIC_OPCODE_SHL:
				state->extra = ((*b << *a) >> 16) & 0xFFFF;
				*b <<= *a;
				interpreter_tick(state, 0);
				break;
			case BASIC_OPCODE_IFB: state->_skip_instruction = (*b & *a) == 0; interpreter_tick(state, 1); break;
			case BASIC_OPCODE_IFC: state->_skip_instruction = (*b & *a) != 0; interpreter_tick(state, 1); break;
			case BASIC_OPCODE_IFE: state->_skip_instruction = *b != *a; interpreter_tick(state, 1); break;
			case BASIC_OPCODE_IFN: state->_skip_instruction = *b == *a; interpreter_tick(state, 1); break;
			case BASIC_OPCODE_IFG: state->_skip_instruction = *b <= *a; interpreter_tick(state, 1); break;
			case BASIC_OPCODE_IFA:
				state->_skip_instruction = *((int16_t*) b) <= *((int16_t*) a);
				interpreter_tick(state, 1);
				break;
			case BASIC_OPCODE_IFL: state->_skip_instruction = *b >= *a; interpreter_tick(state, 1); break;
			case BASIC_OPCODE_IFU:
				state->_skip_instruction = *((int16_t*) b) >= *((int16_t*) a);
				interpreter_tick(state, 1);
				break;
			case BASIC_OPCODE_ADX:
				// If there is an overflow
				if(*b > 0xFFFF - *a - state->extra)
				{
					*b += *a + state->extra;
					state->extra = 0x0001;
				}
				else
				{
					*b += *a + state->extra;
					state->extra = 0x0000;
				}
				interpreter_tick(state, 2);
				break;
			case BASIC_OPCODE_SBX:
				// If there is an underflow
				if(*b < *a - state->extra)
				{
					*b -= *a - state->extra;
					state->extra = 0xFFFF;
				}
				// If there is an overflow
				else if(*b > 0xFFFF + *a - state->extra)
				{
					*b -= *a - state->extra;
					state->extra = 0x0001;
				}
				else
				{
					*b -= *a - state->extra;
					state->extra = 0x0000;
				}
				interpreter_tick(state, 2);
				break;
			case BASIC_OPCODE_STI:
				*b = *a;
				state->registers[I]++;
				state->registers[J]++;
				interpreter_tick(state, 1);
				break;
			case BASIC_OPCODE_STD:
				*b = *a;
				state->registers[I]--;
				state->registers[J]--;
				interpreter_tick(state, 1);
				break;
			default:
				#ifdef DEBUG
				printf("Invalid basic opcode\n");
				#endif
				state->crashed = true;
				return;
		}
	}
	#ifdef DEBUG
	printf("====\n");
	#endif
}

void interpreter_run_program(InterpreterState *state) {
	while(!state->crashed)
	{
		interpreter_run_program_step(state);
	}
}
