// receiver.c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <semaphore.h>
#include <time.h>
#include <string.h>

#define SHM_NAME "/mysharedmem"
#define SEM_NAME "/mysem"

typedef struct {
    char buffer[128];
    int sender_running;
} shared_data;

int main() {
    int shm_fd = shm_open(SHM_NAME, O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("shm_open (возможно отправитель не запущен)");
        exit(1);
    }

    shared_data *data = mmap(NULL, sizeof(shared_data),
                             PROT_READ | PROT_WRITE, MAP_SHARED,
                             shm_fd, 0);
    if (data == MAP_FAILED) {
        perror("mmap");
        exit(1);
    }

    sem_t *sem = sem_open(SEM_NAME, 0);
    if (sem == SEM_FAILED) {
        perror("sem_open");
        exit(1);
    }

    printf("Получатель PID=%d запущен\n", getpid());

    while (1) {
        sleep(1);

        time_t now = time(NULL);
        struct tm *t = localtime(&now);
        char localtime_str[32];
        strftime(localtime_str, sizeof(localtime_str), "%H:%M:%S", t);

        sem_wait(sem);
        char msg[128];
        strncpy(msg, data->buffer, sizeof(msg));
        sem_post(sem);

        printf("[Receiver PID=%d TIME=%s] Получено: %s\n",
               getpid(), localtime_str, msg);
    }

    return 0;
}
