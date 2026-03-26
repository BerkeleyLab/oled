// Implements a simple framebuffer and some drawing functions with 4 bit greyscale
#include <string.h>
#include <stdio.h>
#include "frame_buffer.h"
// #include <math.h>

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
		*p |= shade; // (*p & 0xF0) | shade;  // set lower nibble
	else
		*p |= (shade << 4); // (*p & 0x0F) | (shade << 4);  // set upper nibble
}

// Set a 4 bit pixel in framebuffer
void hardSetPixel(unsigned x, unsigned y, uint8_t shade)
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
// Invert all pixels in rectangle
void invertRect(int x0, int x1, int y0, int y1) {
    // clamp coordinates
    if (x0 < 0) x0 = 0;
    if (y0 < 0) y0 = 0;
    if (x1 >= DISPLAY_WIDTH)  x1 = DISPLAY_WIDTH - 1;
    if (y1 >= DISPLAY_HEIGHT) y1 = DISPLAY_HEIGHT - 1;
    for (int y = y0; y <= y1; y++) {
        for (int x = x0; x <= x1; x++) {
            uint8_t shade = getPixel(x, y) & 0x0F;
            hardSetPixel(x, y, (uint8_t)(0x0F - shade));
        }
    }
}

// Invert all pixels in rectangle with rounded corners, with AA shading
void invertRoundedRect(int x1, int y1, int x2, int y2, int radius) {
    // ensure proper ordering
    if (x1 > x2) { int t = x1; x1 = x2; x2 = t; }
    if (y1 > y2) { int t = y1; y1 = y2; y2 = t; }
    if (radius < 0) radius = 0;
    // clamp to display bounds
    int maxW = DISPLAY_WIDTH - 1;
    int maxH = DISPLAY_HEIGHT - 1;
    x1 = (x1 < 0 ? 0 : x1);
    y1 = (y1 < 0 ? 0 : y1);
    x2 = (x2 > maxW ? maxW : x2);
    y2 = (y2 > maxH ? maxH : y2);
    // precompute for AA
    float fr = radius + 1.0f;
    float fr2 = fr * fr;
    int cx1 = x1 + radius;
    int cx2 = x2 - radius;
    int cy1 = y1 + radius;
    int cy2 = y2 - radius;
    for (int yy = y1; yy <= y2; ++yy) {
        for (int xx = x1; xx <= x2; ++xx) {
            // distance components from corner area
            float dx = 0, dy = 0;
            if (xx < cx1) dx = cx1 - xx;
            else if (xx > cx2) dx = xx - cx2;
            if (yy < cy1) dy = cy1 - yy;
            else if (yy > cy2) dy = yy - cy2;
            float dist2 = dx*dx + dy*dy;
            if (dist2 <= fr2) {
                // float dist = sqrtf(dist2);
                float dist = radius + (dist2-radius*radius)/(fr2-radius*radius); // approximation without taking sqrt
                float cover;
                if (dx == 0.0f || dy == 0.0f) {
                    cover = 1.0f;
                } else {
                    cover = fr - dist; // approximation without taking sqrt
                }
                if (cover > 1.0f) cover = 1.0f;
                else if (cover < 0.0f) cover = 0.0f;
                uint8_t old = getPixel(xx, yy) & 0x0F;
                // invert shade with AA: new = old + cover*(15 - 2*old)
                int delta = (int)(cover * (15 - 2*old));
                uint8_t ns = (uint8_t)(old + delta);
                hardSetPixel(xx, yy, ns);
            }
        }
    }
}

// Draw empty (outline) rounded rectangle with specified thickness
void emptyRoundedRect(int x1, int y1, int x2, int y2, int radius, int thickness) {
    // ensure proper ordering
    if (x1 > x2) { int t = x1; x1 = x2; x2 = t; }
    if (y1 > y2) { int t = y1; y1 = y2; y2 = t; }
    if (radius < 0) radius = 0;
    if (thickness < 1) thickness = 1;
    // clamp to display bounds
    int maxW = DISPLAY_WIDTH - 1, maxH = DISPLAY_HEIGHT - 1;
    x1 = (x1 < 0 ? 0 : x1);
    y1 = (y1 < 0 ? 0 : y1);
    x2 = (x2 > maxW ? maxW : x2);
    y2 = (y2 > maxH ? maxH : y2);
    // draw top and bottom border
    for (int y = y1; y < y1 + thickness; ++y) {
        for (int x = x1 + radius; x <= x2 - radius; ++x)
            setPixel(x, y, 0x0F);
    }
    for (int y = y2 - thickness + 1; y <= y2; ++y) {
        for (int x = x1 + radius; x <= x2 - radius; ++x)
            setPixel(x, y, 0x0F);
    }
    // draw left and right border
    for (int x = x1; x < x1 + thickness; ++x) {
        for (int y = y1 + radius; y <= y2 - radius; ++y)
            setPixel(x, y, 0x0F);
    }
    for (int x = x2 - thickness + 1; x <= x2; ++x) {
        for (int y = y1 + radius; y <= y2 - radius; ++y)
            setPixel(x, y, 0x0F);
    }
    // draw rounded corners with 1px antialiasing
    {
        int r = radius;
        int t = thickness;
        float rNext = r + 1.0f;
        float rNext2 = rNext * rNext;
        for (int dy = 0; dy <= rNext; ++dy) {
            for (int dx = 0; dx <= rNext; ++dx) {
                float dist2 = dx*dx + dy*dy;
                if (dist2 < rNext2) {
                    float dist = r + (dist2-r*r)/(rNext2-r*r); // approximation without taking sqrt
                    float cover;
                    cover = rNext - dist;
                    if (cover > 1.0f*t+1) cover = 0.0f;
                    if (cover > 1.0f*t) cover = -cover + 1.0f*t + 1.0f;
                    if (cover > 1.0f) cover = 1.0f;
                    if (cover < 0.0f) cover = 0.0f;
                    if (cover > 0.0f) {
                        uint8_t shade = (uint8_t)(cover * 15.0f + 0.5f);
                        if (shade) {
                            setPixel(x1 + r - dx, y1 + r - dy, shade);
                            setPixel(x2 - r + dx, y1 + r - dy, shade);
                            setPixel(x1 + r - dx, y2 - r + dy, shade);
                            setPixel(x2 - r + dx, y2 - r + dy, shade);
                        }
                    }
                }
            }
        }
    }
}
