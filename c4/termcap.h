// termcap.h -- there is none here, for gcc's build as for c4lc's, and a game that
// looks is told so: it goes on without (backgammon's plain board).
#ifndef C4BSD_TERMCAP_H
#define C4BSD_TERMCAP_H 1
#define tgetent(b, n) 0
#define tgetstr(n, a) ((char *)0)
#define tgetflag(n) 0
#define tgetnum(n) (-1)
#define tgoto(c, x, y) ""
#define tputs(s, n, f) 0
#endif
