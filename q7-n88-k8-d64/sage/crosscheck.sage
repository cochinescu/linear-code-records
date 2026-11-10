# Cross-check of our codeword arithmetic against SageMath (TOOLS.md 2.7).
#   sage crosscheck.sage <code.txt> <gfencode-output>
# Every line of the second file is "m_1 .. m_k ; c_1 .. c_n" (element
# codes). Exit 0 only if every line satisfies m*G = c over SageMath's field
# built from the modulus in code.txt, and the file has at least one line.
# Prints rank=<rank of G in SageMath>, lines=<count>, and the first bad line.
import os
import sys

sys.dont_write_bytecode = True
sys.path.insert(0, os.path.dirname(os.path.abspath(sys.argv[0])))
import codetxt
import sagefield

CHUNK = 5000


def main(args):
    if len(args) != 2:
        print("usage: sage crosscheck.sage <code.txt> <gfencode-output>", file=sys.stderr)
        return 1
    try:
        info, F, G = sagefield.load(args[0])
    except (codetxt.CodeFileError, sagefield.SageFieldError) as e:
        print("FAIL invalid code file: %s" % e)
        return 2
    q, n, k = info["q"], info["n"], info["k"]
    print("rank=%d" % G.rank())
    els = sagefield.elements_by_code(F, q)
    valid = dict((str(c), els[c]) for c in range(q))
    count = 0
    msgs, words, linenos = [], [], []

    def flush():
        if not msgs:
            return None
        M = matrix(F, msgs)
        W = matrix(F, words)
        P = M * G
        if P == W:
            return None
        for r in range(P.nrows()):
            if P[r] != W[r]:
                return linenos[r]
        return linenos[0]

    with open(args[1]) as fh:
        for lineno, line in enumerate(fh, 1):
            line = line.rstrip("\n")
            if line == "" or line.startswith("#"):
                continue
            parts = line.split(" ; ")
            if len(parts) != 2:
                print("FAIL line %d: expected 'm ; c'" % lineno)
                return 1
            m = parts[0].split(" ")
            c = parts[1].split(" ")
            if len(m) != k or len(c) != n:
                print("FAIL line %d: %d message and %d codeword entries, expected %d and %d" % (lineno, len(m), len(c), k, n))
                return 1
            try:
                msgs.append([valid[x] for x in m])
                words.append([valid[x] for x in c])
            except KeyError as e:
                print("FAIL line %d: %s is not an element code of GF(%d)" % (lineno, e, q))
                return 1
            linenos.append(lineno)
            count += 1
            if len(msgs) == CHUNK:
                bad = flush()
                if bad is not None:
                    print("FAIL line %d: m*G differs from c in SageMath" % bad)
                    return 1
                msgs, words, linenos = [], [], []
    bad = flush()
    if bad is not None:
        print("FAIL line %d: m*G differs from c in SageMath" % bad)
        return 1
    print("lines=%d" % count)
    if count == 0:
        print("FAIL no lines")
        return 1
    print("ok")
    return 0


sys.exit(int(main(sys.argv[1:])))  # int(): a SageMath Integer exit code would print and exit 1
