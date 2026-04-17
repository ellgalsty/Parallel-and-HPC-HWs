#include <stdio.h>
#include <omp.h>

int fib(int n) {
    if (n <= 1)
        return n;

    // lets compute small values sequentially to reduce overhead
    if (n <= 10)
        return fib(n - 1) + fib(n - 2);

    int x, y;

    #pragma omp task shared(x)
    x = fib(n - 1);

    #pragma omp task shared(y)
    y = fib(n - 2);

    #pragma omp taskwait

    return x + y;
}

int main() {
    int num;
    scanf("%d", &num);

    int result;

    #pragma omp parallel
    {
        #pragma omp single
        result = fib(num);
    }

    printf("%d\n", result);
    return 0;
}