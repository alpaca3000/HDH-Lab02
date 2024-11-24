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
  uint64 start;
  int pnum;
  uint64 dest;
  argaddr(0,&start);
  argint(1,&pnum);
  argaddr(2,&dest);


  uint64 maskbits = 0;
  struct proc *proc = myproc();
  for (int i = 0; i < pnum ; i++ ){
    pte_t *pte = walk(proc->pagetable, start + PGSIZE*i, 0);// lấy page table entry, nếu là pte A (page tablea entry Accessed) thì thực hiện cập nhật bộ nhớ đệm
    if(*pte & PTE_A) {
      maskbits |= (1L << i);
      *pte &= ~PTE_A;
    }
    *pte = ((*pte&PTE_A) ^ *pte) ^ 0 ;// thực hiện xóa bit PTE_A sau khi kiểm tra
  }
  if (copyout(proc->pagetable, dest, (char*)&maskbits, sizeof(maskbits)) < 0)// Ghi bitmap vào địa chỉ kết thúc, nếu việc ghi không thành công thì gửi về lỗi
    return -1;

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
