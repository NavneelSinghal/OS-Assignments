#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "../include/myOwnthread.h"

void dprint(const char* format, ...) {
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    fprintf(stderr, "\n");
    fflush(stderr);
}

#define N 10
#define ITER 5
#define WAIT_ITER 2000000

volatile int dummy = 0;

void* a(void* arg) {
    for (int i = 0; i < ITER; ++i) {
        // might get buffered
        printf("thread id is %d\n", myThread_self()->tid);
        for (int i = 0; i < WAIT_ITER; ++i) {
            dummy += rand();
        }
    }
    myThread_exit(NULL);
}

int main() {
    int n = 100;

    int* arg = (int*)malloc(sizeof(int) * N);
    for (int i = 0; i < N; ++i) {
        *(arg + i) = n;
    }

    dprint("MAIN: initializing attributes");

    myThread_t thread[N];

    for (int i = 0; i < N; ++i) {
        myThread_attr_t attr;
        myThread_attr_init(&attr);
        dprint("MAIN: creating thread");
        myThread_create(&thread[i], &attr, &a, arg);
        dprint("MAIN: thread created");
    }

    for (int i = 0; i < ITER; ++i) {
        printf("thread id is %d\n", myThread_self()->tid);
        for (int i = 0; i < WAIT_ITER; ++i) {
            dummy += rand();
        }
    }

    for (int i = 0; i < N; ++i) {
        dprint("MAIN: joining thread number %d", thread[i]->tid);
        dprint("MAIN: joining thread");
        myThread_join(thread[i], NULL);
    }

    free(arg);

    dprint("MAIN: thread joined, exiting...");

    return 0;
}
