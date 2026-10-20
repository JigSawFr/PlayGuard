#!/usr/bin/env python3
"""Runs every scenario of tools/desktop_smoke.py, then the visual check of
each one that has references, and prints a summary.

The scenarios are read from desktop_smoke.py itself: the default one (no
argument, named "smoke" like its references, tests/visual/smoke/) and every
name it tests with SCENARIO == "...". They run N at a time (-j, 3 by
default), each on its own X display and D-Bus session
(xvfb-run -a ... dbus-run-session), into <out-dir>/<scenario>/, with its
output in <out-dir>/<scenario>.log.

Then tools/visual_check.py compares each scenario that passed and has
tests/visual/<scenario>/ with its references. With --update, a scenario
whose screens changed gets its references replaced (visual_check.py
--update): look at the new images before committing them.

It fails when a scenario fails, or when a screen changed (unless --update).

Usage: tools/smoke_all.py <out-dir> [-j N] [--update] [--dry-run] [scenario ...]
       (needs build-desktop/playguard, xvfb-run, dbus-run-session, xdotool,
       ImageMagick; a few scenario names limit the run to them)
"""
import argparse
import concurrent.futures
import os
import re
import shlex
import subprocess
import sys
import time

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
SMOKE = os.path.join(ROOT, "tools", "desktop_smoke.py")
VISUAL = os.path.join(ROOT, "tools", "visual_check.py")
REFERENCES = os.path.join(ROOT, "tests", "visual")
DEFAULT = "smoke"   # the scenario run with no argument, as tests/visual/ names it
XVFB = ["xvfb-run", "-a", "-s", "-screen 0 1280x720x24", "dbus-run-session"]


def scenarios():
    """The default scenario, then those desktop_smoke.py tests for, in the
    order it names them."""
    with open(SMOKE, encoding="utf-8") as f:
        names = re.findall(r'SCENARIO\s*==\s*"(\w+)"', f.read())
    return [DEFAULT] + list(dict.fromkeys(names))


def smoke_command(out_dir, name):
    arg = [] if name == DEFAULT else [name]
    return XVFB + [sys.executable, SMOKE, os.path.join(out_dir, name)] + arg


def run_smoke(out_dir, name):
    """Runs one scenario; returns (passed, seconds)."""
    start = time.monotonic()
    with open(os.path.join(out_dir, name + ".log"), "w") as log:
        code = subprocess.run(smoke_command(out_dir, name), stdout=log, stderr=subprocess.STDOUT,
                              cwd=ROOT).returncode
    return code == 0, time.monotonic() - start


def visual_check(shots, reference, update):
    """Compares a scenario's screens with its references. Returns (state,
    changed screens): state is "ok", "changed" or "updated"."""
    out = subprocess.run([sys.executable, VISUAL, shots, reference], capture_output=True, text=True)
    with open(shots + "-visual.txt", "w") as f:
        f.write(out.stdout + out.stderr)
    if out.returncode == 0:
        return "ok", []
    m = re.search(r"^visual check: \d+ screen\(s\) changed: (.*)$", out.stdout, re.M)
    if not m:   # no reference image, an ImageMagick error...: its last line says what
        lines = (out.stdout + out.stderr).strip().splitlines()
        return "changed", [lines[-1] if lines else "visual check failed"]
    changed = [s.strip() for s in m.group(1).split(",")]
    if update:
        subprocess.run([sys.executable, VISUAL, shots, reference, "--update"], check=True,
                       stdout=subprocess.DEVNULL)
        return "updated", changed
    return "changed", changed


def main():
    ap = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    ap.add_argument("out_dir")
    ap.add_argument("scenarios", nargs="*", metavar="scenario", help="only these (default: all of them)")
    ap.add_argument("-j", "--jobs", type=int, default=3, help="scenarios run at the same time (3)")
    ap.add_argument("--update", action="store_true", help="replace the references of the screens that changed")
    ap.add_argument("--dry-run", action="store_true", help="print the scenarios and their commands, run nothing")
    args = ap.parse_args()

    known = scenarios()
    unknown = [s for s in args.scenarios if s not in known]
    if unknown:
        sys.exit(f"unknown scenario(s): {', '.join(unknown)} (known: {', '.join(known)})")
    todo = args.scenarios or known
    out_dir = os.path.abspath(args.out_dir)

    if args.dry_run:
        for name in todo:
            ref = os.path.join(REFERENCES, name)
            check = f"   then visual_check.py against tests/visual/{name}" if os.path.isdir(ref) else "   (no references)"
            print(f"{name}: {shlex.join(smoke_command(out_dir, name))}\n{check}")
        print(f"{len(todo)} scenario(s), {args.jobs} at a time")
        return 0

    if not os.path.exists(os.path.join(ROOT, "build-desktop", "playguard")):
        sys.exit("no build-desktop/playguard: run `make desktop` first")
    os.makedirs(out_dir, exist_ok=True)
    smoke = {}
    with concurrent.futures.ThreadPoolExecutor(max_workers=max(1, args.jobs)) as pool:
        futures = {pool.submit(run_smoke, out_dir, name): name for name in todo}
        for future in concurrent.futures.as_completed(futures):
            name = futures[future]
            smoke[name] = future.result()
            passed, seconds = smoke[name]
            print(f"{'ok  ' if passed else 'FAIL'} {name} ({seconds:.0f} s)", flush=True)

    rows = []
    failed = False
    for name in todo:
        passed, seconds = smoke[name]
        ref = os.path.join(REFERENCES, name)
        if not passed:
            state, changed = "-", []
            failed = True
        elif not os.path.isdir(ref):
            state, changed = "none", []
        else:
            state, changed = visual_check(os.path.join(out_dir, name), ref, args.update)
            failed |= state == "changed"
        rows.append((name, "ok" if passed else "FAIL", f"{seconds:.0f} s", state, ", ".join(changed)))

    print()
    header = ("scenario", "smoke", "time", "visual", "changed screens")
    widths = [max(len(r[i]) for r in rows + [header]) for i in range(4)]
    for row in [header] + rows:
        print("  ".join(c.ljust(w) for c, w in zip(row, widths)) + "  " + row[4])
    for name, result, _, state, _ in rows:
        if result == "FAIL":
            print(f"{name}: see {os.path.join(out_dir, name + '.log')}")
        elif state == "changed":
            print(f"{name}: side-by-side images in {os.path.join(out_dir, name, 'visual_diff')}")
    if any(r[3] == "updated" for r in rows):
        print("References updated: look at the new images in tests/visual/ before committing them.")
    return 1 if failed else 0


if __name__ == "__main__":
    sys.exit(main())
