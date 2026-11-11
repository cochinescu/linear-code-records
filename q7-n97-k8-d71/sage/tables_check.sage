# Arithmetic table dump check (TOOLS.md 1.6, 2.7).
#   sage tables_check.sage <dumpfile> [<dumpfile> ...]
# Exit 0 only if, in every dump, every element line and every add and mul
# line equals SageMath's field built from the stated modulus under the
# element encoding of TOOLS.md 1.1 (and every pair a, b appears exactly
# once for add and for mul). One line per file: "file=<path> ok ..." or
# "file=<path> FAIL <reason>".
import os
import sys

sys.dont_write_bytecode = True
sys.path.insert(0, os.path.dirname(os.path.abspath(sys.argv[0])))
import codetxt
import sagefield


class DumpError(Exception):
    pass


def check_dump(path):
    with open(path) as fh:
        lines = [(i, l.rstrip("\n")) for i, l in enumerate(fh, 1)]
    lines = [(i, l) for i, l in lines if l != "" and not l.startswith("#")]
    if len(lines) < 3:
        raise DumpError("too short")
    i, l = lines[0]
    p = l.split(" ")
    if len(p) != 2 or p[0] != "q" or p[1] not in ("7", "8", "9"):
        raise DumpError("line %d: expected 'q 7|8|9'" % i)
    q = int(p[1])
    i, l = lines[1]
    p = l.split(" ")
    if len(p) != 2 or p[0] != "modulus" or p[1] != codetxt.MODULI[q]:
        raise DumpError("line %d: modulus for q=%d must be %s" % (i, q, codetxt.MODULI[q]))
    F = sagefield.build_field(q, p[1])
    els = sagefield.elements_by_code(F, q)
    m = F.degree()
    rest = lines[2:]
    pos = 0
    if q != 7:
        for code in range(1, q):
            if pos >= len(rest):
                raise DumpError("missing element line for code %d" % code)
            i, l = rest[pos]
            pos += 1
            p = l.split(" ")
            if len(p) != 3 + m or p[0] != "element":
                raise DumpError("line %d: expected 'element <code> <symbol> <%d coefficients>'" % (i, m))
            if p[1] != str(code):
                raise DumpError("line %d: element lines must list codes 1..%d in order" % (i, q - 1))
            if p[2] != "w^%d" % (code - 1):
                raise DumpError("line %d: code %d must be w^%d, got %s" % (i, code, code - 1, p[2]))
            want = [str(c) for c in sagefield.coefficients(F, els[code])]
            if p[3:] != want:
                raise DumpError("line %d: w^%d has coefficients %s in SageMath, dump says %s"
                                % (i, code - 1, " ".join(want), " ".join(p[3:])))
    seen = {"add": set(), "mul": set()}
    ended = False
    valid = [str(c) for c in range(q)]
    for i, l in rest[pos:]:
        if ended:
            raise DumpError("line %d: text after end" % i)
        if l == "end":
            ended = True
            continue
        p = l.split(" ")
        if len(p) != 4 or p[0] not in ("add", "mul") or any(x not in valid for x in p[1:]):
            raise DumpError("line %d: expected 'add|mul a b c' with codes 0..%d: %r" % (i, q - 1, l))
        a, b, c = int(p[1]), int(p[2]), int(p[3])
        if (a, b) in seen[p[0]]:
            raise DumpError("line %d: %s %d %d appears twice" % (i, p[0], a, b))
        seen[p[0]].add((a, b))
        want = els[a] + els[b] if p[0] == "add" else els[a] * els[b]
        if els[c] != want:
            raise DumpError("line %d: %s %d %d gives %d, SageMath gives %d"
                            % (i, p[0], a, b, c, els.index(want)))
    if not ended:
        raise DumpError("no 'end' line")
    for op in ("add", "mul"):
        if len(seen[op]) != q * q:
            raise DumpError("%s table has %d of %d entries" % (op, len(seen[op]), q * q))
    return q


def main(args):
    if not args:
        print("usage: sage tables_check.sage <dumpfile> [...]", file=sys.stderr)
        return 1
    bad = 0
    for path in args:
        try:
            q = check_dump(path)
            print("file=%s ok q=%d add=%d mul=%d" % (path, q, q * q, q * q))
        except (DumpError, sagefield.SageFieldError, OSError) as e:
            print("file=%s FAIL %s" % (path, e))
            bad += 1
    return 0 if bad == 0 else 1


sys.exit(int(main(sys.argv[1:])))  # int(): a SageMath Integer exit code would print and exit 1
