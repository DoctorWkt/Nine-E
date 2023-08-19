#include <sys/types.h>
#include <stdio.h>

#define SIZE 256

char buf[SIZE];

int main(int argc, char *argv[]) {
  char *ptr;

                          printf("buf starts at 0x%x\n", buf);
  ptr= buf + SIZE;        printf("ptr points at 0x%x\n", ptr);
  ptr= buf + sizeof(buf); printf("ptr points at 0x%x\n", ptr);
  ptr= &buf[SIZE];        printf("ptr points at 0x%x\n", ptr);
  return(0);
}
