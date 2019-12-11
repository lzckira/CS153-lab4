#include "param.h"
#include "types.h"
#include "defs.h"
#include "x86.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "spinlock.h"

struct {
  struct spinlock lock;
  struct shm_page {
    uint id;
    char *frame;
    int refcnt;
  } shm_pages[64];
} shm_table;

void shminit() {
  int i;
  initlock(&(shm_table.lock), "SHM lock");
  acquire(&(shm_table.lock));
  for (i = 0; i< 64; i++) {
    shm_table.shm_pages[i].id =0;
    shm_table.shm_pages[i].frame =0;
    shm_table.shm_pages[i].refcnt =0;
  }
  release(&(shm_table.lock));
}

int shm_open(int id, char **pointer) {
  int i;
  acquire(&(shm_table.lock));
  struct proc *curproc = myproc();

// lab4, case 1
  for (i = 0; i < 64; i++) {
	  char* va = (char*)PGROUNDUP(curproc->sz);
	  if (shm_table.shm_pages[i].id == id) {
		  shm_table.shm_pages[i].refcnt++;
	  }
// lab4, case 2
	  else {
		  shm_table.shm_pages[i].id = id;
		  shm_table.shm_pages[i].frame = kalloc();  
		  shm_table.shm_pages[i].refcnt=1;
		  memset(shm_table.shm_pages[i].frame, 0, PGSIZE);		
	  }
	  mappages(curproc->pgdir, va, PGSIZE, V2P(shm_table.shm_pages[i].frame), PTE_W|PTE_U);
	  *pointer = va;
	  curproc->sz += PGSIZE;
  }
release(&(shm_table.lock));
return 0; 
}


int shm_close(int id) {
   int tempRef;
   int i;

   acquire(&(shm_table.lock));
   for (i = 0; i < 64; i++) {
       if (shm_table.shm_pages[i].id == id) {
           shm_table.shm_pages[i].refcnt--;
       }
       tempRef = --shm_table.shm_pages[i].refcnt;
       if (tempRef == 0) {
            shm_table.shm_pages[i].frame = 0;
            shm_table.shm_pages[i].id = 0;
       }
   }
   
   release(&(shm_table.lock));

   return 0; 
}
