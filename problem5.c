#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sched.h>
#include <time.h>

#define NUM_THREADS 4
#define ITERATIONS 5000000000LL

void* worker(void* arg) {
    int id = *(int*)arg;

    unsigned long long x = 0;

    for (long long i = 0; i < ITERATIONS; i++) {
        x += i;
    }

    int cpu = sched_getcpu();

    printf("thread %d running on CPU %d\n", id, cpu);

    return NULL;
}

int main() {
    pthread_t threads[NUM_THREADS];
    int ids[NUM_THREADS];
    
    clock_t s1 = clock();

    for (int i = 0; i < NUM_THREADS; i++) {
        ids[i] = i;
        pthread_create(&threads[i], NULL, worker, &ids[i]);
    }

    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }
        
    clock_t e1 = clock();
    printf("time spent: %.3f seconds\n", (double)(e1 - s1) / CLOCKS_PER_SEC);

    return 0;
}