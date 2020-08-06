#include <stdlib.h>
#include "frame_buffer.h"
#include "aa_line.h"
#include "lv_font.h"
#include "sin1.h"
#include "print.h"
#include "demo.h"

extern lv_font_t lv_font_montserrat_18;
extern lv_font_t lv_font_montserrat_12;

void _putchar(char c)
{
    // hook for all print_* functions
    draw_char(c);
}

static int RAND_AB(int a, int b)
{
	return (rand() % (b + 1 - a) + a);
}

void drawLasers()
{
	static unsigned frm = 0;
	static int16_t alpha=0;
	static unsigned n_lines=8, x=64, y=16;
	int dx = 0, dy = 0;
	char tst[] = LV_SYMBOL_BATTERY_FULL;

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

	int bla = cos1(alpha * 10) * DISPLAY_WIDTH / 32768 / 4 + DISPLAY_WIDTH / 6;
	int blu = cos1(alpha * 16) * DISPLAY_HEIGHT / 32768 / 4 + DISPLAY_HEIGHT / 8;

	set_cursor(bla, blu);
	set_font(&lv_font_montserrat_12);
	print_str(LV_SYMBOL_WARNING " Hallo Welt! " LV_SYMBOL_OK "\nZwölf Boxkämpfer");

	if (bla < 0) bla = 0;
	if (blu < 0) blu = 0;

	set_cursor(bla, 45);
	set_font(&lv_font_montserrat_18);

	if ((frm >> 7) & 1)
		print_str(LV_SYMBOL_EYE_CLOSE);
	else
		print_str(LV_SYMBOL_EYE_OPEN);
	print_dec(frm);

	tst[2] = 0x80 + (frm >> 6) % 5;
	int w, h;
	get_bb(tst, &w, &h);
	set_cursor(DISPLAY_WIDTH - w, blu);
	print_str(tst);

	alpha += 10;
	frm++;
}
