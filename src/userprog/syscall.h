#ifndef USERPROG_SYSCALL_H
#define USERPROG_SYSCALL_H
#include "lib/user/syscall.h"

void syscall_init (void);

void halt(void);
void exit (int status);
void close (int fd);
void seek (int fd, unsigned position);
pid_t exec (const char *cmd_line);
int wait (pid_t pid);
unsigned tell (int fd);
int open (const char *file);
int filesize (int fd);
int read (int fd, void *buffer, unsigned size);
int write (int fd, const void *buffer, unsigned size);
bool create (const char *file, unsigned initial_size);
bool remove (const char *file) ;

#endif /* userprog/syscall.h */
