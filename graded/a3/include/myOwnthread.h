#ifndef INCLUDE_MYOWNTHREAD
#define INCLUDE_MYOWNTHREAD

#include <setjmp.h>

#include "queue.h"

/* thread definition */
typedef struct tcb_t {
    /* thread identity */
    int tid;
    /* stack size */
    int stack_size;
    /* stack pointer to the first position in the stack */
    void *stack_start;
    /* stack pointer to the last position in the stack */
    void *stack_end;
    /* function call stuff */
    void *(*start_routine)(void *);
    void *arg;
    /* for catching return values from the thread that just joined */
    void *retval;
    /* environment */
    jmp_buf env;
    /* which thread is waiting for it to join */
    struct tcb_t *join_caller_id;
    /* which thread is it currently waiting for to join */
    struct tcb_t *joiner;
    /* is this the main thread? - maybe useful for debugging */
    int is_main;
    /* is this thread running? */
    int is_finished;
} tcb_t;
typedef struct tcb_t *myThread_t;
typedef int myThread_attr_t;

/* thread functions */
int myThread_create(myThread_t *thread, const myThread_attr_t *attr,
                    void *(*start_routine)(void *), void *arg);
void myThread_exit(void *retval);
int myThread_cancel(myThread_t thread);
int myThread_attr_init(myThread_attr_t *attr);
int myThread_attr_destroy(myThread_attr_t *attr);
myThread_t myThread_self(void);
int myThread_yield(void);
void myThread_join(myThread_t thread, void **retval);

/* lock definition */
typedef struct mutex_t {
    int is_free;
    queue *waiting;
} mutex_t;

/* lock functions */
void myThread_mutex_init(mutex_t *mutex, void *attr);
void myThread_mutex_destroy(mutex_t *mutex);
void myThread_mutex_lock(mutex_t *mutex);
void myThread_mutex_unlock(mutex_t *mutex);

/* condition variable definition */
typedef struct cv_t {
    queue *waiting;
} cv_t;

/* condition variable functions */
void myThread_cond_init(cv_t *cond, void *attr);
void myThread_cond_destroy(cv_t *cond);
void myThread_cond_wait(cv_t *cond, mutex_t *mutex);
void myThread_cond_signal(cv_t *cond);
void myThread_cond_broadcast(cv_t *cond);

#endif
