// c4bsd.h -- what the BSD games need from a Unix that C4's systems are not, in
// both builds: c4lc's (through c4/sys/cdefs.h, which every unit includes) and
// gcc's, the reference (through -include). c4bsd.c has the functions.
//
// Anything a game can tell apart between the two is the same code in both, so
// that a run from the same seed plays the same game on gcc's build and on
// c4lc's: test-bsdgames compares them (c4/tests/run.sh). That is the generator,
// the clock, signals and timers, the user, and the environment.
#ifndef C4BSD_H
#define C4BSD_H 1

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <pwd.h>
#include <termios.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/ttydefaults.h>

// ---- one generator, wherever the game asks. RAND_MAX is what it gives: libc's
// is 32767 and glibc's is not.
long bsdc_random(void);
void bsdc_srandom(unsigned int seed);
#undef RAND_MAX
#define RAND_MAX 2147483647
#define random bsdc_random
#define srandom bsdc_srandom
#define rand() ((int)bsdc_random())
#define srand(s) bsdc_srandom((unsigned int)(s))

// ---- the clock. With a file called bsdseed in the current directory, holding a
// number, time starts there and moves only as the game sleeps: the run is
// repeatable. Without it these are the real thing.
time_t bsdc_time(time_t *t);
unsigned int bsdc_sleep(unsigned int s);
int bsdc_usleep(unsigned int us);
int bsdc_gettimeofday(struct timeval *tv);
int bsdc_seeded(void);
void bsdc_tick_ms(int ms);       // ms have passed: alarms come due
#define time(p) bsdc_time(p)
#define sleep(s) bsdc_sleep(s)
#define usleep(u) bsdc_usleep(u)
#define gettimeofday(tv, tz) bsdc_gettimeofday(tv)

// ---- signals. Nothing sends one but the game's own timers. Those are checked
// when time passes -- a sleep, or curses waiting for a key (c4bsd-curses.c) --
// and the handler is called from there, longjmp and all.
typedef void (*bsdc_sig)(int);
bsdc_sig bsdc_signal(int n, bsdc_sig h);
unsigned int bsdc_alarm(unsigned int s);
int bsdc_setitimer(const struct itimerval *v, struct itimerval *old);
#define signal(n, h) bsdc_signal((n), (bsdc_sig)(h))
#define alarm(s) bsdc_alarm(s)
#define setitimer(w, v, o) bsdc_setitimer((v), (o))
#define sigprocmask(h, s, o) ((void)(o), 0)
#define sigemptyset(s) ((void)(s), 0)
#define sigfillset(s) ((void)(s), 0)
#define sigaddset(s, n) ((void)(s), 0)
#define sigdelset(s, n) ((void)(s), 0)
#define kill(p, s) 0
#define fork() (-1)
#ifndef _PATH_BSHELL
#define _PATH_BSHELL "sh"
#endif
#define sigaction(n, a, o) bsdc_signal((n), (bsdc_sig)(a)->sa_handler)

// ---- the user: one, called player
struct passwd *bsdc_getpwuid(int uid);
#define getpwuid(u) bsdc_getpwuid(u)
#define getpwnam(n) bsdc_getpwuid(0)
#define getlogin() "player"
#define getpid() bsdc_getpid()
int bsdc_getpid(void);
#define getuid() 0
#define geteuid() 0
#define getgid() 0
#define getegid() 0
#define setuid(i) 0
#define setgid(i) 0
#define seteuid(i) 0
#define setegid(i) 0
#define setregid(r, e) 0
#define setreuid(r, e) 0
#define flock(f, o) 0
#define umask(m) 0

// ---- curses: initscr starts the clock (c4bsd-curses.c). And some games read keys
// from stdin themselves, under curses's settings, where curses has the keys.
#define initscr bsdc_initscr
#ifdef BSD_CURSES_GAME
int bsdc_getkey(void);
int bsdc_readk(int fd, void *p, int n);
#undef getchar
#define getchar() bsdc_getkey()
#define read(f, p, n) bsdc_readk((f), (p), (n))
#endif

// ---- the terminal: curses or nothing, so it has no modes, and it is a terminal
#define tcgetattr(f, t) (memset((t), 0, sizeof(*(t))), 0)
#define tcsetattr(f, h, t) 0
#define tcflush(f, q) 0
#define tcdrain(f) 0
#define cfgetospeed(t) B38400
#define cfgetispeed(t) B38400
#define isatty(f) 1

// ---- the environment: in a repeatable run, nothing in it
char *bsdc_getenv(const char *name);
#define getenv(n) bsdc_getenv(n)

// ---- a game's data file: in C4IX's /usr/games/bsdgames/data if it is there, and
// otherwise in the current directory, or for C4DOS on its APPEND path. The bare
// machine and C4DOS have a flat disk with short names, so these are 8.3.
const char *bsdc_path(const char *name);

// A file on stdout, in place of a fork, an exec and the user's $PAGER
void bsdc_page(const char *path);

// ---- what backgammon asks for by name, and no program starts another here
#define EXEC_PATH "backgammon"
#define TEACH_PATH "teachgammon"
int bsdc_execl(const char *path, ...);
#define execl bsdc_execl

// ---- BSD's, that neither libc has
char *bsdc_fgetln(FILE *f, size_t *len);
size_t bsdc_strlcpy(char *to, const char *from, size_t n);
const char *bsdc_getprogname(void);
#define fgetln bsdc_fgetln
#define strlcpy bsdc_strlcpy
#define getprogname bsdc_getprogname

// RCS ids, and what upstream's own <sys/cdefs.h> gives its games
#undef __RCSID
#undef __COPYRIGHT
#define __RCSID(s) extern int bsdc_rcsid
#define __COPYRIGHT(s) extern int bsdc_copyright
#ifndef __dead
#define __dead
#endif

#ifndef OXTABS
#define OXTABS 0
#endif

// Big-endian files, on machines that aren't: in place, as upstream's <sys/endian.h>
void bsdc_swapb(void *p, int n);
#define BE16TOH(x) bsdc_swapb(&(x), 2)
#define BE32TOH(x) bsdc_swapb(&(x), 4)
#define BE64TOH(x) bsdc_swapb(&(x), 8)

// A time as text, in both builds
char *bsdc_ctime(const time_t *t);
#define ctime bsdc_ctime

#ifndef C4BSD_GUEST
#include <arpa/inet.h>
#endif

// atc's own: c4/atc/grammar.c
const char *atc_path(const char *s);

// ---- on C4's systems only: gcc's build has the real ones
#ifdef C4BSD_GUEST
#define F_GETFD 1
#define F_SETFD 2
#define FD_CLOEXEC 1
#define ntohl(x) bsdc_swap32(x)
#define be32toh(x) bsdc_swap32(x)
#define htobe32(x) bsdc_swap32(x)
#define htonl(x) bsdc_swap32(x)
#define ntohs(x) ((unsigned short)((((x) >> 8) & 255) | (((x) & 255) << 8)))
#define htons(x) ntohs(x)
unsigned int bsdc_swap32(unsigned int v);
#define isascii(c) ((unsigned int)(c) < 128)
#define setbuf(f, b) ((void)0)
int getw(FILE *f);
int putw(int w, FILE *f);
int fcntl(int f, int c, ...);
#include <limits.h>
#ifndef ULONG_MAX
#define ULONG_MAX 4294967295u
#endif
#ifndef LONG_MAX
#define LONG_MAX 2147483647
#endif
#define __attribute__(x)
typedef unsigned char u_char;
typedef unsigned short u_short;
typedef unsigned int u_int;
typedef unsigned long u_long;
typedef unsigned char u_int8_t;
typedef unsigned short u_int16_t;
typedef unsigned int u_int32_t;
typedef unsigned long long u_int64_t;
typedef short int16_t;
typedef int int32_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
#endif

#endif
