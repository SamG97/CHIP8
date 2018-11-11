#ifndef CHIP8_CHIP8_H
#define CHIP8_CHIP8_H
static const size_t program_size = 4096 - 512;
#endif //CHIP8_CHIP8_H

#include <iostream>

using std::string;

class chip8 {
public:
    void initialise();
    bool loadGame(string);
    void emulateCycle();
    bool drawFlag;
    unsigned char key[16];
    unsigned char gfx[64 * 32];
private:
    unsigned short opcode;
    unsigned char memory[4096];
    unsigned char V[16];
    unsigned short I;
    unsigned short pc;
    unsigned char delay_timer;
    unsigned char sound_timer;
    unsigned short stack[16];
    unsigned short sp;
};
