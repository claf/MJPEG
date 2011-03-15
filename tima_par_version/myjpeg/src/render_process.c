#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <dpn.h>

#include "mjpeg.h"
#include "jpeg.h"
#include "screen.h"
#include "utils.h"
#include "upsampler.h"
#include "conv.h"

const uint32_t framerate = 25;
const uint32_t color = 1;

int render_process(Channel *c[NB_IDCT + 1])
{
   uint8_t idct_index = 0;
   uint16_t width = 0, height = 0;
   uint16_t nb_MCU_X = 0, nb_MCU_Y = 0;
   uint16_t max_ss_h = 0, max_ss_v = 0;
   uint16_t hv[3];
   uint8_t  index = 0, i, j;
   uint8_t  components;
   uint8_t  *YCbCr_MCU_ds[3];
   uint8_t  *YCbCr_MCU_us[3];
   uint32_t *RGB_MCU;

#ifdef PROGRESS
   char progress_tab[4] = { '/', '-', '\\', '|' };
   uint32_t imageCount = 1;
   uint32_t block_index = 0;
#endif

   channelRead(c[0], (unsigned char *)&width, sizeof(uint16_t));
   channelRead(c[0], (unsigned char *)&height, sizeof(uint16_t));
   channelRead(c[0], (unsigned char *)&nb_MCU_X, sizeof(uint16_t));
   channelRead(c[0], (unsigned char *)&nb_MCU_Y, sizeof(uint16_t));
   channelRead(c[0], (unsigned char *)&max_ss_h, sizeof(uint16_t));
   channelRead(c[0], (unsigned char *)&max_ss_v, sizeof(uint16_t));
   channelRead(c[0], (unsigned char *)&components, sizeof(uint16_t));

   screen_init(width, height, framerate);

   for (index = 0; index < components; index++) {
      YCbCr_MCU_ds[index] = malloc(MCU_sx * MCU_sy * max_ss_h * max_ss_v);
      YCbCr_MCU_us[index] = malloc(MCU_sx * MCU_sy * max_ss_h * max_ss_v);
   }
   RGB_MCU = malloc(MCU_sx * MCU_sy * max_ss_h * max_ss_v * sizeof(int32_t));

   while (1) {
      uint8_t hv[3];
      channelRead(c[0], hv, sizeof(hv));
      for (i = 0; i < nb_MCU_X; i++) {
         for (j = 0; j < nb_MCU_Y; j++) {
            for (index = 0; index < components; index++) {
               channelRead(c[idct_index+1],
                           YCbCr_MCU_ds[index],
                           MCU_sx * MCU_sy * max_ss_h * max_ss_v);
               idct_index = (idct_index + 1) % NB_IDCT;
               upsampler(YCbCr_MCU_ds[index],
                         YCbCr_MCU_us[index],
                         max_ss_h / ((hv[index] >> 4) & 0xf),
                         max_ss_v / ((hv[index]) & 0xf),
                         max_ss_h, max_ss_v);
            }

            if (color && components > 1)
               YCbCr_to_ARGB(YCbCr_MCU_us, RGB_MCU, max_ss_h, max_ss_v);
            else
               YCbCr_to_BaW(YCbCr_MCU_us, RGB_MCU, max_ss_h, max_ss_v);

            screen_cpyrect(j * MCU_sy * max_ss_h, i * MCU_sx * max_ss_v,
                           MCU_sy * max_ss_h, MCU_sx * max_ss_v,
                           RGB_MCU);
         }
      }
      screen_refresh();
   }
   return 0;
}
