#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define BUF_SIZE 4096

typedef struct {
    const char *pattern;
} SearchCfg;

static void show_usage(const char *prog) {
    fprintf(stderr, "Usage: %s PATTERN [FILE ...]\n", prog);
}

static int has_match(const char *line, const char *pat) {
    return strstr(line, pat) != NULL;
}

static int scan_stream(FILE *fp, const char *label,
                       const SearchCfg *cfg, int show_prefix) {
    char buffer[BUF_SIZE];
    int matched = 0;

    while (fgets(buffer, sizeof(buffer), fp)) {
        size_t len = strlen(buffer);
        if (len && buffer[len - 1] == '\r') {
            buffer[len - 1] = '\0';
        }

        if (has_match(buffer, cfg->pattern)) {
            if (show_prefix) {
                printf("%s:", label);
            }
            fputs(buffer, stdout);
            matched = 1;
        }
    }

    if (ferror(fp)) {
        fprintf(stderr, "mygrep: read error '%s': %s\n",
                label, strerror(errno));
        return 2;
    }

    return matched ? 0 : 1;
}

int main(int argc, char **argv) {
    if (argc < 2) {
        show_usage(argv[0]);
        return 2;
    }

    SearchCfg cfg;
    cfg.pattern = argv[1];

    if (argc == 2) {
        return scan_stream(stdin, "-", &cfg, 0);
    }

    int files_count = argc - 2;
    int print_name = (files_count > 1);
    int exit_code = 0;

    for (int i = 2; i < argc; ++i) {
        FILE *fp;

        if (strcmp(argv[i], "-") == 0) {
            exit_code = scan_stream(stdin, "-", &cfg, print_name);
            continue;
        }

        fp = fopen(argv[i], "r");
        if (!fp) {
            fprintf(stderr, "mygrep: cannot open '%s': %s\n",
                    argv[i], strerror(errno));
            exit_code = 2;
            continue;
        }

        exit_code = scan_stream(fp, argv[i], &cfg, print_name);
        fclose(fp);
    }

    return exit_code;
}
