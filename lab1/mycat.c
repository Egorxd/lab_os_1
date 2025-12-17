#include <stdio.h>
#include <string.h>
#include <errno.h>

typedef struct {
    int num_all;
    int num_nonempty;
    int mark_eol;
} Flags;

static void usage(const char *app) {
    fprintf(stderr,
        "Usage: %s [-n] [-b] [-E] [FILE ...]\n"
        "       %s [--] [FILE ...]\n", app, app);
}

static int copy_file(FILE *stream, const char *label, const Flags *flg) {
    int ch;
    int new_line = 1;
    long long counter = 1;
    int status = 0;

    while ((ch = getc(stream)) != EOF) {
        if (new_line) {
            int print_num = 0;

            if (flg->num_nonempty && ch != '\n') {
                print_num = 1;
            } else if (flg->num_all && !flg->num_nonempty) {
                print_num = 1;
            }

            if (print_num) {
                printf("%6lld\t", counter++);
            }
            new_line = 0;
        }

        if (flg->mark_eol && ch == '\n') {
            putchar('$');
        }

        putchar(ch);

        if (ch == '\n') {
            new_line = 1;
        }
    }

    if (ferror(stream)) {
        fprintf(stderr, "mycat: read error '%s': %s\n",
                label, strerror(errno));
        status = 1;
    }

    return status;
}

int main(int argc, char *argv[]) {
    Flags flg = {0, 0, 0};
    int pos = 1;
    int stop_opts = 0;

    while (pos < argc) {
        char *arg = argv[pos];

        if (!stop_opts && strcmp(arg, "--") == 0) {
            stop_opts = 1;
            pos++;
            continue;
        }

        if (!stop_opts && arg[0] == '-' && arg[1]) {
            for (int i = 1; arg[i]; i++) {
                switch (arg[i]) {
                    case 'n': flg.num_all = 1; break;
                    case 'b': flg.num_nonempty = 1; break;
                    case 'E': flg.mark_eol = 1; break;
                    default:
                        fprintf(stderr, "mycat: invalid option -- %c\n", arg[i]);
                        usage(argv[0]);
                        return 2;
                }
            }
            pos++;
        } else {
            break;
        }
    }

    if (flg.num_nonempty) {
        flg.num_all = 0;
    }

    int result = 0;

    if (pos == argc) {
        return copy_file(stdin, "-", &flg);
    }

    for (; pos < argc; pos++) {
        FILE *f;

        if (strcmp(argv[pos], "-") == 0) {
            result |= copy_file(stdin, "-", &flg);
            continue;
        }

        f = fopen(argv[pos], "rb");
        if (!f) {
            fprintf(stderr, "mycat: cannot open '%s': %s\n",
                    argv[pos], strerror(errno));
            result = 1;
            continue;
        }

        result |= copy_file(f, argv[pos], &flg);
        fclose(f);
    }

    return result;
}
