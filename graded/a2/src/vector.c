#include "../include/vector.h"

#include <stdlib.h>
#include <stdio.h>

#define DEFAULT_CAPACITY 16

void vector_init(vector* v) {
    v->size = 0;
    v->capacity = DEFAULT_CAPACITY;
    v->a = (data_t*)malloc(DEFAULT_CAPACITY * sizeof(data_t));
}

void vector_destroy(vector* v) {
    v->size = 0;
    v->capacity = 0;
    free(v->a);
}

/* can potentially lead to memory leaks */
void vector_push_back(vector* v, data_t x) {
    if (v->capacity == 0) {
        fprintf(stderr, "vector_push_back to a destroyed vector failed\n");
        exit(1);
    }
    if (v->size >= v->capacity) {
        v->capacity *= 2;
        data_t* new_a = (data_t*)realloc(v->a, v->capacity * sizeof(data_t));
        if (!new_a) {
            vector_destroy(v);
            v->a = NULL;
            fprintf(stderr, "reallocation of vector failed\n");
            exit(1);
        } else {
            v->a = new_a;
        }
    }
    v->a[v->size++] = x;
}
