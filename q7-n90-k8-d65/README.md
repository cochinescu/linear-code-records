# A linear [90, 8, 65] code over GF(7)

## Result

This folder holds a generator matrix of a linear [90, 8, 65] code over GF(7). GF(7) is the field of integers modulo 7. Its minimum distance 65 exceeds the lower bound 64 listed for n = 90, k = 8 over GF(7) at codetables.de on 2026-11-10; the upper bound listed there was 71. The table page was read at 2026-11-10T06:46:39Z from http://www.codetables.de/BKLC/BKLC.php?q=7&n=90&k=8.

These sources were checked for an earlier code with these parameters: Chen's online database of quasi-twisted codes, MinT, the codetables.de page for this cell, arXiv, web searches and the GitHub repository of the Quantavane benchmark.

## Construction

The code is the linear [88, 8, 64] code over GF(7) of https://doi.org/10.5281/zenodo.24098667 with two further appended coordinates. That [88, 8, 64] code is a one-generator quasi-cyclic code of index 3 and circulant size 29 with one appended coordinate; its generator polynomials are g1(x) = x^26 + 4*x^25 + 5*x^24 + 5*x^23 + 6*x^22 + 5*x^21 + 6*x^20 + 4*x^19 + x^18 + 4*x^17 + 4*x^16 + 4*x^15 + 3*x^14 + 5*x^13 + x^12 + 2*x^10 + x^9 + x^8 + 2*x^7 + x^6 + 3*x^5 + 2*x^4 + x^3 + x^2 + x + 5, g2(x) = x^27 + 2*x^26 + x^25 + x^24 + 3*x^23 + x^22 + 2*x^21 + 6*x^20 + 5*x^19 + 3*x^17 + 2*x^16 + 4*x^14 + 3*x^13 + 6*x^12 + 5*x^11 + x^10 + 6*x^8 + 3*x^7 + 3*x^6 + 3*x^5 + 2*x^4 + 4*x^3 + 3*x^2 + 2*x + 2 and g3(x) = x^27 + 4*x^26 + x^25 + 3*x^24 + 4*x^23 + 5*x^22 + 2*x^21 + 2*x^20 + 3*x^19 + 2*x^18 + 2*x^17 + 3*x^16 + x^15 + 3*x^13 + 3*x^12 + 6*x^11 + 2*x^10 + x^9 + 2*x^8 + 3*x^7 + 2*x^6 + 5*x^5 + 4*x^4 + x^3 + 6*x^2 + 2.

A polynomial of degree below 29 is written as its coefficient vector of length 29, lowest degree first. For i = 0, 1, ..., 7, the first 87 entries of row i of the generator matrix are the coefficient vectors of x^i*g1(x), x^i*g2(x) and x^i*g3(x), each reduced modulo x^29 - 1, side by side, and entry 88 is 1; for the message polynomial a(x) = a_0 + a_1*x + ... + a_7*x^7, entry 88 of the codeword is a(1) = a_0 + a_1 + ... + a_7.

The two appended columns, with their entries listed for rows 0 to 7, are (1, 1, 5, 5, 6, 1, 2, 6) and (0, 1, 4, 3, 1, 6, 3, 3); they are coordinates 89 and 90.

The columns form an (l, s)-extension in the sense of A. Kohnert, "(l,s)-Extension of Linear Codes", arXiv:cs/0701112, with l = 2 and s = 1: every word of weight 64 of the [88, 8, 64] code has at least one nonzero entry in the two new coordinates, and the [90, 8] code has minimum distance 65.

The [87, 8, 63] code formed by the first 87 coordinates has the same circulant size, index and check polynomial h(x) = (x^29 - 1)/gcd(g1(x), g2(x), g3(x), x^29 - 1) = x^8 + 5*x^7 + 2*x^6 + x^5 + 5*x^4 + 2*x^3 + 3*x^2 + x + 1 as the [87, 8, 63] code listed in Chen's database of quasi-twisted codes and credited there to Arhan Canli, and it is a different code: the two row spaces meet only in the zero vector.

## Verification

The minimum distance was computed by two separate exact programs. The first is mindist (C, the Brouwer-Zimmermann algorithm with several information sets). The second is SageMath 10.10, which computes LinearCode(G).minimum_distance() (sage/checkB.sage). The versions of the compiler, Python, SageMath and GAP are listed in TOOLS-PUBLIC.md. Running make and then python3 verify.py --release in this folder (on Zenodo: in the unpacked zip file) repeats both computations and compares every file listed in MANIFEST.json with its SHA-256 value there. SageMath is not included: it must be installed, and verify.py finds it through the environment variable SAGE_BIN or as sage on the PATH.

## Files

This folder holds the files below. On Zenodo they are in the zip file linear-code-90-8-65-GF7.zip, and README.md, code.txt, code.sage, code.magma, construction.json, MANIFEST.json and LICENSE are also there as single files.

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

Cochinescu, Sebastian (2026). A linear [90, 8, 65] code over GF(7). Zenodo. https://doi.org/10.5281/zenodo.24098671

## Licence

The generator matrices (code.txt, code.sage, code.magma) and the texts in this folder are licensed under the Creative Commons Attribution 4.0 International licence (CC BY 4.0, https://creativecommons.org/licenses/by/4.0/legalcode), and the programs may also be used under the MIT licence, whose full text is in LICENSE.
