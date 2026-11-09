// signal.h -- C4IX's libc has no signals. c4bsd.h gives both builds its own
// signal(), so these are only the names. gcc's build has glibc's.
#ifndef C4BSD_SIGNAL_H
#define C4BSD_SIGNAL_H 1
typedef void (*sig_t)(int);
typedef unsigned int sigset_t;
#define SIG_DFL ((sig_t)0)
#define SIG_IGN ((sig_t)1)
#define SIG_ERR ((sig_t)-1)
enum { SIGHUP = 1, SIGINT = 2, SIGQUIT = 3, SIGILL = 4, SIGABRT = 6, SIGFPE = 8, SIGKILL = 9,
       SIGBUS = 10, SIGSEGV = 11, SIGPIPE = 13, SIGALRM = 14, SIGTERM = 15, SIGSTOP = 17,
       SIGTSTP = 18, SIGCONT = 19, SIGCHLD = 20, SIGTTIN = 21, SIGTTOU = 22, SIGWINCH = 28 };
struct sigaction { sig_t sa_handler; sigset_t sa_mask; int sa_flags; };
enum { SIG_BLOCK = 0, SIG_UNBLOCK = 1, SIG_SETMASK = 2 };
#endif
