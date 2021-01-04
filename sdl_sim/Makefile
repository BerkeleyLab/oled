include ../../bedrock/dir_list.mk

vpath %.c ../firmware
vpath %.c ../firmware/lib
vpath %.c $(PICORV_DIR)/firmware/src

LDLIBS = -lSDL2
CFLAGS += -I../firmware -I../firmware/lib -I$(PICORV_DIR)/firmware/inc -I../../analog_chassis_firmware/lib
OBJS = frame_buffer.o psu_board_gui.o lv_font.o print.o gui.o
# OBJS += printf.o
OBJS += lv_font_roboto_mono_17.o lv_font_fa.o

all: test

test: $(OBJS)

clean:
	rm -rf test $(OBJS)
