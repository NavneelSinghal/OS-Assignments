#include <math.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "../include/vector.h"

// assume C99
#ifdef DEBUG
#define dprintf(fmt, ...) fprintf(stderr, fmt, __VA_ARGS__);
#else
#define dprintf(...)
#endif

/**
 * --------------------------- DATA ----------------------------
 */

int num_threads, chunk_size, total_chunks, max_small_prime;
long long N;
int* is_small_prime;
vector small_primes;

vector* primes;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

double* wallclock_time_static;
double* wallclock_time_dynamic;
double* cpuclock_time_static;
double* cpuclock_time_dynamic;

/**
 * ------------------------ FUNCTIONS --------------------------
 */

/* find difference in time between two times in microseconds */
long double time_duration(struct timespec* a, struct timespec* b) {
    return (b->tv_sec - a->tv_sec) * 1000000.0L +
           (b->tv_nsec - a->tv_nsec) / 1000.0L;
}

/* function to be passed to thread */
void* run_thread_static(void* arg) {
    /* allocate once per thread */
    int* is_prime = (int*)malloc(chunk_size * sizeof(int));

    int* thread_num_ptr = (int*)arg;
    int cur_thread = *thread_num_ptr;

    struct timespec start_time_cpu, start_time_real;
    clock_gettime(CLOCK_THREAD_CPUTIME_ID, &start_time_cpu);
    clock_gettime(CLOCK_MONOTONIC, &start_time_real);

    /* statically run over the chunks */
    for (int chunk_num = cur_thread; chunk_num < total_chunks;
         chunk_num += num_threads) {
        /* ascertain chunk boundaries and size */
        long long l = 1LL * chunk_num * chunk_size + 1;
        long long r = 1LL * (chunk_num + 1) * chunk_size;
        if (r > N) r = N;
        long long current_chunk_size = r - l + 1;

        /* boolean array of whether it is a prime or not */
        for (long long i = 0; i < current_chunk_size; ++i) is_prime[i] = 1;
        if (l == 1) is_prime[0] = 0;

        /* segmented sieve run */
        for (int i = 0; i < small_primes.size; ++i) {
            long long p = small_primes.a[i];
            long long p_square = p * p;
            if (p_square > r) break;
            long long chunk_start = p * ((l + p - 1) / p);
            if (chunk_start < p_square) chunk_start = p_square;
            for (long long j = chunk_start; j <= r; j += p) is_prime[j - l] = 0;
        }

        /* build the current chunk's answer */
        for (long long i = l; i <= r; ++i)
            if (is_prime[i - l]) vector_push_back(&primes[chunk_num], i);
    }

    free(is_prime);

    struct timespec end_time_cpu, end_time_real;
    clock_gettime(CLOCK_THREAD_CPUTIME_ID, &end_time_cpu);
    clock_gettime(CLOCK_MONOTONIC, &end_time_real);

    cpuclock_time_static[cur_thread] =
        time_duration(&start_time_cpu, &end_time_cpu);
    wallclock_time_static[cur_thread] =
        time_duration(&start_time_real, &end_time_real);

    return NULL;
}

/* function to allocate chunk number */
int allocate_chunk() {
    static int current_chunk = 0;
    if (current_chunk >= total_chunks) return -1;
    int ret_val = current_chunk;
    current_chunk++;
    return ret_val;
}

/* function to be passed to thread */
void* run_thread_dynamic(void* arg) {
    /* allocate once per thread */
    int* is_prime = (int*)malloc(chunk_size * sizeof(int));

    int* thread_num_ptr = (int*)arg;
    int cur_thread = *thread_num_ptr;

    struct timespec start_time_cpu, start_time_real;
    clock_gettime(CLOCK_THREAD_CPUTIME_ID, &start_time_cpu);
    clock_gettime(CLOCK_MONOTONIC, &start_time_real);

    /* while chunks are being allocated */
    while (1) {
        /* get chunk number to operate on */
        pthread_mutex_lock(&mutex);
        int chunk_num = allocate_chunk();
        pthread_mutex_unlock(&mutex);

        if (chunk_num == -1) break;

        /* ascertain chunk boundaries and size */
        long long l = 1LL * chunk_num * chunk_size + 1;
        long long r = 1LL * (chunk_num + 1) * chunk_size;
        if (r > N) r = N;
        long long current_chunk_size = r - l + 1;

        /* boolean array of whether it is a prime or not */
        for (long long i = 0; i < current_chunk_size; ++i) is_prime[i] = 1;
        if (l == 1) is_prime[0] = 0;

        /* segmented sieve run */
        for (int i = 0; i < small_primes.size; ++i) {
            long long p = small_primes.a[i];
            long long p_square = p * p;
            if (p_square > r) break;
            long long chunk_start = p * ((l + p - 1) / p);
            if (chunk_start < p_square) chunk_start = p_square;
            for (long long j = chunk_start; j <= r; j += p) is_prime[j - l] = 0;
        }

        /* build the current chunk's answer */
        for (long long i = l; i <= r; ++i)
            if (is_prime[i - l]) vector_push_back(&primes[chunk_num], i);
    }

    free(is_prime);

    struct timespec end_time_cpu, end_time_real;
    clock_gettime(CLOCK_THREAD_CPUTIME_ID, &end_time_cpu);
    clock_gettime(CLOCK_MONOTONIC, &end_time_real);

    cpuclock_time_dynamic[cur_thread] =
        time_duration(&start_time_cpu, &end_time_cpu);
    wallclock_time_dynamic[cur_thread] =
        time_duration(&start_time_real, &end_time_real);

    return NULL;
}

/**
 * -------------------------- MAIN ----------------------------
 */

int main(int argc, char* argv[]) {
    /* parsing input parameters */
    num_threads = 10;
    N = 1000000;
    if (argc >= 2) num_threads = atoi(argv[1]);
    if (argc >= 3) N = atoll(argv[2]);
    dprintf("Setting options num_threads = %d, N = %lld...\n", num_threads, N);

    /* setting parameters */
    max_small_prime = (int)sqrt(N) - 2;
    if (max_small_prime < 0) max_small_prime = 0;
    while (1LL * max_small_prime * max_small_prime < N) ++max_small_prime;
    chunk_size = max_small_prime;
    total_chunks = (N + chunk_size - 1) / chunk_size;

    /* initializing data structures */
    primes = (vector*)malloc(total_chunks * sizeof(vector));
    for (int i = 0; i < total_chunks; ++i) vector_init(&primes[i]);
    is_small_prime = (int*)malloc((max_small_prime + 1) * sizeof(int));
    vector_init(&small_primes);

    /* initializing timing data */

    wallclock_time_static = (double*)malloc(num_threads * sizeof(double));
    wallclock_time_dynamic = (double*)malloc(num_threads * sizeof(double));
    cpuclock_time_static = (double*)malloc(num_threads * sizeof(double));
    cpuclock_time_dynamic = (double*)malloc(num_threads * sizeof(double));

    for (int i = 0; i < num_threads; ++i) {
        wallclock_time_static[i] = 0;
        wallclock_time_dynamic[i] = 0;
        cpuclock_time_static[i] = 0;
        cpuclock_time_dynamic[i] = 0;
    }

    /* initialize sieve of small size */
    for (int i = 2; i <= max_small_prime; ++i) is_small_prime[i] = 1;
    is_small_prime[0] = 0;
    is_small_prime[1] = 0;
    for (int i = 2; i <= max_small_prime; ++i) {
        if (is_small_prime[i]) {
            vector_push_back(&small_primes, i);
            if (1LL * i * i > max_small_prime) continue;
            for (int j = i * i; j <= max_small_prime; j += i) {
                is_small_prime[j] = 0;
            }
        }
    }

    int* thread_num = (int*) malloc(num_threads * sizeof(int));
    for (int i = 0; i < num_threads; ++i) thread_num[i] = i;

    /* static program */

    /* run all threads and all here */
    pthread_t* tid_static = (pthread_t*)malloc(num_threads * sizeof(pthread_t));
    for (int i = 0; i < num_threads; ++i) {
        pthread_attr_t attr;
        pthread_attr_init(&attr);
        pthread_create(&tid_static[i], &attr, run_thread_static, &thread_num[i]);
    }

    for (int i = 0; i < num_threads; ++i) pthread_join(tid_static[i], NULL);
    free(tid_static);

    /* dynamic program */

    for (int i = 0; i < total_chunks; ++i) {
        vector_destroy(&primes[i]);
        vector_init(&primes[i]);
    }

    /* run all threads and all here */
    pthread_t* tid_dynamic =
        (pthread_t*)malloc(num_threads * sizeof(pthread_t));
    for (int i = 0; i < num_threads; ++i) {
        pthread_attr_t attr;
        pthread_attr_init(&attr);
        pthread_create(&tid_dynamic[i], &attr, run_thread_dynamic, &thread_num[i]);
    }

    for (int i = 0; i < num_threads; ++i) pthread_join(tid_dynamic[i], NULL);
    free(tid_dynamic);

    /* print all primes in order */
    // for (int i = 0; i < total_chunks; ++i)
    //     for (int j = 0; j < primes[i].size; ++j)
    //         printf("%lld\n", primes[i].a[j]);

    printf("static allocation data follows\n");
    for (int i = 0; i < num_threads; ++i) {
        printf("thread %d:\n\twallclock: %lf\n\tcpuclock: %lf\n", i,
               wallclock_time_static[i], cpuclock_time_static[i]);
    }

    printf("dynamic allocation data follows\n");
    for (int i = 0; i < num_threads; ++i) {
        printf("thread %d:\n\twallclock: %lf\n\tcpuclock: %lf\n", i,
               wallclock_time_dynamic[i], cpuclock_time_dynamic[i]);
    }

    /* freeing timing data */
    free(wallclock_time_static);
    free(wallclock_time_dynamic);
    free(cpuclock_time_static);
    free(cpuclock_time_dynamic);

    /* freeing data structures */
    free(thread_num);
    vector_destroy(&small_primes);
    free(is_small_prime);
    for (int i = 0; i < total_chunks; ++i) vector_destroy(&primes[i]);
    free(primes);
    return 0;
}