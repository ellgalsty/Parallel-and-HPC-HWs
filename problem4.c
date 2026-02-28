#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define M 4

pthread_barrier_t barrier;

void* worker(void* arg) {
    int id = *(int*)arg;

    printf("thread %d: stage 1\n", id);
    usleep(100000);
    pthread_barrier_wait(&barrier);

    printf("Thread %d: stage 2\n", id);
    usleep(100000);
    pthread_barrier_wait(&barrier);

    printf("Thread %d: stage 3\n", id);
    usleep(100000);
    pthread_barrier_wait(&barrier);

    return NULL;
}

int main() {
    pthread_t threads[M];
    int ids[M];

    pthread_barrier_init(&barrier, NULL, M);

    for (int i = 0; i < M; i++) {
        ids[i] = i;
        pthread_create(&threads[i], NULL, worker, &ids[i]);
    }

    for (int i = 0; i < M; i++) {
        pthread_join(threads[i], NULL);
    }

    pthread_barrier_destroy(&barrier);
    return 0;
}