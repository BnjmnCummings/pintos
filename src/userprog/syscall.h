#ifndef USERPROG_SYSCALL_H
#define USERPROG_SYSCALL_H
#include "filesys/file.h"
#include "filesys/filesys.h"
#include <hash.h>
#include <inttypes.h>
#include <stdbool.h>
#include "threads/synch.h"

struct exec_waiter {
    struct semaphore sema;
    bool success;
};

#define FD_ERROR -1               /* Error value for file descriptors. */

struct file_elem {
    int fd;
    struct file *faddr;
    struct hash_elem hash_elem;
};

unsigned file_elem_hash (const struct hash_elem *, void *aux);
bool file_elem_less (const struct hash_elem *, const struct hash_elem *, void *aux);
struct file *file_lookup (const int);
void syscall_init (void);

typedef void (*handler) (int32_t *, uint32_t *);

#endif /* userprog/syscall.h */
