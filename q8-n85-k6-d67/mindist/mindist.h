/* mindist.h: exact minimum distance of a linear code over GF(7), GF(8) or
 * GF(9) by the Brouwer-Zimmermann algorithm (see the comment at the top of
 * mindist.c for the algorithm and its termination rule).
 */
#ifndef MINDIST_H
#define MINDIST_H

#include "../fields/gf.h"

#define MD_EXACT 0
#define MD_EARLY 10
#define MD_RANK 3
#define MD_TIMEOUT 4
#define MD_ERROR 1      /* bad arguments, out of memory, or a failed internal check */

typedef struct {
    int status, d, rank;
    unsigned char *codeword;      /* n element codes, malloc'd by the callee, may be NULL on MD_RANK */
    unsigned char *message;       /* k element codes */
    unsigned long long enumerated;/* codewords generated */
    int infosets;                 /* number of information sets used */
    double seconds;
    /* Additions to the agreed structure (statistics only): */
    int lower_bound;              /* every codeword not generated has weight >= lower_bound */
    int last_w;                   /* last row count w that was enumerated completely */
} md_result_t;

/* G: k x n element codes, row-major; it is not modified.
 * bound < 0: exact computation. bound >= 0: return MD_EARLY at the first
 * codeword of weight <= bound. max_seconds <= 0: no time limit.
 * Returns out->status (MD_EXACT, MD_EARLY, MD_RANK, MD_TIMEOUT or MD_ERROR).
 * On MD_EXACT and MD_EARLY, codeword and message are set, message*G ==
 * codeword and the weight of codeword is d (both checked with plain field
 * arithmetic before returning). On MD_TIMEOUT, d is the smallest weight
 * found so far (an upper bound on the minimum distance) with its codeword
 * and message, or d = -1 and NULL pointers if no codeword was generated. */
int md_min_distance(const gf_t *F, int n, int k, const unsigned char *G,
                    int bound, double max_seconds, md_result_t *out);
void md_result_free(md_result_t *r);

#endif
