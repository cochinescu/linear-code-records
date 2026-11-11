# Check B: minimum distance by SageMath.
#   sage checkB.sage <code.txt> [<code.txt> ...]
# For each file: read it in plain Python (codetxt.py), build the field in
# SageMath from the modulus stated in the file, check the element lines
# against that field, map the entries, compute the rank and the minimum
# distance of LinearCode(G). One block per file:
#   file=<path>  q= n= k= rank= status=exact d= codeword=  end
# (status=invalid and error=<reason> for a file that is not valid).
# Exit 0 if every file was processed, 2 if a file was invalid, 1 otherwise.
#
# The minimum distance: in SageMath 10.10, LinearCode.minimum_distance()
# returns self._minimum_weight_codeword().hamming_weight() for q = 7, 8, 9
# (sage/coding/linear_code.py). This script calls _minimum_weight_codeword()
# once and takes its Hamming weight, so it prints the same d and also the
# codeword; if a SageMath version lacks that method, minimum_distance() is
# used and no codeword is printed.
import os
import sys
import time

sys.dont_write_bytecode = True
sys.path.insert(0, os.path.dirname(os.path.abspath(sys.argv[0])))
import codetxt
import sagefield


def check_one(path):
    print("file=%s" % path)
    t0 = time.time()
    try:
        info, F, G = sagefield.load(path)
    except (codetxt.CodeFileError, sagefield.SageFieldError) as e:
        print("status=invalid")
        print("error=%s" % str(e).replace("\n", " "))
        print("end")
        return 2
    q, n, k = info["q"], info["n"], info["k"]
    r = G.rank()
    print("q=%d" % q)
    print("n=%d" % n)
    print("k=%d" % k)
    print("rank=%d" % r)
    codes = sagefield.code_of(F, q)
    if r == 0:
        print("status=exact")
        print("d=0")
        print("end")
        return 0
    C = LinearCode(G)
    if hasattr(C, "_minimum_weight_codeword"):
        cw = C._minimum_weight_codeword()
        d = cw.hamming_weight()
        if cw.is_zero() or cw not in C:
            print("status=error")
            print("error=SageMath returned a codeword that is zero or not in the code")
            print("end")
            return 1
    else:
        cw = None
        d = C.minimum_distance()
    print("status=exact")
    print("d=%d" % d)
    if cw is not None:
        print("codeword=%s" % " ".join(sagefield.symbol(F, q, x, codes) for x in cw))
    print("# seconds=%.3f" % (time.time() - t0))
    print("end")
    return 0


def main(args):
    if not args:
        print("usage: sage checkB.sage <code.txt> [<code.txt> ...]", file=sys.stderr)
        return 1
    import sage.version
    print("# sage_version=%s gap_version=%s" % (sage.version.version, libgap.eval("GAPInfo.Version")))
    worst = 0
    for path in args:
        try:
            rc = check_one(path)
        except Exception as e:
            print("status=error")
            print("error=%s: %s" % (type(e).__name__, str(e).replace("\n", " ")))
            print("end")
            rc = 1
        sys.stdout.flush()
        if rc == 1 or (rc == 2 and worst == 0):
            worst = rc
    return worst


sys.exit(int(main(sys.argv[1:])))  # int(): a SageMath Integer exit code would print and exit 1
