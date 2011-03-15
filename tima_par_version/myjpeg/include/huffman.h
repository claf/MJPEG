/*************************************************************************************
 * vim :set ts=8:
 ******************************************************************************/
#ifndef __HUFFMAN_H
#define __HUFFMAN_H

#include "jpeg.h"
#include <stdint.h>

extern void free_huffman_tables(huff_table_t *root);

extern int load_huffman_table(FILE *movie, huff_table_t *ht);

#endif
