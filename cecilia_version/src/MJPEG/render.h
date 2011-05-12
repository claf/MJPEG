#ifndef __RENDER_H__
#define __RENDER_H__

#include<stdint.h>

/*  render_init initializes the screen for JPEG visualization
 *  Parameters are :
 *  	width : width of the screen (image) in pixels
 *  	height : height of the screen (image) in pixels
 *  	framerate : maximum framerate for the screen (for mjpeg). Framerate is in frame/seconds
 */

extern void render_init(uint32_t width, uint32_t height, uint32_t framerate);



/* render_exit cleanly close the screen
*/

extern int render_exit();



/* render refresh the screen. Since copied data are not
 * immediately visible, it is possible to wait until the whole image is copied
 * before refreshing, to smooth the apparition of images
 */

extern int render(void* nothing);

#endif
