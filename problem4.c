#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

#define N 20000000
#define NUM_THREADS 4

typedef struct {
    int start;
    int end;
    int count;
} ThreadData;

int is_prime(int x) {
    if (x < 2) return 0;
    if (x == 2) return 1;
    if (x % 2 == 0) return 0;

    for (int i = 3; i * i <= x; i += 2) {
        if (x % i == 0) return 0;
    }
    return 1;
}

void* thread_count_primes(void* arg) {
    ThreadData* data = (ThreadData*)arg;

    int c = 0;
    for (int i = data->start; i <= data->end; i++) {
        if (is_prime(i)) c++;
    }

    data->count = c;
    return NULL;
}

int main() {
    clock_t s1 = clock();

    int seq_count = 0;
    for (int i = 1; i <= N; i++) {
        if (is_prime(i)) seq_count++;
    }

    clock_t e1 = clock();

    printf("sequential case prime count: %d\n", seq_count);
    printf("sequential case time: %.3f seconds\n", (double)(e1 - s1) / CLOCKS_PER_SEC);

    pthread_t threads[NUM_THREADS];
    ThreadData data[NUM_THREADS];

    int chunk = N / NUM_THREADS;

    clock_t s2 = clock();

    for (int t = 0; t < NUM_THREADS; t++) {
        data[t].start = t * chunk + 1;
        data[t].end = (t == NUM_THREADS - 1) ? N : (t + 1) * chunk;
        data[t].count = 0;
        pthread_create(&threads[t], NULL, thread_count_primes, &data[t]);
    }

    for (int t = 0; t < NUM_THREADS; t++) {
        pthread_join(threads[t], NULL);
    }

    int total = 0;
    for (int t = 0; t < NUM_THREADS; t++) {
        total += data[t].count;
    }

    clock_t e2 = clock();

    printf("multithread prime count (%d threads): %d\n", NUM_THREADS, total);
    printf("multithread case time: %.3f seconds\n",
           (double)(e2 - s2) / CLOCKS_PER_SEC);

    return 0;
}