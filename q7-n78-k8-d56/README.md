# A linear [78, 8, 56] code over GF(7)

## Result

This folder holds a generator matrix of a linear [78, 8, 56] code over GF(7). GF(7) is the field of integers modulo 7. The code's minimum distance 56 exceeds the lower bound 55 listed for n = 78, k = 8 over GF(7) at codetables.de on 2026-11-11; the upper bound listed there was 61. The table page was read at 2026-11-11T12:05:16Z from http://www.codetables.de/BKLC/BKLC.php?q=7&n=78&k=8.

These sources were checked for an earlier code with these parameters: Chen's online database of quasi-twisted codes, MinT, the codetables.de page for this cell, a 2001 paper by Daskalov and Gulliver, arXiv and web searches.

## Construction

C0 is the one-generator quasi-cyclic [76, 7, 56] code over GF(7) of index 2 and circulant size 38 with generator polynomials g1(x) = x^31 + 4*x^30 + 5*x^29 + 5*x^28 + 2*x^27 + 6*x^26 + 3*x^25 + 5*x^24 + 2*x^23 + x^21 + 6*x^20 + 5*x^19 + 2*x^18 + 2*x^17 + 2*x^16 + 5*x^15 + 5*x^14 + 6*x^13 + x^12 + 3*x^11 + 4*x^10 + 4*x^9 + x^8 + 6*x^7 + 2*x^5 + 4*x^4 + 3*x^3 + x^2 + x + 1 and g2(x) = x^36 + 5*x^35 + x^34 + 6*x^33 + 2*x^32 + 4*x^31 + 4*x^30 + 2*x^28 + 6*x^27 + 5*x^26 + 3*x^25 + x^24 + 3*x^23 + 6*x^22 + 3*x^21 + 4*x^20 + 6*x^19 + x^18 + 4*x^17 + 4*x^15 + 3*x^14 + 4*x^12 + x^11 + 3*x^9 + 6*x^8 + 6*x^7 + 6*x^6 + 3*x^5 + 4*x^4 + 2*x^3 + x^2 + x + 1. A polynomial of degree below 38 is written as its coefficient vector of length 38, lowest degree first; for i = 0, 1, ..., 6, row i of the generator matrix of C0 consists of the coefficient vectors of x^i*g1(x) and x^i*g2(x), each reduced modulo x^38 - 1, side by side.

The vector j is the all-ones vector of length 76. The code C0 + <j> spanned by C0 and j is a [76, 8] code with minimum distance 54.

The generator matrix of the [78, 8, 56] code has 8 rows: rows 0 to 6 are the rows of C0, each followed by 0, 0, and row 7 is j followed by 1, 1. A nonzero codeword that ends in 0, 0 is a nonzero word of C0 followed by 0, 0 and has weight at least 56; a codeword that does not end in 0, 0 has a nonzero word of C0 + <j> in its first 76 coordinates and weight at least 54 + 2 = 56. This is Construction X applied to C0 inside C0 + <j>, with the repetition code of length 2 as the auxiliary code.

The [78, 8, 55] code listed at codetables.de for n = 78, k = 8 over GF(7) is a quasi-cyclic [80, 8, 57] code of index 5 and circulant size 16 with two coordinates deleted. That [80, 8, 57] code has 2208 words of weight 57. Every pair of coordinates lies inside the support of at least one of them, so deleting two coordinates of that code always gives minimum distance 55.

The quasi-cyclic [80, 8, 56] code over GF(7) of R. N. Daskalov and T. A. Gulliver ("New Minimum Distance Bounds for Linear Codes over Small Fields", Problemy Peredachi Informatsii, vol. 37, no. 3, 2001), rebuilt here from the digits printed in that paper, has 600 words of weight 56. Every pair of coordinates lies inside the support of at least one of them, so deleting two coordinates of that code always gives minimum distance 54.

Deleting one or two coordinates from the listed [78, 8, 55] code and from the code of this record never gives two codes with the same weight distribution (all 78 coordinates and all 3003 pairs on each side were compared). So no code obtained by deleting one or two coordinates of this code is equivalent to a code obtained by deleting the same number of coordinates of the listed [78, 8, 55] code.

## Verification

The minimum distance was computed by two separate exact programs. The first is mindist (C, the Brouwer-Zimmermann algorithm with several information sets). The second is SageMath 10.10, which computes LinearCode(G).minimum_distance() (sage/checkB.sage). The versions of the compiler, Python, SageMath and GAP are listed in TOOLS-PUBLIC.md. Running make and then python3 verify.py --release in this folder (on Zenodo: in the unpacked zip file) repeats both computations and compares every file listed in MANIFEST.json with its SHA-256 value there. SageMath is not included: it must be installed, and verify.py finds it through the environment variable SAGE_BIN or as sage on the PATH.

## Files

This folder holds the files below. On Zenodo they are in the zip file linear-code-78-8-56-GF7.zip, and README.md, code.txt, code.sage, code.magma, construction.json, MANIFEST.json and LICENSE are also there as single files.

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

Cochinescu, Sebastian (2026). A linear [78, 8, 56] code over GF(7). Zenodo. https://doi.org/10.5281/zenodo.24136306

## Licence

The generator matrices (code.txt, code.sage, code.magma) and the texts in this folder are licensed under the Creative Commons Attribution 4.0 International licence (CC BY 4.0, https://creativecommons.org/licenses/by/4.0/legalcode), and the programs may also be used under the MIT licence, whose full text is in LICENSE.
