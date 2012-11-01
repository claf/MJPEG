CC=gcc -Wall -std=gnu99
CFLAGS+=-g -O0 -I../include/ -I/home/claferri/opt/include/ 

#CFLAGS+=-DC2X_USES_TIMING # Allow to print global time spent to steal / push / pop ...
CFLAGS+=-DMJPEG_USES_TIMING # Allow to print global time spent to decode / resize by thread
CFLAGS+=-DMJPEG_TRACE_FRAME # Generate a ViTe trace of execution
CFLAGS+=-D_C2X_USES_LOCKFREE # Uses lockfree implem of workqueue (see c2x.h)
#CFLAGS+=-DC2X_USES_PAPI # Uses PAPI counters
#CFLAGS+=-D_C2X_DEBUG # Debug stuffs

#CFLAGS+=-gdwarf-2 -g3 # Macros debug stuffs

LDFLAGS=-L/home/claferri/opt/lib/ -lpthread -lSDL -lSGE -lgtg -lpapi -ltrace_gtg
LDLIB=/home/claferri/opt/lib/

ORI=../src/MJPEG
SRC=decode.c fetch.c resize.c render.c conv.c huffman.c idct.c iqzz.c skip_segment.c unpack_block.c upsampler.c timing.c trace.c
HEADERS=conv.h define_common.h huffman.h idct.h iqzz.h main.h MJPEG.h skip_segment.h unpack_block.h upsampler.h utils.h timing.h trace.h
OBJ=$(SRC:.c=.o)
OUT=mjpeg
TMPFILE=.Generated

all: $(OUT)

$(OUT):$(OBJ)
	$(CC) $(LDFLAGS) -o $@ $^ 

%.o:%.c
	$(CC) $(CFLAGS) -o $@ -c $^

run: $(OUT)
	LD_LIBRARY_PATH=$(LDLIB) ./mjpeg ~/toto.mjpeg

gdb: $(OUT)
	LD_LIBRARY_PATH=$(LDLIB) cgdb mjpeg core

debug: $(OUT)
	LD_LIBRARY_PATH=$(LDLIB) cgdb -- --args mjpeg ~/toto.mjpeg 

diff:
	mkdir -p dtemp/
	mv -n *.[ch] dtemp/
	make veryclean && make perl
	diff -q ./ dtemp/

clean:
	rm -rf $(OUT) *.o *.ept

#.SILENT: clean veryclean
