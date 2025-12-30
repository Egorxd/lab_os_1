#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <stdatomic.h>

#define READERS_COUNT 10
#define ARRAY_SIZE 64
#define WRITE_ITERATIONS 20

char shared_array[ARRAY_SIZE];
pthread_rwlock_t rwlock;

_Atomic int counter = 0;
_Atomic int keep_running = 1;  

void *writer_thread(void *arg)
{
    (void)arg;

    for (int i = 0; i < WRITE_ITERATIONS && atomic_load(&keep_running); i++) {
        pthread_rwlock_wrlock(&rwlock);

        counter++;
        snprintf(shared_array, ARRAY_SIZE, "Запись № %d", counter);
        
        pthread_rwlock_unlock(&rwlock);

        usleep(500000);  
    }

    atomic_store(&keep_running, 0);  
    
    printf("Пишущий поток завершил работу после %d итераций\n", WRITE_ITERATIONS);
    return NULL;
}

void *reader_thread(void *arg)
{
    long id = (long)arg;
    char local_buffer[ARRAY_SIZE];  

    while (atomic_load(&keep_running)) {
        pthread_rwlock_rdlock(&rwlock);

        strncpy(local_buffer, shared_array, ARRAY_SIZE - 1);
        local_buffer[ARRAY_SIZE - 1] = '\0';
        
        pthread_rwlock_unlock(&rwlock);

        printf("Читающий поток TID=%ld | Данные: \"%s\"\n", id, local_buffer);

        usleep(200000 + (id * 10000));  
    }

    printf("Читающий поток TID=%ld завершил работу\n", id);
    return NULL;
}

int main(void)
{
    pthread_t readers[READERS_COUNT];
    pthread_t writer;

    pthread_rwlock_init(&rwlock, NULL);
    memset(shared_array, 0, sizeof(shared_array));
    snprintf(shared_array, ARRAY_SIZE, "Начальное состояние");

    for (long i = 0; i < READERS_COUNT; i++) {
        if (pthread_create(&readers[i], NULL, reader_thread, (void *)i) != 0) {
            perror("pthread_create reader");
            exit(EXIT_FAILURE);
        }
    }

    if (pthread_create(&writer, NULL, writer_thread, NULL) != 0) {
        perror("pthread_create writer");
        exit(EXIT_FAILURE);
    }

    pthread_join(writer, NULL);

    usleep(100000);

    for (int i = 0; i < READERS_COUNT; i++) {
        pthread_join(readers[i], NULL);
    }

    pthread_rwlock_destroy(&rwlock);
    
    printf("\nВсе потоки завершены. Программа завершена.\n");
    return 0;
}