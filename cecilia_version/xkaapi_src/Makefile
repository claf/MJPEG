CC=gcc #-Wall
CFLAGS=-std=c99 -g -I../include/ $(shell pkg-config --cflags kaapi)
#CFLAGS=-std=c99 -O3 -I../include/ $(shell pkg-config --cflags kaapi)
LDFLAGS=$(shell pkg-config --libs kaapi) -lpthread -lSDL -lSGE

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
	touch .Generated

$(SRC):$(TMPFILE)

run: $(OUT)
	LD_LIBRARY_PATH=/home/claferri/opt/lib/ ./mjpeg ~/toto.mjpeg

clean:
	rm -rf $(OUT) *.o $(TMPFILE)

veryclean:
	rm -rf $(OUT) *.o *.h $(SRC) $(TMPFILE) 

.SILENT: clean veryclean
