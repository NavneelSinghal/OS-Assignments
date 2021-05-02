#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

#include "../include/myOwnthread.h"

int n, m;
int stop;

int *buf; /* the buffer */
int left,
    right; /* left is the position where the consumer picks up stuff from,
              and right is the position where the producer deposits stuff */
int total;

mutex_t mutex;

cv_t is_empty;
cv_t is_full;

myThread_t *producer_threads, *consumer_threads;

void *produce(void *arg) {
    myThread_mutex_lock(&mutex);
    while (total < stop) {
        myThread_mutex_unlock(&mutex);
        myThread_mutex_lock(&mutex);
        if (total == stop) {
            myThread_mutex_unlock(&mutex);
            myThread_exit(NULL);
        }
        while (right - left == n) {
            myThread_cond_wait(&is_full, &mutex);
        }
        if (total >= stop) {
            myThread_cond_signal(&is_empty);
            myThread_mutex_unlock(&mutex);
            break;
        } else {
            buf[right] = right;
            right++;
            total++;
            printf(
                "Thread %d has deposited a container into the queue, total "
                "insertions/deletions: %d\n",
                myThread_self()->tid - 2, total);
            myThread_cond_signal(&is_empty);
            myThread_mutex_unlock(&mutex);
        }
    }
    myThread_exit(NULL);
}

void *consume(void *arg) {
    myThread_mutex_lock(&mutex);
    while (total < stop) {
        myThread_mutex_unlock(&mutex);
        myThread_mutex_lock(&mutex);
        if (total == stop) {
            myThread_mutex_unlock(&mutex);
            myThread_exit(NULL);
        }
        while (right == left) {
            myThread_cond_wait(&is_empty, &mutex);
        }
        if (total >= stop) {
            myThread_cond_signal(&is_full);
            myThread_mutex_unlock(&mutex);
            break;
        } else {
            buf[left] *= -1;
            left++;
            total++;
            printf(
                "Thread %d has picked up a container from the queue, total "
                "insertions/deletions: %d\n",
                myThread_self()->tid - 2, total);
            myThread_cond_signal(&is_full);
            myThread_mutex_unlock(&mutex);
        }
    }
    myThread_exit(NULL);
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr,
                "usage: %s <number of producers> <number of containers>\n",
                argv[0]);
        exit(1);
    }
    m = atoi(argv[1]);
    n = atoi(argv[2]);
    stop = n * 10;

    if (m <= n) {
        fprintf(
            stderr,
            "invalid input, there should be more producers than containers\n");
        exit(1);
    }

    myThread_mutex_init(&mutex, NULL);
    myThread_cond_init(&is_full, NULL);
    myThread_cond_init(&is_empty, NULL);

    buf = (int *)malloc(20 * n * sizeof(int));
    left = 0;
    right = 0;
    total = 0;

    producer_threads = (myThread_t *)malloc(m * sizeof(myThread_t));
    consumer_threads = (myThread_t *)malloc(m * sizeof(myThread_t));

    for (int i = 0; i < m; ++i) {
        myThread_attr_t attr;
        myThread_attr_init(&attr);
        myThread_create(&producer_threads[i], &attr, produce, NULL);
    }

    for (int i = 0; i < m; ++i) {
        myThread_attr_t attr;
        myThread_attr_init(&attr);
        myThread_create(&consumer_threads[i], &attr, consume, NULL);
    }

    for (long long i = 0; i < n * 2e8; ++i); exit(0);

    for (int i = 0; i < m; ++i) {
        myThread_join(consumer_threads[i], NULL);
    }

    for (int i = 0; i < m; ++i) {
        myThread_join(producer_threads[i], NULL);
    }

    myThread_mutex_destroy(&mutex);
    myThread_cond_destroy(&is_full);
    myThread_cond_destroy(&is_empty);

    free(producer_threads);
    free(consumer_threads);
    free(buf);

    return 0;
}
