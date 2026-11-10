#!/usr/bin/env python3
"""verify.py: check the minimum distance of a linear code over GF(7), GF(8)
or GF(9) with two independent exact computations.

  verify.py <code.txt> [--claim-d D] [--b sage|enum] [--b-timeout SEC] [--a-timeout SEC]
  verify.py --release [DIR]

Check A is mindist/mindist (Brouwer-Zimmermann, C). Check B is SageMath:
sage/checkB.sage computes LinearCode(G).minimum_distance(). With --b enum,
check B is replaced by enum/enumd (plain enumeration, C, written separately
from check A) plus cross-checks in SageMath: the rank of G, the complete
addition and multiplication tables of both C programs, 100000 random
codewords made by our arithmetic, and the minimum-weight codeword.

Everything is found relative to this file; missing C programs are built
with make -C. SageMath is found through the environment variable SAGE_BIN,
else as `sage` on the PATH.

Exit codes: 0 all good; 2 invalid entries or format; 3 rank is not k;
4 the two checks disagree; 5 n, k or d differ from the claim or the
manifest; 6 a check did not finish; 7 a manifest hash does not match;
1 other error.
"""
import argparse
import hashlib
import json
import os
import shutil
import subprocess
import sys
import tempfile
import time

sys.dont_write_bytecode = True
HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, "sage"))
import codetxt  # noqa: E402

ENCODE_COUNT = 100000
ENCODE_SEED = 12345


class Fatal(Exception):
    def __init__(self, code, msg):
        Exception.__init__(self, msg)
        self.code = code


def say(line):
    print(line)
    sys.stdout.flush()


def run(cmd, timeout=None, env=None):
    """Run cmd (in our process group, so a supervisor that stops us stops
    it too); returns (rc, out, err, timed_out, seconds)."""
    t0 = time.time()
    p = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE,
                         env=env, universal_newlines=True)
    try:
        out, err = p.communicate(timeout=timeout)
        return p.returncode, out, err, False, time.time() - t0
    except subprocess.TimeoutExpired:
        out, err = "", ""
        for stop in (p.terminate, p.kill):
            try:
                stop()
            except OSError:
                pass
            try:
                out, err = p.communicate(timeout=10)
                break
            except subprocess.TimeoutExpired:
                pass
        return None, out or "", err or "", True, time.time() - t0


def keyvals(text):
    """key=value lines (not starting with #) of a checker's output."""
    d = {}
    for line in text.splitlines():
        if line.startswith("#") or "=" not in line:
            continue
        k, v = line.split("=", 1)
        d[k] = v
    return d


def program(path, default_rel, make_dirs):
    """The binary given on the command line, else the default next to this
    file, built with make -C <dir> for each of make_dirs (in order) if it is
    missing. mindist needs fields/libgf.a, so fields is built first."""
    if path:
        if not os.path.isfile(path):
            raise Fatal(1, "program not found: %s" % path)
        return path
    p = os.path.join(HERE, default_rel)
    if not os.path.isfile(p):
        make = shutil.which("make")
        if make is None:
            raise Fatal(1, "%s is missing and make is not on the PATH" % default_rel)
        for make_dir in make_dirs:
            say("build=make -C %s" % os.path.relpath(make_dir, HERE))
            rc, out, err, _, _ = run([make, "-C", make_dir])
            if rc != 0:
                sys.stderr.write(out + err)
                raise Fatal(1, "could not build %s (make -C %s failed)" % (default_rel, os.path.relpath(make_dir, HERE)))
        if not os.path.isfile(p):
            raise Fatal(1, "could not build %s" % default_rel)
    return p


def sage_command():
    sb = os.environ.get("SAGE_BIN") or shutil.which("sage")
    if not sb:
        raise Fatal(1, "SageMath not found: set SAGE_BIN or put sage on the PATH")
    env = dict(os.environ)
    env["PATH"] = os.path.dirname(os.path.abspath(sb)) + os.pathsep + env.get("PATH", "")
    return sb, env


def check_codeword(F, info, name, res, need_message):
    """The codeword a check printed: weight d, in the code (message * G when
    a message is given). Returns an error string or None."""
    cw_s = res.get("codeword")
    if cw_s is None:
        return None if not need_message else "%s printed no codeword" % name
    cw = [F.parse_symbol(s) for s in cw_s.split(" ")] if cw_s else []
    if len(cw) != info["n"] or None in cw:
        return "%s printed an invalid codeword" % name
    if codetxt.weight(cw) != res["d"]:
        return "%s printed a codeword of weight %d with d=%d" % (name, codetxt.weight(cw), res["d"])
    if need_message:
        m_s = res.get("message")
        msg = [F.parse_symbol(s) for s in m_s.split(" ")] if m_s else []
        if len(msg) != info["k"] or None in msg:
            return "%s printed an invalid message" % name
        if codetxt.mat_vec(F, msg, info["rows"]) != cw:
            return "%s: message * G is not the codeword it printed" % name
    elif not codetxt.in_row_space(F, info["rows"], cw):
        return "%s printed a codeword that is not in the code" % name
    return None


def parse_checker(name, rc, out, timed_out, secs, info):
    """Output of mindist / enumd (TOOLS.md 1.3) -> result dict."""
    r = {"name": name, "rc": rc, "seconds": secs, "finished": False}
    kv = keyvals(out)
    r.update(kv)
    if timed_out or rc == 4:
        r["state"] = "timeout"
        return r
    if rc == 2:
        r["state"] = "invalid"
        return r
    if rc == 3:
        r["state"] = "rank"
        r["finished"] = True
        return r
    if rc != 0 or kv.get("status") != "exact":
        r["state"] = "error"
        return r
    try:
        r["d"] = int(kv["d"])
        r["rank_i"] = int(kv["rank"])
        for key in ("q", "n", "k"):
            if int(kv[key]) != info[key]:
                r["state"] = "error"
                r["why"] = "%s=%s, the file says %d" % (key, kv[key], info[key])
                return r
    except (KeyError, ValueError):
        r["state"] = "error"
        r["why"] = "output lacks q, n, k, rank or d"
        return r
    r["state"] = "exact"
    r["finished"] = True
    return r


def line_for(label, r):
    parts = ["%s=%s" % (label, r["name"]), "state=%s" % r.get("state")]
    for key in ("d", "rank"):
        if key in r:
            parts.append("%s=%s" % (key, r[key]))
    parts.append("seconds=%.1f" % r["seconds"])
    parts.append("exit=%s" % ("killed" if r["rc"] is None else r["rc"]))
    if r.get("why"):
        parts.append("note=%s" % r["why"])
    return " ".join(parts)


def check_a(args, code, info, F):
    mindist = program(args.mindist_bin, os.path.join("mindist", "mindist"),
                      [os.path.join(HERE, "fields"), os.path.join(HERE, "mindist")])
    cmd = [mindist]
    if args.a_timeout:
        cmd += ["--max-seconds", str(args.a_timeout)]
    cmd.append(code)
    rc, out, err, to, secs = run(cmd, timeout=(args.a_timeout + 60) if args.a_timeout else None)
    r = parse_checker("mindist", rc, out, to, secs, info)
    if r["state"] == "exact":
        r["why"] = check_codeword(F, info, "check A", r, True)
        if r["why"]:
            r["state"] = "inconsistent"
    say(line_for("checkA", r))
    return r


def check_b_sage(args, code, info, F):
    sb, env = sage_command()
    rc, out, err, to, secs = run([sb, os.path.join(HERE, "sage", "checkB.sage"), code],
                                 timeout=args.b_timeout, env=env)
    r = {"name": "sage", "rc": rc, "seconds": secs, "finished": False}
    kv = keyvals(out)
    r.update(kv)
    if to:
        r["state"] = "timeout"
    elif rc == 2 or kv.get("status") == "invalid":
        r["state"] = "invalid"
        r["why"] = kv.get("error")
    elif rc != 0 or kv.get("status") != "exact":
        r["state"] = "error"
        r["why"] = kv.get("error") or (err.strip().splitlines()[-1] if err.strip() else None)
    else:
        try:
            r["d"] = int(kv["d"])
            r["rank_i"] = int(kv["rank"])
            r["state"] = "exact"
            r["finished"] = True
            for key in ("q", "n", "k"):
                if int(kv[key]) != info[key]:
                    r["state"] = "error"
                    r["why"] = "%s=%s, the file says %d" % (key, kv[key], info[key])
        except (KeyError, ValueError):
            r["state"] = "error"
            r["why"] = "output lacks q, n, k, rank or d"
        if r["state"] == "exact":
            r["why"] = check_codeword(F, info, "check B", r, False)
            if r["why"]:
                r["state"] = "inconsistent"
    say(line_for("checkB", r))
    return r


def check_b_enum(args, code, info, F):
    """Substitute form of check B: enumd plus the SageMath cross-checks."""
    enumd = program(args.enumd_bin, os.path.join("enum", "enumd"), [os.path.join(HERE, "enum")])
    cmd = [enumd]
    if args.b_timeout:
        cmd += ["--max-seconds", str(args.b_timeout)]
    cmd.append(code)
    rc, out, err, to, secs = run(cmd, timeout=(args.b_timeout + 60) if args.b_timeout else None)
    r = parse_checker("enumd", rc, out, to, secs, info)
    if r["state"] == "exact":
        r["why"] = check_codeword(F, info, "enumd", r, True)
        if r["why"]:
            r["state"] = "inconsistent"
    say(line_for("checkB", r))
    if r["state"] != "exact":
        return r
    # cross-checks in SageMath
    gfdump = program(args.gfdump_bin, os.path.join("fields", "gfdump"), [os.path.join(HERE, "fields")])
    gfencode = program(args.gfencode_bin, os.path.join("fields", "gfencode"), [os.path.join(HERE, "fields")])
    sb, env = sage_command()
    sagedir = os.path.join(HERE, "sage")
    q = str(info["q"])
    failures = []
    t0 = time.time()
    with tempfile.TemporaryDirectory(prefix="verify-") as tmp:
        dumps = []
        for label, c in (("gfdump", [gfdump, q]), ("enumd", [enumd, "--dump-tables", q])):
            rc2, out2, err2, to2, _ = run(c, timeout=600)
            path = os.path.join(tmp, label + ".dump")
            with open(path, "w") as fh:
                fh.write(out2)
            if rc2 != 0:
                failures.append("%s dump exit %s" % (label, rc2))
            dumps.append(path)
        rc2, out2, err2, to2, _ = run([sb, os.path.join(sagedir, "tables_check.sage")] + dumps, timeout=600, env=env)
        say("crosscheck_tables=%s" % ("ok" if rc2 == 0 else "FAIL"))
        if rc2 != 0:
            failures.append("tables_check: " + " | ".join(l for l in out2.splitlines() if "FAIL" in l))
        enc = os.path.join(tmp, "encode.txt")
        rc2, out2, err2, to2, _ = run([gfencode, code, str(ENCODE_COUNT), str(ENCODE_SEED)], timeout=600)
        with open(enc, "w") as fh:
            fh.write(out2)
        if rc2 != 0:
            failures.append("gfencode exit %s" % rc2)
        rc2, out2, err2, to2, _ = run([sb, os.path.join(sagedir, "crosscheck.sage"), code, enc], timeout=1800, env=env)
        kv = keyvals(out2)
        ok = rc2 == 0 and kv.get("lines") == str(ENCODE_COUNT)
        say("crosscheck_codewords=%s lines=%s" % ("ok" if ok else "FAIL", kv.get("lines")))
        if not ok:
            failures.append("crosscheck: " + " | ".join(l for l in out2.splitlines() if "FAIL" in l))
        try:
            r["sage_rank"] = int(kv["rank"])
        except (KeyError, ValueError):
            failures.append("crosscheck printed no rank")
        say("crosscheck_rank=%s" % r.get("sage_rank"))
    rc2, out2, err2, to2, _ = run([sb, os.path.join(sagedir, "checkword.sage"), code, str(r["d"])] + r["codeword"].split(" "),
                                  timeout=600, env=env)
    say("crosscheck_minword=%s" % ("ok" if rc2 == 0 else "FAIL"))
    if rc2 != 0:
        failures.append("checkword: " + " | ".join(l for l in out2.splitlines() if "FAIL" in l))
    r["seconds"] += time.time() - t0
    say("crosscheck_seconds=%.1f" % (time.time() - t0))
    if failures:
        r["state"] = "inconsistent"
        r["why"] = "; ".join(failures)
    return r


def verify_code(args, code, claim=None):
    """Returns the exit code. claim: dict with any of q, n, k, d."""
    say("file=%s" % code)
    try:
        info = codetxt.read_code(code)
    except codetxt.CodeFileError as e:
        say("format=invalid note=%s" % e)
        say("n=? k=? d=?")
        say("result=invalid exit=2")
        return 2
    except OSError as e:
        say("format=unreadable note=%s" % e)
        return 1
    F = info["field"]
    own_rank = codetxt.rank(F, info["rows"])
    say("format=ok q=%d n=%d k=%d rank=%d" % (info["q"], info["n"], info["k"], own_rank))
    a = check_a(args, code, info, F)
    b = check_b_enum(args, code, info, F) if args.b == "enum" else check_b_sage(args, code, info, F)
    say("checkB_form=%s" % ("substitute" if args.b == "enum" else "sage"))

    ranks = [own_rank]
    for r in (a, b):
        if "rank_i" in r:
            ranks.append(r["rank_i"])
        if "sage_rank" in r:
            ranks.append(r["sage_rank"])
    codes = []
    if a["state"] == "invalid" or b["state"] == "invalid":
        codes.append((2, "a check found the file invalid"))
    if any(x != info["k"] for x in ranks) or a["state"] == "rank" or b["state"] == "rank":
        codes.append((3, "rank is not k (ranks found: %s, k=%d)" % (sorted(set(ranks)), info["k"])))
    if a["state"] == "inconsistent" or b["state"] == "inconsistent":
        codes.append((4, a.get("why") or b.get("why")))
    if a["state"] == "exact" and b["state"] == "exact" and a["d"] != b["d"]:
        codes.append((4, "check A d=%d, check B d=%d" % (a["d"], b["d"])))
    if a["state"] in ("timeout", "error") or b["state"] in ("timeout", "error"):
        what = []
        for label, r in (("check A", a), ("check B", b)):
            if r["state"] in ("timeout", "error"):
                what.append("%s %s%s" % (label, r["state"], (": " + r["why"]) if r.get("why") else ""))
        codes.append((6 if all(r["state"] != "error" for r in (a, b)) else 1, "; ".join(what)))
    d = a["d"] if (a["state"] == "exact" and b["state"] == "exact" and a["d"] == b["d"]) else None
    if claim:
        for key in ("q", "n", "k"):
            if key in claim and claim[key] is not None and int(claim[key]) != info[key]:
                codes.append((5, "%s=%d, the claim says %s" % (key, info[key], claim[key])))
        if claim.get("d") is not None and d is not None and d != int(claim["d"]):
            codes.append((5, "d=%d, the claim says %s" % (d, claim["d"])))
    say("n=%d k=%d d=%s" % (info["n"], info["k"], d if d is not None else "?"))
    order = [2, 3, 4, 1, 6, 5]
    if codes:
        codes.sort(key=lambda c: order.index(c[0]))
        for c, why in codes:
            say("problem=%d %s" % (c, why))
        say("result=FAIL exit=%d" % codes[0][0])
        return codes[0][0]
    if claim and claim.get("d") is not None:
        say("claim=ok d=%s" % claim["d"])
    say("result=OK exit=0")
    return 0


def sha256(path):
    h = hashlib.sha256()
    with open(path, "rb") as fh:
        for chunk in iter(lambda: fh.read(1 << 20), b""):
            h.update(chunk)
    return h.hexdigest()


def verify_release(args, folder):
    folder = os.path.abspath(folder)
    say("release=%s" % folder)
    mpath = os.path.join(folder, "MANIFEST.json")
    try:
        with open(mpath) as fh:
            man = json.load(fh)
    except (OSError, ValueError) as e:
        say("manifest=unreadable note=%s" % e)
        return 1
    hashes = man.get("sha256")
    if not isinstance(hashes, dict) or not hashes:
        say("manifest=invalid note=no sha256 table")
        return 1
    bad = 0
    for rel in sorted(hashes):
        p = os.path.join(folder, rel)
        if os.path.isabs(rel) or ".." in rel.split("/") or not os.path.isfile(p):
            say("hash=MISSING %s" % rel)
            bad += 1
        elif sha256(p) != hashes[rel].lower():
            say("hash=MISMATCH %s" % rel)
            bad += 1
    listed = set(hashes)
    for root, dirs, files in os.walk(folder):
        dirs[:] = [x for x in dirs if x != "__pycache__"]
        for f in files:
            rel = os.path.relpath(os.path.join(root, f), folder).replace(os.sep, "/")
            if rel != "MANIFEST.json" and rel not in listed:
                say("unlisted=%s" % rel)
    say("hashes=%s checked=%d" % ("ok" if bad == 0 else "FAIL", len(hashes)))
    if bad:
        say("result=FAIL exit=7")
        return 7
    code = os.path.join(folder, man.get("codefile", "code.txt"))
    claim = dict((key, man.get(key)) for key in ("q", "n", "k", "d"))
    return verify_code(args, code, claim)


class Parser(argparse.ArgumentParser):
    def error(self, message):
        # exit 1 (other error): exit 2 means an invalid code file here
        self.print_usage(sys.stderr)
        sys.stderr.write("verify.py: error: %s\n" % message)
        sys.exit(1)


def main(argv):
    ap = Parser(description="Two exact minimum-distance checks of a code.txt file.")
    ap.add_argument("code", nargs="?", help="code.txt")
    ap.add_argument("--release", nargs="?", const=HERE, default=None, metavar="DIR",
                    help="check a release folder against its MANIFEST.json (default: the folder of verify.py)")
    ap.add_argument("--claim-d", type=int, default=None)
    ap.add_argument("--b", choices=("sage", "enum"), default="sage")
    ap.add_argument("--b-timeout", type=int, default=None, metavar="SEC")
    ap.add_argument("--a-timeout", type=int, default=None, metavar="SEC")
    ap.add_argument("--mindist-bin", default=None, help=argparse.SUPPRESS)
    ap.add_argument("--enumd-bin", default=None, help=argparse.SUPPRESS)
    ap.add_argument("--gfdump-bin", default=None, help=argparse.SUPPRESS)
    ap.add_argument("--gfencode-bin", default=None, help=argparse.SUPPRESS)
    args = ap.parse_args(argv)
    try:
        if args.release is not None:
            if args.code:
                ap.error("give either <code.txt> or --release [DIR]")
            return verify_release(args, args.release)
        if not args.code:
            ap.error("missing <code.txt>")
        claim = {"d": args.claim_d} if args.claim_d is not None else None
        return verify_code(args, args.code, claim)
    except Fatal as e:
        say("error=%s" % e)
        say("result=FAIL exit=%d" % e.code)
        return e.code


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
