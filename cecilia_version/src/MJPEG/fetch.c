#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>

#include "MJPEG.h"
#include "huffman.h"
#include "unpack_block.h"
#include "define_common.h"
#include "skip_segment.h"

#include "timing.h"

DECLARE_DATA{
  //int foo;
};

#include "cecilia.h"

/* Global definition : */
int frame_lookahead = 5;
uint8_t nb_streams = 0; 
volatile uint8_t end_of_file = 0;
volatile uint8_t termination = 0;
#ifdef MJPEG_USES_TIMING
time_mjpeg_t *mjpeg_time_table; //[6] = {{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0}};
#endif

// Global Surfaces structures :
SDL_Surface **Surfaces_normal[MAX_STREAM];
SDL_Surface **Surfaces_resized;

// Global stream table :
stream_info_t* streams;

// Global info for SDL_Surface resizing :
shift_t* decalage;
shift_t* resize_Factors;
uint8_t position[MAX_STREAM];

// Global frame achievement table (nb_chunk already treated) :
uint32_t* Achievements[MAX_STREAM];

// Global black & white or color decode :
uint32_t color = 1;

// Global last frame that has been (or should have been) printed on screen :
volatile int32_t last_frame_id;

// Global frame id table currently decoding :
int32_t* in_progress;

/* Intern declarations : */


typedef int32_t unit_table_t[4][64];
typedef unit_table_t mcu_table_t[MAX_MCU_X][MAX_MCU_Y][4];
mcu_table_t *MCUs[MAX_STREAM];
uint8_t (*DQT_table[MAX_STREAM])[4][64]; //[FRAME_LOOKAHEAD][4][64];


volatile int32_t nb_ftp; // number of frame to process.
uint8_t HT_type = 0;
uint8_t HT_index = 0;
int chroma_ss;
uint16_t nb_MCU = 0, nb_MCU_X = 0, nb_MCU_Y = 0;
uint8_t component_order[4];
int *frame_id;
FILE **movies;
uint8_t *stream_init;
uint32_t framerate = 0;
frame_chunk_t *chunks;
SOF_component_t SOF_component[3];
DHT_section_t DHT_section;
SOF_section_t SOF_section;
SOS_section_t SOS_section;
SOS_component_t SOS_component[3];
scan_desc_t scan_desc = { 0, 0, {}, {} };
huff_table_t *tables[2][4] = {
  {NULL , NULL, NULL, NULL} ,
  {NULL , NULL, NULL, NULL}
};

/* Intern functions : */
static void surfaces_init ();
static void factors_init ();
static void skip_frame (FILE* movie);
static int read_1_byte (FILE* movie);
static unsigned int read_2_bytes (FILE* movie);

static void usage(char *progname) {
  printf("Usage : %s [options] <mjpeg_file_1> [<mjpeg_file_2> ...]\n", progname);
  printf("Options list :\n");
  printf("\t-f <framerate> : set framerate for the movie\n");
  printf("\t-b <buffer_size> : set buffer size (frame loolahead) for the movie\n");
  printf("\t-c : decode streams in black and white format\n");
  printf("\t-h : display this help and exit\n");
}

int METHOD(entry, main)(void *_this, int argc, char** argv)
{
  PXKAAPI("Fetch start\n");
#ifdef MJPEG_USES_TIMING
  tick_t t1, t2;
  GET_TICK(t1);
#endif

  /* TODO : dynamic alloc :
   * Streams 
   * Achievements
   * MCUs
   */

  pthread_t thid;
  uint8_t marker[2];
  uint8_t QT_index = 0;
  uint16_t max_ss_h = 0 , max_ss_v = 0;
  uint8_t index = 0, index_X, index_Y, index_SOF;
  uint32_t YH = 0, YV = 0;
  //uint32_t CrH = 0, CrV = 0, CbH = 0, CbV = 0;
  uint32_t screen_init_needed;
  jfif_header_t jfif_header;
  DQT_section_t DQT_section;
  int stream_id = 0;
  int noskip = 1;
  int nb_threads = kaapi_getconcurrency() + 1;
  int32_t *MCU = NULL;
  struct timespec time;

  time.tv_sec = 0;
  time.tv_nsec = 200; 
  last_frame_id = -1;
  screen_init_needed = 1;

  /* Set a global per thread identifier */
  if (unlikely (tid == -1))
    tid = kaapi_get_self_kid ();

#ifdef MJPEG_USES_TIMING
  mjpeg_time_table = (time_mjpeg_t*) malloc (sizeof (time_mjpeg_t) * nb_threads);
  for (int i = 0; i < nb_threads; i++)
  {
    mjpeg_time_table[i].twait = 0;
    mjpeg_time_table[i].tread = 0;
    mjpeg_time_table[i].tdec = 0;
    mjpeg_time_table[i].tcopy = 0;
    mjpeg_time_table[i].trsz = 0;
  }
#endif

  /* Options management : */
  int args = 1;

  if (argc < 2) {
    usage(argv[0]);
    return;
  }

  while (args < argc) {
    if (argv[args][0] ==  '-') {
      switch(argv[args][1]) {
        case 'f':
          if ((argc - args) < 1) {
            usage(argv[0]);
            return;
          }
          framerate = atoi(argv[args+1]);
          args +=2;
          break;
        case 'h':
          usage(argv[0]);
          return;
          break;
        case 'b':
          if ((argc - args) < 1) {
            usage(argv[0]);
            return;
          }
          frame_lookahead = atoi(argv[args+1]);
          args += 2;
          break;
        case 'c':
          color=0;
          args++;
          break;
        default :
          usage(argv[0]);
          return;
      }
    } else {
      break;
    }
  }

  /* If there is no stream args : */
  if ((argc - args) < 1) {
    usage(argv[0]);
    return;
  }
  
  nb_streams = argc - args;
  
  frame_id    = malloc (nb_streams * sizeof (int));
  movies      = malloc (nb_streams * sizeof (FILE*));
  stream_init = malloc (nb_streams * sizeof (uint8_t));
  streams     = malloc (nb_streams * sizeof (stream_info_t));
  chunks      = malloc (nb_streams * frame_lookahead * MAX_MCU_X * MAX_MCU_Y * sizeof (frame_chunk_t));
  
  // Allocation for streams FILE table :
  for (int i = 0; i < nb_streams; i++)
  { 
    frame_id[i] = 0;
    movies[i] = NULL;
    stream_init[i] = 1;
    if ((movies[i] = fopen(argv[args], "r")) == NULL) {
      perror(strerror(errno));
      exit(-1);
    }
    args++;
  }

  /* Init Management : */
#ifdef MJPEG_USES_GTG
  gtg_init ();
#ifdef MJPEG_TRACE_FRAME
  gtg_frameTrace_init ();
#endif
#ifdef MJPEG_TRACE_THREAD
  gtg_threadTrace_init ();
#endif
#endif
  surfaces_init ();
  factors_init ();

  for (int i = 0; i < nb_streams; i++)
  {
    MCUs[i] = (void *) malloc (sizeof (int32_t[MAX_MCU_X][MAX_MCU_Y][4][4][64]) * frame_lookahead);
    DQT_table[i] = (void *) malloc (sizeof (uint8_t[4][64]) * frame_lookahead);
  }


  /* TODO : read begining of every files to find max_X and max_Y use
   * skip_frame to obtain movies sizes and fill position, decalage and
   * resize tables. */
  // TODO TRACE FRAME :
  //traceInit ();

  nb_ftp = frame_lookahead;
  /* Actual computation : */
  int elem_read;
  while ((termination == 0) || (end_of_file == 0)) {
    // TODO TRACE :
    // traceThread WAIT START
#ifdef MJPEG_TRACE_THREAD
    doState ("Wa", tid);
#endif
#ifdef MJPEG_USES_TIMING
    GET_TICK(t2);
    mjpeg_time_table[0].tread += TICK_RAW_DIFF (t1,t2);
#endif 

    // while no frame to process :
    // find a better way!
    while (nb_ftp <= 0) {
#ifdef MJPEG_TRACE_THREAD
      doVar (c2x_workqueue_size(&work.wq));
#endif
      nanosleep (&time, NULL); 
      if (end_of_file != 0)
      {
nbftp:
        while(nb_ftp < frame_lookahead) {}
        printf ("SKIPLINE202 %d fetch at the end on %d buffer size\n", nb_ftp, frame_lookahead);
        termination = 1;
        goto clean_end;
      }

      if (termination == 1)
      {
        sleep (1);
        goto clean_end;
      }
      // TODO TRACE : 
      // threadEventTrace FETCH STOP (start exec and stop).

    }
#ifdef MJPEG_TRACE_THREAD
    doState ("Re", tid);
#endif
#ifdef MJPEG_USES_TIMING
    GET_TICK(t1);
    mjpeg_time_table[0].twait += TICK_RAW_DIFF (t2,t1);
#endif 


    /* Don't skip first frame because of SDL init : */
    if (noskip == 1)
      goto noskip;

    // if it's the first stream (in order to drop a frame for every stream) and
    // if we need to drop the frame, then drop it for every stream!
    if ((stream_id == 0) && (frame_id[stream_id] <= last_frame_id)) {
      while(frame_id[stream_id] <= last_frame_id) {
        for (int s = 0; s < nb_streams; s++) {
          //PFRAME("Skipping frame %d for stream %d\n", 1, frame_id[s], s);  
          PFETCH("Skipping frame %d for stream %d\n", frame_id[s], s);  
          skip_frame (movies[s]);
          if (end_of_file != 0)
            goto nbftp;

          frame_id[s]++;
        }
        __sync_fetch_and_sub(&nb_ftp, 1);
      }
    }

noskip:

    // read next token :
    elem_read = fread(&marker, 2, 1, movies[stream_id]);
    if (elem_read != 1) {
      if (feof(movies[stream_id])) {
        // TODO : atomic increment, the if end_of_file == nb_streams ...
        end_of_file = 1;
        goto nbftp;
      } else {
        fprintf(stderr, "Error reading marker from input file\n");
        exit (1);
      }
    }


    if (marker[0] == M_SMS) {
      // Don't skip a frame we started to decode :
      noskip = 1;

      switch (marker[1]) {
        case M_SOF0:
          {
            IPRINTF("SOF0 marker found for stream %d : frame %d\r\n", stream_id, frame_id[stream_id]);

            COPY_SECTION(&SOF_section, sizeof(SOF_section), movies[stream_id]);
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
                sizeof(SOF_component_t)* SOF_section.n, movies[stream_id]);

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

            /*
            if (SOF_section.n > 1) {
              CrV = SOF_component[1].HV & 0x0f;
              CrH = (SOF_component[1].HV >> 4) & 0x0f;
              CbV = SOF_component[2].HV & 0x0f;
              CbH = (SOF_component[2].HV >> 4) & 0x0f;
            }
            IPRINTF("Subsampling factor = %ux%u, %ux%u, %ux%u\r\n",
                YH, YV, CrH, CrV, CbH, CbV);
            */

            if (stream_init[stream_id] == 1){
              stream_init[stream_id] = 0;

              Achievements[stream_id] = (uint32_t*) malloc (sizeof (uint32_t) * frame_lookahead);

              for (int i = 0; i < frame_lookahead; i++)
              {
                Achievements[stream_id][i] = 0;
              }

              if (screen_init_needed == 1) {
                screen_init_needed = 0;

                PFETCH ("Init : setting Done and Free tables to 0\n");

                Done = (int32_t*) malloc (sizeof(int32_t) * frame_lookahead);
                Free = (int32_t*) malloc (sizeof(int32_t) * frame_lookahead);

                for (int i = 0; i < frame_lookahead; i++)
                {
                  Done[i] = 0;
                  Free[i] = 0;
                }

                // TODO : here 1st and 2nd arguments are not used.
                // TODO : pthread_create now!
                //render_init(SOF_section.width, SOF_section.height, framerate);
                //CALL (render, render, WINDOW_H, WINDOW_W, framerate);
                render_arg_t* render_args = (render_arg_t*) malloc (sizeof (render_arg_t));
                render_args->width =  WINDOW_H;
                render_args->height =  WINDOW_W;
                render_args->framerate =  framerate;

                pthread_create(&thid, NULL, (void*(*)(void*)) &render_body, render_args);

              }

              nb_MCU_X = intceil(SOF_section.height, MCU_sx * max_ss_v);
              nb_MCU_Y = intceil(SOF_section.width, MCU_sy * max_ss_h);
              nb_MCU = nb_MCU_X * nb_MCU_Y;

              streams[stream_id].nb_MCU = nb_MCU;

              //#ifdef MY_DEBUG
              //printf ("Read and Set nb_MCU :  %d\n",nb_MCU);
              //#endif

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

            COPY_SECTION(&DHT_section.length, 2, movies[stream_id]);
            CPU_DATA_IS_BIGENDIAN(16, DHT_section.length);
            // We've read the size : DHT_size is now 2
            DHT_size += 2;

            //We loop while we've not read all the data of DHT section
            while (DHT_size < DHT_section.length) {

              int loaded_size = 0;
              // read huffman info, DHT size ++
              NEXT_TOKEN(DHT_section.huff_info, movies[stream_id]);
              DHT_size++;

              // load the current huffman table
              HT_index = DHT_section.huff_info & 0x0f;
              HT_type = (DHT_section.huff_info >> 4) & 0x01;

              IPRINTF("Huffman table index is %d\r\n", HT_index);
              IPRINTF("Huffman table type is %s\r\n",
                  HT_type ? "AC" : "DC");

              VPRINTF("Loading Huffman table\r\n");
              tables[HT_type][HT_index] = (huff_table_t *) malloc(sizeof(huff_table_t));
              loaded_size = load_huffman_table_size(movies[stream_id],
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
            PFETCH("Start decoding of frame %d for stream %d\n", 1, frame_id[stream_id], stream_id);  

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
            struct timeval tim;
            gettimeofday(&tim, NULL);

#ifdef MJPEG_TRACE_FRAME
            pushFrameState ("De", frame_id[stream_id]);
#endif
            // If you want to skip next ...
            noskip = 0;

            PFETCH ("Processing next frame (%d) for stream_id %d\n",
                frame_id[stream_id], stream_id);

            IPRINTF("SOS marker found\r\n");

            COPY_SECTION(&SOS_section, sizeof(SOS_section), movies[stream_id]);
            CPU_DATA_IS_BIGENDIAN(16, SOS_section.length);

            COPY_SECTION(&SOS_component,
                sizeof(SOS_component_t) * SOS_section.n, movies[stream_id]);
            IPRINTF("Scan with %d components\r\n", SOS_section.n);

            /* On ignore les champs Ss, Se, Ah, Al */
            SKIP(3, movies[stream_id]);

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
                // TODO : allocate chunks with nb_stream, frame_lookahead, max_X and max_Y
                // max_X and max_Y are largest dimension of streams.
                // TODO : add stream_id and frame_id infos!
                //chunks = malloc (sizeof(frame_chunk_t) * nb_streams * frame_lookahead * MAX_MCU_X * MAX_MCU_Y);
                frame_chunk_t *chunk = chunks + (stream_id * frame_lookahead * MAX_MCU_X * MAX_MCU_Y)
                  + ((frame_id[stream_id] % frame_lookahead) * MAX_MCU_X * MAX_MCU_Y) 
                  + (index_X * MAX_MCU_Y)
                  + index_Y;

                chunk->index = SOS_section.n;
                chunk->x = index_X;
                chunk->y = index_Y;
                chunk->stream_id = stream_id;
                chunk->frame_id  = frame_id[stream_id];
                
                in_progress[frame_id[stream_id] % frame_lookahead] = chunk->frame_id;

                // Fill MCU structure :
                for (index = 0; index < SOS_section.n; index++)
                {
                  uint32_t component_index = component_order[index];
                  chunk->component_index[index] = component_index;
                  //avoiding unneeded computation
                  int nb_MCUz = ((SOF_component[component_index].HV>> 4) & 0xf)
                    * (SOF_component[component_index].HV & 0x0f);

                  chunk->nb_MCU[index] = nb_MCUz;

                  //printf("MCU used %d MCU orig %d\n", nb_MCUz, nb_MCU);

                  for (chroma_ss = 0; chroma_ss < nb_MCUz; chroma_ss++)
                  {
                    MCU = MCUs [stream_id] [frame_id[stream_id] % frame_lookahead] [index_X] [index_Y] [index] [chroma_ss];
                    unpack_block(movies[stream_id], &scan_desc,index, MCU);
                    chunk->DQT_index[index][chroma_ss] = SOF_component[component_index].q_table;
                  }

                }
                chunk->data = (int32_t*) MCUs [stream_id] [frame_id[stream_id] % frame_lookahead] [index_X] [index_Y];
                chunk->DQT_table = (uint8_t*) &(DQT_table[stream_id][frame_id[stream_id] % frame_lookahead]);/*[SOF_component[component_index].q_table];*/
#ifdef MJPEG_TRACE_THREAD
                doState ("Fo", tid);
#endif
#ifdef MJPEG_USES_TIMING
    GET_TICK(t2);
    mjpeg_time_table[0].tread += TICK_RAW_DIFF (t1,t2);
#endif 
                CALL (decode, decode, chunk, tim);
#ifdef MJPEG_USES_TIMING
    GET_TICK(t1);
    mjpeg_time_table[0].tread += TICK_RAW_DIFF (t2,t1);
#endif 
#ifdef MJPEG_TRACE_THREAD
                doState ("Re", tid);
#endif
              }
            }

            // TODO : atomic inc?
            frame_id[stream_id]++; // next frame for this stream

            // if it's the last stream, decrement number of frame to process
            if (stream_id == nb_streams -1){
              __sync_fetch_and_sub(&nb_ftp, 1);
            }

            for (HT_index = 0; HT_index < 4; HT_index++) {
              free_huffman_tables(tables[HUFF_DC][HT_index]);
              free_huffman_tables(tables[HUFF_AC][HT_index]);
              tables[HUFF_DC][HT_index] = NULL;
              tables[HUFF_AC][HT_index] = NULL;
            }

            COPY_SECTION(&marker, 2, movies[stream_id]);

            stream_id = (stream_id + 1) % nb_streams; // next stream

            // TODO TRACE :
            // traceThread FORK STOP
            break;
          }

        case M_DQT:
          {
            int DQT_size = 0;
            IPRINTF("DQT marker found\r\n");

            COPY_SECTION(&DQT_section.length, 2, movies[stream_id]);
            CPU_DATA_IS_BIGENDIAN(16, DQT_section.length);
            DQT_size += 2;

            while (DQT_size < DQT_section.length) {

              NEXT_TOKEN(DQT_section.pre_quant, movies[stream_id]);
              DQT_size += 1;
              IPRINTF
                ("Quantization precision is %s\r\n",
                 ((DQT_section.
                   pre_quant >> 4) & 0x0f) ? "16 bits" : "8 bits");

              QT_index = DQT_section.pre_quant & 0x0f;
              IPRINTF("Quantization table index is %d\r\n", QT_index);

              IPRINTF("Reading quantization table\r\n");
              // printf("\nQT_index = %d\n", QT_index);
              COPY_SECTION(DQT_table[stream_id][frame_id[stream_id] % frame_lookahead][QT_index], 64, movies[stream_id]);
              DQT_size += 64;
              //PRINT_DQT(DQT_table[stream_id][frame_id[stream_id] % frame_lookahead][QT_index]);
            }

            break;
          }

        case M_APP0:
          {
            IPRINTF("APP0 marker found\r\n");

            COPY_SECTION(&jfif_header, sizeof(jfif_header), movies[stream_id]);
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
            COPY_SECTION(&length, 2, movies[stream_id]);
            CPU_DATA_IS_BIGENDIAN(16, length);
            comment = (char *) malloc (length - 2);
            COPY_SECTION(comment, length - 2, movies[stream_id]);
            IPRINTF("Comment found : %s\r\n", comment);
            free(comment);
            break;
          }





        default:
          {
            IPRINTF("Unsupported token: 0x%x\r\n", marker[1]);
            skip_segment(movies[stream_id]);
            break;
          }
      }
    } else {
      VPRINTF("Invalid marker, expected SMS (0xff), got 0x%lx (second byte is 0x%lx) \n", marker[0], marker[1]);
    }
  }

clean_end:
  pthread_join (thid,NULL);

  /* Free malloc'ed stuff : */
  for (HT_index = 0; HT_index < 4; HT_index++) {
    free_huffman_tables(tables[HUFF_DC][HT_index]);
    free_huffman_tables(tables[HUFF_AC][HT_index]);
  }

  free (frame_id);
  free (movies);
  free (stream_init);
  free (chunks);
  free (decalage);
  free (resize_Factors);
  free (in_progress);
  free (Done);
  free (Free);
  free (Surfaces_resized);
  for (int i=0; i < nb_streams; i++)
  {
    free (Surfaces_normal[i]);
    free (Achievements[i]);
  }

#ifdef MJPEG_USES_GTG
  gtg_finalize();
#endif

  //printf ("\n#dropped frames : %d\n", dropped);

#ifdef MJPEG_USES_TIMING
  long twork = 0;
  printf ("\n*** MJPEG TIMING INFOS ***\n");

  printf ("Time for thread %d :\t read :%ld\n",0, (long)tick2usec(mjpeg_time_table[0].tread));
  printf ("Time for thread %d :\t wait :%ld\n",0, (long)tick2usec(mjpeg_time_table[0].twait));
  printf ("--------------------------------\n");

  for (int i = 1; i < nb_threads - 1; i++)
  {
    printf ("Time for thread %d :\t decode :%ld\n",i, (long)tick2usec(mjpeg_time_table[i].tdec));
    printf ("Time for thread %d :\t resize :%ld\n",i, (long)tick2usec(mjpeg_time_table[i].trsz));
    twork += (long)tick2usec(mjpeg_time_table[i].tdec) + (long)tick2usec(mjpeg_time_table[i].trsz);
    printf ("--------------------------------\n");
  }

  printf ("Time for thread %d :\t copy :%ld\n",nb_threads - 1, (long)tick2usec(mjpeg_time_table[nb_threads-1].tcopy));
  printf ("--------------------------------\n");

  printf ("\nTotal work : %ld\n", twork);
  printf ("\n*** END ***\n");

  free (mjpeg_time_table);
#endif


  PFETCH ("End\n");
  return;
}

void METHOD (fetch, fetch)(void *_this, struct timeval beg)
{
#ifdef MJPEG_TRACE_THREAD
  linkEnd (frame_id[0] + nb_ftp, tid);
#endif

#ifdef MJPEG_TRACE_FRAME
  popFrameState ("F", frame_id[0] + nb_ftp);
  //TRACE_FRAME (frame_id[0] + nb_ftp , beg, end, "fetch");
#endif

  PFETCH("Adding 1 frame to process, nb_ftp = %d\n", nb_ftp);
  __sync_fetch_and_add(&nb_ftp, 1);
}


static void surfaces_init ()
{
  in_progress = (int32_t*) malloc (sizeof (int32_t) * frame_lookahead);
  Surfaces_resized = (SDL_Surface**) malloc (sizeof (SDL_Surface*) * frame_lookahead);

  for (int frame = 0; frame < frame_lookahead; frame++)
  {
    in_progress[frame] = -1;

    Surfaces_resized[frame] = SDL_CreateRGBSurface(SDL_SWSURFACE, WINDOW_H, WINDOW_W, 32, 0, 0, 0, 0);
    if (Surfaces_resized[frame] == NULL)
      printf ("SDL_CreateRGBSurface ERROR %s:%d\n", __FILE__, __LINE__);
  }
  
  for (int stream = 0; stream < nb_streams; stream++)
  {
    Surfaces_normal[stream] = (SDL_Surface**) malloc (sizeof (SDL_Surface*) * frame_lookahead);
    for (int frame = 0; frame < frame_lookahead; frame++)
    {
      Surfaces_normal[stream][frame] = SDL_CreateRGBSurface(SDL_SWSURFACE, WINDOW_H, WINDOW_W, 32, 0, 0, 0, 0);
      if (Surfaces_normal[stream][frame] == NULL)
        printf ("SDL_CreateRGBSurface ERROR %s:%d\n", __FILE__, __LINE__);
    }
  }
}

static void factors_init ()
{
  // TODO : open every stream, read first SOF0 marker to know h and w
  // and calculate resizing factors, then close every filehandlers.

  decalage = (shift_t*) malloc (sizeof(shift_t) * MAX_STREAM);
  resize_Factors = (shift_t*) malloc (sizeof(shift_t) * MAX_STREAM);

  // decalage and resize_Factors are related, the index is not the
  // stream_id but position onto the final window.
  // [0] is the upper left area (no resize);
  // [2] is the upper right area (no resize);
  // [1] is the main area (zoom x2);
  decalage[0].x = 0;
  decalage[0].y = 144;
  decalage[1].x = 256;
  decalage[1].y = 0;
  decalage[2].x = 0;
  decalage[2].y = 0;

  resize_Factors[0].x = 2;
  resize_Factors[0].y = 2;
  resize_Factors[1].x = 1;
  resize_Factors[1].y = 1;
  resize_Factors[2].x = 1;
  resize_Factors[2].y = 1;

  //decalage[2].x = 0;
  //decalage[2].y = 0;
  //resize_Factors[2].x = 1;
  //resize_Factors[2].y = 1;

  position[0] = 0;
  position[1] = 1;
  position[2] = 2;
}

/*
 * Read the initial marker, which should be SOI.
 * For a JFIF file, the first two bytes of the file should be literally
 * 0xFF M_SOI.  To be more general, we could use next_marker, but if the
 * input file weren't actually JPEG at all, next_marker might read the whole
 * file and then return a misleading error message...
 */
/*
static int first_marker (FILE* movie)
{
  int c1, c2; 

  NEXT_TOKEN(c1, movie);
  NEXT_TOKEN(c2, movie);
  if (c1 != 0xFF)
    abort();
  if (c2 != M_SOI){
    PSKIP("Wrong marker found avec M_SMS : %d\n", c2);
    //abort();
  }
  return c2; 
}
*/
/*
 * Find the next JPEG marker and return its marker code.
 * We expect at least one FF byte, possibly more if the compressor used FFs
 * to pad the file.
 * There could also be non-FF garbage between markers.  The treatment of such
 * garbage is unspecified; we choose to skip over it but emit a warning msg.
 * NB: this routine must not be used after seeing SOS marker, since it will
 * not deal correctly with FF/00 sequences in the compressed image data...
 */

static int next_marker (FILE* movie)
{
  int c;
  int discarded_bytes = 0;

  /* Find 0xFF byte; count and skip any non-FFs. */
  c = read_1_byte(movie);
  if (c == EOF)
    return c;

  while (c != 0xFF) {
    PSKIP ("M_SMS : Found non matching byte !!! byte found : %d\tdiscrded_bytes : %d\n", c, discarded_bytes);
    discarded_bytes++;
    c = read_1_byte(movie);
  }
  /* Get marker code byte, swallowing any duplicate FF bytes.  Extra FFs
   * are legal as pad bytes, so don't count them in discarded_bytes.
   */
  do {
    c = read_1_byte(movie);
    PSKIP ("M_SMS : Found M_SMS matching byte !!! getting next byte  : %d\n", c);
  } while (c == 0xFF);

  if (discarded_bytes != 0) {
    fprintf(stderr, "Warning: garbage data found in JPEG file\n");
  }

  return c;
}

/*
 * Most types of marker are followed by a variable-length parameter segment.
 * This routine skips over the parameters for any marker we don't otherwise
 * want to process.
 * Note that we MUST skip the parameter segment explicitly in order not to
 * be fooled by 0xFF bytes that might appear within the parameter segment;
 * such bytes do NOT introduce new markers.
 */

/* Skip over an unknown or uninteresting variable-length marker */
static void skip_variable (FILE* movie)
{
  unsigned int length;

  /* Get the marker parameter length count */
  length = read_2_bytes(movie);
  /* Length includes itself, so must be at least 2 */
  if (length < 2)
    ERREXIT("Erroneous JPEG marker length");
  length -= 2;
  /* Skip over the remaining bytes */
#ifdef VFETCH
  PFETCH ("Skipping %d bytes\n", length);
#endif
  while (length > 0) {
    (void) read_1_byte(movie);
    length--;
  }
}

/*
 * Process a SOFn marker.
 * This code is only needed if you want to know the image dimensions...
 */

/*
static void process_SOFn (int marker, FILE* movie)
{
  unsigned int length;
  unsigned int image_height, image_width;
  int data_precision, num_components;
  const char * process;
  int ci;

  length = read_2_bytes(movie);      // usual parameter length count

  data_precision = read_1_byte(movie);
  image_height = read_2_bytes(movie);
  image_width = read_2_bytes(movie);
  num_components = read_1_byte(movie);

  switch (marker) {
    case M_SOF0:  process = "Baseline";  break;
    case M_SOF1:  process = "Extended sequential";  break;
    case M_SOF2:  process = "Progressive";  break;
    case M_SOF3:  process = "Lossless";  break;
    case M_SOF5:  process = "Differential sequential";  break;
    case M_SOF6:  process = "Differential progressive";  break;
    case M_SOF7:  process = "Differential lossless";  break;
    case M_SOF9:  process = "Extended sequential, arithmetic coding";  break;
    case M_SOF10: process = "Progressive, arithmetic coding";  break;
    case M_SOF11: process = "Lossless, arithmetic coding";  break;
    case M_SOF13: process = "Differential sequential, arithmetic coding";  break;
    case M_SOF14: process = "Differential progressive, arithmetic coding"; break;
    case M_SOF15: process = "Differential lossless, arithmetic coding";  break;
    default:      process = "Unknown";  break;
  }

#ifdef VFETCH
  PFETCH("JPEG image is %uw * %uh, %d color components, %d bits per sample\n",
      image_width, image_height, num_components, data_precision);
  PFETCH("JPEG process: %s\n", process);
#endif

  if (length != (unsigned int) (8 + num_components * 3))
    ERREXIT("Bogus SOF marker length");

  for (ci = 0; ci < num_components; ci++) {
    (void) read_1_byte(movie);       // Component ID code
    (void) read_1_byte(movie);       // H, V sampling factors
    (void) read_1_byte(movie);       // Quantization table number
  }
}
*/

static void skip_SOS (FILE* movie)
{
  uint8_t marker[2];
  int32_t trash[64];
  COPY_SECTION(&SOS_section, sizeof(SOS_section), movie);
  CPU_DATA_IS_BIGENDIAN(16, SOS_section.length);

  COPY_SECTION(&SOS_component,
      sizeof(SOS_component_t) * SOS_section.n, movie);

  SKIP(3, movie);

  scan_desc.bit_count = 0;
  for (int index = 0; index < SOS_section.n; index++) {
    for(int index_SOF=0 ; index_SOF < SOF_section.n; index_SOF++) {
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

  for (int index_X = 0; index_X < nb_MCU_X; index_X++) {
    for (int index_Y = 0; index_Y < nb_MCU_Y; index_Y++) {
      for (int index = 0; index < SOS_section.n; index++) {
        uint32_t component_index = component_order[index];
        //avoiding unneeded computation
        int nb_MCU = ((SOF_component[component_index].HV>> 4) & 0xf) * (SOF_component[component_index].HV & 0x0f);

        for (chroma_ss = 0; chroma_ss < nb_MCU; chroma_ss++) {
          unpack_block(movie, &scan_desc,index, trash);
        }
      }

    }
  }

  COPY_SECTION(&marker, 2, movie);
}

/*
 * Parse the marker stream until SOS or EOI is seen;
 * display any COM markers.
 * While the companion program wrjpgcom will always insert COM markers before
 * SOFn, other implementations might not, so we scan to SOS before stopping.
 * If we were only interested in the image dimensions, we would stop at SOFn.
 * (Conversely, if we only cared about COM markers, there would be no need
 * for special code to handle SOFn; we could treat it like other markers.)
 */

static void skip_frame (FILE* movie)
{
  int marker;

  /* Expect SOI at start of file */
  /*
     if (first_marker(movie) != M_SOI)
     ERREXIT("Expected SOI marker first");
     */
  /* Scan miscellaneous markers until we reach SOS. */
  for (;;) {
    marker = next_marker(movie);
    switch (marker) {
      /* Note that marker codes 0xC4, 0xC8, 0xCC are not, and must not be,
       * treated as SOFn.  C4 in particular is actually DHT.
       */
      case M_SOI: break;
      case M_SOF0:                /* Baseline */
      case M_SOF1:                /* Extended sequential, Huffman */
      case M_SOF2:                /* Progressive, Huffman */
      case M_SOF3:                /* Lossless, Huffman */
      case M_SOF5:                /* Differential sequential, Huffman */
      case M_SOF6:                /* Differential progressive, Huffman */
      case M_SOF7:                /* Differential lossless, Huffman */
      case M_SOF9:                /* Extended sequential, arithmetic */
      case M_SOF10:               /* Progressive, arithmetic */
      case M_SOF11:               /* Lossless, arithmetic */
      case M_SOF13:               /* Differential sequential, arithmetic */
      case M_SOF14:               /* Differential progressive, arithmetic */
      case M_SOF15:               /* Differential lossless, arithmetic */
                  //process_SOFn(marker, movie);
                  skip_variable(movie);
                  break;

      case M_SOS:                 /* stop before hitting compressed data */
                  skip_SOS (movie);         
                  return;

      case M_EOI:                 /* in case it's a tables-only JPEG stream */
                  return;
      case EOF:
                  return;


      case M_DHT:
                  {
                    // Depending on how the JPEG is encoded, DHT marker may not be
                    // repeated for each DHT. We need to take care of the general
                    // state where all the tables are stored sequentially
                    // DHT size represent the currently read data... it starts as a
                    // zero value
                    volatile int DHT_size = 0;
                    IPRINTF("DHT marker found\r\n");

                    COPY_SECTION(&DHT_section.length, 2, movie);
                    CPU_DATA_IS_BIGENDIAN(16, DHT_section.length);
                    // We've read the size : DHT_size is now 2
                    DHT_size += 2;

                    //We loop while we've not read all the data of DHT section
                    while (DHT_size < DHT_section.length) {

                      int loaded_size = 0;
                      // read huffman info, DHT size ++
                      NEXT_TOKEN(DHT_section.huff_info, movie);
                      DHT_size++;

                      // load the current huffman table
                      HT_index = DHT_section.huff_info & 0x0f;
                      HT_type = (DHT_section.huff_info >> 4) & 0x01;

                      IPRINTF("Huffman table index is %d\r\n", HT_index);
                      IPRINTF("Huffman table type is %s\r\n",
                          HT_type ? "AC" : "DC");

                      VPRINTF("Loading Huffman table\r\n");
                      tables[HT_type][HT_index] = (huff_table_t *) malloc(sizeof(huff_table_t));
                      loaded_size = load_huffman_table_size(movie,
                          DHT_section.length,
                          DHT_section.huff_info,
                          tables[HT_type][HT_index]);
                      if (loaded_size < 0) {
                        VPRINTF("Failed to load Huffman table\n");

                        abort();//goto clean_end;
                      }
                      DHT_size += loaded_size;

                      IPRINTF("Huffman table length is %d, read %d\r\n", DHT_section.length, DHT_size);
                    }

                    break;
                  }

      default:                    /* Anything else just gets skipped */
                  skip_variable(movie);             /* we assume it has a parameter count... */
                  break;
    }
  } /* end loop */
}

/* Read one byte, testing for EOF */
static int 
read_1_byte (FILE* movie)
{
  int c;

  NEXT_TOKEN(c, movie);
  if (c == EOF)
  {
    end_of_file = -1;
  }
    //ERREXIT("Premature EOF in JPEG file");
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
