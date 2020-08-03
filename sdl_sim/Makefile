vpath %.c ../firmware
vpath %.c ../firmware/lib

LDLIBS = -lSDL2
CFLAGS += -I../firmware -I../firmware/lib

all: test

test: frame_buffer.o aa_line.o demo.o sin1.o

clean:
	rm -rf test *.o
