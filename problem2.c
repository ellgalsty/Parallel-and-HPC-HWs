#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <string.h>
#include <pthread.h>
#include <immintrin.h>

#define BUFFER_SIZE_MB 256
#define NUM_THREADS 4

typedef struct {
    char *buffer;
    size_t start;
    size_t end;
} ThreadData;

double now_sec(){
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec / 1e9;
}

char *generate_buffer(size_t n) {
    char *buf = malloc(n);
    const char chars[] =
        "abcdefghijklmnopqrstuvwxyz"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "0123456789"
        ".,!?;:()-_[]{}<>@#$%^&*+/= "
        ;

    int len = (int)strlen(chars);

    for (size_t i = 0; i < n; i++)
        buf[i] = chars[rand() % len];

    return buf;
}

void convert_scalar_range(char *buffer, size_t start, size_t end) {
    for (size_t i = start; i < end; i++) {
        if (buffer[i] >= 'a' && buffer[i] <= 'z')
            buffer[i] = buffer[i] - 32;
    }
}

void *thread_multithread(void *arg){
    ThreadData *td = (ThreadData *)arg;
    convert_scalar_range(td->buffer, td->start, td->end);
    return NULL;
}

void convert_multithreaded(char *buffer, size_t n) {
    pthread_t threads[NUM_THREADS];
    ThreadData td[NUM_THREADS];

    size_t chunk = n / NUM_THREADS;

    for (int i = 0; i < NUM_THREADS; i++) {
        td[i].buffer = buffer;
        td[i].start = i * chunk;
        td[i].end = (i == NUM_THREADS - 1) ? n : (i + 1) * chunk;

        pthread_create(&threads[i], NULL, thread_multithread, &td[i]);
    }

    for (int i = 0; i < NUM_THREADS; i++)
        pthread_join(threads[i], NULL);
}

void convert_simd(char *buffer, size_t n) {
    size_t i = 0;

    __m256i va = _mm256_set1_epi8('a');
    __m256i vz = _mm256_set1_epi8('z');
    __m256i diff = _mm256_set1_epi8(32);
    __m256i one = _mm256_set1_epi8(1);

    for (; i + 32 <= n; i += 32) {
        __m256i v = _mm256_loadu_si256((__m256i *)(buffer + i));

        __m256i ge_a = _mm256_cmpgt_epi8(v, _mm256_sub_epi8(va, one));
        __m256i le_z = _mm256_cmpgt_epi8(_mm256_add_epi8(vz, one), v);
        __m256i mask = _mm256_and_si256(ge_a, le_z);

        __m256i sub = _mm256_and_si256(mask, diff);
        v = _mm256_sub_epi8(v, sub);

        _mm256_storeu_si256((__m256i *)(buffer + i), v);
    }

    for (; i < n; i++) {
        if (buffer[i] >= 'a' && buffer[i] <= 'z')
            buffer[i] = buffer[i] - 32;
    }
}

void *thread_simd(void *arg) {
    ThreadData *td = (ThreadData *)arg;
    convert_simd(td->buffer + td->start, td->end - td->start);
    return NULL;
}

void convert_simd_multithreaded(char *buffer, size_t n) {
    pthread_t threads[NUM_THREADS];
    ThreadData td[NUM_THREADS];

    size_t chunk = n / NUM_THREADS;

    for (int i = 0; i < NUM_THREADS; i++) {
        td[i].buffer = buffer;
        td[i].start = i * chunk;
        td[i].end = (i == NUM_THREADS - 1) ? n : (i + 1) * chunk;

        pthread_create(&threads[i], NULL, thread_simd, &td[i]);
    }

    for (int i = 0; i < NUM_THREADS; i++)
        pthread_join(threads[i], NULL);
}

int main() {
    size_t n = BUFFER_SIZE_MB * 1024ULL * 1024ULL;

    srand(time(NULL));

    char *original = generate_buffer(n);
    char *buffer_mt = malloc(n);
    char *buffer_simd = malloc(n);
    char *buffer_simd_mt = malloc(n);

    memcpy(buffer_mt, original, n);
    memcpy(buffer_simd, original, n);
    memcpy(buffer_simd_mt, original, n);

    double t1, t2;

    t1 = now_sec();
    convert_multithreaded(buffer_mt, n);
    t2 = now_sec();
    double mt_time = t2 - t1;

    t1 = now_sec();
    convert_simd(buffer_simd, n);
    t2 = now_sec();
    double simd_time = t2 - t1;

    t1 = now_sec();
    convert_simd_multithreaded(buffer_simd_mt, n);
    t2 = now_sec();
    double simd_mt_time = t2 - t1;

    printf("buffer size: %d MB\n", BUFFER_SIZE_MB);
    printf("threads used: %d\n\n", NUM_THREADS);
    printf("multithreading time: %.3f sec\n", mt_time);
    printf("SIMD time: %.3f sec\n", simd_time);
    printf("SIMD + multithreading: %.3f sec\n", simd_mt_time);

    free(original);
    free(buffer_mt);
    free(buffer_simd);
    free(buffer_simd_mt);
    return 0;
}