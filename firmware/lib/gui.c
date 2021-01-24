#include <stdarg.h>
// #include "printf.h"  // tiny printf
#include "print.h"
#include "lv_font.h"
#include "ssd1322.h"
#include "gui.h"
#include "frame_buffer.h"

void lv_print(const char *str)
{
	reset_bb();
	while (*str)
		draw_char(*str++);
}

// Initialize a `label`, which has a fixed bounding box and alignment
// x, y = position of anchor point
// fnt = the font header file to use
// init = a string to determine width of the bounding box
// a = text alignment __AND__ anchor point!
// draw = if true, also draw the init string to the framebuffer
// use lv_update_label to change the label content
void lv_init_label(t_label *lbl, int x, int y, lv_font_t *fnt, const char *init, t_align a, bool draw)
{
	int w=0, h=0;
	lbl->x = x;
	lbl->y = y;
	lbl->y0 = y;
	lbl->fnt = fnt;
	lbl->align = a;
	set_font(fnt);
	get_bb(init, &w, &h);
	if (a == LV_LEFT) {
		// x, y anchor on the left and aligned left
		lbl->x0 = x;
		lbl->x1 = x + w;
	} else if (a == LV_CENTER) {
		// x, y anchor centered and aligned centered
		lbl->x0 = x - w / 2;
		lbl->x1 = x + w / 2;
	} else if (a == LV_RIGHT) {
		// x, y anchor on the right and aligned right
		lbl->x0 = x - w;
		lbl->x1 = x;
	} else if (a == LV_RIGHT_REF_LEFT) {
		// x, y anchor on the left but aligned right
		lbl->x0 = x;
		lbl->x1 = x + w;
		lbl->x += w;
		lbl->align = LV_RIGHT;
	}
	lbl->y1 = y + h;
	if (draw)
		lv_update_label(lbl, init);
}

// call it like printf (no space for printf :p)
// void lv_update_label(t_label *lbl, const char *format, ...)
// {
// 	int w=0, h=0;
// 	char buf[32], *p=buf;

// 	va_list argptr;
// 	va_start(argptr, format);
// 	vsnprintf(buf, sizeof(buf), format, argptr);
// 	va_end(argptr);

// 	fillRect(lbl->x0, lbl->x1, lbl->y0, lbl->y1, 0);
// 	// rect(lbl->x0, lbl->x1, lbl->y0, lbl->y1, 7);  // show bb

// 	set_font(lbl->fnt);
// 	if (lbl->align == LV_LEFT) {
// 		set_cursor(lbl->x, lbl->y);
// 	} else if (lbl->align == LV_CENTER) {
// 		get_bb(buf, &w, &h);
// 		set_cursor(lbl->x - w / 2, lbl->y);
// 	} else if (lbl->align == LV_RIGHT) {
// 		get_bb(buf, &w, &h);
// 		set_cursor(lbl->x - w, lbl->y);
// 	}
// 	set_bb(lbl->x0, lbl->x1, lbl->y0, lbl->y1);
// 	while (*p)
// 		draw_char(*p++);
// }

void lv_update_label(t_label *lbl, const char *buf)
{
	int w=0, h=0;
	fillRect(lbl->x0, lbl->x1, lbl->y0, lbl->y1, 0);
	// rect(lbl->x0, lbl->x1, lbl->y0, lbl->y1, 7);  // show bb

	set_font(lbl->fnt);
	if (lbl->align == LV_LEFT) {
		set_cursor(lbl->x, lbl->y);
	} else if (lbl->align == LV_CENTER) {
		get_bb(buf, &w, &h);
		set_cursor(lbl->x - w / 2, lbl->y);
	} else if (lbl->align == LV_RIGHT) {
		get_bb(buf, &w, &h);
		set_cursor(lbl->x - w, lbl->y);
	}
	set_bb(lbl->x0, lbl->x1, lbl->y0, lbl->y1);
	while (*buf)
		draw_char(*buf++);
}

void lv_update_label_dp(t_label *lbl, int32_t val, const uint8_t n, const uint8_t dp)
{
	char buf[16];
	dec_dp(val, n, dp, buf);
	lv_update_label(lbl, buf);
}


void lv_update_label_fix(t_label *lbl, int32_t val, const uint8_t nFract, uint8_t nDigits)
{
	char buf[16];
	dec_fix(val, nFract, nDigits, buf);
	lv_update_label(lbl, buf);
}
