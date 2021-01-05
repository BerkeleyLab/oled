#!/bin/bash
lv_font_conv --size 17 --bpp 4 --format lvgl --no-kerning --no-compress --lv-include lv_font.h \
  --font RobotoMono-Regular.ttf -r 0x20-0x7F,0xB0 \
  --font RobotoMono-Bold.ttf -r 0x20,0x2D,0x2E,0x30-0x39 \
  --font fa-solid-900.woff -r 0xf0c8,0xf14a,0xf111,0xf058,0xf077,0xf078 \
  -o ../firmware/lv_font_roboto_mono_17.c

lv_font_conv --size 32 --bpp 4 --format lvgl --no-kerning --no-compress --lv-include lv_font.h -o ../firmware/lv_font_fa.c \
--font fa-solid-900.woff -r 0xf2c7,0xf0e7,0xf1e6,0xf2db,0xf519,0xf13e

# size 17
SYM_S="square check-square circle check-circle chevron-up chevron-down"
python3 font_gen.py $SYM_S

# big icons
SYM_B="thermometer-full bolt plug microchip broadcast-tower unlock-alt"
python3 font_gen.py $SYM_B
