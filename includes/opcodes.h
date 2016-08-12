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

// Basic opcodes
#define BASIC_OPCODE_SET 0x01
#define BASIC_OPCODE_ADD 0x02
#define BASIC_OPCODE_SUB 0x03
#define BASIC_OPCODE_MUL 0x04
#define BASIC_OPCODE_MLI 0x05
#define BASIC_OPCODE_DIV 0x06
#define BASIC_OPCODE_DVI 0x07
#define BASIC_OPCODE_MOD 0x08
#define BASIC_OPCODE_MDI 0x09

#define BASIC_OPCODE_AND 0x0A
#define BASIC_OPCODE_BOR 0x0B
#define BASIC_OPCODE_XOR 0x0C
#define BASIC_OPCODE_SHR 0x0D
#define BASIC_OPCODE_ASR 0x0E
#define BASIC_OPCODE_SHL 0x0F

#define BASIC_OPCODE_IFB 0x10
#define BASIC_OPCODE_IFC 0x11
#define BASIC_OPCODE_IFE 0x12
#define BASIC_OPCODE_IFN 0x13
#define BASIC_OPCODE_IFG 0x14
#define BASIC_OPCODE_IFA 0x15
#define BASIC_OPCODE_IFL 0x16
#define BASIC_OPCODE_IFU 0x17

#define BASIC_OPCODE_ADX 0x1A
#define BASIC_OPCODE_SBX 0x1B

#define BASIC_OPCODE_STI 0x1E
#define BASIC_OPCODE_STD 0x1F

// Special opcodes
#define SPECIAL_OPCODE_JSR 0x01

#define SPECIAL_OPCODE_INT 0x08
#define SPECIAL_OPCODE_IAG 0x09
#define SPECIAL_OPCODE_IAS 0x0A
#define SPECIAL_OPCODE_RFI 0x0B
#define SPECIAL_OPCODE_IAQ 0x0C

#define SPECIAL_OPCODE_HWN 0x10
#define SPECIAL_OPCODE_HWQ 0x11
#define SPECIAL_OPCODE_HWI 0x12
