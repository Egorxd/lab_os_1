#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <utime.h>
#include <errno.h>

#define FNAME_LEN 256
#define BLOCK 4096

struct file_header {
    char name[FNAME_LEN];
    off_t size;
    mode_t mode;
    uid_t uid;
    gid_t gid;
    struct timespec atime;
    struct timespec mtime;
};

static void print_help(void) {
    puts("Использование:");
    puts("  ./archiver архив -i файл     Добавить файл в архив");
    puts("  ./archiver архив -e файл     Извлечь файл и удалить из архива");
    puts("  ./archiver архив -s          Показать содержимое архива");
    puts("  ./archiver -h                Справка");
}

static int save_header(int fd, const struct file_header *hdr) {
    ssize_t written = write(fd, hdr, sizeof(*hdr));
    if (written != sizeof(*hdr)) {
        perror("Ошибка при записи заголовка");
        return -1;
    }
    return 0;
}

static int load_header(int fd, struct file_header *hdr) {
    ssize_t readed = read(fd, hdr, sizeof(*hdr));
    if (readed == 0) return 0;
    if (readed != sizeof(*hdr)) {
        perror("Ошибка чтения заголовка");
        return -1;
    }
    return 1;
}

static int add_to_archive(const char *arch, const char *fname) {
    struct stat st;
    if (stat(fname, &st) < 0) {
        perror("stat");
        return 1;
    }

    int a = open(arch, O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (a < 0) {
        perror("open archive");
        return 1;
    }

    struct file_header hdr = {0};
    strncpy(hdr.name, fname, FNAME_LEN - 1);
    hdr.size = st.st_size;
    hdr.mode = st.st_mode;
    hdr.uid  = st.st_uid;
    hdr.gid  = st.st_gid;
    hdr.atime = st.st_atim;
    hdr.mtime = st.st_mtim;

    if (save_header(a, &hdr) < 0) {
        close(a);
        return 1;
    }

    int f = open(fname, O_RDONLY);
    if (f < 0) {
        perror("open file");
        close(a);
        return 1;
    }

    char chunk[BLOCK];
    ssize_t r;
    while ((r = read(f, chunk, sizeof(chunk))) > 0) {
        if (write(a, chunk, r) != r) {
            perror("write archive");
            close(f);
            close(a);
            return 1;
        }
    }
    if (r < 0) perror("read file");

    close(f);
    close(a);
    printf("[+] Файл '%s' добавлен в '%s'\n", fname, arch);
    return 0;
}

static int extract_from_archive(const char *arch, const char *target) {
    int src = open(arch, O_RDWR);
    if (src < 0) {
        perror("open archive");
        return 1;
    }

    struct stat st;
    if (fstat(src, &st) < 0) {
        perror("fstat");
        close(src);
        return 1;
    }

    if (st.st_size == 0) {
        puts("Архив пуст");
        close(src);
        return 1;
    }

    char tmp[] = "arch_tmpXXXXXX";
    int tmp_fd = mkstemp(tmp);
    if (tmp_fd < 0) {
        perror("mkstemp");
        close(src);
        return 1;
    }

    off_t offset = 0;
    int found = 0;
    struct file_header hdr;

    while (offset < st.st_size) {
        ssize_t got = pread(src, &hdr, sizeof(hdr), offset);
        if (got <= 0) break;

        off_t data_pos = offset + sizeof(hdr);
        off_t data_end = data_pos + hdr.size;

        if (strcmp(hdr.name, target) == 0) {
            int out = open(target, O_CREAT | O_TRUNC | O_WRONLY, hdr.mode);
            if (out < 0) {
                perror("create output");
                goto fail;
            }

            off_t remaining = hdr.size;
            char buf[BLOCK];
            while (remaining > 0) {
                ssize_t n = pread(src, buf, remaining > BLOCK ? BLOCK : remaining, data_pos);
                if (n <= 0) break;
                write(out, buf, n);
                remaining -= n;
                data_pos += n;
            }
            close(out);

            struct timespec t[2] = {hdr.atime, hdr.mtime};
            utimensat(AT_FDCWD, target, t, 0);
            chmod(target, hdr.mode);

            found = 1;
            printf("[>] Извлечён файл '%s'\n", target);
        } else {
            if (save_header(tmp_fd, &hdr) < 0) goto fail;

            char buf[BLOCK];
            off_t left = hdr.size;
            while (left > 0) {
                ssize_t n = pread(src, buf, left > BLOCK ? BLOCK : left, data_pos);
                if (n <= 0) break;
                write(tmp_fd, buf, n);
                left -= n;
                data_pos += n;
            }
        }
        offset = data_end;
    }

    close(src);
    close(tmp_fd);

    if (!found) {
        printf("Файл '%s' не найден\n", target);
        unlink(tmp);
        return 1;
    }

    if (rename(tmp, arch) < 0) {
        perror("rename");
        unlink(tmp);
        return 1;
    }

    printf("[*] Архив '%s' обновлён\n", arch);
    return 0;

fail:
    close(src);
    close(tmp_fd);
    unlink(tmp);
    return 1;
}

static int show_archive(const char *arch) {
    int fd = open(arch, O_RDONLY);
    if (fd < 0) {
        perror("open");
        return 1;
    }

    struct stat st;
    if (fstat(fd, &st) < 0) {
        perror("fstat");
        close(fd);
        return 1;
    }

    if (st.st_size == 0) {
        printf("Архив '%s' пуст\n", arch);
        close(fd);
        return 0;
    }

    printf("Содержимое архива '%s':\n", arch);
    printf("%-25s %-8s %-6s\n", "Имя", "Размер", "Права");

    off_t pos = 0;
    struct file_header hdr;
    while (pos < st.st_size) {
        if (pread(fd, &hdr, sizeof(hdr), pos) != sizeof(hdr))
            break;
        printf("%-25s %-8ld %04o\n", hdr.name, (long)hdr.size, hdr.mode & 0777);
        pos += sizeof(hdr) + hdr.size;
    }

    close(fd);
    return 0;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        print_help();
        return 1;
    }

    const char *arch = NULL;
    const char *file = NULL;
    int mode = 0;

    if (!strcmp(argv[1], "-h") || !strcmp(argv[1], "--help")) {
        print_help();
        return 0;
    }

    if (argc >= 3) {
        arch = argv[1];
        if (!strcmp(argv[2], "-i") && argc >= 4) { mode = 1; file = argv[3]; }
        else if (!strcmp(argv[2], "-e") && argc >= 4) { mode = 2; file = argv[3]; }
        else if (!strcmp(argv[2], "-s")) { mode = 3; }
    }

    switch (mode) {
        case 1: return add_to_archive(arch, file);
        case 2: return extract_from_archive(arch, file);
        case 3: return show_archive(arch);
        default:
            print_help();
            return 1;
    }
}
