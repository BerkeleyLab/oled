#include <stdlib.h>
#include "frame_buffer.h"
#include "aa_line.h"
#include "lv_font.h"
#include "sin1.h"
#include "print.h"
#include "lv_symbols.h"
#include "ui_board.h"
#include "demo.h"

extern lv_font_t lv_font_roboto_mono_17, lv_font_fa;

void _putchar(char c)
{
    // hook for all print_* functions
    draw_char(c);
}

static int RAND_AB(int a, int b)
{
	return (rand() % (b + 1 - a) + a);
}

void demo(unsigned btns)
{
	static unsigned frm = 0;
	static int16_t alpha=0;
	static unsigned n_lines=8, x=64, y=16;
	static uint8_t led = 0;
	int dx = 0, dy = 0;

	fill(0);

	// Radial lines
	if (frm % 400 == 0) {
		n_lines = RAND_AB(2, 9);  // number of lines
		x = RAND_AB(4, DISPLAY_WIDTH - 5);  // center point
		y = RAND_AB(1, DISPLAY_HEIGHT - 2);
	}
	for (unsigned i=0; i<n_lines; i++) {
		dx = cos1(alpha + 32767 * i / n_lines) * DISPLAY_WIDTH / 32768;
		dy = sin1(alpha + 32767 * i / n_lines) * DISPLAY_WIDTH / 32768;
		drawLine(x, y, x + dx, y + dy);
	}
	alpha += 30;

	// Show some text
	set_cursor(1, 4);
	set_font(&lv_font_roboto_mono_17);
	print_str(RESISTOR " Hallo Welt " SWITCH_OPEN "\nFranz jagt im komplett\nverwahrlosten Taxi ");

	// flash something on encoder events
	if (btns & (1 << 0))  // left
		print_str(CHEVRON_DOWN);
	else if (btns & (1 << 1))  // right
		print_str(CHEVRON_UP);
	else if (btns & (1 << 2)) {  // push
		print_str(CHECK_CIRCLE);
		led += 1;
		setLed(led);
	}

	// frame counter
	set_cursor(200, 4);
	print_dec(frm);

	// cycle all included large symbols
	const char *symbols[] = {THERMOMETER_FULL, BOLT, PLUG, MICROCHIP, BROADCAST_TOWER, UNLOCK_ALT};
	set_cursor(228, 25);
	set_font(&lv_font_fa);
	print_str(symbols[(frm >> 6) % 6]);

	frm++;
}
