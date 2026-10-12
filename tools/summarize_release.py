#!/usr/bin/env python3
"""A summary of a release's notes, written by a language model through
OpenRouter, for when no one wrote .github/release-notes/<version>.md.

  OPENROUTER_API_KEY=… tools/summarize_release.py VERSION [EXAMPLES_DIR [EMOJI_DIR]] < notes.md > summary.md

notes.md is the list release-please wrote (one line per commit). The other
summaries in EXAMPLES_DIR (.github/release-notes/*.md) are given as examples
of the tone and the form; the emoji EMOJI_DIR (resources/img/emoji/) has an
image of are the ones a bullet may open on (the console draws them as
icons). The answer is checked before it is printed: a
"### " heading first, no "## " heading (the console reads the notes under the
version's own), a sensible length. Anything else (no key, a network error, a
refusal, an answer of the wrong shape) prints nothing and exits 1 with the
reason on stderr: the caller keeps the notes as they are.

OPENROUTER_MODEL picks the model (default below). Standard library only.
"""
import glob
import json
import os
import sys
import urllib.error
import urllib.request

URL = "https://openrouter.ai/api/v1/chat/completions"
DEFAULT_MODEL = "anthropic/claude-sonnet-5.5"
MAX_NOTES = 120_000   # characters of notes sent; 1.0.0's are about 28 000
MAX_SUMMARY = 3_000   # characters of summary accepted

PROMPT = """You write the summary at the top of the release notes of PlayGuard {version}.

PlayGuard is a Nintendo Switch homebrew app (Atmosphère custom firmware) that brings the settings of Nintendo's Parental Controls phone app onto the console, offline: play-time limits, restrictions, PIN, network clock, play activity.

Readers are parents and homebrew users, on the GitHub release page and on the console's update screen. The full list of changes follows your summary, so do not repeat it: say what a user notices.

Form, exactly:
- First line: "### 🌟 Highlights", then a blank line.
- One or two plain sentences on what this release is about.
- Then 3 to 8 bullets, "* <emoji> **Topic**: what changed for the user." Open each bullet on one emoji{emoji_list}. Group related changes; leave out tests, CI, refactoring and documentation unless a user would notice.
- If an item breaks something or asks the user to act (a moved folder, a setting reset), end with one plain line saying so. Do not say that a full list follows.
- English, no other heading, no links, no commit or pull-request numbers, no closing remark. Under 2000 characters.

{examples}The notes of {version}:

{notes}"""


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
    key = os.environ.get("OPENROUTER_API_KEY", "")
    if not key:
        print("no OPENROUTER_API_KEY", file=sys.stderr)
        return 1
    notes = sys.stdin.read()[:MAX_NOTES]
    prompt = PROMPT.format(version=version, notes=notes,
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
        summary = check(answer["choices"][0]["message"]["content"] or "")
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
