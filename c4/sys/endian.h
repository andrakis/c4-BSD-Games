// sys/endian.h -- BSD's, for its byte order: C4 is little-endian, as gcc -m32 here
#ifndef BYTE_ORDER
#define LITTLE_ENDIAN 1234
#define BIG_ENDIAN 4321
#define BYTE_ORDER LITTLE_ENDIAN
#endif
