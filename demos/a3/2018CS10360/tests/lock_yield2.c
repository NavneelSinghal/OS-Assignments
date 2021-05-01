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
// #define WAIT_ITER 20000000
#define WAIT_ITER 0

mutex_t mutex;

int common = 0;

void* a(void* arg) {
    myThread_mutex_lock(&mutex);
    printf("thread id is %d\n", myThread_self()->tid);
    for (int j = 0; j < ITER; ++j) {
        common += j;
    }
    printf("common value becomes %d\n", common);
    myThread_yield();
    for (int i = 0; i < WAIT_ITER; ++i)
        ;
    myThread_mutex_unlock(&mutex);
    myThread_exit(NULL);
}

int main() {
    int n = 100;

    int* arg = (int*)malloc(sizeof(int) * N);
    for (int i = 0; i < N; ++i) {
        *(arg + i) = n;
    }

    myThread_mutex_init(&mutex, NULL);

    dprint("MAIN: initializing attributes");

    myThread_t thread[N];

    for (int i = 0; i < N; ++i) {
        myThread_attr_t attr;
        myThread_attr_init(&attr);
        dprint("MAIN: creating thread");
        myThread_create(&thread[i], &attr, &a, arg);
        dprint("MAIN: thread created");
    }

    // for (int i = 0; i < 20; ++i) {
    //     printf("thread id is %d\n", myThread_self()->tid);
    //     for (int i = 0; i < WAIT_ITER; ++i);
    //     myThread_yield();
    // }

    for (int i = 0; i < N; ++i) {
        dprint("MAIN: joining thread number %d", thread[i]->tid);
        myThread_join(thread[i], NULL);
        dprint("MAIN: thread joined");
    }

    free(arg);
    myThread_mutex_destroy(&mutex);

    dprint("MAIN: thread joined, exiting...");

    return 0;
}
