/* mindist.c: exact minimum distance of a linear [n,k] code over GF(q),
 * q = 7, 8, 9, by the Brouwer-Zimmermann algorithm.
 *
 * ALGORITHM
 *
 * Input: a k x n generator matrix G (element codes, row-major), an
 * optional bound B >= 0 and an optional time limit.
 *
 * 1. Information sets. Start with all n columns unused. Repeat:
 *    Gaussian elimination of the full k x n matrix, using pivots only in
 *    unused columns, gives a matrix G_j that generates the same code, has
 *    rank r_j on the unused columns, an identity on its r_j pivot columns
 *    (the information set I_j), and whose k - r_j rows without a pivot
 *    are zero on every unused column. Mark I_j as used. Stop when r_j = 0
 *    or no unused column is left. r_1 must be k, else the rank of G is
 *    below k (MD_RANK). Keep every G_j with 4 * r_j > k; a G_j with
 *    r_j <= k/4 is dropped (it is never enumerated and contributes nothing
 *    to the bound below). G_1 is always kept.
 *    The elimination is done on [G | I_k], so each G_j comes with the
 *    k x k matrix T_j such that G_j = T_j * G.
 *
 * 2. Enumeration. For w = 1, 2, ..., k: for every kept G_j, generate every
 *    codeword a * G_j where the message a has exactly w nonzero
 *    coefficients and its first nonzero coefficient is 1 (scalar
 *    multiples have the same weight). Track the smallest weight found and
 *    one codeword attaining it, with its matrix j and its message a.
 *
 * 3. Termination. Let c = a * G_j be any codeword. On a pivot column of
 *    G_j that belongs to row p, the rows with a pivot are the identity and
 *    the rows without a pivot are zero (the column was unused when G_j
 *    was built), so c equals a_p there. Hence c has at least
 *    wt(a) - (k - r_j) nonzero entries on I_j. When every kept G_j has been
 *    enumerated for all w' <= w, a codeword not yet generated has a
 *    message of weight >= w + 1 with respect to every kept G_j, and the
 *    sets I_j are disjoint, so its weight is at least
 *        L_w = sum over kept j of max(0, (w + 1) - (k - r_j)).
 *    After all kept matrices are done for weight w: stop when
 *    L_w >= the smallest weight found; that weight is then the minimum
 *    distance d. At w = k every codeword has been generated, so the
 *    smallest weight found is d.
 *
 * 4. Early exit. With a bound B >= 0, stop at the first codeword of
 *    weight <= B and return MD_EARLY with that codeword. If d <= B this
 *    always happens before step 3 stops: a minimum weight word not yet
 *    generated keeps L_w <= d <= B < (smallest weight found).
 *
 * 5. Output check. The message for the original matrix is m = a * T_j.
 *    Before returning, m * G is computed with plain field arithmetic and
 *    must equal the codeword a * G_j (also computed with plain field
 *    arithmetic when it was recorded), and its weight must equal d.
 *    Any failure returns MD_ERROR.
 *
 * Nothing but the generator matrix is used: no symmetry and no other
 * structure of the code is assumed. Single thread.
 *
 * SPEED (the enumeration of step 2): every element is one byte. For every
 * kept G_j the products c * (row i) are precomputed for all c. The rows
 * i_1 < i_2 < ... < i_w are chosen depth first; partial[t] holds the sum
 * of the first t chosen rows times their coefficients, so a new partial
 * sum costs one vector addition through the flat table addt. In the last
 * level the sum is not stored: p + c*v is zero at x exactly when
 * p[x] == (-c)*v[x], so its weight is the number of x with
 * p[x] != ((-c)*v)[x], a plain byte comparison.
 */
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "mindist.h"

#define CHECK_CLOCK_EVERY (1ULL << 18)   /* codewords between two clock reads */

typedef struct {
    int r;                  /* r_j: pivots found, = |I_j| */
    unsigned char *G;       /* k x n: G_j */
    unsigned char *T;       /* k x k: G_j = T_j * G */
    unsigned char *mult;    /* k x q x n: mult[(i*q + c)*n + x] = c * G_j[i][x] */
} infoset_t;

typedef struct {
    const gf_t *F;
    int n, k, q;
    unsigned char addt[256];    /* addt[(a << 4) | b] = a + b */
    /* the level being enumerated */
    int w;                      /* rows per combination */
    int set;                    /* index of the kept matrix being enumerated */
    const infoset_t *S;         /* that matrix */
    unsigned char *partial;     /* (k + 1) x n; partial[0] is zero */
    int *rows, *coefs;          /* rows[t], coefs[t]: the t-th chosen row and its coefficient */
    /* the best codeword so far */
    int best;                   /* its weight; n + 1 while none was found */
    int best_set;               /* the matrix it came from */
    unsigned char *best_a;      /* k: its message with respect to G_best_set */
    unsigned char *best_word;   /* n: a * G_best_set, by plain arithmetic */
    /* control */
    int bound;
    double deadline;            /* < 0: none */
    unsigned long long enumerated, next_check;
    int stop;                   /* 0 while running, else MD_EARLY, MD_TIMEOUT or MD_ERROR */
} enum_t;

static double now_seconds(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec + 1e-9 * (double)ts.tv_nsec;
}

/* ---- plain field arithmetic on vectors (used outside the inner loops) ---- */

/* out (length ncols) = sum over i < nrows of a[i] * M[i][...] */
static void vec_times_matrix(const gf_t *F, int nrows, int ncols,
                             const unsigned char *a, const unsigned char *M,
                             unsigned char *out)
{
    memset(out, 0, (size_t)ncols);
    for (int i = 0; i < nrows; i++) {
        if (a[i] == 0) continue;
        for (int x = 0; x < ncols; x++)
            out[x] = F->add[out[x]][F->mul[a[i]][M[(size_t)i * ncols + x]]];
    }
}

static int weight(int n, const unsigned char *v)
{
    int wt = 0;
    for (int x = 0; x < n; x++) wt += (v[x] != 0);
    return wt;
}

/* ---- step 1: one information set ---- */

/* A is k x W with W = n + k, holding [G | I_k] on entry. Row-reduces A
 * using pivots only in columns x < n with used[x] == 0, scanning those
 * columns from left to right. Every pivot column x gets newpivot[x] = 1
 * and becomes a unit column (1 in its pivot row, 0 elsewhere). Returns the
 * number of pivots r; the pivot rows are rows 0..r-1. The rows r..k-1 are
 * zero on every column x < n with used[x] == 0. */
static int reduce_unused(const gf_t *F, int n, int k, unsigned char *A,
                         const unsigned char *used, unsigned char *newpivot)
{
    const int W = n + k;
    unsigned char *tmp = malloc((size_t)W);
    int r = 0;
    if (!tmp) return -1;
    for (int x = 0; x < n && r < k; x++) {
        if (used[x]) continue;
        int p = -1;
        for (int i = r; i < k; i++)
            if (A[(size_t)i * W + x] != 0) { p = i; break; }
        if (p < 0) continue;            /* zero on all rows without a pivot */
        if (p != r) {                   /* swap rows p and r */
            memcpy(tmp, A + (size_t)p * W, (size_t)W);
            memcpy(A + (size_t)p * W, A + (size_t)r * W, (size_t)W);
            memcpy(A + (size_t)r * W, tmp, (size_t)W);
        }
        unsigned char *pr = A + (size_t)r * W;
        unsigned char s = F->inv[pr[x]];
        for (int y = 0; y < W; y++) pr[y] = F->mul[s][pr[y]];
        for (int i = 0; i < k; i++) {   /* clear column x in every other row */
            if (i == r) continue;
            unsigned char *ri = A + (size_t)i * W;
            unsigned char f = ri[x];
            if (f == 0) continue;
            unsigned char nf = F->neg[f];
            for (int y = 0; y < W; y++) ri[y] = F->add[ri[y]][F->mul[nf][pr[y]]];
        }
        newpivot[x] = 1;
        r++;
    }
    free(tmp);
    return r;
}

/* L_w of step 3 over the kept matrices */
static int lower_bound_after(int w, int k, const infoset_t *S, int nsets)
{
    int L = 0;
    for (int j = 0; j < nsets; j++) {
        int c = (w + 1) - (k - S[j].r);
        if (c > 0) L += c;
    }
    return L;
}

/* ---- step 2: enumeration ---- */

/* A codeword of weight wt was generated: rows[0..t-1] with coefs[0..t-1]
 * and row i with coefficient c, in the current matrix. Record it. */
static void record(enum_t *E, int t, int i, int c, int wt)
{
    const int n = E->n, k = E->k;
    memset(E->best_a, 0, (size_t)k);
    for (int u = 0; u < t; u++) E->best_a[E->rows[u]] = (unsigned char)E->coefs[u];
    E->best_a[i] = (unsigned char)c;
    vec_times_matrix(E->F, k, n, E->best_a, E->S->G, E->best_word);
    if (weight(n, E->best_word) != wt || wt == 0) {  /* fast path disagrees, or rank was wrong */
        E->stop = MD_ERROR;
        return;
    }
    E->best = wt;
    E->best_set = E->set;
}

/* Chooses the (t+1)-th row (index >= start) of a combination of E->w rows.
 * partial[t] holds the sum of the t rows chosen so far. */
static void choose(enum_t *E, int t, int start)
{
    const int n = E->n, k = E->k, q = E->q, w = E->w;
    const unsigned char *p = E->partial + (size_t)t * n;
    const unsigned char *mult = E->S->mult;
    const int c_last = (t == 0) ? 1 : q - 1;      /* first coefficient is 1 */

    if (t == w - 1) {
        /* last row: weight of p + c * row_i without storing the sum */
        for (int i = start; i < k; i++) {
            for (int c = 1; c <= c_last; c++) {
                const unsigned char *v = mult + ((size_t)i * q + E->F->neg[c]) * n;
                int wt = 0;
                for (int x = 0; x < n; x++) wt += (p[x] != v[x]);
                E->enumerated++;
                if (wt < E->best) {
                    record(E, t, i, c, wt);
                    if (E->stop) return;
                    if (E->bound >= 0 && wt <= E->bound) { E->stop = MD_EARLY; return; }
                }
            }
            if (E->enumerated >= E->next_check) {
                E->next_check = E->enumerated + CHECK_CLOCK_EVERY;
                if (E->deadline >= 0 && now_seconds() > E->deadline) { E->stop = MD_TIMEOUT; return; }
            }
        }
        return;
    }

    unsigned char *s = E->partial + (size_t)(t + 1) * n;
    for (int i = start; i <= k - (w - t); i++) {   /* leave w - t - 1 rows after i */
        for (int c = 1; c <= c_last; c++) {
            const unsigned char *v = mult + ((size_t)i * q + c) * n;
            for (int x = 0; x < n; x++) s[x] = E->addt[(p[x] << 4) | v[x]];
            E->rows[t] = i;
            E->coefs[t] = c;
            choose(E, t + 1, i + 1);
            if (E->stop) return;
        }
    }
}

/* ---- the whole computation ---- */

void md_result_free(md_result_t *r)
{
    if (!r) return;
    free(r->codeword);
    free(r->message);
    r->codeword = NULL;
    r->message = NULL;
}

int md_min_distance(const gf_t *F, int n, int k, const unsigned char *G,
                    int bound, double max_seconds, md_result_t *out)
{
    const double t0 = now_seconds();
    memset(out, 0, sizeof *out);
    out->status = MD_ERROR;
    out->d = -1;
    out->rank = -1;
    if (!F || !G || n < 1 || k < 1 || F->q < 2 || F->q > 9) return MD_ERROR;
    const int q = F->q;
    for (size_t e = 0; e < (size_t)k * n; e++)
        if (G[e] >= q) return MD_ERROR;

    const int W = n + k;
    infoset_t *S = calloc((size_t)n + 1, sizeof *S);
    unsigned char *used = calloc((size_t)n, 1);
    unsigned char *newpivot = calloc((size_t)n, 1);
    unsigned char *A = malloc((size_t)k * W);
    enum_t E;
    memset(&E, 0, sizeof E);
    E.partial = calloc((size_t)(k + 1) * n, 1);
    E.rows = calloc((size_t)k, sizeof(int));
    E.coefs = calloc((size_t)k, sizeof(int));
    E.best_a = calloc((size_t)k, 1);
    E.best_word = calloc((size_t)n, 1);
    int nsets = 0, status = MD_ERROR;
    if (!S || !used || !newpivot || !A || !E.partial || !E.rows || !E.coefs
        || !E.best_a || !E.best_word)
        goto done;

    /* Step 1: information sets. */
    for (int j = 0; ; j++) {
        int unused_left = 0;
        for (int x = 0; x < n; x++) unused_left += !used[x];
        if (unused_left == 0) break;
        for (int i = 0; i < k; i++) {             /* A = [G | I_k] */
            memcpy(A + (size_t)i * W, G + (size_t)i * n, (size_t)n);
            memset(A + (size_t)i * W + n, 0, (size_t)k);
            A[(size_t)i * W + n + i] = 1;
        }
        memset(newpivot, 0, (size_t)n);
        int r = reduce_unused(F, n, k, A, used, newpivot);
        if (r < 0) goto done;
        if (j == 0) {
            out->rank = r;
            if (r < k) { status = MD_RANK; goto done; }
        }
        if (r == 0) break;
        for (int x = 0; x < n; x++) if (newpivot[x]) used[x] = 1;
        if (4 * r <= k) continue;                 /* dropped: never enumerated, not in L_w */
        infoset_t *s = &S[nsets];
        s->r = r;
        s->G = malloc((size_t)k * n);
        s->T = malloc((size_t)k * k);
        s->mult = malloc((size_t)k * q * n);
        if (!s->G || !s->T || !s->mult) { nsets++; goto done; }
        nsets++;
        for (int i = 0; i < k; i++) {
            memcpy(s->G + (size_t)i * n, A + (size_t)i * W, (size_t)n);
            memcpy(s->T + (size_t)i * k, A + (size_t)i * W + n, (size_t)k);
        }
        for (int i = 0; i < k; i++)
            for (int c = 0; c < q; c++)
                for (int x = 0; x < n; x++)
                    s->mult[((size_t)i * q + c) * n + x] = F->mul[c][s->G[(size_t)i * n + x]];
    }
    out->infosets = nsets;

    /* Step 2 and 3: enumeration by increasing w with the termination rule. */
    E.F = F; E.n = n; E.k = k; E.q = q;
    for (int a = 0; a < 16; a++)
        for (int b = 0; b < 16; b++)
            E.addt[(a << 4) | b] = (a < q && b < q) ? F->add[a][b] : 0;
    E.best = n + 1;
    E.best_set = -1;
    E.bound = bound;
    E.deadline = (max_seconds > 0) ? t0 + max_seconds : -1.0;
    E.next_check = CHECK_CLOCK_EVERY;
    out->last_w = 0;
    out->lower_bound = lower_bound_after(0, k, S, nsets);
    status = -1;
    for (int w = 1; w <= k && status < 0; w++) {
        E.w = w;
        for (int j = 0; j < nsets && !E.stop; j++) {
            E.set = j;
            E.S = &S[j];
            choose(&E, 0, 0);
        }
        if (E.stop) { status = E.stop; break; }
        out->last_w = w;
        out->lower_bound = lower_bound_after(w, k, S, nsets);
        if (out->lower_bound >= E.best || w == k) status = MD_EXACT;
        else if (E.deadline >= 0 && now_seconds() > E.deadline) status = MD_TIMEOUT;
    }

    /* Step 5: the message for G, and the check of the result. */
    if ((status == MD_EXACT || status == MD_EARLY || status == MD_TIMEOUT) && E.best_set >= 0) {
        out->codeword = malloc((size_t)n);
        out->message = malloc((size_t)k);
        if (!out->codeword || !out->message) { status = MD_ERROR; goto done; }
        vec_times_matrix(F, k, k, E.best_a, S[E.best_set].T, out->message);   /* m = a * T_j */
        vec_times_matrix(F, k, n, out->message, G, out->codeword);              /* m * G */
        if (memcmp(out->codeword, E.best_word, (size_t)n) != 0
            || weight(n, out->codeword) != E.best) {
            status = MD_ERROR;
            goto done;
        }
        out->d = E.best;
    } else if (status == MD_EXACT || status == MD_EARLY) {
        status = MD_ERROR;                        /* cannot happen: w = 1 records a row */
    }

done:
    if (status != MD_EXACT && status != MD_EARLY && status != MD_TIMEOUT && status != MD_RANK) {
        status = MD_ERROR;
        md_result_free(out);
        out->d = -1;
    }
    out->status = status;
    out->enumerated = E.enumerated;
    out->seconds = now_seconds() - t0;
    if (S)
        for (int j = 0; j < nsets; j++) { free(S[j].G); free(S[j].T); free(S[j].mult); }
    free(S); free(used); free(newpivot); free(A);
    free(E.partial); free(E.rows); free(E.coefs); free(E.best_a); free(E.best_word);
    return status;
}
