/* gf.c: builds the tables of gf_t from the defining polynomial. */
#include "gf.h"

#include <stdio.h>
#include <string.h>

const char *gf_modulus_string(int q)
{
    switch (q) {
    case 7: return "none";
    case 8: return "x^3+x+1";
    case 9: return "x^2+2*x+2";
    default: return NULL;
    }
}

/* Characteristic p, degree m, and the low coefficients of the monic
 * defining polynomial x^m + low[m-1] x^(m-1) + ... + low[0]. */
static int field_params(int q, int *p, int *m, int low[3])
{
    low[0] = low[1] = low[2] = 0;
    switch (q) {
    case 7: *p = 7; *m = 1; return 0;
    case 8: *p = 2; *m = 3; low[0] = 1; low[1] = 1; low[2] = 0; return 0;  /* x^3 + x + 1 */
    case 9: *p = 3; *m = 2; low[0] = 2; low[1] = 2; return 0;              /* x^2 + 2*x + 2 */
    default: return -1;
    }
}

/* Product of two polynomials of degree < m over GF(p), reduced by the
 * defining polynomial. */
static void poly_mulmod(const int *a, const int *b, int *out, int p, int m, const int *low)
{
    int prod[5] = {0, 0, 0, 0, 0};
    for (int i = 0; i < m; i++)
        for (int j = 0; j < m; j++)
            prod[i + j] = (prod[i + j] + a[i] * b[j]) % p;
    /* x^d = x^(d-m) * x^m and x^m = -(low[m-1] x^(m-1) + ... + low[0]) */
    for (int d = 2 * m - 2; d >= m; d--) {
        int c = prod[d];
        prod[d] = 0;
        for (int i = 0; i < m; i++)
            prod[d - m + i] = ((prod[d - m + i] - c * low[i]) % p + p) % p;
    }
    for (int i = 0; i < m; i++)
        out[i] = prod[i];
}

static int vec_index(const int *v, int p, int m)
{
    int x = 0;
    for (int i = m - 1; i >= 0; i--)
        x = x * p + v[i];
    return x;
}

int gf_init(gf_t *F, int q)
{
    int p, m, low[3];
    if (!F || field_params(q, &p, &m, low) != 0)
        return -1;
    memset(F, 0, sizeof *F);
    F->q = q;
    F->p = p;
    F->m = m;

    int vec[9][3];        /* coefficient vector of each code */
    int code_of[27];      /* vector index -> code */
    for (int i = 0; i < 27; i++)
        code_of[i] = -1;
    memset(vec, 0, sizeof vec);

    if (m == 1) {
        /* prime field: the code is the residue */
        for (int c = 0; c < q; c++)
            vec[c][0] = c;
    } else {
        /* code 0 is zero, code e >= 1 is w^(e-1) with w = x */
        int x[3] = {0, 1, 0};
        vec[1][0] = 1;
        for (int e = 2; e < q; e++)
            poly_mulmod(vec[e - 1], x, vec[e], p, m, low);
    }
    for (int c = 0; c < q; c++) {
        int ix = vec_index(vec[c], p, m);
        if (code_of[ix] != -1)
            return -1;    /* two codes with the same vector: w is not primitive */
        code_of[ix] = c;
        for (int i = 0; i < 3; i++)
            F->coef[c][i] = (unsigned char)vec[c][i];
    }

    for (int a = 0; a < q; a++) {
        int nv[3];
        for (int i = 0; i < m; i++)
            nv[i] = (p - vec[a][i]) % p;
        F->neg[a] = (unsigned char)code_of[vec_index(nv, p, m)];
        for (int b = 0; b < q; b++) {
            int s[3], t[3];
            for (int i = 0; i < m; i++)
                s[i] = (vec[a][i] + vec[b][i]) % p;
            poly_mulmod(vec[a], vec[b], t, p, m, low);
            int cs = code_of[vec_index(s, p, m)];
            int ct = code_of[vec_index(t, p, m)];
            if (cs < 0 || ct < 0)
                return -1;
            F->add[a][b] = (unsigned char)cs;
            F->mul[a][b] = (unsigned char)ct;
        }
    }
    F->inv[0] = 0;
    for (int a = 1; a < q; a++) {
        int found = -1;
        for (int b = 1; b < q; b++)
            if (F->mul[a][b] == 1)
                found = b;
        if (found < 0)
            return -1;    /* not a field */
        F->inv[a] = (unsigned char)found;
    }
    return 0;
}

int gf_parse_tok(const gf_t *F, const char *tok)
{
    if (!F || !tok)
        return -1;
    if (F->q == 7) {
        if (tok[0] >= '0' && tok[0] <= '6' && tok[1] == '\0')
            return tok[0] - '0';
        return -1;
    }
    if (tok[0] == '0' && tok[1] == '\0')
        return 0;
    if (tok[0] == 'w' && tok[1] == '^' && tok[2] >= '0' && tok[2] <= '9' && tok[3] == '\0') {
        int i = tok[2] - '0';
        if (i <= F->q - 2)
            return i + 1;
    }
    return -1;
}

void gf_format_tok(const gf_t *F, int code, char *buf)
{
    if (!F || code < 0 || code >= F->q) {
        strcpy(buf, "?");
        return;
    }
    if (F->q == 7)
        snprintf(buf, 8, "%d", code);
    else if (code == 0)
        strcpy(buf, "0");
    else
        snprintf(buf, 8, "w^%d", code - 1);
}
