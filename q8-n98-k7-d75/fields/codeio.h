/* codeio.h: reader and writer for the canonical generator matrix file
 * (code.txt), and the rank of a matrix over a gf_t field.
 *
 * File format, one item per line (lines starting with '#' and empty lines
 * are ignored, symbols separated by single spaces):
 *   codetxt 1
 *   q <7|8|9>
 *   modulus <none | x^3+x+1 | x^2+2*x+2>
 *   element <symbol> <c0> <c1> [<c2>]   for w^0 .. w^(q-2); absent for q = 7
 *   n <n>
 *   k <k>
 *   matrix
 *   <k lines of n symbols>
 *   end
 */
#ifndef CODEIO_H
#define CODEIO_H

#include "gf.h"

typedef struct { int q, n, k; unsigned char *g; } codefile_t;  /* g: k*n element codes, row-major */

/* 0 ok, 2 invalid (a message is written into err, errlen bytes at most).
 * On success out->g is malloc'd; free it with codeio_free. */
int codeio_read(const char *path, codefile_t *out, char *err, int errlen);

/* Writes the exact format above. 0 ok, nonzero on error. */
int codeio_write(const char *path, const codefile_t *c);

void codeio_free(codefile_t *c);

/* Rank of the k x n matrix g (row-major element codes) by Gaussian
 * elimination; g is not modified. */
int gf_rank(const gf_t *F, int k, int n, const unsigned char *g);

#endif
