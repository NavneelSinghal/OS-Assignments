#ifndef INCLUDE_MYOWNTHREAD
#define INCLUDE_MYOWNTHREAD

#include "myOwnthreadstruct.h"

int myThread_create(myThread_t* thread, const myThread_attr_t *attr, void *(*start_routine) (void*), void *arg);

void myThread_exit(void *retval);

int myThread_cancel(myThread_t thread);

int myThread_attr_init(myThread_attr_t *attr);

int myThread_attr_destroy(myThread_attr_t *attr);

myThread_t myThread_self(void);

int myThread_yield(void);

void myThread_join(myThread_t thread, void** retval);

#endif
