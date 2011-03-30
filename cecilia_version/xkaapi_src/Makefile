CC=gcc
CFLAGS=-std=c99 -Wall -g -I../include/ $(shell pkg-config --cflags kaapi)
#CFLAGS=-std=c99 -O3 -I../include/ $(shell pkg-config --cflags kaapi)
LDFLAGS=$(shell pkg-config --libs kaapi) -lpthread

ORI=../src/MJPEG
SRC=main.c decode.c fetch.c resize.c render.c
OBJ=$(SRC:.c=.o)
OUT=mjpeg
TMPFILE=.Generated

all: $(OUT)

$(OUT):$(OBJ)
	$(CC) $(LDFLAGS) -o $@ $^ 

%.o:%.c
	$(CC) $(CFLAGS) -o $@ -c $^

$(TMPFILE):$(wildcard $(ORI)/*.c)
	c2x ./ $(ORI) 
	touch .Generated

$(SRC):$(TMPFILE)

run: $(OUT)
	LD_LIBRARY_PATH=/home/claferri/opt/lib/ ./mjpeg ~/toto.mjpeg

clean:
	rm -rf $(OUT) *.o $(TMPFILE)

veryclean:
	rm -rf $(OUT) *.o main.h $(SRC) $(TMPFILE) 

.SILENT: clean veryclean
