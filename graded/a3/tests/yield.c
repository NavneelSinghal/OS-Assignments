#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "../include/myOwnthread.h"

#define N 20
#define ITER 5

#define LARGEN 10

void* a(void* arg) {
    for (int i = 0; i < ITER; ++i) {
        printf("thread id is %d\n", myThread_self()->tid);
        myThread_yield();
    }
    myThread_exit(NULL);
}

int main() {
    int n = 100;

    int* arg = (int*)malloc(sizeof(int) * N);
    for (int i = 0; i < N; ++i) {
        *(arg + i) = n;
    }

    printf("MAIN: initializing attributes\n");

    myThread_t thread[N];

    for (int i = 0; i < N; ++i) {
        myThread_attr_t attr;
        myThread_attr_init(&attr);
        printf("MAIN: creating thread\n");
        myThread_create(&thread[i], &attr, &a, arg);
        printf("MAIN: thread created\n");
    }

    for (int i = 0; i < 10000000; ++i);

    int *arg_large = (int*) malloc(sizeof(int) * LARGEN);
    for (int i = 0; i < N; ++i) {
        *(arg + i) = n;
    }

    myThread_t large[LARGEN];

    for (int i = 0; i < LARGEN; ++i) {
        myThread_attr_t attr;
        myThread_attr_init(&attr);
        printf("MAIN: creating thread\n");
        myThread_create(&large[i], &attr, &a, arg_large);
        printf("MAIN: thread created\n");
    }

    for (int i = 0; i < ITER; ++i) {
        printf("thread id is %d\n", myThread_self()->tid);
        myThread_yield();
    }

    for (int i = 0; i < LARGEN; ++i) {
        myThread_join(large[i], NULL);
        printf("MAIN: joined thread\n");
    }

    for (int i = 0; i < N; ++i) {
        myThread_join(thread[i], NULL);
        printf("MAIN: joined thread\n");
    }

    free(arg);

    free(arg_large);

    printf("MAIN: thread joined, exiting...\n");

    return 0;
}
