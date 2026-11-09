// err.h -- BSD's, in c4bsd.c. gcc's build has glibc's.
#ifndef C4BSD_ERR_H
#define C4BSD_ERR_H 1
#include <stdarg.h>
void err(int code, const char *fmt, ...);
void errx(int code, const char *fmt, ...);
void warn(const char *fmt, ...);
void warnx(const char *fmt, ...);
void vwarn(const char *fmt, va_list ap);
void vwarnx(const char *fmt, va_list ap);
#endif
