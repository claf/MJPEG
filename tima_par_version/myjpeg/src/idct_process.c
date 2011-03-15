#include <pthread.h>
#include <dpn.h>

#include "idct.h"
#include "mjpeg.h"
#include "jpeg.h"

int idct_process(Channel *c[2])
{
   int32_t YCbCr_Frequency[MCU_sx * MCU_sy];
   uint8_t YCbCr_Pixel[MCU_sx * MCU_sy];

   while (1) {
      channelRead(c[0],
                  (unsigned char *)YCbCr_Frequency,
                  sizeof(YCbCr_Frequency));

      idct(YCbCr_Frequency, YCbCr_Pixel);

      channelWrite(c[1],
                   (unsigned char *)YCbCr_Pixel,
                   sizeof(YCbCr_Pixel));
   }

   return 0;
}
