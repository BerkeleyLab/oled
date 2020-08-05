#include <stdlib.h>
#include "frame_buffer.h"
#include "aa_line.h"
#include "lv_font.h"
#include "sin1.h"
#include "demo.h"

static int RAND_AB(int a, int b)
{
	return (rand() % (b + 1 - a) + a);
}

/**
 * C++ version 0.4 char* style "itoa":
 * Written by Lukás Chmela
 * Released under GPLv3.
 */
static char* itoa(int value, char* result, int base) {
    // check that the base if valid
    if (base < 2 || base > 36) { *result = '\0'; return result; }

    char* ptr = result, *ptr1 = result, tmp_char;
    int tmp_value;

    do {
        tmp_value = value;
        value /= base;
        *ptr++ = "zyxwvutsrqponmlkjihgfedcba9876543210123456789abcdefghijklmnopqrstuvwxyz" [35 + (tmp_value - value * base)];
    } while ( value );

    // Apply negative sign
    if (tmp_value < 0) *ptr++ = '-';
    *ptr-- = '\0';
    while(ptr1 < ptr) {
        tmp_char = *ptr;
        *ptr--= *ptr1;
        *ptr1++ = tmp_char;
    }
    return result;
}

void drawLasers()
{
	static unsigned frm = 0;
	static int16_t alpha=0;
	static unsigned n_lines=8, x=64, y=16;
	int dx = 0, dy = 0;

	fill(0);

	if (frm % 400 == 0) {
		n_lines = RAND_AB(3, 16);  // number of lines
		x = RAND_AB(4, DISPLAY_WIDTH - 5);  // center point
		y = RAND_AB(1, DISPLAY_HEIGHT - 2);

	}

	for (unsigned i=0; i<n_lines; i++) {
		dx = cos1(alpha + 32767 * i / n_lines) * DISPLAY_WIDTH / 32768;
		dy = sin1(alpha + 32767 * i / n_lines) * DISPLAY_WIDTH / 32768;
		drawLine(x, y, x + dx, y + dy);
	}

	char chars[32] = {0xef, 0x81, 0xae, 0x20};

	int bla = cos1(alpha * 10) * DISPLAY_WIDTH / 32768 / 4 + DISPLAY_WIDTH / 8;
	int blu = cos1(alpha * 16) * DISPLAY_HEIGHT / 32768 / 4;

	draw_str(LV_SYMBOL_WARNING " Hallo Welt! " LV_SYMBOL_OK "\nThe qwjck brown Fox", bla, blu);
	itoa(frm, chars + 4, 10);
	draw_str(chars, bla, 45);

	rect(5, 5, 33, 48, 1);

	alpha += 10;
	frm++;
}
