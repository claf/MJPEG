#ifndef _MJPEG_HEADER_
#define _MJPEG_HEADER_

#include <stdint.h>

#define FRAME_LOOKAHEAD 5

typedef struct {
  int x;
  int y;
  int stream_id;
  int frame_id;
  uint32_t component_index;
  void* buffer; /* will point to whatever changing type data are */
  frame_chunk_t* next; /* chain $nb_MCU chunk from Fetch to Decode */
} frame_chunk_t;

typedef struct {
  /* TODO : could be stored in stream_info_t* table */
  /* H(4bits high) and V(4bits low) sampling * factors */
  /* ((SOF_component[component_index].HV >> 4) & 0xf) */
  uint8_t HV;
  int max_ss_h;
  int max_ss_v;
  uint8_t DQT_table[FRAME_LOOKAHEAD][4][64]; /* Quantization tables */
} stream_info_t;

// Global stream table :
extern stream_info_t* Streams;

// Global chunk achievement table :
/* in fact A[X][Y] with X stream and Y frame lookahead */
extern uint32_t* Achievements[FRAME_LOOKAHEAD];

// Global unresized buffers table :
/* in fact A[X][Y] with X stream and Y frame lookahead */
extern void* Buffers[FRAME_LOOKAHEAD];

// Global resized buffers table :
/* in fact A[X][Y] with X stream and Y frame lookahead */
extern void* resized_Buffers[FRAME_LOOKAHEAD];

// Global resizing factors table :
/* */
extern uint8_t* resize_Factors;

// Global last frame_id on the screen :
extern uint32_t last_frame_id;

#endif // _MJPEG_HEADER_
