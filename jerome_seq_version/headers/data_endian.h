#ifndef DATA_ENDIAN_H
#define DATA_ENDIAN_H

#define SWITCH_ENDIAN_32(x) 																		\
	x =	((x & 0x000000ff) << 24) | ((x & 0x0000ff00) << 8)  |			\
	((x & 0x00ff0000) >> 8)  | ((x & 0xff000000) >> 24) 			

#define SWITCH_ENDIAN_16(x) x = ((x & 0x00ff) << 8) |	((x & 0xff00) >> 8) 				

#define DATA_IS_BIGENDIAN(type,data) SWITCH_ENDIAN_##type(data)
#define DATA_IS_LITTLEENDIAN(type,data)

#endif

