# A linear [97, 8, 72] code over GF(8)

## Result

This folder holds a generator matrix of a linear [97, 8, 72] code over GF(8). The elements of GF(8) are written 0 and w^i, where w is a root of the polynomial x^3+x+1 over GF(2). Its minimum distance 72 exceeds the lower bound 71 listed for n = 97, k = 8 over GF(8) at codetables.de on 2026-11-10; the upper bound listed there was 79. The table page was read at 2026-11-10T15:06:56Z from http://www.codetables.de/BKLC/BKLC.php?q=8&n=97&k=8.

These sources were checked for an earlier code with these parameters: Chen's online database of quasi-twisted codes, MinT, the references of the codetables.de page, arXiv, web searches and public GitHub repositories of other projects on linear codes.

## Construction

Here w is a root of x^3 + x + 1 over GF(2). C0 is the one-generator quasi-cyclic [95, 7, 72] code over GF(8) of index 5 and circulant size 19 with generator polynomials g1(x) = x^15 + w^5*x^14 + w^6*x^13 + w*x^11 + w^3*x^10 + w^6*x^9 + w^3*x^8 + w^5*x^7 + w^6*x^6 + w*x^5 + w^2*x^4 + x^3 + w^6*x^2 + w*x + w, g2(x) = x^17 + w^5*x^16 + x^15 + w^3*x^14 + w^3*x^13 + w^2*x^12 + w^6*x^11 + w^2*x^10 + x^9 + w^5*x^8 + w^2*x^7 + w^5*x^5 + w*x^3 + w^2*x^2 + w^2*x + w^4, g3(x) = x^17 + w^2*x^16 + w^4*x^14 + w^5*x^13 + w^2*x^12 + w^2*x^11 + w^5*x^10 + w^3*x^9 + w^3*x^8 + w*x^7 + w^2*x^6 + w^4*x^5 + w^3*x^4 + w^6*x^3 + w^6*x^2 + w^3*x + 1, g4(x) = x^17 + w^5*x^15 + w*x^14 + w^2*x^13 + w^2*x^12 + w^6*x^11 + w^6*x^10 + w^4*x^9 + w^4*x^8 + w^4*x^7 + w^2*x^6 + w^6*x^5 + w^4*x^4 + w*x^3 + w^3*x^2 + w^4 and g5(x) = x^18 + w^4*x^17 + w^4*x^16 + w^4*x^15 + w*x^14 + w^3*x^13 + w^5*x^12 + w^6*x^11 + w^3*x^10 + w^2*x^9 + w^2*x^8 + x^7 + w^6*x^6 + w^2*x^5 + w^6*x^4 + w^6*x^3 + w^2*x^2 + w^5*x + w^4. A polynomial of degree below 19 is written as its coefficient vector of length 19, lowest degree first; for i = 0, 1, ..., 6, row i of the generator matrix of C0 consists of the coefficient vectors of x^i*g1(x), ..., x^i*g5(x), each reduced modulo x^19 - 1, side by side.

The vector j of length 95 is constant on each of the 5 blocks of 19 coordinates, with the values 1, 0, w^5, 1 and w^2 in that order. The code C0 + <j> spanned by C0 and j is a [95, 8] code with minimum distance 70.

The generator matrix of the [97, 8, 72] code has 8 rows: rows 0 to 6 are the rows of C0, each followed by 0, 0, and row 7 is j followed by 1, 1. A nonzero codeword that ends in 0, 0 is a nonzero word of C0 followed by 0, 0 and has weight at least 72; a codeword that does not end in 0, 0 has a nonzero word of C0 + <j> in its first 95 coordinates and weight at least 70 + 2 = 72. This is Construction X applied to C0 inside C0 + <j>, with the repetition code of length 2 as the auxiliary code.

Deleting the last coordinate gives a linear [96, 8, 71] code, which exceeds the lower bound 70 listed at codetables.de for n = 96, k = 8 on 2026-11-10. Deleting the last two coordinates gives the [95, 8, 70] code C0 + <j>, which exceeds the lower bound 69 listed at codetables.de for n = 95, k = 8 on 2026-11-10. Deleting the last three coordinates, that is, the last coordinate of C0 + <j>, gives a linear [94, 8, 69] code, which exceeds the lower bound 68 listed at codetables.de for n = 94, k = 8 on 2026-11-10.

## Verification

The minimum distance was computed by two separate exact programs. The first is mindist (C, the Brouwer-Zimmermann algorithm with several information sets). The second is SageMath 10.10, which computes LinearCode(G).minimum_distance() (sage/checkB.sage). The versions of the compiler, Python, SageMath and GAP are listed in TOOLS-PUBLIC.md. Running make and then python3 verify.py --release in this folder (on Zenodo: in the unpacked zip file) repeats both computations and compares every file listed in MANIFEST.json with its SHA-256 value there. SageMath is not included: it must be installed, and verify.py finds it through the environment variable SAGE_BIN or as sage on the PATH.

## Files

This folder holds the files below. On Zenodo they are in the zip file linear-code-97-8-72-GF8.zip, and README.md, code.txt, code.sage, code.magma, construction.json, MANIFEST.json and LICENSE are also there as single files.

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

Cochinescu, Sebastian (2026). A linear [97, 8, 72] code over GF(8). Zenodo. https://doi.org/10.5281/zenodo.24112842

## Licence

The generator matrices (code.txt, code.sage, code.magma) and the texts in this folder are licensed under the Creative Commons Attribution 4.0 International licence (CC BY 4.0, https://creativecommons.org/licenses/by/4.0/legalcode), and the programs may also be used under the MIT licence, whose full text is in LICENSE.
