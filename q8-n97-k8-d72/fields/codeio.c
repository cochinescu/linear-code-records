/* codeio.c: strict reader and writer of code.txt, and the rank of a matrix. */
#define _POSIX_C_SOURCE 200809L
#include "codeio.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CODEIO_MAX_DIM 100000
#define CODEIO_MAX_ENTRIES 100000000L
#define MAX_FIELDS 6   /* longest item line: element w^i c0 c1 c2 */

typedef struct {
    FILE *fp;
    char *line;
    size_t cap;
    long lineno;
} reader_t;

static void set_err(char *err, int errlen, long lineno, const char *fmt, ...)
{
    if (!err || errlen <= 0)
        return;
    int off = 0;
    if (lineno > 0)
        off = snprintf(err, (size_t)errlen, "line %ld: ", lineno);
    if (off < 0 || off >= errlen)
        return;
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(err + off, (size_t)(errlen - off), fmt, ap);
    va_end(ap);
}

/* Next line that is neither empty nor a comment, without its newline.
 * Returns 1 and sets r->line, 0 at end of file, -1 on a line with a NUL
 * byte or a carriage return. */
static int next_line(reader_t *r)
{
    for (;;) {
        ssize_t len = getline(&r->line, &r->cap, r->fp);
        if (len < 0)
            return 0;
        r->lineno++;
        if (len > 0 && r->line[len - 1] == '\n')
            r->line[--len] = '\0';
        if ((ssize_t)strlen(r->line) != len || strchr(r->line, '\r'))
            return -1;
        if (len == 0 || r->line[0] == '#')
            continue;
        return 1;
    }
}

/* Splits s in place at single spaces. Returns the number of fields, or -1
 * if a field is empty (leading, trailing or double space) or there are
 * more than max fields. */
static int split_fields(char *s, char **f, int max)
{
    int nf = 0;
    char *p = s;
    for (;;) {
        char *sp = strchr(p, ' ');
        if (nf >= max)
            return -1;
        f[nf++] = p;
        if (!sp)
            break;
        *sp = '\0';
        p = sp + 1;
    }
    for (int i = 0; i < nf; i++)
        if (f[i][0] == '\0')
            return -1;
    return nf;
}

/* Decimal integer 1..CODEIO_MAX_DIM without sign or leading zero; -1 otherwise. */
static long parse_dim(const char *s)
{
    if (s[0] < '1' || s[0] > '9')
        return -1;
    long v = 0;
    for (const char *p = s; *p; p++) {
        if (*p < '0' || *p > '9')
            return -1;
        v = v * 10 + (*p - '0');
        if (v > CODEIO_MAX_DIM)
            return -1;
    }
    return v;
}

/* Reads the next item line and checks that it is "<key> <value>"; returns
 * the value field or NULL (error written). */
static char *expect_pair(reader_t *r, const char *key, char *err, int errlen)
{
    char *f[MAX_FIELDS];
    int st = next_line(r);
    if (st <= 0) {
        set_err(err, errlen, r->lineno, st == 0 ? "end of file, expected '%s'" : "carriage return or NUL byte (expected '%s')", key);
        return NULL;
    }
    int nf = split_fields(r->line, f, MAX_FIELDS);
    if (nf != 2 || strcmp(f[0], key) != 0) {
        set_err(err, errlen, r->lineno, "expected '%s <value>'", key);
        return NULL;
    }
    return f[1];
}

static int expect_word(reader_t *r, const char *word, char *err, int errlen)
{
    int st = next_line(r);
    if (st <= 0) {
        set_err(err, errlen, r->lineno, st == 0 ? "end of file, expected '%s'" : "carriage return or NUL byte (expected '%s')", word);
        return -1;
    }
    if (strcmp(r->line, word) != 0) {
        set_err(err, errlen, r->lineno, "expected '%s'", word);
        return -1;
    }
    return 0;
}

static int read_body(reader_t *r, codefile_t *out, char *err, int errlen)
{
    char *f[MAX_FIELDS];
    char *v;
    gf_t F;

    v = expect_pair(r, "codetxt", err, errlen);
    if (!v)
        return 2;
    if (strcmp(v, "1") != 0) {
        set_err(err, errlen, r->lineno, "unsupported format version '%s'", v);
        return 2;
    }

    v = expect_pair(r, "q", err, errlen);
    if (!v)
        return 2;
    int q = (strcmp(v, "7") == 0) ? 7 : (strcmp(v, "8") == 0) ? 8 : (strcmp(v, "9") == 0) ? 9 : 0;
    if (q == 0 || gf_init(&F, q) != 0) {
        set_err(err, errlen, r->lineno, "q must be 7, 8 or 9, got '%s'", v);
        return 2;
    }

    v = expect_pair(r, "modulus", err, errlen);
    if (!v)
        return 2;
    if (strcmp(v, gf_modulus_string(q)) != 0) {
        set_err(err, errlen, r->lineno, "modulus for q=%d must be '%s', got '%s'", q, gf_modulus_string(q), v);
        return 2;
    }

    if (q != 7) {
        for (int e = 1; e < q; e++) {
            int st = next_line(r);
            if (st <= 0) {
                set_err(err, errlen, r->lineno, "expected element line for w^%d", e - 1);
                return 2;
            }
            int nf = split_fields(r->line, f, MAX_FIELDS);
            char want[8];
            gf_format_tok(&F, e, want);
            if (nf != 2 + F.m || strcmp(f[0], "element") != 0 || strcmp(f[1], want) != 0) {
                set_err(err, errlen, r->lineno, "expected 'element %s' with %d coefficients", want, F.m);
                return 2;
            }
            for (int i = 0; i < F.m; i++) {
                const char *c = f[2 + i];
                if (c[0] < '0' || c[0] >= '0' + F.p || c[1] != '\0' || c[0] - '0' != F.coef[e][i]) {
                    set_err(err, errlen, r->lineno, "element line for %s disagrees with the modulus %s", want, gf_modulus_string(q));
                    return 2;
                }
            }
        }
    }

    v = expect_pair(r, "n", err, errlen);
    if (!v)
        return 2;
    long n = parse_dim(v);
    if (n < 0) {
        set_err(err, errlen, r->lineno, "bad n '%s'", v);
        return 2;
    }
    v = expect_pair(r, "k", err, errlen);
    if (!v)
        return 2;
    long k = parse_dim(v);
    if (k < 0) {
        set_err(err, errlen, r->lineno, "bad k '%s'", v);
        return 2;
    }
    if (n * k > CODEIO_MAX_ENTRIES) {
        set_err(err, errlen, r->lineno, "matrix too large (%ld x %ld)", k, n);
        return 2;
    }
    if (expect_word(r, "matrix", err, errlen) != 0)
        return 2;

    unsigned char *g = malloc((size_t)(n * k));
    if (!g) {
        set_err(err, errlen, r->lineno, "out of memory");
        return 2;
    }
    for (long i = 0; i < k; i++) {
        int st = next_line(r);
        if (st <= 0) {
            set_err(err, errlen, r->lineno, "expected row %ld of %ld", i + 1, k);
            free(g);
            return 2;
        }
        long j = 0;
        char *p = r->line;
        for (;;) {
            char *sp = strchr(p, ' ');
            if (sp)
                *sp = '\0';
            if (j >= n) {
                set_err(err, errlen, r->lineno, "row %ld has more than %ld entries", i + 1, n);
                free(g);
                return 2;
            }
            int c = gf_parse_tok(&F, p);
            if (c < 0) {
                set_err(err, errlen, r->lineno, "row %ld entry %ld: '%s' is not an element of GF(%d)", i + 1, j + 1, p, q);
                free(g);
                return 2;
            }
            g[i * n + j] = (unsigned char)c;
            j++;
            if (!sp)
                break;
            p = sp + 1;
        }
        if (j != n) {
            set_err(err, errlen, r->lineno, "row %ld has %ld entries, expected %ld", i + 1, j, n);
            free(g);
            return 2;
        }
    }
    if (expect_word(r, "end", err, errlen) != 0) {
        free(g);
        return 2;
    }
    int st = next_line(r);
    if (st != 0) {
        set_err(err, errlen, r->lineno, "content after 'end'");
        free(g);
        return 2;
    }
    out->q = q;
    out->n = (int)n;
    out->k = (int)k;
    out->g = g;
    return 0;
}

int codeio_read(const char *path, codefile_t *out, char *err, int errlen)
{
    if (err && errlen > 0)
        err[0] = '\0';
    if (!out)
        return 2;
    out->q = out->n = out->k = 0;
    out->g = NULL;
    reader_t r = {NULL, NULL, 0, 0};
    r.fp = fopen(path, "r");
    if (!r.fp) {
        set_err(err, errlen, 0, "cannot open %s", path);
        return 2;
    }
    int rc = read_body(&r, out, err, errlen);
    if (rc == 0 && ferror(r.fp)) {
        set_err(err, errlen, 0, "read error on %s", path);
        codeio_free(out);
        rc = 2;
    }
    free(r.line);
    fclose(r.fp);
    return rc;
}

int codeio_write(const char *path, const codefile_t *c)
{
    gf_t F;
    if (!c || !c->g || c->n < 1 || c->k < 1 || gf_init(&F, c->q) != 0)
        return 1;
    for (long i = 0; i < (long)c->n * c->k; i++)
        if (c->g[i] >= c->q)
            return 1;
    FILE *fp = fopen(path, "w");
    if (!fp)
        return 1;
    char sym[8];
    fprintf(fp, "codetxt 1\nq %d\nmodulus %s\n", c->q, gf_modulus_string(c->q));
    if (c->q != 7) {
        for (int e = 1; e < c->q; e++) {
            gf_format_tok(&F, e, sym);
            fprintf(fp, "element %s", sym);
            for (int i = 0; i < F.m; i++)
                fprintf(fp, " %d", F.coef[e][i]);
            fputc('\n', fp);
        }
    }
    fprintf(fp, "n %d\nk %d\nmatrix\n", c->n, c->k);
    for (int i = 0; i < c->k; i++) {
        for (int j = 0; j < c->n; j++) {
            gf_format_tok(&F, c->g[(long)i * c->n + j], sym);
            fprintf(fp, j ? " %s" : "%s", sym);
        }
        fputc('\n', fp);
    }
    fprintf(fp, "end\n");
    int bad = ferror(fp);
    if (fclose(fp) != 0)
        bad = 1;
    return bad ? 1 : 0;
}

void codeio_free(codefile_t *c)
{
    if (!c)
        return;
    free(c->g);
    c->g = NULL;
}

int gf_rank(const gf_t *F, int k, int n, const unsigned char *g)
{
    if (k <= 0 || n <= 0)
        return 0;
    unsigned char *a = malloc((size_t)k * (size_t)n);
    if (!a)
        return -1;
    memcpy(a, g, (size_t)k * (size_t)n);
    int r = 0;
    for (int col = 0; col < n && r < k; col++) {
        int piv = -1;
        for (int i = r; i < k; i++)
            if (a[(size_t)i * n + col]) {
                piv = i;
                break;
            }
        if (piv < 0)
            continue;
        unsigned char *pr = a + (size_t)piv * n;
        if (piv != r) {
            unsigned char *rr = a + (size_t)r * n;
            for (int j = 0; j < n; j++) {
                unsigned char t = rr[j];
                rr[j] = pr[j];
                pr[j] = t;
            }
            pr = rr;
        }
        unsigned char s = F->inv[pr[col]];
        for (int j = col; j < n; j++)
            pr[j] = F->mul[s][pr[j]];
        for (int i = r + 1; i < k; i++) {
            unsigned char *ri = a + (size_t)i * n;
            unsigned char f = ri[col];
            if (!f)
                continue;
            unsigned char nf = F->neg[f];
            for (int j = col; j < n; j++)
                ri[j] = F->add[ri[j]][F->mul[nf][pr[j]]];
        }
        r++;
    }
    free(a);
    return r;
}
