#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>

typedef struct boundedqueue {
    pthread_mutex_t mutex;
    pthread_cond_t not_full;
    pthread_cond_t not_empty;
    int *a;
    int capacity;
    int size;
    int first;
    int last;
} boundedqueue;

void init(boundedqueue *q, int n) {
    pthread_mutex_init(&(q->mutex), NULL);
    pthread_cond_init(&(q->not_empty), NULL);
    pthread_cond_init(&(q->not_full), NULL);
    q->capacity = n;
    q->a = (int *)malloc(n * sizeof(int));
    q->size = 0;
    q->first = 0;
    q->last = 0;
}

void clear(boundedqueue *q) {
    pthread_mutex_destroy(&(q->mutex));
    pthread_cond_destroy(&(q->not_empty));
    pthread_cond_destroy(&(q->not_full));
    free(q->a);
}

int isFull(boundedqueue *q) { printf("FULL\n"); return q->capacity == q->size; }

int isEmpty(boundedqueue *q) { printf("EMPTY\n"); return q->size == 0; }

void push(boundedqueue *q, int x) {
    pthread_mutex_lock(&(q->mutex));
    while (isFull(q)) {
        pthread_cond_wait(&(q->not_full), &(q->mutex));
    }
    assert(!isFull(q));
    printf("adding element %d to q\n", x);
    (q->a)[q->last] = x;
    (q->size)++;
    q->last = (q->last + 1) % (q->capacity);
    pthread_cond_signal(&(q->not_empty));
    pthread_mutex_unlock(&(q->mutex));
    return;
}

int pop(boundedqueue *q) {
    pthread_mutex_lock(&(q->mutex));
    while (isEmpty(q)) {
        pthread_cond_wait(&(q->not_empty), &(q->mutex));
    }
    assert(!isEmpty(q));
    printf("removing element from q\n");
    int ans = (q->a)[q->first];
    (q->size)--;
    q->first = (q->first + 1) % (q->capacity);
    pthread_cond_signal(&(q->not_full));
    pthread_mutex_unlock(&(q->mutex));
    return ans;
}

int n, m;
int stop;
int total;

boundedqueue q;

pthread_mutex_t mutex;

pthread_t *producer_threads, *consumer_threads;

void *produce(void *arg) {
    while (1) {
        pthread_mutex_lock(&mutex);
        if (total == stop) {
            pthread_mutex_unlock(&mutex);
            pthread_exit(NULL);
        }
        push(&q, total);
        total++;
        printf(
            "Thread %d has deposited a container into the boundedqueue, total "
            "insertions/deletions: %d\n",
            (int)gettid() - 2, total);
        pthread_mutex_unlock(&mutex);
    }
    pthread_exit(NULL);
}

void *consume(void *arg) {
    while (1) {
        pthread_mutex_lock(&mutex);
        if (total == stop) {
            pthread_mutex_unlock(&mutex);
            pthread_exit(NULL);
        }
        pop(&q);
        total++;
        printf(
            "Thread %d has picked up a container from the boundedqueue, total "
            "insertions/deletions: %d\n",
            (int)gettid() - 2, total);
        pthread_mutex_unlock(&mutex);
    }
    pthread_exit(NULL);
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

    pthread_mutex_init(&mutex, NULL);

    init(&q, n);

    total = 0;

    producer_threads = (pthread_t *)malloc(m * sizeof(pthread_t));
    consumer_threads = (pthread_t *)malloc(m * sizeof(pthread_t));

    for (int i = 0; i < m; ++i) {
        pthread_attr_t attr;
        pthread_attr_init(&attr);
        pthread_create(&producer_threads[i], &attr, produce, NULL);
    }

    for (int i = 0; i < m; ++i) {
        pthread_attr_t attr;
        pthread_attr_init(&attr);
        pthread_create(&consumer_threads[i], &attr, consume, NULL);
    }

    for (int i = 0; i < 1e7; ++i) i += rand() % 2;

    for (int i = 0; i < m; ++i) {
        pthread_join(producer_threads[i], NULL);
    }

    for (int i = 0; i < m; ++i) {
        pthread_join(consumer_threads[i], NULL);
    }

    pthread_mutex_destroy(&mutex);

    clear(&q);

    free(producer_threads);
    free(consumer_threads);

    return 0;
}
