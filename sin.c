#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <math.h>
#include <time.h>
#include <unistd.h>

// NOTE: можно захардкодить таблицу для синуса

typedef struct {
    double x;
    int terms_count;
    int thread_id;
    int num_threads;
    double result;
} ThreadData;

void* calculate_sine_part(void* arg) {
    ThreadData* data = (ThreadData*) arg;
    data->result = 0.0;

    for (int n = data->thread_id; n < data->terms_count; n += data->num_threads) {
        double term = 1.0;
        
        for (int i = 1; i <= 2 * n + 1; i++) {
            term *= data->x / i;
        }
        if (n % 2 != 0) {
            term = -term;
        }
        
        data->result += term;
    }

    pthread_exit(NULL);
}

int main(int argc, char* argv[]) {
    if (argc < 4) {
        printf("Usage: %s <x (radians)> <number_of_series_terms> <number_of_threads>\n", argv[0]);
        return 1;
    }

    double x = atof(argv[1]);
    int terms_count = atoi(argv[2]);
    int num_threads = atoi(argv[3]);

    if (num_threads <= 0 || terms_count <= 0) {
        printf("Error: the number of threads and series terms must be greater than 0.\n");
        return 1;
    }

    pthread_t* threads = malloc(num_threads * sizeof(pthread_t));
    ThreadData* thread_data = malloc(num_threads * sizeof(ThreadData));

    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    for (int i = 0; i < num_threads; i++) {
        thread_data[i].x = x;
        thread_data[i].terms_count = terms_count;
        thread_data[i].thread_id = i;
        thread_data[i].num_threads = num_threads;
        
        if (pthread_create(&threads[i], NULL, calculate_sine_part, &thread_data[i]) != 0) {
            perror("Ошибка создания потока");
            return 1;
        }
    }

    double total_sine = 0.0;
    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
        total_sine += thread_data[i].result;
    }

    clock_gettime(CLOCK_MONOTONIC, &end);
    double elapsed_time = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;

    struct timespec std_start, std_end;
    clock_gettime(CLOCK_MONOTONIC, &std_start);
    double std_sine = sin(x);
    clock_gettime(CLOCK_MONOTONIC, &std_end);
    double std_elapsed_time = (std_end.tv_sec - std_start.tv_sec) + (std_end.tv_nsec - std_start.tv_nsec) / 1e9;

    printf("--- Results ---\n");
    printf("Parameters: x = %f, terms count = %d, threads = %d\n\n", x, terms_count, num_threads);
    printf("Multithreaded Taylor: %.15f (Time: %f sec)\n", total_sine, elapsed_time);
    printf("Standard sin(x):      %.15f (Time: %f sec)\n", std_sine, std_elapsed_time);
    printf("Absolute error:       %.15e\n", fabs(total_sine - std_sine));

    free(thread_data);

    return 0;
}