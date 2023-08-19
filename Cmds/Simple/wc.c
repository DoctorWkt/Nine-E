#include <stdlib.h>
#include "romcalls.h"
char *strchr(const char *s, int c);
void cprintf(char *fmt, ...);

char buf[512];

void
wc(int fd, char *name)
{
  int i, n;
  int l, w, c, inword;

  l = w = c = 0;
  inword = 0;
  while((n = sys_read(fd, buf, sizeof(buf))) > 0){
    for(i=0; i<n; i++){
      c++;
      if(buf[i] == '\n')
        l++;
      if(strchr(" \r\t\n\v", buf[i]))
        inword = 0;
      else if(!inword){
        w++;
        inword = 1;
      }
    }
  }
  if(n < 0){
    cprintf("wc: read error\n");
    exit(0);
  }
  cprintf("%d %d %d %s\n", l, w, c, name);
}

int
main(int argc, char *argv[])
{
  int fd, i;

  for(i = 1; i < argc; i++){
    if((fd = sys_open(argv[i], 0)) < 0){
      cprintf("wc: cannot open %s\n", argv[i]);
      exit(0);
    }
    wc(fd, argv[i]);
    sys_close(fd);
  }
  exit(0); return(0);
}
