#ifdef __linux__
// Simulate a block device with an underlying Unix file

#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "types.h"
#include "defs.h"
#include "param.h"
#include "fs.h"
#include "buf.h"

static int blkfd=-1;		// File descriptor of open file

// Open the underlying file if not yet done
void blkinit(void) {
  if (blkfd == -1) {
    blkfd= open("lfs.img", O_RDWR);
    if (blkfd==-1)
      panic("Unable to open lfs.img");
  }
}

// Sync buf with disk.
// If B_DIRTY is set, write buf to disk, clear B_DIRTY, set B_VALID.
// Else if B_VALID is not set, read buf from disk, set B_VALID.
void
blkrw(struct buf *b)
{
  blkinit();
  if((b->flags & (B_VALID|B_DIRTY)) == B_VALID)
    panic("blkrw: nothing to do");

  if (lseek(blkfd, b->blockno * BSIZE, SEEK_SET) <0)
    panic("blkrw: lseek failed");

  if (b->flags & B_DIRTY) {
    // printf("blkw %08X\n", b->blockno);
    if (write(blkfd, b->data, BSIZE)<0)
      panic("blkrw: write failed");
    b->flags &= ~B_DIRTY;
  } else {
    // printf("blkr %08X\n", b->blockno);
    if (read(blkfd, b->data, BSIZE)<0)
      panic("blkrw: read failed");
  }
  b->flags |= B_VALID;
}

#else

// Access the CH375 block device

#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <xv6/types.h>
#include <xv6/defs.h>
#include <xv6/param.h>
#include <xv6/fs.h>
#include <xv6/buf.h>
#include <xv6/romfuncs.h>

// Read/write a buffer
// If B_DIRTY is set, write buf to disk, clear B_DIRTY, set B_VALID.
// Else if B_VALID is not set, read buf from disk, set B_VALID.
void blkrw(struct buf *b) {
  if ((b->flags & (B_VALID | B_DIRTY)) == B_VALID)
    panic("bl2");

  if (b->flags & B_DIRTY) {
    if (writeblock(b->data, b->blockno) == 0)
      panic("bl3");
    b->flags &= ~B_DIRTY;
  } else {
    if (readblock(b->data, b->blockno) == 0)
      panic("bl4");
  }
  b->flags |= B_VALID;
}
#endif
