#include <math.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#include "../include/vector.h"

// assume C99
#ifdef DEBUG
#define dprintf(fmt, ...) fprintf(stderr, fmt, __VA_ARGS__);
#else
#define dprintf(...)
#endif

// TODO: look at optimizations for even numbers?

/**
 * data
 */
int num_threads, chunk_size, total_chunks, max_small_prime;
long long N;
int* is_small_prime;
vector small_primes;
vector* primes;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

/* function to be passed to thread */
void* run_thread(void* arg) {
    /* allocate once per thread */
    int* is_prime = (int*)malloc(chunk_size * sizeof(int));

    int* thread_num_ptr = (int*) arg;
    int cur_thread = *thread_num_ptr;
    free(thread_num_ptr);

    /* statically run over the chunks */
    for (int chunk_num = cur_thread; chunk_num < total_chunks; chunk_num += num_threads) {
        
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
            for (long long j = chunk_start; j <= r; j += p) is_prime[j - l] =
            0;
        }

        /* build the current chunk's answer */
        for (long long i = l; i <= r; ++i)
            if (is_prime[i - l]) vector_push_back(&primes[chunk_num], i);
    }

    free(is_prime);
    return NULL;
}

int main(int argc, char* argv[]) {
    /* parsing input parameters */
    num_threads = 10;
    N = 1000000;
    if (argc >= 2) num_threads = atoi(argv[1]);
    if (argc >= 3) N = atoll(argv[2]);
    dprintf("Setting options num_threads = %d, N = %d...\n", num_threads, N);

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

    /* do stuff now */

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

    /* run all threads and all here */
    pthread_t* tid = (pthread_t*)malloc(num_threads * sizeof(pthread_t));
    for (int i = 0; i < num_threads; ++i) {
        pthread_attr_t attr;
        pthread_attr_init(&attr);
        int* thread_num = malloc(sizeof(int));
        *thread_num = i;
        pthread_create(&tid[i], &attr, run_thread, &thread_num);
    }
    for (int i = 0; i < num_threads; ++i) pthread_join(tid[i], NULL);
    free(tid);

    /* print all primes in order */
    for (int i = 0; i < total_chunks; ++i)
        for (int j = 0; j < primes[i].size; ++j)
            printf("%lld\n", primes[i].a[j]);

    /* freeing data structures */
    vector_destroy(&small_primes);
    free(is_small_prime);
    for (int i = 0; i < total_chunks; ++i) vector_destroy(&primes[i]);
    free(primes);
    return 0;
}
