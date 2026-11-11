"""Reader for code.txt (generator matrix file) and a small exact field
arithmetic for GF(7), GF(8), GF(9), in plain Python (standard library only).

Element code: for GF(7) the residue 0..6; for GF(8) and GF(9) code 0 is
zero and code e >= 1 is w^(e-1), w a root of the stated modulus.
Text form in code.txt: GF(7) the decimal 0..6; GF(8), GF(9) "0" or
"w^i" with 0 <= i <= q-2, written out in full.

Used by verify.py and by the SageMath scripts next to this file. It does
not call any compiled program.
"""

MODULI = {7: "none", 8: "x^3+x+1", 9: "x^2+2*x+2"}
# modulus coefficients, lowest degree first (monic)
_MODULUS_COEFFS = {8: (2, [1, 1, 0, 1]), 9: (3, [2, 2, 1])}


class CodeFileError(Exception):
    pass


class Field(object):
    """GF(q) for q in 7, 8, 9 on element codes 0..q-1."""

    def __init__(self, q):
        if q not in MODULI:
            raise CodeFileError("q must be 7, 8 or 9, got %r" % (q,))
        self.q = q
        if q == 7:
            self.p, self.m = 7, 1
            self.coef = [(a,) for a in range(7)]
            self._add = [[(a + b) % 7 for b in range(7)] for a in range(7)]
            self._mul = [[(a * b) % 7 for b in range(7)] for a in range(7)]
        else:
            p, mod = _MODULUS_COEFFS[q]
            m = len(mod) - 1
            self.p, self.m = p, m
            powers = []
            v = [1] + [0] * (m - 1)
            for _ in range(q - 1):
                powers.append(tuple(v))
                # multiply by x and reduce with x^m = -(mod[0] + ... + mod[m-1] x^(m-1))
                top = v[m - 1]
                v = [0] + v[:m - 1]
                v = [(v[j] - top * mod[j]) % p for j in range(m)]
            if len(set(powers)) != q - 1 or v != [1] + [0] * (m - 1):
                raise CodeFileError("modulus for q=%d is not primitive" % q)
            self.coef = [tuple([0] * m)] + powers
            index = dict((c, e) for e, c in enumerate(self.coef))
            self._add = [[index[tuple((x + y) % p for x, y in zip(self.coef[a], self.coef[b]))]
                          for b in range(q)] for a in range(q)]
            self._mul = [[0 if (a == 0 or b == 0) else 1 + ((a - 1) + (b - 1)) % (q - 1)
                          for b in range(q)] for a in range(q)]
        self.neg = [self._add[a].index(0) for a in range(q)]
        self.inv = [0] + [self._mul[a].index(1) for a in range(1, q)]

    def add(self, a, b):
        return self._add[a][b]

    def mul(self, a, b):
        return self._mul[a][b]

    def symbol(self, e):
        if self.q == 7 or e == 0:
            return str(e)
        return "w^%d" % (e - 1)

    def parse_symbol(self, s):
        """Element code of the text form s, or None if s is not valid."""
        if self.q == 7:
            if len(s) == 1 and s in "0123456":
                return int(s)
            return None
        if s == "0":
            return 0
        if not s.startswith("w^"):
            return None
        e = s[2:]
        if not e.isdigit() or not e.isascii() or (len(e) > 1 and e[0] == "0"):
            return None
        i = int(e)
        if i > self.q - 2:
            return None
        return i + 1


def _split(line, lineno):
    parts = line.split(" ")
    if any(p == "" for p in parts):
        raise CodeFileError("line %d: items must be separated by single spaces" % lineno)
    return parts


def _int(s, what, lineno):
    if not s.isdigit() or not s.isascii() or (len(s) > 1 and s[0] == "0"):
        raise CodeFileError("line %d: %s is not a decimal integer: %r" % (lineno, what, s))
    return int(s)


def read_code(path):
    """Parse and validate a code.txt file (TOOLS.md 1.2).

    Returns a dict: q, modulus, field (Field), elements (list of
    (symbol, coefficient list) as written), n, k, rows (k lists of n
    element codes). Raises CodeFileError if the file is not valid.
    The rank is not checked here (see rank()).
    """
    with open(path, "rb") as fh:
        raw = fh.read()
    try:
        text = raw.decode("ascii")
    except UnicodeDecodeError:
        raise CodeFileError("file is not plain ASCII")
    lines = []
    for lineno, line in enumerate(text.split("\n"), 1):
        if "\r" in line or "\t" in line:
            raise CodeFileError("line %d: carriage return or tab" % lineno)
        if line == "" or line.startswith("#"):
            continue
        lines.append((lineno, line))
    pos = [0]

    def nxt(what):
        if pos[0] >= len(lines):
            raise CodeFileError("file ends before %s" % what)
        item = lines[pos[0]]
        pos[0] += 1
        return item

    lineno, line = nxt("header")
    if line != "codetxt 1":
        raise CodeFileError("line %d: header must be 'codetxt 1'" % lineno)
    lineno, line = nxt("q")
    parts = _split(line, lineno)
    if len(parts) != 2 or parts[0] != "q" or parts[1] not in ("7", "8", "9"):
        raise CodeFileError("line %d: expected 'q 7|8|9'" % lineno)
    q = int(parts[1])
    F = Field(q)
    lineno, line = nxt("modulus")
    parts = _split(line, lineno)
    if len(parts) != 2 or parts[0] != "modulus":
        raise CodeFileError("line %d: expected 'modulus ...'" % lineno)
    if parts[1] != MODULI[q]:
        raise CodeFileError("line %d: modulus for q=%d must be %s, got %s" % (lineno, q, MODULI[q], parts[1]))
    modulus = parts[1]
    elements = []
    if q != 7:
        for i in range(q - 1):
            lineno, line = nxt("element line w^%d" % i)
            parts = _split(line, lineno)
            if len(parts) != 2 + F.m or parts[0] != "element":
                raise CodeFileError("line %d: expected 'element w^%d' and %d coefficients" % (lineno, i, F.m))
            if parts[1] != "w^%d" % i:
                raise CodeFileError("line %d: element lines must list w^0 .. w^%d in order" % (lineno, q - 2))
            coeffs = []
            for c in parts[2:]:
                if len(c) != 1 or not c.isdigit() or int(c) >= F.p:
                    raise CodeFileError("line %d: coefficient %r is not in 0..%d" % (lineno, c, F.p - 1))
                coeffs.append(int(c))
            if tuple(coeffs) != F.coef[i + 1]:
                raise CodeFileError("line %d: w^%d has coefficients %s, not %s, under modulus %s"
                                    % (lineno, i, " ".join(map(str, F.coef[i + 1])), " ".join(parts[2:]), modulus))
            elements.append((parts[1], coeffs))
    lineno, line = nxt("n")
    parts = _split(line, lineno)
    if len(parts) != 2 or parts[0] != "n":
        raise CodeFileError("line %d: expected 'n <n>'" % lineno)
    n = _int(parts[1], "n", lineno)
    lineno, line = nxt("k")
    parts = _split(line, lineno)
    if len(parts) != 2 or parts[0] != "k":
        raise CodeFileError("line %d: expected 'k <k>'" % lineno)
    k = _int(parts[1], "k", lineno)
    if n < 1 or k < 1:
        raise CodeFileError("n and k must be at least 1")
    lineno, line = nxt("matrix")
    if line != "matrix":
        raise CodeFileError("line %d: expected 'matrix'" % lineno)
    rows = []
    for r in range(k):
        lineno, line = nxt("matrix row %d" % (r + 1))
        parts = _split(line, lineno)
        if len(parts) != n:
            raise CodeFileError("line %d: row %d has %d entries, not n=%d" % (lineno, r + 1, len(parts), n))
        row = []
        for j, s in enumerate(parts):
            e = F.parse_symbol(s)
            if e is None:
                raise CodeFileError("line %d: row %d column %d: %r is not an element of GF(%d)" % (lineno, r + 1, j + 1, s, q))
            row.append(e)
        rows.append(row)
    lineno, line = nxt("end")
    if line != "end":
        raise CodeFileError("line %d: expected 'end' after %d rows" % (lineno, k))
    if pos[0] != len(lines):
        raise CodeFileError("line %d: text after 'end'" % lines[pos[0]][0])
    return {"q": q, "modulus": modulus, "field": F, "elements": elements,
            "n": n, "k": k, "rows": rows}


def write_code(path, q, rows):
    """Write rows (lists of element codes) in the exact format of TOOLS.md 1.2."""
    F = Field(q)
    out = ["codetxt 1", "q %d" % q, "modulus %s" % MODULI[q]]
    if q != 7:
        for i in range(q - 1):
            out.append("element w^%d %s" % (i, " ".join(map(str, F.coef[i + 1]))))
    out.append("n %d" % len(rows[0]))
    out.append("k %d" % len(rows))
    out.append("matrix")
    for row in rows:
        out.append(" ".join(F.symbol(e) for e in row))
    out.append("end")
    with open(path, "w") as fh:
        fh.write("\n".join(out) + "\n")


def _echelon(F, rows):
    """Row-reduced copy; returns (list of nonzero rows, pivot columns)."""
    M = [list(r) for r in rows]
    pivots = []
    r = 0
    ncols = len(M[0]) if M else 0
    for c in range(ncols):
        piv = None
        for i in range(r, len(M)):
            if M[i][c] != 0:
                piv = i
                break
        if piv is None:
            continue
        M[r], M[piv] = M[piv], M[r]
        s = F.inv[M[r][c]]
        M[r] = [F.mul(s, x) for x in M[r]]
        for i in range(len(M)):
            if i != r and M[i][c] != 0:
                f = F.neg[M[i][c]]
                M[i] = [F.add(x, F.mul(f, y)) for x, y in zip(M[i], M[r])]
        pivots.append(c)
        r += 1
        if r == len(M):
            break
    return M[:r], pivots


def rank(F, rows):
    return len(_echelon(F, rows)[0])


def in_row_space(F, rows, vec):
    return rank(F, list(rows) + [list(vec)]) == rank(F, rows)


def mat_vec(F, msg, rows):
    """msg * G for msg (k codes) and G (k rows of n codes)."""
    out = [0] * len(rows[0])
    for a, row in zip(msg, rows):
        if a:
            out = [F.add(x, F.mul(a, y)) for x, y in zip(out, row)]
    return out


def weight(vec):
    return sum(1 for x in vec if x != 0)
