#ifndef __FETCH_H
#define __FETCH_H
#include "MJPEG.h"
#include "define_common.h"

#define READ_BINARY     "r"

/* Error exit handler */
#define ERREXIT(msg)  (fprintf(stderr, "%s\n", msg), exit(EXIT_FAILURE))

/* Read one byte, testing for EOF */
static int 
read_1_byte (FILE* movie)
{
  int c;

  NEXT_TOKEN(c, movie);
  if (c == EOF)
    ERREXIT("Premature EOF in JPEG file");
  return c;
}

/* Read 2 bytes, convert to unsigned int */
/* All 2-byte quantities in JPEG markers are MSB first */
static unsigned int 
read_2_bytes (FILE* movie)
{
  int c1, c2; 

  NEXT_TOKEN(c1, movie);
  if (c1 == EOF)
    ERREXIT("Premature EOF in JPEG file");
  NEXT_TOKEN(c2, movie);
  if (c2 == EOF)
    ERREXIT("Premature EOF in JPEG file");
  return (((unsigned int) c1) << 8) + ((unsigned int) c2);
}

void fetch();

#endif
