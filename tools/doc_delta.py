#!/usr/bin/env python3
"""What changed in an English page of the user guide since a translation of
it was stamped (tools/doc_stamp.py), to update the translation.

For docs/<language>/<page>.md, reads its first line,
`<!-- translated from docs/<page>.md, sha256 <12 hex> -->`, finds the version
of docs/<page>.md with that hash in the git history (the commits that
changed it, newest first, each version hashed as doc_stamp.py does), and
prints the diff from that version to the page as it is now, or "up to date".

--all does it for every translation of every page and lists the stale ones
(without the diffs; give a page to see its diff).

Exits 1 when a page is stale, has no stamp or was stamped from a version
the history does not have (a shallow clone: `git fetch --unshallow`).

Usage: tools/doc_delta.py docs/<language>/<page>.md ...
       tools/doc_delta.py --all
"""
import glob
import hashlib
import os
import re
import subprocess
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
STAMP = re.compile(r"<!-- translated from (docs/[\w-]+\.md), sha256 ([0-9a-f]{12}) -->")


def digest(data):
    return hashlib.sha256(data).hexdigest()[:12]


def git(*args):
    return subprocess.run(["git", "-C", ROOT] + list(args), capture_output=True, check=True).stdout


_versions = {}


def versions(english):
    """{hash: commit} of every committed version of `english`, the newest
    commit kept for a content several commits share."""
    if english not in _versions:
        found = {}
        for commit in git("log", "--format=%H", "--", english).decode().split():
            try:
                found.setdefault(digest(git("show", f"{commit}:{english}")), commit)
            except subprocess.CalledProcessError:
                pass   # the commit that deleted the page
        _versions[english] = found
    return _versions[english]


def delta(path):
    """(state, english page, commit, diff): state is "up to date", "stale",
    "no stamp" or "unknown version"."""
    with open(os.path.join(ROOT, path), encoding="utf-8") as f:
        m = STAMP.match(f.readline())
    if not m:
        return "no stamp", None, None, ""
    english, stamped = m.group(1), m.group(2)
    with open(os.path.join(ROOT, english), "rb") as f:
        if digest(f.read()) == stamped:
            return "up to date", english, None, ""
    commit = versions(english).get(stamped)
    if not commit:
        return "unknown version", english, None, ""
    diff = git("diff", "--no-color", commit, "--", english).decode("utf-8", "replace")
    return "stale", english, commit, diff


def translations():
    """Every docs/<language>/<page>.md whose English page is docs/<page>.md."""
    return sorted(os.path.relpath(p, ROOT) for p in glob.glob(os.path.join(ROOT, "docs", "*", "*.md"))
                  if os.path.exists(os.path.join(ROOT, "docs", os.path.basename(p))))


def describe(path, state, english, commit, diff):
    if state == "no stamp":
        return f"{path}: no stamp (tools/doc_stamp.py adds it)"
    if state == "unknown version":
        return f"{path}: stamped from a version of {english} this clone's history does not have " \
               f"(a shallow clone? git fetch --unshallow)"
    if state == "up to date":
        return f"{path}: up to date"
    lines = sum(1 for l in diff.splitlines() if l[:1] in "+-" and l[:3] not in ("+++", "---"))
    return f"{path}: stale, {english} changed since {commit[:7]} ({lines} line(s) added or removed)"


def main(args):
    if args == ["--all"]:
        results = [(p,) + delta(p) for p in translations()]
        for r in results:
            print(describe(*r))
        stale = [r[0] for r in results if r[1] != "up to date"]
        print(f"\n{len(stale)} of {len(results)} translation(s) to update" + (":" if stale else ""))
        for p in stale:
            print(" ", p)
        return 1 if stale else 0
    if not args or any(a.startswith("-") for a in args):
        print(__doc__)
        return 2
    behind = False
    for arg in args:
        path = os.path.relpath(os.path.abspath(arg), ROOT)
        state, english, commit, diff = delta(path)
        print(describe(path, state, english, commit, diff))
        if diff:
            print(diff, end="" if diff.endswith("\n") else "\n")
        behind |= state != "up to date"
    return 1 if behind else 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
