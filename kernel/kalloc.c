// Physical memory allocator, for user processes,
// kernel stacks, page-table pages,
// and pipe buffers. Allocates whole 4096-byte pages.

#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "riscv.h"
#include "defs.h"

struct pgindex pgcount;
void freerange(void *pa_start, void *pa_end);

extern char end[]; // first address after kernel.
                   // defined by kernel.ld.

struct run {
  struct run *next;
};

struct {
  struct spinlock lock;
  struct run *freelist;
} kmem;



uint64 MINN = __UINT64_MAX__;
uint64 MAXX = 0;
void
kinit()
{
  initlock(&kmem.lock, "kmem");

  // printf("%d\n",pgcount.MINN);
  // printf("%d\n",pgcount.MAXX);
  freerange(end, (void*)PHYSTOP);
  // memset(&pgcount,0,sizeof(pgcount));
  // printf("%d\n",pgcount.MINN);
  // printf("%d\n",pgcount.MAXX);
}

void
freerange(void *pa_start, void *pa_end)
{
  char *p;
  // printf("%d\n",pgcount.MINN);
  p = (char*)PGROUNDUP((uint64)pa_start);
  for(; p + PGSIZE <= (char*)pa_end; p += PGSIZE){
    if(((uint64)p % PGSIZE) != 0 || (char*)p < end || (uint64)p >= PHYSTOP)
      panic("kfree");
    if((uint64)p<MINN)MINN = (uint64)p;
    if((uint64)p>MAXX)MAXX = (uint64)p;
    // printf("%d\n",((uint64)p)/4096-pgcount.MINN);
    // pgcount.ind[((uint64)p)/4096-pgcount.MINN]=0;
    // kfree(p);
  }
  pgcount.MAXX = MAXX/4096;
  pgcount.MINN = MINN/4096;
  p = (char*)PGROUNDUP((uint64)pa_start);
  for(; p + PGSIZE <= (char*)pa_end; p += PGSIZE){
    // printf("%d\n",((uint64)p)/4096-pgcount.MINN);
    
    pgcount.ind[((uint64)p)/4096-pgcount.MINN]=0;
    kfree(p);
  }

}

// Free the page of physical memory pointed at by v,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void
kfree(void *pa)
{
  struct run *r;

  if(((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP)
    panic("kfree");

  // Fill with junk to catch dangling refs.
  if(pgcount.ind[((uint64)pa)/4096-pgcount.MINN]==0){
  memset(pa, 1, PGSIZE);
  r = (struct run*)pa;
  acquire(&kmem.lock);
  
    r->next = kmem.freelist;
    kmem.freelist = r;
    release(&kmem.lock);
  }
  else if(pgcount.ind[((uint64)pa)/4096-pgcount.MINN]<0)
  {
    printf("fuckshit\n");
  }
  else if(pgcount.ind[((uint64)pa)/4096-pgcount.MINN]>0)
  {
    
    
  }
}
void trapframe_free(void *pa)
{
  struct run *r;

  if(((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP)
    panic("kfree");

  // Fill with junk to catch dangling refs.
  memset(pa, 1, PGSIZE);
  r = (struct run*)pa;
  acquire(&kmem.lock);
  
    r->next = kmem.freelist;
    kmem.freelist = r;
    release(&kmem.lock);
  
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
void *
kalloc(void)
{
  struct run *r;

  acquire(&kmem.lock);
  r = kmem.freelist;
  if(r)
    kmem.freelist = r->next;
  release(&kmem.lock);

  if(r){
    memset((char*)r, 5, PGSIZE); // fill with junk
    pgcount.ind[((uint64)r)/4096-pgcount.MINN]=1;
    // printf("%d\n",pgcount.ind[((uint64)r)/4096-pgcount.MINN]);
  }
  return (void*)r;
}
