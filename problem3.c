#include <stdio.h>
#include <stdlib.h>
#include <float.h>
#include <omp.h>

#define N 50000000

void init_array(double *A) {
    for (long i = 0; i < N; i++) {
        A[i] = (double)rand() / RAND_MAX;
    }
}

int main() {
    double *A = malloc(N * sizeof(double));
    if (A == NULL) {
        printf("memory allocation failed, sry dude\n");
        return 1;
    }

    srand(0);
    init_array(A);

    double max_val = -DBL_MAX;
    double sum = 0.0;
    double T;

    double start, end;

    start = omp_get_wtime();

    #pragma omp parallel for reduction(max:max_val)
    for (long i = 0; i < N; i++) {
        if (A[i] > max_val) {
            max_val = A[i];
        }
    }

    T = 0.8 * max_val;

    #pragma omp parallel for reduction(+:sum)
    for (long i = 0; i < N; i++) {
        if (A[i] > T) {
            sum += A[i];
        }
    }

    end = omp_get_wtime();

    printf("max value: %f\n", max_val);
    printf("threshold T: %f\n", T);
    printf("filtered sum: %f\n", sum);
    printf("time: %.6f seconds\n", end - start);

    free(A);
    return 0;
}