#!/usr/bin/env python3
"""A hand-written summary at the top of a GitHub release's notes.

  tools/release_notes.py SUMMARY < notes.md > new-notes.md

release-please writes the release's notes from the conventional commits: one
line per commit, a long list for a release with many. SUMMARY
(.github/release-notes/<version>.md, written in a pull request before the
release) goes right under the notes' "## <version>" heading, so it is the
first thing read on the release page and on the console's "PlayGuard X.Y.Z
is available" screen (which reads the release's notes under that heading, and
keeps their first 16 KB only). With no "## " heading the summary goes first.
Notes that already hold the summary come out unchanged: a re-run of the
release job edits nothing. Standard library only.
"""
import sys


def insert(notes: str, summary: str) -> str:
    summary = summary.strip()
    if not summary or summary in notes:
        return notes
    lines = notes.splitlines(keepends=True)
    for i, line in enumerate(lines):
        if line.startswith("## "):
            head = "".join(lines[: i + 1])
            rest = "".join(lines[i + 1 :]).lstrip("\n")
            if not head.endswith("\n"):
                head += "\n"
            return head + "\n" + summary + "\n\n" + rest
    return summary + "\n\n" + notes.lstrip("\n")


def main() -> int:
    if len(sys.argv) != 2:
        print(__doc__.strip().splitlines()[2].strip(), file=sys.stderr)
        return 2
    with open(sys.argv[1], encoding="utf-8") as f:
        summary = f.read()
    sys.stdout.write(insert(sys.stdin.read(), summary))
    return 0


if __name__ == "__main__":
    sys.exit(main())
