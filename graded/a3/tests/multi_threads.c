#include <stdio.h>
#include <stdlib.h>

#include "../include/myOwnthread.h"

#include <stdarg.h>
#include <stdio.h>

void dprint(const char *format, ...) {
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    fprintf(stderr, "\n");
    fflush(stderr);
}

void* a(void* arg) {
    dprint("hello, %d has been passed to a()", *((int*)arg));
    myThread_exit(NULL);
}

int main() {

    int n = 100;

    int* arg = (int*)malloc(sizeof(int));
    *arg = n;

    dprint("MAIN: initializing attributes");

    myThread_attr_t attr;
    myThread_attr_init(&attr);
    
    myThread_t thread;

    dprint("MAIN: creating thread");

    myThread_create(&thread, &attr, &a, arg);
    
    dprint("MAIN: thread created");

    dprint("MAIN: joining thread");

    myThread_join(thread, NULL);
   
    dprint("MAIN: thread joined, exiting...");

    return 0;
}
