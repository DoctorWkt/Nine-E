Welcome to the Nine-E project. The aim here is to create a Unix-like
environment on an 8-bit computer.

On the hardware side, there is:
 - 8K of ROM,
 - 54K of RAM,
 - a UART, and
 - access to a USB key for disk storage.

On the software side:
 - the xv6 filesystem code resides in ROM,
 - the libc and include files come mostly from Fuzix,
 - the userland applications come mainly from xv6-freebsd.

There is no MMU and no clock device, so there isn't any multitasking
(i.e. no `fork()`, `exec()`, `time()`. Apart from that, what is able
to work is pretty impressive.

## Status -- mid-August 2023

The hardware is rock solid. I've got the xv6 filesystem in ROM and working well.
I've got some of the FUZIX libc running including the mem/string functions, the
stdio functions, and malloc/brk/sbrk. The rest of the libc is untested.

There are some basic programs, e.g. the ones that come with xv6. There is also
a simple vi-like editor, a simple less-like pager, and a document processor
called `roff`. So, with the current system, you can edit documents, process them
and see the output.

There is a simple shell that knows about `*` and `?` expansion, file redirection
such as `>`, `<`, `>>` and `2>`.

## Quick Start

There should be a copy of the USB key, `fs.img`, already here.
And, in the `XV6FS` dfirectory, a copy of the ROM image `xv6rom`.
Go into the `Salmi` directory and do a `make` to make the Nine-E simulator.
Now run the `VROMXV6` script to start the simulator with the ROM and the
filesystem:

```
$ ./VROMXV6 
$ ls -l
-rwxrwxrwx     1 root root    129 README
drwxrwxrwx     2 root root    464 bin
-rwxrwxrwx     1 root root   6071 roff_manual
$ cd bin
$ ls -l
drwxrwxrwx     1 root root     32 Z
-rwxrwxrwx     1 root root   3494 basename
-rwxrwxrwx     1 root root   5277 cal
-rwxrwxrwx     1 root root   1798 cat
-rwxrwxrwx     1 root root   5285 cksum
-rwxrwxrwx     1 root root   4537 cmp
-rwxrwxrwx     1 root root   4305 comm
-rwxrwxrwx     1 root root   6486 crc
-rwxrwxrwx     1 root root   7676 cut
-rwxrwxrwx     1 root root   1168 echo
-rwxrwxrwx     1 root root   6341 expand
-rwxrwxrwx     1 root root   8747 grep
-rwxrwxrwx     1 root root   7052 lesser
-rwxrwxrwx     1 root root   1630 ln
-rwxrwxrwx     1 root root   8165 ls
-rwxrwxrwx     1 root root   1653 mkdir
-rwxrwxrwx     1 root root   6479 od
-rwxrwxrwx     1 root root   2327 oldgrep
-rwxrwxrwx     1 root root   2488 oldls
-rwxrwxrwx     1 root root   3001 pwd
-rwxrwxrwx     1 root root   1647 rm
-rwxrwxrwx     1 root root   6594 roff
-rwxrwxrwx     1 root root   5626 sh
-rwxrwxrwx     1 root root   3178 try
-rwxrwxrwx     1 root root  10951 usertests
-rwxrwxrwx     1 root root  24626 vi
-rwxrwxrwx     1 root root   1930 wc
```

## TODO

 - Test more of the libc functions.
 - Write stub functions that do nothing, or do
   very little, e.g. return a constant `time()`.
 - Bring in more programs.
 - Modify the shell to simulate pipelines by
   saving the standard output of one program to a 
   temporary file and, when it `exit()`s,
   start the next program with the temporary
   file as its standard input.
