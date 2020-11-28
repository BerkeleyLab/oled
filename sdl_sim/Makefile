vpath %.c ../firmware
vpath %.c ../firmware/lib

LDLIBS = -lSDL2
CFLAGS += -I../firmware -I../firmware/lib
OBJS = frame_buffer.o aa_line.o demo.o sin1.o lv_font.o print.o
OBJS += lv_font_montserrat_18.o lv_font_montserrat_12.o lv_font_roboto_mono_10.o

all: test

test: $(OBJS)

clean:
	rm -rf test $(OBJS)
