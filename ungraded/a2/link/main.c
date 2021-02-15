#include <stdio.h>

int my_add(int a, int b);
int my_sub(int a, int b);

int main(int argc, char* argv[]) {
    printf("1 + 1 = %d\n", my_add(1, 1));
    printf("1 - 1 = %d\n", my_sub(1, 1));
    return 0;
}
