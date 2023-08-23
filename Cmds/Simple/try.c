#include <sys/types.h>
#include <stdio.h>
#include <string.h>

int main(int argc, char *argv[]) {
  long x;
  long y;
  for (x= 0; x < 100; x= x + 10) {
    y= x / 10;
    printf("y is %ld\n", y);
  }
  return(0);
}
