#ifndef USERPROG_SYSCALL_H
#define USERPROG_SYSCALL_H
#include "filesys/file.h"
#include "filesys/filesys.h"
#include <hash.h>
#include <inttypes.h>
#include <stdbool.h>
#include "threads/synch.h"
#include "threads/thread.h"
#include "userprog/pagedir.h"
#include "threads/vaddr.h"

#define PAGE_SIZE 0x1000 /* 4KB */

#define FD_ERROR -1                         /* Error value for file descriptors. */
#define INVALID_ARG_ERROR -1                  /* Error value for null pointers in file handlers */
#define MAX_STDOUT_BUFF_SIZE 128            /* Maximum buffer size for stdout writes. */

/* Stores the next argument on the stack into the provided variable */
#define get_argument(var_name, arg_ptr, type) \
({ \
    validate_pointer (arg_ptr);               \
    var_name = *(type *) (arg_ptr++);         \
})

struct exec_waiter {
    struct semaphore sema;
    bool success;
};

struct file_elem {
    int fd;
    struct file *faddr;
    struct hash_elem hash_elem;
};

unsigned file_elem_hash (const struct hash_elem *, void *aux);
bool file_elem_less (const struct hash_elem *, const struct hash_elem *, void *aux);
struct file *file_lookup (const int);
void syscall_init (void);
void exit_thread (int);

typedef void (*handler) (int32_t *, uint32_t *);

/* Checks if a pointer can be accessed legally by the user process */
inline static void
validate_pointer (void *ptr)
{
  if (ptr == NULL || !is_user_vaddr(ptr) || pagedir_get_page(thread_current()->pagedir, ptr) == NULL) {
    exit_thread(INVALID_ARG_ERROR);
  }
}

#endif /* userprog/syscall.h */
