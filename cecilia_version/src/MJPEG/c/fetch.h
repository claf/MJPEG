#ifndef __FETCH_H
#define __FETCH_H
#include "MJPEG.h"

#define READ_BINARY     "r"

/* Return next input byte, or EOF if no more */
#define NEXTBYTE(movie)  getc(movie)

/* Error exit handler */
#define ERREXIT(msg)  (fprintf(stderr, "%s\n", msg), exit(EXIT_FAILURE))

/* Read one byte, testing for EOF */
static int 
read_1_byte (FILE* movie)
{
  int c;

  c = NEXTBYTE(movie);
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

  c1 = NEXTBYTE(movie);
  if (c1 == EOF)
    ERREXIT("Premature EOF in JPEG file");
  c2 = NEXTBYTE(movie);
  if (c2 == EOF)
    ERREXIT("Premature EOF in JPEG file");
  return (((unsigned int) c1) << 8) + ((unsigned int) c2);
}

void fetch();

#endif
