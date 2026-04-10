#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define N 100000000
#define BINS 256

void init_array(int *A) {
    for (long i = 0; i < N; i++) {
        A[i] = rand() % 256;
    }
}

void zero_hist(int hist[BINS]) {
    for (int i = 0; i < BINS; i++) {
        hist[i] = 0;
    }
}

int main() {
    int *A = malloc(N * sizeof(int));
    if (A == NULL) {
        printf("oops no memory allocation this time\n");
        return 1;
    }

    int hist_naive[BINS];
    int hist_critical[BINS];
    int hist_reduction[BINS];

    srand(0);
    init_array(A);

    double start, end;

    zero_hist(hist_naive);
    start = omp_get_wtime();

    #pragma omp parallel for
    for (long i = 0; i < N; i++) {
        hist_naive[A[i]]++;
    }

    end = omp_get_wtime();
    printf("naive time: %.6f\n", end - start);

    zero_hist(hist_critical);
    start = omp_get_wtime();

    #pragma omp parallel for
    for (long i = 0; i < N; i++) {
        #pragma omp critical
        hist_critical[A[i]]++;
    }

    end = omp_get_wtime();
    printf("critical time: %.6f\n", end - start);

    zero_hist(hist_reduction);
    start = omp_get_wtime();

    #pragma omp parallel for reduction(+:hist_reduction[:BINS])
    for (long i = 0; i < N; i++) {
        hist_reduction[A[i]]++;
    }

    end = omp_get_wtime();
    printf("reduction time: %.6f\n", end - start);

    free(A);
    return 0;
}