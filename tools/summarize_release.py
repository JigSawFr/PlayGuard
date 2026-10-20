#!/usr/bin/env python3
"""A summary of a release's notes, written by a language model through
OpenRouter, for when no one wrote .github/release-notes/<version>.md.

  OPENROUTER_API_KEY=… tools/summarize_release.py VERSION [EXAMPLES_DIR [EMOJI_DIR]] < notes.md > summary.md

notes.md is the list release-please wrote (one line per commit). With
GH_TOKEN and GH_REPO set, the descriptions of the pull requests the notes
name (in the sections a user notices) are read from the GitHub API and given
too: a commit title says what changed, the description what it is for. A
pull request that cannot be read is left out. The other
summaries in EXAMPLES_DIR (.github/release-notes/*.md) are given as examples
of the tone and the form; the emoji EMOJI_DIR (resources/img/emoji/) has an
image of are the ones a bullet may open on (the console draws them as
icons). The answer is checked before it is printed: a
"### " heading first, no "## " heading (the console reads the notes under the
version's own), a sensible length. Anything else (no key, a network error, a
refusal, an answer of the wrong shape) prints nothing and exits 1 with the
reason on stderr: the caller keeps the notes as they are.

A release may have nothing to highlight: notes with no section a user
notices (only build, tests, docs) are not sent, and the model may answer
NONE when what is there is too small for a summary (a typo, a minor fix).
Either prints nothing and exits 3: no summary, and nothing went wrong.

OPENROUTER_MODEL picks the model (default below). Standard library only.
"""
import glob
import json
import os
import re
import sys
from concurrent.futures import ThreadPoolExecutor
import urllib.error
import urllib.request

from release_notes import CONTRIBUTOR_SECTIONS, items, sections

URL = "https://openrouter.ai/api/v1/chat/completions"
DEFAULT_MODEL = "anthropic/claude-sonnet-5.5"
MAX_NOTES = 120_000   # characters of notes sent; 1.0.0's are about 28 000
MAX_PR = 4_000        # characters of one pull request's description sent
MAX_PRS = 150_000     # characters of descriptions sent; 1.2.0's are about 128 000
MAX_SUMMARY = 3_000   # characters of summary accepted
NOTHING = 3           # the exit code when there is nothing to highlight

PROMPT = """You write the summary at the top of the release notes of PlayGuard {version}.

PlayGuard is a Nintendo Switch homebrew app (Atmosphère custom firmware) that brings the settings of Nintendo's Parental Controls phone app onto the console, offline: play-time limits, restrictions, PIN, network clock, play activity.

Readers are parents and homebrew users, on the GitHub release page and on the console's update screen. The full list of changes follows your summary, so do not repeat it: say what a user notices.

A release may have nothing worth a summary: a typo, a minor fix, changes a user does not notice. Then answer exactly NONE, nothing else: the list alone is better than a summary that pads. Never invent or inflate a change to fill the form.

Otherwise, the form, exactly:
- First line: "### 🌟 Highlights", then a blank line.
- One or two plain sentences on what this release is about.
- Then 1 to 8 bullets, one per change a user notices (one bullet for a release with one such change), "* <emoji> **Topic**: what changed for the user." Open each bullet on one emoji{emoji_list}. Group related changes; leave out tests, CI, refactoring and documentation unless a user would notice.
- If an item breaks something or asks the user to act (a moved folder, a setting reset), end with one plain line saying so. Do not say that a full list follows.
- English, no other heading, no links, no commit or pull-request numbers, no closing remark. Under 2000 characters.

{examples}The notes of {version}:

{notes}{prs}"""

PRS = """

The descriptions of the pull requests behind these notes (#number as in the notes), for context: what a change is for and what a user sees. They are written for reviewers: leave out how a change was built or tested, and trust the notes when they disagree.

{descriptions}"""


def examples(folder: str, version: str) -> str:
    out = []
    for path in sorted(glob.glob(os.path.join(folder, "*.md"))):
        name = os.path.basename(path)[:-3]
        if name == version or not name[:1].isdigit():
            continue
        with open(path, encoding="utf-8") as f:
            out.append(f"The summary of {name}, for the tone (not its content):\n\n{f.read().strip()}\n\n")
    return "".join(out[-2:])


def emoji_list(folder: str) -> str:
    """The emoji of the images in `folder` ("1f680.png" → 🚀), for the prompt."""
    chars = []
    for path in sorted(glob.glob(os.path.join(folder, "*.png"))):
        try:
            chars.append(chr(int(os.path.basename(path)[:-4], 16)))
        except ValueError:
            continue
    return f", one of these (the console has an icon for them only): {' '.join(chars)}" if chars else ""


def noticeable(notes: str) -> bool:
    """Whether the notes have a change a user notices: a bullet outside the
    sections for contributors (build, tests, docs)."""
    return any(title and title not in CONTRIBUTOR_SECTIONS and items(block)
               for title, block in sections(notes))


def pr_numbers(notes: str) -> list:
    """The pull requests of the sections a user notices, in the order of the
    notes: release-please links each as "([#123](…))"."""
    out = []
    for title, block in sections(notes):
        if title and title not in CONTRIBUTOR_SECTIONS:
            out += [int(n) for n in re.findall(r"\[#(\d+)\]", block) if int(n) not in out]
    return out


def description(body: str) -> str:
    """The part of a description that says what a change is for: its "What
    this changes" section (.github/pull_request_template.md) when it has one,
    else the whole, without the template's comments, the checklist and the
    footer."""
    body = re.sub(r"<!--.*?-->", "", body or "", flags=re.S).replace("\r\n", "\n")
    parts = re.split(r"^## +", body, flags=re.M)
    what = [p for p in parts[1:] if p.lower().startswith("what this changes")]
    text = what[0].partition("\n")[2] if what else re.split(r"^(?:## +Checklist|---\s*$)", body, flags=re.M | re.I)[0]
    text = text.strip()
    return text if len(text) <= MAX_PR else text[:MAX_PR].rstrip() + " […]"


def pull_request(number: int, repo: str, token: str):
    api = os.environ.get("GITHUB_API_URL", "https://api.github.com")
    req = urllib.request.Request(f"{api}/repos/{repo}/pulls/{number}", headers={
        "Authorization": f"Bearer {token}",
        "Accept": "application/vnd.github+json",
        "X-GitHub-Api-Version": "2022-11-28",
    })
    try:
        with urllib.request.urlopen(req, timeout=30) as r:
            pr = json.load(r)
    except (OSError, ValueError) as e:
        print(f"#{number}: {e}", file=sys.stderr)
        return None
    text = description(pr.get("body") or "")
    return f"#{number} {pr.get('title', '')}\n\n{text}" if text else None


def pull_requests(notes: str) -> str:
    """The descriptions of the notes' pull requests, for the prompt; "" without
    GH_TOKEN and GH_REPO."""
    repo, token = os.environ.get("GH_REPO", ""), os.environ.get("GH_TOKEN", "")
    numbers = pr_numbers(notes)
    if not (repo and token and numbers):
        return ""
    with ThreadPoolExecutor(8) as pool:
        found = [d for d in pool.map(lambda n: pull_request(n, repo, token), numbers) if d]
    out, size = [], 0
    for d in found:
        if size + len(d) > MAX_PRS:
            break
        out.append(d)
        size += len(d)
    print(f"{len(out)} of {len(numbers)} pull request descriptions given", file=sys.stderr)
    return PRS.format(descriptions="\n\n---\n\n".join(out)) if out else ""


def nothing(answer: str) -> bool:
    return answer.strip().strip("`*.\"' ").upper() == "NONE"


def check(summary: str) -> str:
    summary = summary.strip()
    if summary.startswith("```"):   # a fenced answer
        summary = summary.strip("`").partition("\n")[2].strip()
    if not summary.startswith("### "):
        raise ValueError("the answer does not start with a ### heading")
    if any(line.startswith(("## ", "# ")) for line in summary.splitlines()):
        raise ValueError("the answer has a # or ## heading")
    if not 100 <= len(summary) <= MAX_SUMMARY:
        raise ValueError(f"the answer is {len(summary)} characters long")
    return summary


def main() -> int:
    if len(sys.argv) not in (2, 3, 4):
        print(__doc__.strip().splitlines()[3].strip(), file=sys.stderr)
        return 2
    version = sys.argv[1].lstrip("v")
    notes = sys.stdin.read()[:MAX_NOTES]
    if not noticeable(notes):
        print("nothing a user notices in the notes: no summary", file=sys.stderr)
        return NOTHING
    key = os.environ.get("OPENROUTER_API_KEY", "")
    if not key:
        print("no OPENROUTER_API_KEY", file=sys.stderr)
        return 1
    prompt = PROMPT.format(version=version, notes=notes, prs=pull_requests(notes),
                           examples=examples(sys.argv[2], version) if len(sys.argv) >= 3 else "",
                           emoji_list=emoji_list(sys.argv[3]) if len(sys.argv) == 4 else "")
    body = {
        "model": os.environ.get("OPENROUTER_MODEL") or DEFAULT_MODEL,
        "messages": [{"role": "user", "content": prompt}],
        "max_tokens": 1500,
        "temperature": 0.3,
    }
    req = urllib.request.Request(URL, data=json.dumps(body).encode(), headers={
        "Authorization": f"Bearer {key}",
        "Content-Type": "application/json",
        "HTTP-Referer": "https://github.com/JigSawFr/PlayGuard",
        "X-Title": "PlayGuard release notes",
    })
    try:
        with urllib.request.urlopen(req, timeout=120) as r:
            answer = json.load(r)
        content = answer["choices"][0]["message"]["content"] or ""
        if nothing(content):
            print(f"{answer.get('model', body['model'])} found nothing to highlight: no summary", file=sys.stderr)
            return NOTHING
        summary = check(content)
    except urllib.error.HTTPError as e:
        print(f"OpenRouter: HTTP {e.code} {e.read()[:300]!r}", file=sys.stderr)
        return 1
    except (OSError, ValueError, KeyError, IndexError, TypeError) as e:
        print(f"OpenRouter: {e}", file=sys.stderr)
        return 1
    print(f"summary by {answer.get('model', body['model'])}", file=sys.stderr)
    # Said plainly: no one read it before it was published.
    sys.stdout.write(summary + "\n\nSummary written automatically.\n")
    return 0


if __name__ == "__main__":
    sys.exit(main())
