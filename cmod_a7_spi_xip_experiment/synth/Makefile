TARGET = system

all: $(TARGET)_top.bit

include ../common/common.mk

BOOTLOADER_BAUDRATE = 921600
BOOTLOADER_SERIAL = /dev/ttyUSB1
HARDWARE = cmod_a7
SYNTH_OPT += -DMEMORY_PACK_FAST

SRC_V += system_top.v xilinx7_clocks.v

# run from block ram
# OBJS += startup_irq.o 0x0e0.lds

# run from flash starting from address 0x00220000
OBJS += startup_flash.o flash.lds

$(TARGET).elf: $(OBJS)
$(TARGET)_top.bit: $(SRC_V)

CLEAN += $(TARGET)_top.bit $(TARGET)_top.bin flash8.bin
CLEAN_DIRS += _xilinx .Xil
include $(BUILD_DIR)/bottom_rules.mk
