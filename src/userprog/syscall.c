#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/vaddr.h"

static void syscall_handler (struct intr_frame *);

void
syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

static void
syscall_handler (struct intr_frame *f UNUSED) 
{
  //TODO retrieve the system call number from the User Stack
  //Pre-Requisite we need to be able to read/write data from the User Virtual Adress Space.

  //validate the alleged pointer to System Call Number.

  if (is_user_vaddr(f->esp)) {
    int32_t *sys_call_number = (int32_t *) f->esp;
    printf ("System Call! System Call Number: %d\n", *sys_call_number);

  }else {
    printf ("invalid memory address!\n");
    thread_exit ();
  }
}
