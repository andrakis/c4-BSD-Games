// setjmp.h -- C4IX's libc has none; c4bsd-jmp.c has one for the c4 VM, from
// Quake's port. gcc's build has glibc's.
//
// Only as C allows it: the whole condition of an if or a switch, a comparison
// with a constant, or a statement of its own. In `r = setjmp(b)` the stack
// machine has r's address on its stack across the call, and what ran since has
// written over it.
#ifndef C4BSD_SETJMP_H
#define C4BSD_SETJMP_H 1
typedef int jmp_buf[3];
int  bsdc_setjmp(int *b);
void bsdc_longjmp(int *b, int v);
#define setjmp(b) bsdc_setjmp((int *)(b))
#define longjmp(b, v) bsdc_longjmp((int *)(b), (v))
#endif
