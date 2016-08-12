rwildcard = $(foreach d, $(wildcard $1*), $(filter $(subst *, %, $2), $d) $(call rwildcard, $d/, $2))

CC=gcc

dir_build			:= build
dir_includes	:= includes
dir_source		:= src
dir_examples	:= examples

CFLAGS=-Wall -Wextra -lSDL2 -I"$(dir_includes)"


build/d16-emu: $(dir_source)/*.c $(dir_source)/hardware/*.c $(dir_build)
	gcc -o $(dir_build)/d16-emu $(dir_source)/*.c $(dir_source)/hardware/*.c $(CFLAGS)

.PHONY: $(dir_build)
$(dir_build):
	@mkdir -p "$(dir_build)"

.PHONY: examples
examples: $(dir_examples)/hello_world.bin \
	$(dir_examples)/simple_operations.bin \
	$(dir_examples)/interrupts.bin \
	$(dir_examples)/hardware.bin

$(dir_examples)/%.bin: $(dir_examples)/%.dasm
	organic --little-endian -o $@ $<


.PHONY: clean-program
clean-program:
	rm -f $(dir_build)/d16-emu

.PHONY: clean
clean:
	rm -f $(dir_build)/d16-emu $(dir_examples)/*.bin

.PHONY: all
all: d16-emu examples
