#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

#define S 4
#define R 5 

pthread_barrier_t barrier;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

double temps[S][R];

void* sensor(void* arg) {
    int id = *(int*)arg;

    for (int r = 0; r < R; r++) {
        pthread_mutex_lock(&mutex);
        double t = 15.0 + (rand() % 2011) / 100.0;
        pthread_mutex_unlock(&mutex);

        temps[id][r] = t;
        printf("sensor %d collected %.2f\n", id, t);
    }

    pthread_barrier_wait(&barrier);

    if (id == 0) {
        double sum = 0.0;
        int count = 0;

        for (int i = 0; i < S; i++) {
            for (int r = 0; r < R; r++) {
                sum += temps[i][r];
                count++;
            }
        }

        printf("\naverage temperature: %.2f\n", sum / count);
    }

    return NULL;
}

int main() {
    srand((unsigned)time(NULL));

    pthread_barrier_init(&barrier, NULL, S);

    pthread_t threads[S];
    int ids[S];

    for (int i = 0; i < S; i++) {
        ids[i] = i;
        pthread_create(&threads[i], NULL, sensor, &ids[i]);
    }

    for (int i = 0; i < S; i++) {
        pthread_join(threads[i], NULL);
    }

    pthread_barrier_destroy(&barrier);
    return 0;
}