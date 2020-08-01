TARGET = system

vpath %.c ../sdl_sim

include ~/fpga_wsp/bedrock/dir_list.mk
include $(BUILD_DIR)/top_rules.mk
include $(PICORV_DIR)/rules.mk

LVGL_DIR=/home/michael/fpga_wsp/oled/firmware
LVGL_DIR_NAME=lvgl
include lvgl/lvgl.mk

all: $(TARGET)32.hex

BOOTLOADER_BAUDRATE = 115200
BOOTLOADER_SERIAL = /dev/ttyUSB1
HARDWARE = cmod_a7
BLOCK_RAM_SIZE = 131072
SYNTH_OPT += -DBLOCK_RAM_SIZE=$(BLOCK_RAM_SIZE)
SYNTH_OPT += -DMEMORY_PACK_FAST

CFLAGS += -DGIT_VERSION=\"$(GIT_VERSION)\"
CFLAGS += -I../sdl_sim -I../cmod_a7/common
CFLAGS += -DBOOTLOADER_BAUDRATE=$(BOOTLOADER_BAUDRATE)
CFLAGS += -ffunction-sections

OBJS += bootloader.o 0x0e0.lds
OBJS += system.o print.o timer.o startup_irq.o frame_buffer.o ssd1322.o
OBJS += $(CSRCS:.c=.o)

$(TARGET).elf: $(OBJS)

include $(BUILD_DIR)/bottom_rules.mk


