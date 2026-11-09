// c4bsd-curses.c -- the curses games' link between c4bsd.c's clock and PDCurses's.
// Time passes for a game while curses waits for a key, so that is when its alarm
// goes off. c4bsd.h makes initscr this, in every unit that sees curses.h.
//
// And some games read keys from stdin themselves, under curses's settings, where
// the keys are curses's here. Reading stdin paints nothing, so these don't go
// through getch, which would refresh stdscr: atc's windows would be painted over.
#include "c4bsd.h"
#include <curspriv.h>
#include "c4input.h"
#undef poll
#undef read
#undef getchar

extern void (*pdc4_tick)(int ms);       // games/pdcurses/c4/pdcc4.c

WINDOW *bsdc_initscr(void);
#undef initscr
extern WINDOW *initscr(void);

WINDOW *bsdc_initscr(void) {
    pdc4_tick = bsdc_tick_ms;
    return initscr();
}

// The next key, blocking while time passes, as a byte. Arrows and such aren't bytes.
static int next_key(void) {
    int k;
    for (;;) {
        while (!PDC_check_key())
            napms(50);
        k = PDC_get_key();
        if (k == '\r') return '\n';
        if (k >= 0 && k < 256) return k;
    }
}

int bsdc_getkey(void) {
    if (!stdscr || isendwin()) return getchar();
    return next_key();
}

int bsdc_read_key(char *p) {
    *p = (char)next_key();
    return 1;
}

int bsdc_readk(int fd, void *p, int n) {
    if (fd != 0 || !stdscr || isendwin()) return read(fd, p, n);
    return bsdc_read_key((char *)p);
}

// c4input.h's: poll for a key. 1 with one waiting, 0 if ms went by first. -1 for
// ms is no limit.
int bsdc_poll_key(int ms) {
    while (!PDC_check_key()) {
        if (ms == 0) return 0;
        napms(50);
        if (ms > 0) {
            ms = ms - 50;
            if (ms < 0) ms = 0;
        }
    }
    return 1;
}
