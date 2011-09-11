#ifndef _USE_TRACE_GTG_H
#define _USE_TRACE_GTG_H

#include <GTG.h>

void mjpeg_gtg_init ();
void doEvent(char* op, int value);
void doVar(int value);
void doState(char* state);
void linkStart(char* thr, int frame_id);
void linkEnd(int frame_id);

#endif
