#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "../include/myOwnthread.h"

#define N 100

#define MAX_THREADS 40

#define STEP 1

int num_threads;

int **a, **b, **c;

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

void* compute_chunk_pthread(void* arg) {
    int thread_id = *((int*)arg);
    int left = thread_id * N / num_threads;
    int right = (thread_id + 1) * N / num_threads;
    if (thread_id == num_threads - 1) right = N;
    for (int i = left; i < right; ++i) {
        for (int j = 0; j < N; ++j) {
            for (int k = 0; k < N; ++k) {
                c[i][j] += a[i][k] * b[k][j];
            }
        }
    }
    pthread_exit(NULL);
}

void* compute_chunk_mythread(void* arg) {
    int thread_id = *((int*)arg);
    int left = thread_id * N / num_threads;
    int right = (thread_id + 1) * N / num_threads;
    if (thread_id == num_threads - 1) right = N;
    for (int i = left; i < right; ++i) {
        for (int j = 0; j < N; ++j) {
            for (int k = 0; k < N; ++k) {
                c[i][j] += a[i][k] * b[k][j];
            }
        }
    }
    myThread_exit(NULL);
}

int main(int argc, char* argv[]) {
    num_threads = atoi(argv[1]);
    // for (num_threads = 1; num_threads <= MAX_THREADS; num_threads += STEP) {
    
    /* using pthreads */

    printf("%d ", num_threads);

    clock_gettime(CLOCK_MONOTONIC, &start_time_pthreads);

    // allocate
    a = (int**)malloc(N * sizeof(int*));
    for (int i = 0; i < N; ++i) {
        a[i] = (int*)malloc(N * sizeof(int));
    }
    b = (int**)malloc(N * sizeof(int*));
    for (int i = 0; i < N; ++i) {
        b[i] = (int*)malloc(N * sizeof(int));
    }
    c = (int**)malloc(N * sizeof(int*));
    for (int i = 0; i < N; ++i) {
        c[i] = (int*)malloc(N * sizeof(int));
    }
    threads = (pthread_t*)malloc(num_threads * sizeof(pthread_t));

    int* args = (int*)malloc(num_threads * sizeof(int));
    for (int i = 0; i < num_threads; ++i) {
        args[i] = i;
    }

    for (int i = 0; i < num_threads; ++i) {
        pthread_attr_t attr;
        pthread_attr_init(&attr);
        pthread_create(&threads[i], &attr, compute_chunk_pthread, &args[i]);
    }

    for (int i = 0; i < num_threads; ++i) {
        pthread_join(threads[i], NULL);
    }

    // for (int i = 0; i < N; ++i) {
    //     for (int j = 0; j < N; ++j) {
    //         printf("%d ", c[i][j]);
    //     }
    //     printf("\n");
    // }

    // cleanup
    free(threads);
    free(args);
    for (int i = 0; i < N; ++i) {
        free(a[i]);
        free(b[i]);
        free(c[i]);
    }
    free(a);
    free(b);
    free(c);
    a = NULL;
    b = NULL;

    clock_gettime(CLOCK_MONOTONIC, &end_time_pthreads);

    printf("%lf ", time_duration(&start_time_pthreads, &end_time_pthreads));

    /* using myThreads */

    // allocate

    clock_gettime(CLOCK_MONOTONIC, &start_time_mythreads);

    a = (int**)malloc(N * sizeof(int*));
    for (int i = 0; i < N; ++i) {
        a[i] = (int*)malloc(N * sizeof(int));
    }
    b = (int**)malloc(N * sizeof(int*));
    for (int i = 0; i < N; ++i) {
        b[i] = (int*)malloc(N * sizeof(int));
    }
    c = (int**)malloc(N * sizeof(int*));
    for (int i = 0; i < N; ++i) {
        c[i] = (int*)malloc(N * sizeof(int));
    }
    mythreads = (myThread_t*)malloc(num_threads * sizeof(myThread_t));

    args = (int*)malloc(num_threads * sizeof(int));
    for (int i = 0; i < num_threads; ++i) {
        args[i] = i;
    }

    for (int i = 0; i < num_threads; ++i) {
        myThread_attr_t attr;
        myThread_attr_init(&attr);
        // attr = 30000000;
        myThread_create(&mythreads[i], &attr, compute_chunk_mythread, &args[i]);
    }

    for (int i = 0; i < num_threads; ++i) {
        myThread_join(mythreads[i], NULL);
    }

    // for (int i = 0; i < N; ++i) {
    //     for (int j = 0; j < N; ++j) {
    //         printf("%d ", c[i][j]);
    //     }
    //     printf("\n");
    // }

    // cleanup
    free(mythreads);
    free(args);
    for (int i = 0; i < N; ++i) {
        free(a[i]);
        free(b[i]);
        free(c[i]);
    }
    free(a);
    free(b);
    free(c);
    a = NULL;
    b = NULL;

    clock_gettime(CLOCK_MONOTONIC, &end_time_mythreads);

    printf("%lf ", time_duration(&start_time_mythreads, &end_time_mythreads));

    /* using normal stuff */

    clock_gettime(CLOCK_MONOTONIC, &start_time_normal);

    a = (int**)malloc(N * sizeof(int*));
    for (int i = 0; i < N; ++i) {
        a[i] = (int*)malloc(N * sizeof(int));
    }
    b = (int**)malloc(N * sizeof(int*));
    for (int i = 0; i < N; ++i) {
        b[i] = (int*)malloc(N * sizeof(int));
    }
    c = (int**)malloc(N * sizeof(int*));
    for (int i = 0; i < N; ++i) {
        c[i] = (int*)malloc(N * sizeof(int));
    }

    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N; ++j) {
            for (int k = 0; k < N; ++k) {
                c[i][j] += a[i][k] * b[k][j];
            }
        }
    }

    for (int i = 0; i < N; ++i) {
        free(a[i]);
        free(b[i]);
        free(c[i]);
    }
    free(a);
    free(b);
    free(c);
    a = NULL;
    b = NULL;

    clock_gettime(CLOCK_MONOTONIC, &end_time_normal);

    printf("%lf\n", time_duration(&start_time_normal, &end_time_normal));

    // }

    // for (int i = 0; i < MAX_THREADS; i += STEP) {
    //     printf("%d: normal: %lf, mythread: %lf, pthread: %lf\n", i + 1,
    //            normal_time[i], mythread_time[i], pthread_time[i]);
    // }
}
