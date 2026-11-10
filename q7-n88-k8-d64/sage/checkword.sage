# Is a word in the code, with weight d? (TOOLS.md 2.7)
#   sage checkword.sage <code.txt> <d> <n entries in the text form of code.txt>
# Exit 0 only if the word has exactly n valid entries, lies in the row
# space of G over SageMath's field and has Hamming weight d.
import os
import sys

sys.dont_write_bytecode = True
sys.path.insert(0, os.path.dirname(os.path.abspath(sys.argv[0])))
import codetxt
import sagefield


def main(args):
    if len(args) < 2:
        print("usage: sage checkword.sage <code.txt> <d> <entries...>", file=sys.stderr)
        return 1
    try:
        info, F, G = sagefield.load(args[0])
    except (codetxt.CodeFileError, sagefield.SageFieldError) as e:
        print("FAIL invalid code file: %s" % e)
        return 2
    q, n = info["q"], info["n"]
    if not args[1].isdigit():
        print("FAIL d is not a non-negative integer: %r" % args[1])
        return 2
    d = int(args[1])
    word = args[2:]
    if len(word) != n:
        print("FAIL the word has %d entries, n=%d" % (len(word), n))
        return 2
    els = sagefield.elements_by_code(F, q)
    codes = []
    for j, s in enumerate(word):
        c = info["field"].parse_symbol(s)
        if c is None:
            print("FAIL entry %d: %r is not an element of GF(%d)" % (j + 1, s, q))
            return 2
        codes.append(c)
    v = vector(F, [els[c] for c in codes])
    wt = v.hamming_weight()
    inside = v in G.row_space()
    print("weight=%d d=%d in_code=%s" % (wt, d, "yes" if inside else "no"))
    if not inside:
        print("FAIL the word is not in the code")
        return 1
    if wt != d:
        print("FAIL the word has weight %d, not %d" % (wt, d))
        return 1
    print("ok")
    return 0


sys.exit(int(main(sys.argv[1:])))  # int(): a SageMath Integer exit code would print and exit 1
