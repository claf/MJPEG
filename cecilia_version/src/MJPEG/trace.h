#ifndef _USE_TRACE_H
#define _USE_TRACE_H

#ifndef likely
# define likely(x)   __builtin_expect(!!(x), 1)
# define unlikely(x) __builtin_expect(!!(x), 0)
#endif

void TRACE_FRAME (int fid, struct timeval b, struct timeval e, char* op);

#endif
