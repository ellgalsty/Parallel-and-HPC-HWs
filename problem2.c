#include <stdio.h>
#include <stdlib.h>
#include <math.h>
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
        printf("memory allocation failed\n");
        return 1;
    }

    srand(0);
    init_array(A);

    double min_diff = DBL_MAX;

    double start = omp_get_wtime();

    #pragma omp parallel for reduction(min:min_diff)
    for (long i = 1; i < N; i++) {
        double diff = fabs(A[i] - A[i - 1]);
        if (diff < min_diff) {
            min_diff = diff;
        }
    }

    double end = omp_get_wtime();

    printf("minimum difference: %f\n", min_diff);
    printf("time: %.6f seconds\n", end - start);

    free(A);
    return 0;
}