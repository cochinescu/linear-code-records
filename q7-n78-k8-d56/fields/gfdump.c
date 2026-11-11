/* gfdump <q>: prints the arithmetic tables of GF(q) in the dump format
 *   q <q>
 *   modulus <none | x^3+x+1 | x^2+2*x+2>
 *   element <code> <symbol> <c0> <c1> [<c2>]   for code 1..q-1 (q = 8, 9)
 *   add <a> <b> <a+b>                          for all codes a, b
 *   mul <a> <b> <a*b>                          for all codes a, b
 *   end
 */
#include "gf.h"

#include <stdio.h>
#include <string.h>

int main(int argc, char **argv)
{
    gf_t F;
    int q = 0;
    if (argc == 2) {
        if (strcmp(argv[1], "7") == 0) q = 7;
        else if (strcmp(argv[1], "8") == 0) q = 8;
        else if (strcmp(argv[1], "9") == 0) q = 9;
    }
    if (q == 0 || gf_init(&F, q) != 0) {
        fprintf(stderr, "usage: gfdump <7|8|9>\n");
        return 1;
    }
    printf("q %d\nmodulus %s\n", q, gf_modulus_string(q));
    if (q != 7) {
        char sym[8];
        for (int e = 1; e < q; e++) {
            gf_format_tok(&F, e, sym);
            printf("element %d %s", e, sym);
            for (int i = 0; i < F.m; i++)
                printf(" %d", F.coef[e][i]);
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
    return fflush(stdout) == 0 ? 0 : 1;
}
