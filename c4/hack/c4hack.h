// c4hack.h -- hack's terminal is curses here (PDCurses, games/pdcurses), in both
// builds. Hack draws by writing to stdout, with termcap's strings to move about
// (hack.termcap.c, which moves with curses here). So what it writes to stdout goes
// to the screen, as a terminal would show it, until the screen ends (settty), and
// to stdout after that. hack/config.h includes this, after the system's headers:
// c4/c4bsd.h has them, and is included before any of hack's own.
#ifndef C4HACK_H
#define C4HACK_H 1
#include <stdio.h>
#include <stdarg.h>

void hk_start(void);             // curses up, and the files hack's install made
void hk_end(void);               // and down
int hk_putchar(int c);
int hk_printf(const char *fmt, ...);
int hk_vprintf(const char *fmt, va_list ap);
int hk_puts(const char *s);
int hk_fputs(const char *s, FILE *f);
int hk_fflush(FILE *f);
int hk_getchar(void);
void hk_move(int x, int y);      // to column x, row y, from 0
void hk_clear(int what);         // 0 the screen, 1 to the end of the line, 2 to its end
void hk_standout(int on);
void hk_bell(void);

// glibc has a getdate of its own, and curses these
#define getdate hack_getdate
#define move hack_move
#define timeout hack_timeout
#define unctrl hack_unctrl
#define delay_output hack_delay_output

#undef putchar
#undef getchar
#define putchar(c) hk_putchar(c)
#define printf hk_printf
#define vprintf hk_vprintf
#define puts(s) hk_puts(s)
#define fputs(s, f) hk_fputs((s), (f))
#define fflush(f) hk_fflush(f)
#define getchar() hk_getchar()
#endif
