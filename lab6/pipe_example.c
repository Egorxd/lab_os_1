#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <sys/types.h>

int main() {
    int fd[2];
    pid_t pid;
    char buffer[256];
    time_t now;
    struct tm *timeinfo;

    if (pipe(fd) == -1) {
        perror("pipe");
        exit(1);
    }

    pid = fork();
    if (pid < 0) {
        perror("fork");
        exit(1);
    }

    if (pid > 0) {  // Родитель
        close(fd[0]); // Закрываем чтение

        time(&now);
        timeinfo = localtime(&now);
        char msg[256];
        snprintf(msg, sizeof(msg),
                 "Сообщение от родителя: текущее время %02d:%02d:%02d, PID=%d",
                 timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec, getpid());

        write(fd[1], msg, strlen(msg) + 1);
        close(fd[1]);

        printf("Родитель: сообщение отправлено. Ждем 5 секунд...\n");
        sleep(5);
        printf("Родитель: завершает работу.\n");
    } else {  // Дочерний
        close(fd[1]); // Закрываем запись
        sleep(5);     // Задержка, чтобы время отличалось

        read(fd[0], buffer, sizeof(buffer));
        close(fd[0]);

        time(&now);
        timeinfo = localtime(&now);
        printf("Дочерний процесс (PID=%d): текущее время %02d:%02d:%02d\n",
               getpid(), timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
        printf("Получено через pipe: %s\n", buffer);
    }

    return 0;
}
