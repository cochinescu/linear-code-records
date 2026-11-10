/* gf.h: arithmetic in GF(7), GF(8) and GF(9) by lookup tables.
 *
 * Element codes: for GF(7) the residue 0..6. For GF(8) and GF(9) code 0 is
 * zero and code e >= 1 is w^(e-1), where w is a root of the defining
 * polynomial (GF(8): x^3 + x + 1 over GF(2); GF(9): x^2 + 2*x + 2 over
 * GF(3)). Code 1 is always the element 1.
 *
 * Text symbols: for GF(7) the decimal digit 0..6. For GF(8) and GF(9) "0"
 * or "w^i" with 0 <= i <= q-2. No other spelling is accepted.
 *
 * The tables are computed by gf_init from the defining polynomial
 * (polynomial arithmetic over the prime field, reduction by the
 * polynomial); none of them is typed in.
 */
#ifndef GF_H
#define GF_H

typedef struct {
    int q;                    /* 7, 8 or 9 */
    int p, m;                 /* characteristic and degree */
    unsigned char add[9][9], mul[9][9], neg[9], inv[9];   /* element codes; inv[0] = 0 */
    unsigned char coef[9][3]; /* coefficient vector c0 c1 c2 of each code */
} gf_t;

/* 0 on success, -1 if q is not 7, 8 or 9 (or the tables fail their
 * internal checks). */
int gf_init(gf_t *F, int q);

/* Element code of a text symbol, or -1 if the symbol is not valid for F. */
int gf_parse_tok(const gf_t *F, const char *tok);

/* Writes the text symbol of an element code into buf (at least 8 bytes).
 * An invalid code gives the string "?". */
void gf_format_tok(const gf_t *F, int code, char *buf);

/* "none", "x^3+x+1", "x^2+2*x+2" for q = 7, 8, 9; NULL for any other q. */
const char *gf_modulus_string(int q);

#endif
