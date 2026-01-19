#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>
#include <string.h>

#define BUF_SIZE 64

char shared_buf[BUF_SIZE];
sem_t sem;
int counter = 0;

void* writer_thread(void* arg)
{
    while (1) {
        sem_wait(&sem);

        snprintf(shared_buf, BUF_SIZE,
                 "Запись номер: %d", counter++);

        sem_post(&sem);
        sleep(1);
    }
    return NULL;
}

void* reader_thread(void* arg)
{
    pthread_t tid = pthread_self();

    while (1) {
        printf("Reader TID: %lu | Общий массив: %s\n",
               (unsigned long)tid, shared_buf);
    }
    return NULL;
}

int main()
{
    pthread_t writer, reader;

    sem_init(&sem, 0, 1);
    memset(shared_buf, 0, BUF_SIZE);

    pthread_create(&writer, NULL, writer_thread, NULL);
    pthread_create(&reader, NULL, reader_thread, NULL);

    pthread_join(writer, NULL);
    pthread_join(reader, NULL);

    sem_destroy(&sem);
    return 0;
}
