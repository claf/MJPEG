# If the MIND_ROOT variable is defined, look for 'mindc' in this directory
# otherwise, expect that 'mindc' is in the PATH.
MINDC = mindc --keep --no-bin -c=-g -e

all: mjpeg

mjpeg: build
	$(MINDC) -src-path=src --ld-flags=-lpthread -o=build mjpeg.MjpegApplication:mjpeg.app

build:
	mkdir -p build

clean:
	rm -Rf build

.PHONY: all mjpeg clean
