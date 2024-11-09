#ifndef USERPROG_PROCESS_H
#define USERPROG_PROCESS_H

#include "threads/thread.h"

#define SPACE_DELIM " "

#define PAGE_LOWEST_ADRESS 0xBFFFF000 /* PHYS_BASE - 4KB */

/* Uses type conversion to decrement provided void* pointer by a given number of bytes */
#define DEC_ESP_BY_BYTES(esp, num) ((void *) ((uint8_t *) (esp) - (num)))

#define stack_push_element(esp, elem, type) ({ \
    if ((unsigned) DEC_ESP_BY_BYTES((esp), sizeof(type)) > PAGE_LOWEST_ADRESS) { \
        esp = DEC_ESP_BY_BYTES((esp), sizeof(type)); \
        *(esp) = (void*) (elem); \
    } else { \
        /* Come up with a way to deal with this */ \
    } })

#define stack_push_string(esp, elem) ({ \
    if ((unsigned) DEC_ESP_BY_BYTES((esp), strlen((elem))+1) > PAGE_LOWEST_ADRESS) { \
        esp = DEC_ESP_BY_BYTES((esp), strlen((elem))+1); \
        strlcpy((char*) (esp), (elem), strlen((elem))+1); \
    } else { \
        /* Come up with a way to deal with this */ \
    } })

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
