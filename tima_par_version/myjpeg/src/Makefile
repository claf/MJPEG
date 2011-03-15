INC = -I../include -I../../dpn
LIB = -L ../../dpn -ldpn -lSDL -lpthread
OBJ = fetch_process.o idct_process.o render_process.o conv.o huffman.o idct.o iqzz.o main.o screen.o skip_segment.o unpack_block.o upsampler.o

CFLAGS = -g --std=c99 $(INC) -DNB_IDCT=1

mjpeg : $(OBJ)
	$(CC) -o $@ $^ $(LIB)
clean :
	$(RM) $(OBJ)
realclean : clean
	$(RM) mjpeg

