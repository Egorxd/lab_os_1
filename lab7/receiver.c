#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/shm.h>
#include <time.h>

#define SHM_KEY 0x1234

struct shm_data {
    pid_t sender_pid;
    char message[128];
};

int main() {
    int shmid = shmget(SHM_KEY, sizeof(struct shm_data), 0666);
    if (shmid == -1) {
        perror("shmget (запусти sender)");
        exit(1);
    }

    struct shm_data *data = shmat(shmid, NULL, 0);
    if (data == (void *)-1) {
        perror("shmat");
        exit(1);
    }

    printf("Receiver PID=%d запущен\n", getpid());

    while (1) {
        time_t now = time(NULL);
        struct tm *t = localtime(&now);
        char timestr[32];
        strftime(timestr, sizeof(timestr), "%H:%M:%S", t);

        printf("[Receiver PID=%d TIME=%s] %s\n",
               getpid(), timestr, data->message);

        sleep(1);
    }
}
