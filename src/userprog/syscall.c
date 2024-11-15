#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include <inttypes.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/vaddr.h"

static void syscall_handler (struct intr_frame *);

static void write (int32_t *args, uint32_t *returnValue);
static void exit (int32_t *args, uint32_t *returnValue UNUSED);

static void halt (int32_t *args UNUSED, uint32_t *returnValue UNUSED);
static void exec (int32_t *args UNUSED, uint32_t *returnValue UNUSED);
static void wait (int32_t *args UNUSED, uint32_t *returnValue UNUSED);
static void create (int32_t *args UNUSED, uint32_t *returnValue UNUSED);
static void remove (int32_t *args UNUSED, uint32_t *returnValue UNUSED);
static void open (int32_t *args UNUSED, uint32_t *returnValue UNUSED);
static void filesize (int32_t *args UNUSED, uint32_t *returnValue UNUSED);
static void read (int32_t *args UNUSED, uint32_t *returnValue UNUSED);
static void seek (int32_t *args UNUSED, uint32_t *returnValue UNUSED);
static void tell (int32_t *args UNUSED, uint32_t *returnValue UNUSED);
static void close (int32_t *args UNUSED, uint32_t *returnValue UNUSED);

static handler sys_call_handlers[19] = {
    halt,                   /* Halt the operating system. */
    exit,                   /* Terminate this process. */
    exec,                   /* Start another process. */
    wait,                   /* Wait for a child process to die. */
    create,                 /* Create a file. */
    remove,                 /* Delete a file. */
    open,                   /* Open a file. */
    filesize,               /* Obtain a file's size. */
    read,                   /* Read from a file. */
    write,                  /* Write to a file. */
    seek,                   /* Change position in a file. */
    tell,                   /* Report current position in a file. */
    close,                  /* Close a file. */
};

void
syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

static void
syscall_handler (struct intr_frame *f) 
{
  if (is_user_vaddr(f->esp)) {
    int32_t *stack_pointer = (int32_t *) f->esp;
    int32_t sys_call_number = *stack_pointer;

    ASSERT (sys_call_number >= 0);

    if (sys_call_number <= SYS_CLOSE) {
      printf ("System Call! System Call Number: %d\n", sys_call_number);

      /* invoked the handler corresponding to the system call number */
      sys_call_handlers[sys_call_number](++stack_pointer, &f->eax);
    } else {
      printf ("Unsupported System Call\n");
      thread_exit ();
    }

  } else {
    printf ("invalid memory address!\n");
    thread_exit ();
  }
}

/* void exit (int status) */
static void exit (int32_t *args, uint32_t *returnValue UNUSED)
{
    /* status must be stored somewhere, maybe in thread struct*/
    struct thread *cur = thread_current ();
    cur->exit_status = (int) *args;
    thread_exit ();
}

/* int write (int fd, const void *buffer, unsigned size) */
static void write (int32_t *args, uint32_t *returnValue) 
{
  /* arguments */
  int fd = args[0];
  const void *buffer = (void *) args[1];
  unsigned size = args[3];

  if (fd == 1) {
    enum intr_level old_level = intr_disable ();
    putbuf (buffer, size);
    intr_set_level (old_level);
    /* putbuf () is guaranteed to successfully print 'size' bytes to console*/
    *returnValue = size;
    return;
  }
  //todo get file path from fd
  *returnValue = 0;
  return;
}

static void halt (int32_t *args UNUSED, uint32_t *returnValue UNUSED)
{
  return;
}
static void exec (int32_t *args UNUSED, uint32_t *returnValue UNUSED)
{
  return;
}
static void wait (int32_t *args UNUSED, uint32_t *returnValue UNUSED)
{
  return;
}
static void create (int32_t *args UNUSED, uint32_t *returnValue UNUSED)
{
  return;
}
static void remove (int32_t *args UNUSED, uint32_t *returnValue UNUSED)
{
  return;
}
static void open (int32_t *args UNUSED, uint32_t *returnValue UNUSED)
{
  return;
}
static void filesize (int32_t *args UNUSED, uint32_t *returnValue UNUSED)
{
  return;
}
static void read (int32_t *args UNUSED, uint32_t *returnValue UNUSED)
{
  return;
}
static void seek (int32_t *args UNUSED, uint32_t *returnValue UNUSED)
{
  return;
}
static void tell (int32_t *args UNUSED, uint32_t *returnValue UNUSED)
{
  return;
}
static void close (int32_t *args UNUSED, uint32_t *returnValue UNUSED)
{
  return;
}
