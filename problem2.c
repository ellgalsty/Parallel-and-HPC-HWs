#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>

#define N 4

pthread_barrier_t barrier;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void* player(void* arg) {
    int id = *(int*)arg;

    pthread_mutex_lock(&mutex);
    int t = rand() % 3 + 1;
    pthread_mutex_unlock(&mutex);

    printf("player %d getting ready (%d sec)\n", id, t);
    sleep(t);
    printf("player %d ready!\n", id);

    pthread_barrier_wait(&barrier);

    printf("player %d: game Started!\n", id);

    return NULL;
}

int main() {
    srand((unsigned)time(NULL));

    pthread_barrier_init(&barrier, NULL, N);

    pthread_t threads[N];
    int ids[N];

    for (int i = 0; i < N; i++) {
        ids[i] = i;
        pthread_create(&threads[i], NULL, player, &ids[i]);
    }

    for (int i = 0; i < N; i++) {
        pthread_join(threads[i], NULL);
    }

    pthread_barrier_destroy(&barrier);
    return 0;
}