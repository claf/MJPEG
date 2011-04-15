#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include "MJPEG.h"
#include "huffman.h"
#include "unpack_block.h"
#include "define_common.h"
#include "skip_segment.h"
#include "decode.h"
#include "screen.h"

/* Global definition : */

// Global stream table :
stream_info_t* streams;

// Global frame achievement table (nb_chunk already treated) :
uint32_t Achievements[MAX_STREAM][FRAME_LOOKAHEAD];
uint32_t color = 1;

/* Intern declarations : */

int32_t MCUs[MAX_STREAM][FRAME_LOOKAHEAD][MAX_MCU_X][MAX_MCU_Y][4][4][64];

/* Intern functions : */

static void usage(char *progname) {
	printf("Usage : %s [options] <mjpeg_file_1> [<mjpeg_file_2> ...]\n", progname);
	printf("Options list :\n");
	printf("\t-f <framerate> : set framerate for the movie\n");
  printf("\t-c : decode streams in black and white format\n");
	printf("\t-h : display this help and exit\n");
}

int main(int argc, char** argv)
{
  printf ("Main start\n");

  /* TODO : dynamic alloc :
   * Streams 
   * Achievements
   * MCUs
   */

  uint8_t marker[2];
  uint8_t HT_type = 0;
  uint8_t HT_index = 0;
  uint8_t DQT_table[MAX_STREAM][FRAME_LOOKAHEAD][4][64];
  uint8_t QT_index = 0;
  uint16_t nb_MCU = 0, nb_MCU_X = 0, nb_MCU_Y = 0;
  uint16_t max_ss_h = 0 , max_ss_v = 0;
  uint8_t index = 0, index_X, index_Y, index_SOF;
  uint8_t component_order[4];
  uint32_t YH = 0, YV = 0;
  uint32_t CrH = 0, CrV = 0, CbH = 0, CbV = 0;
  uint32_t screen_init_needed;
  uint32_t end_of_file;
  int chroma_ss;
  int args;
  uint32_t framerate = 0;


  jfif_header_t jfif_header;
  DQT_section_t DQT_section;
  SOF_section_t SOF_section;
  SOF_component_t SOF_component[3];
  DHT_section_t DHT_section;
  SOS_section_t SOS_section;
  SOS_component_t SOS_component[3];

  frame_chunk_t *chunks;
  uint8_t nb_streams = 0;
  int stream_id = 0;
  int *frame_id;
  int32_t *MCU = NULL;

  scan_desc_t scan_desc = { 0, 0, {}, {} };
  huff_table_t *tables[2][4] = {
    {NULL , NULL, NULL, NULL} ,
    {NULL , NULL, NULL, NULL}
  };

  screen_init_needed = 1;
  if (argc < 2) {
    usage(argv[0]);
    exit(1);
  }
  args = 1;

  while (args < argc) {
    if (argv[args][0] ==  '-') {
      switch(argv[args][1]) {
        case 'f':
          if ((argc - args) < 1) {
            usage(argv[0]);
            exit(1);
          }
          framerate = atoi(argv[args+1]);
          args +=2;
          break;
        case 'h':
          usage(argv[0]);
          exit(0);
          break;
        case 'c':
          color=0;
          args++;
          break;
        default :
          usage(argv[0]);
          exit(1);
          break;
      }
    } else {
      break;
    }
  }

  if ((argc - args) < 1) {
    usage(argv[0]);
    exit(1);
  } else {
    printf ("debug once\n");
    nb_streams = argc - args;
    chunks = malloc (sizeof(frame_chunk_t) * nb_streams * FRAME_LOOKAHEAD * MAX_MCU_X * MAX_MCU_Y);
    frame_id = malloc (sizeof(int) * nb_streams);
    //TODO zero frame_id
    frame_id[0] = 0;
    streams = (stream_info_t*) malloc (sizeof(stream_info_t) * nb_streams);
  }
    
  // Allocation for streams FILE table :
  FILE *movies[nb_streams];

  for (int i = 0; i < nb_streams; i++)
  {
    movies[i] = NULL;
    if ((movies[i] = fopen(argv[args], "r")) == NULL)
    {
      perror(strerror(errno));
      exit(-1);
    }
    args++;
  }

//TODO : read begining of every files to find max_X and max_Y

  /*---- Actual computation ----*/
  end_of_file = 0;
  while (!end_of_file ) {
    int elem_read = fread(&marker, 2, 1, movies[0]);
    if (elem_read != 1) {
      if (feof(movies[0])) {
        // TODO : atomic increment, the if end_of_file == nb_streams ...
        end_of_file = 1;
        break;
      } else {
        fprintf(stderr, "Error reading marker from input file\n");
        exit (1);
      }
    }

    if (marker[0] == M_SMS) {
      switch (marker[1]) {
        case M_SOF0:
          {
            IPRINTF("SOF0 marker found for stream %d : frame %d\r\n", stream_id, frame_id[stream_id]);
            
            COPY_SECTION(&SOF_section, sizeof(SOF_section), movies[0]);
            CPU_DATA_IS_BIGENDIAN(16, SOF_section.length);
            CPU_DATA_IS_BIGENDIAN(16, SOF_section.height);
            CPU_DATA_IS_BIGENDIAN(16, SOF_section.width);

            VPRINTF("Data precision = %d\r\n",
                SOF_section.data_precision);
            VPRINTF("Image height = %d\r\n",
                SOF_section.height);
            VPRINTF("Image width = %d\r\n",
                SOF_section.width);
            VPRINTF("%d component%c\r\n",
                SOF_section.n,
                (SOF_section.n > 1) ? 's' : ' ');

            COPY_SECTION(&SOF_component,
                sizeof(SOF_component_t)* SOF_section.n, movies[0]);

            YV = SOF_component[0].HV & 0x0f;
            YH = (SOF_component[0].HV >> 4) & 0x0f;

            streams[stream_id].HV = SOF_component[0].HV;

            for (index = 0 ; index < SOF_section.n ; index++) {
              if (YV > max_ss_v) {
                max_ss_v = YV;
              }
              if (YH > max_ss_h) {
                max_ss_h = YH;
              }
            }
            streams[stream_id].max_ss_h = max_ss_h;
            streams[stream_id].max_ss_v = max_ss_v;
  
            printf ("max_ss_h = %d, max_ss_v = %d\n", max_ss_h, max_ss_v);

            if (SOF_section.n > 1) {
              CrV = SOF_component[1].HV & 0x0f;
              CrH = (SOF_component[1].HV >> 4) & 0x0f;
              CbV = SOF_component[2].HV & 0x0f;
              CbH = (SOF_component[2].HV >> 4) & 0x0f;
            }
            IPRINTF("Subsampling factor = %ux%u, %ux%u, %ux%u\r\n",
                YH, YV, CrH, CrV, CbH, CbV);

            if (screen_init_needed == 1) {
              screen_init_needed = 0;
              screen_init(SOF_section.width, SOF_section.height, framerate);

              nb_MCU_X = intceil(SOF_section.height, MCU_sx * max_ss_v);
              nb_MCU_Y = intceil(SOF_section.width, MCU_sy * max_ss_h);
              nb_MCU = nb_MCU_X * nb_MCU_Y;


            }

            break;
          }

        case M_DHT:
          {
            // Depending on how the JPEG is encoded, DHT marker may not be
            // repeated for each DHT. We need to take care of the general
            // state where all the tables are stored sequentially
            // DHT size represent the currently read data... it starts as a
            // zero value
            volatile int DHT_size = 0;
            IPRINTF("DHT marker found\r\n");

            COPY_SECTION(&DHT_section.length, 2, movies[0]);
            CPU_DATA_IS_BIGENDIAN(16, DHT_section.length);
            // We've read the size : DHT_size is now 2
            DHT_size += 2;

            //We loop while we've not read all the data of DHT section
            while (DHT_size < DHT_section.length) {

              int loaded_size = 0;
              // read huffman info, DHT size ++
              NEXT_TOKEN(DHT_section.huff_info, movies[0]);
              DHT_size++;

              // load the current huffman table
              HT_index = DHT_section.huff_info & 0x0f;
              HT_type = (DHT_section.huff_info >> 4) & 0x01;

              IPRINTF("Huffman table index is %d\r\n", HT_index);
              IPRINTF("Huffman table type is %s\r\n",
                  HT_type ? "AC" : "DC");

              VPRINTF("Loading Huffman table\r\n");
              tables[HT_type][HT_index] = (huff_table_t *) malloc(sizeof(huff_table_t));
              loaded_size = load_huffman_table_size(movies[0],
                  DHT_section.length,
                  DHT_section.huff_info,
                  tables[HT_type][HT_index]);
              if (loaded_size < 0) {
                VPRINTF("Failed to load Huffman table\n");

                goto clean_end;
              }
              DHT_size += loaded_size;

              IPRINTF("Huffman table length is %d, read %d\r\n", DHT_section.length, DHT_size);
            }

            break;
          }

        case M_SOI:
          {
            IPRINTF("SOI marker found\r\n");
            break;
          }

        case M_EOI:
          {
            IPRINTF("EOI marker found\r\n");
            end_of_file = 1;
            break;
          }

        case M_SOS:
          {
            IPRINTF("SOS marker found\r\n");

            COPY_SECTION(&SOS_section, sizeof(SOS_section), movies[0]);
            CPU_DATA_IS_BIGENDIAN(16, SOS_section.length);

            COPY_SECTION(&SOS_component,
                sizeof(SOS_component_t) * SOS_section.n, movies[0]);
            IPRINTF("Scan with %d components\r\n", SOS_section.n);

            /* On ignore les champs Ss, Se, Ah, Al */
            SKIP(3, movies[0]);

            scan_desc.bit_count = 0;
            for (index = 0; index < SOS_section.n; index++) {
              /*index de SOS correspond à l'ordre d'apparition des MCU,
               *  mais pas forcement égal à celui de déclarations des labels*/

              for(index_SOF=0 ; index_SOF < SOF_section.n; index_SOF++) {
                if(SOF_component[index_SOF].index==SOS_component[index].index){
                  component_order[index]=index_SOF;
                  break;
                }
                if(index_SOF==SOF_section.n-1)
                  VPRINTF("Invalid component label in SOS section");
              }


              scan_desc.pred[index] = 0;
              scan_desc.table[HUFF_AC][index] =
                tables[HUFF_AC][(SOS_component[index].acdc >> 4) & 0x0f];
              scan_desc.table[HUFF_DC][index] =
                tables[HUFF_DC][SOS_component[index].acdc & 0x0f];
            }

            for (index_X = 0; index_X < nb_MCU_X; index_X++) {
              for (index_Y = 0; index_Y < nb_MCU_Y; index_Y++) {
                // TODO : allocate chunks with nb_stream, FRAME_LOOKAHEAD, max_X and max_Y
                // max_X and max_Y are largest dimension of streams.
                // TODO : add stream_id and frame_id infos!
                frame_chunk_t *chunk = chunks + (stream_id * (frame_id[stream_id] % FRAME_LOOKAHEAD) * index_X * index_Y);
                chunk->index = SOS_section.n;
                chunk->x = index_X;
                chunk->y = index_Y;
                chunk->stream_id = stream_id;
                chunk->frame_id  = frame_id[stream_id];

                // Fill MCU structure :
                for (index = 0; index < SOS_section.n; index++)
                {
                  uint32_t component_index = component_order[index];
                  chunk->component_index[index] = component_index;
                  //avoiding unneeded computation
                  int nb_MCU = ((SOF_component[component_index].HV>> 4) & 0xf)
                                * (SOF_component[component_index].HV & 0x0f);

                  for (chroma_ss = 0; chroma_ss < nb_MCU; chroma_ss++)
                  {
                    MCU = MCUs [stream_id] [frame_id[stream_id] % FRAME_LOOKAHEAD] [index_X] [index_Y] [index] [chroma_ss];
                    unpack_block(movies[0], &scan_desc,index, MCU);
                    chunk->DQT_index[index][chroma_ss] = SOF_component[component_index].q_table;
                  }

                }
                chunk->data = (int32_t*) MCUs [stream_id] [frame_id[stream_id] % FRAME_LOOKAHEAD] [index_X] [index_Y];
                chunk->DQT_table = (uint8_t*) &(DQT_table[stream_id][frame_id[stream_id] % FRAME_LOOKAHEAD]);/*[SOF_component[component_index].q_table];*/
                decode(chunk);
              }
            }

            if (screen_refresh() == 1) {
              end_of_file = 1;
            }
            for (HT_index = 0; HT_index < 4; HT_index++) {
              free_huffman_tables(tables[HUFF_DC][HT_index]);
              free_huffman_tables(tables[HUFF_AC][HT_index]);
              tables[HUFF_DC][HT_index] = NULL;
              tables[HUFF_AC][HT_index] = NULL;
            }


            COPY_SECTION(&marker, 2, movies[0]);
            frame_id[stream_id]++;

            break;
          }

        case M_DQT:
          {
            int DQT_size = 0;
            IPRINTF("DQT marker found\r\n");

            COPY_SECTION(&DQT_section.length, 2, movies[0]);
            CPU_DATA_IS_BIGENDIAN(16, DQT_section.length);
            DQT_size += 2;

            while (DQT_size < DQT_section.length) {

              NEXT_TOKEN(DQT_section.pre_quant, movies[0]);
              DQT_size += 1;
              IPRINTF
                ("Quantization precision is %s\r\n",
                 ((DQT_section.
                   pre_quant >> 4) & 0x0f) ? "16 bits" : "8 bits");

              QT_index = DQT_section.pre_quant & 0x0f;
              IPRINTF("Quantization table index is %d\r\n", QT_index);

              IPRINTF("Reading quantization table\r\n");
              COPY_SECTION(DQT_table[stream_id][frame_id[stream_id] % FRAME_LOOKAHEAD][QT_index], 64, movies[0]);
              DQT_size += 64;

            }

            break;
          }

        case M_APP0:
          {
            IPRINTF("APP0 marker found\r\n");

            COPY_SECTION(&jfif_header, sizeof(jfif_header), movies[0]);
            CPU_DATA_IS_BIGENDIAN(16, jfif_header.length);
            CPU_DATA_IS_BIGENDIAN(16, jfif_header.xdensity);
            CPU_DATA_IS_BIGENDIAN(16, jfif_header.ydensity);

            if (jfif_header.identifier[0] != 'J'
                || jfif_header.identifier[1] != 'F'
                || jfif_header.identifier[2] != 'I'
                || jfif_header.identifier[3] != 'F') {
              VPRINTF("Not a JFIF file\r\n");
            }

            break;
          }

        case M_COM:
          {
            IPRINTF("COM marker found\r\n");
            uint16_t length;
            char * comment;
            COPY_SECTION(&length, 2, movies[0]);
            CPU_DATA_IS_BIGENDIAN(16, length);
            comment = (char *) malloc (length - 2);
            COPY_SECTION(comment, length - 2, movies[0]);
            IPRINTF("Comment found : %s\r\n", comment);
            free(comment);
            break;
          }





        default:
          {
            IPRINTF("Unsupported token: 0x%x\r\n", marker[1]);
            skip_segment(movies[0]);
            break;
          }
      }
    } else {
      VPRINTF("Invalid marker, expected SMS (0xff), got 0x%lx (second byte is 0x%lx) \n", marker[0], marker[1]);
    }
  }

clean_end:/*
  for (int i = 0 ; i < 3 ; i++) {
    if (YCbCr_MCU[i] != NULL) {
      free(YCbCr_MCU[i]);
      YCbCr_MCU[i] = NULL;
    }
    if (YCbCr_MCU_ds[i] != NULL) {
      free(YCbCr_MCU_ds[i]);
      YCbCr_MCU_ds[i] = NULL;
    }
  }
  if (RGB_MCU != NULL) {
    free(RGB_MCU);
    RGB_MCU = NULL;
  }*/
  for (HT_index = 0; HT_index < 4; HT_index++) {
    free_huffman_tables(tables[HUFF_DC][HT_index]);
    free_huffman_tables(tables[HUFF_AC][HT_index]);
  }

  screen_exit();
  return 0;



  printf ("Main end\n");

}
