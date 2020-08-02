#!/bin/bash
openocd -f digilent_cmod_a7.cfg -c "init; jtagspi_init 0 $HOME/.openocd/bscan_spi_bitstreams/bscan_spi_xc7a35t.bit; flash read_bank 0 r.bin; exit;"
