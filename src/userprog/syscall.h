#ifndef USERPROG_SYSCALL_H
#define USERPROG_SYSCALL_H
#include "lib/user/syscall.h"
#include "filesys/file.c"
#include <inttypes.h>

#define FD_ERROR -1               /* Error value for file descriptors. */

struct file_elem {
    int fd;
    struct file *faddr;
    struct hash_elem hash_elem;
}

void syscall_init (void);
typedef void (*handler) (int32_t *, uint32_t *);

#endif /* userprog/syscall.h */
