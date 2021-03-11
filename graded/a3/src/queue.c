#include "../include/queue.h"

#include <stdio.h>
#include <stdlib.h>

node* node_create(void* data) {
    node* n = (node*)malloc(sizeof(node));
    if (n == NULL) {
        // return NULL;
        fprintf(stderr, "node malloc failed\n");
        exit(1);
    }
    n->data = data;
    n->nxt = NULL;
    return n;
}

queue* queue_create() {
    queue* ret;
    ret = (queue*)malloc(sizeof(queue));
    if (ret == NULL) {
        // return NULL;
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
        // return;
        fprintf(stderr, "can't add to null queue");
        exit(1);
    }
    if (q->head == NULL) {
        q->head = n;
        q->tail = n;
        q->tail->nxt = NULL;
    } else {
        q->tail->nxt = n;
        q->tail = n;
        q->tail->nxt = NULL;
    }
    ++(q->size);
}

node* queue_erase(queue* q, void* data) {
    if (q == NULL) {
        // return NULL;
        fprintf(stderr, "tried popping from null queue");
        exit(1);
    }
    if (q->size == 0) {
        // return NULL;
        fprintf(stderr, "tried popping from empty queue");
        exit(1);
    }
    if (q->size == 1) {
        if (q->head->data == data) {
            node *n = q->head;
            q->head = NULL;
            q->tail = NULL;
            --(q->size);
            n->nxt = NULL;
            return n;
        } else {
            return NULL;
        }
    } else {
        node *n = q->head;
        node *prev = NULL;
        while (n != NULL && n->data != data) {
            prev = n;
            n = n->nxt;
        }
        if (n == NULL) {
            return NULL;
        } else {
            --(q->size);
            if (n == q->head) {
                q->head = q->head->nxt;
                n->nxt = NULL;
                return n;
            } else if (n == q->tail) {
                prev->nxt = NULL;
                q->tail = prev;
                return n;
            } else {
                prev->nxt = n->nxt;
                n->nxt = NULL;
                return n;
            }
        }
    }
}

node* queue_peek(queue *q) {
    return q->head;
}

void queue_destroy(queue* q) {
    if (q == NULL) return;
    while (q->size > 0) {
        node* n = queue_erase(q, q->head->data);
        // TODO: comment out later on
        // free(n->data);
        free(n);
    }
    free(q);
}
