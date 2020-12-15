#include "types.h"
#include "x86.h"
#include "defs.h"
#include "date.h"
#include "memlayout.h"
#include "mmu.h"
#include "param.h"
#include "proc.h"
#include "mman.h"

#define MMAPBASE 0x40000000
#define NULL (struct mmapped_region*)0



//this function removes mmaped region node from the linked list (this function is called by munmap)
static void remove_node(struct mmapped_region *node, struct mmapped_region *prev){

  //if the node to be removed is the head node
  if (node == myproc()->mmap_regions){
    //if further nodes exist, make the next node as head
    if(myproc()->mmap_regions->next != NULL){
      myproc()->mmap_regions = myproc()->mmap_regions->next;
    }else{  //if no other nodes are in the linked list, then just point to NULL
      myproc()->mmap_regions = NULL;
    }
  }else{  //if it's not the head node then just connect the previous node to the next node thus unlinking the node to be removed
    prev->next = node->next;
  }

  kmfree(node);

}



void* mmap(void* addr, uint length, int prot, int flags, int fd, int offset){

  //check if length is valid
  if(length < 1){
    return (void*)-1;
  }

  //check if address is valid
  if(addr < (void*)0 || addr >= (void*)KERNBASE){
    return (void*)-1;
  }

  //increase process size
  uint size1 = myproc()->sz;
  uint size2 = myproc()->sz + length;
  myproc()->sz = size2;

  //allocate space for the new region using kmalloc
  struct mmapped_region* mr = (struct mmapped_region*)kmalloc(sizeof(struct mmapped_region));

  //if kmalloc was unsuccessful
  if(mr == NULL){
    return (void*)-1;
  }

  addr = (void*)(PGROUNDDOWN(size1) + MMAPBASE);
  mr->start_addr = addr;

  //check flags and file descriptors
  if(flags == MAP_ANONYMOUS){
    //fd has to be -1 for anonymous mmap
    if(fd != -1){
      kmfree(mr);
      return (void*)-1;
    }
  }else if(flags == MAP_FILE){
    //fd cannot be -1 or lesser for file backed mmap
    if(fd <= -1){
      kmfree(mr);
      return (void*)-1;
    }else{
      fd = fdalloc(myproc()->ofile[fd]);
      if(fd < 0){
        return (void*)-1;
      }
      filedup(myproc()->ofile[fd]);
      mr->fd = fd;
    }
  }


  //fill in the remaining fields for the newly mapped region
  mr->length = length;
  mr->prot = prot;
  mr->region_type = flags;
  mr->offset = offset;
  mr->next = NULL;


  //if this is the first mapped region for the process 
  if(myproc()->total_regions == 0){
    myproc()->mmap_regions = mr;
  }else{  //else add newly mapped region to linked list
    struct mmapped_region *node = myproc()->mmap_regions;

    while(node->next != NULL){
      //if address is already allocated to a node then increment the address and start by checking again from head node of linked list
      if(addr == node->start_addr){
        addr += PGROUNDDOWN(node->length + 4096);
        node = myproc()->mmap_regions;
      }else if(addr >= (void*)KERNBASE){ //memory is going into kernel space so exit
        kmfree(mr);
        return (void*)-1;
      }
      node = node->next;
    }

    //check the last node
    if(addr == node->start_addr){
      addr += PGROUNDDOWN(node->length + 4096);
    }

    //add new region to end of list
    node->next = mr;
  }

  (myproc()->total_regions)++;
  mr->start_addr = addr;
  return mr->start_addr;
}





int munmap(void *addr, uint length){

  //check if length is valid
  if(length < 1){
    return -1;
  }

  //check if address is valid
  if(addr < (void*)0 || addr >= (void*)KERNBASE){
    return -1;
  }

  //if there are no mmaped regions, return -1
  if(myproc()->total_regions == 0){
    return -1;
  }

  int size = 0;

  struct mmapped_region *prev = myproc()->mmap_regions;
  struct mmapped_region *next = myproc()->mmap_regions->next;

  //if the head node matches the addr and length to be munmaped
  if((addr == myproc()->mmap_regions->start_addr) && (length == myproc()->mmap_regions->length)){

    //deallocate memory from current process
    myproc()->sz = deallocuvm(myproc()->pgdir, myproc()->sz, myproc()->sz-length);
    switchuvm(myproc());
    (myproc()->total_regions)--;  

    //check if it was file based mapping
    if(myproc()->mmap_regions->region_type == MAP_FILE){
      fileclose(myproc()->ofile[myproc()->mmap_regions->fd]);
      myproc()->ofile[myproc()->mmap_regions->fd] = 0;
    }

    if(myproc()->mmap_regions->next != NULL){
      size = myproc()->mmap_regions->next->length;
      remove_node(myproc()->mmap_regions, 0);
      myproc()->mmap_regions->length = size;
    }else{
      remove_node(myproc()->mmap_regions, 0);
    }

    return 0;
  }

  //if it's not the head node, loop over remaining nodes
  while(next != NULL){
    if((addr == next->start_addr) && (length == next->length)){
      //deallocate memory from process
      myproc()->sz = deallocuvm(myproc()->pgdir, myproc()->sz, myproc()->sz - length);
      switchuvm(myproc());
      (myproc()->total_regions)--;  
      
      //check if it was file based mapping
      if(next->region_type == MAP_FILE)
      {
        fileclose(myproc()->ofile[next->fd]);
        myproc()->ofile[next->fd] = 0;
      }

      //remove the node from the linked list
      size = next->next->length;
      remove_node(next, prev);
      prev->next->length = size;

      return 0;
    }

    //for next iteration
    prev = next;
    next = prev->next;
  }

  //if no address and length matched, then return -1
  return -1;
}





int msync(void* start_addr, uint length){

  //check if length is valid
  if(length < 1){
    return -1;
  }

  //check if address is valid
  if(start_addr < (void*)0 || start_addr >= (void*)KERNBASE){
    return -1;
  }

  //if there are no mmaped regions, return -1
  if(myproc()->total_regions == 0){
    return -1;
  }

  struct mmapped_region *node = myproc()->mmap_regions;

  while(node != NULL){
    if((start_addr == node->start_addr) && (length == node->length)){
      fileseek(myproc()->ofile[node->fd], node->offset);
      filewrite(myproc()->ofile[node->fd], start_addr, length);
      return 0;
    }
    node = node->next;
  }

  //if no address and length matched, then return -1
  return -1;
}
