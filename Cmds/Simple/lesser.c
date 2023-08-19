// A tiny implementation of the less pager. We only read from a file, no
// input from stdin. The code assumes a VT100 with 80x24 display.
// (c) 2023, Warren Toomey, BSD license.

#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <romcalls.h>

#define BUFLEN	512		// Size of the line buffer
#define LINELEN	 79		// We only print out this much of any line

// We keep track of lines and their offsets from the input file
// in a doubly-linked list with the following nodes

struct lineposn {
  long linenum;
  off_t offset;
  struct lineposn *prev;
  struct lineposn *next;
};

// Global variables
FILE *fp;				// File to page through
char buf[BUFLEN];			// Line buffer
struct lineposn *linehead = NULL;	// Head of the line offset buffer
struct lineposn *linetail = NULL;	// Tail of the line offset buffer

struct termios orig_termios;	// Original blocking terminal setting

// Escape sequences
char *cls = "[2J";		// Clear the screen
char *home = "[;H";		// Move to top-left corner
char *noattr = "[m";		// No bold or underlining
char *bold = "[1m";		// Bold on
char *underline = "[4m";	// Underlining on

// Set the terminal back to blocking and echo
void reset_terminal() {
  tcsetattr(0, TCSANOW, &orig_termios);
}

// Put the terminal into cbreak mode with no echo
void set_cbreak() {
  struct termios t;

  // Get the original terminal settings twice,
  // one for restoration later
  tcgetattr(0, &orig_termios);
  if (tcgetattr(0, &t) == -1) {
    fprintf(stderr, "Cannot tcgetattr\n"); exit(1);
  }

  t.c_lflag &= ~(ICANON | ECHO);
  t.c_lflag |= ISIG;
  t.c_iflag &= ~ICRNL;
  t.c_cc[VMIN] = 1;			// Character-at-a-time input
  t.c_cc[VTIME] = 0;			// with blocking

  if (tcsetattr(0, TCSAFLUSH, &t) == -1) {
    fprintf(stderr, "Cannot tcsetattr\n"); exit(1);
  }

  // Ensure we reset the terminal when we exit
  atexit(reset_terminal);
}

// Build the list of line numbers and their offsets
void build_line_list(char *filename) {
  long linenum = 1;
  off_t offset= 0;
  struct lineposn *this, *prev = NULL;

  // Open up the file
  if ((fp = fopen(filename, "r")) == NULL) {
    fprintf(stderr, "Unable to open %s\n", filename); exit(1);
  }

  // Loop reading in lines
  while (1) {
    // Get the offset of the next line and try to read it in.
    // Give up if there is nothing left to read.
    if (fgets(buf, BUFLEN, fp) == NULL) break;

    // Add another line node to the linked list
    this = (struct lineposn *) malloc(sizeof(struct lineposn));
    if (this == NULL) {
      fprintf(stderr, "malloc failure in build_line_list\n"); exit(1);
    }

    // Fill in the node's fields and update the offset
    this->linenum = linenum++; this->offset = offset;
    offset += strlen(buf);

    // Add this node to the doubly linked list
    this->prev = prev; this->next = NULL;
    if (linehead == NULL) linehead = this;
    if (prev != NULL) prev->next = this;

    // Point the previous node at this node before we move on
    prev = this;
  }

  // No more lines, set the tail to the end of the list
  linetail = prev; fclose(fp);
}

// We keep a buffer of actual characters to print out
// and a corresponding buffer of character attributes.

char linebuf[LINELEN];
char attrbuf[LINELEN];

#define NOATTR		0
#define ISBOLD		1
#define ISUNDER		2

// Print out a single line. The cursor has been positioned
// at the start of the correct line on the screen.
// We have to interpret any backspace character in the input buffer
void paint_line(struct lineposn *this) {
  char *lineptr = linebuf;
  char *attrptr = attrbuf;
  char *bufptr = buf;
  char *lineend = &linebuf[LINELEN];  // One past the end of the linebuf
  int thisattr= 0, lastattr = 0;
  lineend++;

  long linenum= this->linenum;
  long offset= this->offset;
// printf("_line %ld offset %ld\n", line, off);


  // Read the line in from the file
  if (fgets(buf, BUFLEN, fp) == NULL) {
    fprintf(stderr, "Unable to read line %ld from the file\n", linenum);
    exit(1);
  }

  // Clear out the line and attribute buffers
  memset(linebuf, 0, LINELEN);
  memset(attrbuf, 0, LINELEN);

  // Deal with as many real characters as we can fit into the linebuf
  while (lineptr < lineend) {
    // End the loop when we hit a newline
    if (*bufptr == '\n') break;

    // Move backwards if we find a backspace. Don't go below zero.
    if (*bufptr == '\b') {
      lineptr = (lineptr == linebuf) ? linebuf : lineptr - 1;
      attrptr = (attrptr == attrbuf) ? attrptr : attrptr - 1;
      bufptr++; continue;
    }

    // If there is nothing yet in the linebuf, add this character
    if (*lineptr == 0) {
      *(lineptr++) = *(bufptr++); attrptr++; continue;
    }

    // Now the fun begins. We have a character already in the linebuf
    // at this position. If it's the same as this character, mark it
    // as being bold.
    if (*lineptr == *bufptr) {
      *attrptr |= ISBOLD; bufptr++; lineptr++; attrptr++; continue;
    }

    // If the character in the linebuf is an underscore, replace it
    // with this character and mark it as underlined.
    if (*lineptr == '_') {
      *lineptr++ = *bufptr++; *attrptr |= ISUNDER; attrptr++; continue;
    }

    // If this character is an underscore, keep the existing character
    // and mark it as underlined.
    if (*bufptr == '_') {
      *attrptr |= ISUNDER; attrptr++; bufptr++; lineptr++; continue;
    }

    // At this point, we have completely different characters; what to do?
    // Replace the old character with the new one and leave it at that.
    *lineptr++ = *bufptr++; attrptr++;
  }

  // Now print out the line, inserting VT100 escape sequences as required
  for (lineptr = linebuf, attrptr = attrbuf; *lineptr; lineptr++, attrptr++) {

    // If the last character's attributes differ from this one,
    // turn off all VT100 attributes
    thisattr = *attrptr;
    if (lastattr != thisattr) {
      fputs(noattr, stdout);

      // If this character is bold, send the VT100 bold sequence
      if ((thisattr & ISBOLD) == ISBOLD) fputs(bold, stdout);

      // If this character is underline, send the VT100 underline sequence
      if ((thisattr & ISUNDER) == ISUNDER) fputs(underline, stdout);
    }

    // Now put out the character and save the character's attributes
    fputc(*lineptr, stdout); lastattr = thisattr;
  }

  // Turn off attributes if the last character had them on
  // and put out a newline
  if (lastattr != NOATTR) fputs(noattr, stdout);
  fputc('\n', stdout);
}

// Given a node in the doubly linked list,
// output 24 lines from this point onwards.
void paint_screen(struct lineposn *this) {
  int i;
  long offset= this->offset;

  // Clear the screen and move to the top left corner
  fputs(cls, stdout); fputs(home, stdout);

  // Move to the file offset for the first line
  if (fseek(fp, offset, SEEK_SET) == -1) {
    fprintf(stderr, "Unable to fseek to position %ld\n", offset);
    exit(1);
  }

  // Print out each line
  for (i = 1; i <= 24; i++) {
    if (this == NULL) return;
    paint_line(this); this = this->next;
  }
}

int main(int argc, char *argv[]) {
  struct lineposn *this;
  struct lineposn *oldthis;
  int looping = 1;
  int ch;

  // Check the arguments
  if (argc != 2) {
    fprintf(stderr, "Usage: less filename\n"); exit(1);
  }

  // Build the doubly-linked list of lines and their offsets
  build_line_list(argv[1]);

#if 0
  // Debug
  for (struct lineposn * this = linehead; this != NULL; this = this->next)
    printf("ptr %x line %ld offset %ld\n", this, this->linenum, this->offset);
  for (struct lineposn * this = linetail; this != NULL; this = this->prev)
    printf("ptr %x line %ld offset %ld\n", this, this->linenum, this->offset);
#endif

  // Reopen up the file
  if ((fp = fopen(argv[1], "r")) == NULL) {
    fprintf(stderr, "Unable to open %s\n", argv[1]); exit(1);
  }

  // Put the terminal into cbreak mode
  // and start at line 1
  set_cbreak(); this = linehead;


  // Get a command and deal with it
  while (looping) {

    // Draw a page of the file
    paint_screen(this);

    ch= romgetputc();
    switch (ch) {
    case 'q':			// Quit the pager
    case EOF:
      looping = 0;
      break;
    case ' ':			// Move down 24 lines
      oldthis = this;
      for (int i = 0; i < 24; i++) {
	this = this->next;
	if (this == NULL) {	// Too far, go back
	  this = oldthis; break;
	}
      }
      break;
    case 'b':			// Move up 24 lines
      oldthis = this;
      for (int i = 0; i < 24; i++) {
	this = this->prev;
	if (this == NULL) {	// Too far, go back down
	  this = oldthis; break;
	}
      }
      break;
    case 'j':			// Down one line
    case '\n':
      oldthis = this;
      this = this->next;
      if (this == NULL) this = oldthis;
      break;
    case 'k':			// Up one line
      oldthis = this;
      this = this->prev;
      if (this == NULL) this = oldthis;
      break;
    }
  }
  return (0);
}
