#include <hash.h>
#include <inttypes.h>
#include <stdio.h>
#include <syscall-nr.h>

#include "devices/input.h"
#include "devices/shutdown.h"

#include "threads/interrupt.h"
#include "threads/malloc.h"
#include "threads/thread.h"
#include "threads/vaddr.h"

#include "userprog/process.h"
#include "userprog/syscall.h"

/* Lock used by allocate_fd(). */
static struct lock fd_lock;

/* Lock used by system calls using the file system. */
static struct lock filesys_lock;

static void syscall_handler (struct intr_frame *);
inline static void validate_pointer (void *ptr);
static void validate_buffer (void* buffer, unsigned size);

static void write (int32_t *args, uint32_t *return_value);
static void exit (int32_t *args, uint32_t *return_value UNUSED);
static void halt (int32_t *args UNUSED, uint32_t *return_value UNUSED);
static void exec (int32_t *args, uint32_t *return_value);
static void wait (int32_t *args, uint32_t *return_value);
static void create (int32_t *args, uint32_t *return_value);
static void remove (int32_t *args, uint32_t *return_value);
static void open (int32_t *args, uint32_t *return_value);
static void filesize (int32_t *args, uint32_t *return_value);
static void read (int32_t *args, uint32_t *return_value);
static void seek (int32_t *args, uint32_t *return_value UNUSED);
static void tell (int32_t *args, uint32_t *return_value);
static void close (int32_t *args, uint32_t *return_value UNUSED);

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
  lock_init(&fd_lock);
  lock_init(&filesys_lock);
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

static void
syscall_handler (struct intr_frame *f)
{
  if (is_user_vaddr(f->esp)) {
    int32_t *stack_pointer = (int32_t *) f->esp;
    int32_t sys_call_number;
    get_argument(sys_call_number, stack_pointer, int32_t);

    ASSERT (sys_call_number >= 0);

    if (sys_call_number <= SYS_CLOSE) {
      // printf ("System Call Number: %d\n", sys_call_number);

      /* invoked the handler corresponding to the system call number */
      sys_call_handlers[sys_call_number](stack_pointer, &f->eax);
    } else {
      thread_current()->exit_status = -1;
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

/* Removes a file from the file system given its name. */
/* SIGNATURE: bool remove (const char *file) */
static void
remove (int32_t *args, uint32_t *return_value)
{
  char *name;
  get_argument(name, args, char *);
  validate_pointer(name);

  lock_acquire(&filesys_lock);
  bool returnStatus = filesys_remove((const char *) name);
  lock_release(&filesys_lock);

  *return_value = returnStatus;
}

/* Creates a new file in the filesystem. */
/* SIGNATURE: bool create (const char *file, unsigned initial_size) */
static void
create (int32_t *args, uint32_t *return_value)
{
  char *name;
  unsigned initial_size;

  get_argument(name, args, char *);
  get_argument(initial_size, args, unsigned);

  validate_pointer(name);

  lock_acquire(&filesys_lock);
  bool returnStatus = filesys_create((const char *) name, initial_size);
  lock_release(&filesys_lock);

  *return_value = returnStatus;
}

/* Closes a file by removing its element from the hash table and freeing it. */
/* SIGNATURE: void close (int fd) */
static void
close (int32_t *args, uint32_t *return_value UNUSED)
{
  int fd;
  get_argument(fd, args, int);

  struct thread *t = thread_current();

  struct file_elem temp;
  struct hash_elem *e;
  temp.fd = fd;
  e = hash_find (&t->files, &temp.hash_elem);

  if (e != NULL) {
    hash_delete(&t->files, e);

    /* Free dynamically allocated file element we've just removed */
    struct file_elem *fe = hash_entry(e, struct file_elem, hash_elem);
    file_close(fe->faddr);
    free(fe);
  }
}

/* Opens a file for a process by adding it to its access hash table. */
/* SIGNATURE: int open (const char *file) */
static void
open (int32_t *args, uint32_t *return_value)
{
  char *file;
  get_argument(file, args, char *);
  validate_pointer(file);

  struct thread *t = thread_current();

  lock_acquire(&filesys_lock);
  struct file *faddr = filesys_open((const char *) file);
  lock_release(&filesys_lock);

  /* The failure return value for filesys_open is NULL. */
  if (faddr == NULL) {
    *return_value = FD_ERROR;
    return;
  }

  struct file_elem *f = malloc(sizeof(struct file_elem));
  f->faddr = faddr;
  f->fd = allocate_fd();

  struct hash_elem *res = hash_insert(&t->files, &f->hash_elem);
  if (res != NULL) {
    free(f);
  }

  *return_value = f->fd;
}

inline static void
validate_pointer (void *ptr)
{
    if (ptr == NULL || !is_user_vaddr(ptr) || pagedir_get_page(thread_current()->pagedir, ptr) == NULL) {
        exit_thread(INVALID_ARG_ERROR);
    }
}

/* Returns the size of the file associated with a given fd. */
/* SIGNATURE: int filesize (int fd)*/
static void
filesize (int32_t *args, uint32_t *return_value)
{
  int fd;
  get_argument(fd, args, int);

  lock_acquire(&filesys_lock);
  struct file *f = file_lookup(fd);

  if (f == NULL) {
    lock_release(&filesys_lock);
    return;
  }

  *return_value = (unsigned) file_length(f);
  lock_release(&filesys_lock);
}

/* Changes a file's read-write position based on its fd. */
/* SIGNATURE: void seek (int fd, unsigned position) */
static void
seek (int32_t *args, uint32_t *return_value UNUSED)
{
  int fd;
  unsigned position;
  get_argument(fd, args, int);
  get_argument(position, args, unsigned);

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
/* SIGNATURE: unsigned tell (int fd) */
static void
tell (int32_t *args, uint32_t *return_value)
{
  int fd;
  get_argument(fd, args, int);

  lock_acquire(&filesys_lock);
  struct file *f = file_lookup(fd);

  if (f == NULL) {
    lock_release(&filesys_lock);
    return;
  }

  *return_value = (unsigned) file_tell(f);
  lock_release(&filesys_lock);
}

void exit_thread(int status) {
    struct thread *cur = thread_current();
    cur->exit_status = cur->wait->exit_status = status;
    if (lock_held_by_current_thread(&fd_lock))
      lock_release(&fd_lock);
    if (lock_held_by_current_thread(&filesys_lock))
      lock_release(&filesys_lock);
    thread_exit();
}

/* SIGNATURE: void exit (int status) */
static void
exit (int32_t *args, uint32_t *return_value UNUSED)
{
    int status;
    get_argument(status, args, int);
    /* status must be stored somewhere, maybe in thread struct*/
    exit_thread(status);
}

/* Checks if a multipage buffer can be safely accessed by the user,
   if not terminates the user process                             */
static void
validate_buffer (void* buffer, unsigned size)
{
  validate_pointer(buffer);

  bool valid = true;
  for (void* tmp = buffer; tmp <= buffer + size - PAGE_SIZE; tmp += PAGE_SIZE) {
    valid &= (is_user_vaddr(tmp) && (pagedir_get_page(thread_current()->pagedir, tmp) != NULL));
  }
  valid &= (is_user_vaddr(buffer+size) && (pagedir_get_page(thread_current()->pagedir, buffer+size) != NULL));

  if (!valid) {
    exit_thread(INVALID_ARG_ERROR);
  }
}

/* SIGNATURE: int write (int fd, const void *buffer, unsigned size) */
static void
read (int32_t *args, uint32_t *return_value)
{
  int fd;
  void *buffer;
  unsigned size;

  get_argument(fd, args, int);
  get_argument(buffer, args, void *);
  get_argument(size, args, unsigned );

  validate_buffer(buffer, size);

  if (fd == 0) {
    int inputs_read = 0;

    uint8_t *input_buffer = (uint8_t *) buffer;
    for (unsigned i = 0; i < size; i++) {
      input_buffer[i] = input_getc();
      inputs_read++;
    }

    *return_value = inputs_read;
    return;
  }

  lock_acquire(&filesys_lock);

  struct file *f = file_lookup(fd);

  if (f == NULL) {
    *return_value = 0;
    lock_release(&filesys_lock);
    return;
  }

  off_t amount_read = file_read(f, buffer, size);
  *return_value = (unsigned) amount_read;

  lock_release(&filesys_lock);
}

/* System write call from a buffer to a file associated with a given fd. */
/* SIGNTATURE: int write (int fd, const void *buffer, unsigned size) */
static void
write (int32_t *args, uint32_t *return_value)
{
  int fd;
  void *buffer;
  unsigned size;

  get_argument(fd, args, int);
  get_argument(buffer, args, void *);
  get_argument(size, args, unsigned);

  validate_buffer(buffer, size);

  if (fd == 1) {
    /* Only write to stdout by a constant amount of bytes per write */
    unsigned written = 0;
    while (written < size) {
      unsigned block_size = (size - written > MAX_STDOUT_BUFF_SIZE)
                            ? MAX_STDOUT_BUFF_SIZE
                            : size - written;
      putbuf((const char *)buffer + written, block_size);
      written += block_size;
    }

    *return_value = (int) size;
    return;
  }

  lock_acquire(&filesys_lock);

  struct file *f = file_lookup(fd);

  if (f == NULL) {
    *return_value = 0;
    lock_release(&filesys_lock);
    return;
  }

  off_t amount_written = file_write(f, buffer, size);
  *return_value = (int) amount_written;

  lock_release(&filesys_lock);
}

/* SIGNATURE: void halt (void) */
static void
halt (int32_t *args UNUSED, uint32_t *return_value UNUSED)
{
  shutdown_power_off ();
}

/* SIGNATURE: tid_t exec (const char *cmd_line) */
static void
exec (int32_t *args, uint32_t *return_value)
{
  char *cmd_line;
  get_argument(cmd_line, args, char *);
  validate_pointer(cmd_line);

  /* wait for the thread to be scheduled and  initialise the process */
  struct exec_waiter waiter;
  sema_init(&waiter.sema, 0);
  tid_t pid = process_execute(cmd_line, &waiter);
  if (pid == TID_ERROR) {
    *return_value = TID_ERROR;
    return;
  }
  sema_down(&waiter.sema);

  /* return the pid of the new process or -1 for failed initialisation */
  if (waiter.success) {
    *return_value = pid;
    return;
  }
  *return_value = TID_ERROR;
}

/* SIGNATURE: int wait (tid_t pid) */
static void
wait (int32_t *args, uint32_t *return_value)
{
  tid_t pid;
  get_argument(pid, args, tid_t);
  *return_value = process_wait(pid);
}
