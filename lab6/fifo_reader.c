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

    printf("Reader: ждем 10 секунд перед чтением...\n");
    sleep(10);

    int fd = open(FIFO_PATH, O_RDONLY);
    char buffer[256];
    read(fd, buffer, sizeof(buffer));
    close(fd);

    time_t now;
    struct tm *timeinfo;
    time(&now);
    timeinfo = localtime(&now);

    printf("Reader: текущее время %02d:%02d:%02d\n",
           timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
    printf("Получено из FIFO: %s\n", buffer);

    unlink(FIFO_PATH); // удалить FIFO после использования
    return 0;
}
