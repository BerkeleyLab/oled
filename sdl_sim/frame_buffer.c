// Implements a simple framebuffer and some drawing functions with 4 bit greyscale
#include <string.h>
#include <stdio.h>
#include "frame_buffer.h"

// framebuffer with `N_LAYERS` in MSB ABGR LSB format
// Colors are premultiplied with their alpha values for easier compositing
uint8_t g_frameBuff[DISPLAY_HEIGHT * DISPLAY_WIDTH / 2];

// Set a 4 bit pixel in framebuffer
void setPixel(unsigned x, unsigned y, uint8_t shade)
{
	// screen clipping needed for aaLine
	if (x >= DISPLAY_WIDTH || y >= DISPLAY_HEIGHT)
		return;

	shade &= 0x0F;

	uint8_t *p = &g_frameBuff[x / 2 + y * (DISPLAY_WIDTH / 2)];

	if (x & 0x01)
		*p = (*p & 0xF0) | shade;  // set lower nibble
	else
		*p = (*p & 0x0F) | (shade << 4);  // set upper nibble
}

// get a 4 bit pixel-value from framebuffer
uint8_t getPixel(unsigned x, unsigned y)
{
	x &= DISPLAY_WIDTH - 1;
	y &= DISPLAY_HEIGHT - 1;
	uint8_t p = g_frameBuff[x / 2 + y * (DISPLAY_WIDTH / 2)];
	if ((x & 0x01) == 0)
		p >>= 4;  // get upper nibble
	return p & 0x0F;
}

// set all pixels of a layer to a color
void setAll(uint8_t shade)
{
	shade |= shade << 4;
	memset(g_frameBuff, shade, DISPLAY_WIDTH * DISPLAY_HEIGHT / 2);
}
