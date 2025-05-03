// File: lab4/race.c
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define NUM_THREADS 20

long accum = 0;

void *square(void *param) {
    int x = *(int *)param;
    accum += x * x;
    //sleep(1);
    pthread_exit(NULL);
}

int main() {
    pthread_t threads[NUM_THREADS];
    int *params[NUM_THREADS];

    for (long t = 0; t < NUM_THREADS; t++) {
        params[t] = malloc(sizeof(int));
        *params[t] = t + 1;
        pthread_create(&threads[t], NULL, square, (void *)params[t]);
    }

    for (long t = 0; t < NUM_THREADS; t++) {
        pthread_join(threads[t], NULL);
        free(params[t]);
    }

    printf("%ld\n", accum);
    pthread_exit(NULL);
}


// the race condition is on the accum variable, which is shared among all threads
/*
the easyest way to fix this is to use a mutex to lock the variable while it is being modified

long accum = 0;
pthread_mutex_t accum_mutex = PTHREAD_MUTEX_INITIALIZER;

void *square(void *param) {
    int x = *(int *)param;
    
    // Lock before updating the shared variable
    pthread_mutex_lock(&accum_mutex);
    accum += x * x;
    pthread_mutex_unlock(&accum_mutex);

*/