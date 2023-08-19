/* setjmp.h - c16x */

#ifndef __SETJMP_H
#define __SETJMP_H 1

typedef int jmp_buf[4];

int setjmp (jmp_buf);
void longjmp (jmp_buf, int);

#endif

