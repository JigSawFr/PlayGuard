#!/usr/bin/env python3
"""The job summaries of the build workflow (.github/workflows/build.yml), as
Markdown on stdout: each job appends one section to $GITHUB_STEP_SUMMARY.

  header     the run: version, commit, branch or pull request, trigger
  host-tests the suites of `make test` from build/host-tests/<suite>.log and .ok
  fuzz       the libFuzzer runs of `make fuzz` from build/fuzz/<target>.log
  coverage   gcovr's JSON summary (--json-summary), per file, and vs main
  smoke      a desktop-smoke shard: each scenario's smoke test and visual check
  build      the Switch build: sizes vs main, downloads, what makes the .nro
             big (the linker map), toolchain, checksums
  map        that linker-map table alone (also for a local build:
             tools/ci_summary.py map build/borealis.map --nro playguard.nro)
  release    the assets attached to a release
  tidy-files the files clang-tidy checks (.github/workflows/clang-tidy.yml):
             the changed sources the desktop build compiles, on stdout
  tidy       the findings of that run, by check, from run-clang-tidy's output

A "baseline" is a small JSON file kept in the Actions cache by the builds of
main (the .nro size, the coverage): --baseline reads the last one for the
"vs main" deltas, --write-baseline writes this build's. A size or coverage
that moved too far prints a ::warning command on stderr (an annotation, not
a failure). Standard library only; `--help` on a command lists its options.
"""
import argparse
import collections
import hashlib
import json
import os
import re
import sys
import zipfile

OK, FAIL, SKIP, WARN = "✅", "❌", "⏭️", "⚠️"


# ---------------------------------------------------------------- formatting

def fmt_size(n):
    """Bytes as the file managers show them: 5.48 MB, 812 KB, 214 B."""
    n = float(n)
    if n < 1024:
        return f"{int(n)} B"
    if n < 1024 * 1024:
        return f"{n / 1024:.0f} KB" if n >= 100 * 1024 else f"{n / 1024:.1f} KB"
    return f"{n / 1024 / 1024:.2f} MB"


def fmt_delta_bytes(d):
    if d == 0:
        return "= same size"
    sign, arrow = ("+", "▲") if d > 0 else ("−", "▼")
    return f"{arrow} {sign}{fmt_size(abs(d))}"


def fmt_delta_points(d, unit="pt"):
    if abs(d) < 0.05:
        return "= unchanged"
    sign, arrow = ("+", "▲") if d > 0 else ("−", "▼")
    return f"{arrow} {sign}{abs(d):.1f} {unit}"


def fmt_seconds(s):
    s = int(round(s))
    return f"{s // 60} min {s % 60:02d} s" if s >= 60 else f"{s} s"


def bar(pct, width=12):
    """A fill bar in a code span (every block the same width): `████░░░░`."""
    pct = max(0.0, min(100.0, float(pct)))
    filled = int(round(pct / 100 * width))
    return "`" + "█" * filled + "░" * (width - filled) + "`"


def cell(s):
    return str(s).replace("|", "\\|").replace("\n", " ")


def table(headers, rows, align=None):
    """A GitHub table. align: one letter per column, l / r / c (default l)."""
    align = align or "l" * len(headers)
    sep = {"l": "---", "r": "---:", "c": ":---:"}
    lines = ["| " + " | ".join(cell(h) for h in headers) + " |",
             "| " + " | ".join(sep[a] for a in align) + " |"]
    for r in rows:
        lines.append("| " + " | ".join(cell(c) for c in r) + " |")
    return "\n".join(lines)


def details(summary, body, open_=False):
    return (f"<details{' open' if open_ else ''}><summary>{summary}</summary>\n\n{body}\n\n</details>")


def link(text, url):
    return f"[{text}]({url})" if url else text


def code(s):
    return "`" + str(s).replace("`", "'") + "`"


def warn(title, message):
    """An annotation on the run (and the pull request), without failing anything."""
    print(f"::warning title={title}::{message}", file=sys.stderr)


def read_text(path):
    try:
        with open(path, encoding="utf-8", errors="replace") as f:
            return f.read()
    except OSError:
        return None


def read_json(path):
    text = read_text(path) if path else None
    if text is None:
        return None
    try:
        return json.loads(text)
    except ValueError:
        return None


def write_json(path, data):
    os.makedirs(os.path.dirname(path) or ".", exist_ok=True)
    with open(path, "w", encoding="utf-8") as f:
        json.dump(data, f, indent=1)
        f.write("\n")


def file_size(path):
    try:
        return os.path.getsize(path)
    except OSError:
        return None


def sha256_of(path):
    h = hashlib.sha256()
    with open(path, "rb") as f:
        for chunk in iter(lambda: f.read(1 << 20), b""):
            h.update(chunk)
    return h.hexdigest()


def commit_link(repo_url, sha):
    return link(code(sha[:7]), f"{repo_url}/commit/{sha}") if repo_url and sha else code(sha[:7])


# -------------------------------------------------------------------- header

def cmd_header(a):
    sha7 = a.sha[:7]
    if a.pr:
        where = f"PR #{a.pr}"
        branch = f"{link(code(a.branch), f'{a.repo_url}/tree/{a.branch}')} → {code(a.base or 'main')}"
        about = link(where, f"{a.repo_url}/pull/{a.pr}")
    elif a.tag:
        where = f"release {a.tag}"
        branch = code(a.tag)
        about = link(a.tag, f"{a.repo_url}/releases/tag/{a.tag}")
    else:
        where = a.branch
        branch = link(code(a.branch), f"{a.repo_url}/tree/{a.branch}")
        about = ""
    out = [f"## 🛡️ PlayGuard {a.version} · {where} · {code(sha7)}", ""]
    if a.title:
        out += [f"**{a.title.strip()}**" + (f" ({about})" if about else ""), ""]
    runner = code(a.runner)
    if a.image:
        runner += " · " + code(a.image[:len("devkitpro/devkita64@sha256:") + 12] + "…" if "@sha256:" in a.image else a.image)
    out.append(table(["Build", "Commit", "Branch", "Trigger", "Runners"],
                     [[code(a.label), commit_link(a.repo_url, a.sha), branch, code(a.event), runner]]))
    out.append("")
    out.append("Every artifact of this run is named `playguard-" + a.label + "-<component>`: "
               "`-nro` (the app, what *Install another build* downloads), `-rescue` (the recovery sysmodule), "
               "`-linker-map`, `-screenshots-<shard>`" + (", `-sdcard` (the store zip)" if a.tag else "") + ".")
    return "\n".join(out)


# ---------------------------------------------------------------- host-tests

def suites_of(makefile):
    """The suites of the Makefile, in its order: (name, test source)."""
    out = []
    text = read_text(makefile) or ""
    for m in re.finditer(r"^(\w+)_SRC\s*:=\s*(.+)$", text, re.M):
        out.append((m.group(1), m.group(2).split()[-1]))
    return out


def cmd_host_tests(a):
    suites = suites_of(a.makefile)
    rows, passed, failed, not_run = [], 0, 0, 0
    for name, test in suites:
        log = read_text(os.path.join(a.dir, name + ".log"))
        ok = os.path.exists(os.path.join(a.dir, name + ".ok"))
        folder = os.path.dirname(test) + "/"
        if ok:
            passed += 1
            last = (log or "").strip().splitlines()
            rows.append([OK, code(name), code(folder), last[-1] if last else "passed"])
        elif log is None:
            # No log: the suite did not compile, or was not reached.
            built = os.path.exists(os.path.join(a.dir, name))
            if built:
                failed += 1
                rows.append([FAIL, code(name), code(folder), "no output"])
            else:
                not_run += 1
                rows.append([SKIP, code(name), code(folder), "did not build (see the log)"])
        else:
            failed += 1
            lines = [l for l in log.splitlines() if "check failed" in l or "check(s) failed" in l or "Sanitizer" in l]
            rows.append([FAIL, code(name), code(folder), "<br>".join(code(l) for l in lines[:6]) or "exited with an error"])
    total = len(suites)
    if failed or not_run:
        verdict = f"{FAIL} {failed + not_run} of {total} suites failed"
    else:
        verdict = f"{OK} {total} suites passed"
    heading = f"## 🧪 Host tests — {verdict}"
    if a.compiler:
        heading += f" · {a.compiler}"
    if a.flags:
        heading += f" · {a.flags}"
    body = table(["", "Suite", "Tests", "Result"], rows)
    return heading + "\n\n" + details(f"{total} suites, build/host-tests/&lt;suite&gt;.log", body, open_=bool(failed or not_run))


# ---------------------------------------------------------------------- fuzz

def fuzzers_of(makefile):
    """The fuzz targets of the Makefile, in its order: (name, target source)."""
    text = read_text(makefile) or ""
    return [(m.group(1), m.group(2).split()[-1]) for m in re.finditer(r"^(\w+)_FUZZ\s*:=\s*(.+)$", text, re.M)]


def cmd_fuzz(a):
    targets = fuzzers_of(a.makefile)
    rows, failed = [], 0
    for name, target in targets:
        log = read_text(os.path.join(a.dir, name + ".log"))
        found = sorted(f for f in os.listdir(a.dir) if f.startswith(name + "-")) if os.path.isdir(a.dir) else []
        stats = dict(re.findall(r"^stat::(\w+):\s*(\d+)", log or "", re.M))
        runs = f"{int(stats['number_of_executed_units']):,}" if "number_of_executed_units" in stats else ""
        if log is None:
            failed += 1
            rows.append([SKIP, code(name), code(target), "", "did not build or run (see the job log)"])
        elif found or not re.search(r"^Done \d+ runs", log, re.M):
            failed += 1
            why = [l for l in log.splitlines() if "ERROR:" in l or "runtime error" in l or "SUMMARY:" in l
                   or "fuzz property failed" in l]
            rows.append([FAIL, code(name), code(target), runs,
                         "<br>".join(code(l[:200]) for l in why[:4] + [f"input: {f}" for f in found[:1]])
                         or "stopped with an error"])
        else:
            rows.append([OK, code(name), code(target), runs, f"{stats.get('new_units_added', '0')} new inputs"])
    verdict = f"{FAIL} {failed} of {len(targets)} targets failed" if failed else f"{OK} {len(targets)} targets ran clean"
    heading = f"## 🎲 Fuzzing — {verdict}"
    if a.seconds:
        heading += f" · {a.seconds} s each"
    body = table(["", "Target", "Source", "Runs", "Result"], rows)
    return heading + "\n\n" + details(f"{len(targets)} targets, build/fuzz/&lt;target&gt;.log", body, open_=bool(failed))


# ------------------------------------------------------------------ coverage

def cmd_coverage(a):
    cov = read_json(a.json)
    if not cov:
        return f"## 📊 Coverage — {WARN} no gcovr summary at {code(a.json)}"
    base = read_json(a.baseline) if a.baseline else None
    kinds = [("line", "Lines"), ("branch", "Branches"), ("function", "Functions")]
    tiles, heads = [], []
    for key, label in kinds:
        pct = cov.get(f"{key}_percent")
        if pct is None:
            continue
        covered, total = cov.get(f"{key}_covered", 0), cov.get(f"{key}_total", 0)
        text = f"{bar(pct)} **{pct:.1f} %** ({covered} / {total})"
        if base and base.get(f"{key}_percent") is not None:
            text += f"<br>{fmt_delta_points(pct - base[f'{key}_percent'])} vs main {code(base.get('sha', '')[:7])}"
        tiles.append(text)
        heads.append(label)
    heading = f"## 📊 Coverage — " + " · ".join(f"{label.lower()} {cov[f'{k}_percent']:.1f} %" for k, label in kinds
                                                  if cov.get(f"{k}_percent") is not None)
    if a.scope:
        heading += f" · {a.scope}"
    out = [heading, "", table(heads, [tiles], "c" * len(heads)), ""]
    if base and base.get("line_percent") is not None:
        drop = base["line_percent"] - cov["line_percent"]
        if drop > a.max_drop:
            warn("Coverage", f"line coverage fell {drop:.1f} pt vs main ({base.get('sha', '')[:7]}): "
                             f"{cov['line_percent']:.1f} % now, {base['line_percent']:.1f} % before")
    elif a.baseline:
        out.append(f"_No baseline from main yet: the next push to main writes one._\n")
    files = sorted(cov.get("files", []), key=lambda f: (f.get("line_percent", 0), f.get("filename", "")))
    rows = []
    for f in files:
        rows.append([code(f.get("filename", "")), bar(f.get("line_percent", 0), 10), f"{f.get('line_percent', 0):.1f} %",
                     f"{f.get('line_covered', 0)} / {f.get('line_total', 0)}",
                     f"{f.get('branch_percent', 0):.1f} %" if f.get("branch_total") else "—"])
    if rows:
        out.append(details(f"Per file, least covered first ({len(rows)} files)",
                           table(["File", "", "Lines", "Covered", "Branches"], rows, "llrrr")))
    if a.write_baseline:
        write_json(a.write_baseline, {"sha": a.sha, **{k: cov.get(k) for k in cov if k.endswith("_percent")}})
    return "\n".join(out)


# --------------------------------------------------------------------- smoke

def read_tsv(path):
    text = read_text(path)
    return [line.split("\t") for line in text.splitlines() if line.strip()] if text else []


def cmd_smoke(a):
    # smoke-results.tsv: scenario, ok|fail, seconds, screens
    # visual-results.tsv: scenario, ok|announced|changed|kept|none, screens compared, the changed
    # (or announced, or kept: ~updated +added -removed) names, log file
    smoke = {r[0]: r for r in read_tsv(a.results)}
    visual_ran = bool(a.visual) and os.path.exists(a.visual)
    visual = {r[0]: r for r in read_tsv(a.visual)} if visual_ran else {}
    rows, failing, screens, compared, changed_all, seconds = [], [], 0, 0, [], 0.0
    changed_logs = []
    for sc in a.scenarios:
        s = smoke.get(sc)
        v = visual.get(sc)
        folder = "smoke" if sc == "smoke" else f"smoke-{sc}"
        if s is None:
            smoke_cell, time_cell, n = f"{SKIP} not run", "—", 0
        else:
            ok = s[1] == "ok"
            n = int(s[3]) if len(s) > 3 and s[3].isdigit() else 0
            smoke_cell = f"{OK} passed" if ok else f"{FAIL} failed"
            time_cell = fmt_seconds(float(s[2])) if len(s) > 2 else "—"
            seconds += float(s[2]) if len(s) > 2 else 0
            screens += n
            if not ok:
                failing.append(sc)
        if v is None:
            visual_cell = "—" if s is None or s[1] != "ok" or not visual_ran else f"{SKIP} not run"
        elif v[1] == "none":
            visual_cell = f"{SKIP} no references"
        elif v[1] == "ok":
            compared += int(v[2]) if v[2].isdigit() else 0
            visual_cell = f"{OK} {v[2]} unchanged"
        elif v[1] == "announced":
            names = [x for x in (v[3] if len(v) > 3 else "").split(",") if x]
            compared += int(v[2]) if v[2].isdigit() else 0
            visual_cell = f"{OK} {v[2]} compared, {len(names)} announced or new: " + ", ".join(code(x) for x in names)
        elif v[1] == "kept":
            names = [x for x in (v[3] if len(v) > 3 else "").split(",") if x]
            compared += int(v[2]) if v[2].isdigit() else 0
            visual_cell = (f"{OK} {v[2]} kept as the references" +
                           (f", {len(names)} changed: " + ", ".join(code(x) for x in names) if names else ", none changed"))
        else:
            names = [x for x in (v[3] if len(v) > 3 else "").split(",") if x]
            compared += int(v[2]) if v[2].isdigit() else 0
            changed_all += names
            failing.append(sc)
            visual_cell = f"{FAIL} {len(names)} changed: " + ", ".join(code(x) for x in names)
            if len(v) > 4 and read_text(v[4]):
                changed_logs.append((sc, read_text(v[4])))
        rows.append([code(sc), smoke_cell, n or "—", visual_cell, time_cell])
    n_sc = len(a.scenarios)
    if failing:
        verdict = f"{FAIL} {len(set(failing))} of {n_sc} scenario{'s' if n_sc > 1 else ''} failed"
    else:
        verdict = f"{OK} {n_sc} scenario{'s' if n_sc > 1 else ''} passed"
    heading = (f"## 📸 desktop-smoke ({a.shard}) — {verdict} · {screens} screens · "
               f"{compared} compared" + (f", {len(changed_all)} changed" if changed_all else ", unchanged") +
               f" · {fmt_seconds(seconds)}")
    out = [heading, "", table(["Scenario", "Smoke test", "Screens", "Visual check", "Time"], rows, "llrll"), ""]
    where = link(code(a.artifact), a.artifact_url) if a.artifact else "the screenshots artifact"
    out.append(f"📦 Screenshots: {where} · one `contact-sheet.png` per scenario · "
               "the smoke test's `app.log` next to them")
    if changed_all:
        out.append("")
        out.append(f"{WARN} A screen changed against main's fails the job. Its side-by-side image (main | now | changes "
                   "in red) is in `<scenario>/visual_diff/` of the artifact. If the change is meant, announce it on the "
                   "pull request's `Screens:` line (`tests/visual/README.md`): `Screens: smoke/01_dashboard, cadran/*`, or `any`.")
        for sc, text in changed_logs:
            out.append("")
            out.append(details(f"Visual check ({sc})", "```\n" + text.strip() + "\n```", open_=True))
    return "\n".join(out)


# ----------------------------------------------------------------------- map

MAP_SECTIONS = (".text", ".rodata", ".data", ".data.rel.ro", ".init_array", ".fini_array", ".eh_frame",
                ".gcc_except_table", ".tdata", ".bss", ".tbss")
MAP_FLASH = tuple(s for s in MAP_SECTIONS if s not in (".bss", ".tbss"))


def parse_map(path):
    """GNU ld's map: the size of every input section, by output section and
    by what it came from (an archive, or PlayGuard's own objects).
    Returns {origin: {output section: bytes}}."""
    out_re = re.compile(r"^(\.[\w.]+)\s+0x[0-9a-f]+\s+0x([0-9a-f]+)\s*$")
    in_re = re.compile(r"^ (\S+)(?:\s+0x[0-9a-f]+\s+0x([0-9a-f]+)\s+(\S.*))?$")
    cont_re = re.compile(r"^\s+0x[0-9a-f]+\s+0x([0-9a-f]+)\s+(\S.*)$")
    sizes = collections.defaultdict(collections.Counter)
    started, out, pending = False, None, False
    with open(path, encoding="utf-8", errors="replace") as f:
        for line in f:
            if not started:
                started = line.startswith("Linker script and memory map")
                continue
            m = out_re.match(line)
            if m:
                out = m.group(1)
                continue
            if out not in MAP_SECTIONS:
                continue
            m = in_re.match(line)
            if m:
                if m.group(2):
                    sizes[origin_of(m.group(3))][out] += int(m.group(2), 16)
                    pending = False
                else:
                    pending = True   # a long section name: its size is on the next line
                continue
            m = cont_re.match(line)
            if m and pending:
                sizes[origin_of(m.group(2))][out] += int(m.group(1), 16)
                pending = False
    return sizes


def origin_of(path):
    path = path.strip()
    m = re.match(r"^(.*?\.a)\((.*)\)$", path)
    if m:
        return os.path.basename(m.group(1))
    if "/playguard.dir/" in path or path.startswith("CMakeFiles/"):
        return "PlayGuard (own code)"
    return os.path.basename(path)


def map_table(path, nro=None, top=12):
    sizes = parse_map(path)
    flash = {o: sum(c[s] for s in MAP_FLASH) for o, c in sizes.items()}
    order = sorted(sizes, key=lambda o: -flash[o])
    total = sum(flash.values())
    biggest = max(flash.values()) if flash else 1
    rows = []
    others = collections.Counter()
    for i, o in enumerate(order):
        c = sizes[o]
        if i < top and flash[o] > 0:
            rows.append([o, fmt_size(flash[o]), fmt_size(c[".text"]), fmt_size(c[".rodata"]),
                         fmt_size(c[".data"] + c[".data.rel.ro"]), fmt_size(c[".bss"] + c[".tbss"]),
                         bar(100.0 * flash[o] / biggest, 14)])
        else:
            others.update(c)
    if others:
        o_flash = sum(others[s] for s in MAP_FLASH)
        rows.append([f"{len(order) - min(top, len(order))} others", fmt_size(o_flash), fmt_size(others[".text"]),
                     fmt_size(others[".rodata"]), fmt_size(others[".data"] + others[".data.rel.ro"]),
                     fmt_size(others[".bss"] + others[".tbss"]), bar(100.0 * o_flash / biggest, 14)])
    out = [table(["Library", "Size", "text", "rodata", "data", "bss (RAM)", ""], rows, "lrrrrrl")]
    note = f"Code and data {fmt_size(total)}"
    if nro and file_size(nro):
        note += f" · assets (icon, NACP, RomFS: layouts, strings, images) ≈ {fmt_size(file_size(nro) - total)}"
    note += f" · {len(order)} libraries in `{os.path.basename(path)}`"
    out.append("")
    out.append(note)
    return "\n".join(out)


def cmd_map(a):
    return "### 📏 What makes the .nro big\n\n" + map_table(a.map, a.nro, a.top)


# --------------------------------------------------------------------- build

def ccache_rate(path):
    """The hit rate of `ccache --print-stats` (key<tab>value lines), or of
    `ccache --show-stats` ("Hits: 12 / 15 (80.00 %)")."""
    text = read_text(path)
    if not text:
        return None
    stats = {}
    for line in text.splitlines():
        parts = line.split()
        if len(parts) == 2 and parts[1].isdigit():
            stats[parts[0]] = int(parts[1])
    hits = stats.get("direct_cache_hit", 0) + stats.get("preprocessed_cache_hit", 0)
    misses = stats.get("cache_miss", 0)
    if hits + misses:
        return 100.0 * hits / (hits + misses)
    m = re.search(r"Hits:\s+(\d+)\s*/\s*(\d+)", text)
    if m and int(m.group(2)):
        return 100.0 * int(m.group(1)) / int(m.group(2))
    return None


def zip_listing(path):
    try:
        with zipfile.ZipFile(path) as z:
            return [(i.filename, i.file_size) for i in z.infolist() if not i.filename.endswith("/")]
    except (OSError, zipfile.BadZipFile):
        return []


def cmd_build(a):
    nro = file_size(a.nro)
    sizes = {name: file_size(path) for name, path in (("playguard.nro", a.nro), ("playguard.zip", a.zip),
                                                      ("playguard-rescue.zip", a.rescue_zip),
                                                      ("playguard-rescue.nsp", a.rescue_nsp))}
    base = read_json(a.baseline) if a.baseline else None
    toolchain = dict(line.split(None, 1) for line in (read_text(a.toolchain) or "").splitlines() if " " in line)
    libnx = toolchain.get("libnx", "?")
    rate = ccache_rate(a.ccache) if a.ccache else None

    out = [f"## 🎮 Switch build — PlayGuard {a.version} · {a.ref} · {code(a.sha[:7])}", ""]
    strip = [f"{OK} `-Werror` clean", f"{OK} reproducible zips (two builds, same bytes)",
             f"{OK} libnx {libnx} ≥ {a.libnx_min}", f"{OK} LNY2 marker (hbmenu)"]
    if rate is not None:
        strip.append(f"⚡ ccache {rate:.0f} % hits")
    out += [" · ".join(strip), ""]

    # The sizes, the .nro against main's last build.
    heads = ["playguard.nro", "playguard.zip", "playguard-rescue.zip", "rescue .nsp"]
    tile = [f"**{fmt_size(sizes['playguard.nro'])}**" if nro else "—",
            fmt_size(sizes["playguard.zip"]) if sizes["playguard.zip"] else "—",
            fmt_size(sizes["playguard-rescue.zip"]) if sizes["playguard-rescue.zip"] else "—",
            fmt_size(sizes["playguard-rescue.nsp"]) if sizes["playguard-rescue.nsp"] else "—"]
    if base and base.get("size") and nro:
        heads.append(f"vs main {code(base.get('sha', '')[:7])}")
        delta = nro - base["size"]
        tile.append(fmt_delta_bytes(delta))
        if delta > a.max_growth:
            warn(".nro size", f"playguard.nro grew by {fmt_size(delta)} vs main ({base.get('sha', '')[:7]}): "
                              f"{fmt_size(nro)} now, {fmt_size(base['size'])} before")
    elif a.baseline:
        heads.append("vs main")
        tile.append("no baseline yet")
    out += [table(heads, [tile], "c" * len(heads)), ""]

    # Downloads: --artifact name|url|contents, one per artifact uploaded.
    if a.artifact:
        rows = []
        for spec in a.artifact:
            name, url, contents = (spec.split("|", 2) + ["", ""])[:3]
            rows.append([link(code(name), url), contents])
        out += ["### 📦 Downloads", "", table(["Artifact", "Contents"], rows), ""]

    if a.map and os.path.exists(a.map):
        out += ["### 📏 What makes the .nro big", "", map_table(a.map, a.nro, a.top), ""]

    # Toolchain: dkp-pacman's packages, the compiler, the image, the submodules.
    rows = [[code(k), code(v)] for k, v in toolchain.items()]
    if a.gcc:
        rows.append([code("aarch64-none-elf-gcc"), code(a.gcc)])
    info = read_text(a.build_info) or ""
    for line in info.splitlines():
        if line.startswith("image: "):
            rows.append(["image", code(line[len("image: "):])])
        elif re.match(r"^ ?[0-9a-f]{40} extern/", line):   # git submodule status: sha path (ref)
            parts = line.split()
            rows.append([code(parts[1]), code(parts[0][:7]) + (" " + " ".join(parts[2:]) if len(parts) > 2 else "")])
    out.append(details("🧰 Toolchain (build-info.txt)", table(["Component", "Version"], rows)))
    out.append("")
    # Checksums, compat.json and the zips' contents.
    sums = read_text(a.sums)
    if sums:
        rows = [[code(l.split()[1]), code(l.split()[0])] for l in sums.splitlines() if len(l.split()) == 2]
        out.append(details("SHA-256 (SHA256SUMS.txt)", table(["File", "SHA-256"], rows)))
        out.append("")
    compat = read_text(a.compat)
    if compat:
        out.append(details("compat.json (read by the app's update check)", "```json\n" + compat.strip() + "\n```"))
        out.append("")
    listing = []
    for name, path in (("playguard.zip", a.zip), ("playguard-rescue.zip", a.rescue_zip)):
        for entry, size in zip_listing(path):
            listing.append([code(name), code(entry), fmt_size(size)])
    if listing:
        out.append(details("Zip contents (SD-card layout)", table(["Zip", "Entry", "Size"], listing, "llr")))
        out.append("")
    if a.write_baseline and nro:
        write_json(a.write_baseline, {"sha": a.sha, "size": nro})
    return "\n".join(out)


# ------------------------------------------------------------------- release

def cmd_release(a):
    rows = []
    for name in a.files:
        path = os.path.join(a.dir, name)
        size = file_size(path)
        if size is None:
            rows.append([code(name), f"{FAIL} missing", ""])
            continue
        rows.append([link(code(name), f"{a.repo_url}/releases/download/{a.tag}/{name}"), fmt_size(size),
                     code(sha256_of(path))])
    out = [f"## 🚀 Release {a.tag} — assets attached to {link(a.tag, f'{a.repo_url}/releases/tag/{a.tag}')}", "",
           table(["Asset", "Size", "SHA-256"], rows, "lrl"), "",
           "Each asset carries a build provenance attestation. To check a download came from this repository's CI, unchanged:",
           "", "```", f"gh attestation verify playguard.nro -R {a.repo_url.split('github.com/')[-1]}", "```", ""]
    info = read_text(os.path.join(a.dir, "build-info.txt"))
    if info:
        out.append(details("build-info.txt", "```\n" + info.strip() + "\n```"))
    return "\n".join(out)


# ---------------------------------------------------------------------- main

# ---------------------------------------------------------------- clang-tidy

SOURCE_EXT = (".c", ".cpp")
HEADER_EXT = {".h": ".c", ".hpp": ".cpp"}


def cmd_tidy_files(a):
    """The files run-clang-tidy is given: of the changed files (one a line),
    the sources under source/ and the source file of a changed header (the
    same stem, .c for .h, .cpp for .hpp), those the compile commands hold,
    as they name them (an absolute path, which run-clang-tidy matches)."""
    db = read_json(a.compile_commands) or []
    compiled = {os.path.normpath(e["file"]) for e in db if "file" in e}
    wanted = set()
    for line in (read_text(a.changed) or "").splitlines():
        path = line.strip()
        if not path.startswith("source/"):
            continue
        stem, ext = os.path.splitext(path)
        if ext in SOURCE_EXT:
            wanted.add(path)
        elif ext in HEADER_EXT:
            for src_ext in (HEADER_EXT[ext],) + SOURCE_EXT:
                if os.path.exists(stem + src_ext):
                    wanted.add(stem + src_ext)
                    break
    return "\n".join(sorted(f for f in compiled if os.path.relpath(f) in wanted))


FINDING = re.compile(r"^(?P<file>[^\s:][^:]*?):(?P<line>\d+):(?P<col>\d+): (?P<kind>warning|error): (?P<text>.*?)"
                     r"(?: \[(?P<checks>[\w.,-]+)\])?$")


def cmd_tidy(a):
    """run-clang-tidy's output as a report: the findings by check, each
    once (a header's is printed once per file that includes it), and the
    list of them, folded."""
    log = read_text(a.log) or ""
    root = os.getcwd() + os.sep
    findings, errors = {}, []
    for line in log.splitlines():
        m = FINDING.match(line)
        if not m:
            continue
        path = m["file"]
        if path.startswith(root):
            path = path[len(root):]
        if "extern/" in path:   # borealis' own: the matcher leaves them out too
            continue
        checks = m["checks"] or ""
        key = (path, int(m["line"]), int(m["col"]), checks, m["text"])
        if m["kind"] == "error" and not checks:
            errors.append(key)   # clang-tidy could not read the file (a compile error)
        else:
            findings[key] = checks.split(",")[0] if checks else "clang-diagnostic"
    checked = [l for l in (read_text(a.files) or "").splitlines() if l.strip()]
    changed = [l.strip() for l in (read_text(a.changed) or "").splitlines() if l.strip().startswith("source/")]
    left_out = sorted(f for f in changed if os.path.splitext(f)[1] in SOURCE_EXT
                      and not any(c.endswith(os.sep + f) or c == f for c in checked))
    by_check = collections.Counter(findings.values())
    files_hit = len({k[0] for k in findings})
    n = len(findings)
    if errors:
        verdict = f"{FAIL} clang-tidy could not read {len({e[0] for e in errors})} file(s)"
    elif not checked:
        verdict = f"{SKIP} nothing to check"
    elif n:
        verdict = (f"{WARN} {n} finding{'s' if n != 1 else ''} in {files_hit} file{'s' if files_hit != 1 else ''}, "
                   f"{len(checked)} checked")
    else:
        verdict = f"{OK} no finding in {len(checked)} file{'s' if len(checked) != 1 else ''}"
    what = "bugprone, cert, performance" + (" and the static analyzer" if a.analyzer == "1" else "")
    heading = f"## 🧹 clang-tidy — {verdict} · {what}" + (f" · clang-tidy {a.version}" if a.version else "")
    out = [heading, ""]
    if left_out:
        out.append(f"{SKIP} Changed but not compiled by the desktop build (the Switch's alone), not checked: "
                   + ", ".join(code(f) for f in left_out))
        out.append("")
    if errors:
        out.append(table(["File", "Error"], [[code(f"{e[0]}:{e[1]}"), e[4][:200]] for e in errors[:20]]))
        out.append("")
    if n:
        rows = [[c, code(name), len({k[0] for k, v in findings.items() if v == name})]
                for name, c in by_check.most_common()]
        out.append(table(["Findings", "Check", "Files"], rows, "rlr"))
        out.append("")
        lines = [f"{k[0]}:{k[1]}:{k[2]}: {k[4]} [{k[3]}]" for k in sorted(findings)]
        shown = lines[: a.max_lines]
        body = "```\n" + "\n".join(shown) + "\n```"
        if len(lines) > len(shown):
            body += f"\n\n… and {len(lines) - len(shown)} more: the `clang-tidy-<run>` artifact has them all."
        out.append(details(f"Every finding ({n})", body))
        out.append("")
    out.append("A finding is a claim to check, not a failure: this job never fails on one. "
               "The checks and why each left-out one is: `.clang-tidy`.")
    return "\n".join(out)


def main():
    ap = argparse.ArgumentParser(description=__doc__.split("\n\n")[0], formatter_class=argparse.RawDescriptionHelpFormatter)
    sub = ap.add_subparsers(dest="command", required=True)

    p = sub.add_parser("header", help="the run: version, commit, branch or pull request")
    p.add_argument("--label", required=True)
    p.add_argument("--version", required=True)
    p.add_argument("--sha", required=True)
    p.add_argument("--branch", default="")
    p.add_argument("--base", default="")
    p.add_argument("--pr", default="")
    p.add_argument("--tag", default="")
    p.add_argument("--title", default="")
    p.add_argument("--event", default="")
    p.add_argument("--repo-url", default="")
    p.add_argument("--runner", default="")
    p.add_argument("--image", default="")
    p.set_defaults(func=cmd_header)

    p = sub.add_parser("host-tests", help="the suites of make test")
    p.add_argument("dir", help="build/host-tests")
    p.add_argument("--makefile", default="Makefile")
    p.add_argument("--compiler", default="")
    p.add_argument("--flags", default="")
    p.set_defaults(func=cmd_host_tests)

    p = sub.add_parser("fuzz", help="the libFuzzer runs of make fuzz")
    p.add_argument("dir", help="build/fuzz")
    p.add_argument("--makefile", default="Makefile")
    p.add_argument("--seconds", default="")
    p.set_defaults(func=cmd_fuzz)

    p = sub.add_parser("coverage", help="gcovr --json-summary")
    p.add_argument("json")
    p.add_argument("--scope", default="")
    p.add_argument("--baseline")
    p.add_argument("--write-baseline")
    p.add_argument("--sha", default="")
    p.add_argument("--max-drop", type=float, default=1.0, help="points of line coverage lost vs main that warn")
    p.set_defaults(func=cmd_coverage)

    p = sub.add_parser("smoke", help="a desktop-smoke shard")
    p.add_argument("--shard", required=True)
    p.add_argument("--scenarios", nargs="+", required=True)
    p.add_argument("--results", required=True, help="smoke-results.tsv")
    p.add_argument("--visual", help="visual-results.tsv")
    p.add_argument("--artifact", default="")
    p.add_argument("--artifact-url", default="")
    p.set_defaults(func=cmd_smoke)

    p = sub.add_parser("map", help="what makes the .nro big, from the linker map")
    p.add_argument("map", help="build/borealis.map")
    p.add_argument("--nro", help="playguard.nro, for the assets' share")
    p.add_argument("--top", type=int, default=12)
    p.set_defaults(func=cmd_map)

    p = sub.add_parser("build", help="the Switch build")
    p.add_argument("--version", required=True)
    p.add_argument("--ref", required=True)
    p.add_argument("--sha", required=True)
    p.add_argument("--nro", default="playguard.nro")
    p.add_argument("--zip", default="playguard.zip")
    p.add_argument("--rescue-zip", default="playguard-rescue.zip")
    p.add_argument("--rescue-nsp", default="sysmodule/out/playguard-rescue.nsp")
    p.add_argument("--toolchain", default="toolchain.txt")
    p.add_argument("--build-info", default="build-info.txt")
    p.add_argument("--sums", default="SHA256SUMS.txt")
    p.add_argument("--compat", default="compat.json")
    p.add_argument("--ccache", help="ccache --print-stats output")
    p.add_argument("--gcc", default="", help="aarch64-none-elf-gcc -dumpfullversion")
    p.add_argument("--libnx-min", default="4.12.0")
    p.add_argument("--map", help="build/borealis.map")
    p.add_argument("--top", type=int, default=12)
    p.add_argument("--artifact", action="append", help="name|url|contents, once per artifact")
    p.add_argument("--baseline")
    p.add_argument("--write-baseline")
    p.add_argument("--max-growth", type=int, default=100 * 1024, help="bytes of .nro growth vs main that warn")
    p.set_defaults(func=cmd_build)

    p = sub.add_parser("release", help="the assets of a release")
    p.add_argument("--tag", required=True)
    p.add_argument("--repo-url", required=True)
    p.add_argument("--dir", default="dist")
    p.add_argument("--files", nargs="+", default=["playguard.nro", "playguard.zip", "playguard-rescue.zip", "compat.json",
                                                  "build-info.txt", "SHA256SUMS.txt"])
    p.set_defaults(func=cmd_release)

    p = sub.add_parser("tidy-files", help="the files clang-tidy checks: the changed sources the desktop build compiles")
    p.add_argument("changed", help="the changed files, one a line (a pull request's, or every file of source/)")
    p.add_argument("compile_commands", help="build-tidy/compile_commands.json")
    p.set_defaults(func=cmd_tidy_files)

    p = sub.add_parser("tidy", help="the findings of run-clang-tidy, by check")
    p.add_argument("log", help="run-clang-tidy's output")
    p.add_argument("--files", default="files.txt", help="the files it was given")
    p.add_argument("--changed", default="changed.txt", help="the changed files it was chosen from")
    p.add_argument("--analyzer", default="0", help="1 when the static analyzer's checks ran")
    p.add_argument("--version", default="")
    p.add_argument("--max-lines", type=int, default=400, help="findings listed in the folded block")
    p.set_defaults(func=cmd_tidy)

    a = ap.parse_args()
    print(a.func(a))
    return 0


if __name__ == "__main__":
    sys.exit(main())
