include ../../../../bedrock/dir_list.mk

vpath %.c ../firmware
vpath %.c ../firmware/lib
vpath %.c $(PICORV_DIR)/firmware/src

GIT_VERSION = $(shell git describe --abbrev=4 --dirty --always --tags)

LDLIBS = -lSDL2
LDLIBS = -L$(shell brew --prefix sdl2)/lib -lSDL2
CFLAGS += -I../firmware -I../firmware/lib -I$(PICORV_DIR)/firmware/inc -I/opt/homebrew/Cellar/sdl2/2.32.10/include/
CFLAGS += -DGIT_VERSION=\"$(GIT_VERSION)\" -DSDL_SIM -g
OBJS = frame_buffer.o lv_font.o print.o aa_line.o demo.o fast_sin.o
OBJS += lv_font_roboto_12.o lv_font_roboto_mono_17.o lv_font_fa.o

all: test

test: $(OBJS)

clean:
	rm -rf test $(OBJS)
