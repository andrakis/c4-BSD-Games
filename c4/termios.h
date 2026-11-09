// termios.h -- C4's systems have no terminal modes to set: a game's own settings
// are noted and do nothing. gcc's build has glibc's.
#ifndef C4BSD_TERMIOS_H
#define C4BSD_TERMIOS_H 1
typedef unsigned int tcflag_t;
typedef unsigned char cc_t;
typedef unsigned int speed_t;
struct termios {
    tcflag_t c_iflag, c_oflag, c_cflag, c_lflag;
    cc_t c_cc[32];
    speed_t c_ispeed, c_ospeed;
};
enum { ECHO = 8, ICANON = 2, ISIG = 1, IEXTEN = 32768, ICRNL = 256, IXON = 1024, OPOST = 1, ONLCR = 4,
       VERASE = 2, VKILL = 3, VEOF = 4, VINTR = 0, VQUIT = 1, VMIN = 6, VTIME = 5, VSUSP = 10,
       VSTART = 8, VSTOP = 9, VWERASE = 14, VLNEXT = 15, VDISCARD = 13, VREPRINT = 12,
       TCSANOW = 0, TCSADRAIN = 1, TCSAFLUSH = 2, TCIFLUSH = 0, TCOFLUSH = 1, TCIOFLUSH = 2,
       B0 = 0, B50 = 1, B75 = 2, B110 = 3, B134 = 4, B150 = 5, B200 = 6, B300 = 7, B600 = 8,
       B1200 = 9, B1800 = 10, B2400 = 11, B4800 = 12, B9600 = 13, B19200 = 14, B38400 = 15 };
int tcgetattr(int fd, struct termios *t);
int tcsetattr(int fd, int how, const struct termios *t);
int tcflush(int fd, int q);
speed_t cfgetospeed(const struct termios *t);
speed_t cfgetispeed(const struct termios *t);
#endif
