# A linear [44, 7, 31] code over GF(9)

## Result

This folder holds a generator matrix of a linear [44, 7, 31] code over GF(9). The elements of GF(9) are written 0 and w^i, where w is a root of the polynomial x^2+2*x+2 over GF(3). Its minimum distance 31 exceeds the lower bound 30 listed for n = 44, k = 7 over GF(9) at codetables.de on 2026-11-10; the upper bound listed there was 34. The table page was read at 2026-11-10T15:07:21Z from http://www.codetables.de/BKLC/BKLC.php?q=9&n=44&k=7.

These sources were checked for an earlier code with these parameters: Chen's online database of quasi-twisted codes, MinT, the codetables.de page for this cell, arXiv, web searches and public GitHub repositories of other projects on linear codes.

## Construction

Here w is a root of x^2 + 2*x + 2 over GF(3). C0 is the one-generator quasi-cyclic [42, 6, 31] code over GF(9) of index 3 and circulant size 14 with generator polynomials g1(x) = x^12 + w^6*x^11 + w^2*x^9 + w^7*x^8 + w^3*x^7 + w^5*x^6 + w^3*x^5 + w^4*x^4 + x^3 + w^5*x^2 + w^5*x + w^2, g2(x) = x^12 + w^4*x^11 + w^4*x^9 + w*x^8 + w^4*x^7 + w^5*x^6 + w^5*x^5 + w^7*x^4 + w^3*x^3 + w*x^2 + w^6*x + w and g3(x) = x^12 + x^11 + w^6*x^10 + w^4*x^8 + w^4*x^7 + x^6 + w^3*x^5 + w^5*x^4 + w^5*x^3 + w*x^2 + w^6*x + w. A polynomial of degree below 14 is written as its coefficient vector of length 14, lowest degree first; for i = 0, 1, ..., 5, row i of the generator matrix of C0 consists of the coefficient vectors of x^i*g1(x), ..., x^i*g3(x), each reduced modulo x^14 - 1, side by side.

The vector j of length 42 is constant on each of the 3 blocks of 14 coordinates, with the values 1, w^7 and w^6 in that order. The code C0 + <j> spanned by C0 and j is a [42, 7] code with minimum distance 29.

The generator matrix of the [44, 7, 31] code has 7 rows: rows 0 to 5 are the rows of C0, each followed by 0, 0, and row 6 is j followed by 1, 1. A nonzero codeword that ends in 0, 0 is a nonzero word of C0 followed by 0, 0 and has weight at least 31; a codeword that does not end in 0, 0 has a nonzero word of C0 + <j> in its first 42 coordinates and weight at least 29 + 2 = 31. This is Construction X applied to C0 inside C0 + <j>, with the repetition code of length 2 as the auxiliary code.

The code listed at codetables.de for n = 44, k = 7 over GF(9) is of the same kind: Construction X applied to quasi-cyclic [42, 7, 28] and [42, 6, 31] codes with the repetition code of length 3 gives a [45, 7, 31] code, and puncturing it at coordinate 45 gives the listed [44, 7, 30] code. Here the code C0 + <j> that contains the [42, 6, 31] code C0 has minimum distance 29, so a repetition code of length 2 is enough.

A [42, 7, 30] code over GF(9) is public in the file New_linear_codes.txt at https://github.com/Yucong1234/Generator_matrix_of_new_linear_codes , where it has been since 2024-01-20; the lower bound listed at codetables.de for n = 42, k = 7 on 2026-11-10 is 29. The code here does not use it. Appending two coordinates to that code also gives a [44, 7, 31] code, which has 1360 words of weight 31; the code here has 2352, so the two codes are not equivalent.

## Verification

The minimum distance was computed by two separate exact programs. The first is mindist (C, the Brouwer-Zimmermann algorithm with several information sets). The second is SageMath 10.10, which computes LinearCode(G).minimum_distance() (sage/checkB.sage). The versions of the compiler, Python, SageMath and GAP are listed in TOOLS-PUBLIC.md. Running make and then python3 verify.py --release in this folder (on Zenodo: in the unpacked zip file) repeats both computations and compares every file listed in MANIFEST.json with its SHA-256 value there. SageMath is not included: it must be installed, and verify.py finds it through the environment variable SAGE_BIN or as sage on the PATH.

## Files

This folder holds the files below. On Zenodo they are in the zip file linear-code-44-7-31-GF9.zip, and README.md, code.txt, code.sage, code.magma, construction.json, MANIFEST.json and LICENSE are also there as single files.

- code.txt: the generator matrix: q, the polynomial, the coefficient vector of each power of w, n and k, then k rows of n entries between the lines matrix and end.
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

Cochinescu, Sebastian (2026). A linear [44, 7, 31] code over GF(9). Zenodo. https://doi.org/10.5281/zenodo.24112844

## Licence

The generator matrices (code.txt, code.sage, code.magma) and the texts in this folder are licensed under the Creative Commons Attribution 4.0 International licence (CC BY 4.0, https://creativecommons.org/licenses/by/4.0/legalcode), and the programs may also be used under the MIT licence, whose full text is in LICENSE.
