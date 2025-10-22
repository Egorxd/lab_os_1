#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <ctype.h>

int is_numeric_mode(const char *mode) {
    for (int i = 0; mode[i] != '\0'; i++) {
        if (mode[i] < '0' || mode[i] > '7') {
            return 0;
        }
    }
    return 1;
}

mode_t parse_numeric_mode(const char *mode) {
    char *endptr;
    long octal_value = strtol(mode, &endptr, 8);
    
    // Валидация
    if (*endptr != '\0' || octal_value < 0 || octal_value > 07777) {
        fprintf(stderr, "Ошибка: недопустимый режим '%s'\n", mode);
        return (mode_t)-1;
    }
    
    return (mode_t)octal_value;
}

int parse_single_symbolic_mode(const char *mode, mode_t current_mode, mode_t *new_mode) {
    const char *ptr = mode;
    mode_t who_mask = 0;
    char operation = 0;
    mode_t permissions = 0;
    
    int who_specified = 0;
    while (*ptr && (*ptr == 'u' || *ptr == 'g' || *ptr == 'o' || *ptr == 'a')) {
        who_specified = 1;
        if (*ptr == 'u') {
            who_mask |= S_IRWXU;
        } else if (*ptr == 'g') {
            who_mask |= S_IRWXG;
        } else if (*ptr == 'o') {
            who_mask |= S_IRWXO;
        } else if (*ptr == 'a') {
            who_mask |= S_IRWXU | S_IRWXG | S_IRWXO;
        }
        ptr++;
    }
    
    if (!who_specified) {
        who_mask = S_IRWXU | S_IRWXG | S_IRWXO;
    }
    
    if (*ptr == '+' || *ptr == '-' || *ptr == '=') {
        operation = *ptr;
        ptr++;
    } else {
        fprintf(stderr, "Ошибка: неверный формат режима '%s'\n", mode);
        return -1;
    }
    
    while (*ptr && (*ptr == 'r' || *ptr == 'w' || *ptr == 'x')) {
        if (*ptr == 'r') {
            permissions |= S_IRUSR | S_IRGRP | S_IROTH; 
        } else if (*ptr == 'w') {
            permissions |= S_IWUSR | S_IWGRP | S_IWOTH; 
        } else if (*ptr == 'x') {
            permissions |= S_IXUSR | S_IXGRP | S_IXOTH; 
        }
        ptr++;
    }
    
    if (*ptr != '\0') {
        fprintf(stderr, "Ошибка: неверный формат режима '%s'\n", mode);
        return -1;
    }
    
    if (operation == '+') {
        *new_mode = current_mode | (permissions & who_mask);
    } else if (operation == '-') {
        *new_mode = current_mode & ~(permissions & who_mask);
    } else if (operation == '=') {
        *new_mode = (current_mode & ~who_mask) | (permissions & who_mask);
    }
    
    return 0;
}

int parse_symbolic_mode(const char *mode, mode_t current_mode, mode_t *new_mode) {
    char mode_copy[256];
    strncpy(mode_copy, mode, sizeof(mode_copy) - 1);
    mode_copy[sizeof(mode_copy) - 1] = '\0';
    
    *new_mode = current_mode;
    
    char *token = strtok(mode_copy, ",");
    while (token != NULL) {
        if (parse_single_symbolic_mode(token, *new_mode, new_mode) == -1) {
            return -1;
        }
        token = strtok(NULL, ",");
    }
    
    return 0;
}

int change_mode(const char *filename, const char *mode_str) {
    struct stat st;
    mode_t new_mode;
    
    if (stat(filename, &st) == -1) {
        perror("stat");
        return -1;
    }
    
    if (is_numeric_mode(mode_str)) {
        new_mode = parse_numeric_mode(mode_str);
        if (new_mode == (mode_t)-1) {  
            return -1;
        }
    } else {
        if (parse_symbolic_mode(mode_str, st.st_mode, &new_mode) == -1) {
            return -1;
        }
    }
    
    if (chmod(filename, new_mode) == -1) {
        perror("chmod");
        return -1;
    }
    
    printf("Права доступа к файлу '%s' успешно изменены\n", filename);
    return 0;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Использование: %s <режим> <файл>\n", argv[0]);
        fprintf(stderr, "Примеры:\n");
        fprintf(stderr, "  %s +x file.txt\n", argv[0]);
        fprintf(stderr, "  %s u-r file.txt\n", argv[0]);
        fprintf(stderr, "  %s g+rw file.txt\n", argv[0]);
        fprintf(stderr, "  %s ug+rw file.txt\n", argv[0]);
        fprintf(stderr, "  %s uga+rwx file.txt\n", argv[0]);
        fprintf(stderr, "  %s u=rw,g=r,o= file.txt\n", argv[0]);
        fprintf(stderr, "  %s 766 file.txt\n", argv[0]);
        return EXIT_FAILURE;
    }
    
    const char *mode = argv[1];
    const char *filename = argv[2];
    
    if (change_mode(filename, mode) == -1) {
        return EXIT_FAILURE;
    }
    
    return EXIT_SUCCESS;
}