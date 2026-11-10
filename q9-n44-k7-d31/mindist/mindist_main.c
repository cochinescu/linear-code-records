/* mindist_main.c: command line front end of the minimum distance library.
 *
 *   mindist [--bound B] [--max-seconds S] <code.txt>
 *
 * Output on stdout, one key=value per line (q, n, k, rank, status, d,
 * codeword, message); lines starting with '#' are statistics.
 * Exit codes: 0 exact, 10 early exit (a codeword of weight <= B was
 * found), 2 invalid file, 3 rank below k, 4 time limit reached, 1 other
 * error. On a time limit, d, codeword and message (if present) are the
 * lightest codeword found so far, an upper bound only.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "../fields/gf.h"
#include "../fields/codeio.h"
#include "mindist.h"

static void usage(void)
{
    fprintf(stderr, "usage: mindist [--bound B] [--max-seconds S] <code.txt>\n");
}

static void print_vector(const gf_t *F, const char *key, int len, const unsigned char *v)
{
    char buf[16];
    printf("%s=", key);
    for (int i = 0; i < len; i++) {
        gf_format_tok(F, v[i], buf);
        printf(i ? " %s" : "%s", buf);
    }
    printf("\n");
}

int main(int argc, char **argv)
{
    long bound = -1;
    double max_seconds = 0;
    const char *path = NULL;

    for (int a = 1; a < argc; a++) {
        if (strcmp(argv[a], "--bound") == 0 && a + 1 < argc) {
            char *end;
            errno = 0;
            bound = strtol(argv[++a], &end, 10);
            if (errno || *end || end == argv[a] || bound < 0 || bound > 1000000) { usage(); return 1; }
        } else if (strcmp(argv[a], "--max-seconds") == 0 && a + 1 < argc) {
            char *end;
            errno = 0;
            max_seconds = strtod(argv[++a], &end);
            if (errno || *end || end == argv[a] || !(max_seconds > 0)) { usage(); return 1; }
        } else if (argv[a][0] == '-' && argv[a][1] == '-') {
            usage();
            return 1;
        } else if (!path) {
            path = argv[a];
        } else {
            usage();
            return 1;
        }
    }
    if (!path) { usage(); return 1; }

    codefile_t C;
    char err[256] = "";
    if (codeio_read(path, &C, err, (int)sizeof err) != 0) {
        fprintf(stderr, "mindist: invalid file %s: %s\n", path, err);
        return 2;
    }
    gf_t F;
    if (gf_init(&F, C.q) != 0) {
        fprintf(stderr, "mindist: field initialisation failed for q=%d\n", C.q);
        codeio_free(&C);
        return 1;
    }

    md_result_t R;
    int st = md_min_distance(&F, C.n, C.k, C.g, (int)bound, max_seconds, &R);

    printf("q=%d\nn=%d\nk=%d\nrank=%d\n", C.q, C.n, C.k, R.rank);
    int ret;
    switch (st) {
    case MD_EXACT:   printf("status=exact\n");      ret = 0;  break;
    case MD_EARLY:   printf("status=early_exit\n"); ret = 10; break;
    case MD_TIMEOUT: printf("status=timeout\n");    ret = 4;  break;
    case MD_RANK:
        fprintf(stderr, "mindist: rank %d is below k = %d\n", R.rank, C.k);
        ret = 3;
        break;
    default:
        fprintf(stderr, "mindist: internal error\n");
        ret = 1;
        break;
    }
    if ((st == MD_EXACT || st == MD_EARLY || st == MD_TIMEOUT) && R.codeword && R.message) {
        printf("d=%d\n", R.d);
        print_vector(&F, "codeword", C.n, R.codeword);
        print_vector(&F, "message", C.k, R.message);
    }
    printf("# infosets=%d\n", R.infosets);
    printf("# enumerated=%llu\n", R.enumerated);
    printf("# last_w=%d\n", R.last_w);
    printf("# lower_bound=%d\n", R.lower_bound);
    printf("# seconds=%.3f\n", R.seconds);

    md_result_free(&R);
    codeio_free(&C);
    if (fflush(stdout) != 0) return 1;
    return ret;
}
