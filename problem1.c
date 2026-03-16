#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <pthread.h>
#include <immintrin.h>

#define DNA_SIZE_MB 256
#define NUM_THREADS 4

typedef struct {
    uint64_t A;
    uint64_t C;
    uint64_t G;
    uint64_t T;
} DnaCounts;

typedef struct {
    const char *data;
    size_t start;
    size_t end;
    DnaCounts local;
} ThreadData;

pthread_mutex_t global_mutex = PTHREAD_MUTEX_INITIALIZER;
DnaCounts global_counts = {0,0,0,0};

double now_sec() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC,&ts);
    return ts.tv_sec + ts.tv_nsec/1e9;
}

void add_counts(DnaCounts *dst, DnaCounts *src) {
    dst->A += src->A;
    dst->C += src->C;
    dst->G += src->G;
    dst->T += src->T;
}

void print_counts(DnaCounts *c) {
    printf("Counts (A C G T):\n");
    printf("%llu %llu %llu %llu\n",
           (unsigned long long)c->A,
           (unsigned long long)c->C,
           (unsigned long long)c->G,
           (unsigned long long)c->T);
}

char *generate_dna(size_t n) {
    char *buf = malloc(n);
    const char alphabet[4] = {'A','C','G','T'};

    for(size_t i=0;i<n;i++)
        buf[i] = alphabet[rand()%4];

    return buf;
}

DnaCounts count_scalar(const char *data,size_t n) {
    DnaCounts c = {0};

    for(size_t i=0;i<n;i++)
    {
        switch(data[i])
        {
            case 'A': c.A++; break;
            case 'C': c.C++; break;
            case 'G': c.G++; break;
            case 'T': c.T++; break;
        }
    }

    return c;
}

void *thread_scalar(void *arg) {
    ThreadData *td = (ThreadData*)arg;

    td->local = count_scalar(td->data + td->start,
                             td->end - td->start);

    pthread_mutex_lock(&global_mutex);
    add_counts(&global_counts,&td->local);
    pthread_mutex_unlock(&global_mutex);

    return NULL;
}

DnaCounts count_multithreaded(const char *data,size_t n) {
    pthread_t threads[NUM_THREADS];
    ThreadData td[NUM_THREADS];

    global_counts = (DnaCounts){0};

    size_t chunk = n / NUM_THREADS;

    for(int i=0;i<NUM_THREADS;i++)
    {
        td[i].data = data;
        td[i].start = i * chunk;
        td[i].end = (i==NUM_THREADS-1) ? n : (i+1)*chunk;

        pthread_create(&threads[i],NULL,thread_scalar,&td[i]);
    }

    for(int i=0;i<NUM_THREADS;i++)
        pthread_join(threads[i],NULL);

    return global_counts;
}

DnaCounts count_simd(const char *data,size_t n){
    DnaCounts c = {0};
    size_t i = 0;

    __m256i vA = _mm256_set1_epi8('A');
    __m256i vC = _mm256_set1_epi8('C');
    __m256i vG = _mm256_set1_epi8('G');
    __m256i vT = _mm256_set1_epi8('T');

    for(; i + 32 <= n; i += 32)
    {
        __m256i v = _mm256_loadu_si256((__m256i*)(data+i));

        uint32_t mA = _mm256_movemask_epi8(_mm256_cmpeq_epi8(v,vA));
        uint32_t mC = _mm256_movemask_epi8(_mm256_cmpeq_epi8(v,vC));
        uint32_t mG = _mm256_movemask_epi8(_mm256_cmpeq_epi8(v,vG));
        uint32_t mT = _mm256_movemask_epi8(_mm256_cmpeq_epi8(v,vT));

        c.A += __builtin_popcount(mA);
        c.C += __builtin_popcount(mC);
        c.G += __builtin_popcount(mG);
        c.T += __builtin_popcount(mT);
    }

    for(; i<n; i++)
    {
        switch(data[i])
        {
            case 'A': c.A++; break;
            case 'C': c.C++; break;
            case 'G': c.G++; break;
            case 'T': c.T++; break;
        }
    }

    return c;
}

void *thread_simd(void *arg) {
    ThreadData *td = (ThreadData*)arg;

    td->local = count_simd(td->data + td->start,
                           td->end - td->start);

    pthread_mutex_lock(&global_mutex);
    add_counts(&global_counts,&td->local);
    pthread_mutex_unlock(&global_mutex);

    return NULL;
}

DnaCounts count_simd_multithreaded(const char *data,size_t n){
    pthread_t threads[NUM_THREADS];
    ThreadData td[NUM_THREADS];

    global_counts = (DnaCounts){0};

    size_t chunk = n / NUM_THREADS;

    for(int i=0;i<NUM_THREADS;i++)
    {
        td[i].data = data;
        td[i].start = i * chunk;
        td[i].end = (i==NUM_THREADS-1) ? n : (i+1)*chunk;

        pthread_create(&threads[i],NULL,thread_simd,&td[i]);
    }

    for(int i=0;i<NUM_THREADS;i++)
        pthread_join(threads[i],NULL);

    return global_counts;
}

int main() {
    size_t n = DNA_SIZE_MB * 1024ULL * 1024ULL;

    srand(time(NULL));

    char *dna = generate_dna(n);

    double t1,t2;

    t1 = now_sec();
    DnaCounts scalar = count_scalar(dna,n);
    t2 = now_sec();
    double scalar_time = t2 - t1;

    t1 = now_sec();
    DnaCounts mt = count_multithreaded(dna,n);
    t2 = now_sec();
    double mt_time = t2 - t1;

    t1 = now_sec();
    DnaCounts simd = count_simd(dna,n);
    t2 = now_sec();
    double simd_time = t2 - t1;

    t1 = now_sec();
    DnaCounts simd_mt = count_simd_multithreaded(dna,n);
    t2 = now_sec();
    double simd_mt_time = t2 - t1;

    printf("DNA size: %d MB\n",DNA_SIZE_MB);
    printf("Threads used: %d\n\n",NUM_THREADS);

    print_counts(&scalar);
    printf("\n");

    printf("Scalar time: %.3f sec\n",scalar_time);
    printf("Multithreading time: %.3f sec\n",mt_time);
    printf("SIMD time: %.3f sec\n",simd_time);
    printf("SIMD + Multithreading time: %.3f sec\n",simd_mt_time);

    free(dna);

    return 0;
}