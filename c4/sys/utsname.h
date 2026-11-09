// sys/utsname.h -- BSD's, for a score line's "where". Both builds say the same.
#ifndef C4BSD_SYS_UTSNAME_H
#define C4BSD_SYS_UTSNAME_H 1
struct utsname { char sysname[16], nodename[16], release[16], version[16], machine[16]; };
#define uname(u) (strcpy((u)->nodename, "c4"), 0)
#endif
