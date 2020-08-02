#ifndef FRAME_BUFFER_H
#define FRAME_BUFFER_H
#include <stdint.h>

#define DISPLAY_WIDTH  256
#define DISPLAY_HEIGHT  64

extern uint8_t g_frameBuff[DISPLAY_HEIGHT * DISPLAY_WIDTH / 2];

// SET / GET a single pixel to a specific shade (0 - 15) in the framebuffer
void setPixel(unsigned x, unsigned y, uint8_t shade);
uint8_t getPixel(unsigned x, unsigned y);

// Set whole screen to fixed shade
void setAll(uint8_t shade);

#endif
