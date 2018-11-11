#include <stdio.h>
#include <GL/glut.h>
#include "chip8.h"

using std::cout;
using std::endl;

static const int screen_width = 64;
static const int screen_height = 32;
static const int multiplier = 10;

int display_width = screen_width * multiplier;
int display_height = screen_height * multiplier;

void display();
void updateQuads(const chip8&);
void drawPixel(int, int);
void reshape_window(GLsizei, GLsizei);
void keyboardUp(unsigned char, int, int);
void keyboardDown(unsigned char, int, int);

chip8 chip;

int main(int argc, char **argv) {
    if (argc < 2) {
        cout << "Usage: chip8 <game>" << endl;
        return EXIT_FAILURE;
    }

    // Load game
    string game = argv[1];
    chip.initialise();
    if (!chip.loadGame(game))
        return EXIT_FAILURE;
    cout << "Successfully loaded game" << endl;

    // Setup OpenGL
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);

    glutInitWindowSize(display_width, display_height);
    glutInitWindowPosition(320, 320);
    glutCreateWindow("CHIP8");

    glutDisplayFunc(display);
    glutIdleFunc(display);
    glutReshapeFunc(reshape_window);
    glutKeyboardFunc(keyboardDown);
    glutKeyboardUpFunc(keyboardUp);

    glutMainLoop();

    return 0;
}

void drawPixel(int x, int y) {
    glBegin(GL_QUADS);
    glVertex3f((x * multiplier) + 0.0f, (y * multiplier) + 0.0f, 0.0f);
    glVertex3f((x * multiplier) + 0.0f, (y * multiplier) + multiplier, 0.0f);
    glVertex3f((x * multiplier) + multiplier, (y * multiplier) + multiplier, 0.0f);
    glVertex3f((x * multiplier) + multiplier, (y * multiplier) + 0.0f, 0.0f);
    glEnd();
}

void updateQuads(const chip8& c8) {
    // Draw
    for (int y = 0; y < 32; ++y)
        for (int x = 0; x < 64; ++x)
        {
            if (c8.gfx[(y * 64) + x] == 0)
                glColor3f(0.0f, 0.0f, 0.0f);
            else
                glColor3f(1.0f, 1.0f, 1.0f);

            drawPixel(x, y);
        }
}

void display() {
    chip.emulateCycle();

    if (chip.drawFlag) {
        // Clear framebuffer
        glClear(GL_COLOR_BUFFER_BIT);

        updateQuads(chip);        

        // Swap buffers!
        glutSwapBuffers();

        // Processed frame
        chip.drawFlag = false;
    }
}

void reshape_window(GLsizei w, GLsizei h)
{
    glClearColor(0.0f, 0.0f, 0.5f, 0.0f);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, w, h, 0);
    glMatrixMode(GL_MODELVIEW);
    glViewport(0, 0, w, h);

    // Resize quad
    display_width = w;
    display_height = h;
}

void keyboardDown(unsigned char key, int x, int y) {
    switch (key) {
    case '\x1B':
        exit(EXIT_SUCCESS); // Exit on Esc
    case '1':
        chip.key[0x1] = 1;
        break;
    case '2':
        chip.key[0x2] = 1;
        break;
    case '3':
        chip.key[0x3] = 1;
        break;
    case '4':
        chip.key[0xC] = 1;
        break;
    case 'q':
        chip.key[0x4] = 1;
        break;
    case 'w':
        chip.key[0x5] = 1;
        break;
    case 'e':
        chip.key[0x6] = 1;
        break;
    case 'r':
        chip.key[0xD] = 1;
        break;
    case 'a':
        chip.key[0x7] = 1;
        break;
    case 's':
        chip.key[0x8] = 1;
        break;
    case 'd':
        chip.key[0x9] = 1;
        break;
    case 'f':
        chip.key[0xE] = 1;
        break;
    case 'z':
        chip.key[0xA] = 1;
        break;
    case 'x':
        chip.key[0x0] = 1;
        break;
    case 'c':
        chip.key[0xB] = 1;
        break;
    case 'v':
        chip.key[0xF] = 1;
        break;
    }
}

void keyboardUp(unsigned char key, int x, int y) {
    switch (key) {
    case '1':
        chip.key[0x1] = 0;
        break;
    case '2':
        chip.key[0x2] = 0;
        break;
    case '3':
        chip.key[0x3] = 0;
        break;
    case '4':
        chip.key[0xC] = 0;
        break;
    case 'q':
        chip.key[0x4] = 0;
        break;
    case 'w':
        chip.key[0x5] = 0;
        break;
    case 'e':
        chip.key[0x6] = 0;
        break;
    case 'r':
        chip.key[0xD] = 0;
        break;
    case 'a':
        chip.key[0x7] = 0;
        break;
    case 's':
        chip.key[0x8] = 0;
        break;
    case 'd':
        chip.key[0x9] = 0;
        break;
    case 'f':
        chip.key[0xE] = 0;
        break;
    case 'z':
        chip.key[0xA] = 0;
        break;
    case 'x':
        chip.key[0x0] = 0;
        break;
    case 'c':
        chip.key[0xB] = 0;
        break;
    case 'v':
        chip.key[0xF] = 0;
        break;
    }
}
