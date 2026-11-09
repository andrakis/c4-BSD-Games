// sys/time.h -- BSD's, for the structs. gcc's build has glibc's.
#ifndef C4BSD_SYS_TIME_H
#define C4BSD_SYS_TIME_H 1
#include <time.h>
struct timeval { long tv_sec; long tv_usec; };
struct timezone { int tz_minuteswest, tz_dsttime; };
struct itimerval { struct timeval it_interval, it_value; };
enum { ITIMER_REAL = 0 };
#endif
