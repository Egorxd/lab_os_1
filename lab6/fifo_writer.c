#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

#define FIFO_PATH "/tmp/myfifo"

int main() {
    mkfifo(FIFO_PATH, 0666);

    time_t now;
    struct tm *timeinfo;
    time(&now);
    timeinfo = localtime(&now);

    char msg[256];
    snprintf(msg, sizeof(msg),
             "Сообщение от writer: время %02d:%02d:%02d, PID=%d",
             timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec, getpid());

    int fd = open(FIFO_PATH, O_WRONLY);
    write(fd, msg, strlen(msg) + 1);
    close(fd);

    printf("Writer: сообщение отправлено. Завершаю работу.\n");
    return 0;
}
