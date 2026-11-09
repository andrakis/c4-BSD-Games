// sys/uio.h -- BSD's, in c4bsd.c. gcc's build has glibc's.
#ifndef C4BSD_SYS_UIO_H
#define C4BSD_SYS_UIO_H 1
struct iovec { void *iov_base; int iov_len; };
int writev(int fd, const struct iovec *v, int n);
int readv(int fd, const struct iovec *v, int n);
#endif
