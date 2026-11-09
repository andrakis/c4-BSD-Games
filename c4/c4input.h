// c4input.h -- for a curses game that waits on stdin itself, with poll and read:
// the same waits and the same key, from curses, which has the keys here. Include
// it after the game's own includes. c4bsd-curses.c has the functions.
int bsdc_poll_key(int ms);
int bsdc_read_key(char *c);
#undef poll
#undef read
#define poll(set, n, ms) bsdc_poll_key(ms)
#define read(fd, p, n) bsdc_read_key((char *)(p))
#ifndef INFTIM
#define INFTIM (-1)
#endif
