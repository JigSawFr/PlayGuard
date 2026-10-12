#!/usr/bin/env python3
"""The notes of a GitHub release, laid out for reading.

  tools/release_notes.py --version 1.1.0 --repo-url https://github.com/o/r \\
      [--summary SUMMARY] [--compat compat.json] [--assets assets.json] \\
      [--compare compare.json] [--run-url URL] < notes.md > new-notes.md

release-please writes the release's notes from the conventional commits: a
"## <version>" heading, then one line per commit under "### " sections. This
keeps the heading and lays out:

  under the heading (what the console's update screen reads)
    a line of figures (features, fixes, … and the full diff), in HTML;
    the summary (.github/release-notes/<version>.md, or one written by
    tools/summarize_release.py), when there is one;
    "Every change in X.Y.Z": the sections a user notices, folded;
    "For contributors": build, tests, documentation, folded;
  then, for a computer
    "## 🖼️ On screen": the screens whose visual-check reference changed
    since the previous release (tests/visual/), before and after;
    "## 📦 Install": the assets, their size and what each is for, the
    firmware checked (compat.json);
    "## 🔍 Build": the build's report (the run page: sizes, tests, coverage,
    screenshots, toolchain, checksums), how to check a download, who
    contributed.

assets.json is `gh release view --json assets`; compare.json is the GitHub
API's compare of the previous tag with this one (its commits give the
contributors, its files the screens). Each is optional: what it feeds is left
out without it.

The console reads the notes under the version's heading and stops at the next
"## ": it shows the summary and the lists, without the HTML lines
(source/util/changelog.cpp). It keeps their first 16 KB only, hence the
summary first.

Notes laid out before are laid out again from the lists in their <details>
blocks, keeping their summary unless another is given: the same input gives
the same page, so a re-run edits nothing. A summary inserted in the notes
before this layout existed (its own "### Highlights" section) is dropped from
the lists. Standard library only.
"""
import argparse
import json
import re
import sys

# release-please-config.json's sections, by their title without the emoji:
# these are for contributors, the others for everyone.
CONTRIBUTOR_SECTIONS = {"Other": "build", "Tests": "tests", "Documentation": "docs"}
# The figures line, by section title: (emoji, singular, plural).
FIGURES = {
    "BREAKING CHANGES": ("⚠️", "breaking change", "breaking changes"),
    "Features": ("🚀", "feature", "features"),
    "Changes": ("⚠️", "change", "changes"),
    "Bug Fixes": ("🐞", "fix", "fixes"),
    "Polish": ("✨", "polish", "polish"),
}
MAX_SCREENS = 12

INSTALL = """## 📦 Install

| File | Size | What it is for |
|---|---|---|
| [`playguard.zip`]({dl}/playguard.zip) | {size_zip} | **New install**: extract it to the root of the SD card. The app lands in `sd:/switch/playguard/`, its files in `sd:/config/playguard/`. |
| [`playguard.nro`]({dl}/playguard.nro) | {size_nro} | The app alone, in place of `sd:/switch/playguard/playguard.nro`. |
| [`playguard-rescue.zip`]({dl}/playguard-rescue.zip) | {size_rescue} | Optional: the recovery sysmodule, for a [forgotten PIN]({repo}#locked-out-second-hand-console-forgotten-pin). |

**Already installed?** Update from PlayGuard itself (*About › Update*), sphaira or the Homebrew App Store.

**Requires** custom firmware (Atmosphère){firmware}. [Compatibility]({repo}#compatibility)"""


def strip_emoji(s: str) -> str:
    return "".join(c for c in s if ord(c) < 0x2600 or 0x2BFF < ord(c) < 0x1F000 and not 0xFE00 <= ord(c) <= 0xFE0F).strip()


def sections(text: str):
    """[(title without emoji, block text)], the text before the first "### " as ("", …)."""
    out, title, block = [], "", []
    for line in text.splitlines():
        if line.startswith("### "):
            if title or any(l.strip() for l in block):
                out.append((title, "\n".join(block).strip("\n")))
            title, block = strip_emoji(line[4:]), [line]
        else:
            block.append(line)
    if title or any(l.strip() for l in block):
        out.append((title, "\n".join(block).strip("\n")))
    return out


def items(block: str) -> int:
    return sum(1 for line in block.splitlines() if line.startswith(("* ", "- ")))


def fmt_size(n) -> str:
    if not isinstance(n, int) or n <= 0:
        return "—"
    return f"{n / 1048576:.1f} MB" if n >= 1048576 else f"{max(1, round(n / 1024))} KB"


def plural(n: int, word: str) -> str:
    return f"{n} {word}{'s' if n != 1 else ''}"


def folded(label: str, what: str, body: str) -> str:
    return f"<details>\n<summary><b>{label}</b> ({what})</summary>\n\n{body}\n\n</details>"


def screens(compare: dict, repo_slug: str, prev: str, tag: str):
    """The visual-check references added or changed between the two tags."""
    raw = f"https://raw.githubusercontent.com/{repo_slug}"
    rows = []
    for f in compare.get("files") or []:
        name, status = f.get("filename", ""), f.get("status", "")
        if not (name.startswith("tests/visual/") and name.endswith(".png")) or status not in ("added", "modified", "renamed"):
            continue
        before = f.get("previous_filename", name) if status != "added" else ""
        rows.append((name[len("tests/visual/"):-4], f"{raw}/{prev}/{before}" if before else "", f"{raw}/{tag}/{name}"))
    return sorted(rows)


def unlayout(notes: str):
    """(release-please's notes, the summary on the page) of notes this laid
    out before; notes it did not lay out come back as they are, with no
    summary. The lists are taken back out of their <details> blocks."""
    lines = notes.splitlines()
    if "<details>" not in lines:
        return notes, ""
    at = next((i for i, line in enumerate(lines) if line.startswith("## ")), -1)
    end = next((i for i in range(at + 1, len(lines)) if lines[i].startswith("## ")), len(lines))
    first = next((i for i in range(at + 1, end) if lines[i] == "<details>"), end)
    # The summary: what stands before the lists, the figures line aside
    # (above or below it, as this laid it out before).
    summary = "\n".join(l for l in lines[at + 1 : first] if not l.startswith("<p")).strip()
    lists, inside = [], None
    for line in lines[first:end]:
        if line.startswith("<summary>"):
            inside = []
        elif line == "</details>":
            if inside is not None:
                lists.append("\n".join(inside).strip("\n"))
            inside = None
        elif inside is not None:
            inside.append(line)
    head = "\n".join(lines[: at + 1]) if at >= 0 else ""
    return head + "\n\n" + "\n\n".join(lists) + "\n", summary


def airy(summary: str) -> str:
    """A blank line between two top-level bullets: on GitHub, a list that
    breathes. (The console reads it the same.)"""
    out = []
    for line in summary.splitlines():
        if line.startswith(("* ", "- ")) and out and out[-1].strip() and not out[-1].startswith("#"):
            out.append("")
        out.append(line)
    return "\n".join(out)


def layout(notes: str, version: str, repo: str, summary: str = "", firmware: str = "",
           assets=None, compare=None, run_url: str = "") -> str:
    notes, existing = unlayout(notes)
    summary = airy((summary or existing).strip())
    lines = notes.splitlines(keepends=True)
    at = next((i for i, line in enumerate(lines) if line.startswith("## ")), None)
    head = "".join(lines[: at + 1]).rstrip("\n") if at is not None else f"## {version}"
    rest = "".join(lines[at + 1 :] if at is not None else lines)
    # An older summary inserted in the notes (its own "### Highlights") goes:
    # the page has one summary, above the lists.
    parts = [(t, b) for t, b in sections(rest.strip("\n")) if t != "Highlights"]
    users = [(t, b) for t, b in parts if t not in CONTRIBUTOR_SECTIONS]
    contributors = [(t, b) for t, b in parts if t in CONTRIBUTOR_SECTIONS]

    tag = f"v{version}"
    slug = repo.split("://", 1)[-1].split("/", 1)[-1]
    m = re.search(r"/compare/([^/\s]+?)\.\.\.", head)
    prev = m.group(1) if m else ""

    out = [head]
    figures = []
    for title, (emoji, one, many) in FIGURES.items():
        n = sum(items(b) for t, b in users if t == title)
        if n:
            figures.append(f"{emoji} <b>{n}</b> {one if n == 1 else many}")
    if prev:
        figures.append(f'<a href="{repo}/compare/{prev}...{tag}">Full diff {prev}…{tag}</a>')
    if figures:
        out.append("<p>" + " · ".join(figures) + "</p>")
    if summary:
        out.append(summary)
    if users:
        out.append(folded(f"📜 Every change in {version}", plural(sum(items(b) for _, b in users), "change"), "\n\n".join(b for _, b in users)))
    if contributors:
        names = ", ".join(CONTRIBUTOR_SECTIONS[t] for t, _ in contributors)
        out.append(folded(f"🛠️ For contributors: {names}", plural(sum(items(b) for _, b in contributors), "change"),
                          "\n\n".join(b for _, b in contributors)))

    shown = screens(compare or {}, slug, prev, tag) if prev else []
    if shown:
        rows = ["| Screen | Before | After |", "|---|---|---|"]
        for name, before, after in shown[:MAX_SCREENS]:
            b = f'<img src="{before}" width="320" alt="{name}, before">' if before else "*new*"
            rows.append(f'| `{name}` | {b} | <img src="{after}" width="320" alt="{name}, after"> |')
        more = len(shown) - MAX_SCREENS
        if more > 0:
            rows.append(f"\n…and {more} more: [every changed file]({repo}/compare/{prev}...{tag}#files_bucket).")
        out.append("## 🖼️ On screen\n\n" + folded("Screens that look different",
                                                   plural(len(shown), "screen") + ", before and after", "\n".join(rows)))

    sizes = {a.get("name"): a.get("size") for a in (assets or []) if isinstance(a, dict)}
    dl = f"{repo}/releases/download/{tag}"
    out.append(INSTALL.format(dl=dl, repo=repo, firmware=firmware, size_zip=fmt_size(sizes.get("playguard.zip")),
                              size_nro=fmt_size(sizes.get("playguard.nro")),
                              size_rescue=fmt_size(sizes.get("playguard-rescue.zip"))))

    build = ["## 🔍 Build", ""]
    if run_url:
        build.append(f"**[Build report]({run_url})**: the `.nro` size and what makes it big, the tests and their "
                     "coverage, every screen of the smoke test, the toolchain, the checksums.")
        build.append("")
    build.append(f"**Check a download**: `SHA256SUMS.txt` lists the files' checksums; "
                 f"`gh attestation verify playguard.nro -R {slug}` checks it was built by this repository's CI.")
    people = sorted({(c.get("author") or {}).get("login", "") for c in (compare or {}).get("commits") or []} - {""},
                    key=str.lower)
    people = [p for p in people if not p.endswith("[bot]")]
    if people:
        build += ["", "**Contributors**: " + ", ".join(f"@{p}" for p in people)]
    out.append("\n".join(build))
    return "\n\n".join(out) + "\n"


def firmware_line(path: str) -> str:
    try:
        with open(path, encoding="utf-8") as f:
            c = json.load(f)
        low, high = c.get("fw_min_play_timer"), c.get("fw_tested_max")
    except (OSError, ValueError, AttributeError):
        return ""
    return f", firmware {low} → {high} checked" if low and high else ""


def read_json(path):
    try:
        with open(path, encoding="utf-8") as f:
            return json.load(f)
    except (OSError, ValueError):
        return None


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__.strip().splitlines()[0])
    ap.add_argument("--version", required=True, help="1.1.0 (a leading v is dropped)")
    ap.add_argument("--repo-url", required=True, help="https://github.com/owner/repo")
    ap.add_argument("--summary", help="the summary file, if any")
    ap.add_argument("--compat", help="the release's compat.json, for the firmware line")
    ap.add_argument("--assets", help="gh release view --json assets, for the sizes")
    ap.add_argument("--compare", help="the API's compare of the previous tag, for the screens and contributors")
    ap.add_argument("--run-url", default="", help="the run that built the release, for the build report")
    ap.add_argument("--raw", action="store_true", help="only print release-please's notes, out of a laid-out page")
    ap.add_argument("--existing-summary", action="store_true", help="only print the summary a laid-out page has")
    a = ap.parse_args()
    if a.raw or a.existing_summary:
        raw, existing = unlayout(sys.stdin.read())
        sys.stdout.write(raw if a.raw else (existing + "\n" if existing else ""))
        return 0
    summary = ""
    if a.summary:
        with open(a.summary, encoding="utf-8") as f:
            summary = f.read()
    assets = (read_json(a.assets) or {}).get("assets") if a.assets else None
    compare = read_json(a.compare) if a.compare else None
    sys.stdout.write(layout(sys.stdin.read(), a.version.lstrip("v"), a.repo_url.rstrip("/"), summary,
                            firmware_line(a.compat) if a.compat else "", assets,
                            compare if isinstance(compare, dict) else None, a.run_url))
    return 0


if __name__ == "__main__":
    sys.exit(main())
