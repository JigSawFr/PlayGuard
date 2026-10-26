#!/usr/bin/env python3
"""Runs every scenario of tools/desktop_smoke.py, then the visual check of
each one that has references, and prints a summary.

The scenarios are read from desktop_smoke.py itself: the default one (no
argument, named "smoke" like its references, tests/visual/smoke/) and every
name it tests with SCENARIO == "..." or SCENARIO in (...). They run N at a time (-j, 3 by
default), each on its own X display and D-Bus session
(xvfb-run -a ... dbus-run-session), into <out-dir>/<scenario>/, with its
output in <out-dir>/<scenario>.log.

Then tools/visual_check.py compares each scenario that passed and has
tests/visual/<scenario>/SCREENS with its references, .visual-refs/<scenario>/
(main's screens: `tools/visual_refs.py fetch` takes the latest release's,
or a run's with --run). With --update, this run's screens become the
local references instead (visual_refs.py keep), to compare the next run with.

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
VISUAL_REFS = os.path.join(ROOT, "tools", "visual_refs.py")
SCREENS = os.path.join(ROOT, "tests", "visual")
REFERENCES = os.environ.get("PLAYGUARD_VISUAL_REFS") or os.path.join(ROOT, ".visual-refs")
DEFAULT = "smoke"   # the scenario run with no argument, as tests/visual/ names it
XVFB = ["xvfb-run", "-a", "-s", "-screen 0 1280x720x24", "dbus-run-session"]


def scenarios():
    """The default scenario, then those desktop_smoke.py tests for, in the
    order it names them."""
    with open(SMOKE, encoding="utf-8") as f:
        text = f.read()
    names = []
    # A flag for two scenarios names them in a tuple (SCENARIO in ("sync", "sync-311")).
    for m in re.finditer(r'SCENARIO\s*==\s*"([\w-]+)"|SCENARIO\s+in\s+\(([^)]*)\)', text):
        names += [m.group(1)] if m.group(1) else re.findall(r'"([\w-]+)"', m.group(2))
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


def visual_check(shots, name, update):
    """Compares a scenario's screens with its references. Returns (state,
    changed screens): state is "ok", "changed", "none" (no references of
    main's here: `tools/visual_refs.py fetch`) or "updated"."""
    reference = os.path.join(REFERENCES, name)
    if update:
        out = subprocess.run([sys.executable, VISUAL_REFS, "keep", shots, name], capture_output=True, text=True)
        with open(shots + "-visual.txt", "w") as f:
            f.write(out.stdout + out.stderr)
        changed = re.findall(r"^(?:updated|added) (\S+)$", out.stdout, re.M)
        return ("updated" if out.returncode == 0 else "changed"), changed
    if not os.path.isdir(reference):
        return "none", []
    out = subprocess.run([sys.executable, VISUAL, shots, reference, "--screens", os.path.join(SCREENS, name, "SCREENS"),
                          "--scenario", name], capture_output=True, text=True)
    with open(shots + "-visual.txt", "w") as f:
        f.write(out.stdout + out.stderr)
    if out.returncode == 0:
        return "ok", []
    m = re.search(r"^visual check: \d+ screen\(s\) changed: (.*)$", out.stdout, re.M)
    if not m:   # an ImageMagick error...: its last line says what
        lines = (out.stdout + out.stderr).strip().splitlines()
        return "changed", [lines[-1] if lines else "visual check failed"]
    return "changed", [s.strip() for s in m.group(1).split(",")]


def main():
    ap = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    ap.add_argument("out_dir")
    ap.add_argument("scenarios", nargs="*", metavar="scenario", help="only these (default: all of them)")
    ap.add_argument("-j", "--jobs", type=int, default=3, help="scenarios run at the same time (3)")
    ap.add_argument("--update", action="store_true", help="make this run's screens the local references")
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
            listed = os.path.exists(os.path.join(SCREENS, name, "SCREENS"))
            check = f"   then visual_check.py against {os.path.relpath(REFERENCES, ROOT)}/{name}" if listed else "   (no stable screen)"
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
        if not passed:
            state, changed = "-", []
            failed = True
        elif not os.path.exists(os.path.join(SCREENS, name, "SCREENS")):
            state, changed = "none", []
        else:
            state, changed = visual_check(os.path.join(out_dir, name), name, args.update)
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
    if any(r[3] == "none" for r in rows):
        print(f"No references of main's in {os.path.relpath(REFERENCES, ROOT)}/ for some scenarios: "
              "`python3 tools/visual_refs.py fetch` takes the latest release's.")
    if any(r[3] == "updated" for r in rows):
        print(f"This run's screens are now the local references ({os.path.relpath(REFERENCES, ROOT)}/).")
    return 1 if failed else 0


if __name__ == "__main__":
    sys.exit(main())
