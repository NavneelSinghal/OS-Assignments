#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

#include "../include/myOwnthread.h"

typedef struct boundedqueue {
    mutex_t mutex;
    cv_t not_full;
    cv_t not_empty;
    int *a;
    int capacity;
    int size;
    int first;
    int last;
} boundedqueue;

void init(boundedqueue *q, int n) {
    myThread_mutex_init(&(q->mutex), NULL);
    myThread_cond_init(&(q->not_empty), NULL);
    myThread_cond_init(&(q->not_full), NULL);
    q->capacity = n;
    q->a = (int *)malloc(n * sizeof(int));
    q->size = 0;
    q->first = 0;
    q->last = 0;
}

void clear(boundedqueue *q) {
    myThread_mutex_destroy(&(q->mutex));
    myThread_cond_destroy(&(q->not_empty));
    myThread_cond_destroy(&(q->not_full));
    free(q->a);
}

int isFull(boundedqueue *q) { return q->capacity == q->size; }

int isEmpty(boundedqueue *q) { return q->size == 0; }

void push(boundedqueue *q, int x) {
    myThread_mutex_lock(&(q->mutex));
    while (isFull(q)) {
        myThread_cond_wait(&(q->not_full), &(q->mutex));
    }
    assert(!isFull(q));
    printf("adding element %d to q", x);
    (q->a)[q->last] = x;
    (q->size)++;
    q->last = (q->last + 1) % (q->capacity);
    myThread_cond_signal(&(q->not_empty));
    myThread_mutex_unlock(&(q->mutex));
    return;
}

int pop(boundedqueue *q) {
    myThread_mutex_lock(&(q->mutex));
    while (isEmpty(q)) {
        myThread_cond_wait(&(q->not_empty), &(q->mutex));
    }
    assert(!isEmpty(q));
    int ans = (q->a)[q->first];
    (q->size)--;
    q->first = (q->first + 1) % (q->capacity);
    myThread_cond_signal(&(q->not_full));
    myThread_mutex_unlock(&(q->mutex));
    return ans;
}

int n, m;
int stop;
int total;

boundedqueue q;

mutex_t mutex;

myThread_t *producer_threads, *consumer_threads;

void *produce(void *arg) {
    while (1) {
        myThread_mutex_lock(&mutex);
        if (total == stop) {
            myThread_mutex_unlock(&mutex);
            myThread_exit(NULL);
        }
        push(&q, total);
        total++;
        printf(
            "Thread %d has deposited a container into the boundedqueue, total "
            "insertions/deletions: %d\n",
            myThread_self()->tid - 2, total);
        myThread_mutex_unlock(&mutex);
    }
    myThread_exit(NULL);
}

void *consume(void *arg) {
    while (1) {
        myThread_mutex_lock(&mutex);
        if (total == stop) {
            myThread_mutex_unlock(&mutex);
            myThread_exit(NULL);
        }
        pop(&q);
        total++;
        printf(
            "Thread %d has picked up a container from the boundedqueue, total "
            "insertions/deletions: %d\n",
            myThread_self()->tid - 2, total);
    }
    myThread_mutex_unlock(&mutex);
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

    init(&q, n);

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

    for (int i = 0; i < 1e7; ++i) i += rand() % 2;

    for (int i = 0; i < m; ++i) {
        myThread_join(producer_threads[i], NULL);
    }

    for (int i = 0; i < m; ++i) {
        myThread_join(consumer_threads[i], NULL);
    }

    myThread_mutex_destroy(&mutex);

    clear(&q);

    free(producer_threads);
    free(consumer_threads);

    return 0;
}
