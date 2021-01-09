#!/usr/env python
from os import system
from fontawesome import icons

custom_icons = {
    "switch-open": 0xFFF0,
    "switch-closed": 0xFFF1,
    "resistor": 0xFFF2,
    "amplifier": 0xFFF3,
}

# Small icons
symbols_s = "square check-square circle check-circle chevron-up chevron-down switch-open switch-closed resistor"

# Large icons
symbols_l = "thermometer-full bolt plug microchip broadcast-tower unlock-alt"

defines = set()


def getRange(names):
    global defines
    n_hex = ""
    for n in names:
        if n in icons:
            v = ord(icons[n])
        elif n in custom_icons:
            v = custom_icons[n]
        else:
            raise KeyError("Symbol Not found " + n)

        n_hex += hex(v) + ','

        dn = n.upper().replace('-', '_')
        dv = str(chr(v).encode('utf8')).replace('\'', '"')[1:]
        defines.add(f'#define {dn} {dv}\n')
    return n_hex[:-1]


r = getRange(symbols_s.split())
system(f'lv_font_conv --size 17 --bpp 4 --format lvgl --no-kerning --no-compress --lv-include lv_font.h \
  --font RobotoMono-Regular.ttf -r 0x20-0x7F,0xB0 \
  --font RobotoMono-Bold.ttf -r 0x20,0x2D,0x2E,0x30-0x39 \
  --font fa-solid-900-custom.woff -r {r} \
  -o ../firmware/lv_font_roboto_mono_17.c')

r = getRange(symbols_l.split())
system(f'lv_font_conv --size 32 --bpp 4 --format lvgl --no-kerning --no-compress --lv-include lv_font.h -o ../firmware/lv_font_fa.c \
--font fa-solid-900-custom.woff -r {r}')

with open('../firmware/lv_symbols.h', 'w') as f:
    f.write('#ifndef LV_SYMBOLS_H\n#define LV_SYMBOLS_H\n')
    f.writelines(sorted(defines))
    f.write('#endif\n')
