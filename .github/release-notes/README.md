# Release summaries

`<version>.md` (`1.1.0.md`, no `v`) is the summary put at the top of that
release's notes on GitHub, under the `## <version>` heading release-please
writes: the build's release job (`.github/workflows/build.yml`) inserts it
with `tools/release_notes.py` once the release exists. It is read on the
release page and on the console's update screen; the commit list follows it.
The build also bundles it: About and the "What's new" screen after an update
show this summary alone, with a *Full list of changes* cell to the release's
`CHANGELOG.md` entry (no summary: that entry, as before).

Write it in a pull request while the `chore(main): release X.Y.Z` pull request
is open (its title gives the version), and merge it before that one. No file:
the notes stay as release-please wrote them. Once the release exists, a
summary merged later is not picked up (release-please does not create the
release twice): edit the release on GitHub, or run the tool on its notes:

    gh release view vX.Y.Z --json body --jq .body \
      | python3 tools/release_notes.py .github/release-notes/X.Y.Z.md > notes.md
    gh release edit vX.Y.Z --notes-file notes.md

Start with a `### ` heading (a section on the console), then a line or two and
a few bullets: what a user notices, not one line per pull request. Emoji are
dropped from the console's text, but the one a `### ` heading or a bullet opens
on is shown as an icon there (an image from `resources/img/emoji/`, the list
is in its README; another one shows no icon). A bullet reads best as a card:

    * 📊 **Activity**: play time per game and per user account, …

Its last line is read on the console just above the *Full list of changes*
cell, not above the list itself: no "the full list follows".
