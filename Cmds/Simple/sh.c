#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <xv6/param.h>
#include <fcntl.h>
#include <romcalls.h>
void cprintf(char *fmt, ...);

#define MAXARG 20		// Maximum number of arguments

// Our variables are global so as not
// to be on the stack when we copy stuff.
int i, fd, realargc, ch, len, memsize, stroffset;
int argc=0, wordc;
int patterncnt=0;
int *newargc;
char *cmdp, *destbuf, *sptr;
char *argv[MAXARG + 1], **newargv, **newarglist;
char *wordlist[MAXARG + 1];
char *patternlist[MAXARG + 1];
DIR *D;
struct dirent *dent;
char buf[100];
char binbuf[100];
char seekbuf[100];

void myexit(int x) {
  // Close any open file descriptors
  for (fd=0; fd < NFILE; fd++) sys_close(fd);
  exit(0);
}

// Copy the arguments to upper memory.
void copyargs() {

  // Calculate the amount of memory to allocate:
  // realargc+1 times char pointers. The +1
  // allows us to put a NULL on the end of newargv.
  // Also allow for the pointer to the arglist after realargc.
  // Save the offset to the future first string location.
  stroffset = memsize = sizeof(char **) + (realargc + 1) * sizeof(char *);

  // Now add on the lengths of all the arguments.
  // Ensure we count the NUL at the end of each argument.
  for (i = 0; i < realargc; i++) memsize += strlen(argv[i]) + 1;


  // Set the start of the destbuf at
  // the top of the stack minus memsize.
  // Point the stack pointer here as well
  destbuf = (char *) (0xc3ff - memsize);
  __asm("\tlds destbuf");

  // Set the location of the new argv and arglist
  newargv = (char **) destbuf;
  newarglist= (char **) (destbuf + sizeof(char **));
  *newargv= (char *)newarglist;

  // Set the base of the strings.
  sptr = destbuf + stroffset;

  // Copy the arguments and put pointers into newargv.
  // Make sure we copy the NUL at the end of each argument.
  for (i = 0; i < realargc; i++) {
    len = strlen(argv[i]) + 1;
    memmove(sptr, argv[i], len);
    newarglist[i] = sptr; sptr += len;
  }

  // Put a NULL at the end of the arglist array and then spawn the program.
  // Put newargv into the D register.
  newarglist[i] = NULL;
  __asm("\tnop");
  __asm("\tldd realargc");
  spawn();
  myexit(0);		// Should never get here!
}

// Return true if the name matches the given pattern.
// Only ? and * are recognised. This code from Russ Cox:
// https://research.swtch.com/glob
int match(char *pattern, char *name) {
  int px=0, nx=0;
  int nextpx=0, nextnx=0;
  int plen= strlen(pattern);
  int nlen= strlen(name);
  char c;

  for (; px < plen || nx < nlen; ) {
    if (px < plen) {
      c= pattern[px];

      switch(c) {
	case '?':		// Single character wildcard
	  if (nx < nlen) {
	    px++;
	    nx++;
	    continue;
	  }
	  break;

	case '*':		// Zero or more character wildcard
				// Try to match at nx. If that
				// doesn't work out, restart at
				// nx+1 next.
	  nextpx= px;
	  nextnx= nx+1;
	  px++;
	  continue;

	default:		// Ordinary character
	  if (nx < nlen && name[nx] == c) {
	    px++;
 	    nx++;
	    continue;
	  }
      }
    }

    				// Mismatch, maybe restart
    if (0 < nextnx && nextnx <= nlen) {
      px = nextpx;
      nx = nextnx;
      continue;
    }
    return(0);
  }
  // Matched all of pattern to all of name. Success.
  return(1);
}

int main() {

  // Close any open file descriptors
  for (fd=0; fd < NFILE; fd++) sys_close(fd);

  // Now open fds 0, 1 and 2 to the console
  fd= sys_open("/tty", O_RDONLY);
  fd= sys_open("/tty", O_WRONLY);
  fd= sys_open("/tty", O_WRONLY);

  // Print out the prompt
  romputc('$');
  romputc(' ');

  // Get characters into the line buffer, ending
  // when we get a newline or run out of buffer.
  // NUL terminate the buffer just in case.
  buf[99] = 0;
  for (i = 0; i < 99; i++) {
    ch = romgetputc();
    if (ch == '\n' || ch == '\r') {
      buf[i] = 0;
      break;
    }
    buf[i] = (char) (ch & 0xff);
  }

  // Split the line up into non-whitespace arguments or operators
  for (cmdp = buf, wordc = 0; wordc < MAXARG; wordc++) {

    // Get an argument. If none left, break the loop
    if ((wordlist[wordc] = strtok(cmdp, " \t\r\n")) == NULL) break;

    // Tell strtok to keep going
    cmdp = NULL;
  }

  // No arguments, just restart the shell
  if (wordc==0) myexit(0);

  // Copy the words into the argv list for now
  for (i=0; i<wordc && argc < MAXARG; i++) {

    // See if the word contains a pattern to match on, i.e. a '*' or '?'
    if ((index(wordlist[i], '?')!=NULL) || (index(wordlist[i], '*')!=NULL)) {
      // For now we can only pattern match in this directory
      if (index(wordlist[i], '/')!=NULL) {
	cprintf("Pattern matching only in this directory, sorry\n"); exit(1);
      }
      // Add the word as a pattern
      patternlist[patterncnt++]= wordlist[i]; continue;
    }

    // Not a pattern, add the word to the argv list
    argv[argc++]= wordlist[i];
  }

  // If we have any patterns, go through all the entries in this
  // directory and see if any of them match
  if (patterncnt!=0) {
    D= opendir(".");
    if (D==NULL) { cprintf("Can't opendir .\n"); exit(1); }

    // Process each entry
    while ((dent=readdir(D))!=NULL) {

      // Skip empty directory entries
      if (dent->d_name[0]=='\0') continue;

      for (i=0; i<patterncnt; i++) {
	// cprintf("About to match %s against %s\n", patternlist[i], dent->d_name);

	// If there's a match, add the filename to the argv list
	if (match(patternlist[i], dent->d_name) && argc < MAXARG) {
          argv[argc++]= dent->d_name;
	}
      }
    }
    closedir(D);
  }

  // See if the first argument is cd. If so, do the chdir and
  // then myexit(), which will respawn the shell :-)
  if (!strcmp(argv[0], "cd")) {
    if (argc==2) {
      if (sys_chdir(argv[1]) == -1)
	cprintf("Cannot cd to %s\n", argv[1]);
    }
    myexit(0);
  }

  // Try to open the argument.
  if ((fd= open(argv[0], O_RDONLY))== -1) {
    // No such luck. Try putting "/bin/" on the front
    strcpy(binbuf, "/bin/");
    strcat(binbuf, argv[0]);
    if ((fd= open(binbuf, O_RDONLY))== -1) {
      cprintf("%s: no such command\n", argv[0]); myexit(0);
    }

    argv[0]= binbuf;
  }

  // See if it's an executable: always start with 0x3406
  read(fd, &i, 2);
  if (i != 0x3406) {
    // cprintf("%s: not an executable, magic is %x\n", argv[0], i); myexit(0);
    cprintf("%s: not an executable\n", argv[0]); myexit(0);
  }
  close(fd);
  
  // Assume we won't have any redirection
  realargc= argc;

  // Walk the argument list and process redirections: <, > and 2> only.
  // We go to the 2nd-last argument so we can still process the next one.
  for (i=0; i<argc-1; i++) {

    // Stdout redirected to a file
    if (!strcmp(argv[i], ">")) {

      // Open the file. 
      if ((fd= sys_open(argv[i+1], O_CREAT | O_TRUNC | O_WRONLY))==-1) {
        cprintf("Cannot open %s\n", argv[i+1]); exit(1);
      }

      // Close stdout and dup the fd down to stdout, then close the fd
      sys_close(1); sys_dup(fd); sys_close(fd);

      // Set the arg count to before this token. Skip the next token
      if (realargc==argc) realargc=i;
      i++;
    }

    // Stdout redirected to a file, appending
    if (!strcmp(argv[i], ">>")) {

      // Open the file for reading and writing, so we can lseek on it.
      if ((fd= sys_open(argv[i+1], O_WRONLY|O_APPEND))==-1) {
        cprintf("Cannot open %s\n", argv[i+1]); exit(1);
      }

      // Close stdout and dup the fd down to stdout, then close the fd
      sys_close(1); sys_dup(fd); sys_close(fd);

      // Read to the end of the file
      // while ((len=sys_read(1, seekbuf, 100))>0) ;

      // Set the arg count to before this token. Skip the next token
      if (realargc==argc) realargc=i;
      i++;
    }

    // Stdin redirected from a file
    if (!strcmp(argv[i], "<")) {

      // Open the file.
      if ((fd= sys_open(argv[i+1], O_RDONLY))==-1) {
        cprintf("Cannot open %s\n", argv[i+1]); exit(1);
      }

      // Close stdin and dup the fd down to stdin, then close the fd
      sys_close(0); sys_dup(fd); sys_close(fd);

      // Set the arg count to before this token. Skip the next token
      if (realargc==argc) realargc=i;
      i++;
    }

    // Stderr redirected to a file
    if (!strcmp(argv[i], "2>")) {

      // Open the file.
      if ((fd= sys_open(argv[i+1], O_CREAT | O_TRUNC | O_WRONLY))==-1) {
        cprintf("Cannot open %s\n", argv[i+1]); exit(1);
      }

      // Close stderr and dup the fd down to stderr, then close the fd
      sys_close(2); sys_dup(fd); sys_close(fd);

      // Set the arg count to before this token. Skip the next token
      if (realargc==argc) realargc=i;
      i++;
    }
  }

  copyargs();

  return (0);
}
