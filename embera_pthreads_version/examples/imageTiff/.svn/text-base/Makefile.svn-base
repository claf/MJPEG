# Makefile Image treatment Application on EMBera

CC=gcc
CFLAGS= -c -g 
LIBS=-pthread
EXECUTION=PTHREADS
TIFFFLAGS= -ltiff  -lm

SRCDIR=src
OBJDIR=build
EXAMDIR=examples

SRCS= component.c observer.c mailbox.c utils.c execution.c
OBJS= component.o observer.o mailbox.o utils.o execution.o

.PHONY: all clean

all:  imageRMS

embera: $(SRCS)
	$(CC) $(CFLAGS) $(LIBS) $(SRCS)

# Examples
examples: helloworld hello-observation hello-composite pi imageRMS #mjpeg

imageRMS: imageRMS.c $(OBJS) image.o
	$(CC) imageRMS.c $(LIBS) -g -o imageRMS $(OBJS) image.o $(TIFFFLAGS) 

image.o: image.c
	$(CC) image.c -c

#Remove embera and examples objects

clean:
	rm *.o imageRMS result.tif
