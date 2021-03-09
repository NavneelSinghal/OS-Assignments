#ifndef INCLUDE_QUEUE
#define INCLUDE_QUEUE

typedef struct node_t {
    void* data;
    struct node_t *nxt;
} node;

typedef struct queue_t {
    int size;
    node* head;
    node* tail;
} queue;

/* create node */
node* node_create(void* data);

/* create an empty queue */
queue* queue_create();

/* push a node into the queue */
void queue_push(queue* q, node* n);

/* remove and return the node if exists, return NULL otherwise */
node* queue_erase(queue* q, void* data);

/* gets the first node */
node* queue_peek(queue* q);

/* destroy queue and all of its data? */
void queue_destroy(queue* q);

#endif
