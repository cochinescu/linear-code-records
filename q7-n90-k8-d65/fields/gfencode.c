/* gfencode <code.txt> <count> <seed>: prints count lines
 *   m_1 .. m_k ; c_1 .. c_n
 * (element codes) with c = m*G, the messages m drawn from a xorshift64
 * generator seeded with seed. Exit 0 ok, 1 usage, 2 invalid file. */
#include "codeio.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

static unsigned long long rng_state;

static unsigned long long xorshift64(void)
{
    unsigned long long x = rng_state;
    x ^= x << 13;
    x ^= x >> 7;
    x ^= x << 17;
    rng_state = x;
    return x;
}

static int parse_ull(const char *s, unsigned long long *v)
{
    char *end;
    if (s[0] < '0' || s[0] > '9')
        return -1;
    errno = 0;
    *v = strtoull(s, &end, 10);
    return (errno || *end) ? -1 : 0;
}

int main(int argc, char **argv)
{
    unsigned long long count, seed;
    if (argc != 4 || parse_ull(argv[2], &count) != 0 || parse_ull(argv[3], &seed) != 0) {
        fprintf(stderr, "usage: gfencode <code.txt> <count> <seed>\n");
        return 1;
    }
    codefile_t C;
    char err[256];
    if (codeio_read(argv[1], &C, err, sizeof err) != 0) {
        fprintf(stderr, "gfencode: %s: %s\n", argv[1], err);
        return 2;
    }
    gf_t F;
    gf_init(&F, C.q);
    /* a zero state would stay zero; mix the seed with a fixed odd constant */
    rng_state = seed ^ 0x9E3779B97F4A7C15ULL;
    if (rng_state == 0)
        rng_state = 0x9E3779B97F4A7C15ULL;

    unsigned char *m = malloc((size_t)C.k);
    unsigned char *c = malloc((size_t)C.n);
    if (!m || !c) {
        fprintf(stderr, "gfencode: out of memory\n");
        return 1;
    }
    for (unsigned long long t = 0; t < count; t++) {
        for (int i = 0; i < C.k; i++)
            m[i] = (unsigned char)(xorshift64() % (unsigned long long)C.q);
        for (int j = 0; j < C.n; j++) {
            unsigned char s = 0;
            for (int i = 0; i < C.k; i++)
                s = F.add[s][F.mul[m[i]][C.g[(size_t)i * C.n + j]]];
            c[j] = s;
        }
        for (int i = 0; i < C.k; i++)
            printf(i ? " %d" : "%d", m[i]);
        printf(" ;");
        for (int j = 0; j < C.n; j++)
            printf(" %d", c[j]);
        printf("\n");
    }
    free(m);
    free(c);
    codeio_free(&C);
    return fflush(stdout) == 0 ? 0 : 1;
}
