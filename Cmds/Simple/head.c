/* head - print the first few lines of a file	Author: Andy Tanenbaum */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define DEFAULT 10

void do_file(int n, FILE *f)
{
  int c;

  /* Print the first 'n' lines of a file. */
  while (n) switch (c = getc(f)) {
	    case EOF:
		return;
	    case '\n':
		--n;
	    default:	putc((char) c, stdout);
	}
}


void usage(void)
{
  fprintf(stderr, "Usage: head [-n] [file ...]\n");
  exit(1);
}

int main(int argc, char *argv[])
{
  FILE *f;
  int n, k, nfiles;
  char *ptr;

  /* Check for flag.  Only flag is -n, to say how many lines to print. */
  k = 1;
  ptr = argv[1];
  n = DEFAULT;
  if (argc > 1 && *ptr++ == '-') {
	k++;
	n = atoi(ptr);
	if (n <= 0) usage();
  }
  nfiles = argc - k;

  if (nfiles == 0) {
	/* Print standard input only. */
	do_file(n, stdin);
	exit(0);
  }

  /* One or more files have been listed explicitly. */
  while (k < argc) {
	if (nfiles > 1) printf("==> %s <==\n", argv[k]);
	if ((f = fopen(argv[k], "r")) == NULL)
		printf("head: cannot open %s\n", argv[k]);
	else {
		do_file(n, f);
		fclose(f);
	}
	k++;
	if (k < argc) printf("\n");
  }
  exit(0);
}
