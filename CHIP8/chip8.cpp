#include <fstream>
#include <string>
#include <cstdlib>
#include "chip8.h"
using std::begin;
using std::end;
using std::ifstream;
using std::ios;

static const size_t PROGRAM_SIZE = 4096 - 512;

unsigned char chip8_fontset[80] =
{
		0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
		0x20, 0x60, 0x20, 0x20, 0x70, // 1
		0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
		0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
		0x90, 0x90, 0xF0, 0x10, 0x10, // 4
		0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
		0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
		0xF0, 0x10, 0x20, 0x40, 0x40, // 7
		0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
		0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
		0xF0, 0x90, 0xF0, 0x90, 0x90, // A
		0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
		0xF0, 0x80, 0x80, 0x80, 0xF0, // C
		0xE0, 0x90, 0x90, 0x90, 0xE0, // D
		0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
		0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

void chip8::initialise() {
	pc = 0x200;
	opcode = 0;
	I = 0;
	sp = 0;

	for (auto i = begin(gfx); i != end(gfx); ++i)
		*i = 0;

	for (auto i = begin(stack); i != end(stack); ++i)
		*i = 0;
	for (auto i = begin(V); i != end(V); ++i)
		*i = 0;
	for (auto i = begin(memory); i != end(memory); ++i)
		*i = 0;
	for (int i = 0; i < 80; ++i) {
		memory[i + 80] = chip8_fontset[i];
	}

	delay_timer = 0;
	sound_timer = 0;
}

void printUnknownOpcode(short opcode) {
	std::cout << "Unknown opcode: 0x" << opcode << std::endl;
}

void chip8::emulateCycle() {
	opcode = memory[pc] << 8 | memory[pc + 1];

	switch (opcode & 0xF000) {
	case 0x0000:
		switch (opcode & 0x000F) {
		case 0x0000: // 00E0: Clears the screen
			for (auto i = begin(gfx); i != end(gfx); ++i)
				*i = 0;
			drawFlag = true;
			pc += 2;
			break;
		case 0x000E: // 00EE: Returns from a subroutine
			pc = stack[--sp];
			pc += 2;
			break;
		default:
			printUnknownOpcode(opcode);
		}
		break;
	case 0x1000: // 1NNN: Jumps to address NNN
		pc = opcode & 0x0FFF;
		break;
	case 0x2000: // 2NNN: Calls subroutine at NNN
		stack[sp] = pc;
		++sp;
		pc = opcode & 0x0FFF;
		break;
	case 0x3000: // 3XNN: Skips the next instruction if VX equals NN
		if (V[(opcode & 0x0F00) >> 8] == opcode & 0x00FF)
			pc += 4;
		else
			pc += 2;
		break;
	case 0x4000: // 4XNN: Skips the next instruction if VX doesn't equal NN
		if (V[(opcode & 0x0F00) >> 8] != opcode & 0x00FF)
			pc += 4;
		else
			pc += 2;
		break;
	case 0x5000: // 5XY0: Skips the next instruction if VX equals VY
		if (V[(opcode & 0x0F00) >> 8] == V[(opcode & 0x00F0) >> 4])
			pc += 4;
		else
			pc += 2;
		break;
	case 0x6000: // 6XNN: Sets VX to NN
		V[(opcode & 0x0F00) >> 8] = opcode & 0x00FF;
		pc += 2;
		break;
	case 0x7000: // 7XNN: Adds NN to VX
		V[(opcode & 0x0F00) >> 8] += opcode & 0x00FF;
		pc += 2;
		break;
	case 0x8000:
		switch (opcode & 0x000F) {
		case 0x0000: // 8XY0: Sets VX to the value of VY
			V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x00F0) >> 4];
			pc += 2;
			break;
		case 0x0001: // 8XY1: Sets VX to VX or VY
			V[(opcode & 0x0F00) >> 8] |= V[(opcode & 0x00F0) >> 4];
			pc += 2;
			break;
		case 0x0002: // 8XY2: Sets VX to VX and VY
			V[(opcode & 0x0F00) >> 8] &= V[(opcode & 0x00F0) >> 4];
			pc += 2;
			break;
		case 0x0003: // 8XY3: Sets VX to VX xor VY
			V[(opcode & 0x0F00) >> 8] ^= V[(opcode & 0x00F0) >> 4];
			pc += 2;
			break;
		case 0x0004: // 8XY4: Adds VY to VX. VF is set to 1 when there's a carry, and to 0 when there isn't.
			if (V[(opcode & 0x00F0) >> 4] > (0xFF - V[(opcode & 0x0F00) >> 8]))
				V[0xF] = 1; //carry
			else
				V[0xF] = 0;
			V[(opcode & 0x0F00) >> 8] += V[(opcode & 0x00F0) >> 4];
			pc += 2;
			break;
		case 0x0005: // 8XY5: VY is subtracted from VX. VF is set to 0 when there's a borrow, and 1 when there isn't.
			if (V[(opcode & 0x00F0) >> 4] > V[(opcode & 0x0F00) >> 8])
				V[0xF] = 0; //borrow
			else
				V[0xF] = 1;
			V[(opcode & 0x0F00) >> 8] -= V[(opcode & 0x00F0) >> 4];
			pc += 2;
			break;
		case 0x0006: // 8XY6: Stores the least significant bit of VX in VF and then shifts VX to the right by 1.
			V[0xF] = V[(opcode & 0x0F00) >> 8] & 0x01;
			V[(opcode & 0x0F00) >> 8] >>= 1;
			pc += 2;
			break;
		case 0x0007: // 8XY7: Sets VX to VY minus VX. VF is set to 0 when there's a borrow, and 1 when there isn't. 
			if (V[(opcode & 0x0F00) >> 8] > V[(opcode & 0x00F0) >> 4])
				V[0xF] = 0; //borrow
			else
				V[0xF] = 1;
			V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x00F0) >> 4] - V[(opcode & 0x0F00) >> 8];
			pc += 2;
			break;
		case 0x000E: // 8XYE: Stores the most significant bit of VX in VF and then shifts VX to the left by 1.
			V[0xF] = (V[(opcode & 0x0F00) >> 8] & 0x80) >> 7;
			V[(opcode & 0x0F00) >> 8] <<= 1;
			pc += 2;
			break;
		default:
			printUnknownOpcode(opcode);
		}
		break;
	case 0x9000: // 9XY0: Skips the next instruction if VX doesn't equal VY
		if (V[(opcode & 0x0F00) >> 8] != V[(opcode & 0x00F0) >> 4])
			pc += 4;
		else
			pc += 2;
		break;
	case 0xA000: // ANNN: Sets I to the address NNN
		I = opcode & 0x0FFF;
		pc += 2;
		break;
	case 0xB000: // BNNN: Jumps to the address NNN plus V0
		pc = V[0] + (opcode & 0x0FFF);
		break;
	case 0xC000: // CXNN: Sets VX to the result of a bitwise and operation on a random number (Typically: 0 to 255) and
				 // NN
		V[(opcode & 0x0F00) >> 8] = rand() & (opcode & 0x00FF);
		pc += 2;
		break;
	case 0xD000: // DXYN: Draws a sprite at coordinate (VX, VY) that has a width of 8 pixels and a height of N pixels.
				 // Each row of 8 pixels is read as bit-coded starting from memory location I; I value doesn’t change
				 // after the execution of this instruction. As described above, VF is set to 1 if any screen pixels
				 // are flipped from set to unset when the sprite is drawn, and to 0 if that doesn’t happen
	{
		unsigned short x = V[(opcode & 0x0F00) >> 8];
		unsigned short y = V[(opcode & 0x00F0) >> 4];
		unsigned short height = opcode & 0x000F;
		unsigned short pixel;

		V[0xF] = 0;
		for (int yline = 0; yline < height; yline++)
		{
			pixel = memory[I + yline];
			for (int xline = 0; xline < 8; xline++)
			{
				if ((pixel & (0x80 >> xline)) != 0)
				{
					if (gfx[(x + xline + ((y + yline) * 64))] == 1)
						V[0xF] = 1;
					gfx[x + xline + ((y + yline) * 64)] ^= 1;
				}
			}
		}

		drawFlag = true;
		pc += 2;
		break;
	}
	case 0xE000:
		switch (opcode & 0x00FF) {
		case 0x009E: // EX9E: Skips the next instruction if the key stored in VX is pressed
			if (key[V[(opcode & 0x0F00) >> 8]] != 0)
				pc += 4;
			else
				pc += 2;
			break;
		case 0x00A1: // EXA1: Skips the next instruction if the key stored in VX isn't pressed
			if (key[V[(opcode & 0x0F00) >> 8]] == 0)
				pc += 4;
			else
				pc += 2;
			break;
		default:
			printUnknownOpcode(opcode);
		}
		break;
	case 0xF000:
		switch (opcode & 0x00FF) {
		case 0x0007: // 0FX07: Sets VX to the value of the delay timer
			V[(opcode & 0x0F00) >> 8] = delay_timer;
			pc += 2;
			break;
		case 0x000A: // 0FX0A: A key press is awaited, and then stored in VX
		{
			bool keyPressed = false;
			for (int i = 0; i < 16; ++i) {
				if (key[i] != 0) {
					V[(opcode & 0x0F00) >> 8] = i;
					keyPressed = true;
					break;
				}
			}

			if (!keyPressed)
				return;

			pc += 2;
			break;
		}
		case 0x0015: // 0FX15: Sets the delay timer to VX
			delay_timer = V[(opcode & 0x0F00) >> 8];
			pc += 2;
			break;
		case 0x0018: // 0FX18: Sets the sound timer to VX
			sound_timer = V[(opcode & 0x0F00) >> 8];
			pc += 2;
			break;
		case 0x001E: // 0FX1E: Adds VX to I
			I += V[(opcode & 0x0F00) >> 8];
			pc += 2;
			break;
		case 0x0029: // 0FX29: Sets I to the location of the sprite for the character in VX. Characters 0-F (in
					 // hexadecimal) are represented by a 4x5 font
			I = V[(opcode & 0x0F00) >> 8] * 0x05;
			pc += 2;
			break;
		case 0x0033: // 0FX33: Stores the binary-coded decimal representation of VX, with the most significant of
					 // three digits at the address in I, the middle digit at I plus 1, and the least significant digit
					 // at I plus 2. (In other words, take the decimal representation of VX, place the hundreds digit
					 // in memory at location in I, the tens digit at location I+1, and the ones digit at location
					 // I+2.) 
			memory[I] = V[(opcode & 0x0F00) >> 8] / 100;
			memory[I + 1] = (V[(opcode & 0x0F00) >> 8] / 10) % 10;
			memory[I + 2] = (V[(opcode & 0x0F00) >> 8] % 100) % 10;
			pc += 2;
			break;
		case 0x0055: // 0FX55: Stores V0 to VX (including VX) in memory starting at address I. The offset from I is
					 // increased by 1 for each value written, but I itself is left unmodified
			for (int i = 0; i < 16; ++i)
				memory[I + i] = V[i];
			pc += 2;
			break;
		case 0x0065: // 0FX65: Fills V0 to VX (including VX) with values from memory starting at address I. The offset
					 // from I is increased by 1 for each value written, but I itself is left unmodified. 
			for (int i = 0; i < 16; ++i)
				V[i] = memory[I + i];
			pc += 2;
			break;
		default:
			printUnknownOpcode(opcode);
		}
	default:
		printUnknownOpcode(opcode);
	}

	if (delay_timer > 0)
		--delay_timer;

	if (sound_timer > 0) {
		if (sound_timer == 1)
			std::cout << "BEEP!" << std::endl;
		--sound_timer;
	}
}

bool chip8::loadGame(string filename) {
	std::ifstream f;
	f.open(filename);
	if (!f.good()) {
		std::cerr << "Error reading file " << filename << std::endl;
		return false;
	}
	f.read(reinterpret_cast<char*>(memory + 512), PROGRAM_SIZE);
	if (!f.good() && !f.eof()) {
		std::cerr << "error reading " << filename << std::endl;
		return false;
	}
	if (f.eof())
		return true;
	f.get();
	if (!f.eof()) {
		std::cerr << filename << " is too long" << std::endl;
		return false;
	}
	return true;
}

void chip8::setKeys() {

}