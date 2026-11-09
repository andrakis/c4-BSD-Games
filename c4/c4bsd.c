// c4bsd.c -- c4bsd.h's functions, in both builds. This unit needs the real time,
// sleep and so on, so it does not keep c4bsd.h's macros for them.
#include "c4bsd.h"
#undef time
#undef sleep
#undef usleep
#undef signal
#undef getenv
#undef ctime

#ifdef C4BSD_GUEST
#include <stdarg.h>
#include <errno.h>
#include "err.h"
#endif

// ---- the generator: xorshift32, whose low bits are as good as its high ones,
// as they must be for random() % 6
static unsigned int rstate = 2463534242u;

void bsdc_srandom(unsigned int seed) {
    rstate = seed * 2654435761u + 2463534242u;
    if (!rstate) rstate = 2463534242u;
}

long bsdc_random(void) {
    unsigned int x;
    x = rstate;
    x = x ^ (x << 13);
    x = x ^ (x >> 17);
    x = x ^ (x << 5);
    rstate = x;
    return (long)(x & 2147483647u);
}

// ---- the clock, and what comes due on it
static int seeded = -1;
static int seed_time;
static int tick_ms;
static int alarm_at;                    // when SIGALRM goes, or 0
static int alarm_every;                 // and again this often (setitimer)
static bsdc_sig handlers[32];

int bsdc_seeded(void) {
    FILE *f;
    int v;
    if (seeded < 0) {
        seeded = 0;
        f = fopen("bsdseed", "r");
        if (f) {
            if (fscanf(f, "%d", &v) == 1) { seed_time = v; seeded = 1; }
            fclose(f);
        }
    }
    return seeded;
}

void bsdc_tick_ms(int ms) {
    bsdc_sig h;
    tick_ms = tick_ms + ms;
    if (alarm_at && tick_ms >= alarm_at) {
        alarm_at = alarm_every ? tick_ms + alarm_every : 0;
        h = handlers[SIGALRM];
        if (h && h != SIG_DFL && h != SIG_IGN) h(SIGALRM);
    }
}

time_t bsdc_time(time_t *t) {
    time_t v;
    if (bsdc_seeded()) {
        v = (time_t)(seed_time + tick_ms / 1000);
        if (t) *t = v;
        return v;
    }
    return time(t);
}

unsigned int bsdc_sleep(unsigned int s) {
    if (!bsdc_seeded()) sleep(s);
    bsdc_tick_ms((int)s * 1000);
    return 0;
}

int bsdc_usleep(unsigned int us) {
    if (!bsdc_seeded()) usleep(us);
    bsdc_tick_ms((int)(us / 1000));
    return 0;
}

int bsdc_gettimeofday(struct timeval *tv) {
    tv->tv_sec = (long)bsdc_time((time_t *)0);
    tv->tv_usec = (long)((tick_ms % 1000) * 1000);
    return 0;
}

// ---- signals: only the game's own timers
bsdc_sig bsdc_signal(int n, bsdc_sig h) {
    bsdc_sig old;
    if (n < 0 || n >= 32) return (bsdc_sig)0;
    old = handlers[n];
    handlers[n] = h;
    return old;
}

unsigned int bsdc_alarm(unsigned int s) {
    int left;
    left = alarm_at ? (alarm_at - tick_ms + 999) / 1000 : 0;
    alarm_every = 0;
    alarm_at = s ? tick_ms + (int)s * 1000 : 0;
    return (unsigned int)left;
}

int bsdc_setitimer(const struct itimerval *v, struct itimerval *old) {
    int ms;
    if (old) {
        old->it_interval.tv_sec = alarm_every / 1000;
        old->it_interval.tv_usec = alarm_every % 1000 * 1000;
        old->it_value.tv_sec = alarm_at ? (alarm_at - tick_ms) / 1000 : 0;
        old->it_value.tv_usec = alarm_at ? (alarm_at - tick_ms) % 1000 * 1000 : 0;
    }
    if (!v) return 0;
    ms = (int)v->it_value.tv_sec * 1000 + ((int)v->it_value.tv_usec + 999) / 1000;
    alarm_at = ms ? tick_ms + ms : 0;
    alarm_every = (int)v->it_interval.tv_sec * 1000 + (int)v->it_interval.tv_usec / 1000;
    return 0;
}

// ---- the user
static struct passwd player;

struct passwd *bsdc_getpwuid(int uid) {
    player.pw_name = "player";
    player.pw_passwd = "";
    player.pw_gecos = "player";
    player.pw_dir = ".";
    player.pw_shell = "sh";
    return &player;
}

// a number that seeds generators, like the clock
#undef getpid
int bsdc_getpid(void) { return bsdc_seeded() ? 4242 : getpid(); }

// ---- the environment
char *bsdc_getenv(const char *name) {
    if (bsdc_seeded()) return (char *)0;
    return getenv((char *)name);
}

// ---- data files
static char pathbuf[80];

const char *bsdc_path(const char *name) {
    FILE *f;
    if (name == pathbuf) return name;
    strcpy(pathbuf, "/usr/games/bsdgames/data/");
    strcat(pathbuf, name);
    f = fopen(pathbuf, "r");
    if (f) { fclose(f); return pathbuf; }
    return name;
}

void bsdc_page(const char *path) {
    FILE *f;
    int c;
    f = fopen(path, "r");
    if (!f) { printf("%s: not found\n", path); return; }
    while ((c = getc(f)) != EOF) putchar(c);
    fclose(f);
    fflush(stdout);
}

int bsdc_execl(const char *path, ...) { return -1; }

void bsdc_swapb(void *p, int n) {
    unsigned char *b;
    unsigned char t;
    int i;
    b = (unsigned char *)p;
    for (i = 0; i < n / 2; i++) { t = b[i]; b[i] = b[n - 1 - i]; b[n - 1 - i] = t; }
}

// termcap's own globals, for a game that names them: c4/termcap.h has no termcap
char PC;
char *BC, *UP;

static char ctime_buf[32];

char *bsdc_ctime(const time_t *t) {
    static char *days = "SunMonTueWedThuFriSat";
    static char *months = "JanFebMarAprMayJunJulAugSepOctNovDec";
    struct tm *m;
    time_t v;
    v = *t;
    m = gmtime(&v);
    sprintf(ctime_buf, "%.3s %.3s %2d %02d:%02d:%02d %d\n", days + m->tm_wday * 3, months + m->tm_mon * 3,
            m->tm_mday, m->tm_hour, m->tm_min, m->tm_sec, m->tm_year + 1900);
    return ctime_buf;
}

// ---- BSD's
static char *ln_buf;
static int ln_cap;

char *bsdc_fgetln(FILE *f, size_t *len) {
    int n, c;
    char *p;
    n = 0;
    while ((c = getc(f)) != EOF) {
        if (n + 2 > ln_cap) {
            ln_cap = ln_cap ? ln_cap * 2 : 256;
            p = (char *)realloc(ln_buf, ln_cap);
            if (!p) return NULL;
            ln_buf = p;
        }
        ln_buf[n] = (char)c;
        n = n + 1;
        if (c == '\n') break;
    }
    if (!n) return NULL;
    ln_buf[n] = 0;
    *len = (size_t)n;
    return ln_buf;
}

size_t bsdc_strlcpy(char *to, const char *from, size_t n) {
    size_t i;
    i = 0;
    while (from[i]) {
        if (n && i + 1 < n) to[i] = from[i];
        i = i + 1;
    }
    if (n) to[i < n ? i : n - 1] = 0;
    return i;
}

const char *bsdc_getprogname(void) { return "bsdgame"; }

// ---- no file has any flags to change here
#ifdef C4BSD_GUEST
unsigned int bsdc_swap32(unsigned int v) {
    return (v >> 24) | ((v >> 8) & 0xff00u) | ((v << 8) & 0xff0000u) | (v << 24);
}

int fcntl(int f, int c, ...) { return 0; }

int getw(FILE *f) {
    int v;
    if (fread(&v, sizeof(v), 1, f) != 1) return EOF;
    return v;
}
int putw(int w, FILE *f) {
    return fwrite(&w, sizeof(w), 1, f) == 1 ? 0 : EOF;
}

// writev and readv, one vector at a time, for mille's saved games
#include <sys/uio.h>

int writev(int fd, const struct iovec *v, int n) {
    int i, r, t;
    t = 0;
    for (i = 0; i < n; i++) {
        r = write(fd, (char *)v[i].iov_base, v[i].iov_len);
        if (r < 0) return r;
        t = t + r;
    }
    return t;
}

int readv(int fd, const struct iovec *v, int n) {
    int i, r, t;
    t = 0;
    for (i = 0; i < n; i++) {
        r = read(fd, (char *)v[i].iov_base, v[i].iov_len);
        if (r < 0) return r;
        t = t + r;
        if (r < v[i].iov_len) break;
    }
    return t;
}
#endif

// ---- the err family, on C4's systems. gcc's build has glibc's.
#ifdef C4BSD_GUEST
void vwarn(const char *fmt, va_list ap) {
    int e;
    e = errno;
    fprintf(stderr, "%s: ", bsdc_getprogname());
    if (fmt) { vfprintf(stderr, (char *)fmt, ap); fprintf(stderr, ": "); }
    fprintf(stderr, "%s\n", strerror(e));
}

void vwarnx(const char *fmt, va_list ap) {
    fprintf(stderr, "%s: ", bsdc_getprogname());
    if (fmt) vfprintf(stderr, (char *)fmt, ap);
    fprintf(stderr, "\n");
}

void warn(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vwarn(fmt, ap);
    va_end(ap);
}

void warnx(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vwarnx(fmt, ap);
    va_end(ap);
}

void err(int code, const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vwarn(fmt, ap);
    va_end(ap);
    exit(code);
}

void errx(int code, const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vwarnx(fmt, ap);
    va_end(ap);
    exit(code);
}
#endif
