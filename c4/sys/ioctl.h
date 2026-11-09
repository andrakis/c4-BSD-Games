// sys/ioctl.h -- no device has any modes to ask about here.
#ifndef C4BSD_SYS_IOCTL_H
#define C4BSD_SYS_IOCTL_H 1
struct winsize { unsigned short ws_row, ws_col, ws_xpixel, ws_ypixel; };
enum { TIOCGWINSZ = 21523 };
#define ioctl(fd, req, arg) (-1)
#endif
