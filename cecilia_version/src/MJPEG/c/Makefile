#Set up directories

OBJ_DIR =.
SRC_DIR =.
INC_DIR =.

# Define compilation flags

CC = gcc -g #-DMY_DEBUG -DINFO #-fno-inline #-mcmodel=medium #-DINFO #-pg
INC = -I$(INC_DIR) -I/opt/local/include/ -I/usr/include/SDL/
CFLAGS += $(INC) -Wall -std=c99
LDFLAGS += -L/opt/local/lib/ -lSDL -lSGE

# Define object files list

GENERATED_OBJ_FILES =  $(OBJ_DIR)/screen.o 
OBJ_FILES = $(GENERATED_OBJ_FILES) $(OBJ_DIR)/fetch.o 
OBJ_FILES += $(OBJ_DIR)/iqzz.o $(OBJ_DIR)/idct.o $(OBJ_DIR)/conv.o $(OBJ_DIR)/upsampler.o $(OBJ_DIR)/skip_segment.o $(OBJ_DIR)/huffman.o $(OBJ_DIR)/render.o $(OBJ_DIR)/unpack_block.o $(OBJ_DIR)/decode.o $(OBJ_DIR)/resize.o 
NEW_OBJ_FILES =

all : mjpeg

# Real compilation

mjpeg : $(OBJ_FILES)
	$(CC) -o mjpeg $(OBJ_FILES) $(LDFLAGS)
$(OBJ_DIR)/screen.o : $(SRC_DIR)/screen.c $(INC_DIR)/screen.h
	$(CC) $(CFLAGS) -c $(SRC_DIR)/screen.c -o $(OBJ_DIR)/screen.o
$(OBJ_DIR)/example.o : example/example.c 
	$(CC) $(CFLAGS) -g -c example/example.c -o $@

# START CUT 
$(OBJ_DIR)/fetch.o : $(SRC_DIR)/fetch.c $(INC_DIR)/MJPEG.h $(INC_DIR)/huffman.h $(INC_DIR)/skip_segment.h $(INC_DIR)/utils.h $(INC_DIR)/define_common.h $(INC_DIR)/unpack_block.h $(INC_DIR)/decode.h
	$(CC) $(CFLAGS) -c $(SRC_DIR)/fetch.c -o $@
$(OBJ_DIR)/idct.o : $(SRC_DIR)/idct.c $(INC_DIR)/idct.h
	$(CC) $(CFLAGS) -c $(SRC_DIR)/idct.c -o $@
$(OBJ_DIR)/iqzz.o : $(SRC_DIR)/iqzz.c $(INC_DIR)/iqzz.h
	$(CC) $(CFLAGS) -c $(SRC_DIR)/iqzz.c -o $@
$(OBJ_DIR)/conv.o : $(SRC_DIR)/conv.c $(INC_DIR)/conv.h
	$(CC) $(CFLAGS) -c $(SRC_DIR)/conv.c -o $@
$(OBJ_DIR)/upsampler.o : $(SRC_DIR)/upsampler.c $(INC_DIR)/upsampler.h
	$(CC) $(CFLAGS) -c $(SRC_DIR)/upsampler.c -o $@
$(OBJ_DIR)/skip_segment.o : $(SRC_DIR)/skip_segment.c $(INC_DIR)/skip_segment.h
	$(CC) $(CFLAGS) -c $(SRC_DIR)/skip_segment.c -o $@
$(OBJ_DIR)/decode.o : $(SRC_DIR)/decode.c $(INC_DIR)/decode.h
	$(CC) $(CFLAGS) -c $(SRC_DIR)/decode.c -o $@
$(OBJ_DIR)/resize.o : $(SRC_DIR)/resize.c $(INC_DIR)/resize.h
	$(CC) $(CFLAGS) -c $(SRC_DIR)/resize.c -o $@
$(OBJ_DIR)/render.o : $(SRC_DIR)/render.c $(INC_DIR)/render.h
	$(CC) $(CFLAGS) -c $(SRC_DIR)/render.c -o $@
$(OBJ_DIR)/huffman.o : $(SRC_DIR)/huffman.c $(INC_DIR)/huffman.h
	$(CC) $(CFLAGS) -c $(SRC_DIR)/huffman.c -o $@
$(OBJ_DIR)/unpack_block.o : $(SRC_DIR)/unpack_block.c $(INC_DIR)/unpack_block.h
	$(CC) $(CFLAGS) -c $(SRC_DIR)/unpack_block.c -o $@
# END CUT

clean:
	rm -f mjpeg $(GENERATED_OBJ_FILES) $(NEW_OBJ_FILES)
	rm -f mire $(OBJ_DIR)/example.o
# START CUT
	rm -f $(OBJ_FILES)
# END CUT
