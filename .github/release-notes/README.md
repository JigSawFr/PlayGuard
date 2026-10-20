# Release summaries

Once a release exists, the build's release job lays out its page
(`tools/release_page.sh`, `tools/release_notes.py`): the title
`PlayGuard X.Y.Z`, then under the heading `## X.Y.Z · <date>`

1. a **download line** (`playguard.zip`, the app alone, how to install) and
   the **figures**: breaking changes, features, fixes, polish, the commits;
2. a **summary**: `<version>.md` from this folder (`1.1.0.md`, no `v`), else
   one written by a language model (below), else none. On the page, each
   bullet's bold lead-in sits on a line of its own, its text under it, and a
   paragraph after the last bullet becomes an *Important* box;
3. **what changed**: what a user notices (features, changes, fixes, polish),
   then **for contributors** (build, tests, docs): release-please's lists,
   folded;
4. **🖼️ On screen**: the screens whose visual-check reference
   (`tests/visual/`) changed since the previous release, before and after;
5. **📦 Install**: three steps (new install, update, locked out) with the
   assets and their size, the firmware checked (from `compat.json`);
6. **🔍 Build**: the build's report (the run page: the `.nro` size, the tests
   and coverage, the smoke screenshots, the toolchain), the checksums and
   the attestation, the contributors.

It also opens the release's discussion, in the category the
`RELEASE_DISCUSSION_CATEGORY` variable names (default `Announcements`;
`none`: no discussion), and keeps the discussion's post the same as the page:
GitHub copies the notes once, when it opens it, and does not follow later
edits (a run of *release-page* updates both). Only the discussion's author
can edit its post: the *PlayGuard Release* app, which published the release,
so the app needs the **Discussions: read and write** permission (GitHub App
settings › Permissions, then accept it on the installation).

The console's update screen shows the summary and the lists: it skips HTML
lines and comments and stops at the screens, and reads the release page's
own comments (`source/util/changelog.cpp`): `<!-- console: a line -->` is
shown there only, the lines of a `<!-- console:only` … `-->` comment too, and
those between `<!-- console:hide -->` and `<!-- /console -->` are not (the
*Important* box: the console shows its words as a plain line). It keeps the
first 16 KB only: the summary is what it shows for sure. Version 1.0.0 knows
none of this: its update screen shows these lines as they are. The build also bundles the file: the "What's
new" screen after an update shows this summary alone, with a *Full list of
changes* cell to the release's `CHANGELOG.md` entry (no summary: that entry,
as before). *About › Release notes* shows every version's page (the summary
as cards, then the lists), the installed one's from the bundled files when
GitHub's is not there.

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
release-please wrote and the descriptions of their pull requests (their
*What this changes, and why* section, for the changes a user notices: a
commit title says what changed, the description what it is for), with the
files here as examples of the form and the emoji of `resources/img/emoji/`
as the icons it may use
(`tools/summarize_release.py`). The `OPENROUTER_MODEL` variable picks the
model (default `anthropic/claude-sonnet-5.5`). It is published without being
read first, so it ends with "Summary written automatically."; the run page
shows it. It goes on the release page only: the app bundles a file from here,
not this. No key, an error or an answer of the wrong shape: no summary, a
warning on the run, and the release goes on. A hand-written file always wins.

A release need not have a summary. Notes with no change a user notices
(only build, tests, docs) are not sent to the model, and the model answers
`NONE` when the rest is too small to summarise (a typo, a minor fix); it
writes one bullet per change a user notices, from one to eight, never
padding. Either way: no summary and no warning. The page then goes from the
figures to the lists, the console shows the lists, and "What's new"
shows the `CHANGELOG.md` entry, as with no file.

## An existing release

Actions › **release-page** › Run workflow, with its tag (empty: the latest):
the page as the release job would lay it out, on the run page only; tick
*apply* to update the release. A way to try the key, the model or the prompt,
to lay out a release published before this existed, or to add a summary
merged after its release. A page laid out before is laid out again from its
folded lists, with the file's summary if there is one, else the one already
on the page (no second call to OpenRouter); the same page is not edited.
