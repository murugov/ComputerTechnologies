#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <math.h>

const double angle = 1.23f;
const size_t iterations = 100000;

// Function for calculating the square of the sine
void * sin_thread (void * arg) {
    
    double *result = (double *) arg;

    double s = sin(angle);
    *result = s * s; // sin^2(x)
    
    return (void *) result;
}

// Function for calculating the square of the cosine
void * cos_thread (void * arg) {
    double *result = (double *) arg;

    double c = cos(angle);
    *result = c * c; // cos^2(x)
    
    return (void *) result;
}

// Function for calculating elapsed time in seconds
double get_time_diff(struct timespec start, struct timespec end) {
    return (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1000000000.0;
}

int main ()
{
    struct timespec start, end;
    volatile double total_sum = 0.0f;

    // === SINGLE-THREADED MODE ===

    clock_gettime(CLOCK_MONOTONIC, &start);
    
    for (size_t i = 0; i < iterations; ++i) {
        double s = sin(angle);
        double c = cos(angle);
        total_sum += (s * s) + (c * c);
    }
    
    clock_gettime(CLOCK_MONOTONIC, &end);
    double time_single = get_time_diff(start, end);
    printf("Single-threaded mode:   %.6f s\n", time_single);

    // === MULTI-THREADED MODE ===
    
    total_sum = 0;
    clock_gettime(CLOCK_MONOTONIC, &start);

    double *result = (double*)malloc(sizeof(double));
    if (result == NULL) return 1;
        
    for (size_t i = 0; i < iterations; ++i) {
        pthread_t t1, t2;
        double *sin_res = NULL, *cos_res = NULL;

        pthread_create(&t1, NULL, sin_thread, (void *)result);
        pthread_create(&t2, NULL, cos_thread, (void *)result);

        pthread_join(t1, (void**)&sin_res);
        pthread_join(t2, (void**)&cos_res);

        if (sin_res && cos_res) {
            total_sum += *sin_res + *cos_res;
        }
    }

    free (result);
    
    clock_gettime(CLOCK_MONOTONIC, &end);
    double time_multi = get_time_diff(start, end);
    printf("Multi-threaded mode:   %.6f s\n", time_multi);

    // === RESULT ===
    
    printf("\nDifference: multi-threaded mode is %.1f times slower!\n", time_multi / time_single);
    
    return 0;
}