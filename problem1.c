#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

#define P 4
#define R 10

pthread_barrier_t barrier;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

int rolls[P];
int wins[P];

void* player(void* arg) {
    int id = *(int*)arg;

    for (int round=1; round<=R; round++) {
	pthread_mutex_lock(&mutex);
	int roll = rand() % 6 + 1;
	pthread_mutex_unlock(&mutex);
	rolls[id] = roll;

	printf("round %d player %d rolled %d\n", round, id, roll);

	pthread_barrier_wait(&barrier);
	if (id ==0) {
	    int max = rolls[0];

	    for (int i = 1; i < P; i++) {
                if (rolls[i] > max) max = rolls[i];
            }
        	int winners_count = 0;
                int last_winner = -1;
                for (int i = 0; i < P; i++) {
                if (rolls[i] == max) {
                    winners_count++;
                    last_winner = i;
                }
            }

            if (winners_count == 1) {
                wins[last_winner]++;
                printf("*** Round %d winner: player %d (roll %d)\n\n",
                       round, last_winner, max);
            } else {
                printf("*** Round %d is a tie (max roll %d)\n\n", round, max);
            }
	}
	pthread_barrier_wait(&barrier);
	}
	return NULL;
}

int main() {
    srand((unsigned)time(NULL));

    pthread_barrier_init(&barrier, NULL, P);

    pthread_t threads[P];
    int ids[P];

    for (int i = 0; i < P; i++) {
        ids[i] = i;
        wins[i] = 0;
        pthread_create(&threads[i], NULL, player, &ids[i]);
    }

    for (int i = 0; i < P; i++) {
        pthread_join(threads[i], NULL);
    }

    // finallyyy
    printf("final wins:\n");
    for (int i = 0; i < P; i++) {
        printf("player %d: %d wins\n", i, wins[i]);
    }

    int best = wins[0];
    for (int i = 1; i < P; i++) {
        if (wins[i] > best) best = wins[i];
    }

    int winners_count = 0;
    int last_winner = -1;
    for (int i = 0; i < P; i++) {
        if (wins[i] == best) {
            winners_count++;
            last_winner = i;
        }
    }

    if (winners_count == 1) {
        printf("overall winner: player %d (%d wins)\n", last_winner, best);
    } else {
        printf("overall result: tie (%d wins)\n", best);
    }

    pthread_barrier_destroy(&barrier);
    return 0;
}