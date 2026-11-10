# Linear codes over GF(7) and GF(8)

This repository holds generator matrices of three linear codes, each in its own folder with a program that recomputes its minimum distance.

- q7-n88-k8-d64: a linear [88, 8, 64] code over GF(7); the lower bound listed at codetables.de on 2026-11-10 is 63. https://doi.org/10.5281/zenodo.24098667
- q7-n90-k8-d65: a linear [90, 8, 65] code over GF(7); the lower bound listed at codetables.de on 2026-11-10 is 64. https://doi.org/10.5281/zenodo.24098671
- q8-n98-k7-d75: a linear [98, 7, 75] code over GF(8), with w a root of x^3 + x + 1; the lower bound listed at codetables.de on 2026-11-10 is 74. https://doi.org/10.5281/zenodo.24098673

The README.md of each folder gives the construction, the bounds listed at codetables.de with the time the table was read, and the checks.

## Verification

In a folder, run make and then python3 verify.py --release. This repeats the two exact computations of the minimum distance, by mindist and by SageMath, and compares every file listed in MANIFEST.json with its SHA-256 value there. SageMath is not included: it must be installed, and verify.py finds it through the environment variable SAGE_BIN or as sage on the PATH.

## Licence

The generator matrices and texts in this repository are licensed under the Creative Commons Attribution 4.0 International licence (CC BY 4.0, https://creativecommons.org/licenses/by/4.0/legalcode), and the programs may also be used under the MIT licence, whose full text is in the LICENSE file of each folder.

## Citation

Each code is cited by its Zenodo record:

Cochinescu, Sebastian (2026). A linear [88, 8, 64] code over GF(7). Zenodo. https://doi.org/10.5281/zenodo.24098667

Cochinescu, Sebastian (2026). A linear [90, 8, 65] code over GF(7). Zenodo. https://doi.org/10.5281/zenodo.24098671

Cochinescu, Sebastian (2026). A linear [98, 7, 75] code over GF(8). Zenodo. https://doi.org/10.5281/zenodo.24098673
