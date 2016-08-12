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
#include <SDL2/SDL.h>

#include "interpreter.h"
#include "hardware/LEM1802.h"
#include "hardware/keyboard.h"
#include "hardware/clock.h"


uint32_t fsize(FILE* fp) {
	fseek(fp, 0, SEEK_END);
	uint32_t size = ftell(fp);
	rewind(fp);

	return size;
}

int32_t main(int32_t argc, char const *argv[]) {
	FILE *fp;
	uint32_t size;
	void *program;
	InterpreterState *state;
	Device *monitor, *keyboard, *hwclock;

	fp = fopen(argv[1], "r");
	if(!fp)
	{
		printf("The file could not be opened.");
		return 1;
	}
	size = fsize(fp);

	printf("File size: %u bytes\n", size);

	program = malloc(size);
	fread(program, sizeof(uint8_t), size, fp);

	fclose(fp);

	state = interpreter_init_state();

	uint32_t result = interpreter_load_program_buffer(state, program, size);
	free(program);
	// Things didn't go well; return.
	if(result != 0)
		return 1;

	if(SDL_Init(SDL_INIT_VIDEO) < 0)
		printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());

	monitor = LEM1802_attach(state);
	keyboard = Keyboard_attach(state);
	hwclock = Clock_attach(state);

	interpreter_run_program(state);

	fp = fopen("memdump.bin", "w");
	fwrite(state->memory, 2, 0x10000, fp);
	fclose(fp);
	printf("Dumped memory\n");

	printf("Execution ended.\n");
	bool stop; SDL_Event e;
	while(!stop)
	{
		while(SDL_PollEvent(&e) != 0)
		{
			switch(e.type)
			{
				case SDL_QUIT: stop = true; break;
				default: monitor->tick(monitor); break;
			}
		}
	}

	hardware_detach(state, monitor);
	hardware_detach(state, keyboard);
	hardware_detach(state, hwclock);
	SDL_Quit();

	return 0;
}
