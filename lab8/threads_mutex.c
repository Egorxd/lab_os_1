#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>

#define READERS_COUNT 10
#define BUFFER_SIZE 64

char shared_buffer[BUFFER_SIZE];
int write_counter = 0;

pthread_mutex_t mutex;

void *writer_thread(void *arg)
{
    while (1)
    {
        pthread_mutex_lock(&mutex);

        write_counter++;
        snprintf(shared_buffer, BUFFER_SIZE,
                 "Запись №%d", write_counter);

        pthread_mutex_unlock(&mutex);

        sleep(1); 
    }
    return NULL;
}

void *reader_thread(void *arg)
{
    pthread_t tid = pthread_self();

    while (1)
    {
        pthread_mutex_lock(&mutex);

        printf("Reader TID=%lu | buffer: \"%s\"\n",
               (unsigned long)tid,
               shared_buffer);

        pthread_mutex_unlock(&mutex);

        usleep(500000); // 0.5 сек
    }
    return NULL;
}

int main(void)
{
    pthread_t writer;
    pthread_t readers[READERS_COUNT];

    if (pthread_mutex_init(&mutex, NULL) != 0)
    {
        perror("pthread_mutex_init");
        return EXIT_FAILURE;
    }

    strcpy(shared_buffer, "Пусто");

    if (pthread_create(&writer, NULL, writer_thread, NULL) != 0)
    {
        perror("pthread_create writer");
        return EXIT_FAILURE;
    }

    for (int i = 0; i < READERS_COUNT; i++)
    {
        if (pthread_create(&readers[i], NULL, reader_thread, NULL) != 0)
        {
            perror("pthread_create reader");
            return EXIT_FAILURE;
        }
    }

    pthread_join(writer, NULL);
    for (int i = 0; i < READERS_COUNT; i++)
        pthread_join(readers[i], NULL);

    pthread_mutex_destroy(&mutex);
    return EXIT_SUCCESS;
}
