# Embera makefile
EMBERA_HOME=pwd

CC=gcc
#CC=/opt/STM/STLinux-2.3/devkit/sh4/bin/sh4-linux-gcc
#CC=/opt/cell/toolchain/bin/ppu-gcc
CFLAGS=-c -g #-DDEBUG
#LIBS=-pthread -std=c99 -ldl 
LIBS=-pthread 
EXECUTION=PTHREADS 
OMP=-fopenmp 

SRCDIR=src
#BASEFILES=mailbox component observer utils execution
OBJDIR=build
EXAMDIR=examples
#EXAMPLES=helloworld hello-composite hello-observation pi basic
TIFFFLAGS= -ltiff  -lm

CUSTOMDIR = ./custom
CUSTOMSRC = $(CUSTOMDIR)/custom.c

STORAGEDIR= ./storage
STORAGESRC= $(STORAGEDIR)/storage.c

INCLUDE = ./include 

SRCS= $(SRCDIR)/mailbox.c $(SRCDIR)/component.c $(SRCDIR)/observer.c $(SRCDIR)/utils.c $(SRCDIR)/execution.c 
OBJS= $(OBJDIR)/component.o $(OBJDIR)/observer.o $(OBJDIR)/mailbox.o $(OBJDIR)/utils.o $(OBJDIR)/execution.o $(OBJDIR)/custom.o $(OBJDIR)/storage.o

#objects = $(OBJS) $(SRCS)

# coment to disable observation
OBSFLAG = -DOBSERVATION_ON 

.PHONY: all clean

all:  embera examples

embera: 
	mkdir -p $(OBJDIR)
	$(CC) $(CFLAGS) $(LIBS) -I $(INCLUDE) -I $(CUSTOMDIR) $(STORAGEDIR) $(SRCS) $(CUSTOMSRC) $(STORAGESRC)
	mv *.o $(OBJDIR)

# Examples

examples: helloworld hello-observation hello-composite pi mjpeg basic composite-stapi

helloworld: $(EXAMDIR)/helloworld/helloworld.c embera
	$(CC) $(OBSFLAG) $(OBJS)  $(EXAMDIR)/$@/$@.c -I $(INCLUDE) -I $(CUSTOMDIR) $(LIBS) -g -o $@

hello-composite: $(EXAMDIR)/hello-composite/hello-composite.c embera
	$(CC) $(OBSFLAG) $(OBJS)  $(EXAMDIR)/$@/$@.c -I $(INCLUDE) -I $(CUSTOMDIR) $(LIBS) -g -o $@

hello-observation: $(EXAMDIR)/hello-observation/hello-observation.c embera
	$(CC) $(OBSFLAG) $(OBJS)  $(EXAMDIR)/$@/$@.c  -I $(INCLUDE) -I $(CUSTOMDIR) $(LIBS) -g -o $@  

pi: $(EXAMDIR)/pi/pi.c embera
	$(CC) $(OBSFLAG) $(OBJS)  $(EXAMDIR)/$@/$@.c -I $(INCLUDE) -I $(CUSTOMDIR) $(LIBS) -g -o $@

imageRMS: $(EXAMDIR)/imageTiff/imageRMS.c $(EXAMDIR)/imageTiff/image.o embera
	$(CC) $(OBSFLAG)  $(OBJS)  $(EXAMDIR)/imageTiff/imageRMS.c $(LIBS) -I $(INCLUDE)-I $(CUSTOMDIR) -g -o imageRMS $(EXAMDIR)/imageTiff/image.o $(TIFFFLAGS) 

image.o: $(EXAMDIR)/imageTiff/image.c embera
	$(CC) $(OBSFLAG)  $(OBJS) -I $(INCLUDE)-I $(CUSTOMDIR)  $(LIBS) -ltiff $(EXAMDIR)/imageTiff/image.c -c

mjpeg: $(EXAMDIR)/mjpeg/src/*.c embera
	make -f Makefile_mjpeg -B -C $(EXAMDIR)/mjpeg/

basic: $(EXAMDIR)/basic/basic.c
	make embera CUSTOMDIR=$(EXAMDIR)/$@/ CUSTOMSRC=$(EXAMDIR)/$@/custom.c
	$(CC) $(OBSFLAG) $(OBJS) $(EXAMDIR)/$@/$@.c -I $(INCLUDE) -I $(EXAMDIR)/$@ $(LIBS) -I$(STORAGEDIR) -g -o $@
	$(CC) $(EXAMDIR)/$@/test.c -o $(EXAMDIR)/$@/test

composite-stapi: $(EXAMDIR)/composite-stapi/composite-stapi.c
	make embera #STORAGEDIR=$(EXAMDIR)/$@/ STORAGESRC=$(EXAMDIR)/$@/storage.c 
	$(CC) $(OBSFLAG) $(OBJS) $(EXAMDIR)/$@/$@.c -I $(INCLUDE) -I $(EXAMDIR)/$@ $(LIBS) -I$(STORAGEDIR) -g -o $@

#Remove embera objects
clean:
	rm -rf $(OBJDIR)/*
	rm -f ./hello-composite
	rm -f ./hello-observation
	rm -f ./helloworld
	rm -f ./imageRMS
	rm -f ./mjpeg_decoder
	rm -f ./pi
	rm -f ./basic
	rm -f ./composite-stapi
	rm -f $(EXAMDIR)/basic/test
	rm -f ./result.tif


