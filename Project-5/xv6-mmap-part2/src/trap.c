#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "x86.h"
#include "traps.h"
#include "spinlock.h"
#include "mman.h"

#define NULL (struct mmapped_region*)0
#define PTE_D 0x040

// Interrupt descriptor table (shared by all CPUs).
struct gatedesc idt[256];
extern uint vectors[];  // in vectors.S: array of 256 entry pointers
struct spinlock tickslock;
uint ticks;

//pagefault_handler()
void pagefault_handler(struct trapframe *tf){

  //control Register 2 contains the faulting page address
  uint fault_addr = rcr2();

  //debugging statement as mentioned in the project handout
  cprintf("============in pagefault_handler============\n");
  cprintf("pid %d %s: trap %d err %d on cpu %d "
  "eip 0x%x addr 0x%x\n",
  myproc()->pid, myproc()->name, tf->trapno,
  tf->err, cpuid(), tf->eip, fault_addr);


  //round the faulting address down to a page boundary
  fault_addr = PGROUNDDOWN(fault_addr);

  struct mmapped_region *node = myproc()->mmap_regions;
  int flag = 0;

  //search for the faulting address in the process's mmaped linked list
  while(node != NULL){
    if(((uint)(node->start_addr) <= fault_addr) && ((uint)(node->start_addr + node->length) > fault_addr)){
      if((node->prot & PROT_WRITE) != 0){
        flag = 1;
      }
      if((T_ERR_PGFLT_W & tf->err) == 0){
      	flag = 1;
      }
      break;
    }else{
      node = node->next;
    }
  }

  //map a page around the faulting address if the faulting address was valid else return
  if(flag == 1){
  	
    char *va = kalloc();
    if(va == 0){
      exit();
    }

    //clear the page allocated
    memset(va, 0, 4096);

    //set permissions appropriately
    int perm;
    if(node->prot == PROT_WRITE){
      perm = PTE_W|PTE_U;
    }else{
      perm = PTE_U;
    }

    if(mappages(myproc()->pgdir, (char*)fault_addr, PGSIZE, V2P(va), perm) != 0){
      kfree(va);
      exit();
    }

    switchuvm(myproc());

    //for file backed mmap
    if(node->region_type == MAP_FILE){
      if(myproc()->ofile[node->fd] != 0){
        fileseek(myproc()->ofile[node->fd], node->offset);
        fileread(myproc()->ofile[node->fd], va, node->length);
        pde_t *pde = &(myproc()->pgdir)[PDX(node->start_addr)];
        pte_t *pgtab = (pte_t*)P2V(PTE_ADDR(*pde));
        pte_t *pte = &pgtab[PTX(node->start_addr)];
        *pte = (*pte) & (~PTE_D);
      }
    }
  }else{  //if page fault address was not valid
      exit();
  }
  return;
}

void
tvinit(void)
{
  int i;

  for(i = 0; i < 256; i++)
    SETGATE(idt[i], 0, SEG_KCODE<<3, vectors[i], 0);
  SETGATE(idt[T_SYSCALL], 1, SEG_KCODE<<3, vectors[T_SYSCALL], DPL_USER);

  initlock(&tickslock, "time");
}

void
idtinit(void)
{
  lidt(idt, sizeof(idt));
}

//PAGEBREAK: 41
void
trap(struct trapframe *tf)
{
  if(tf->trapno == T_SYSCALL){
    if(myproc()->killed)
      exit();
    myproc()->tf = tf;
    syscall();
    if(myproc()->killed)
      exit();
    return;
  }
  
  switch(tf->trapno){
  case T_IRQ0 + IRQ_TIMER:
    if(cpuid() == 0){
      acquire(&tickslock);
      ticks++;
      wakeup(&ticks);
      release(&tickslock);
    }
    lapiceoi();
    break;
  case T_IRQ0 + IRQ_IDE:
    ideintr();
    lapiceoi();
    break;
  case T_IRQ0 + IRQ_IDE+1:
    // Bochs generates spurious IDE1 interrupts.
    break;
  case T_IRQ0 + IRQ_KBD:
    kbdintr();
    lapiceoi();
    break;
  case T_IRQ0 + IRQ_COM1:
    uartintr();
    lapiceoi();
    break;
  case T_IRQ0 + 7:
  case T_IRQ0 + IRQ_SPURIOUS:
    cprintf("cpu%d: spurious interrupt at %x:%x\n",
            cpuid(), tf->cs, tf->eip);
    lapiceoi();
    break;
  case T_PGFLT: //pagefault check
    pagefault_handler(tf);
    break;
  //PAGEBREAK: 13
  default:
    if(myproc() == 0 || (tf->cs&3) == 0){
      // In kernel, it must be our mistake.
      cprintf("unexpected trap %d from cpu %d eip %x (cr2=0x%x)\n",
              tf->trapno, cpuid(), tf->eip, rcr2());
      panic("trap");
    }
    // In user space, assume process misbehaved.
    cprintf("pid %d %s: trap %d err %d on cpu %d "
            "eip 0x%x addr 0x%x--kill proc\n",
            myproc()->pid, myproc()->name, tf->trapno,
            tf->err, cpuid(), tf->eip, rcr2());
    myproc()->killed = 1;
  }

  // Force process exit if it has been killed and is in user space.
  // (If it is still executing in the kernel, let it keep running
  // until it gets to the regular system call return.)
  if(myproc() && myproc()->killed && (tf->cs&3) == DPL_USER)
    exit();

  // Force process to give up CPU on clock tick.
  // If interrupts were on while locks held, would need to check nlock.
  if(myproc() && myproc()->state == RUNNING &&
     tf->trapno == T_IRQ0+IRQ_TIMER)
    yield();

  // Check if the process has been killed since we yielded
  if(myproc() && myproc()->killed && (tf->cs&3) == DPL_USER)
    exit();
}
