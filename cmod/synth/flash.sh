#!/bin/bash
FLASH_BIT=$HOME/.openocd/bscan_spi_bitstreams/bscan_spi_xc7a35t.bit

if [ ! -f $FLASH_BIT ]; then
    echo "$FLASH_BIT not found!"
    git clone https://github.com/jordens/bscan_spi_bitstreams.git $HOME/.openocd/bscan_spi_bitstreams
fi

openocd -f ./digilent_cmod_a7.cfg -c "init; jtagspi_init 0 $FLASH_BIT; jtagspi_program system_top.bin 0x0; fpga_program; exit;"
