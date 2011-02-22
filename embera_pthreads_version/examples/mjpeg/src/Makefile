CC = gcc
#CC=/opt/STM/STLinux-2.3/devkit/sh4/bin/sh4-linux-gcc
BUILDDIR = ../../../build
SDLCOMMAND= sdl-config --cflags
CFLAGS = -g --std=c99 -I../include -I../../../include -I../../../custom -I.. -I/usr/include/malloc -DPROGRESS #-DVERBOSE -DINFO #  # -DVERBOSE -DINFO -DDEADLOCK

#CFLAGS = -g --std=c99 `sdl-config --cflags` -I../include -I../../../include -I.. -I/usr/include/malloc -DPROGRESS -DLIBSDL


SRC=    $(wildcard *.c)
OBJ=    $(SRC:%.c=$(BUILDDIR)/%.o)

all: $(OBJ)

$(BUILDDIR)/%.o : %.c
	echo '         | CC $<'
	$(CC) -c -o $@ $(CFLAGS) $<
