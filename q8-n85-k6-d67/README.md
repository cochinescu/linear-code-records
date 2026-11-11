# A linear [85, 6, 67] code over GF(8)

## Result

This folder holds a generator matrix of a linear [85, 6, 67] code over GF(8). The elements of GF(8) are written 0 and w^i, where w is a root of the polynomial x^3+x+1 over GF(2). Its minimum distance 67 exceeds the lower bound 66 listed for n = 85, k = 6 over GF(8) at codetables.de on 2026-11-11; the upper bound listed there was 71. The table page was read at 2026-11-11T09:58:44Z from http://www.codetables.de/BKLC/BKLC.php?q=8&n=85&k=6.

These sources were checked for an earlier code with these parameters: Chen's online database of quasi-twisted codes, MinT, the codetables.de page for this cell, the paper by Maruta, Shinohara and Takenaka cited there, arXiv, web searches and public GitHub repositories of other projects on linear codes.

## Construction

Here w is a root of x^3 + x + 1 over GF(2). The code is a one-generator quasi-cyclic code of index 4 and circulant size 21 over GF(8), with one appended coordinate. The generator polynomials are g1(x) = x^18 + w^3*x^17 + w^2*x^16 + w*x^14 + w*x^13 + w^6*x^12 + w*x^11 + w^2*x^9 + w^4*x^8 + w^6*x^7 + w^6*x^6 + w^4*x^5 + w^3*x^4 + w^6*x^3 + x^2 + w*x + w, g2(x) = x^19 + w^5*x^18 + w^4*x^17 + w^2*x^16 + w^5*x^15 + w^3*x^14 + x^13 + w*x^12 + w^3*x^11 + w^3*x^10 + w*x^8 + w^5*x^7 + w^4*x^6 + x^5 + w^5*x^4 + w^2*x^3 + x^2 + w^3*x + w^2, g3(x) = x^19 + x^18 + w^5*x^17 + x^16 + w*x^15 + w^3*x^14 + w^5*x^13 + w^4*x^12 + w*x^11 + w^3*x^10 + w^5*x^9 + w^6*x^8 + w^4*x^7 + w^4*x^6 + w^5*x^5 + x^4 + w*x^3 + w^3*x^2 + w^2*x + 1 and g4(x) = x^19 + w^3*x^18 + x^17 + w^3*x^16 + w^6*x^15 + w^2*x^14 + w*x^13 + x^12 + w^3*x^11 + w^5*x^10 + w^4*x^9 + w^2*x^8 + w*x^7 + w^3*x^6 + w^3*x^5 + w^5*x^4 + w*x^3 + w^5*x^2 + w^2*x + w^4.

A polynomial of degree below 21 is written as its coefficient vector of length 21, lowest degree first. For i = 0, 1, ..., 5, row i of the generator matrix consists of the coefficient vectors of x^i*g1(x), x^i*g2(x), x^i*g3(x) and x^i*g4(x), each reduced modulo x^21 - 1, followed by the entry 1. The message polynomial a(x) = a_0 + a_1*x + ... + a_5*x^5 therefore gives the codeword whose first 84 entries are the coefficient vectors of a(x)*g1(x), a(x)*g2(x), a(x)*g3(x) and a(x)*g4(x) modulo x^21 - 1 and whose last entry is a(1) = a_0 + a_1 + ... + a_5.

The first 84 coordinates form a quasi-cyclic [84, 6, 66] code with check polynomial h(x) = (x^21 - 1)/gcd(g1(x), g2(x), g3(x), g4(x), x^21 - 1) = x^6 + w^6*x^5 + w^6*x^4 + w^3*x^3 + w^6*x^2 + w^5. It has 4998 words of weight 66 and none of weight 67, and a(1) is nonzero for each of the 4998, so the [85, 6] code has minimum distance 67, with 4998 words of that weight.

The code listed at codetables.de for n = 84, k = 6 over GF(8) on 2026-11-10, with construction type Hristov there, is also a quasi-cyclic [84, 6, 66] code of index 4 and circulant size 21. It has 5145 words of weight 66, and whichever column is appended to its generator matrix, at least one of these words gets a zero in the new coordinate, so no extension of that code by one coordinate has minimum distance 67. The [84, 6, 66] code formed by the first 84 coordinates here has 4998 words of weight 66, so it is not equivalent to that code.

Maruta, Shinohara and Takenaka ("Constructing linear codes from some orbits of projectivities", Discrete Mathematics 308 (2008) 832-841) list an [85, 6, 66] code over GF(8) in Table 6 of their paper; the paper has no [85, 6, 67] code. Rebuilt here from the data printed there, that code has minimum distance 66, with 777 words of weight 66. The [93, 6, 73] code of the same table has 973 words of weight 73, and whichever 8 coordinates are deleted from it, the [85, 6] code that remains has minimum distance below 67. The [95, 6, 75] code listed at codetables.de for n = 95, k = 6 over GF(8) on 2026-11-10, with construction type Hristov there, has 2660 words of weight 75, and whichever 10 coordinates are deleted from it, the [85, 6] code that remains has minimum distance below 67. Both statements were checked by exhaustive searches over the sets of deleted coordinates.

## Verification

The minimum distance was computed by two separate exact programs. The first is mindist (C, the Brouwer-Zimmermann algorithm with several information sets). The second is SageMath 10.10, which computes LinearCode(G).minimum_distance() (sage/checkB.sage). The versions of the compiler, Python, SageMath and GAP are listed in TOOLS-PUBLIC.md. Running make and then python3 verify.py --release in this folder (on Zenodo: in the unpacked zip file) repeats both computations and compares every file listed in MANIFEST.json with its SHA-256 value there. SageMath is not included: it must be installed, and verify.py finds it through the environment variable SAGE_BIN or as sage on the PATH.

## Files

This folder holds the files below. On Zenodo they are in the zip file linear-code-85-6-67-GF8.zip, and README.md, code.txt, code.sage, code.magma, construction.json, MANIFEST.json and LICENSE are also there as single files.

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

Cochinescu, Sebastian (2026). A linear [85, 6, 67] code over GF(8). Zenodo. https://doi.org/10.5281/zenodo.24132262

## Licence

The generator matrices (code.txt, code.sage, code.magma) and the texts in this folder are licensed under the Creative Commons Attribution 4.0 International licence (CC BY 4.0, https://creativecommons.org/licenses/by/4.0/legalcode), and the programs may also be used under the MIT licence, whose full text is in LICENSE.
