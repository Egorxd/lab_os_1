// sender.c
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
#define FLAG_NAME "/myflag"

typedef struct {
    char buffer[128];
    int sender_running;   // флаг единственного экземпляра
} shared_data;

int main() {
    int shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("shm_open");
        exit(1);
    }

    if (ftruncate(shm_fd, sizeof(shared_data)) == -1) {
        perror("ftruncate");
        exit(1);
    }

    shared_data *data = mmap(NULL, sizeof(shared_data),
                             PROT_READ | PROT_WRITE, MAP_SHARED,
                             shm_fd, 0);
    if (data == MAP_FAILED) {
        perror("mmap");
        exit(1);
    }

    sem_t *sem = sem_open(SEM_NAME, O_CREAT, 0666, 1);
    if (sem == SEM_FAILED) {
        perror("sem_open");
        exit(1);
    }

    // проверка единственности отправителя 
    sem_wait(sem);
    if (data->sender_running) {
        sem_post(sem);
        printf("Отправитель уже запущен. Второй экземпляр завершает работу.\n");
        exit(0);
    }
    data->sender_running = 1;
    sem_post(sem);

    printf("Отправитель запущен, PID=%d\n", getpid());

    while (1) {
        sleep(1);

        time_t now = time(NULL);
        struct tm *t = localtime(&now);
        char timestr[32];
        strftime(timestr, sizeof(timestr), "%H:%M:%S", t);

        char msg[128];
        snprintf(msg, sizeof(msg), "PID: %d TIME: %s", getpid(), timestr);

        sem_wait(sem);
        strncpy(data->buffer, msg, sizeof(data->buffer));
        sem_post(sem);
    }

    return 0;
}
