#include <iostream>
#include "chip8.h"

chip8 chip;

int main(int argc, char* argv[]) {
	if (argc != 2) {
		std::cout << "Usage: chip8 <game>" << std::endl;
		return -1;
	}
	string game = argv[1];
	chip.initialise();
	chip.loadGame(game);
	std::cout << "Successfully loaded game" << std::endl;

	for (;;) {
		chip.emulateCycle();

		if (chip.drawFlag) {
			// Draw graphics
		}

		chip.setKeys();
	}
}