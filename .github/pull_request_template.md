<!--
The title must be a conventional commit with a scope (feat(history): …, fix(play-timer): …,
docs(install): …): CI checks it, and the squash merge makes it the commit subject that
release-please writes the changelog from.
-->

## What this changes, and why

<!-- The diff says what; this says what it is for. Link the issue it closes, if any. -->

## How it was verified

<!-- Beyond CI. Say whether it ran on a console (firmware, Atmosphère, launcher) or only in `make desktop`. -->

Screens: none

<!-- The screens this change alters on purpose, compared with main's by CI's visual check (tests/visual/README.md):
`Screens: smoke/01_dashboard, cadran/*` (scenario/name patterns), `any` for every screen, `none` for no change meant.
A screen that changes without being named here fails the smoke job. -->

## Checklist

- [ ] `make test` and `python3 tools/check_resources.py .` pass
- [ ] New strings are in every `resources/i18n/*/playguard.json` (English is fine as a placeholder)
- [ ] A screen that changes on purpose is named on the `Screens:` line above
- [ ] A change users can see is in the README or the user guide (`docs/`), in English and French (`docs/fr/`)
- [ ] Nothing writes the play timer while it counts down
