#include <stdlib.h>
#include "frame_buffer.h"
#include "aa_line.h"
#include "sin1.h"
#include "demo.h"

static int RAND_AB(int a, int b)
{
	return (rand() % (b + 1 - a) + a);
}

void drawLasers(unsigned frm)
{
	static int16_t alpha=0;
	static unsigned n_lines=8, x=64, y=16;
	int dx, dy;

	if (frm % 200 == 0) {
		n_lines = RAND_AB(3, 32);  // number of lines
		x = RAND_AB(4, DISPLAY_WIDTH - 5);  // center point
		y = RAND_AB(1, DISPLAY_HEIGHT - 2);
	}

	for (unsigned i=0; i<n_lines; i++) {
		dx = cos1(alpha + 32767 * i / n_lines) * DISPLAY_WIDTH / 32768;
		dy = sin1(alpha + 32767 * i / n_lines) * DISPLAY_WIDTH / 32768;
		drawLine(x, y, x + dx, y + dy);
	}

	alpha += 10;
}

void draw_test_frame()
{
	static unsigned frm = 0;
	setAll(0);
	// for (unsigned x=0; x<=256; x++)
 //        setPixel(x, x / 8, 0x08);
	// drawLine(0, 0, 255, 8);
	// drawLine(0, 0, 8, 63);
	drawLasers(frm);
	frm++;
}
