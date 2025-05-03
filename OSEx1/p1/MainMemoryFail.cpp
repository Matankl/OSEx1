#include <iostream>
#include <stdio.h>

int main() {
    //accessing memory that is not allocated
    int *p = (int*)0;
    printf("%d\n", *p); // Segmentation fault (core dumped)

    return 0;
}
