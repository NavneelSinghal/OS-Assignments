#ifndef INCLUDE_MYOWNTHREAD
#define INCLUDE_MYOWNTHREAD

#include "queue.h"
#include <setjmp.h>

typedef struct tcb_t {
    
    /* thread identity */
    int tid;
    
    /* stack size */
    int stack_size;
   
    /* stack pointer end */
    void* stack_start;

    /* function call stuff */
    void *(*start_routine)(void*);
    void *arg;

    /* for catching return values from the thread that just joined */
    void *retval;

    /* environment */
    jmp_buf env;
    
    /* which thread is waiting for it to join */
    struct tcb_t *join_caller_id;

    /* which thread is it currently waiting for to join */
    struct tcb_t *joiner;

    /* is this the main thread? */
    int is_main;

    /* is this thread running? */
    int is_finished;

    /* add more stuff as and when needed */
} tcb_t;

typedef struct tcb_t *myThread_t;
typedef int myThread_attr_t;

// void myThread_init_main(myThread_t* thread, const myThread_attr_t *attr, void *(*start_routine) (void*), void *arg);

int myThread_create(myThread_t* thread, const myThread_attr_t *attr, void* (*start_routine) (void*), void *arg);

void myThread_exit(void *retval);

int myThread_cancel(myThread_t thread);

int myThread_attr_init(myThread_attr_t *attr);

int myThread_attr_destroy(myThread_attr_t *attr);

myThread_t myThread_self(void);

int myThread_yield(void);

void myThread_join(myThread_t thread, void** retval);

#endif
