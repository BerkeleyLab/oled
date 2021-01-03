#ifndef GUI_H
#define GUI_H

#include "lv_font.h"

//-----------------------------------------------
// Dirty GUI stuff
//-----------------------------------------------
// Horizontal alignment
typedef enum {
    LV_LEFT,
    LV_CENTER,
    LV_RIGHT,
    LV_RIGHT_REF_LEFT
} t_align;

typedef struct {
    // text origin
    int16_t x;
    int16_t y;
    // clip window
    int16_t x0;
    int16_t y0;
    int16_t x1;
    int16_t y1;
    t_align align;
    lv_font_t *fnt;
} t_label;

// initialize a label with aligned text, also handling the bounding box which
// is erased on redraw
// x0, y0 is the anchor point, which is on the top left / middle / right
// depending on the chosen alignment
// size of the bounding box, which is erased for redraws, is inferred from `init` text
void lv_init_label(t_label *lbl, int x, int y, lv_font_t *fnt, const char *init, t_align a);

// Update the text in a label, same signature as printf
// void lv_update_label(t_label *lbl, const char *format, ...);
void lv_update_label(t_label *lbl, const char *buf);

// Update the text in a label with a fixed point fractional number
void lv_update_label_fix(t_label *lbl, int32_t val, const uint8_t nFract, uint8_t nDigits);

// Update the text in a label with a n-digit decimal number with decimal point fixed at dp
void lv_update_label_dp(t_label *lbl, uint32_t val, const uint8_t n, const uint8_t dp);

void lv_print(const char *str);

#endif
