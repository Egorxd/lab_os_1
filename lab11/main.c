#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <stdatomic.h>
#include <errno.h>
#include <time.h>

#define READERS_COUNT 10
#define ARRAY_SIZE 64
#define WRITE_ITERATIONS 20
#define READER_TIMEOUT_NS 100000000  

char shared_array[ARRAY_SIZE];

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t data_updated = PTHREAD_COND_INITIALIZER;

_Atomic int keep_running = 1;
_Atomic int data_version = 0;  

void *writer_thread(void *arg)
{
    (void)arg;
    int iterations_done = 0;

    while (iterations_done < WRITE_ITERATIONS && atomic_load(&keep_running)) {
        pthread_mutex_lock(&mutex);

        iterations_done++;
        snprintf(shared_array, ARRAY_SIZE, "Запись № %d", iterations_done);

        atomic_fetch_add(&data_version, 1);
        
        printf("Писатель: обновил данные до версии %d: \"%s\"\n", 
               atomic_load(&data_version), shared_array);

        pthread_cond_broadcast(&data_updated);
        
        pthread_mutex_unlock(&mutex);

        usleep(500000);
    }

    atomic_store(&keep_running, 0);

    pthread_mutex_lock(&mutex);
    pthread_cond_broadcast(&data_updated);
    pthread_mutex_unlock(&mutex);
    
    printf("Пишущий поток завершил работу после %d итераций\n", iterations_done);
    return NULL;
}

void *reader_thread(void *arg)
{
    long thread_id = (long)arg;
    char local_buffer[ARRAY_SIZE];
    int last_seen_version = 0;
    struct timespec timeout;
    
    pthread_t self = pthread_self();
    
    pthread_mutex_lock(&mutex);
    while (atomic_load(&keep_running)) {

        while (last_seen_version == atomic_load(&data_version) && 
               atomic_load(&keep_running)) {

            clock_gettime(CLOCK_REALTIME, &timeout);
            timeout.tv_nsec += READER_TIMEOUT_NS;
            if (timeout.tv_nsec >= 1000000000) {
                timeout.tv_sec++;
                timeout.tv_nsec -= 1000000000;
            }
            
            int rc = pthread_cond_timedwait(&data_updated, &mutex, &timeout);
            if (rc == ETIMEDOUT) {
                if (!atomic_load(&keep_running)) {
                    break;
                }
                continue;
            }
        }

        if (!atomic_load(&keep_running)) {
            break;
        }

        int current_version = atomic_load(&data_version);
        last_seen_version = current_version;
        strncpy(local_buffer, shared_array, ARRAY_SIZE - 1);
        local_buffer[ARRAY_SIZE - 1] = '\0';

        pthread_mutex_unlock(&mutex);

        printf("Читатель [TID: %lu, ID: %ld] | Версия: %d | Данные: \"%s\"\n",
               (unsigned long)self, thread_id, current_version, local_buffer);

        usleep(100000 + (thread_id % 7) * 30000);  

        pthread_mutex_lock(&mutex);
    }
    
    pthread_mutex_unlock(&mutex);
    
    printf("Читающий поток ID=%ld завершил работу\n", thread_id);
    return NULL;
}

int main(void)
{
    pthread_t readers[READERS_COUNT];
    pthread_t writer;
    
    printf("=== Программа с условными переменными ===\n");
    printf("Читателей: %d, Итераций записи: %d\n\n", READERS_COUNT, WRITE_ITERATIONS);

    memset(shared_array, 0, sizeof(shared_array));
    snprintf(shared_array, ARRAY_SIZE, "Начальное состояние");
    atomic_store(&data_version, 0);

    for (long i = 0; i < READERS_COUNT; i++) {
        if (pthread_create(&readers[i], NULL, reader_thread, (void *)i) != 0) {
            perror("Ошибка создания читающего потока");
            exit(EXIT_FAILURE);
        }
    }

    usleep(100000);

    if (pthread_create(&writer, NULL, writer_thread, NULL) != 0) {
        perror("Ошибка создания пишущего потока");
        exit(EXIT_FAILURE);
    }

    pthread_join(writer, NULL);
    
    printf("\nПишущий поток завершён. Ожидание завершения читателей...\n");

    for (int i = 0; i < READERS_COUNT; i++) {
        pthread_join(readers[i], NULL);
    }

    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&data_updated);

    printf("\nВсе потоки завершены. Программа завершена.\n");
    return 0;
}