/*
    main.c   	-	main program for 6809 simulator
    Copyright (C) 2001  Arto Salmi
                        Joze Fabcic

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include "6809.h"

enum { HEX, S19, BIN };

extern int romaddr;
extern struct watchpoint *watchhead;
int total = 0;
char *exename;
char *ch375file= "unknown";

// Debut output file
FILE *debugout= NULL;

// Push any arguments on to the stack
#define MAX_ARGS 100

void arrange_arguments(int argc, char *argv[]) {

  int aposn[MAX_ARGS];
  int posn;

  if (argc > MAX_ARGS) {
    fprintf(stderr, "Too many arguments to store on the stack!\n"); exit(1);
  }

  posn= get_s();				// Get the current stack ptr

  for (int i = argc - 1; i != -1; i--) {	// For each arg string
    int len = strlen(argv[i]) + 1;		// get its length
    posn -= len;
    memcpy(&memory[posn], argv[i], (size_t) len); // Copy it to the stack
    aposn[i] = posn;
  }

  posn -= 2;
  WRMEM16(posn, 0);				// Put a NULL on the stack

  for (int i = argc - 1; i != -1; i--) {	// For each arg string
    posn -= 2;
    WRMEM16(posn, aposn[i]);			// Put the arg ptr on the stack
  }

  posn -= 2;					// Put ptr to array on stack
  WRMEM16(posn, posn+2);

  posn -= 2;
  WRMEM16(posn, argc);				// Finally, the argc
  set_s(posn);					// and update the SP
}

// Add a watchpoint to the linked list
static void add_watchpoint(int addr) {
  struct watchpoint *this;
  this=(struct watchpoint *)malloc(sizeof(struct watchpoint));
  if (this==NULL) {
    fprintf(stderr, "malloc failed in add_watchpoint\n"); exit(1); 
  }
  this->addr= addr;

  // Prepend to the linked list
  if (watchhead==NULL)
    this->next= NULL;
  else
    this->next= watchhead;
  watchhead= this;
}

static void usage (void)
{
  printf("Usage: %s <options> s19_filename\n",exename);
  printf("Options are:\n");
  printf("-m        - start in the monitor\n");
  printf("-x        - randomise memory contents\n");
  printf("-b   addr - set a breakpoint in hex and run until that address\n");
  printf("-r   addr - set ROM start address in hex\n");
  printf("-s   addr - set stack start address in hex\n");
  printf("-a   addr - start address in hex (instead of reset vector)\n");
  printf("-i   name - use the named fs image for CH375 block operations\n");
  printf("-p   name - also load the named s19 image\n");
  printf("-d   name - write debug output to this file\n");
  printf("-w   addr - stop execution when there is a write to this address\n");

  if(memory != NULL) free(memory);
  exit (1);
}


int main (int argc, char *argv[])
{
  char *name;
  int opt;
  int breakpoint= -1;
  int randomise_memory=0;
  int start_in_monitor=0;
  int start_addr= -1;
  int start_stack= 0xC7FF;		// Nine-E V1. V2 will be $D7FF

  int newargc;
  char **newargv;

  exename = argv[0];

  if (argc == 1) usage();

  memory = (UINT8 *)malloc(64 * 1024);	// Allocate 64K of RAM
  if (memory == NULL) usage();
  memset (memory, 0, 0x10000);		// Clear RAM

  // Get the options
  while ((opt = getopt(argc, argv, "mxb:r:s:a:i:p:d:w:")) != -1) {
    switch(opt) {
      case 'm': start_in_monitor=1; break;
      case 'x': randomise_memory=1; break;
      case 'b': breakpoint= strtoul(optarg,NULL,16); break;
      case 'r': romaddr= strtoul(optarg,NULL,16); break;
      case 's': start_stack= strtoul(optarg,NULL,16); break;
      case 'a': start_addr= strtoul(optarg,NULL,16); break;
      case 'i': ch375file= optarg; break;
      case 'p': if (load_s19(optarg)) usage(); break;
      case 'd': if ((debugout=fopen(optarg, "w"))==NULL) {
		  fprintf(stderr, "Unable to open %s\n", optarg); exit(1); 
		}
		break;
      case 'w': add_watchpoint(strtoul(optarg,NULL,16));
		break;
    }
  }

  // Randomise memory if required
  if (randomise_memory)
    for (int i=0; i < 0x10000; i++)
      memory[i]= (char)rand();

  // Get the filename to load
  if (optind >= argc) usage();
  name= argv[optind];

  // Get any arguments to pass to the simulated 6809 program
  newargc= argc - optind;
  newargv= &(argv[optind]);
 
  cpu_quit = 1;

  // Load the binary
  if(load_s19(name)) usage();

  monitor_init(start_in_monitor);

  // If there's a breakpoint, set it
  if (breakpoint != -1) {
    add_breakpoint(breakpoint);
    monitor_on = 0;
    do_break = 1;
  }

  cpu_reset(start_addr, start_stack);

  // Set up the arguments on the stack
  // once we know where it starts
  arrange_arguments(newargc, newargv);

  do
  {
    total += cpu_execute (6000000);
  } while (cpu_quit != 0);

  printf("6809 stopped after %d cycles\n",total);

  free(memory);

  return 0;
}
