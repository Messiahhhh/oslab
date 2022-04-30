#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"

uint64
sys_exit(void)
{
  int n;
  if(argint(0, &n) < 0)
    return -1;
  exit(n);
  return 0;  // not reached
}

uint64
sys_getpid(void)
{
  return myproc()->pid;
}

uint64
sys_fork(void)
{
  return fork();
}

uint64
sys_wait(void)
{
  uint64 p;
  if(argaddr(0, &p) < 0)
    return -1;
  return wait(p);
}

uint64
sys_sbrk(void)
{
  int addr;
  int n;

  if(argint(0, &n) < 0)
    return -1;
  addr = myproc()->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

uint64
sys_sleep(void)
{
  int n;
  uint ticks0;
  if(argint(0, &n) < 0)
    return -1;
  
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(myproc()->killed){
      release(&tickslock);
      return -1;
    }
    backtrace();
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

uint64
sys_kill(void)
{
  int pid;

  if(argint(0, &pid) < 0)
    return -1;
  return kill(pid);
}

// return how many clock tick interrupts have occurred
// since start.
uint64
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

uint64 
sys_sigalarm(void)
{
  int ticks;
  uint64 hand;
  if(argint(0,&ticks)<0)return -1;
  if(argaddr(1,&hand)<0)return -1;
  struct proc* p = myproc();
  if(ticks==0&&hand==0){
    p->ticks=-1;
    p->lticks=-1;
    p->handler=0;
    return 0;
  }
  p->ticks = ticks;
  p->handler = hand;
  p->lticks = ticks;

  
  return 0;
}
uint64 
sys_sigreturn(void)
{

  struct proc* p = myproc();
        p->trapframe->a0 = p->save->a0;
        p->trapframe->a1 = p->save->a1;
        p->trapframe->a2 = p->save->a2;
        p->trapframe->a3 = p->save->a3;
        p->trapframe->a4 = p->save->a4;
        p->trapframe->a5 = p->save->a5;
        p->trapframe->a6 = p->save->a6;
        p->trapframe->a7 = p->save->a7;
        p->trapframe->epc = p->save->epc;
        p->trapframe->gp = p->save->gp;
        p->trapframe->kernel_hartid = p->save->kernel_hartid;
        p->trapframe->kernel_satp = p->save->kernel_satp;
        p->trapframe->kernel_sp = p->save->kernel_sp;
        p->trapframe->kernel_trap = p->save->kernel_trap;
        p->trapframe->ra = p->save->ra;
        p->trapframe->s0 = p->save->s0;
        p->trapframe->s10 = p->save->s10;
        p->trapframe->s11 = p->save->s11;
        p->trapframe->s1 = p->save->s1;
        p->trapframe->s2 = p->save->s2;
        p->trapframe->s3 = p->save->s3;
        p->trapframe->s4 = p->save->s4;
        p->trapframe->s5 = p->save->s5;
        p->trapframe->s6 = p->save->s6;
        p->trapframe->s7 = p->save->s7;
        p->trapframe->s8 = p->save->s8;
        p->trapframe->s9 = p->save->s9;
        p->trapframe->sp = p->save->sp;
        p->trapframe->t0 = p->save->t0;
        p->trapframe->t1 = p->save->t1;
        p->trapframe->t2 = p->save->t2;
        p->trapframe->t3 = p->save->t3;
        p->trapframe->t4 = p->save->t4;
        p->trapframe->t5 = p->save->t5;
        p->trapframe->t6 = p->save->t6;
        p->trapframe->tp = p->save->tp;
        p->flag=0;
  return 0;
}

