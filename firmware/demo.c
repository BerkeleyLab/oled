#include <stdlib.h>
#include "frame_buffer.h"
#include "aa_line.h"
#include "lv_font.h"
#include "sin1.h"
#include "print.h"
#include "demo.h"
#include "gui.h"
#include "analog_registers.h"

extern lv_font_t lv_font_roboto_mono_17;
extern lv_font_t lv_font_fa;

uint16_t reg_map[64] = {
    0x0000, // PSU_CYCLES
    0x9770, // PSU_TEMP (25 degC)
    0x0000, // PSU_VCCINT (0 V)
    0x5550, // PSU_VCCAUX  (1 V)
    0x9980, // PSU_VCCBRAM  (1.8 V)
     12546, // PSU_IN_VOLTAGE
      1234, // PSU_IN_CURRENT
      5535, // PSU_A_VOLTAGE
       385, // PSU_A_CURRENT
      5522, // PSU_B_VOLTAGE
       119, // PSU_B_CURRENT
      5627, // PSU_C_VOLTAGE
       110, // PSU_C_CURRENT
      5124, // PSU_D_VOLTAGE
        50, // PSU_D_CURRENT
      5928, // PSU_E_VOLTAGE
       198, // PSU_E_CURRENT
    0x7D00, // DC_A_TEMP (125 degC)
       100, // DC_A_VOLTAGE (1 V)
    0xE6F0, // DC_B_TEMP (-25 degC)
     0x1F4, // DC_B_VOLTAGE (5 V)
};

void _putchar(char c) {}

// <a: static label><b: dynamic number><c: static unit>,
// x, y refers to the anchor on the left
// use `lv_update_label(nmb, ...)` to update the dynamic number
void lv_triple(t_label *nmb, int x, int y, const char *a, const char *b, const char *c)
{
    t_label tmp;
    lv_init_label(&tmp, x, y, &lv_font_roboto_mono_17, a, LV_LEFT);
    lv_init_label(nmb, tmp.x1 + 4, y, &lv_font_roboto_mono_17, b, LV_RIGHT_REF_LEFT);
    lv_init_label(&tmp, nmb->x1 + 4, y, &lv_font_roboto_mono_17, c, LV_LEFT);
}

// These are global to minimize memory usage
static t_label l0, l1, l2, l3, l4, l5;

// 6 numbers with units screen. type 0: PSU voltages, type 1: PSU currents
static void scr_psu(bool isInit, int type, unsigned btns)
{
    if (isInit) {
        // PSU board voltages / currents
        const char *unit = type ? "A" : "V";
        lv_init_label(
            &l0, 10, 15, &lv_font_fa,
            type ? LV_SYMBOL_PLUG : LV_SYMBOL_BOLT,
            LV_LEFT
        );
        lv_triple(&l0,  48, 1,          "IN", "000000", unit);
        lv_triple(&l1,  48, l0.y1 + 2,  " A", "000000", unit);
        lv_triple(&l2,  48, l1.y1 + 2,  " B", "000000", unit);
        lv_triple(&l3, 165, 1,          "C",  "000000", unit);
        lv_triple(&l4, 165, l3.y1 + 2,  "D",  "000000", unit);
        lv_triple(&l5, 165, l4.y1 + 2,  "E",  "000000", unit);
    }
    switch (type) {
        case 0:
            lv_update_label_dp(&l0, reg_map[PSU_IN_VOLTAGE], 5, 3);
            lv_update_label_dp(&l1, reg_map[PSU_A_VOLTAGE],  5, 3);
            lv_update_label_dp(&l2, reg_map[PSU_B_VOLTAGE],  5, 3);
            lv_update_label_dp(&l3, reg_map[PSU_C_VOLTAGE],  5, 3);
            lv_update_label_dp(&l4, reg_map[PSU_D_VOLTAGE],  5, 3);
            lv_update_label_dp(&l5, reg_map[PSU_E_VOLTAGE],  5, 3);
            break;

        case 1:
            lv_update_label_dp(&l0, reg_map[PSU_IN_CURRENT], 5, 3);
            lv_update_label_dp(&l1, reg_map[PSU_A_CURRENT],  5, 3);
            lv_update_label_dp(&l2, reg_map[PSU_B_CURRENT],  5, 3);
            lv_update_label_dp(&l3, reg_map[PSU_C_CURRENT],  5, 3);
            lv_update_label_dp(&l4, reg_map[PSU_D_CURRENT],  5, 3);
            lv_update_label_dp(&l5, reg_map[PSU_E_CURRENT],  5, 3);
            break;
    }
}

// FPGA voltages screen
static void scr_fpga(bool isInit, int type, unsigned btns)
{
    if (isInit) {
        // PSU board voltages / currents
        lv_init_label(&l0, 10, 15, &lv_font_fa, LV_SYMBOL_MICROCHIP, LV_LEFT);
        lv_triple(&l2, 145, 1,         "VINT", "00000", "V");
        lv_triple(&l3, 145, l2.y1 + 2, "VAUX", "00000", "V");
        lv_triple(&l4, 145, l3.y1 + 2, "VBRM", "00000", "V");
        lv_triple(&l1,  50, 1,         "",     "000000","h");
        lv_triple(&l0,  50, l3.y1 + 2, "",     "00000", "°C");
    }
    lv_update_label_fix(&l0, reg_map[PSU_TEMP] * 504 - 0x1112a00, 16, 1);
    lv_update_label_dp( &l1, reg_map[PSU_UP] * 10 / 6, 5, 1);
    lv_update_label_fix(&l2, reg_map[PSU_VCCINT] * 3, 16, 3);
    lv_update_label_fix(&l3, reg_map[PSU_VCCAUX] * 3, 16, 3);
    lv_update_label_fix(&l4, reg_map[PSU_VCCBRAM] * 3, 16, 3);
}

// down converter temperature and voltage screen
static void scr_dc(bool isInit, int type, unsigned btns)
{
    if (isInit) {
        lv_init_label(&l0, 15, 15, &lv_font_fa, LV_SYMBOL_THERMOMETER_FULL, LV_LEFT);
        lv_triple(&l0, 45, 11,        "DCA","0000000", "°C");
        lv_triple(&l1, 45, l0.y1 + 2, "DCB","0000000", "°C");
        lv_triple(&l2, 180, l0.y,     "",   "00000",   "V");
        lv_triple(&l3, 180, l2.y1 + 2,"",   "00000",   "V");
    }
    lv_update_label_fix(&l0, reg_map[DC_A_TEMP] << 16, 24, 2);
    lv_update_label_fix(&l1, reg_map[DC_B_TEMP] << 16, 24, 2);
    lv_update_label_dp( &l2, reg_map[DC_A_VOLTAGE], 4, 2);
    lv_update_label_dp( &l3, reg_map[DC_B_VOLTAGE], 4, 2);
}

// up converter attenuator and RF switch screen
static void scr_uc(bool isInit, int type, unsigned btns)
{
    if (isInit) {
        lv_init_label(&l0, 5, 15, &lv_font_fa, LV_SYMBOL_BROADCAST_TOWER, LV_LEFT);
        lv_triple(&l0, 60,  11,        "UCA  SW",  LV_SYMBOL_SQUARE, "");
        lv_triple(&l1, 60,  l0.y1 + 2, "UCB  SW",  LV_SYMBOL_SQUARE, "");
        lv_triple(&l2, 165, l0.y,      "ATT", "00", "dB");
        lv_triple(&l3, 165, l2.y1 + 2, "ATT", "00", "dB");
    }
    lv_update_label(   &l0, (reg_map[INLK_A_FLAGS] & 0x02) ? LV_SYMBOL_CHECK_SQUARE : LV_SYMBOL_SQUARE);
    lv_update_label(   &l1, (reg_map[INLK_B_FLAGS] & 0x02) ? LV_SYMBOL_CHECK_SQUARE : LV_SYMBOL_SQUARE);
    lv_update_label_dp(&l2, reg_map[UP_A_ATT], 2, 0);
    lv_update_label_dp(&l3, reg_map[UP_B_ATT], 2, 0);
}

// Local oscillator status
static void scr_lo_inlk(bool isInit, int type, unsigned btns)
{
    if (isInit) {
        lv_init_label(&l0, 10, 15, &lv_font_fa, LV_SYMBOL_UNLOCK, LV_LEFT);

        lv_triple(&l0, 50, 1, "LOA", "00000", "dBm");
        lv_init_label(&l1, 190, l0.y, &lv_font_roboto_mono_17, LV_SYMBOL_CHECK_CIRCLE, LV_LEFT);
        lv_init_label(&l2, l1.x1 + 5, l0.y, &lv_font_roboto_mono_17, LV_SYMBOL_SQUARE, LV_LEFT);

        lv_triple(&l3, 50, l0.y1 + 2, "LOB", "00000", "dBm");
        lv_init_label(&l4, 190, l3.y, &lv_font_roboto_mono_17, LV_SYMBOL_CHECK_CIRCLE, LV_LEFT);
        lv_init_label(&l5, l4.x1 + 5, l3.y, &lv_font_roboto_mono_17, LV_SYMBOL_SQUARE, LV_LEFT);

        t_label tmp;
        lv_init_label(&tmp, 50, l3.y1 + 2, &lv_font_roboto_mono_17, "Push to reset latch", LV_LEFT);
    }
    lv_update_label_fix(&l0, reg_map[INLK_A_VAL] * 300, 16, 1);  // TODO [dBm] scaling
    lv_update_label(    &l1, (reg_map[INLK_A_FLAGS] & 0x01) ? LV_SYMBOL_CHECK_CIRCLE : LV_SYMBOL_CIRCLE);
    lv_update_label(    &l2, (reg_map[INLK_A_FLAGS] & 0x02) ? LV_SYMBOL_CHECK_SQUARE : LV_SYMBOL_SQUARE);

    lv_update_label_fix(&l3, reg_map[INLK_B_VAL] * 300, 16, 1);  // TODO [dBm] scaling
    lv_update_label(    &l4, (reg_map[INLK_B_FLAGS] & 0x01) ? LV_SYMBOL_CHECK_CIRCLE : LV_SYMBOL_CIRCLE);
    lv_update_label(    &l5, (reg_map[INLK_B_FLAGS] & 0x02) ? LV_SYMBOL_CHECK_SQUARE : LV_SYMBOL_SQUARE);
}

// show interlock thresholds and maxmin values
static void scr_inlk_maxmin(bool isInit, int type, unsigned btns)
{
    if (isInit) {
        lv_init_label(&l0, 10, 15, &lv_font_fa, LV_SYMBOL_UNLOCK, LV_LEFT);

        lv_triple(&l0, 65, 1, LV_SYMBOL_CHEVRON_UP, "00000", "dBm");
        lv_triple(&l1, 50, l0.y1 + 2, type ? "LOB" : "LOA", "00000", "dBm");
        unsigned y = l1.y1 + 2;
        lv_triple(&l2, 65, y, LV_SYMBOL_CHEVRON_DOWN, "00000", "dBm");

        lv_triple(&l3, 175, 1, "(", "00000", ")");
        lv_triple(&l3, 175, y, "(", "00000", ")");
    }
    // TODO update code
}

void drawLasers(unsigned btns)
{
    static unsigned frm=0, screen=0;
    static t_label lbl, tmp;

    if (screen > 0 && (btns & 1))
        screen--;

    if (screen < 7 && (btns & 2))
        screen++;

    bool isRedraw = frm == 0 || (btns & 3);
    if (isRedraw)
        fill(3);

    switch (screen) {
        case 0:
            scr_psu(isRedraw, 0, 0);
            break;

        case 1:
            scr_psu(isRedraw, 1, 0);
            break;

        case 2:
            scr_fpga(isRedraw, 0, 0);
            break;

        case 3:
            scr_dc(isRedraw, 0, 0);
            break;

        case 4:
            scr_uc(isRedraw, 0, 0);
            break;

        case 5:
            scr_lo_inlk(isRedraw, 0, 0);
            break;

        case 6:
            scr_inlk_maxmin(isRedraw, 0, 0);
            break;

        case 7:
            scr_inlk_maxmin(isRedraw, 1, 0);
            break;
    }

    for (unsigned i=0; i < sizeof(reg_map) / sizeof(reg_map[0]); i++)
        reg_map[i] += 1;

    frm++;
}
