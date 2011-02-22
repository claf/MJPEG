/*************************************************************************************
 * Copyright (C) 2007 TIMA Laboratory
 * Author(s) :      Patrice, GERIN patrice.gerin@imag.fr
 *                  Alexandre CHAGOYA alexandre.chagoya-garzon@imag.fr (dol porting) 
 * Bug Fixer(s) :   
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 *************************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <malloc.h>

#include <dpn.h>

#include "fetch.h"
#include "utils.h"
#include "mjpeg.h" 


/*---- Localy used variables ----*/
const uint8_t G_ZZ[64] = {
    0, 1, 8, 16, 9, 2, 3, 10,
    17, 24, 32, 25, 18, 11, 4, 5,
    12, 19, 26, 33, 40, 48, 41, 34,
    27, 20, 13, 6, 7, 14, 21, 28,
    35, 42, 49, 56, 57, 50, 43, 36,
    29, 22, 15, 23, 30, 37, 44, 51,
    58, 59, 52, 45, 38, 31, 39, 46,
    53, 60, 61, 54, 47, 55, 62, 63
};

bool f_dispatch_info = true;
uint8_t f_marker[2];
uint8_t f_HT_type = 0;
uint8_t f_HT_index = 0;
uint8_t f_DQT_table[4][64];
uint8_t f_index = 0;
uint8_t f_QT_index = 0;
uint8_t f_to_idct_index = 0;
uint16_t f_nb_MCU = 0;
uint16_t f_nb_MCU_sx = 0;
int32_t f_MCU[64], * f_FLIT[NB_IDCT];
uint32_t f_YH = 0, f_YV = 0, f_flit_size = 0, f_mcu_size = 0;

MOVIE_DATA;

jfif_header_t	f_jfif_header;
DQT_section_t	f_DQT_section;
SOF_section_t	f_SOF_section;
SOF_component_t	f_SOF_component[3];
DHT_section_t	f_DHT_section;
SOS_section_t f_SOS_section;
SOS_component_t	f_SOS_component[3];
scan_desc_t	f_scan_desc = {0, 0, {}, {}};
huff_table_t f_tables[2][4];

Channel ** c_fetch;

/*---- Fetch fire function ----*/
int fetch_init (Channel * _c) {
  printf("%s\n", __FUNCTION__);
  
  c_fetch = _c;

	INITIALIZE_MOVIE_DATA;

	for (f_HT_index = 0; f_HT_index < 4; f_HT_index++) {
		f_tables[HUFF_DC][f_HT_index] . table = (uint8_t *) malloc(MAX_SIZE(HUFF_DC));
		if (f_tables[HUFF_DC][f_HT_index] . table == NULL) printf ("%s,%d: malloc failed\n", __FILE__, __LINE__);

		f_tables[HUFF_AC][f_HT_index] . table = (uint8_t *) malloc(MAX_SIZE(HUFF_AC));
		if (f_tables[HUFF_AC][f_HT_index] . table == NULL)  printf ("%s,%d: malloc failed\n", __FILE__, __LINE__);
	}
}

int fetch_process () {
  printf("%s\n", __FUNCTION__);
	/*---- Actual computation ----*/
		COPY_SECTION(& f_marker, 2);
    // STart f_marker segment
		if (f_marker[0] == M_SMS) {
  printf("%s\n", __FUNCTION__);
			switch (f_marker[1]) {
				case M_SOF0: {
  printf("%s\n", __FUNCTION__);
					IPRINTF("SOF0 f_marker found\r\n");

					COPY_SECTION(& f_SOF_section, sizeof (f_SOF_section));
					DATA_IS_BIGENDIAN(16, f_SOF_section . length);
					DATA_IS_BIGENDIAN(16, f_SOF_section . height);
					DATA_IS_BIGENDIAN(16, f_SOF_section . width);

					VPRINTF("Data precision = %d\r\n", f_SOF_section . data_precision);
					VPRINTF("Image height = %d\r\n", f_SOF_section . height);
					VPRINTF("Image width = %d\r\n", f_SOF_section . width);
					VPRINTF("%d component%c\r\n", f_SOF_section . n, (f_SOF_section . n > 1) ? 's' : ' ');

					COPY_SECTION(& f_SOF_component, sizeof (SOF_component_t) * f_SOF_section . n);

					f_YV = f_SOF_component[0] . HV & 0x0f;
					f_YH = (f_SOF_component[0] . HV >> 4) & 0x0f;

					VPRINTF("Subsampling factor = %lux%lu\r\n", f_YH, f_YV);

					/*---- We only do that once, since we suppose all the picture do have the same format ----*/
					if (f_dispatch_info) {
  printf("%s\n", __FUNCTION__);
						f_nb_MCU_sx = intceil(f_SOF_section.width, MCU_sx);
						f_flit_size = f_YV * f_nb_MCU_sx + (f_SOF_section . n - 1) * f_nb_MCU_sx / f_YH;
						f_mcu_size = f_YV * f_YH + f_SOF_section . n - 1;

						for (uint32_t i = 0; i < NB_IDCT; i++) {
							f_FLIT[i] = (int32_t *) malloc (f_flit_size * 64 * sizeof (int32_t));
							if (f_FLIT[i] == NULL) printf ("%s,%d: malloc failed\n", __FILE__, __LINE__);
						}

  printf("%s\n", __FUNCTION__);
						VPRINTF("Send SOF info to LIBU and IDCT\r\n");
						channelWrite (c_fetch[0], (unsigned char *) & f_SOF_section, sizeof (SOF_section_t));
						channelWrite (c_fetch[0], (unsigned char *) & f_YV, sizeof (uint32_t));
						channelWrite (c_fetch[0], (unsigned char *) & f_YH, sizeof (uint32_t));
						channelWrite (c_fetch[0], (unsigned char *) & f_flit_size, sizeof (uint32_t));

						for (uint32_t i = 1; i <= NB_IDCT; i++) 
			/* Initialisation de la fonction en face, qui va connaitre la bonne taille des flits */
							channelWrite (c_fetch[i], (uint8_t *) & f_flit_size, sizeof (uint32_t));
						f_dispatch_info = false;
					}

					break;
				}

				case M_SOF1: {
					IPRINTF("SOF1 f_marker found\r\n");
					skip_segment (movie);
					break;
				}

				case M_SOF2: {
					IPRINTF("SOF2 f_marker found\r\n");
					skip_segment (movie);
					break;
				}

				case M_SOF3: {
					IPRINTF("SOF3 f_marker found\r\n");
					skip_segment (movie);
					break;
				}

				case M_DHT: {
					IPRINTF("DHT f_marker found\r\n");

					COPY_SECTION(& f_DHT_section, sizeof (f_DHT_section));
					DATA_IS_BIGENDIAN(16, f_DHT_section . length);

					f_HT_index = f_DHT_section . huff_info & 0x0f;
					f_HT_type = (f_DHT_section . huff_info >> 4) & 0x01;

					VPRINTF("Huffman table f_index is %d\r\n", f_HT_index);
					VPRINTF("Huffman table type is %s\r\n", f_HT_type ? "AC" : "DC");

					VPRINTF("Loading Huffman table\r\n");
					load_huffman_table (movie, & f_DHT_section, & f_tables[f_HT_type][f_HT_index]);
					
					break;
				}

				case M_SOF5: {
					IPRINTF("SOF5 f_marker found\r\n");
					skip_segment (movie);
					break;
				}

				case M_SOF6: {
					IPRINTF("SOF6 f_marker found\r\n");
					skip_segment (movie);
					break;
				}

				case M_SOF7: {
					IPRINTF("SOF7 f_marker found\r\n");
					skip_segment (movie);
					break;
				}

				case M_JPG: {
					IPRINTF("JPG f_marker found\r\n");
					skip_segment (movie);
					break;
				}
	
				case M_SOF9: {
					IPRINTF("SOF9 f_marker found\r\n");
					skip_segment (movie);
					break;
				}
				case M_SOF10: {
					IPRINTF("SOF10 f_marker found\r\n");
					skip_segment (movie);
					break;
				}

				case M_SOF11: {
					IPRINTF("SOF11 f_marker found\r\n");
					skip_segment (movie);
					break;
				}

				case M_DAC: {
					IPRINTF("DAC f_marker found\r\n");
					skip_segment (movie);
					break;
				}

				case M_SOF13: {
					IPRINTF("SOF13 f_marker found\r\n");
					skip_segment (movie);
					break;
				}

				case M_SOF14: {
					IPRINTF("SOF14 f_marker found\r\n");
					skip_segment (movie);
					break;
				}

				case M_SOF15: {
					IPRINTF("SOF15 f_marker found\r\n");
					skip_segment (movie);
					break;
				}

				case M_RST0: {
					IPRINTF("RST0 f_marker found\r\n");
					skip_segment (movie);
					break;
				}

				case M_RST1: {
					IPRINTF("RST1 f_marker found\r\n");
					skip_segment (movie);
					break;
				}

				case M_RST2: {
					IPRINTF("RST2 f_marker found\r\n");
					skip_segment (movie);
					break;
				}

				case M_RST3: {
					IPRINTF("RST3 f_marker found\r\n");
					skip_segment (movie);
					break;
				}

				case M_RST4: {
					IPRINTF("RST4 f_marker found\r\n");
					skip_segment (movie);
					break;
				}

				case M_RST5: {
					IPRINTF("RST5 f_marker found\r\n");
					skip_segment (movie);
					break;
				}

				case M_RST6: {
					IPRINTF("RST6 f_marker found\r\n");
					skip_segment (movie);
					break;
				}

				case M_RST7: {
					IPRINTF("RST7 f_marker found\r\n");
					skip_segment (movie);
					break;
				}
        // Start of Image
				case M_SOI: {
					IPRINTF("SOI f_marker found\r\n");
					break;
				}
        // End of Image
        // TODO: find a way to stop the computation
				case M_EOI: {
					IPRINTF("EOI f_marker found\r\n");
					break;
				}

        // Start of Scan part
				case M_SOS: {
					IPRINTF("SOS f_marker found\r\n");

					COPY_SECTION(& f_SOS_section, sizeof (f_SOS_section));
					DATA_IS_BIGENDIAN(16, f_SOS_section . length);

					COPY_SECTION(& f_SOS_component, sizeof(SOS_component_t)  * f_SOS_section . n);
					VPRINTF("Scan with %d components\r\n", f_SOS_section . n);

					SKIP(f_SOS_section . n);

					f_scan_desc . bit_count = 0;
					for (f_index = 0; f_index < f_SOS_section . n; f_index++) {
						f_scan_desc . pred[f_index] = 0;
						f_scan_desc . table[HUFF_AC][f_index] = & f_tables[HUFF_AC][(f_SOS_component[f_index] . acdc >> 4) & 0x0f];
						f_scan_desc . table[HUFF_DC][f_index] = & f_tables[HUFF_DC][f_SOS_component[f_index] . acdc & 0x0f];
					}

					f_nb_MCU = intceil (f_SOF_section . height, MCU_sx) * intceil (f_SOF_section . width, MCU_sy);
					IPRINTF("%d f_MCU to unpack\r\n", f_nb_MCU);

					while (f_nb_MCU) {
						for (uint32_t step = 0; step < f_flit_size; step += f_mcu_size) {
							for (f_index = 0; f_index < f_YV * f_YH; f_index++) {
								unpack_block (movie, & f_scan_desc, 0, f_MCU);
								iqzz_block (f_MCU, & f_FLIT[f_to_idct_index][(step + f_index) * 64], f_DQT_table[f_SOF_component[0] . q_table]);
							}

							for (f_index =  1; f_index < f_SOF_section . n; f_index++) {
								unpack_block (movie, & f_scan_desc, f_index, f_MCU);
								iqzz_block (f_MCU, & f_FLIT[f_to_idct_index][(step + (f_YV * f_YH + f_index - 1)) * 64], f_DQT_table[f_SOF_component[f_index] . q_table]);
							}
						}

            // here to send the data to compute to the threads
						channelWrite (c_fetch[f_to_idct_index + 1], (unsigned char *) f_FLIT[f_to_idct_index], f_flit_size * 64 * sizeof (int32_t));
						f_to_idct_index = (f_to_idct_index + 1) % NB_IDCT;

						f_nb_MCU -= f_YV * f_nb_MCU_sx;
					}

					break;
				}
        
        // Define Quantization Table
				case M_DQT: {
					IPRINTF("DQT f_marker found\r\n");

					COPY_SECTION(& f_DQT_section, sizeof (f_DQT_section));
					DATA_IS_BIGENDIAN(16, f_DQT_section . length);

					VPRINTF("Quantization precision is %s\r\n", ((f_DQT_section . pre_quant >> 4) & 0x0f) ? "16 bits" :	"8 bits");

					f_QT_index = f_DQT_section . pre_quant & 0x0f;
					VPRINTF("Quantization table f_index is %d\r\n", f_QT_index);
				
					VPRINTF("Reading quantization table\r\n");
					COPY_SECTION(f_DQT_table[f_QT_index], 64);
					
					break;
				}

				case M_DNL: {
					IPRINTF("DNL f_marker found\r\n");
					skip_segment (movie);
					break;
				}

				case M_DRI: {
					IPRINTF("DRI f_marker found\r\n");
					skip_segment (movie);
					break;
				}

				case M_DHP: {
					IPRINTF("DHP f_marker found\r\n");
					skip_segment (movie);
					break;
				}

				case M_EXP: {
					IPRINTF("EXP f_marker found\r\n");
					skip_segment (movie);
					break;
				}

				case M_APP0: {
					IPRINTF("APP0 f_marker found\r\n");
					
					COPY_SECTION(& f_jfif_header, sizeof (f_jfif_header));
					DATA_IS_BIGENDIAN(16, f_jfif_header.length);
					DATA_IS_BIGENDIAN(16, f_jfif_header.xdensity);
					DATA_IS_BIGENDIAN(16, f_jfif_header.ydensity);

					if (f_jfif_header.identifier[0] != 'J' || f_jfif_header.identifier[1] != 'F' ||
							f_jfif_header.identifier[2] != 'I' || f_jfif_header.identifier[3] != 'F') {
						VPRINTF("Not a JFIF file\r\n");
					}

					break;
				}

				case M_APP1: {
					IPRINTF("APP1 f_marker found\r\n");
					skip_segment (movie);
					break;
				}
										 
				case M_APP2: {
					IPRINTF("APP2 f_marker found\r\n");
					skip_segment (movie);
					break;
				}

				case M_APP3: {
					IPRINTF("APP3 f_marker found\r\n");
					skip_segment (movie);
					break;
				}

				case M_APP4: {
					IPRINTF("APP4 f_marker found\r\n");
					skip_segment (movie);
					break;
				}

				case M_APP5: {
					IPRINTF("APP5 f_marker found\r\n");
					skip_segment (movie);
					break;
				}

				case M_APP6: {
					IPRINTF("APP6 f_marker found\r\n");
					skip_segment (movie);
					break;
				}

				case M_APP7: {
					IPRINTF("APP7 f_marker found\r\n");
					skip_segment (movie);
					break;
				}

				case M_APP8: {
					IPRINTF("APP8 f_marker found\r\n");
					skip_segment (movie);
					break;
				}

				case M_APP9: {
					IPRINTF("APP9 f_marker found\r\n");
					skip_segment (movie);
					break;
				}

				case M_APP10: {
					IPRINTF("APP10 f_marker found\r\n");
					skip_segment (movie);
					break;
				}

				case M_APP11: {
					IPRINTF("APP11 f_marker found\r\n");
					skip_segment (movie);
					break;
				}

				case M_APP12: {
					IPRINTF("APP12 f_marker found\r\n");
					skip_segment (movie);
					break;
				}

				case M_APP13: {
					IPRINTF("APP13 f_marker found\r\n");
					skip_segment (movie);
					break;
				}

				case M_APP14: {
					IPRINTF("APP14 f_marker found\r\n");
					skip_segment (movie);
					break;
				}

				case M_APP15: {
					IPRINTF("APP14 f_marker found\r\n");
					skip_segment (movie);
					break;
				}

				case M_JPG0: {
					IPRINTF("JPG0 f_marker found\r\n");
					skip_segment (movie);
					break;
				}

				case M_JPG13: {
					IPRINTF("JPG13 f_marker found\r\n");
					skip_segment (movie);
					break;
				}

				case M_COM: {
					IPRINTF("COM f_marker found\r\n");
					skip_segment (movie);
					break;
				}

				case M_TEM: {
					IPRINTF("TEM f_marker found\r\n");
					skip_segment (movie);
					break;
				}

				case M_SMS: {
					IPRINTF("SMS f_marker found\r\n");
					skip_segment (movie);
					break;
				}

				default: {
					IPRINTF("Error or unknown token: 0x%x\r\n", f_marker[1]);
					skip_segment (movie);
					break;
				}
			}
		}

	return 0;
}

