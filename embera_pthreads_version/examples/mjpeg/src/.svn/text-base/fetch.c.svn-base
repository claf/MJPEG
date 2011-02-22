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

#include "fetch.h"
#include "utils.h"
#include "mjpeg.h" 

#include "embera.h"

#ifdef DEADLOCK
#include <pthread.h>
#endif

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

/*---- Fetch fire function ----*/
void *fetch_process (void *args) {

	component *comp = retrieveComponent(args);
	int coreID = 0;
	executionSupportDeployment(coreID);

	saveStartTime (comp);
	printf("[%s] component running [NB_IDCT: %ld]\n", comp->componentName, NB_IDCT);
	
#ifdef DEADLOCK
	printf("Thread fetch 0x%.8x has been created\n", (unsigned int)pthread_self());
#endif

	bool dispatch_info = true;
	uint8_t marker[2];
	uint8_t HT_type = 0;
	uint8_t HT_index = 0;
	uint8_t DQT_table[4][64];
	uint8_t index = 0;
	uint8_t QT_index = 0;
	uint8_t to_idct_index = 0;
	uint16_t nb_MCU = 0;
	uint16_t nb_MCU_sx = 0;
	int32_t MCU[64], * FLIT[NB_IDCT];
	uint32_t YH = 0, YV = 0, flit_size = 0, mcu_size = 0;

	MOVIE_DATA;

	jfif_header_t	jfif_header;
	DQT_section_t	DQT_section;
	SOF_section_t	SOF_section;
	SOF_component_t	SOF_component[3];
	DHT_section_t	DHT_section;
	SOS_section_t SOS_section;
	SOS_component_t	SOS_component[3];
	scan_desc_t	scan_desc = {0, 0, {}, {}};
	huff_table_t tables[2][4];

	INITIALIZE_MOVIE_DATA;

	for (HT_index = 0; HT_index < 4; HT_index++) {
		tables[HUFF_DC][HT_index] . table = (uint8_t *) malloc(MAX_SIZE(HUFF_DC));
		if (tables[HUFF_DC][HT_index] . table == NULL) 
			printf ("%s,%d: malloc failed\n", __FILE__, __LINE__);

		tables[HUFF_AC][HT_index] . table = (uint8_t *) malloc(MAX_SIZE(HUFF_AC));
		if (tables[HUFF_AC][HT_index] . table == NULL)  printf ("%s,%d: malloc failed\n", __FILE__, __LINE__);
	} 
	/*---- Actual computation ----*/

	
	int bufferCount =0;
	int msgNotDelivered =0;
	int sendResult;
	int num = 0;

	interface *fetchDispatch = retrieveInterface (comp, "fetchDispatch", "client");
	interface *fetchCompute;
	char *fetchComputeItf = (char *)malloc(sizeof(char *));
	unsigned long sendTimeBefore =0, sendTimeAfter = 0, sendTime = 0, totalSendTime = 0;
	float meanSendTime = 0.0;


	while (!IS_EOF) {
		COPY_SECTION(& marker, 2);
		// STart marker segment
		if (marker[0] == M_SMS) {
			switch (marker[1]) {
				case M_SOF0: {
					IPRINTF("SOF0 marker found\r\n");

					COPY_SECTION(& SOF_section, sizeof (SOF_section));
					DATA_IS_BIGENDIAN(16, SOF_section . length);
					DATA_IS_BIGENDIAN(16, SOF_section . height);
					DATA_IS_BIGENDIAN(16, SOF_section . width);
					//printf("** %ld %ld %ld\n", SOF_section . length, SOF_section . height, SOF_section . width);
					VPRINTF("Data precision = %d\r\n", SOF_section . data_precision);
					VPRINTF("Image height = %d\r\n", SOF_section . height);
					VPRINTF("Image width = %d\r\n", SOF_section . width);
					VPRINTF("%d component%c\r\n", SOF_section . n, (SOF_section . n > 1) ? 's' : ' ');

					COPY_SECTION(& SOF_component, sizeof (SOF_component_t) * SOF_section . n);

					YV = SOF_component[0] . HV & 0x0f;
					YH = (SOF_component[0] . HV >> 4) & 0x0f;

					VPRINTF("Subsampling factor = %lux%lu\r\n", YH, YV);

					/*---- We only do that once, since we suppose all the picture do have the same format ----*/
					if (dispatch_info) {
						nb_MCU_sx = intceil(SOF_section.width, MCU_sx);
						flit_size = YV * nb_MCU_sx + (SOF_section . n - 1) * nb_MCU_sx / YH;
						//printf("[%s] - Flit_size = %ld\n", comp->componentName, flit_size);
						mcu_size = YV * YH + SOF_section . n - 1;

						for (uint32_t i = 0; i < NB_IDCT; i++) {
							FLIT[i] = (int32_t *) malloc (flit_size * 64 * sizeof (int32_t));
							if (FLIT[i] == NULL) printf ("%s,%d: malloc failed\n", __FILE__, __LINE__);
						}

						VPRINTF("Send SOF info to LIBU and IDCT\r\n");

						/* Embera code - Begin */

						message *msgDispatch;

						//char *_buffer = (char *)malloc (functionParametersDefaultSize);
						printf("taille %d\n" ,sizeof(SOF_section));
printf("taille2 %d\n" ,strlen((char *)& SOF_section));
						msgDispatch = createMessage (3000, 
										comp->componentID, 
										fetchDispatch->mbox->ownerID, 
										"setInitialParameters", 
										(char *)& SOF_section,
										//_buffer,
										"R",
										sizeof(SOF_section));
						
						send (msgDispatch, fetchDispatch->mbox);


						//memcpy(_buffer, &YV, sizeof(YV));
						msgDispatch = createMessage (3001, 
										comp->componentID, 
										fetchDispatch->mbox->ownerID, 
										"setInitialParameters", 
										(char *)& YV,
										//_buffer,
										"R",
sizeof(YV));

						send (msgDispatch, fetchDispatch->mbox);


						//memcpy(_buffer, &YH, sizeof(YH));
						msgDispatch = createMessage (3002, 
										comp->componentID, 
										fetchDispatch->mbox->ownerID, 
										"setInitialParameters", 
										(char *)& YH,
										//_buffer,
										"R",
sizeof(YH));

						send (msgDispatch, fetchDispatch->mbox);

						//memcpy(_buffer, &flit_size, sizeof(flit_size));
						msgDispatch = createMessage (3003, 
										comp->componentID, 
										fetchDispatch->mbox->ownerID, 
										"setInitialParameters", 
										(char *)& flit_size,
										//_buffer,
										"R",
sizeof(flit_size));

						send (msgDispatch, fetchDispatch->mbox);

// 						channelWrite (c[0], (unsigned char *) & SOF_section, sizeof (SOF_section_t));
// 						channelWrite (c[0], (unsigned char *) & YV, sizeof (uint32_t));
// 						channelWrite (c[0], (unsigned char *) & YH, sizeof (uint32_t));
// 						channelWrite (c[0], (unsigned char *) & flit_size, sizeof (uint32_t));

						message *msgInitialCompute;

						for (uint32_t i = 1; i <= NB_IDCT; i++) { 
					/* Initialisation de la fonction en face, qui 
					va connaitre la bonne taille des flits */
							sprintf(fetchComputeItf, "fetchCompute%d", i-1); 
							fetchCompute = retrieveInterface(comp, fetchComputeItf, "client");

							//memcpy(_buffer, &flit_size, sizeof(flit_size));
							msgInitialCompute = createMessage(4000,	
											comp->componentID, 
											fetchCompute->mbox->ownerID, 
											"setFlitSize",
											(char *)& flit_size,
											//_buffer,
											"R",
sizeof(flit_size));

							send (msgInitialCompute, fetchCompute->mbox);
						}

// 							channelWrite (c[i], (uint8_t *) & flit_size, sizeof (uint32_t));
						
						/* Embera code - End */
						dispatch_info = false;
					}

					break;
				}

				case M_SOF1: {
					IPRINTF("SOF1 marker found\r\n");
					skip_segment (movie);
					break;
				}

				case M_SOF2: {
					IPRINTF("SOF2 marker found\r\n");
					skip_segment (movie);
					break;
				}

				case M_SOF3: {
					IPRINTF("SOF3 marker found\r\n");
					skip_segment (movie);
					break;
				}

				case M_DHT: {
					IPRINTF("DHT marker found\r\n");

					COPY_SECTION(& DHT_section, sizeof (DHT_section));
					DATA_IS_BIGENDIAN(16, DHT_section . length);

					HT_index = DHT_section . huff_info & 0x0f;
					HT_type = (DHT_section . huff_info >> 4) & 0x01;

					VPRINTF("Huffman table index is %d\r\n", HT_index);
					VPRINTF("Huffman table type is %s\r\n", HT_type ? "AC" : "DC");

					VPRINTF("Loading Huffman table\r\n");
					load_huffman_table (movie, & DHT_section, & tables[HT_type][HT_index]);
					
					break;
				}

				case M_SOF5: {
					IPRINTF("SOF5 marker found\r\n");
					skip_segment (movie);
					break;
				}

				case M_SOF6: {
					IPRINTF("SOF6 marker found\r\n");
					skip_segment (movie);
					break;
				}

				case M_SOF7: {
					IPRINTF("SOF7 marker found\r\n");
					skip_segment (movie);
					break;
				}

				case M_JPG: {
					IPRINTF("JPG marker found\r\n");
					skip_segment (movie);
					break;
				}
	
				case M_SOF9: {
					IPRINTF("SOF9 marker found\r\n");
					skip_segment (movie);
					break;
				}
				case M_SOF10: {
					IPRINTF("SOF10 marker found\r\n");
					skip_segment (movie);
					break;
				}

				case M_SOF11: {
					IPRINTF("SOF11 marker found\r\n");
					skip_segment (movie);
					break;
				}

				case M_DAC: {
					IPRINTF("DAC marker found\r\n");
					skip_segment (movie);
					break;
				}

				case M_SOF13: {
					IPRINTF("SOF13 marker found\r\n");
					skip_segment (movie);
					break;
				}

				case M_SOF14: {
					IPRINTF("SOF14 marker found\r\n");
					skip_segment (movie);
					break;
				}

				case M_SOF15: {
					IPRINTF("SOF15 marker found\r\n");
					skip_segment (movie);
					break;
				}

				case M_RST0: {
					IPRINTF("RST0 marker found\r\n");
					skip_segment (movie);
					break;
				}

				case M_RST1: {
					IPRINTF("RST1 marker found\r\n");
					skip_segment (movie);
					break;
				}

				case M_RST2: {
					IPRINTF("RST2 marker found\r\n");
					skip_segment (movie);
					break;
				}

				case M_RST3: {
					IPRINTF("RST3 marker found\r\n");
					skip_segment (movie);
					break;
				}

				case M_RST4: {
					IPRINTF("RST4 marker found\r\n");
					skip_segment (movie);
					break;
				}

				case M_RST5: {
					IPRINTF("RST5 marker found\r\n");
					skip_segment (movie);
					break;
				}

				case M_RST6: {
					IPRINTF("RST6 marker found\r\n");
					skip_segment (movie);
					break;
				}

				case M_RST7: {
					IPRINTF("RST7 marker found\r\n");
					skip_segment (movie);
					break;
				}
        // Start of Image
				case M_SOI: {
					IPRINTF("SOI marker found\r\n");
					break;
				}
        // End of Image
        // TODO: find a way to stop the computation
				case M_EOI: {
					IPRINTF("EOI marker found\r\n");
					break;
				}

        // Start of Scan part
				case M_SOS: {
					IPRINTF("SOS marker found\r\n");

					COPY_SECTION(& SOS_section, sizeof (SOS_section));
					DATA_IS_BIGENDIAN(16, SOS_section . length);

					COPY_SECTION(& SOS_component, sizeof(SOS_component_t)  * SOS_section . n);
					VPRINTF("Scan with %d components\r\n", SOS_section . n);

					SKIP(SOS_section . n);

					scan_desc . bit_count = 0;
					for (index = 0; index < SOS_section . n; index++) {
						scan_desc . pred[index] = 0;
						scan_desc . table[HUFF_AC][index] = & tables[HUFF_AC][(SOS_component[index] . acdc >> 4) & 0x0f];
						scan_desc . table[HUFF_DC][index] = & tables[HUFF_DC][SOS_component[index] . acdc & 0x0f];
					}

					nb_MCU = intceil (SOF_section . height, MCU_sx) * intceil (SOF_section . width, MCU_sy);
					IPRINTF("%d MCU to unpack\r\n", nb_MCU);

					while (nb_MCU) {
						for (uint32_t step = 0; step < flit_size; step += mcu_size) {
							for (index = 0; index < YV * YH; index++) {
								unpack_block (movie, & scan_desc, 0, MCU);
								iqzz_block (MCU, & FLIT[to_idct_index][(step + index) * 64], DQT_table[SOF_component[0] . q_table]);
							}

							for (index =  1; index < SOF_section . n; index++) {
								unpack_block (movie, & scan_desc, index, MCU);
								iqzz_block (MCU, & FLIT[to_idct_index][(step + (YV * YH + index - 1)) * 64], DQT_table[SOF_component[index] . q_table]);
							}
						}

					// here to send the data to compute to the threads

					/* Embera code - Begin */
						message *msgBuffer;
						//char *buffer = (char *)malloc(sizeof(char *));
						//char *buffer = (char *)malloc(flit_size * 64 * sizeof (int32_t));
						char *buffer = (char *)malloc(functionParametersDefaultSize);
						//buffer = (char *) FLIT[to_idct_index];
						//memcpy(buffer, FLIT[to_idct_index], flit_size * 64 * sizeof (int32_t));
						//memcpy(buffer, FLIT[to_idct_index], functionParametersDefaultSize);
						//channelWrite (c[to_idct_index + 1], (unsigned char *) FLIT[to_idct_index], flit_size * 64 * sizeof (int32_t));
						//printf("[%ld] Buffer length [%d]\n", bufferCount, sizeof(buffer));
						sprintf (fetchComputeItf, "fetchCompute%d", to_idct_index);
						fetchCompute = retrieveInterface (comp, fetchComputeItf, "client");
						msgBuffer = createMessage(
										100000, 
										comp->componentID, 
										fetchCompute->mbox->ownerID, 
										"setBuffer", 
										FLIT[to_idct_index], 
										"R",
										functionParametersDefaultSize);
						
						//printf("### %ld %ld\n", flit_size * 64 * sizeof (int32_t), sizeof (int32_t));
// Time stamping
						sendTimeBefore = getCurrentTime();
/*
						while ((send (msgBuffer, fetchCompute->mbox)) ==-1) {
							//sendResult;
							comp->_obsStruct->sendQty++;
						}
*/
						sendResult = send (msgBuffer, fetchCompute->mbox);
						sendTimeAfter = getCurrentTime();
						sendTime = sendTimeAfter - sendTimeBefore;
						totalSendTime = totalSendTime +sendTime;
						//printf ("[%s] Send time [%ld]\n", comp->componentName, sendTime);
// Time stamping						
						if (sendResult==-1){
							msgNotDelivered++;
						}
						else comp->_obsStruct->sendQty++;
					
						bufferCount++;

					/* Embera code - End */
					//	if (to_idct_index == 2)
					//		return 0;

						to_idct_index = (to_idct_index + 1) % NB_IDCT;
						nb_MCU -= YV * nb_MCU_sx;

					}

					break;
				}

	// Define Quantization Table
				case M_DQT: {
					IPRINTF("DQT marker found\r\n");

					COPY_SECTION(& DQT_section, sizeof (DQT_section));
					DATA_IS_BIGENDIAN(16, DQT_section . length);

					VPRINTF("Quantization precision is %s\r\n", ((DQT_section . pre_quant >> 4) & 0x0f) ? "16 bits" :	"8 bits");

					QT_index = DQT_section . pre_quant & 0x0f;
					VPRINTF("Quantization table index is %d\r\n", QT_index);
				
					VPRINTF("Reading quantization table\r\n");
					COPY_SECTION(DQT_table[QT_index], 64);
					
					break;
				}

				case M_DNL: {
					IPRINTF("DNL marker found\r\n");
					skip_segment (movie);
					break;
				}

				case M_DRI: {
					IPRINTF("DRI marker found\r\n");
					skip_segment (movie);
					break;
				}

				case M_DHP: {
					IPRINTF("DHP marker found\r\n");
					skip_segment (movie);
					break;
				}

				case M_EXP: {
					IPRINTF("EXP marker found\r\n");
					skip_segment (movie);
					break;
				}

				case M_APP0: {
					IPRINTF("APP0 marker found\r\n");
					
					COPY_SECTION(& jfif_header, sizeof (jfif_header));
					DATA_IS_BIGENDIAN(16, jfif_header.length);
					DATA_IS_BIGENDIAN(16, jfif_header.xdensity);
					DATA_IS_BIGENDIAN(16, jfif_header.ydensity);

					if (jfif_header.identifier[0] != 'J' || jfif_header.identifier[1] != 'F' ||
							jfif_header.identifier[2] != 'I' || jfif_header.identifier[3] != 'F') {
						VPRINTF("Not a JFIF file\r\n");
					}

					break;
				}

				case M_APP1: {
					IPRINTF("APP1 marker found\r\n");
					skip_segment (movie);
					break;
				}
										 
				case M_APP2: {
					IPRINTF("APP2 marker found\r\n");
					skip_segment (movie);
					break;
				}

				case M_APP3: {
					IPRINTF("APP3 marker found\r\n");
					skip_segment (movie);
					break;
				}

				case M_APP4: {
					IPRINTF("APP4 marker found\r\n");
					skip_segment (movie);
					break;
				}

				case M_APP5: {
					IPRINTF("APP5 marker found\r\n");
					skip_segment (movie);
					break;
				}

				case M_APP6: {
					IPRINTF("APP6 marker found\r\n");
					skip_segment (movie);
					break;
				}

				case M_APP7: {
					IPRINTF("APP7 marker found\r\n");
					skip_segment (movie);
					break;
				}

				case M_APP8: {
					IPRINTF("APP8 marker found\r\n");
					skip_segment (movie);
					break;
				}

				case M_APP9: {
					IPRINTF("APP9 marker found\r\n");
					skip_segment (movie);
					break;
				}

				case M_APP10: {
					IPRINTF("APP10 marker found\r\n");
					skip_segment (movie);
					break;
				}

				case M_APP11: {
					IPRINTF("APP11 marker found\r\n");
					skip_segment (movie);
					break;
				}

				case M_APP12: {
					IPRINTF("APP12 marker found\r\n");
					skip_segment (movie);
					break;
				}

				case M_APP13: {
					IPRINTF("APP13 marker found\r\n");
					skip_segment (movie);
					break;
				}

				case M_APP14: {
					IPRINTF("APP14 marker found\r\n");
					skip_segment (movie);
					break;
				}

				case M_APP15: {
					IPRINTF("APP14 marker found\r\n");
					skip_segment (movie);
					break;
				}

				case M_JPG0: {
					IPRINTF("JPG0 marker found\r\n");
					skip_segment (movie);
					break;
				}

				case M_JPG13: {
					IPRINTF("JPG13 marker found\r\n");
					skip_segment (movie);
					break;
				}

				case M_COM: {
					IPRINTF("COM marker found\r\n");
					skip_segment (movie);
					break;
				}

				case M_TEM: {
					IPRINTF("TEM marker found\r\n");
					skip_segment (movie);
					break;
				}

				case M_SMS: {
					IPRINTF("SMS marker found\r\n");
					skip_segment (movie);
					break;
				}

				default: {
					IPRINTF("Error or unknown token: 0x%x\r\n", marker[1]);
					skip_segment (movie);
					break;
				}
			}
		}
	}

	int i;
	message *msgEOF;

	for (i=0; i<NB_IDCT; i++) {

		sprintf (fetchComputeItf, "fetchCompute%d", i);
		fetchCompute = retrieveInterface (comp, fetchComputeItf, "client");
 		//char *buffer = (char *)malloc(functionParametersDefaultSize);
		//char *rr = "EOF";
		//memcpy(buffer, rr, 4);
		msgEOF = createMessage(100000, comp->componentID, fetchCompute->mbox->ownerID, "setEOF", "EOF", "R",4);
		sendResult = send (msgEOF, fetchCompute->mbox);
	}

	/*void *executionTime = (void *)malloc(sizeof(unsigned long));
	executionTime = unLongToVoid(getExecutionTime(comp));

	// Observation
	interface *obsItf = retrieveInterface (comp, "introspection", "server");
	message *obsMsg = receive(obsItf->mbox);

	if (compareString(obsMsg->functionName, "getExecutionTime")==0) {

		//void *executionTime = unLongToVoid(getExecutionTime(comp));
		//printf("[%s] Execution Time: %ld\n", comp->componentName, getExecutionTime(comp));


		getAllocatedMemory(comp);

		interface *obsItfObs = retrieveInterface (comp, "introspection", "client");
		message *ansMsg = _createMessage (	9000+comp->componentID, 
							comp->componentID, 
							obsItfObs->mbox->ownerID, 
							"obsAnswer", 
							executionTime, 
							"A",
							sizeof(executionTime));
 		send(ansMsg, obsItfObs->mbox);
	}
*/

	printf(		"[%s] Number of images sent %ld, [to_idct_index: %ld][Messages not delivered: %ld]\n", 
			comp->componentName, 
			bufferCount, 
			to_idct_index, 
			msgNotDelivered);


	meanSendTime = (float)totalSendTime / (float)(comp->_obsStruct->sendQty);
	printf("[%s] Send performed: [%ld :T=%ld] Last send time [%ld] Send mean time [%.3f]\n", 
			comp->componentName, 
			comp->_obsStruct->sendQty,
			totalSendTime,
			sendTime,
			meanSendTime);


	return 0;
}

