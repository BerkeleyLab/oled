#include <stdlib.h>
#include "frame_buffer.h"
#include "aa_line.h"
#include "lv_font.h"
#include "fast_sin.h"
#include "print.h"
#include "lv_symbols.h"
#include "ui_board.h"
#include "demo.h"
#include <stdio.h>

extern lv_font_t lv_font_roboto_12, lv_font_roboto_mono_17, lv_font_fa;

typedef enum {
  MENU=0,
  STATUS,
  POWER,
#if ENABLE_FMC_CURRENT_CHECK
  PAGE_FMC,
#endif
  TEMPERATURE,
  ERRORS,
  CONSOLE_WARNING,
  CONSOLE,
// console pages:
  SET_IP,
  SET_MAC,
  NUMBER_OF_PAGES, // Keep me at the end
} display_page_t;


static const char *MenuItems[NUMBER_OF_PAGES] = { // triggers compile warning if not all enum values are covered
    "Main Menu",
	"Status",
	"Power",
	"Temperature",
	"Error logs",
	"Configuration", // new menu page
	"Set IP Address",
	"Set MAC Address"
};


static display_page_t menu(unsigned btns);
static display_page_t config_warning(unsigned btns);
static display_page_t console(unsigned btns);
static display_page_t set_IP(unsigned btns);
static display_page_t demo2(unsigned btns);

static void scrollbar(int16_t percentage);
static void errorLight(unsigned frm);



void _putchar(char c)
{
	// hook for all print_* functions
	draw_char(c);
}

static int RAND_AB(int a, int b)
{
	return (rand() % (b + 1 - a) + a);
}



void demo(unsigned btns) // window manager
{
	static display_page_t current_page = MENU;

	switch(current_page) {
		case MENU:
		 	current_page = menu(btns);
			break;
		case STATUS:
			current_page = demo2(btns);
			break;
		case CONSOLE_WARNING:
			current_page = config_warning(btns);
			break;
		case CONSOLE:
			current_page = console(btns);
			break;
		case SET_IP:
			current_page = set_IP(btns);
			break;
		default:
			current_page = MENU;
			break;
	}

}

static display_page_t menu(unsigned btns)
{
	static unsigned frm = 0;
	static int16_t cursor_y_goal = 23;
	static int16_t text_cursor = 20;
	static int16_t selection_id = 0;
	static uint8_t blink = 0;

	fill(0);

	if (btns & (1 << 0)) {  // left
		selection_id--;
	} else if (btns & (1 << 1)) {  // right
		selection_id++;
	}
	
	cursor_y_goal = 43-(20*selection_id);
	if(text_cursor == (text_cursor+cursor_y_goal)/2) {
		text_cursor = cursor_y_goal;
		if(selection_id >= CONSOLE_WARNING+1) // bounce back at the end
			selection_id = CONSOLE_WARNING;
		if(selection_id <= 0) // bounce back at the end
			selection_id = STATUS;
		cursor_y_goal = 43-(20*selection_id);
	}
	else {
		text_cursor=(text_cursor+cursor_y_goal)/2;
	}

	for(display_page_t p=STATUS; p<=CONSOLE_WARNING; p++) {
		set_cursor(16, text_cursor + 20*(p-1));
		set_font(&lv_font_roboto_mono_17);
		print_str(MenuItems[p]);
	}

	if(text_cursor == cursor_y_goal && selection_id >=STATUS && selection_id <= CONSOLE_WARNING) {
		if (btns & (1 << 2))  // push
			blink++;
		if (blink > 0) {
			if((blink>>1)%2)
				invertRoundedRect(6, 22, 250, 42, 10);
			else {
				emptyRoundedRect(6, 22, 250, 42, 10, 1);
			}
			blink++;
			if (blink > 6) {// selection made
				blink = 0;
				return selection_id;
			}
		} else {
			invertRoundedRect(6, 22, 250, 42, 10);
		}
	}
	else {
			emptyRoundedRect(6, 22, 250, 42, 10, 1);
	}
	
	errorLight(frm);
	frm++;
	return MENU; // stay in menu
}


static display_page_t config_warning(unsigned btns)
{
	static unsigned frm = 0;
	static int16_t selection_id = 0;

	const int rectangle_coordinates[][4] = {
		{DISPLAY_WIDTH/2 - 98, 44, DISPLAY_WIDTH/2 - 23, 64},  // CANCEL
		{DISPLAY_WIDTH/2 + 22, 44, DISPLAY_WIDTH/2 + 107, 64}  // Proceed
	};

	fill(0);

	set_cursor(DISPLAY_WIDTH/2 - 35, 2);
	set_font(&lv_font_roboto_mono_17);
	print_str("WARNING");
	set_cursor(1, 21);
	set_font(&lv_font_roboto_12);
	print_str("You are about to change critical device settings");
	set_font(&lv_font_roboto_mono_17);
	set_cursor(DISPLAY_WIDTH/2 - 90, 44);
	print_str("Cancel");
	set_cursor(DISPLAY_WIDTH/2 + 30, 44);
	print_str("Proceed");
		// navigate menu
		if (btns & (1 << 0)) {  // left
			selection_id--;
		} else if (btns & (1 << 1)) {  // right
			selection_id++;
		}
		if(selection_id >= 2) // bounce back at the end
			selection_id = 0;
		if(selection_id <= -1) // bounce back at the end
			selection_id = 1;
	

	if(selection_id == 0) {
		invertRoundedRect(rectangle_coordinates[0][0]-1, rectangle_coordinates[0][1]-1, rectangle_coordinates[0][2]+1, rectangle_coordinates[0][3]+1, 10);
		emptyRoundedRect(rectangle_coordinates[1][0]-1, rectangle_coordinates[1][1]-1, rectangle_coordinates[1][2]+1, rectangle_coordinates[1][3]+1, 10, 1);
		if (btns & (1 << 2)) { // push
			selection_id = 0; // default cancel button
			return MENU; // back to console menu
		}
	} else if (selection_id == 1) {
		emptyRoundedRect(rectangle_coordinates[0][0]-1, rectangle_coordinates[0][1]-1, rectangle_coordinates[0][2]+1, rectangle_coordinates[0][3]+1, 10, 1);
		invertRoundedRect(rectangle_coordinates[1][0]-1, rectangle_coordinates[1][1]-1, rectangle_coordinates[1][2]+1, rectangle_coordinates[1][3]+1, 10);
		if (btns & (1 << 2)) { // push
			selection_id = 0; // default cancel button
			return CONSOLE; // back to console menu
		}
	}

	frm++;
	return CONSOLE_WARNING; // stay in menu
}


static display_page_t console(unsigned btns)
{
	static unsigned frm = 0;
	static unsigned n_lines=4, x=128, y=32;
	static uint8_t led = 0;
	static unsigned isBtn;
	int dx = 0, dy = 0;
	static int16_t cursor_y_goal = 23;
	static int16_t text_cursor = 20;
	static int16_t selection_id = 0;
	static uint8_t blink = 0;

	fill(0);

	if (btns & (1 << 0)) {  // left
		selection_id--;
	} else if (btns & (1 << 1)) {  // right
		selection_id++;
	}
	
	cursor_y_goal = 23-(20*selection_id);

	if(text_cursor == (text_cursor+cursor_y_goal)/2) {
		text_cursor = cursor_y_goal;
		if(selection_id >= 6) // bounce back at the end
			selection_id = 5;
		if(selection_id <= -1) // bounce back at the end
			selection_id = 0;
		cursor_y_goal = 23-(20*selection_id);
	}
	else {
		text_cursor=(text_cursor+cursor_y_goal)/2;
	}
	// if(text_cursor < cursor_y_goal) {
	// 	text_cursor=(text_cursor+cursor_y_goal)/2;
	// } else if (text_cursor > cursor_y_goal) {
	// 	text_cursor--;
	// }

	set_cursor(21, text_cursor);
	set_font(&lv_font_roboto_mono_17);
	print_str("Main Menu");

	set_cursor(21, text_cursor + 20);
	set_font(&lv_font_roboto_mono_17);
	print_str("Set IP Address");

	set_cursor(21, text_cursor + 40);
	set_font(&lv_font_roboto_mono_17);
	print_str("Set MAC Address");

	set_cursor(21, text_cursor + 60);
	set_font(&lv_font_roboto_mono_17);
	print_str("Other stuff");

	set_cursor(21, text_cursor + 80);
	set_font(&lv_font_roboto_mono_17);
	print_str("More other stuff");

	set_cursor(21, text_cursor + 100);
	set_font(&lv_font_roboto_mono_17);
	print_str("...");

	// set_cursor(220, 4);
	// print_dec(cursor_y_goal);
	// set_cursor(220, 49);
	// print_dec(text_cursor);

	// if(text_cursor == cursor_y_goal && selection_id >=0 && selection_id <= 5){
		// set_font(&lv_font_roboto_mono_17);
		// set_cursor(224, 25);
		// print_str(CHEVRON_DOWN);
	if(text_cursor == cursor_y_goal && selection_id >=0 && selection_id <= 5) {
		if (btns & (1 << 2))  // push
			blink++;
		if (blink > 0) {
			if((blink>>1)%2)
				invertRoundedRect(6, 22, 250, 42, 10);
			else {
				emptyRoundedRect(6, 22, 250, 42, 10, 1);
			}
			blink++;
			if (blink > 6) {// selection made
				blink = 0;
				if(selection_id == 0)
					return MENU;
				else
					return selection_id + CONSOLE; // page id start at 1
			}
		} else {
			invertRoundedRect(11, 22, 254, 42, 10);
		}
	}
	else {
			emptyRoundedRect(11, 22, 254, 42, 10, 1);
	}

	scrollbar((-(text_cursor - 23)*10)/10); // percentage
	errorLight(frm);
	frm++;
	return CONSOLE; // stay in menu
}


static display_page_t set_IP(unsigned btns)
{
	static unsigned frm = 0;
	static unsigned isBtn;
	int dx = 0, dy = 0;
	static int16_t cursor_x1 = 0;
	static int16_t cursor_x2 = 0;
	static int16_t cursor_y1 = 0;
	static int16_t cursor_y2 = 0;
	static int16_t selection_id = 0;
	static uint8_t ip_bytes[4] = {192, 168, 1, 100};
	static bool ip_selected = 0;
	static bool ip_success = 0;
	const int rectangle_coordinates[][4] = {
		{DISPLAY_WIDTH/2 - 98, 44, DISPLAY_WIDTH/2 - 22, 64},  // CANCEL
		{88, 20, 121, 39}, // IP 1
		{128, 20, 161, 39}, // IP 2
		{168, 20, 201, 39}, // IP 3
		{208, 20, 241, 39},  // IP 4
		{DISPLAY_WIDTH/2 + 22, 44, DISPLAY_WIDTH/2 + 98, 64}  // SET IP
	};

	fill(0);

	set_cursor(10, 21);
	set_font(&lv_font_roboto_mono_17);
	char buffer[100];
	snprintf(buffer, sizeof(buffer), "Set IP: %03d.%03d.%03d.%03d", ip_bytes[0], ip_bytes[1], ip_bytes[2], ip_bytes[3]);
	print_str(buffer);
	set_cursor(DISPLAY_WIDTH/2 - 80, 44);
	print_str("Exit");
	set_cursor(DISPLAY_WIDTH/2 + 30, 44);
	print_str("Set IP");
	if(ip_success) {
		set_font(&lv_font_roboto_12);
		set_cursor(75, 2);
		print_str("IP set successfully!");
		invertRoundedRect(65, 2, 190, 16, 7);
		if (btns & 0x03) {  // left or right
			ip_success = 0; // reset success message
		}
	}

	if(ip_selected) {
		// modify selected byte
		if (btns & (1 << 0)) {  // left
			if(ip_bytes[selection_id - 1] == 0)
				ip_bytes[selection_id - 1] = 255;
			else
				ip_bytes[selection_id - 1]--;
		} else if (btns & (1 << 1)) {  // right
			if(ip_bytes[selection_id - 1] == 255)
				ip_bytes[selection_id - 1] = 0;
			else
				ip_bytes[selection_id - 1]++;
		}
	} else {
		// navigate menu
		if (btns & (1 << 0)) {  // left
			selection_id--;
		} else if (btns & (1 << 1)) {  // right
			selection_id++;
		}
		if(selection_id >= 6) // bounce back at the end
			selection_id = 0;
		if(selection_id <= -1) // bounce back at the end
			selection_id = 5;
	}

	if(selection_id == 0) {
		cursor_x1 = rectangle_coordinates[1][0];
		cursor_y1 = rectangle_coordinates[1][1];
		cursor_x2 = rectangle_coordinates[1][2];
		cursor_y2 = rectangle_coordinates[1][3];
		invertRoundedRect(rectangle_coordinates[0][0]-1, rectangle_coordinates[0][1]-1, rectangle_coordinates[0][2]+1, rectangle_coordinates[0][3]+1, 10);
		emptyRoundedRect(rectangle_coordinates[5][0]-1, rectangle_coordinates[5][1]-1, rectangle_coordinates[5][2]+1, rectangle_coordinates[5][3]+1, 10, 1);
		if (btns & (1 << 2)) { // push
			ip_success = 0;
			return CONSOLE; // back to console menu
		}
	} else if (selection_id == 5) {
		cursor_x1 = rectangle_coordinates[4][0];
		cursor_y1 = rectangle_coordinates[4][1];
		cursor_x2 = rectangle_coordinates[4][2];
		cursor_y2 = rectangle_coordinates[4][3];
		emptyRoundedRect(rectangle_coordinates[0][0]-1, rectangle_coordinates[0][1]-1, rectangle_coordinates[0][2]+1, rectangle_coordinates[0][3]+1, 10, 1);
		invertRoundedRect(rectangle_coordinates[5][0]-1, rectangle_coordinates[5][1]-1, rectangle_coordinates[5][2]+1, rectangle_coordinates[5][3]+1, 10);
		if (btns & (1 << 2)) { // push
			ip_success = 1;
		}
	} else { // IP byte selected
		cursor_x1 = ((rectangle_coordinates[selection_id][0]+cursor_x1)>>1 == cursor_x1) ? rectangle_coordinates[selection_id][0] : (rectangle_coordinates[selection_id][0]+cursor_x1)>>1;
		cursor_y1 = ((rectangle_coordinates[selection_id][1]+cursor_y1)>>1 == cursor_y1) ? rectangle_coordinates[selection_id][1] : (rectangle_coordinates[selection_id][1]+cursor_y1)>>1;
		cursor_x2 = ((rectangle_coordinates[selection_id][2]+cursor_x2)>>1 == cursor_x2) ? rectangle_coordinates[selection_id][2] : (rectangle_coordinates[selection_id][2]+cursor_x2)>>1;
		cursor_y2 = ((rectangle_coordinates[selection_id][3]+cursor_y2)>>1 == cursor_y2) ? rectangle_coordinates[selection_id][3] : (rectangle_coordinates[selection_id][3]+cursor_y2)>>1;
		if(ip_selected) {
			invertRoundedRect(cursor_x1, cursor_y1, cursor_x2, cursor_y2, 5);
		} else {
			emptyRoundedRect(cursor_x1, cursor_y1, cursor_x2, cursor_y2, 5, 1);
		}
		// print cancel and set ip buttons
		emptyRoundedRect(rectangle_coordinates[0][0]-1, rectangle_coordinates[0][1]-1, rectangle_coordinates[0][2]+1, rectangle_coordinates[0][3]+1, 10, 1);
		emptyRoundedRect(rectangle_coordinates[5][0]-1, rectangle_coordinates[5][1]-1, rectangle_coordinates[5][2]+1, rectangle_coordinates[5][3]+1, 10, 1);
		if (btns & (1 << 2)) { // push
			ip_selected = !ip_selected; // select ip byte
		}
	}

	frm++;
	return SET_IP; // stay in menu
}









static display_page_t demo2(unsigned btns)
{
	static unsigned frm = 0;
	static int alpha = 0;
	static unsigned n_lines=4, x=128, y=32;
	static uint8_t led = 0;
	static unsigned isBtn;
	int dx = 0, dy = 0;
	static int16_t cursor_y_goal = 20;
	static int16_t text_cursor = 20;

	fill(0);

	// Radial lines
	if (frm % 400 == 0) {
		n_lines = RAND_AB(2, 9);  // number of lines
		x = RAND_AB(4, DISPLAY_WIDTH - 5);  // center point
		y = RAND_AB(1, DISPLAY_HEIGHT - 2);
	}
	for (unsigned i=0; i<n_lines; i++) {
		dx = get_cos(alpha + 32767 * i / n_lines) * DISPLAY_WIDTH / 32768;
		dy = get_sin(alpha + 32767 * i / n_lines) * DISPLAY_WIDTH / 32768;
		drawLine(x, y, x + dx, y + dy);
	}

	isBtn |= btns & 3;
	if (isBtn == 0)
		alpha += 30;

	// Show some text + custom symbols
	set_cursor(1, 4);
	set_font(&lv_font_roboto_mono_17);
	print_str(RESISTOR " Hallo Welt ");
	if (led & 3) {
		print_str(SWITCH_CLOSED "\n");
	}
	else
		print_str(SWITCH_OPEN "\n");

	// flash something on encoder events
	set_font(&lv_font_roboto_mono_17);
	set_cursor(180, 40);
	if (btns & (1 << 0)) {  // left
		print_str(CHEVRON_DOWN);
		cursor_y_goal = cursor_y_goal+20;
		alpha -= 1024;
	} else if (btns & (1 << 1)) {  // right
		print_str(CHEVRON_UP);
		cursor_y_goal = cursor_y_goal-20;
		alpha += 1024;
	} else if (btns & (1 << 2)) {  // push
		print_str(CHECK_CIRCLE);
		led += 1;
		setLed(led);
		return MENU;
	}
	text_cursor=(text_cursor+cursor_y_goal)/2;
	if(text_cursor < cursor_y_goal) {
		text_cursor=(text_cursor+cursor_y_goal)/2;
	} else if (text_cursor > cursor_y_goal) {
		text_cursor--;
	}
	set_cursor(20, text_cursor);
	set_font(&lv_font_roboto_12);
	print_str("Franz jagt im komplett verwahr\n");

	// frame counter
	set_cursor(190, 4);
	print_dec(frm);

	// cycle all included large symbols
	const char *symbols[] = {INFO, THERMOMETER_FULL, BOLT, PLUG, MICROCHIP, BROADCAST_TOWER};//, UNLOCK_ALT};
	set_cursor(210, 25);
	set_font(&lv_font_fa);
	print_str(symbols[(frm >> 6) % 6]);

	set_font(&lv_font_roboto_mono_17);
	set_cursor(220, 25);
	print_str(CHEVRON_DOWN);


	errorLight(frm);
	frm++;
	return STATUS;
}


static void scrollbar(int16_t percentage) {
	invertRoundedRect(2, 2 + (percentage*47)/100, 6, 14 + (percentage*47)/100, 2); // FIXME: take percentage
	emptyRoundedRect(1, 1, 7, 62, 3, 1);
}


static void errorLight(unsigned frm) {
	set_cursor(200, 1);
	set_font(&lv_font_roboto_mono_17);
	print_str("ERROR");
	if((frm>>2)%2)
		invertRoundedRect(195, 0, 254, 18, 9);
}