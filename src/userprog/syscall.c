#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/vaddr.h"

static void syscall_handler (struct intr_frame *);

/* Lock used by allocate_fd(). */
static struct lock fd_lock;

/* Returns a file descriptor to use for a new file. */
static int
allocate_fd (void) 
{
  static int next_fd = 2;
  int fd;

  lock_acquire (&fd_lock);
  fd = next_fd++;
  lock_release (&fd_lock);

  return fd;
}

/* Returns a hash for file p via allocation of an fd. */
unsigned
file_hash (const struct hash_elem *f_, void *aux UNUSED)
{
  const struct file_elem *f = hash_entry (f_, struct file_elem, hash_elem);
  return allocate_fd()
}

/* Returns true if the fd of file a precedes file b. */
bool
file_less (const struct hash_elem *a_, const struct hash_elem *b_,
void *aux UNUSED)
{
  const struct file_elem *a = hash_entry (a_, struct file_elem, hash_elem);
  const struct file_elem *b = hash_entry (b_, struct file_elem, hash_elem);
  return a->fd < b->fd;
}

/* Returns the file pointer associated the given fd,
or a null pointer if no such file exists. */
struct file *
file_lookup (struct thread *t, const int fd)
{
  struct file_elem f;
  struct hash_elem *e;
  f.fd = fd;
  e = hash_find (t->files, &f.hash_elem);
  return e != NULL ? hash_entry (e, struct file_elem, hash_elem) : NULL;
}

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
