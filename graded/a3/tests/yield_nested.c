#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "../include/myOwnthread.h"

#define N 5
#define ITER 5000

void* b(void* arg) {
    printf("calling thread id is %d\n", *((int*)arg));
    printf("called thread id is %d\n", myThread_self()->tid);
    for (int i = 0; i < ITER; ++i);
    myThread_exit(NULL);
}

void* a(void* arg) {
    
    for (int i = 0; i < ITER; ++i);
    
    int n = 99;
    int *nested_arg = (int*) malloc(sizeof(int) * N);
    for (int i = 0; i < N; ++i) {
        *(nested_arg + i) = myThread_self()->tid;
    }

    printf("A: initializing attributes\n");

    myThread_t thread[N];

    for (int i = 0; i < N; ++i) {
        myThread_attr_t attr;
        myThread_attr_init(&attr);
        printf("A: creating thread\n");
        myThread_create(&thread[i], &attr, &b, nested_arg);
        printf("A: thread created\n");
    }

    for (int i = 0; i < ITER; ++i);

    for (int i = 0; i < ITER; ++i) {
        printf("thread id is %d\n", myThread_self()->tid);
        myThread_yield();
    }

    for (int i = 0; i < N; ++i) {
        myThread_join(thread[i], NULL);
        printf("A: joined thread\n");
    }

    free(nested_arg);

    printf("A: thread joined, exiting...\n");

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
    
    for (int i = 0; i < ITER; ++i);

    for (int i = 0; i < ITER; ++i) {
        printf("thread id is %d\n", myThread_self()->tid);
        myThread_yield();
    }

    for (int i = 0; i < N; ++i) {
        myThread_join(thread[i], NULL);
        printf("MAIN: joined thread\n");
    }

    free(arg);

    printf("MAIN: thread joined, exiting...\n");

    return 0;
}
