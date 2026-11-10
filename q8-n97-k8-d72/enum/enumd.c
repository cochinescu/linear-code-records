/*
 * enumd: plain enumerator for linear codes over GF(7), GF(8) and GF(9).
 *
 *   enumd [--max-seconds S] <code.txt>
 *   enumd --dump-tables <q>
 *
 * Every nonzero message whose first nonzero coefficient is 1 is visited
 * once, so (q^k - 1)/(q - 1) codewords are generated, and the smallest
 * weight is reported together with one codeword of that weight and its
 * message. The program is self-contained: it builds its own field
 * arithmetic from the defining polynomial and has its own reader of the
 * code.txt format.
 *
 * Field elements are numbered by element codes: for GF(7) the residue
 * 0..6; for GF(8) and GF(9) code 0 is zero and code e >= 1 is w^(e-1),
 * where w is a root of the defining polynomial. In code.txt an element is
 * written as a text symbol: the residue for GF(7), and "0" or "w^i" for
 * GF(8) and GF(9).
 *
 * Exit codes: 0 exact, 2 invalid file, 3 rank below k, 4 time limit
 * reached (or too many codewords for the limit of 2*10^11), 1 any other
 * error (including a failed internal consistency check).
 */

#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define EX_OK 0
#define EX_OTHER 1
#define EX_INVALID 2
#define EX_RANK 3
#define EX_TIMEOUT 4

#define WORD_LIMIT 200000000000ULL   /* 2*10^11 codewords */
#define MAX_FILE_BYTES (1UL << 30)

/* ------------------------------------------------------------------ */
/* Messages                                                            */

static _Noreturn void fail(int code, const char *fmt, ...)
{
    va_list ap;
    fprintf(stderr, "enumd: ");
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
    fprintf(stderr, "\n");
    exit(code);
}

static void *xmalloc(size_t size)
{
    void *p = malloc(size ? size : 1);
    if (!p)
        fail(EX_OTHER, "out of memory (%zu bytes)", size);
    return p;
}

/* ------------------------------------------------------------------ */
/* Field arithmetic                                                    */

typedef struct {
    int q, p, m;                 /* q = p^m */
    const char *modulus;         /* text of the defining polynomial */
    int f[4];                    /* its coefficients, lowest degree first */
    unsigned char vec[9][3];     /* coefficient vector of each element code */
    int code_of_key[27];         /* coefficient vector (as base-p number) -> code */
    unsigned char add[9][9], mul[9][9], neg[9], inv[9];
    char sym[9][4];              /* text symbol of each element code */
} field_t;

static field_t F;

static int vec_key(const unsigned char *v)
{
    int key = 0, mult = 1;
    for (int j = 0; j < F.m; j++) {
        key += v[j] * mult;
        mult *= F.p;
    }
    return key;
}

/* product of two coefficient vectors as polynomials, reduced modulo f */
static void poly_mul(const unsigned char *a, const unsigned char *b, unsigned char *out)
{
    int prod[6] = {0, 0, 0, 0, 0, 0};
    int m = F.m, p = F.p;
    for (int i = 0; i < m; i++)
        for (int j = 0; j < m; j++)
            prod[i + j] += a[i] * b[j];
    for (int d = 2 * m - 2; d >= m; d--) {
        int top = prod[d] % p;
        prod[d] = 0;
        if (!top)
            continue;
        for (int j = 0; j < m; j++)   /* x^d = x^(d-m) * (x^m - f(x)) */
            prod[d - m + j] -= top * F.f[j];
    }
    for (int j = 0; j < m; j++)
        out[j] = (unsigned char)(((prod[j] % p) + p) % p);
}

static void field_check(void)
{
    int q = F.q;
    for (int a = 0; a < q; a++) {
        if (F.add[a][0] != a || F.mul[a][1] != a || F.mul[a][0] != 0)
            fail(EX_OTHER, "internal check failed: identities in GF(%d)", q);
        if (F.add[a][F.neg[a]] != 0 || (a && F.mul[a][F.inv[a]] != 1))
            fail(EX_OTHER, "internal check failed: inverses in GF(%d)", q);
        for (int b = 0; b < q; b++) {
            if (F.add[a][b] != F.add[b][a] || F.mul[a][b] != F.mul[b][a])
                fail(EX_OTHER, "internal check failed: commutativity in GF(%d)", q);
            if (F.m > 1) {
                unsigned char pr[3];
                poly_mul(F.vec[a], F.vec[b], pr);
                if (F.code_of_key[vec_key(pr)] != F.mul[a][b])
                    fail(EX_OTHER, "internal check failed: product of polynomials in GF(%d)", q);
            }
            for (int c = 0; c < q; c++) {
                if (F.add[F.add[a][b]][c] != F.add[a][F.add[b][c]] ||
                    F.mul[F.mul[a][b]][c] != F.mul[a][F.mul[b][c]] ||
                    F.mul[a][F.add[b][c]] != F.add[F.mul[a][b]][F.mul[a][c]])
                    fail(EX_OTHER, "internal check failed: field axioms in GF(%d)", q);
            }
        }
    }
}

static void build_field(int q)
{
    memset(&F, 0, sizeof F);
    F.q = q;
    for (int i = 0; i < 27; i++)
        F.code_of_key[i] = -1;

    if (q == 7) {
        F.p = 7;
        F.m = 1;
        F.modulus = "none";
        for (int a = 0; a < 7; a++) {
            F.vec[a][0] = (unsigned char)a;
            F.code_of_key[a] = a;
            snprintf(F.sym[a], sizeof F.sym[a], "%d", a);
            for (int b = 0; b < 7; b++) {
                F.add[a][b] = (unsigned char)((a + b) % 7);
                F.mul[a][b] = (unsigned char)((a * b) % 7);
            }
        }
    } else {
        if (q == 8) {          /* x^3 + x + 1 over GF(2) */
            F.p = 2;
            F.m = 3;
            F.modulus = "x^3+x+1";
            F.f[0] = 1; F.f[1] = 1; F.f[2] = 0; F.f[3] = 1;
        } else if (q == 9) {   /* x^2 + 2x + 2 over GF(3) */
            F.p = 3;
            F.m = 2;
            F.modulus = "x^2+2*x+2";
            F.f[0] = 2; F.f[1] = 2; F.f[2] = 1;
        } else {
            fail(EX_OTHER, "unsupported field size %d", q);
        }
        int m = F.m, p = F.p;
        /* code 0 = zero; code e = w^(e-1), built by repeated multiplication by x */
        unsigned char cur[3] = {1, 0, 0};
        F.code_of_key[0] = 0;
        strcpy(F.sym[0], "0");
        for (int e = 1; e < q; e++) {
            memcpy(F.vec[e], cur, 3);
            int key = vec_key(cur);
            if (key == 0 || F.code_of_key[key] != -1)
                fail(EX_OTHER, "internal check failed: w is not primitive for GF(%d)", q);
            F.code_of_key[key] = e;
            snprintf(F.sym[e], sizeof F.sym[e], "w^%d", e - 1);
            /* cur = cur * x mod f */
            int top = cur[m - 1];
            for (int j = m - 1; j > 0; j--)
                cur[j] = cur[j - 1];
            cur[0] = 0;
            for (int j = 0; j < m; j++)
                cur[j] = (unsigned char)(((cur[j] - top * F.f[j]) % p + p) % p);
        }
        if (cur[0] != 1 || cur[1] != 0 || (m > 2 && cur[2] != 0))
            fail(EX_OTHER, "internal check failed: w^(q-1) != 1 in GF(%d)", q);
        for (int a = 0; a < q; a++) {
            for (int b = 0; b < q; b++) {
                unsigned char s[3] = {0, 0, 0};
                for (int j = 0; j < m; j++)
                    s[j] = (unsigned char)((F.vec[a][j] + F.vec[b][j]) % p);
                F.add[a][b] = (unsigned char)F.code_of_key[vec_key(s)];
                F.mul[a][b] = (unsigned char)((a && b) ? 1 + ((a - 1) + (b - 1)) % (q - 1) : 0);
            }
        }
    }
    for (int a = 0; a < q; a++) {
        for (int b = 0; b < q; b++) {
            if (F.add[a][b] == 0)
                F.neg[a] = (unsigned char)b;
            if (F.mul[a][b] == 1)
                F.inv[a] = (unsigned char)b;
        }
    }
    field_check();
}

static void dump_tables(int q)
{
    build_field(q);
    printf("q %d\n", q);
    printf("modulus %s\n", F.modulus);
    if (q != 7) {
        for (int e = 1; e < q; e++) {
            printf("element %d %s", e, F.sym[e]);
            for (int j = 0; j < F.m; j++)
                printf(" %d", F.vec[e][j]);
            printf("\n");
        }
    }
    for (int a = 0; a < q; a++)
        for (int b = 0; b < q; b++)
            printf("add %d %d %d\n", a, b, F.add[a][b]);
    for (int a = 0; a < q; a++)
        for (int b = 0; b < q; b++)
            printf("mul %d %d %d\n", a, b, F.mul[a][b]);
    printf("end\n");
}

/* ------------------------------------------------------------------ */
/* Reader of code.txt                                                  */

typedef struct {
    const char *s;
    size_t len;
    long lineno;
} line_t;

static char *read_file(const char *path, size_t *size)
{
    FILE *fp = fopen(path, "rb");
    if (!fp)
        fail(EX_OTHER, "cannot open %s", path);
    size_t cap = 1 << 16, len = 0;
    char *buf = xmalloc(cap);
    for (;;) {
        if (len == cap) {
            if (cap >= MAX_FILE_BYTES)
                fail(EX_INVALID, "invalid file: larger than %lu bytes", (unsigned long)MAX_FILE_BYTES);
            cap *= 2;
            buf = realloc(buf, cap);
            if (!buf)
                fail(EX_OTHER, "out of memory");
        }
        size_t got = fread(buf + len, 1, cap - len, fp);
        len += got;
        if (got == 0) {
            if (ferror(fp))
                fail(EX_OTHER, "read error on %s", path);
            break;
        }
    }
    fclose(fp);
    *size = len;
    return buf;
}

static int line_is(const line_t *L, const char *text)
{
    size_t n = strlen(text);
    return L->len == n && memcmp(L->s, text, n) == 0;
}

/* "<word> <positive decimal integer without leading zeros>" */
static long parse_count_line(const line_t *L, const char *word)
{
    size_t w = strlen(word);
    if (L->len < w + 2 || memcmp(L->s, word, w) != 0 || L->s[w] != ' ')
        fail(EX_INVALID, "invalid file: line %ld: expected '%s <number>'", L->lineno, word);
    const char *d = L->s + w + 1;
    size_t nd = L->len - w - 1;
    if (nd > 9 || d[0] < '1' || d[0] > '9')
        fail(EX_INVALID, "invalid file: line %ld: '%s' needs a positive integer without leading zeros", L->lineno, word);
    long v = 0;
    for (size_t i = 0; i < nd; i++) {
        if (d[i] < '0' || d[i] > '9')
            fail(EX_INVALID, "invalid file: line %ld: '%s' needs a positive integer", L->lineno, word);
        v = v * 10 + (d[i] - '0');
    }
    return v;
}

static int symbol_code(const char *s, size_t len)
{
    for (int e = 0; e < F.q; e++)
        if (strlen(F.sym[e]) == len && memcmp(F.sym[e], s, len) == 0)
            return e;
    return -1;
}

/* Reads and validates the file; returns the matrix as element codes, k rows of n. */
static unsigned char *read_code(const char *path, int *pq, int *pn, int *pk)
{
    size_t size;
    char *buf = read_file(path, &size);

    /* plain ASCII: printable characters and line feeds only */
    long lineno = 1;
    for (size_t i = 0; i < size; i++) {
        unsigned char c = (unsigned char)buf[i];
        if (c == '\n') {
            lineno++;
            continue;
        }
        if (c < 0x20 || c > 0x7e)
            fail(EX_INVALID, "invalid file: line %ld: byte 0x%02x is not allowed "
                 "(plain ASCII, single spaces, line feeds only)", lineno, c);
    }

    /* split into lines, keeping only those that are not empty and not comments */
    size_t maxlines = (size_t)lineno + 1, nl = 0;
    line_t *lines = xmalloc(maxlines * sizeof *lines);
    lineno = 1;
    for (size_t start = 0; start < size;) {
        size_t end = start;
        while (end < size && buf[end] != '\n')
            end++;
        if (end > start && buf[start] != '#') {
            lines[nl].s = buf + start;
            lines[nl].len = end - start;
            lines[nl].lineno = lineno;
            nl++;
        }
        lineno++;
        start = end + 1;
    }

    size_t at = 0;
#define NEXT_LINE(what)                                                              \
    do {                                                                             \
        if (at >= nl)                                                                \
            fail(EX_INVALID, "invalid file: ends before the %s line", (what));       \
        L = &lines[at++];                                                            \
    } while (0)
    const line_t *L;

    NEXT_LINE("codetxt");
    if (!line_is(L, "codetxt 1"))
        fail(EX_INVALID, "invalid file: line %ld: expected 'codetxt 1'", L->lineno);

    NEXT_LINE("q");
    int q;
    if (line_is(L, "q 7"))
        q = 7;
    else if (line_is(L, "q 8"))
        q = 8;
    else if (line_is(L, "q 9"))
        q = 9;
    else
        fail(EX_INVALID, "invalid file: line %ld: expected 'q 7', 'q 8' or 'q 9'", L->lineno);
    build_field(q);

    char expect[64];
    NEXT_LINE("modulus");
    snprintf(expect, sizeof expect, "modulus %s", F.modulus);
    if (!line_is(L, expect))
        fail(EX_INVALID, "invalid file: line %ld: expected '%s' for q = %d", L->lineno, expect, q);

    if (q != 7) {
        for (int e = 1; e < q; e++) {
            NEXT_LINE("element");
            int len = snprintf(expect, sizeof expect, "element %s", F.sym[e]);
            for (int j = 0; j < F.m; j++)
                len += snprintf(expect + len, sizeof expect - (size_t)len, " %d", F.vec[e][j]);
            if (!line_is(L, expect))
                fail(EX_INVALID, "invalid file: line %ld: expected '%s' (element lines must "
                     "agree with the modulus, in order w^0 .. w^%d)", L->lineno, expect, q - 2);
        }
    }

    NEXT_LINE("n");
    long n = parse_count_line(L, "n");
    NEXT_LINE("k");
    long k = parse_count_line(L, "k");
    NEXT_LINE("matrix");
    if (!line_is(L, "matrix"))
        fail(EX_INVALID, "invalid file: line %ld: expected 'matrix'", L->lineno);

    /* every entry needs at least one byte, so a shorter file cannot be valid */
    if ((unsigned long long)n * (unsigned long long)k > size)
        fail(EX_INVALID, "invalid file: too short to hold %ld x %ld entries", k, n);

    unsigned char *G = xmalloc((size_t)n * (size_t)k);
    for (long r = 0; r < k; r++) {
        NEXT_LINE("matrix row");
        if (line_is(L, "end"))
            fail(EX_INVALID, "invalid file: line %ld: 'end' after %ld of %ld matrix rows", L->lineno, r, k);
        size_t pos = 0;
        for (long c = 0; c < n; c++) {
            if (pos >= L->len)
                fail(EX_INVALID, "invalid file: line %ld: row has %ld symbols, expected %ld", L->lineno, c, n);
            size_t end = pos;
            while (end < L->len && L->s[end] != ' ')
                end++;
            if (end == pos)
                fail(EX_INVALID, "invalid file: line %ld: empty symbol (symbols are separated by single spaces)", L->lineno);
            int e = symbol_code(L->s + pos, end - pos);
            if (e < 0)
                fail(EX_INVALID, "invalid file: line %ld: '%.*s' is not a valid symbol for q = %d",
                     L->lineno, (int)(end - pos), L->s + pos, q);
            G[(size_t)r * (size_t)n + (size_t)c] = (unsigned char)e;
            pos = end;
            if (c < n - 1) {
                if (pos >= L->len)
                    fail(EX_INVALID, "invalid file: line %ld: row has %ld symbols, expected %ld", L->lineno, c + 1, n);
                pos++;   /* the single space */
            }
        }
        if (pos != L->len)
            fail(EX_INVALID, "invalid file: line %ld: row has more than %ld symbols or a trailing space", L->lineno, n);
    }

    NEXT_LINE("end");
    if (!line_is(L, "end"))
        fail(EX_INVALID, "invalid file: line %ld: expected 'end' after %ld matrix rows", L->lineno, k);
    if (at != nl)
        fail(EX_INVALID, "invalid file: line %ld: nothing may follow 'end'", lines[at].lineno);
#undef NEXT_LINE

    free(lines);
    free(buf);
    *pq = q;
    *pn = (int)n;
    *pk = (int)k;
    return G;
}

/* ------------------------------------------------------------------ */
/* Rank                                                                */

static int matrix_rank(const unsigned char *G, int n, int k)
{
    unsigned char *M = xmalloc((size_t)n * (size_t)k);
    memcpy(M, G, (size_t)n * (size_t)k);
    int rank = 0;
    for (int col = 0; col < n && rank < k; col++) {
        int piv = -1;
        for (int r = rank; r < k; r++)
            if (M[(size_t)r * n + col]) {
                piv = r;
                break;
            }
        if (piv < 0)
            continue;
        unsigned char *R = M + (size_t)rank * n;
        if (piv != rank) {
            unsigned char *P = M + (size_t)piv * n;
            for (int j = 0; j < n; j++) {
                unsigned char t = R[j];
                R[j] = P[j];
                P[j] = t;
            }
        }
        unsigned char s = F.inv[R[col]];
        for (int j = 0; j < n; j++)
            R[j] = F.mul[s][R[j]];
        for (int r = 0; r < k; r++) {
            if (r == rank)
                continue;
            unsigned char *S = M + (size_t)r * n;
            unsigned char fac = S[col];
            if (!fac)
                continue;
            for (int j = 0; j < n; j++)
                S[j] = F.add[S[j]][F.neg[F.mul[fac][R[j]]]];
        }
        rank++;
    }
    free(M);
    return rank;
}

/* ------------------------------------------------------------------ */
/* Enumeration                                                         */
/*
 * A codeword is held as m planes of n bytes: plane j holds coefficient j
 * of every entry (an element of GF(p)), so adding two words is a
 * coordinatewise addition modulo p.
 */

static double now_seconds(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec + (double)ts.tv_nsec * 1e-9;
}

static void add_planes(unsigned char *restrict c, const unsigned char *restrict d, size_t len, unsigned p)
{
    for (size_t i = 0; i < len; i++) {
        unsigned x = (unsigned)c[i] + d[i];
        c[i] = (unsigned char)(x >= p ? x - p : x);
    }
}

static int weight_planes(const unsigned char *c, int n, int m)
{
    int w = 0;
    if (m == 1) {
        for (int i = 0; i < n; i++)
            w += c[i] != 0;
    } else if (m == 2) {
        for (int i = 0; i < n; i++)
            w += (c[i] | c[n + i]) != 0;
    } else {
        for (int i = 0; i < n; i++)
            w += (c[i] | c[n + i] | c[2 * n + i]) != 0;
    }
    return w;
}

static void put_planes(unsigned char *planes, int n, int i, int e)
{
    for (int j = 0; j < F.m; j++)
        planes[(size_t)j * n + i] = F.vec[e][j];
}

static void print_symbols(const char *key, const unsigned char *codes, int len)
{
    printf("%s=", key);
    for (int i = 0; i < len; i++)
        printf(i ? " %s" : "%s", F.sym[codes[i]]);
    printf("\n");
}

static _Noreturn void usage(void)
{
    fail(EX_OTHER, "usage: enumd [--max-seconds S] <code.txt> | enumd --dump-tables <q>");
}

int main(int argc, char **argv)
{
    double t0 = now_seconds();
    double max_seconds = 0;
    const char *path = NULL;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--dump-tables") == 0) {
            if (i + 1 >= argc || argc != 3)
                usage();
            const char *a = argv[i + 1];
            if (strcmp(a, "7") && strcmp(a, "8") && strcmp(a, "9"))
                fail(EX_OTHER, "--dump-tables takes 7, 8 or 9");
            dump_tables(a[0] - '0');
            return fflush(stdout) == 0 ? EX_OK : EX_OTHER;
        } else if (strcmp(argv[i], "--max-seconds") == 0) {
            if (i + 1 >= argc)
                usage();
            char *endp;
            max_seconds = strtod(argv[++i], &endp);
            if (*endp != '\0' || !(max_seconds > 0) || max_seconds > 1e9)
                fail(EX_OTHER, "--max-seconds needs a positive number");
        } else if (argv[i][0] == '-' && argv[i][1] != '\0') {
            usage();
        } else {
            if (path)
                usage();
            path = argv[i];
        }
    }
    if (!path)
        usage();

    int q, n, k;
    unsigned char *G = read_code(path, &q, &n, &k);
    int rank = matrix_rank(G, n, k);

    printf("q=%d\nn=%d\nk=%d\nrank=%d\n", q, n, k, rank);
    if (rank < k) {
        fflush(stdout);
        fprintf(stderr, "enumd: rank %d is below k = %d\n", rank, k);
        return EX_RANK;
    }

    /* number of codewords to visit: (q^k - 1)/(q - 1) = 1 + q + ... + q^(k-1) */
    unsigned long long total = 0, power = 1;
    int too_large = 0;
    for (int i = 0; i < k; i++) {
        total += power;
        if (total > WORD_LIMIT) {
            too_large = 1;
            break;
        }
        power *= (unsigned long long)q;
    }
    if (too_large) {
        printf("status=timeout\n");
        printf("# not started: (q^k-1)/(q-1) exceeds the limit of 2*10^11 codewords\n");
        return EX_TIMEOUT;
    }

    int m = F.m;
    unsigned p = (unsigned)F.p;
    size_t L = (size_t)m * (size_t)n;

    /* rows in plane form, and the steps of the odometer:
       D[r][t] = (code(t+1 mod q) - code(t)) * row r */
    unsigned char *rowp = xmalloc((size_t)k * L);
    unsigned char *D = xmalloc((size_t)k * (size_t)q * L);
    for (int r = 0; r < k; r++) {
        for (int i = 0; i < n; i++)
            put_planes(rowp + (size_t)r * L, n, i, G[(size_t)r * n + i]);
        for (int t = 0; t < q; t++) {
            int nxt = (t + 1) % q;
            int diff = F.add[nxt][F.neg[t]];
            unsigned char *Dt = D + ((size_t)r * q + t) * L;
            for (int i = 0; i < n; i++)
                put_planes(Dt, n, i, F.mul[diff][G[(size_t)r * n + i]]);
        }
    }

    unsigned char *cw = xmalloc(L), *best_cw = xmalloc(L);
    unsigned char *dig = xmalloc((size_t)k), *best_msg = xmalloc((size_t)k);
    int best = n + 1;
    unsigned long long count = 0;
    double deadline = max_seconds > 0 ? t0 + max_seconds : 0;
    int timed_out = 0;

    for (int lead = 0; lead < k && !timed_out; lead++) {
        memcpy(cw, rowp + (size_t)lead * L, L);
        memset(dig, 0, (size_t)k);
        dig[lead] = 1;
        for (;;) {
            int w = weight_planes(cw, n, m);
            count++;
            if (w < best) {
                best = w;
                memcpy(best_msg, dig, (size_t)k);
                memcpy(best_cw, cw, L);
            }
            if ((count & 0xFFFF) == 0 && deadline > 0 && now_seconds() >= deadline) {
                timed_out = 1;
                break;
            }
            int r = k - 1;
            while (r > lead) {
                add_planes(cw, D + ((size_t)r * q + dig[r]) * L, L, p);
                dig[r] = (unsigned char)(dig[r] + 1 == q ? 0 : dig[r] + 1);
                if (dig[r] != 0)
                    break;
                r--;
            }
            if (r == lead)
                break;
        }
        /* after a full cycle every coordinate is back at 0: the word is row lead again */
        if (!timed_out && memcmp(cw, rowp + (size_t)lead * L, L) != 0)
            fail(EX_OTHER, "internal check failed: running word drifted in block %d", lead);
    }
    double secs = now_seconds() - t0;

    if (timed_out) {
        printf("status=timeout\n");
        printf("# codewords=%llu of %llu\n", count, total);
        printf("# best_weight_so_far=%d\n", best);
        printf("# seconds=%.3f\n", secs);
        return EX_TIMEOUT;
    }
    if (count != total)
        fail(EX_OTHER, "internal check failed: visited %llu codewords, expected %llu", count, total);

    /* recompute message * G and its weight from scratch with the tables */
    unsigned char *word = xmalloc((size_t)n), *check = xmalloc(L);
    int lead = -1;
    for (int j = 0; j < k; j++)
        if (best_msg[j]) {
            lead = j;
            break;
        }
    if (lead < 0 || best_msg[lead] != 1)
        fail(EX_OTHER, "internal check failed: message is not normalized");
    int w2 = 0;
    for (int i = 0; i < n; i++) {
        int acc = 0;
        for (int j = 0; j < k; j++)
            acc = F.add[acc][F.mul[best_msg[j]][G[(size_t)j * n + i]]];
        word[i] = (unsigned char)acc;
        w2 += acc != 0;
        put_planes(check, n, i, acc);
    }
    if (memcmp(check, best_cw, L) != 0 || w2 != best || best < 1)
        fail(EX_OTHER, "internal check failed: recomputed codeword or weight differs (%d vs %d)", w2, best);

    printf("status=exact\n");
    printf("d=%d\n", best);
    print_symbols("codeword", word, n);
    print_symbols("message", best_msg, k);
    printf("# codewords=%llu\n", count);
    printf("# seconds=%.3f\n", secs);
    printf("# codewords_per_second=%.0f\n", secs > 0 ? (double)count / secs : 0.0);
    return fflush(stdout) == 0 ? EX_OK : EX_OTHER;
}
