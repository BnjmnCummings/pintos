#ifndef USERPROG_PROCESS_H
#define USERPROG_PROCESS_H

#include "threads/thread.h"

#define SPACE_DELIM " "

/* round stack pointer down to a multiple of 4 for word-aligned access*/
#define TRUNCATE_SP(sp) ((sp) - (sp % 4))

tid_t process_execute (const char *file_name);
int process_wait (tid_t);
void process_exit (void);
void process_activate (void);

#endif /* userprog/process.h */
