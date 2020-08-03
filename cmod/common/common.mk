XILINX_TOOL = VIVADO

include ~/fpga_wsp/bedrock/dir_list.mk
include $(BUILD_DIR)/top_rules.mk
include $(PICORV_DIR)/rules.mk

CMOD_DIR=../common
vpath %.S $(CMOD_DIR)
vpath %.lds $(CMOD_DIR)
vpath %.c $(CMOD_DIR) ../../firmware
vpath %.c $(CMOD_DIR) ../../firmware/lib
vpath system.v $(CMOD_DIR)

SRC_V += picorv32.v system.v uart_pack.v uart_rx.v uart_tx.v mpack.v munpack.v
SRC_V += memory2_pack.v pico_pack.v
SRC_V += stream_fifo.v shortfifo.v uart_fifo_pack.v uart_stream.v
SRC_V += sfr_pack.v gpio_pack.v gpioz_pack.v
SRC_V += pb_debouncer.v spi_pack.v spi_engine.v

OBJS += system.o print.o timer.o ssd1322.o
OBJS += frame_buffer.o aa_line.o demo.o sin1.o lv_font.o lv_font_montserrat_18.o

# run from block ram
OBJS += startup_irq.o

#size of the blockRam [bytes]
BLOCK_RAM_SIZE = 131072
SYNTH_OPT += -DBLOCK_RAM_SIZE=$(BLOCK_RAM_SIZE)

CFLAGS += -DGIT_VERSION=\"$(GIT_VERSION)\"
CFLAGS += -I$(CMOD_DIR) -I../../firmware -I../../firmware/lib
CFLAGS += -DBOOTLOADER_BAUDRATE=$(BOOTLOADER_BAUDRATE)
CFLAGS += -ffunction-sections
