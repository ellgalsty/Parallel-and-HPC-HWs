#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define N 10000

typedef struct {
    int order_id;
    float distance_km;
    int priority;
} Order;

int main() {
    Order orders[N];
    int thread_high_count[4] = {0, 0, 0, 0};
    float threshold = 0.0;

    for (int i = 0; i < N; i++) {
        orders[i].order_id = i + 1;
        orders[i].distance_km = (float)(rand() % 50);
        orders[i].priority = 0;
    }

    #pragma omp parallel num_threads(4)
    {
        int tid = omp_get_thread_num();

        #pragma omp single
        {
            threshold = 20.0;
            printf("Distance threshold set to %.1f km\n", threshold);
        }

        #pragma omp for
        for (int i = 0; i < N; i++) {
            if (orders[i].distance_km < threshold)
                orders[i].priority = 1;
            else
                orders[i].priority = 0;
        }
        #pragma omp barrier

        #pragma omp single
        {
            printf("Priority assignment is finished.\n");
        }

        int local_count = 0;

        #pragma omp for
        for (int i = 0; i < N; i++) {
            if (orders[i].priority == 1)
                local_count++;
        }

        thread_high_count[tid] = local_count;

        #pragma omp barrier

        #pragma omp single
        {
            int total_high = 0;

            for (int i = 0; i < 4; i++) {
                printf("thread_high_count[%d] = %d\n", i, thread_high_count[i]);
                total_high += thread_high_count[i];
            }

            printf("T=total high priority orders = %d\n", total_high);
        }
    }

    return 0;
}