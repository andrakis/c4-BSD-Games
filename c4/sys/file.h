// sys/file.h -- BSD's, for its open flags and lock names.
#include <fcntl.h>
#ifndef LOCK_EX
enum { LOCK_SH = 1, LOCK_EX = 2, LOCK_NB = 4, LOCK_UN = 8 };
#endif
