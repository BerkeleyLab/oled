XILINX_TOOL = VIVADO

include ~/fpga_wsp/bedrock/dir_list.mk
include $(BUILD_DIR)/top_rules.mk
include $(PICORV_DIR)/rules.mk

CMOD_DIR=../common
vpath %.S $(CMOD_DIR)
vpath %.lds $(CMOD_DIR)
vpath %.c $(CMOD_DIR) ../../sdl_sim
vpath %.c $(CMOD_DIR) ../../firmware
vpath %.cpp $(CMOD_DIR) ../../sdl_sim
vpath system.v $(CMOD_DIR)
vpath sram_model.v $(PICORV_DIR)/test/sram
vpath spiflash.v $(PICORV_DIR)/test/memio

SRC_V += picorv32.v system.v uart_pack.v uart_rx.v uart_tx.v mpack.v munpack.v
# builds can use memory_pack.v or memory_pack2.v, depending on MEMORY_PACK_FAST
SRC_V += memory2_pack.v pico_pack.v
SRC_V += stream_fifo.v shortfifo.v uart_fifo_pack.v uart_stream.v
SRC_V += sfr_pack.v gpio_pack.v gpioz_pack.v spimemio.v spimemio_pack.v
SRC_V += pb_debouncer.v sram_pack.v sram2_pack.v spi_pack.v spi_engine.v

OBJS += system.o print.o timer.o frame_buffer.o aa_line.o demo.o sin1.o ssd1322.o

#size of the blockRam [bytes]
BLOCK_RAM_SIZE = 16384
SYNTH_OPT += -DBLOCK_RAM_SIZE=$(BLOCK_RAM_SIZE)

CFLAGS += -DGIT_VERSION=\"$(GIT_VERSION)\"
CFLAGS += -I../common -I../../sdl_sim -I../../firmware
CFLAGS += -DBOOTLOADER_BAUDRATE=$(BOOTLOADER_BAUDRATE)
CFLAGS += -ffunction-sections
