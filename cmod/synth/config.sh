i#!/bin/bash
openocd -f digilent_cmod_a7.cfg -c "init; pld load 0 system_top.bit; exit;"
