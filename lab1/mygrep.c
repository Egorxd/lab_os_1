#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

typedef struct {
    const char *needle;
} SearchCfg;

static void show_help(const char *app) {
    fprintf(stderr, "Usage: %s PATTERN [FILE ...]\n", app);
}

static int contains_pattern(const char *text, const char *pat) {
    return strstr(text, pat) != NULL;
}

static int scan_input(FILE *in, const char *src,
                      const SearchCfg *cfg, int show_name) {
    char *line = NULL;
    size_t cap = 0;
    ssize_t nread;
    int found = 0;

    while ((nread = getline(&line, &cap, in)) != -1) {
        /* remove CR if present (Windows files) */
        if (nread > 0 && line[nread - 1] == '\r') {
            line[nread - 1] = '\0';
        }

        if (contains_pattern(line, cfg->needle)) {
            if (show_name) {
                printf("%s:", src);
            }
            fputs(line, stdout);
            found = 1;
        }
    }

    if (ferror(in)) {
        fprintf(stderr, "mygrep: read failure '%s': %s\n",
                src, strerror(errno));
        free(line);
        return 2;
    }

    free(line);
    return found ? 0 : 1;
}

int main(int argc, char **argv) {
    if (argc < 2) {
        show_help(argv[0]);
        return 2;
    }

    SearchCfg cfg;
    cfg.needle = argv[1];

    if (argc == 2) {
        return scan_input(stdin, "-", &cfg, 0);
    }

    int files = argc - 2;
    int need_prefix = (files > 1);
    int status = 0;

    for (int i = 2; i < argc; i++) {
        FILE *fp;

        if (strcmp(argv[i], "-") == 0) {
            status = scan_input(stdin, "-", &cfg, need_prefix);
            continue;
        }

        fp = fopen(argv[i], "r");
        if (!fp) {
            fprintf(stderr, "mygrep: cannot open '%s': %s\n",
                    argv[i], strerror(errno));
            status = 2;
            continue;
        }

        status = scan_input(fp, argv[i], &cfg, need_prefix);
        fclose(fp);
    }

    return status;
}
