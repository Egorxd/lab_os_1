#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <time.h>

#define SHM_KEY 0x1234
#define SEM_KEY 0x5678
#define SHM_SIZE 128

int shmid, semid;
char *shm;

void sem_lock(int semid)
{
    struct sembuf sb = {0, -1, 0};
    semop(semid, &sb, 1);
}

void sem_unlock(int semid)
{
    struct sembuf sb = {0, 1, 0};
    semop(semid, &sb, 1);
}

void cleanup(int sig)
{
    shmdt(shm);
    printf("\nSender: отсоединился от памяти\n");
    exit(0);
}

int main()
{
    signal(SIGINT, cleanup);

    shmid = shmget(SHM_KEY, SHM_SIZE, IPC_CREAT | 0666);
    shm = (char *)shmat(shmid, NULL, 0);

    semid = semget(SEM_KEY, 1, IPC_CREAT | 0666);
    semctl(semid, 0, SETVAL, 1);

    while (1) {
        time_t now = time(NULL);

        sem_lock(semid);
        snprintf(shm, SHM_SIZE,
                 "Отправитель PID=%d Время=%s",
                 getpid(), ctime(&now));
        sem_unlock(semid);

        sleep(3);
    }
}
