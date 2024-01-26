#include "types.h"
#include "riscv.h"
#include "param.h"
#include "defs.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"

uint64
sys_exit(void)
{
  int n;
  argint(0, &n);
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
  if(growproc(n) < 0)
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
  while(ticks - ticks0 < n){
    if(killed(myproc())){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}


#ifdef LAB_PGTBL
int
sys_pgaccess(void)
{
  // lab pgtbl: your code here.
  // get user-space system call parameters
  uint64 startPageAddr;
  int numPage;
  uint64 resultAddr;
  argaddr(0, &startPageAddr);
  argint(1, &numPage);
  argaddr(2, &resultAddr);

  // kernel result buffer
  uint64 result = 0;

  // get the user-space page table
  struct proc *p = myproc();
  pagetable_t userPageTable = p->pagetable;

  // fill in the result buffer by checking the `A` bit of PTE
  for (int i = 0; i < numPage; i++) {
    // page walk to find the PA of PTE
    uint64* pa = walk(userPageTable, startPageAddr, 0);
    pte_t pte = *pa;
    // get the `A` bit of PTE
    result |= (((pte & PTE_A) >> 6) << i);
    // clear the PTE_A bit
    *pa = (pte & ~PTE_A);
    // update user-space start address
    startPageAddr += PGSIZE;
  }

  // copy result to user-space
  copyout(userPageTable, resultAddr, (char*)(&result), 4);

  return 0;
}
#endif

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
