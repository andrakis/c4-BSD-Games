/* cscreen.c -- tetris's screen.c for curses. Upstream's drives the terminal itself
   through termcap, which C4's systems have no use for: a curses program gets each
   key, and a Terminal gives lines (games/pdcurses). The same functions, and the
   same picture. */
#include <sys/cdefs.h>
#include <curses.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "screen.h"
#include "tetris.h"

static cell curscreen[B_SIZE];          /* non-zero => standout (or otherwise marked) */
static int curscore;
static int isset;                       /* true => terminal is in game mode */

char *SEstr = "", *SOstr = "";            /* scores.c puts them out: to nothing */
extern int Rows, Cols;                  /* current screen size: tetris.c's */

int put(int c) { return addch(c); }

void tscr_init(void) {
    initscr();
    cbreak();
    noecho();
    nonl();
}

void tscr_set(void) {
    Rows = LINES;
    Cols = COLS;
    if (Rows < MINROWS || Cols < MINCOLS) {
        endwin();
        fprintf(stderr, "the screen is too small: it needs %d by %d\n", MINROWS, MINCOLS);
        exit(1);
    }
    isset = 1;
    scr_clear();
}

void scr_end(void) {
    move(Rows - 1, 0);
    refresh();
    endwin();
    isset = 0;
}

void stop(const char *why) {
    if (isset)
        scr_end();
    fprintf(stderr, "aborting: %s\n", why);
    exit(1);
}

void scr_clear(void) {
    clear();
    curscore = -1;
    memset((char *)curscreen, 0, sizeof(curscreen));
}

void scr_update(void) {
    cell *bp, *sp;
    int i, j, so;
    static const struct shape *lastshape;

    /* always leave cursor after last displayed point */
    curscreen[D_LAST * B_COLS - 1] = -1;

    if (score != curscore) {
        move(0, 0);
        printw("Score: %d", score);
        curscore = score;
    }

    /* draw preview of nextpattern */
    if (showpreview && nextshape != lastshape) {
        int t, tr, tc;
        static int r = 5, c = 2;

        lastshape = nextshape;
        attrset(A_NORMAL);
        mvaddstr(r - 1, c - 1, "          ");
        mvaddstr(r, c - 1, "          ");
        mvaddstr(r + 1, c - 1, "          ");
        mvaddstr(r + 2, c - 1, "          ");
        mvaddstr(r - 3, c - 2, "Next shape:");
        attrset(A_REVERSE);
        mvaddstr(r, 2 * c, "  ");
        for (i = 0; i < 3; i++) {
            t = c + r * B_COLS + nextshape->off[i];
            tr = t / B_COLS;
            tc = t % B_COLS;
            mvaddstr(tr, 2 * tc, "  ");
        }
        attrset(A_NORMAL);
    }

    bp = &board[D_FIRST * B_COLS];
    sp = &curscreen[D_FIRST * B_COLS];
    for (j = D_FIRST; j < D_LAST; j++) {
        for (i = 0; i < B_COLS; bp++, sp++, i++) {
            so = *bp;
            if (*sp == so)
                continue;
            *sp = so;
            attrset(so ? A_REVERSE : A_NORMAL);
            mvaddstr(RTOD(j), CTOD(i), "  ");
        }
    }
    attrset(A_NORMAL);
    refresh();
}

void scr_msg(char *s, int set) {
    int l;
    l = strlen(s);
    move(Rows - 2, ((Cols - l) >> 1) - 1);
    if (set)
        addstr(s);
    else
        while (--l >= 0)
            addch(' ');
}
