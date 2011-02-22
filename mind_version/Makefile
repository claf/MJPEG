# If the MIND_ROOT variable is defined, look for 'mindc' in this directory
# otherwise, expect that 'mindc' is in the PATH.
ifdef MIND_ROOT
  # If MIND_ROOT contains '\', replace them by '/' (for Windows)
  MINDC = $(subst \,/, $(MIND_ROOT)/bin/mindc)
else 
  MINDC = mindc
endif

all: mjpeg

mjpeg: build
	$(MINDC) -src-path=src --ld-flags=-lpthread -o=build mjpeg.MjpegApplication:mjpeg.app

build:
	mkdir -p build

clean:
	rm -Rf build

.PHONY: all mjpeg clean
