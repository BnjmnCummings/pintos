#ifndef USERPROG_SYSCALL_H
#define USERPROG_SYSCALL_H

#include <inttypes.h>

void syscall_init (void);
typedef void (*handler) (int32_t *, uint32_t *);

#endif /* userprog/syscall.h */
