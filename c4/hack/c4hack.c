// c4hack.c -- hack's terminal, on curses (c4hack.h). What hack writes to stdout
// is put on the screen as a terminal shows it: a newline goes to the start of the
// next line, a carriage return to the start of this one, a backspace back one,
// and the bell rings. Hack moves the cursor itself (hack.termcap.c). Nothing is
// drawn until hack flushes stdout or waits for a key, as on a terminal.
//
// Hack also keeps its levels, its scores and its lock in files in the current
// directory, which must be one it can write (in C4IX, /tmp): its install made
// the two that must be there before it starts, and this makes them.
#include "c4bsd.h"
#include <curses.h>
#include <fcntl.h>
#include "c4hack.h"
#undef move
#undef timeout
#undef unctrl
#undef delay_output
#undef putchar
#undef getchar
#undef printf
#undef vprintf
#undef puts
#undef fputs
#undef fflush

static int on;                          // the screen is up

static void hk_make(const char *name) {
    int fd;
    fd = open(name, O_RDWR);
    if (fd < 0) fd = open(name, O_RDWR | O_CREAT, 0660);
    if (fd < 0) {
        fprintf(stderr, "hack: can't make %s here. It keeps its levels and scores in\n"
            "the current directory, which must be one it can write to, such as /tmp.\n", name);
        exit(1);
    }
    close(fd);
}

void hk_start(void) {
    hk_make("record");
    hk_make("perm");
    if (on) return;
    initscr();
    cbreak();
    noecho();
    nonl();
    on = 1;
}

void hk_end(void) {
    if (!on) return;
    refresh();
    endwin();
    on = 0;
}

int hk_putchar(int c) {
    int y;
    int x;
    c = c & 255;
    if (!on) return putchar(c);
    getyx(stdscr, y, x);
    if (c == '\n') {
        if (y + 1 < LINES) move(y + 1, 0);
        else move(y, 0);
    } else if (c == '\r') {
        move(y, 0);
    } else if (c == '\b') {
        if (x > 0) move(y, x - 1);
    } else if (c == 7) {
        beep();
    } else if (c == '\t') {
        x = (x + 8) & ~7;
        if (x > COLS - 1) x = COLS - 1;
        move(y, x);
    } else if (c >= ' ') {
        addch((chtype)c);
    }
    return c;
}

static void hk_put(const char *s) {
    while (*s) {
        hk_putchar(*s);
        s++;
    }
}

int hk_vprintf(const char *fmt, va_list ap) {
    char buf[1024];
    if (!on) return vprintf(fmt, ap);
    vsprintf(buf, fmt, ap);
    hk_put(buf);
    return (int)strlen(buf);
}

int hk_printf(const char *fmt, ...) {
    va_list ap;
    int n;
    va_start(ap, fmt);
    n = hk_vprintf(fmt, ap);
    va_end(ap);
    return n;
}

int hk_puts(const char *s) {
    if (!on) return puts(s);
    hk_put(s);
    hk_putchar('\n');
    return 0;
}

int hk_fputs(const char *s, FILE *f) {
    if (!on || f != stdout) return fputs(s, f);
    hk_put(s);
    return 0;
}

int hk_fflush(FILE *f) {
    if (!on || f != stdout) return fflush(f);
    refresh();
    return 0;
}

int hk_getchar(void) {
    if (!on) return getchar();
    refresh();
    return bsdc_getkey();
}

void hk_move(int x, int y) {
    if (!on) return;
    if (y >= LINES) y = LINES - 1;
    if (x >= COLS) x = COLS - 1;
    if (x < 0) x = 0;
    if (y < 0) y = 0;
    move(y, x);
}

void hk_clear(int what) {
    if (!on) return;
    if (what == 0) erase();
    else if (what == 1) clrtoeol();
    else clrtobot();
}

void hk_standout(int b) {
    if (!on) return;
    if (b) standout();
    else standend();
}

void hk_bell(void) {
    if (on) beep();
}
