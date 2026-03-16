#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <pthread.h>
#include <time.h>
#include <immintrin.h>

#define NUM_THREADS 4

typedef struct {
    uint8_t *input;
    uint8_t *output;
    int start_pixel;
    int end_pixel;
} ThreadData;

static int width, height, maxval;

double now_sec() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec / 1e9;
}

void read_ppm(const char *filename, uint8_t **data) {
    FILE *f = fopen(filename, "rb");
    if (!f) {
        perror("fopen");
        exit(1);
    }

    char format[3];

    if (fscanf(f, "%2s", format) != 1) exit(1);
    if (strcmp(format, "P6") != 0) exit(1);

    if (fscanf(f, "%d %d", &width, &height) != 2) exit(1);
    if (fscanf(f, "%d", &maxval) != 1) exit(1);

    fgetc(f);

    int size = width * height * 3;
    *data = (uint8_t *)malloc(size);
    if (!*data) exit(1);

    if ((int)fread(*data, 1, size, f) != size) exit(1);

    fclose(f);
}

void write_ppm(const char *filename, uint8_t *data) {
    FILE *f = fopen(filename, "wb");
    if (!f) {
        perror("fopen");
        exit(1);
    }

    fprintf(f, "P6\n%d %d\n%d\n", width, height, maxval);
    fwrite(data, 1, width * height * 3, f);
    fclose(f);
}

void grayscale_scalar_range(uint8_t *input, uint8_t *output, int start_pixel, int end_pixel){
    for (int i = start_pixel; i < end_pixel; i++) {
        int r = input[i * 3];
        int g = input[i * 3 + 1];
        int b = input[i * 3 + 2];

        uint8_t gray = (uint8_t)((77 * r + 150 * g + 29 * b) >> 8);

        output[i * 3] = gray;
        output[i * 3 + 1] = gray;
        output[i * 3 + 2] = gray;
    }
}

void grayscale_scalar(uint8_t *input, uint8_t *output){
    grayscale_scalar_range(input, output, 0, width * height);
}

void grayscale_simd_range(uint8_t *input, uint8_t *output, int start_pixel, int end_pixel){
    int i = start_pixel;

    __m256i coeff_r = _mm256_set1_epi16(77);
    __m256i coeff_g = _mm256_set1_epi16(150);
    __m256i coeff_b = _mm256_set1_epi16(29);

    for (; i + 15 < end_pixel; i += 16) {
        uint8_t r_buf[16];
        uint8_t g_buf[16];
        uint8_t b_buf[16];

        for (int j = 0; j < 16; j++) {
            r_buf[j] = input[(i + j) * 3];
            g_buf[j] = input[(i + j) * 3 + 1];
            b_buf[j] = input[(i + j) * 3 + 2];
        }

        __m128i r8 = _mm_loadu_si128((__m128i *)r_buf);
        __m128i g8 = _mm_loadu_si128((__m128i *)g_buf);
        __m128i b8 = _mm_loadu_si128((__m128i *)b_buf);

        __m256i r16 = _mm256_cvtepu8_epi16(r8);
        __m256i g16 = _mm256_cvtepu8_epi16(g8);
        __m256i b16 = _mm256_cvtepu8_epi16(b8);

        __m256i r_mul = _mm256_mullo_epi16(r16, coeff_r);
        __m256i g_mul = _mm256_mullo_epi16(g16, coeff_g);
        __m256i b_mul = _mm256_mullo_epi16(b16, coeff_b);

        __m256i sum = _mm256_add_epi16(_mm256_add_epi16(r_mul, g_mul), b_mul);
        __m256i gray16 = _mm256_srli_epi16(sum, 8);

        __m128i gray8 = _mm_packus_epi16(
            _mm256_castsi256_si128(gray16),
            _mm256_extracti128_si256(gray16, 1)
        );

        uint8_t gray_buf[16];
        _mm_storeu_si128((__m128i *)gray_buf, gray8);

        for (int j = 0; j < 16; j++) {
            uint8_t gray = gray_buf[j];
            output[(i + j) * 3] = gray;
            output[(i + j) * 3 + 1] = gray;
            output[(i + j) * 3 + 2] = gray;
        }
    }

    for (; i < end_pixel; i++) {
        int r = input[i * 3];
        int g = input[i * 3 + 1];
        int b = input[i * 3 + 2];

        uint8_t gray = (uint8_t)((77 * r + 150 * g + 29 * b) >> 8);

        output[i * 3] = gray;
        output[i * 3 + 1] = gray;
        output[i * 3 + 2] = gray;
    }
}

void grayscale_simd(uint8_t *input, uint8_t *output) {
    grayscale_simd_range(input, output, 0, width * height);
}

void *thread_scalar(void *arg){
    ThreadData *td = (ThreadData *)arg;
    grayscale_scalar_range(td->input, td->output, td->start_pixel, td->end_pixel);
    return NULL;
}

void grayscale_multithread(uint8_t *input, uint8_t *output) {
    pthread_t threads[NUM_THREADS];
    ThreadData td[NUM_THREADS];

    int pixels = width * height;
    int chunk = pixels / NUM_THREADS;

    for (int i = 0; i < NUM_THREADS; i++) {
        td[i].input = input;
        td[i].output = output;
        td[i].start_pixel = i * chunk;
        td[i].end_pixel = (i == NUM_THREADS - 1) ? pixels : (i + 1) * chunk;

        pthread_create(&threads[i], NULL, thread_scalar, &td[i]);
    }

    for (int i = 0; i < NUM_THREADS; i++)
        pthread_join(threads[i], NULL);
}

void *thread_simd(void *arg) {
    ThreadData *td = (ThreadData *)arg;
    grayscale_simd_range(td->input, td->output, td->start_pixel, td->end_pixel);
    return NULL;
}

void grayscale_multithread_simd(uint8_t *input, uint8_t *output) {
    pthread_t threads[NUM_THREADS];
    ThreadData td[NUM_THREADS];

    int pixels = width * height;
    int chunk = pixels / NUM_THREADS;

    for (int i = 0; i < NUM_THREADS; i++) {
        td[i].input = input;
        td[i].output = output;
        td[i].start_pixel = i * chunk;
        td[i].end_pixel = (i == NUM_THREADS - 1) ? pixels : (i + 1) * chunk;

        pthread_create(&threads[i], NULL, thread_simd, &td[i]);
    }

    for (int i = 0; i < NUM_THREADS; i++)
        pthread_join(threads[i], NULL);
}

int verify(uint8_t *a, uint8_t *b, int size){
    for (int i = 0; i < size; i++) {
        if (a[i] != b[i])
            return 0;
    }
    return 1;
}

int main() {
    uint8_t *input;
    read_ppm("input.ppm", &input);

    int size = width * height * 3;

    uint8_t *scalar_out = (uint8_t *)malloc(size);
    uint8_t *simd_out = (uint8_t *)malloc(size);
    uint8_t *mt_out = (uint8_t *)malloc(size);
    uint8_t *mt_simd_out = (uint8_t *)malloc(size);

    if (!scalar_out || !simd_out || !mt_out || !mt_simd_out)
        exit(1);

    double t1, t2;

    t1 = now_sec();
    grayscale_scalar(input, scalar_out);
    t2 = now_sec();
    double scalar_time = t2 - t1;

    t1 = now_sec();
    grayscale_simd(input, simd_out);
    t2 = now_sec();
    double simd_time = t2 - t1;

    t1 = now_sec();
    grayscale_multithread(input, mt_out);
    t2 = now_sec();
    double mt_time = t2 - t1;

    t1 = now_sec();
    grayscale_multithread_simd(input, mt_simd_out);
    t2 = now_sec();
    double mt_simd_time = t2 - t1;

    printf("image size: %d x %d\n", width, height);
    printf("threads used: %d\n\n", NUM_THREADS);

    printf("scalar time: %.6f sec\n", scalar_time);
    printf("SIMD time: %.6f sec\n", simd_time);
    printf("multithreading time: %.6f sec\n", mt_time);
    printf("multithreading + SIMD time: %.6f sec\n\n", mt_simd_time);

    if (verify(scalar_out, simd_out, size) &&
        verify(scalar_out, mt_out, size) &&
        verify(scalar_out, mt_simd_out, size)) {
        printf("Verification: PASSED\n");
    } else {
        printf("Verification: FAILED\n");
    }

    write_ppm("gray_output.ppm", mt_simd_out);
    printf("Output image: gray_output.ppm\n");

    free(input);
    free(scalar_out);
    free(simd_out);
    free(mt_out);
    free(mt_simd_out);

    return 0;
}