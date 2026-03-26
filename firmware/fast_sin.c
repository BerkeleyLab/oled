#include "fast_sin.h"
#include <limits.h>

#define HALF_MAX_ANGLE (MAX_ANGLE / 2)
#define QUARTER_MAX_ANGLE (MAX_ANGLE / 4)

#define FRAC_BITS 8  // Number of bits which are used for linear interpolation between table values
#define FRAC_MASK ((1 << FRAC_BITS) - 1)
#define TABLE_SIZE (QUARTER_MAX_ANGLE >> FRAC_BITS)

// [int(sin(2.0 * pi * i / MAX_ANGLE) * SHRT_MAX) for i in range(0, QUARTER_MAX_ANGLE,
// QUARTER_MAX_ANGLE // TABLE_SIZE)]
static const short quarter_sin_lut[TABLE_SIZE + 1] = {
    0,     1607,  3211,  4807,  6392,  7961,  9511,  11038, 12539, 14009, 15446,
    16845, 18204, 19519, 20787, 22004, 23169, 24278, 25329, 26318, 27244, 28105,
    28897, 29621, 30272, 30851, 31356, 31785, 32137, 32412, 32609, 32727, SHRT_MAX};

static int interpolate(unsigned alpha) {
    int int_val = alpha >> FRAC_BITS;
    int frac_val = alpha & FRAC_MASK;  // 0 .. 0xFF
    int a = quarter_sin_lut[int_val];
    int b = quarter_sin_lut[int_val + 1];
    int out = ((a << FRAC_BITS) + frac_val * (b - a)) >> FRAC_BITS;
    return out;
}

int get_sin(int alpha) {
    alpha &= MAX_ANGLE - 1;
    const int sign = alpha > HALF_MAX_ANGLE ? -1 : 1;
    alpha &= HALF_MAX_ANGLE - 1;
    if (alpha < QUARTER_MAX_ANGLE)
        return interpolate(alpha) * sign;
    if (alpha == QUARTER_MAX_ANGLE)
        return SHRT_MAX * sign;
    if (alpha > QUARTER_MAX_ANGLE) {
        return interpolate(2 * QUARTER_MAX_ANGLE - alpha) * sign;
    }
    return 0;
}

int get_cos(int alpha) { return get_sin(alpha + QUARTER_MAX_ANGLE); }
