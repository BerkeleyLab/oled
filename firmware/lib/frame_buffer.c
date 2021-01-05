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


	uint8_t *p = &g_frameBuff[x / 2 + y * (DISPLAY_WIDTH / 2)];

	if ((x & 0x01) == 0)
		shade <<= 4;
	else
		shade &= 0x0F;

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

static void limit(int *a, int *b, int lim) {
	if (*a < 0)
		*a = 0;
	if (*a > lim)
		*a = lim;
	if (*b < 0)
		*b = 0;
	if (*b > lim)
		*b = lim;
	if (*a > *b) {
		int c = *a;
		*a = *b;
		*b = c;
	}
}

// Draw one horizontal line with a certain shade (fast, no checks)
static void hLine(unsigned x, unsigned y, unsigned w, uint8_t shade)
{
	if (w == 0)
		return;
	// printf("hLine(%2d, %2d, %2d, %2d)\n", x, y, w, shade);

	uint8_t *p = &g_frameBuff[x / 2 + y * (DISPLAY_WIDTH / 2)];

	if (x & 0x01) {
		*p = (*p & 0xF0) | shade;  // set lower nibble only
		p++;
		x++;
		w--;
	}

	unsigned len = w / 2;
	memset(p, (shade << 4) | shade, len);  // set bytes / words

	if (((x + w) & 0x01)) {
		p += len;
		*p = (*p & 0x0F) | (shade << 4);  // set upper nibble only
	}
}

static void vLine(unsigned x, unsigned y, unsigned h, uint8_t shade)
{
	for (unsigned i=0; i<h; i++)
		setPixel(x, y + i, shade);
}

void fillRect(int x0, int x1, int y0, int y1, uint8_t shade)
{
	shade &= 0x0F;

	limit(&x0, &x1, DISPLAY_WIDTH - 1);
	limit(&y0, &y1, DISPLAY_HEIGHT - 1);
	unsigned w = x1 - x0 + 1;

	// printf("fillRect(%2d, %2d, %2d, %2d, %2d)\n", x0, x1, y0, y1, shade);

	for (int row=y0; row<=y1; row++)
		hLine(x0, row, w, shade);
}

void rect(int x0, int x1, int y0, int y1, uint8_t shade)
{
	limit(&x0, &x1, DISPLAY_WIDTH - 1);
	limit(&y0, &y1, DISPLAY_HEIGHT - 1);
	unsigned w = x1 - x0;
	unsigned h = y1 - y0;

	// printf("    rect(%2d, %2d, %2d, %2d, %2d)\n", x0, x1, y0, y1, shade);

	hLine(x0, y0, w, shade);
	hLine(x0, y0 + h, w, shade);
	vLine(x0, y0, h + 1, shade);
	vLine(x0 + w, y0, h + 1, shade);
}
