#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <time.h>

#define SHM_KEY 0x1234
#define SEM_KEY 0x5678
#define SHM_SIZE 128

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

int main()
{
    int shmid = shmget(SHM_KEY, SHM_SIZE, 0666);
    char* shm = (char*)shmat(shmid, NULL, 0);

    int semid = semget(SEM_KEY, 1, 0666);

    while (1) {
        time_t now = time(NULL);

        sem_lock(semid);
        printf("Получатель PID=%d Время=%sПринято: %s\n",
               getpid(), ctime(&now), shm);
        sem_unlock(semid);

        sleep(3);
    }
}
