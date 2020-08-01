TARGET = system

all: $(TARGET).vcd

include ../common/common.mk

# max. baudrate is F_CLK / 8
BOOTLOADER_BAUDRATE = 8522727
VFLAGS += -DMEMORY_PACK_FAST -DBOOTLOADER_BAUDRATE=$(BOOTLOADER_BAUDRATE)
# virtual SPI flash contents
VCD_ARGS += +firmware=flash8.hex
CFLAGS += -DSIMULATION
SRC_V += system_tb.v sram_model.v spiflash.v

# run from block ram
# OBJS += startup_irq.o 0x000.lds

# run from flash
OBJS += startup_flash.o flash.lds

# need to subtract the flash BASE_ADDR
flash8.hex: $(TARGET).elf
	$(RISCV_TOOLS_PREFIX)objcopy --change-addresses -0x01000000 $^ -O verilog $@

$(TARGET).vcd: flash8.hex
$(TARGET).elf: $(OBJS)
$(TARGET)_tb: $(SRC_V)

CLEAN += flash8.hex flash32.hex startup_irq.o

include $(BUILD_DIR)/bottom_rules.mk

.PRECIOUS: system.vcd
