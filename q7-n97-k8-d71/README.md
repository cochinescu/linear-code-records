# A linear [97, 8, 71] code over GF(7)

## Result

This folder holds a generator matrix of a linear [97, 8, 71] code over GF(7). GF(7) is the field of integers modulo 7. Its minimum distance 71 exceeds the lower bound 70 listed for n = 97, k = 8 over GF(7) at codetables.de on 2026-11-11; the upper bound listed there was 77. The table page was read at 2026-11-11T09:58:19Z from http://www.codetables.de/BKLC/BKLC.php?q=7&n=97&k=8.

These sources were checked for an earlier code with these parameters: Chen's online database of quasi-twisted codes, MinT, the codetables.de page for this cell, arXiv, web searches and public GitHub repositories of other projects on linear codes.

## Construction

C0 is the one-generator quasi-cyclic [95, 7, 71] code over GF(7) of index 5 and circulant size 19 with generator polynomials g1(x) = x^16 + x^15 + 5*x^14 + x^13 + 2*x^11 + x^10 + 3*x^9 + 4*x^8 + 5*x^6 + 2*x^5 + x^4 + 5*x^3 + 3*x + 3, g2(x) = x^17 + x^16 + 5*x^15 + 4*x^14 + 5*x^13 + 3*x^12 + x^11 + 4*x^10 + 2*x^9 + 5*x^8 + 2*x^7 + x^5 + 3*x^4 + x^3 + 3*x^2 + x + 2, g3(x) = x^17 + 3*x^16 + 2*x^14 + 3*x^13 + 4*x^12 + 6*x^11 + 6*x^10 + 4*x^9 + 5*x^8 + 6*x^7 + 2*x^6 + 5*x^5 + 3*x^4 + 3*x^3 + 4*x^2 + 4*x + 3, g4(x) = x^17 + 4*x^16 + 2*x^15 + 3*x^14 + 6*x^13 + x^12 + 6*x^11 + 5*x^10 + x^9 + 6*x^8 + 3*x^7 + 3*x^6 + 3*x^5 + 5*x^4 + 3*x^3 + 3*x^2 + 5*x + 4 and g5(x) = x^17 + 6*x^16 + x^15 + 2*x^14 + 4*x^13 + x^12 + 4*x^11 + 5*x^10 + 6*x^9 + 2*x^8 + 6*x^7 + 6*x^6 + 6*x^5 + 4*x^4 + 3*x^3 + 6*x^2 + 2*x + 1. A polynomial of degree below 19 is written as its coefficient vector of length 19, lowest degree first; for i = 0, 1, ..., 6, row i of the generator matrix of C0 consists of the coefficient vectors of x^i*g1(x), ..., x^i*g5(x), each reduced modulo x^19 - 1, side by side.

The vector j of length 95 is constant on each of the 5 blocks of 19 coordinates, with the values 0, 1, 1, 6 and 6 in that order. The code C0 + <j> spanned by C0 and j is a [95, 8] code with minimum distance 69.

The generator matrix of the [97, 8, 71] code has 8 rows: rows 0 to 6 are the rows of C0, each followed by 0, 0, and row 7 is j followed by 1, 1. A nonzero codeword that ends in 0, 0 is a nonzero word of C0 followed by 0, 0 and has weight at least 71; a codeword that does not end in 0, 0 has a nonzero word of C0 + <j> in its first 95 coordinates and weight at least 69 + 2 = 71. This is Construction X applied to C0 inside C0 + <j>, with the repetition code of length 2 as the auxiliary code.

The [97, 8, 70] code listed at codetables.de for n = 97, k = 8 over GF(7) is a quasi-cyclic [96, 8, 70] code of index 4 and circulant size 24 with one coordinate appended. That [96, 8, 70] code has 3528 words of weight 70, and whichever column is appended to its generator matrix, at least one of these words gets a zero in the new coordinate, so no extension of that code by one coordinate has minimum distance 71.

The quasi-cyclic [100, 8, 73] code over GF(7) of Plamen Hristov ("Some new linear codes over small finite fields", Sixth International Workshop on Optimal Codes and Related Topics, Varna, 2009), rebuilt here from the polynomials printed in that paper, has 2100 words of weight 73. Every set of three coordinates lies inside the support of at least one of them, so deleting three coordinates of that code always gives minimum distance 70.

## Verification

The minimum distance was computed by two separate exact programs. The first is mindist (C, the Brouwer-Zimmermann algorithm with several information sets). The second is SageMath 10.10, which computes LinearCode(G).minimum_distance() (sage/checkB.sage). The versions of the compiler, Python, SageMath and GAP are listed in TOOLS-PUBLIC.md. Running make and then python3 verify.py --release in this folder (on Zenodo: in the unpacked zip file) repeats both computations and compares every file listed in MANIFEST.json with its SHA-256 value there. SageMath is not included: it must be installed, and verify.py finds it through the environment variable SAGE_BIN or as sage on the PATH.

## Files

This folder holds the files below. On Zenodo they are in the zip file linear-code-97-8-71-GF7.zip, and README.md, code.txt, code.sage, code.magma, construction.json, MANIFEST.json and LICENSE are also there as single files.

- code.txt: the generator matrix: q, the line modulus none (GF(7) needs no polynomial), n and k, then k rows of n entries, integers from 0 to 6, between the lines matrix and end.
- code.sage: the same matrix as a SageMath script that prints the length, the dimension and the minimum distance.
- code.magma: the same matrix as a Magma script. The Magma script was generated from the verified matrix and was not executed in Magma.
- construction.json: the construction in machine-readable form; its key readme_construction holds the text of the Construction section above.
- verify.py: runs both checks and checks the folder against MANIFEST.json.
- Makefile: builds the C programs in fields/, mindist/ and enum/.
- fields/, mindist/, enum/, sage/: the source of the field arithmetic and of the checks.
- TOOLS-PUBLIC.md: tool versions.
- MANIFEST.json: the parameters, the table bounds with the page and its time, and the SHA-256 of every other file.
- LICENSE: the licence text.

## Citation

Cochinescu, Sebastian (2026). A linear [97, 8, 71] code over GF(7). Zenodo. https://doi.org/10.5281/zenodo.24132260

## Licence

The generator matrices (code.txt, code.sage, code.magma) and the texts in this folder are licensed under the Creative Commons Attribution 4.0 International licence (CC BY 4.0, https://creativecommons.org/licenses/by/4.0/legalcode), and the programs may also be used under the MIT licence, whose full text is in LICENSE.
