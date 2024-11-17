#ifndef USERPROG_SYSCALL_H
#define USERPROG_SYSCALL_H

#include <inttypes.h>
#include <stdbool.h>
#include "threads/synch.h"

struct exec_waiter {
    struct semaphore sema;
    bool success;
};

void syscall_init (void);
typedef void (*handler) (int32_t *, uint32_t *);

#endif /* userprog/syscall.h */
