#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

#define SIZE 50000000UL
#define NUM_THREADS 4

int *a;
long long partial_sums[NUM_THREADS];

void* thread_sum(void* arg) {
    int id = *(int*)arg;

    int chunk = SIZE / NUM_THREADS;
    int start = id * chunk;
    int end = (id == NUM_THREADS - 1) ? SIZE : start + chunk;

    partial_sums[id] = 0;

    for (int i = start; i < end; i++) {
        partial_sums[id] += a[i];
    }

    return NULL;
}

int main() {
    a = malloc((size_t)SIZE * sizeof(int));
    if (!a) {
            return 1;
    }
    srand(1);
    for (int i=0; i<SIZE; i++) {
            a[i]=rand();
    }

      clock_t start = clock();

    long long sum = 0;
    for (int i = 0; i < SIZE; i++) {
        sum += a[i];
    }

    clock_t end = clock();

    printf("sequential sum: %lld\n", sum);
    printf("sequential case time: %.3f seconds\n", (double)(end - start)/CLOCKS_PER_SEC);

    
    pthread_t threads[NUM_THREADS];
    int ids[NUM_THREADS];

    clock_t start2 = clock();

    for (int i = 0; i < NUM_THREADS; i++) {
        ids[i] = i;
        pthread_create(&threads[i], NULL, thread_sum, &ids[i]);
    }

    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    long long total = 0;
    for (int i = 0; i < NUM_THREADS; i++) {
        total += partial_sums[i];
    }

    clock_t end2 = clock();

    printf("\nthread case sum: %lld\n", total);
    printf("thread case time: %.3f seconds\n",
           (double)(end2 - start2) / CLOCKS_PER_SEC);

    free(a);

    return 0;
}