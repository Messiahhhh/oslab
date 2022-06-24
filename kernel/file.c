//
// Support functions for system calls that involve file descriptors.
//

#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "fs.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "file.h"
#include "stat.h"
#include "proc.h"
#include "vma.h"

struct vma VMA[16];
struct devsw devsw[NDEV];
struct {
  struct spinlock lock;
  struct file file[NFILE];
} ftable;


void
fileinit(void)
{
  initlock(&ftable.lock, "ftable");
}

// Allocate a file structure.
struct file*
filealloc(void)
{
  struct file *f;

  acquire(&ftable.lock);
  for(f = ftable.file; f < ftable.file + NFILE; f++){
    if(f->ref == 0){
      f->ref = 1;
      release(&ftable.lock);
      return f;
    }
  }
  release(&ftable.lock);
  return 0;
}

// Increment ref count for file f.
struct file*
filedup(struct file *f)
{
  acquire(&ftable.lock);
  if(f->ref < 1)
    panic("filedup");
  f->ref++;
  release(&ftable.lock);
  return f;
}

void filedec(struct file* f)
{
  acquire(&ftable.lock);
  if(f->ref < 1)
    panic("fileclose");
  --f->ref;
  release(&ftable.lock);
}
// Close file f.  (Decrement ref count, close when reaches 0.)
void
fileclose(struct file *f)
{
  struct file ff;

  acquire(&ftable.lock);
  if(f->ref < 1)
    panic("fileclose");
  if(--f->ref > 0){
    release(&ftable.lock);
    return;
  }
  ff = *f;
  f->ref = 0;
  f->type = FD_NONE;
  release(&ftable.lock);

  if(ff.type == FD_PIPE){
    pipeclose(ff.pipe, ff.writable);
  } else if(ff.type == FD_INODE || ff.type == FD_DEVICE){
    begin_op();
    iput(ff.ip);
    end_op();
  }
}

// Get metadata about file f.
// addr is a user virtual address, pointing to a struct stat.
int
filestat(struct file *f, uint64 addr)
{
  struct proc *p = myproc();
  struct stat st;
  
  if(f->type == FD_INODE || f->type == FD_DEVICE){
    ilock(f->ip);
    stati(f->ip, &st);
    iunlock(f->ip);
    if(copyout(p->pagetable, addr, (char *)&st, sizeof(st)) < 0)
      return -1;
    return 0;
  }
  return -1;
}

// Read from file f.
// addr is a user virtual address.
int
fileread(struct file *f, uint64 addr, int n)
{
  int r = 0;

  if(f->readable == 0)
    return -1;

  if(f->type == FD_PIPE){
    r = piperead(f->pipe, addr, n);
  } else if(f->type == FD_DEVICE){
    if(f->major < 0 || f->major >= NDEV || !devsw[f->major].read)
      return -1;
    r = devsw[f->major].read(1, addr, n);
  } else if(f->type == FD_INODE){
    ilock(f->ip);
    if((r = readi(f->ip, 1, addr, f->off, n)) > 0)
      f->off += r;
    iunlock(f->ip);
  } else {
    panic("fileread");
  }

  return r;
}

// Write to file f.
// addr is a user virtual address.
int
filewrite(struct file *f, uint64 addr, int n)
{
  int r, ret = 0;

  if(f->writable == 0)
    return -1;

  if(f->type == FD_PIPE){
    ret = pipewrite(f->pipe, addr, n);
  } else if(f->type == FD_DEVICE){
    if(f->major < 0 || f->major >= NDEV || !devsw[f->major].write)
      return -1;
    ret = devsw[f->major].write(1, addr, n);
  } else if(f->type == FD_INODE){
    // write a few blocks at a time to avoid exceeding
    // the maximum log transaction size, including
    // i-node, indirect block, allocation blocks,
    // and 2 blocks of slop for non-aligned writes.
    // this really belongs lower down, since writei()
    // might be writing a device like the console.
    int max = ((MAXOPBLOCKS-1-1-2) / 2) * BSIZE;
    int i = 0;
    while(i < n){
      int n1 = n - i;
      if(n1 > max)
        n1 = max;

      begin_op();
      ilock(f->ip);
      if ((r = writei(f->ip, 1, addr + i, f->off, n1)) > 0)
        f->off += r;
      iunlock(f->ip);
      end_op();

      if(r != n1){
        // error from writei
        break;
      }
      i += r;
    }
    ret = (i == n ? n : -1);
  } else {
    panic("filewrite");
  }

  return ret;
}
int
mmpfilewrite(struct file *f, uint64 addr, int n)
{
  int r, ret = 0;
  if(f->writable == 0)
    return -1;
  if(f->type == FD_PIPE){
    ret = pipewrite(f->pipe, addr, n);
  } else if(f->type == FD_DEVICE){
    if(f->major < 0 || f->major >= NDEV || !devsw[f->major].write)
      return -1;
    ret = devsw[f->major].write(1, addr, n);
  } else if(f->type == FD_INODE){
    // write a few blocks at a time to avoid exceeding
    // the maximum log transaction size, including
    // i-node, indirect block, allocation blocks,
    // and 2 blocks of slop for non-aligned writes.
    // this really belongs lower down, since writei()
    // might be writing a device like the console.
    int max = ((MAXOPBLOCKS-1-1-2) / 2) * BSIZE;
    int i = 0;
    while(i < n){
      int n1 = n - i;
      if(n1 > max)
        n1 = max;
      begin_op();
      ilock(f->ip);
      // printf("%dfuck\n",*((char*)(addr+i)));
      // printf("%ddickoff\n",n1);
      if ((r = writei(f->ip, 1, addr + i, f->off, n1)) > 0){

        f->off+=r;

      }
        // f->off += r;

      iunlock(f->ip);
      end_op();

      if(r != n1){
        // error from writei
        break;
      }
      i += r;
    }
    ret = (i == n ? n : -1);
  } else {
    panic("filewrite");
  }

  return ret;
}
uint64 mmap(uint64 vaddr,int length,int prot,int flags,struct file* fd,int off)
{
  uint64 va=0;
  pte_t* pte;
  struct proc* p = myproc();
  int sz = 0;
  uint64 staddr = 0;
  int flpg = -1;
  int k;
  int exi=0;
  int i = 0;
  if(vaddr == 0)
  {
      for(;va<MAXVA&&sz<length;va+=PGSIZE)
      {
          pte = walk(p->pagetable,va,0);
          for(k=0;k<16;k++)
          {
            exi = 0;
            if(VMA[k].v==1)
            {
              if(VMA[k].staddr<=va&&va<VMA[k].staddr+VMA[k].length)
              {
                exi = 1;
                break;
              }
            }
          }
          if(exi==1)
          {
            continue;
          }
          if(pte==0)
          {
            if(flpg==-1)
            {
                flpg = 1;
                staddr = va;
            }
            sz+=PGSIZE;
          }
          else{
            if(flpg == 1)
            {
              flpg = -1;
              staddr = 0;
              sz = 0;
            }
          }
      }
      if(staddr==0&&flpg==-1)return -1;
      filedup(fd);
      
      for(i = 0;i<16;i++)
      {
        if(VMA[i].v == 0)
        {
            if(fd->readable&&fd->writable)
            {
              if((prot&0x1)&&(prot&0x2))
              {

              }
              else
              {
                
              }
            }
            else if(fd->readable&&!fd->writable)
            {
              if(flags == 0x02)
              {
                if((prot&0x1))
                {
                  
                }
                else{
                  return -1;
                }
              }
              else{
              if(((prot&0x1)&&!(prot&0x2)))
              {
              }
              else return -1;
              }
            }
            else if(!fd->readable&&fd->writable)
            {
              if(!(prot&0x1)&&(prot&0x2))
              {

              }
              else return -1;
            }
            else 
            {
              if(!(prot&0x1)&&!(prot&0x2))
              {

              }
              else return -1;
            }
            VMA[i].f = fd;
            VMA[i].flag = flags;
            VMA[i].prot = prot;
            VMA[i].length = length;
            VMA[i].off = off;
            VMA[i].staddr = staddr;
            VMA[i].curstaddr = staddr;
            VMA[i].curoff = off;
            VMA[i].curlength = length;
            VMA[i].v = 1;
            VMA[i].refcnt=1;
            break;

        }
        
      }
  }
  if(p->vma[i]==0)
  p->vma[i] = staddr;
  else return -1; 
  return staddr;
}
uint64 munmap(uint64 vaddr,int length)
{
  int i=0;
  struct proc* p = myproc();
  for(;i<16;i++)
  {
    if(VMA[i].v == 1){
    if(vaddr>=VMA[i].staddr&&vaddr<VMA[i].staddr+VMA[i].length)
    {
        if(VMA[i].flag == 0x01)
        {
          if(walkaddr(myproc()->pagetable,PGROUNDDOWN(vaddr))==0)return 0; 
          // uint64 pa = walkaddr(myproc()->pagetable,PGROUNDDOWN(vaddr));
          // printf("%dgetting\n",*((char*)(pa-4097)));
          mmpfilewrite(VMA[i].f,vaddr,(uint64)length);
          
        }
        uvmunmap(myproc()->pagetable,vaddr,length/PGSIZE,1);
        p->umlen[i]+=length;
        if(VMA[i].length == p->umlen[i])
        {
          VMA[i].refcnt--;
          if(VMA[i].refcnt == 0)
            VMA[i].v = 0;
          p->vma[i]=0;
          p->umlen[i]=0;
          filedec(VMA[i].f);
        }
        
        return 0;
    }
    }
  }
  return -1;
    
}
