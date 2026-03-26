#ifndef FAST_SIN_H
#define FAST_SIN_H

#define MAX_ANGLE 0x8000

// for angle alpha (360 deg = MAX_ANGLE) return its sine, scaled to +-SHORT_MAX
int get_sin(int alpha);

// return its cosine, scaled to +-SHORT_MAX
int get_cos(int alpha);

#endif
