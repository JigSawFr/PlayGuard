<!-- Title: a conventional commit with a scope (feat(history): …, fix(play-timer): …). CI checks it; it becomes the squash subject and the changelog entry. -->

## 🎯 At a glance

<!-- 2 to 3 sentences for someone who has not seen the diff: what changes and what it gives. -->

## 💡 Why

<!-- The need behind it. Link the issue it closes (Closes #N). -->

## 🔧 What changes

<!-- A short list of the real changes, not a copy of the diff. -->
-

### 🖼️ Before / After

<!-- Only when a screen changes. Fictional games and names only. Delete the table otherwise. -->

| Before | After |
|---|---|
| | |

Screens: none

<!-- Screens changed on purpose, read by CI's visual check (tests/visual/README.md): `smoke/01_dashboard, cadran/*`, `any`, or `none`. An unnamed change fails the smoke job. Keep this line alone on its own line. -->

## ✅ Checked

<!-- What was run beyond CI, and whether it ran on a console or only in `make desktop`. Say what could not be checked. -->

- [ ] New strings are in every `resources/i18n/*/playguard.json` (English is fine as a placeholder)
- [ ] A change users can see is in the README or `docs/`, in English and French (`docs/fr/`)
- [ ] Nothing writes the play timer while it counts down

Merge: squash
