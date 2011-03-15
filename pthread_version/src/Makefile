CC = gcc
BUILDDIR = ./
CFLAGS = -g --std=c99 `sdl-config --cflags` -I../headers -I/usr/include/malloc# -DVERBOSE -DINFO # -DPROGRESS # -DVERBOSE -DINFO -DDEADLOCK

SRC=    $(wildcard *.c)
OBJ=    $(SRC:%.c=$(BUILDDIR)/%.o)

all: $(OBJ)

$(BUILDDIR)/%.o : %.c
	echo '         | CC $<'
	$(CC) -c -o $@ $(CFLAGS) $<
