#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/shm.h>
#include <time.h>
#include <string.h>

#define SHM_KEY 0x1234

struct shm_data {
    pid_t sender_pid;
    char message[128];
};

int main() {
    int shmid = shmget(SHM_KEY, sizeof(struct shm_data), 0666 | IPC_CREAT);
    if (shmid == -1) {
        perror("shmget");
        exit(1);
    }

    struct shm_data *data = shmat(shmid, NULL, 0);
    if (data == (void *)-1) {
        perror("shmat");
        exit(1);
    }

    if (data->sender_pid != 0) {
        printf("Отправитель уже запущен (PID=%d)\n", data->sender_pid);
        exit(0);
    }

    data->sender_pid = getpid();
    printf("Отправитель запущен, PID=%d\n", getpid());

    while (1) {
        time_t now = time(NULL);
        struct tm *t = localtime(&now);
        char timestr[32];
        strftime(timestr, sizeof(timestr), "%H:%M:%S", t);

        snprintf(data->message, sizeof(data->message),
                 "Sender PID=%d TIME=%s", getpid(), timestr);

        sleep(1);
    }
}
