// sys/poll.h -- BSD's, for the names: c4input.h has what a game does with them.
#ifndef C4BSD_SYS_POLL_H
#define C4BSD_SYS_POLL_H 1
struct pollfd { int fd; short events, revents; };
enum { POLLIN = 1, POLLOUT = 4 };
#endif
#ifndef INFTIM
#define INFTIM (-1)
#endif
