#ifndef USERPROG_SYSCALL_H
#define USERPROG_SYSCALL_H
#include "filesys/file.h"
#include "filesys/filesys.h"
#include <hash.h>
#include <inttypes.h>
#include <stdbool.h>
#include "threads/synch.h"
#include "threads/thread.h"
#include "threads/vaddr.h"
#include "userprog/pagedir.h"
#include "userprog/process.h"

#define PAGE_SIZE 0x1000 /* 4KB */

#define FD_ERROR -1                         /* Error value for file descriptors. */
#define FD_START 2                          /* Starting file descriptor to be allocated. */
#define INVALID_ARG_ERROR -1                /* Error value for null pointers in file handlers */
#define MAX_STDOUT_BUFF_SIZE 128            /* Maximum buffer size for stdout writes. */

/* Stores the next argument on the stack into the provided variable */
#define get_argument(var_name, arg_ptr, type) \
({ \
    validate_pointer (arg_ptr);               \
    var_name = *(type *) (arg_ptr++);         \
})

typedef uint32_t stack_arg;

typedef void (*handler) (stack_arg *, stack_arg *);

struct exec_waiter
{
    struct semaphore sema;         /*  */
    bool success;                  /* Return status for executable. */
};

struct file_elem
{
    int fd;                        /* Unique file descriptor. */
    struct file *faddr;            /* File pointer. */
    struct hash_elem hash_elem;    /* Hash table element. */
};

unsigned file_elem_hash (const struct hash_elem *, void *aux);
bool file_elem_less (const struct hash_elem *, const struct hash_elem *, void *aux);
struct file *file_lookup (const int);
void syscall_init (void);
void thread_exit_safe (int);

/* Checks if a pointer can be accessed legally by the user process */
inline static void
validate_pointer (void *ptr)
{
  if (ptr == NULL || !is_user_vaddr(ptr) || pagedir_get_page(thread_current()->pagedir, ptr) == NULL) {
    thread_exit_safe(INVALID_ARG_ERROR);
  }
}

#endif /* userprog/syscall.h */
