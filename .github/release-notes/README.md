# Release summaries

Once a release exists, the build's release job lays out its page
(`tools/release_page.sh`, `tools/release_notes.py`): the title
`PlayGuard X.Y.Z`, then under the `## X.Y.Z` heading release-please writes

1. a **summary**: `<version>.md` from this folder (`1.1.0.md`, no `v`), else
   one written by a language model (below), else none;
2. the **figures**: features, fixes, polish, and the full diff;
3. **every change** a user notices (features, changes, fixes, polish), then
   **for contributors** (build, tests, docs): release-please's lists, folded;
4. **🖼️ On screen**: the screens whose visual-check reference
   (`tests/visual/`) changed since the previous release, before and after;
5. **📦 Install**: the assets, their size and what each is for, the firmware
   checked (from `compat.json`);
6. **🔍 Build**: the build's report (the run page: the `.nro` size, the tests
   and coverage, the smoke screenshots, the toolchain, the checksums), how to
   check a download, the contributors.

It also opens the release's discussion, in the category the
`RELEASE_DISCUSSION_CATEGORY` variable names (default `Announcements`;
`none`: no discussion).

The console's update screen shows the summary and the lists (it skips the
HTML lines and stops at the screens) and keeps the first 16 KB only: the
summary is what it shows for sure. The build also bundles the file: About and
the "What's new" screen after an update show this summary alone, with a *Full
list of changes* cell to the release's `CHANGELOG.md` entry (no summary: that
entry, as before).

Write the summary in a pull request while the `chore(main): release X.Y.Z`
pull request is open (its title gives the version), and merge it before that
one. Start with a `### ` heading (a section on the console), then a line or
two and a few bullets: what a user notices, not one line per pull request.
Emoji are dropped from the console's text, but the one a `### ` heading or a
bullet opens on is shown as an icon there (an image from
`resources/img/emoji/`, the list is in its README; another one shows no icon).
A bullet reads best as a card:

    * 📊 **Activity**: play time per game and per user account, …

Its last line is read on the console just above the *Full list of changes*
cell, not above the list itself: no "the full list follows".

## Written by a language model

With no file for the version, and the `OPENROUTER_API_KEY` secret set
(Settings › Secrets and variables › Actions), the release job asks a model
through [OpenRouter](https://openrouter.ai) for one, from the notes
release-please wrote, with the files here as examples of the form and the
emoji of `resources/img/emoji/` as the icons it may use
(`tools/summarize_release.py`). The `OPENROUTER_MODEL` variable picks the
model (default `anthropic/claude-sonnet-5.5`). It is published without being
read first, so it ends with "Summary written automatically."; the run page
shows it. It goes on the release page only: the app bundles a file from here,
not this. No key, an error or an answer of the wrong shape: no summary, a
warning on the run, and the release goes on. A hand-written file always wins.

## An existing release

Actions › **release-page** › Run workflow, with its tag (empty: the latest):
the page as the release job would lay it out, on the run page only; tick
*apply* to update the release. A way to try the key, the model or the prompt,
to lay out a release published before this existed, or to add a summary
merged after its release. A page laid out before is laid out again from its
folded lists, with the file's summary if there is one, else the one already
on the page (no second call to OpenRouter); the same page is not edited.
