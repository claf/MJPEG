CC=gcc -Wall -std=gnu99
CFLAGS+=-g -O0 -I../include/ -I/home/claferri/opt/include/ $(shell pkg-config --cflags kaapi)
CFLAGS+=-DC2X_USES_TIMING -DMJPEG_USES_TIMING
#CFLAGS+=-gdwarf-2 -g3 # Macros debug stuffs

LDFLAGS=$(shell pkg-config --libs kaapi) -L/home/claferri/opt/lib/ -lpthread -lSDL -lSGE -lgtg
LDLIB=/home/claferri/opt/lib/:$(shell pkg-config --libs-only-L kaapi | sed 's/^-L//')

ORI=../src/MJPEG
SRC=main.c decode.c fetch.c resize.c render.c conv.c huffman.c idct.c iqzz.c skip_segment.c unpack_block.c upsampler.c c2x.c timing.c trace_gtg.c
HEADERS=conv.h define_common.h huffman.h idct.h iqzz.h main.h MJPEG.h skip_segment.h unpack_block.h upsampler.h utils.h c2x.h timing.h trace_gtg.h
OBJ=$(SRC:.c=.o)
OUT=mjpeg
TMPFILE=.Generated

all: $(OUT)

$(OUT):$(OBJ)
	$(CC) $(LDFLAGS) -o $@ $^ 

%.o:%.c
	$(CC) $(CFLAGS) -o $@ -c $^

$(TMPFILE):$(wildcard $(ORI)/*.c)
	c2x -V -s splitter -i MJPEG.h ./ $(ORI) 
	touch $(TMPFILE)

$(SRC):$(TMPFILE)

run: $(OUT)
	LD_LIBRARY_PATH=$(LDLIB) ./mjpeg ~/toto.mjpeg

gdb: $(OUT)
	LD_LIBRARY_PATH=$(LDLIB) cgdb mjpeg core

debug: $(OUT)
	LD_LIBRARY_PATH=$(LDLIB) cgdb -- --args mjpeg ~/toto.mjpeg 

perl:$(TMPFILE)

clean:
	rm -rf $(OUT) *.o

veryclean:
	rm -rf $(OUT) *.o $(HEADERS) $(SRC) $(TMPFILE) 

.SILENT: clean veryclean
