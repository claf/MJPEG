# If the MIND_ROOT variable is defined, look for 'mindc' in this directory
# otherwise, expect that 'mindc' is in the PATH.

MFLAGS= -src-path=src
CFLAGS= -c=-g -I=/usr/include/SDL -c=-D_GNU_SOURCE=1 -c=-D_REENTRANT -c=-DDEBUG -c=-Wall
LDFLAGS= -l=-lSDL -l=-lpthread
OPTIONS= --keep --no-bin 

MINDC = mindc ${OPTIONS} ${MFLAGS} ${CFLAGS} ${LDFLAGS}
all: mjpeg

mjpeg: build
	$(MINDC) -o=build mjpeg.MjpegApplication:mjpeg.app

build:
	mkdir -p build

clean:
	rm -Rf build

.PHONY: all mjpeg clean
