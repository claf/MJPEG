SUBDIRS = sources
BUILDDIR = build
CC = gcc
TARGET_CFLAGS = `sdl-config --cflags`
LIBS = -lpthread `sdl-config --libs`

all: $(OBJ)
	for i in $(SUBDIRS) ; do make -C $$i || exit $? ; done;
	$(CC) -o APP.x $(BUILDDIR)/*.o $(LIBS)

clean:
	rm -rf $(BUILDDIR)/*
