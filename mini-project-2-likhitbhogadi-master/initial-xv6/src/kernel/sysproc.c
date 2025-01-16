#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"

uint64
sys_exit(void)
{
  int n;
  argint(0, &n);
  exit(n);
  return 0; // not reached
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
  argaddr(0, &p);
  return wait(p);
}

uint64
sys_sbrk(void)
{
  uint64 addr;
  int n;

  argint(0, &n);
  addr = myproc()->sz;
  if (growproc(n) < 0)
    return -1;
  return addr;
}

uint64
sys_sleep(void)
{
  int n;
  uint ticks0;

  argint(0, &n);
  acquire(&tickslock);
  ticks0 = ticks;
  while (ticks - ticks0 < n)
  {
    if (killed(myproc()))
    {
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

uint64
sys_kill(void)
{
  int pid;

  argint(0, &pid);
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
sys_waitx(void)
{
  uint64 addr, addr1, addr2;
  uint wtime, rtime;
  argaddr(0, &addr);
  argaddr(1, &addr1); // user virtual memory
  argaddr(2, &addr2);
  int ret = waitx(addr, &wtime, &rtime);
  struct proc *p = myproc();
  if (copyout(p->pagetable, addr1, (char *)&wtime, sizeof(int)) < 0)
    return -1;
  if (copyout(p->pagetable, addr2, (char *)&rtime, sizeof(int)) < 0)
    return -1;
  return ret;
}

uint64
sys_hello(void){
  printf("\033[1;93mHello World\n\033[0m");
  return 12;
}

uint64
sys_getSysCount(void)
{
  int mask;
  argint(0, &mask); // extracts required argument as the argument sent is void

  // struct proc *p = myproc(); // will point to the process control block (PCB) of the currently running process, allowing access to its attributes.
  int count = 0;

  // Iterate through all possible system calls
  for(int i = 1; i < 33; i++) {
    if(mask & (1 << i)) {
      // // Add the count for this system call
      // count += p->syscall_counts[i-1];

      // // Also add counts from child processes
      // struct proc *child;
      // acquire(&tickslock);
      // for(child = proc; child < &proc[NPROC]; child++) {
      //     if(child->parent == p) {
      //         count += child->syscall_counts[i];
      //     }
      // }
      // release(&tickslock);
      // // locks not handled yet!!

      count = shared_array[i-1];

      break; // Since only one bit is set, we can break after finding it
    }
  }

  acquire(&shared_array_lock);
  memset(shared_array, 0, SHARED_ARRAY_SIZE);
  release(&shared_array_lock);

  return count;
}
// might not be correct !!

// System call handler for settickets
uint64
sys_settickets(void)
{
  int n;
  
  // Retrieve the first argument (number of tickets)
  argint(0, &n);
  
  if(n < 1)
    return -1;   // Invalid number of tickets
  
  struct proc *p = myproc();
  
  p->tickets = n; // Set the number of tickets
  
  return p->tickets; // Return the updated number of tickets
}

uint64
sys_sigalarm(void)
{
  int interval;
  uint64 handler;

  argint(0, &interval);
  argaddr(1, &handler);
  
  
  struct proc *p = myproc();
  p->alarm_interval = interval;
  p->alarm_handler = handler;
  p->ticks_count = 0;
  p->alarm_active = 1; // indicating that the sigalarm has been called 
  // and that alarm handler is running??
  return 0;
}

uint64
sys_sigreturn(void)
{
  struct proc *p = myproc();
  if(p->alarm_tf) {
    memmove(p->trapframe, p->alarm_tf, sizeof(struct trapframe));
    p->ticks_count = 0;
    p->alarm_active = 1;  
  }
  return p->trapframe->a0;
}
