#include <stdlib.h>
#include "frame_buffer.h"
#include "lv_font.h"
#include "print.h"
#include "gui.h"
#include "analog_registers.h"
#include "lv_symbols.h"

extern lv_font_t lv_font_roboto_mono_17;
extern lv_font_t lv_font_fa;
extern uint16_t reg_map[];

// These are global to minimize memory usage
static t_label l0, l1, l2, l3, l4, l5, l6;

// Initializes 3 labels:
// <a: static label><b: dynamic number><c: static unit>
// use `lv_update_label(nmb, ...)` to update the dynamic number in the middle
// x, y:    position of anchor point on the top left
static void lv_triple(t_label *nmb, int x, int y, const char *a, const char *b, const char *c)
{
    t_label tmp;
    lv_init_label(&tmp, x, y, &lv_font_roboto_mono_17, a, LV_LEFT, true);
    lv_init_label(nmb, tmp.x1 + 4, y, &lv_font_roboto_mono_17, b, LV_RIGHT_REF_LEFT, false);
    lv_init_label(&tmp, nmb->x1 + 4, y, &lv_font_roboto_mono_17, c, LV_LEFT, true);
}

// 6 numbers with units screen. type 0: PSU voltages, type 1: PSU currents
static void scr_psu(bool isInit, int type)
{
    if (isInit) {
        // PSU board voltages / currents
        const char *unit = type ? "A" : "V";
        lv_init_label(
            &l0, 10, 15, &lv_font_fa,
            type ? PLUG : BOLT,
            LV_LEFT,
            true
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
            lv_update_label_dp(&l0, (int16_t)reg_map[PSU_IN_CURRENT], 4, 3);
            lv_update_label_dp(&l1, (int16_t)reg_map[PSU_A_CURRENT],  4, 3);
            lv_update_label_dp(&l2, (int16_t)reg_map[PSU_B_CURRENT],  4, 3);
            lv_update_label_dp(&l3, (int16_t)reg_map[PSU_C_CURRENT],  4, 3);
            lv_update_label_dp(&l4, (int16_t)reg_map[PSU_D_CURRENT],  4, 3);
            lv_update_label_dp(&l5, (int16_t)reg_map[PSU_E_CURRENT],  4, 3);
            break;
    }
}

#define F_CLK 100000000
unsigned dModbus = F_CLK * 0.02;

// FPGA voltages screen
static void scr_fpga(bool isInit)
{
    if (isInit) {
        // PSU board voltages / currents
        lv_init_label(&l0, 8, 15, &lv_font_fa, MICROCHIP, LV_LEFT, true);

        lv_triple(&l0,  40, 9,         "",   "0000000","h");
        // lv_init_label(&l1, 40, l0.y1 + 2, &lv_font_roboto_mono_17, GIT_VERSION, LV_LEFT, true);
        // lv_triple(&l1,  50, l0.y1 + 2, "MB RTU", CHECK_CIRCLE, "");
        lv_triple(&l2,  40, l0.y1 + 9, "",    "000000", "°C");

        lv_triple(&l3, 145, 1,         "VINT", "00000", "V");
        lv_triple(&l4, 145, l3.y1 + 2, "VAUX", "00000", "V");
        lv_triple(&l5, 145, l4.y1 + 2, "VBRM", "00000", "V");
    }
    lv_update_label_dp( &l0, reg_map[PSU_UP] * 10 / 6, 6, 1);
    // Modbus timeout
    // unsigned tMb = dModbus / (F_CLK / 100);
    // lv_update_label(&l1, (tMb <= 1000) ? CHECK_CIRCLE : CIRCLE);
    lv_update_label_fix(&l2, (int16_t)reg_map[PSU_TEMP], 8, 1);
    lv_update_label_dp( &l3, reg_map[PSU_VCCINT], 4, 3);
    lv_update_label_dp( &l4, reg_map[PSU_VCCAUX], 4, 3);
    lv_update_label_dp( &l5, reg_map[PSU_VCCBRAM],4, 3);
}

// down converter temperature and voltage screen
static void scr_dc(bool isInit)
{
    if (isInit) {
        lv_init_label(&l0, 12, 15, &lv_font_fa, THERMOMETER_FULL, LV_LEFT, true);
        lv_triple(&l0, 45, 9,        "DCA","0000000", "°C");
        lv_triple(&l1, 45, l0.y1 + 9, "DCB","0000000", "°C");
        lv_triple(&l2, 175, l0.y,     "",   "000000",   "V");
        lv_triple(&l3, 175, l2.y1 + 9,"",   "000000",   "V");
    }
    lv_update_label_fix(&l0, (int16_t)reg_map[DC_A_TEMP], 8, 2);
    lv_update_label_fix(&l1, (int16_t)reg_map[DC_B_TEMP], 8, 2);
    lv_update_label_dp( &l2, reg_map[DC_A_VOLTAGE], 5, 3);
    lv_update_label_dp( &l3, reg_map[DC_B_VOLTAGE], 5, 3);
}

// up converter attenuator and RF switch screen
static void scr_uc(bool isInit)
{
    if (isInit) {
        lv_init_label(&l0,   5, 15, &lv_font_fa, BROADCAST_TOWER, LV_LEFT, true);

        lv_triple(    &l1,  62, 9, "UCA", "00000", "dB " RESISTOR);
        lv_init_label(&l2, 208, 9, &lv_font_roboto_mono_17, SWITCH_OPEN, LV_LEFT, false);

        lv_triple(    &l3,  62, l1.y1 + 9, "UCB", "00000", "dB " RESISTOR);
        lv_init_label(&l4, 208, l1.y1 + 9, &lv_font_roboto_mono_17, SWITCH_OPEN, LV_LEFT, false);
    }
    lv_update_label_fix(&l1, -reg_map[UP_A_ATT], 1, 1);
    lv_update_label(&l2, (reg_map[INLK_A_FLAGS] & 0x02) ? SWITCH_CLOSED : SWITCH_OPEN);

    lv_update_label_fix(&l3, -reg_map[UP_B_ATT], 1, 1);
    lv_update_label(&l4, (reg_map[INLK_B_FLAGS] & 0x02) ? SWITCH_CLOSED : SWITCH_OPEN);
}

// Local oscillator status
static void scr_lo_inlk(bool isInit)
{
    if (isInit) {
        lv_init_label(&l0, 8, 15, &lv_font_fa, UNLOCK_ALT, LV_LEFT, true);

        lv_triple(&l0, 50, 1, "LOA", "00000", "dBm");
        lv_init_label(&l1, 190, l0.y, &lv_font_roboto_mono_17, CHECK_CIRCLE, LV_LEFT, false);
        lv_init_label(&l2, l1.x1 + 5, l0.y, &lv_font_roboto_mono_17, SQUARE, LV_LEFT, false);

        lv_triple(&l3, 50, l0.y1 + 2, "LOB", "00000", "dBm");
        lv_init_label(&l4, 190, l3.y, &lv_font_roboto_mono_17, CHECK_CIRCLE, LV_LEFT, false);
        lv_init_label(&l5, l4.x1 + 5, l3.y, &lv_font_roboto_mono_17, SQUARE, LV_LEFT, false);

        lv_init_label(&l6, 50, l3.y1 + 2, &lv_font_roboto_mono_17, "Push to reset latch", LV_LEFT, true);
    }
    bool isA = (reg_map[INLK_A_FLAGS] & 0x01);
    bool isAL = (reg_map[INLK_A_FLAGS] & 0x02);
    bool isB = (reg_map[INLK_B_FLAGS] & 0x01);
    bool isBL = (reg_map[INLK_B_FLAGS] & 0x02);

    lv_update_label_fix(&l0, (int16_t)reg_map[INLK_A_VAL], 8, 1);
    lv_update_label(&l1, isA ? CHECK_CIRCLE : CIRCLE);
    lv_update_label(&l2, isAL ? CHECK_SQUARE : SQUARE);

    lv_update_label_fix(&l3, (int16_t)reg_map[INLK_B_VAL], 8, 1);
    lv_update_label(&l4, isB ? CHECK_CIRCLE : CIRCLE);
    lv_update_label(&l5, isBL ? CHECK_SQUARE : SQUARE);

    // lv_update_label(&l6, (!isAL || !isBL) ? "Push to reset latch" : "");
}

// show interlock thresholds and maxmin values
static void scr_inlk_maxmin(bool isInit, int type)
{
    if (isInit) {
        lv_init_label(&l0, 8, 15, &lv_font_fa, UNLOCK_ALT, LV_LEFT, true);

        lv_triple(&l0, 65, 1, CHEVRON_UP, "00000", "dBm");
        lv_triple(&l1, 50, l0.y1 + 2, type ? "LOB" : "LOA", "00000", "dBm");
        unsigned y = l1.y1 + 2;
        lv_triple(&l2, 65, y, CHEVRON_DOWN, "00000", "dBm");

        lv_triple(&l3, 175, 1, "(", "00000", ")");
        lv_triple(&l4, 175, y, "(", "00000", ")");
    }
    switch (type) {
        case 0:
            lv_update_label_fix(&l0, (int16_t)reg_map[INLK_A_VAL_MAX], 8, 1);
            lv_update_label_fix(&l1, (int16_t)reg_map[INLK_A_VAL], 8, 1);
            lv_update_label_fix(&l2, (int16_t)reg_map[INLK_A_VAL_MIN], 8, 1);
            lv_update_label_fix(&l3, (int16_t)reg_map[INLK_A_MAX_THR], 8, 1);
            lv_update_label_fix(&l4, (int16_t)reg_map[INLK_A_MIN_THR], 8, 1);
            break;

        case 1:
            lv_update_label_fix(&l0, (int16_t)reg_map[INLK_B_VAL_MAX], 8, 1);
            lv_update_label_fix(&l1, (int16_t)reg_map[INLK_B_VAL], 8, 1);
            lv_update_label_fix(&l2, (int16_t)reg_map[INLK_B_VAL_MIN], 8, 1);
            lv_update_label_fix(&l3, (int16_t)reg_map[INLK_B_MAX_THR], 8, 1);
            lv_update_label_fix(&l4, (int16_t)reg_map[INLK_B_MIN_THR], 8, 1);
            break;
    }

}

void draw_psu_gui(unsigned btns)
{
    static unsigned frm=0, screen=0;

    if (screen > 0 && (btns & 1))
        screen--;

    if (screen < 7 && (btns & 2))
        screen++;

    bool isInit = (frm == 0) || (btns & 3);
    if (isInit)
        fill(0);

    switch (screen) {
        case 0:
            scr_psu(isInit, 0);
            break;

        case 1:
            scr_psu(isInit, 1);
            break;

        case 2:
            scr_fpga(isInit);
            break;

        case 3:
            scr_dc(isInit);
            break;

        case 4:
            scr_uc(isInit);
            break;

        case 5:
            scr_lo_inlk(isInit);
            break;

        case 6:
            scr_inlk_maxmin(isInit, 0);
            break;

        case 7:
            scr_inlk_maxmin(isInit, 1);
            break;
    }

    frm++;
}
