#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define N 20000

typedef struct {
    int request_id;
    int user_id;
    int response_time_ms;
    int category;
} LogEntry;

int main() {
    LogEntry logs[N];
    int fast_count = 0;
    int medium_count = 0;
    int slow_count = 0;

    #pragma omp parallel num_threads(4)
    {
        #pragma omp single
        {
            for (int i = 0; i < N; i++) {
                logs[i].request_id = i + 1;
                logs[i].user_id = rand() % 1000;
                logs[i].response_time_ms = rand() % 500;
                logs[i].category = -1;
            }
        }
        #pragma omp barrier

        #pragma omp for
        for (int i = 0; i < N; i++) {
            if (logs[i].response_time_ms < 100)
                logs[i].category = 0;
            else if (logs[i].response_time_ms <= 300)
                logs[i].category = 1;
            else
                logs[i].category = 2;
        }
        #pragma omp barrier

        #pragma omp single
        {
            for (int i = 0; i < N; i++) {
                if (logs[i].category == 0)
                    fast_count++;
                else if (logs[i].category == 1)
                    medium_count++;
                else if (logs[i].category == 2)
                    slow_count++;
            }

            printf("fast logs: %d\n", fast_count);
            printf("medium logs: %d\n", medium_count);
            printf("slow logs: %d\n", slow_count);
        }
    }

    return 0;
}