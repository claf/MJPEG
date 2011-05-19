CC=gcc -Wall
CFLAGS+=-std=c99 -g -I../include/ $(shell pkg-config --cflags kaapi)
#CFLAGS=-std=c99 -O3 -I../include/ $(shell pkg-config --cflags kaapi)
LDFLAGS=$(shell pkg-config --libs kaapi) -lpthread -lSDL -lSGE
LDLIB=$(shell pkg-config --libs-only-L kaapi | sed 's/^-L//')

ORI=../src/MJPEG
SRC=main.c decode.c fetch.c resize.c render.c conv.c huffman.c idct.c iqzz.c skip_segment.c unpack_block.c upsampler.c
OBJ=$(SRC:.c=.o)
OUT=mjpeg
TMPFILE=.Generated

all: $(OUT)

$(OUT):$(OBJ)
	$(CC) $(LDFLAGS) -o $@ $^ 

%.o:%.c
	$(CC) $(CFLAGS) -o $@ -c $^

$(TMPFILE):$(wildcard $(ORI)/*.c)
	c2x -i MJPEG.h ./ $(ORI) 
	touch $(TMPFILE)

$(SRC):$(TMPFILE)

run: $(OUT)
	LD_LIBRARY_PATH=$(LDLIB) ./mjpeg ~/toto.mjpeg

clean:
	rm -rf $(OUT) *.o

veryclean:
	rm -rf $(OUT) *.o *.h $(SRC) $(TMPFILE) 

.SILENT: clean veryclean
