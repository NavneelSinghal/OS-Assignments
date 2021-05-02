#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sched.h>

#include "../include/myOwnthread.h"

#define N 100

#define MAX_THREADS 40

#define STEP 1

#define MAX_ITER 1000000

int num_threads;

pthread_t* threads;
myThread_t* mythreads;

struct timespec start_time_pthreads, end_time_pthreads;
struct timespec start_time_mythreads, end_time_mythreads;
struct timespec start_time_normal, end_time_normal;
// double pthread_time[MAX_THREADS];
// double mythread_time[MAX_THREADS];
// double normal_time[MAX_THREADS];

/* find difference in time between two times in microseconds */
double time_duration(struct timespec* a, struct timespec* b) {
    return (b->tv_sec - a->tv_sec) * 1000000.0L +
           (b->tv_nsec - a->tv_nsec) / 1000.0L;
}

pthread_mutex_t pmutex = PTHREAD_MUTEX_INITIALIZER;
mutex_t mmutex;

int pthread_count = 0;
int mythread_count = 0;

void* keep_yielding_pthread(void* arg) {
    int x = 0;
    pthread_mutex_lock(&pmutex);
    pthread_count++;
    x = (pthread_count < MAX_ITER);
    pthread_mutex_unlock(&pmutex);
    if (x) sched_yield();
    else pthread_exit(NULL);
}

void* keep_yielding_mythread(void* arg) {
    int x = 0;
    myThread_mutex_lock(&mmutex);
    mythread_count++;
    x = (mythread_count < MAX_ITER);
    myThread_mutex_unlock(&mmutex);
    if (x) myThread_yield();
    else myThread_exit(NULL);
}

int main(int argc, char* argv[]) {
    num_threads = atoi(argv[1]);
    // for (num_threads = 1; num_threads <= MAX_THREADS; num_threads += STEP) {
    
    /* using pthreads */

    printf("%d ", num_threads);

    clock_gettime(CLOCK_MONOTONIC, &start_time_pthreads);

    // allocate
    threads = (pthread_t*)malloc(num_threads * sizeof(pthread_t));

    int* args = (int*)malloc(num_threads * sizeof(int));
    for (int i = 0; i < num_threads; ++i) {
        args[i] = i;
    }

    for (int i = 0; i < num_threads; ++i) {
        pthread_attr_t attr;
        pthread_attr_init(&attr);
        pthread_create(&threads[i], &attr, keep_yielding_pthread, &args[i]);
    }

    for (int i = 0; i < num_threads; ++i) {
        pthread_join(threads[i], NULL);
    }

    // cleanup
    free(threads);
    free(args);

    clock_gettime(CLOCK_MONOTONIC, &end_time_pthreads);

    printf("%lf ", time_duration(&start_time_pthreads, &end_time_pthreads));

    /* using myThreads */

    // allocate
    myThread_mutex_init(&mmutex, NULL);
    clock_gettime(CLOCK_MONOTONIC, &start_time_mythreads);

    mythreads = (myThread_t*)malloc(num_threads * sizeof(myThread_t));

    args = (int*)malloc(num_threads * sizeof(int));
    for (int i = 0; i < num_threads; ++i) {
        args[i] = i;
    }

    for (int i = 0; i < num_threads; ++i) {
        myThread_attr_t attr;
        myThread_attr_init(&attr);
        myThread_create(&mythreads[i], &attr, keep_yielding_mythread, &args[i]);
    }

    for (int i = 0; i < num_threads; ++i) {
        myThread_join(mythreads[i], NULL);
    }
    myThread_mutex_destroy(&mmutex);

    // cleanup
    free(mythreads);
    free(args);

    clock_gettime(CLOCK_MONOTONIC, &end_time_mythreads);

    printf("%lf\n", time_duration(&start_time_mythreads, &end_time_mythreads));

}
