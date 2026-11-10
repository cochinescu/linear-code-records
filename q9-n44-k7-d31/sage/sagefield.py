"""SageMath side of the element encoding: the field is built by SageMath
from the modulus written in the file, and element codes are mapped to
SageMath elements (code 0 = zero, code e >= 1 = w^(e-1); GF(7): the
residue itself). Imported by the .sage scripts in this directory.
"""
from sage.all import GF, PolynomialRing, ZZ

import codetxt


class SageFieldError(Exception):
    pass


def build_field(q, modulus):
    if q == 7:
        if modulus != "none":
            raise SageFieldError("q=7 takes modulus none")
        return GF(7)
    p = {8: 2, 9: 3}[q]
    P = PolynomialRing(GF(p), "x")
    F = GF(q, name="w", modulus=P(modulus))
    if F.gen().multiplicative_order() != q - 1:
        raise SageFieldError("w is not primitive for modulus %s" % modulus)
    return F


def elements_by_code(F, q):
    """List: element code -> SageMath element."""
    if q == 7:
        return [F(a) for a in range(7)]
    w = F.gen()
    return [F(0)] + [w ** i for i in range(q - 1)]


def code_of(F, q):
    """Dict: SageMath element -> element code."""
    return dict((e, c) for c, e in enumerate(elements_by_code(F, q)))


def coefficients(F, e):
    """Coefficient vector c0 c1 [c2] of e (lowest degree first), as ints."""
    m = F.degree()
    if m == 1:
        return [int(ZZ(e))]
    cs = [int(ZZ(c)) for c in e.polynomial().list()]
    return cs + [0] * (m - len(cs))


def check_element_lines(F, q, elements):
    """The element lines of a code.txt (list of (symbol, coefficients)) against SageMath's field."""
    if q == 7:
        if elements:
            raise SageFieldError("q=7 has no element lines")
        return
    if len(elements) != q - 1:
        raise SageFieldError("expected %d element lines" % (q - 1))
    w = F.gen()
    for i, (sym, cs) in enumerate(elements):
        if sym != "w^%d" % i:
            raise SageFieldError("element line %d names %s" % (i, sym))
        if coefficients(F, w ** i) != list(cs):
            raise SageFieldError("element line %s: file says %s, SageMath says %s"
                                 % (sym, cs, coefficients(F, w ** i)))


def symbol(F, q, e, codes=None):
    if codes is None:
        codes = code_of(F, q)
    c = codes[e]
    if q == 7 or c == 0:
        return str(c)
    return "w^%d" % (c - 1)


def load(path):
    """Read a code.txt with codetxt (plain Python), build the field in
    SageMath, check the element lines against it and return
    (info, F, G) with G the SageMath matrix of the file."""
    from sage.all import matrix
    info = codetxt.read_code(path)
    q = info["q"]
    F = build_field(q, info["modulus"])
    check_element_lines(F, q, info["elements"])
    els = elements_by_code(F, q)
    G = matrix(F, [[els[c] for c in row] for row in info["rows"]])
    return info, F, G
