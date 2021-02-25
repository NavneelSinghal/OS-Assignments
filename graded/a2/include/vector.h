#ifndef INCLUDE_VECTOR
#define INCLUDE_VECTOR

#define data_t long long

typedef struct {
    int size;
    int capacity;
    data_t* a;
} vector;

void vector_init(vector*);
void vector_destroy(vector*);
void vector_push_back(vector*, data_t);

#endif
