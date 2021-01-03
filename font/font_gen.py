#!/usr/env python
from sys import argv
from fontawesome import icons

names = argv[1:]
# names = "square check-square circle check-circle".split()

print('// ', end='')
for n in names:
    print(f'0x{repr(icons[n])[3:-1]},', end='')
print('')

for n in names:
    u = str(icons[n].encode('utf8')).replace('\'', '"')[1:]
    n = n.upper().replace('-', '_')
    print(f'#define LV_SYMBOL_{n} {u}')
print('')
