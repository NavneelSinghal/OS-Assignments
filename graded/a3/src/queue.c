#include "../include/queue.h"

#include <stdio.h>
#include <stdlib.h>

node* node_create(void* data) {
    node* n = (node*)malloc(sizeof(node));
    if (n == NULL) {
        fprintf(stderr, "node malloc failed\n");
        exit(1);
    }
    n->data = data;
    n->nxt = NULL;
    return n;
}

/* do a circular queue later on */

queue* queue_create() {
    queue* ret;
    ret = (queue*)malloc(sizeof(queue));
    if (ret == NULL) {
        fprintf(stderr, "queue malloc failed\n");
        exit(1);
    }
    ret->head = NULL;
    ret->tail = NULL;
    ret->size = 0;
    return ret;
}

void queue_push(queue* q, node* n) {
    if (q == NULL) {
        fprintf(stderr, "can't add to null queue");
        exit(1);
    }
    if (q->head == NULL) {
        q->head = n;
        q->tail = n;
        q->tail->nxt = q->head;
    } else {
        q->tail->nxt = n;
        n->nxt = q->head;
    }
    ++q->size;
}


/* TODO */
node* queue_pop(queue* q, void* data) {
    if (q == NULL) {
        fprintf(stderr, "tried popping from null queue");
        exit(1);
    }
    if (q->size == 0) {
        fprintf(stderr, "tried popping from empty queue");
        exit(1);
    }
    if (q->size == 1) {
        if (q->head->data == data) {
            node *n = q->head;
            q->head = NULL;
            q->tail = NULL;
            --q->size;
            n->nxt = NULL;
            return n;
        } else {
            return NULL;
        }
    } else {
        node *n = q->head;
        node *prev = q->tail;
        while (n->data != data && n->nxt != q->head) {
            prev = n;
            n = n->nxt;
        }
        if (n == q->head) {
            if (n->data == data) {

            } else {
                return NULL;
            }
        } else {

        }
    }
    node* n = q->head;
    q->head = q->head->nxt;
    --q->size;
    void* ret = n->data;
    free(n);
    return ret;
}

void queue_destroy(queue* q) {
    if (q == NULL) return;
    while (q->size > 0) {
        free(queue_pop(q));
    }
    free(q);
}
