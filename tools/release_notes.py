#!/usr/bin/env python3
"""The notes of a GitHub release, laid out for reading.

  tools/release_notes.py --version 1.1.0 --repo-url https://github.com/o/r \\
      [--summary SUMMARY] [--compat compat.json] [--assets assets.json] \\
      [--compare compare.json] [--contributors logins.json] [--run-url URL] \\
      < notes.md > new-notes.md

release-please writes the release's notes from the conventional commits: a
"## <version> (<date>)" heading, then one line per commit under "### "
sections. This lays out, under "## <version> · <date>" (what the console's
update screen reads, up to the next "## "):

  a download line and a line of figures (breaking changes, features, fixes,
  polish, the commits), in HTML;
  the summary (.github/release-notes/<version>.md, or one written by
  tools/summarize_release.py), when there is one: its bullets on two lines
  (the bold lead-in, then the text), a closing paragraph as an "Important"
  box;
  "What changed": the sections a user notices, then those for contributors
  (build, tests, docs), each folded;
then, for a computer:
  "## 🖼️ On screen": the screens that look different from the previous
  release (its playguard-screens.zip against this one's, --screens: the
  lists and the before/after image attached to the release);
  "## 📦 Install": three steps (new install, update, locked out) with the
  assets and their size, the firmware checked (compat.json);
  "## 🔍 Build": the build's report (the run page), the checksums and the
  attestation, the contributors.

What is for GitHub only is HTML, or Markdown inside "<!-- console:hide -->" …
"<!-- /console -->"; what the console reads instead is in a
"<!-- console: … -->" comment (source/util/changelog.cpp). The console keeps
the notes' first 16 KB only, hence the summary first.

assets.json is `gh release view --json assets`; compare.json the GitHub API's
compare of the previous tag with this one (its commits give the contributors,
its files the screens); logins.json a list of logins, the contributors of a
first release. Each is optional: what it feeds is left out without it.

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
# The website (tools/build_site.py): the latest release's documentation at docs/.
SITE_URL = "https://playguard.link/"
HIDE, SHOW = "<!-- console:hide -->", "<!-- /console -->"

INSTALL = """## 📦 Install

1. **New install**: download [`playguard.zip`]({dl}/playguard.zip){size_zip} and extract it to the **root of the SD card**.
   The app lands in `sd:/switch/playguard/`, its files in `sd:/config/playguard/`.

2. **Already installed?** Update from PlayGuard itself (*About › Update*), sphaira or the Homebrew App Store. The app alone: [`playguard.nro`]({dl}/playguard.nro){size_nro}.

3. **Locked out?** [`playguard-rescue.zip`]({dl}/playguard-rescue.zip){size_rescue} is the optional recovery sysmodule, for a [forgotten PIN]({site}docs/troubleshooting/#locked-out).

> [!NOTE]
> Requires custom firmware (**Atmosphère**).{firmware} [Compatibility]({site}docs/compatibility/)"""


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
        return ""
    return f"{n / 1048576:.1f} MB" if n >= 1048576 else f"{max(1, round(n / 1024))} KB"


def plural(n: int, word: str) -> str:
    return f"{n} {word}{'s' if n != 1 else ''}"


def folded(label: str, what: str, body: str) -> str:
    return f"<details>\n<summary><b>{label}</b> ({what})</summary>\n\n{body}\n\n</details>"


def screens_section(shown: dict, repo: str, tag: str) -> str:
    """The "On screen" section from tools/visual_refs.py diff's JSON: the
    screens changed, new and gone since the previous release, the
    before/after image attached to the release (its `sheet`), the zip."""
    changed, new, gone = shown.get("changed") or [], shown.get("new") or [], shown.get("gone") or []
    if not (changed or new or gone):
        return ""
    body = []
    if shown.get("sheet"):
        body.append(f'<img src="{repo}/releases/download/{tag}/{shown["sheet"]}" width="100%" '
                    f'alt="the screens that changed, before and after">')
    if changed:
        body.append("Changed: " + ", ".join(f"`{s}`" for s in changed))
    if new:
        body.append("New: " + ", ".join(f"`{s}`" for s in new))
    if gone:
        body.append("Gone: " + ", ".join(f"`{s}`" for s in gone))
    body.append(f"Every reference screen of this release: [`playguard-screens.zip`]({repo}/releases/download/{tag}/playguard-screens.zip).")
    n = len(changed) + len(new) + len(gone)
    return "## 🖼️ On screen\n\n" + folded("Screens that look different", plural(n, "screen") + ", before and after",
                                             "\n\n".join(body))


# --- the summary, plain (as written) and for the page ----------------------

LEAD_IN = re.compile(r"^([*-] (?:\S+ )?\*\*[^*]+\*\*:)\s*(.*)$")


def plain(summary: str) -> str:
    """A summary as written, out of what github() makes of it (or of an older
    layout): the GitHub-only blocks and HTML lines dropped, a console line
    back as a paragraph, a bullet's two lines joined."""
    out, hide, comment = [], False, False
    for line in summary.splitlines():
        t = line.strip()
        if hide:
            hide = t != SHOW
            continue
        if comment:
            comment = "-->" not in t
            continue
        if t == HIDE:
            hide = True
        elif t.startswith("<!-- console:") and t.endswith("-->"):
            out += ["", t[len("<!-- console:"):-3].strip(), ""]
        elif t.startswith("<!--"):
            comment = not t.endswith("-->")
        elif t.startswith("<") and t.endswith(">"):
            continue
        elif out and LEAD_IN.match(out[-1]) and not LEAD_IN.match(out[-1]).group(2) and line.startswith("  ") and t:
            out[-1] = out[-1].rstrip() + " " + t
        else:
            out.append(line.rstrip())
    return re.sub(r"\n{3,}", "\n\n", "\n".join(out)).strip()


def github(summary: str) -> str:
    """The summary for the page: a blank line between bullets, each bullet's
    bold lead-in on a line of its own and its text, capitalised, under it; a
    paragraph after the last bullet in an "Important" box, with the same
    words in a console line."""
    blocks = [b for b in re.split(r"\n\s*\n", plain(summary)) if b.strip()]
    last = max((i for i, b in enumerate(blocks) if b.lstrip().startswith(("* ", "- "))), default=-1)
    out = []
    for i, block in enumerate(blocks):
        if block.lstrip().startswith(("* ", "- ")):
            for b in re.split(r"\n(?=[*-] )", block):
                b = " ".join(l.strip() for l in b.splitlines())
                m = LEAD_IN.match(b)
                if m and m.group(2):
                    text = m.group(2)
                    out.append(f"{m.group(1)}  \n  {text[:1].upper()}{text[1:]}")
                else:
                    out.append(b)
        elif last >= 0 and i > last and not block.startswith("#"):
            words = " ".join(l.strip() for l in block.splitlines())
            box = "\n".join("> " + l for l in block.splitlines())
            out.append(f"{HIDE}\n> [!IMPORTANT]\n{box}\n{SHOW}\n<!-- console: {words.replace('-->', '->')} -->")
        else:
            out.append(block)
    return "\n\n".join(out)


# --- the page ---------------------------------------------------------------

def heading(line: str) -> str:
    """ "## [1.1.0](…) (2026-10-10)" → "## [1.1.0](…) · 2026-10-10"."""
    m = re.match(r"^(## .*?) \((\d{4}-\d\d-\d\d)\)\s*$", line)
    return f"{m.group(1)} · {m.group(2)}" if m else line.rstrip()


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
    summary = plain("\n".join(lines[at + 1 : first]))
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
    head = lines[at] if at >= 0 else ""
    return head + "\n\n" + "\n\n".join(lists) + "\n", summary


def layout(notes: str, version: str, repo: str, summary: str = "", firmware: str = "",
           assets=None, compare=None, run_url: str = "", contributors=None, screens_diff=None) -> str:
    notes, existing = unlayout(notes)
    summary = (summary or existing).strip()
    lines = notes.splitlines(keepends=True)
    at = next((i for i, line in enumerate(lines) if line.startswith("## ")), None)
    head = heading(lines[at]) if at is not None else f"## {version}"
    rest = "".join(lines[at + 1 :] if at is not None else lines)
    # An older summary inserted in the notes (its own "### Highlights") goes:
    # the page has one summary, above the lists.
    parts = [(t, b) for t, b in sections(rest.strip("\n")) if t != "Highlights"]
    users = [(t, b) for t, b in parts if t not in CONTRIBUTOR_SECTIONS]
    contrib = [(t, b) for t, b in parts if t in CONTRIBUTOR_SECTIONS]

    tag = f"v{version}"
    slug = repo.split("://", 1)[-1].split("/", 1)[-1]
    m = re.search(r"/compare/([^/\s]+?)\.\.\.", head)
    prev = m.group(1) if m else ""
    dl = f"{repo}/releases/download/{tag}"
    sizes = {a.get("name"): fmt_size(a.get("size")) for a in (assets or []) if isinstance(a, dict)}

    def size(name):
        return f" ({sizes[name]})" if sizes.get(name) else ""

    out = [head]
    out.append(f'<p>⬇️ <a href="{dl}/playguard.zip"><b>Download playguard.zip</b></a>{size("playguard.zip")} · '
               f'<a href="{dl}/playguard.nro">the app alone</a>{size("playguard.nro")} · '
               f'<a href="#-install">how to install</a></p>')
    figures = []
    for title, (emoji, one, many) in FIGURES.items():
        n = sum(items(b) for t, b in users if t == title)
        if n:
            label = f"{emoji} <b>{n}</b> {one if n == 1 else many}"
            figures.append(f'<a href="#every-change">{label}</a>' if title == "BREAKING CHANGES" else label)
    commits = (compare or {}).get("total_commits")
    if prev:
        what = plural(commits, "commit") if isinstance(commits, int) else "every commit"
        figures.append(f'<a href="{repo}/compare/{prev}...{tag}">{what} since {prev}</a>')
    else:
        figures.append(f'<a href="{repo}/commits/{tag}">every commit</a>')
    out.append("<p>" + " · ".join(figures) + "</p>")
    out.append("<hr>")
    if summary:
        out += [github(summary), "<hr>"]

    if users or contrib:
        out.append(f"{HIDE}\n### 📜 What changed\n{SHOW}")
        out.append('<a name="every-change"></a>')
    if users:
        out.append(folded(f"Every change in {version}", plural(sum(items(b) for _, b in users), "change"),
                          "\n\n".join(b for _, b in users)))
    if contrib:
        names = ", ".join(CONTRIBUTOR_SECTIONS[t] for t, _ in contrib)
        out.append(folded(f"🛠️ For contributors: {names}", plural(sum(items(b) for _, b in contrib), "change"),
                          "\n\n".join(b for _, b in contrib)))

    section = screens_section(screens_diff, repo, tag) if screens_diff else ""
    if section:
        out.append(section)

    out.append(INSTALL.format(dl=dl, site=SITE_URL, firmware=firmware, size_zip=size("playguard.zip"),
                              size_nro=size("playguard.nro"), size_rescue=size("playguard-rescue.zip")))

    build = []
    if run_url:
        build.append(f"[**Build report**]({run_url}) (size, tests, coverage, screenshots, toolchain)")
    build.append("`SHA256SUMS.txt`")
    build.append(f"provenance: `gh attestation verify playguard.nro -R {slug}`")
    people = {(c.get("author") or {}).get("login", "") for c in (compare or {}).get("commits") or []}
    if not compare:
        people |= set(contributors or [])
    people = sorted((p for p in people if p and not p.endswith("[bot]")), key=str.lower)
    section = "## 🔍 Build\n\n" + " · ".join(build)
    if people:
        section += "\n\n**Contributors**: " + ", ".join(f"@{p}" for p in people)
    out.append(section)
    return "\n\n".join(out) + "\n"


def firmware_line(path: str) -> str:
    try:
        with open(path, encoding="utf-8") as f:
            c = json.load(f)
        low, high = c.get("fw_min_play_timer"), c.get("fw_tested_max")
    except (OSError, ValueError, AttributeError):
        return ""
    return f" Checked on firmware **{low} → {high}**." if low and high else ""


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
    ap.add_argument("--compare", help="the API's compare of the previous tag, for the contributors")
    ap.add_argument("--screens", help="tools/visual_refs.py diff's JSON: the screens that look different, their image")
    ap.add_argument("--contributors", help="a JSON list of logins, the contributors when there is no compare")
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
    contributors = read_json(a.contributors) if a.contributors else None
    screens_diff = read_json(a.screens) if a.screens else None
    sys.stdout.write(layout(sys.stdin.read(), a.version.lstrip("v"), a.repo_url.rstrip("/"), summary,
                            firmware_line(a.compat) if a.compat else "", assets,
                            compare if isinstance(compare, dict) else None, a.run_url,
                            contributors if isinstance(contributors, list) else None,
                            screens_diff if isinstance(screens_diff, dict) else None))
    return 0


if __name__ == "__main__":
    sys.exit(main())
