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
  if (is_user_vaddr(f->esp)) {
    int32_t *sys_call_number = (int32_t *) f->esp;

    switch(*sys_call_number) {
    /* void handlers */
      /* void arguemnt*/
      case SYS_HALT:
      /* single argument*/
      case SYS_EXIT:
      case SYS_CLOSE:
      case SYS_SEEK:
    /* handlers with a return value*/
      /* single argument*/
      case SYS_EXEC:  
      case SYS_WAIT:  
      case SYS_TELL:    
      case SYS_OPEN:    
      case SYS_FILESIZE:
      case SYS_REMOVE: 
      /* two arguments */ 
      case SYS_CREATE: 
      /* three arguemnts */
      case SYS_READ:    
      case SYS_WRITE:   
         printf ("System Call! System Call Number: %d\n", *sys_call_number);
         break;
      default:
        printf ("Unsupported System Call\n");
        thread_exit ();
    }
  } else {
    printf ("invalid memory address!\n");
    thread_exit ();
  }
}
