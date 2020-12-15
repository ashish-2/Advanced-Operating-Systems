#include "types.h"
#include "x86.h"
#include "defs.h"
#include "date.h"
#include "memlayout.h"
#include "mmu.h"
#include "param.h"
#include "proc.h"

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


void* mmap(void *addr, uint length, int prot, int flags, int fd, int offset){

  //check if length is valid
  if(length < 1){
    return (void*)-1;
  }

  //check if address is valid
  if(addr < (void*)0 || addr >= (void*)KERNBASE){
    return (void*)-1;
  }

  //increase the address space of the process
  uint size1 = myproc()->sz;
  uint size2 = myproc()->sz + length;
  myproc()->sz = allocuvm(myproc()->pgdir, size1, size2);

  //if increasing the address space is not possible return -1
  if (myproc()->sz == 0){
    return (void*)-1;
  }

  addr = (void*)PGROUNDDOWN(size1);
  switchuvm(myproc());

  //allocate space for the new region using kmalloc
  struct mmapped_region *mr = (struct mmapped_region*)kmalloc(sizeof(struct mmapped_region*));

  //if kmalloc was unsuccessful
  if (mr == NULL){
    deallocuvm(myproc()->pgdir, size2, size1);
    return (void*)-1;
  }


  //fill in the fields for the newly mapped region
  mr->start_addr = addr;
  mr->length = length;
  mr->region_type = ANONYMOUS;
  mr->offset = offset;
  mr->next = NULL;

  //if this is the first mmaped region of the process then it is the start of the linked list 
  if (myproc()->total_regions == 0){
    myproc()->mmap_regions = mr;
  }else{ //else add newly mapped region to linked list

    //check if address is already allocated to head node of linked list, if so increment by 4096
    if (addr == myproc()->mmap_regions->start_addr){
      addr += PGROUNDDOWN(length + 4096);
    }

    //traverse the linked list to check if the address is already allocated to other nodes
    struct mmapped_region* node = myproc()->mmap_regions;

    //traverse till we reach the last node
    while (node->next != NULL){

      //if address is already allocated to a node then increment the address and start by checking again from head node of linked list
      if(addr == node->start_addr){
        addr += PGROUNDDOWN(length + 4096);
        node = myproc()->mmap_regions;
      }else if(addr >= (void*)KERNBASE){ //else if no more memory is left
        kmfree(mr);
        deallocuvm(myproc()->pgdir, size2, size1);
        return (void*)-1;
      }

      node = node->next;

    }

    //check is the address clashes with the last node, if yes again increment the address
    if (addr == node->start_addr){
      addr += PGROUNDDOWN(length + 4096);
    }

    //check if the address is still within limits
    if(addr >= (void*)KERNBASE){
      kmfree(mr);
      deallocuvm(myproc()->pgdir, size2, size1);
      return (void*)-1;
    }else{
      //link the newly mapped region into the linked list, newly attached nodes will be at the tail end of the linked list
      node->next = mr;
    }
  }

  //increment the total regions field of the process
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

  //if  the head node matches the addr and length to be munmaped
  if (addr == myproc()->mmap_regions->start_addr && length == myproc()->mmap_regions->length){

    //deallocate memory from current process
    myproc()->sz = deallocuvm(myproc()->pgdir, myproc()->sz, myproc()->sz-length);
    switchuvm(myproc());
    (myproc()->total_regions)--;  

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

    if (addr == next->start_addr && length == next->length){

      //deallocate memory from process
      myproc()->sz = deallocuvm(myproc()->pgdir, myproc()->sz, myproc()->sz-length);
      switchuvm(myproc());
      (myproc()->total_regions)--;
      
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
