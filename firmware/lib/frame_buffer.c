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

// Increase brightness of 4 bit pixel in framebuffer
void addPixel(unsigned x, unsigned y, uint8_t shade)
{
	// screen clipping needed for aaLine
	if (x >= DISPLAY_WIDTH || y >= DISPLAY_HEIGHT)
		return;

	shade &= 0x0F;

	uint8_t *p = &g_frameBuff[x / 2 + y * (DISPLAY_WIDTH / 2)];

	if ((x & 0x01) == 0)
		shade <<= 4;

	*p |= shade;
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

// set all pixels to a shade
void fill(uint8_t shade)
{
	shade |= shade << 4;
	memset(g_frameBuff, shade, DISPLAY_WIDTH * DISPLAY_HEIGHT / 2);
}

// Draw one horizontal line with a certain shade (fast, no checks)
static void hLine(unsigned x1, unsigned x2, unsigned y, unsigned shade)
{
	uint8_t *p = &g_frameBuff[x1 / 2 + y * (DISPLAY_WIDTH / 2)];

	if (x1 & 0x01) {
		*p = (*p & 0xF0) | shade;  // set lower nibble only
		p++;
		x1++;
	}

	unsigned len = (x2 - x1 + 1) / 2;
	memset(p, (shade << 4) | shade, len);  // set bytes / words

	if ((x2 & 0x01) == 0) {
		p += len;
		*p = (*p & 0x0F) | (shade << 4);  // set upper nibble only
	}
}

static void swap(int *a, int *b)
{
	int t = *a;
	*a = *b;
	*b = t;
}

// limits val to l <= val <= h
static int limit(int l, int val, int h)
{
	if (val < l) val = l;
	if (val > h) val = h;
	return val;
}

void rect(int x1, int y1, int x2, int y2, uint8_t shade)
{
	shade &= 0x0F;

	x1 = limit(0, x1, DISPLAY_WIDTH - 1);
	x2 = limit(0, x2, DISPLAY_WIDTH - 1);
	y1 = limit(0, y1, DISPLAY_HEIGHT - 1);
	y2 = limit(0, y2, DISPLAY_HEIGHT - 1);

	if (x1 > x2)
		swap(&x1, &x2);
	if (y1 > y2)
		swap(&y1, &y2);

	for (int row=y1; row<=y2; row++)
		hLine(x1, x2, row, shade);

	printf("%d %d\n", x1, x2);
}
