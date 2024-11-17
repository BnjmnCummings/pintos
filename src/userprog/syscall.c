#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include <hash.h>
#include <inttypes.h>
#include "threads/malloc.h"
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/vaddr.h"

/* Lock used by allocate_fd(). */
static struct lock fd_lock;

/* Lock used by system calls using the file system. */
static struct lock filesys_lock;

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
      printf ("System Call Number: %d\n", sys_call_number);

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

/* Returns a hash for file p via its fd. */
unsigned
file_elem_hash (const struct hash_elem *f_, void *aux UNUSED)
{
  const struct file_elem *f = hash_entry (f_, struct file_elem, hash_elem);
  return hash_int(f->fd);
}

/* Returns true if the fd of file a precedes file b. */
bool
file_elem_less (const struct hash_elem *a_, const struct hash_elem *b_,
void *aux UNUSED)
{
  const struct file_elem *a = hash_entry (a_, struct file_elem, hash_elem);
  const struct file_elem *b = hash_entry (b_, struct file_elem, hash_elem);
  return a->fd < b->fd;
}

/* Returns the file pointer associated the given fd,
or a null pointer if no such file exists. */
struct file *
file_lookup (const int fd)
{
  struct thread *t = thread_current();
  struct file_elem temp;
  struct hash_elem *e;
  temp.fd = fd;
  e = hash_find (&t->files, &temp.hash_elem);
  return e != NULL ? hash_entry (e, struct file_elem, hash_elem)->faddr : NULL;
}

/* Creates a new file in the filesystem */
static void 
create (int32_t *args UNUSED, uint32_t *returnValue UNUSED)
{
  const char *name = *(const char *) args[0];
  unsigned initial_size = *(unsigned *) args[1];

  lock_acquire(&filesys_lock);
  bool returnStatus = filesys_create(name, initial_size);
  lock_release(&filesys_lock);

  *returnValue = returnStats;
}

/* Closes a file by removing its element from the hash table and freeing it. */
static void
close (int32_t *args, uint32_t *returnValue UNUSED)
{
  int fd = *(int *) args;

  struct thread *t = thread_current();

  struct file_elem temp;
  struct hash_elem *e;
  temp.fd = fd;
  e = hash_find (&t->files, &temp.hash_elem);

  if (e != NULL) {
    hash_delete(&t->files, e);

    /* Free dynamically allocated file element we've just removed */
    struct file_elem *fe = hash_entry(e, struct file_elem, hash_elem);
    free(fe);
  }
}

/* Opens a file for a process by adding it to its access hash table. */
static void
open (int32_t *args, uint32_t *returnValue) 
{
  const char *file = (const char *) args;
  struct thread *t = thread_current();

  // TODO: DENY ACCESS WITH FD_ERROR

  lock_acquire(&filesys_lock);
  struct file *faddr = filesys_open(file);
  lock_release(&filesys_lock);
  
  struct file_elem *f = malloc(sizeof(struct file_elem));
  f->faddr = faddr;
  f->fd = allocate_fd();
  
  struct hash_elem *res = hash_insert(&t->files, &f->hash_elem);
  if (res != NULL) {
      free(&f);
  }

  *returnValue = f->fd;
}

/* Changes a file's read-write position based on its fd. */
static void
seek (int32_t *args, uint32_t *returnValue UNUSED)
{
  int fd = *(int *) args[0];
  unsigned position = *(unsigned *) args[1];

  lock_acquire(&filesys_lock);
  struct file *f = file_lookup(fd);

  if (f == NULL) {
    lock_release(&filesys_lock);
    return;
  }

  file_seek(f, position);
  lock_release(&filesys_lock);
}

/* Returns the next read-write position of a file. */
static void
tell (int32_t *args, uint32_t *returnValue)
{
  int fd = *(int *) args;
  
  lock_acquire(&filesys_lock);
  struct file *f = file_lookup(fd);

  if (f == NULL) {
    lock_release(&filesys_lock);
    return;
  }

  *returnValue = file_tell(f);
  lock_release(&filesys_lock);
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
  unsigned size = args[2];

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
static void remove (int32_t *args UNUSED, uint32_t *returnValue UNUSED)
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
