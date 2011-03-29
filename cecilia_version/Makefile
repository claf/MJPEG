# If the CECILIA_ROOT variable is defined, look for 'ceciliac' in this directory
# otherwise, expect that 'ceciliac' is in the PATH.
ifdef CECILIA_ROOT
  # If CECILIA_ROOT contains '\', replace them by '/' (for Windows)
  CECILIAC = $(subst \,/, $(CECILIA_ROOT)/bin/ceciliac)
else 
  CECILIAC = ceciliac
endif

XKAAPI_V="xkaapi_src"

all: mjpeg 
	$(MAKE) -C $(XKAAPI_V)

cecilia: mjpeg

xkaapi:
	$(MAKE) -C $(XKAAPI_V)

mjpeg: build/mjpeg
	$(CECILIAC) -src-path=src -o=build/mjpeg -t=unix MJPEG.MJPEG:mjpeg

build/mjpeg:
	mkdir -p build/mjpeg

clean:
	rm -Rf target
	$(MAKE) -C $(XKAAPI_V) $@

veryclean:
	rm -Rf build target
	$(MAKE) -C $(XKAAPI_V) $@

run: mjpeg
	./build/mjpeg/obj/mjpeg

.PHONY: all mjpeg cecilia xkaapi
	
.SILENT: clean veryclean
