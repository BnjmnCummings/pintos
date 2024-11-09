#ifndef USERPROG_PROCESS_H
#define USERPROG_PROCESS_H

#include "threads/thread.h"

#define SPACE_DELIM " "

/* Uses type conversion to decrement provided void* pointer by a given number of bytes */
#define DEC_ESP_BY_BYTES(esp, num) ((esp) = (void *) ((uint8_t *) (esp) - (num)))

#define stack_push_element(esp, elem, type) ({ \
    DEC_ESP_BY_BYTES((esp), sizeof(type)); \
    *(esp) = (void*) (elem); \
    })

#define stack_push_string(esp, elem) ({ \
    DEC_ESP_BY_BYTES((esp), strlen((elem))+1); \
    strlcpy((char*) (esp), (elem), strlen((elem))+1); \
    })

/* Stores the arguments needed to initalise a user process stack */
struct stack_entries 
{
    char* argv[10]; /* Size of array is the maximum number of arguments a program is expected to get */
    int    argc;     /* Actual number of arguments */
    char*  fn_copy;  /* Used in given implementation */
};

tid_t process_execute (const char *file_name);
int process_wait (tid_t);
void process_exit (void);
void process_activate (void);

#endif /* userprog/process.h */
