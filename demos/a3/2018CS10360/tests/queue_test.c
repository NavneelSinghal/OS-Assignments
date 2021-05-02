#include "../include/queue.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

void dprint(const char *format, ...) {
    va_list args;
    // fprintf(stderr, "Log:\n");
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    fprintf(stderr, "\n");
}

int main() {
    queue *q = queue_create();
    for (int i = 0; i < 3; ++i) {
        int *j = malloc(sizeof(int));
        *j = i;
        queue_push(q, node_create(j));
        dprint("%d", q->size);
    }
    queue_destroy(q);
}
