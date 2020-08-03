TARGET = system

all: $(TARGET)_top.bit

include ../common/common.mk

BOOTLOADER_BAUDRATE = 921600
BOOTLOADER_SERIAL = /dev/ttyUSB1
HARDWARE = cmod_a7
SYNTH_OPT += -DMEMORY_PACK_FAST

SRC_V += system_top.v xilinx7_clocks.v
OBJS += bootloader.o 0x0e0.lds

# -----------------
#  open-ocd config
# -----------------
# SPI proxy bitstreams from https://github.com/quartiq/bscan_spi_bitstreams
BS_BIT = $(HOME)/.openocd/bscan_spi_bitstreams/bscan_spi_xc7a35t.bit
OCD_CMD = openocd -f digilent_cmod_a7.cfg -c "init; jtagspi_init 0 $(BS_BIT); $(OCD_OPS) exit;"
SRC_V += system_top.v xilinx7_clocks.v

$(TARGET).elf: $(OBJS)
$(TARGET)_top.bit: $(SRC_V) system32.dat

flash_gateware: $(TARGET)_top.bin
	$(eval OCD_OPS = jtagspi_program $^ 0x0; fpga_program;)
	$(OCD_CMD)

CLEAN += $(TARGET)_top.bit $(TARGET)_top.bin $(TARGET)_top.prm system32.dat
CLEAN_DIRS += _xilinx .Xil
include $(BUILD_DIR)/bottom_rules.mk
