// Buffer cache.
//
// The buffer cache is a linked list of buf structures holding
// cached copies of disk block contents.  Caching disk blocks
// in memory reduces the number of disk reads and also provides
// a synchronization point for disk blocks used by multiple processes.
//
// Interface:
// * To get a buffer for a particular disk block, call bread.
// * After changing buffer data, call bwrite to write it to disk.
// * When done with the buffer, call brelse.
// * Do not use the buffer after calling brelse.
// * Only one process at a time can use a buffer,
//     so do not keep them longer than necessary.


#include "types.h"
#include "param.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "riscv.h"
#include "defs.h"
#include "fs.h"
#include "buf.h"
#define hashnum 7
struct {
  struct spinlock lock;
  struct buf buf[NBUF];

  // Linked list of all buffers, through prev/next.
  // Sorted by how recently the buffer was used.
  // head.next is most recent, head.prev is least.
  // struct buf head;
} bcache[hashnum];

void
binit(void)
{
  // struct buf *b;
  int i=0,j=0;
  for(i=0;i<hashnum;i++)
  {
  initlock(&(bcache[i].lock), "bcache");
  for(j=0;j<NBUF;j++){
    bcache[i].buf[j].tick = ticks;
    bcache[i].buf[j].refcnt = 0;
    initsleeplock(&(bcache[i].buf[j].lock), "buffer");
  }
  }
}

// Look through buffer cache for block on device dev.
// If not found, allocate a buffer.
// In either case, return locked buffer.
static struct buf*
bget(uint dev, uint blockno)
{
  struct buf *b;

  acquire(&(bcache[blockno%hashnum].lock));
  int i = 0;
  // Is the block already cached?
  for(i=0;i<NBUF;i++){
    if(bcache[blockno%hashnum].buf[i].dev == dev && bcache[blockno%hashnum].buf[i].blockno == blockno){
      bcache[blockno%hashnum].buf[i].refcnt++;
      // bcache[blockno%hashnum].buf[i].tick = ticks;
      release(&bcache[blockno%hashnum].lock);
      acquiresleep(&(bcache[blockno%hashnum].buf[i].lock));
      return &(bcache[blockno%hashnum].buf[i]);
    }
  }

  // Not cached.
  // Recycle the least recently used (LRU) unused buffer.
  uint minn = __UINT32_MAX__;
  int pos = -1;
  for(i = 0; i<NBUF;i++){
    if( bcache[blockno%hashnum].buf[i].refcnt == 0 && bcache[blockno%hashnum].buf[i].tick<minn) {
      pos = i;
      minn = bcache[blockno%hashnum].buf[i].tick;
    }
  }
  if(pos==-1)panic("bget: no buffers");
  else 
  {
    b = &(bcache[blockno%hashnum].buf[pos]);
    b->dev = dev;
    b->blockno = blockno;
    b->valid = 0;
    b->refcnt = 1;
    b->tick = ticks;
    release(&(bcache[blockno%hashnum].lock));
    acquiresleep(&b->lock);
    return b;
  }

  
}

// Return a locked buf with the contents of the indicated block.
struct buf*
bread(uint dev, uint blockno)
{
  struct buf *b;

  b = bget(dev, blockno);
  if(!b->valid) {
    virtio_disk_rw(b, 0);
    b->valid = 1;
  }
  return b;
}

// Write b's contents to disk.  Must be locked.
void
bwrite(struct buf *b)
{
  if(!holdingsleep(&b->lock))
    panic("bwrite");
  virtio_disk_rw(b, 1);
}

// Release a locked buffer.
// Move to the head of the most-recently-used list.
void
brelse(struct buf *b)
{
  if(!holdingsleep(&b->lock))
    panic("brelse");

  releasesleep(&b->lock);

  
  b->refcnt--;
  // if(b->refcnt==0)
  b->tick = ticks;
}

void
bpin(struct buf *b) {
  acquire(&(bcache[b->blockno%hashnum].lock));
  b->refcnt++;
  b->tick = ticks;
  release(&(bcache[b->blockno%hashnum].lock));
}

void
bunpin(struct buf *b) {
  acquire(&(bcache[b->blockno%hashnum].lock));
  b->refcnt--;
  b->tick = ticks;
  release(&(bcache[b->blockno%hashnum].lock));
}


