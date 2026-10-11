# Contributing to PlayGuard

Thanks for helping! This file covers building, testing, the code layout, releases and translations. For what the app does, see the [README](README.md); for what to work on, the [roadmap](ROADMAP.md). Everyone taking part follows the [code of conduct](CODE_OF_CONDUCT.md); for help rather than a change, see [SUPPORT.md](.github/SUPPORT.md).

## Build from source

| Command | What it does | Needs |
|---|---|---|
| `make test` | Host unit tests of the C service layer and of the UI-free C++ helpers and flow logic, with ASan + UBSan (`SAN=` to turn them off); the suites run in parallel, all of them even when one fails | gcc and g++ with the sanitizers, zlib headers, the borealis submodule — no devkitPro |
| `make desktop` | The real UI on Linux, against a simulated console | GLFW / X11 / D-Bus dev packages |
| `make` | `./playguard.nro` — drawn with deko3d (`GL=1` for OpenGL) | devkitPro `switch-dev`, `DEVKITPRO` set |
| `make dist` | `./playguard.zip` (SD-card layout) | as above |
| `make dist-rescue` | `./playguard-rescue.zip`, the optional recovery sysmodule | as above |
| `./run.sh [ip]` | Builds in the `devkitpro/devkita64` Docker image, at the digest CI uses (read from `build.yml`), optionally `nxlink`s to a console | Docker |

Clone with submodules (`git clone --recursive`, or `git submodule update --init`): borealis is pinned at `extern/borealis/`.

## The desktop simulator

`make desktop` runs the real borealis UI against `source/sim/`, a simulated console. Environment knobs:

| Variable | Simulates |
|---|---|
| `PLAYGUARD_SIM_FW=20.5.0` | Another firmware (default 22.5.0, the newest verified on a console; above it, the firmware screen) |
| `PLAYGUARD_SIM_NO_CFW=1` | No Atmosphère |
| `PLAYGUARD_SIM_NOT_SET_UP=1` | Parental controls never set up (no PIN) |
| `PLAYGUARD_SIM_TIMER_OFF=1` | No play-time limit |
| `PLAYGUARD_SIM_RESTRICTED=1` | Today's limit reached |
| `PLAYGUARD_SIM_NOT_COUNTING=1` | The time left not going down (a clock set back): the Overview says the console is not counting after 90 s on screen |
| `PLAYGUARD_SIM_UNLOCKED=1` | Parental controls temporarily unlocked |
| `PLAYGUARD_SIM_UNPAIRED=1` | No companion app linked |
| `PLAYGUARD_SIM_ALARM_OFF=1` | The "time's up" alarm off |
| `PLAYGUARD_SIM_ACCURATE=1` | An accurate network clock |
| `PLAYGUARD_SIM_AUTOSYNC_OFF=1` | *Synchronise Clock via Internet* off |
| `PLAYGUARD_SIM_NETWORK_OFFSET=1411` | A network clock that many seconds ahead of the console clock until it is set (the console clock follows it while *Synchronise* is on) |
| `PLAYGUARD_SIM_NTP=offline` | No time server answers (a console without Internet) |
| `PLAYGUARD_SIM_FAIL=timer,clock,unverified,bedtime` | Failures: the play-timer write, setting the clock, an unlock the system does not confirm, a bedtime the console does not take |
| `PLAYGUARD_SIM_EMUMMC=1` | Running on emuMMC |
| `PLAYGUARD_SIM_BLANK=1` | Atmosphère blanking the serial number |
| `PLAYGUARD_SIM_APPLET=1` | Applet (album) mode |
| `PLAYGUARD_SIM_NO_PDM=1` | No activity log |
| `PLAYGUARD_SIM_GAMES=200` | That many more games played this week (a large library) |
| `PLAYGUARD_SIM_IDLE=1` | No game running |
| `PLAYGUARD_SIM_BACKGROUND=1` | PlayGuard out of focus |
| `PLAYGUARD_SIM_REGION=2` | The console region |
| `PLAYGUARD_SIM_LATEST=1.1.0:24.0.0` | The latest release and the newest firmware it supports, for the update check (`offline` for no network) |
| `PLAYGUARD_SIM_PASTE=https://gist.github.com/u/ab12` | What GitHub answers to *Send a report online* (`offline` for no network) |
| `PLAYGUARD_SIM_DEV_BUILDS=<folder>` | What the GitHub API returns for *Install another build*: `latest.json` (the latest release), `artifacts.json`, `pulls.json`; `https://<local path>` download URLs are copied from disk (`offline` for no network) |
| `PLAYGUARD_SIM_GITHUB_LOGIN=ok` | The GitHub device flow of *GitHub account*: `ok` approves at once, `denied` refuses, `offline` fails |
| `PLAYGUARD_SIM_HBLOADER=1` | A homebrew loader that can hand an update over to a store |
| `PLAYGUARD_SIM_NUMPAD=1:30` | What the system number pad returns |
| `PLAYGUARD_SIM_NOW=<POSIX seconds>` | A frozen console time (with `TZ=` for its time zone) |
| `PLAYGUARD_SIM_STILL_FOCUS=1` | Not the console: the focus highlight without its moving glow, and the loading screen's ring still, for screenshots that compare |
| `PLAYGUARD_SIM_VERSION=1.0.0:1a2b3c4` | Not the console: the version and commit the header's badge shows (`1.0.0:` for a release), not the build's own, for screenshots that compare |
| `PLAYGUARD_SIM_HOLD_LOADING=1` | Not the console: the start-up stops on the loading screen, at its last step (the `loading` smoke scenario's screenshot) |
| `PLAYGUARD_SIM_CADRAN=1` | Not the console: the "Cadran" prototype after the design boards, to look at and measure: the Overview, Play timer (the week edited with the stick), Activity and Restrictions tabs and the child screen (*Montrer à l'enfant*), in both themes (`source/tab/cadran_*.cpp`, `source/activity/cadran_child_activity.cpp`, `source/view/cadran_*.cpp`: an icon rail, a ring for today's time, the figures in Sora from `resources/font/`, OFL); `PLAYGUARD_SIM_CADRAN_NOGLOW=1` and `PLAYGUARD_SIM_CADRAN_NORING=1` leave out its two custom draws |
| `PLAYGUARD_SIM_STATS=<file>` | Not the console: one line per frame (frame time with the FPS limiter off; nanovg's draw calls and triangles with `tools/perf_nanovg_stats.patch`), see `tools/perf_frames.sh` |

Game patches are read from `./playguard_data/sd/`, the simulated SD card root.

## Tests and CI

- `make test` — host unit tests (`tests/`, see [`tests/README.md`](tests/README.md)), including the recovery sysmodule logic (`tests/rescue/`); every suite runs, in parallel, and leaves its output in `build/host-tests/<suite>.log`.
- `tools/desktop_smoke.py <out-dir> [gate|errors|rescue|forged|lock|devbuild|library|outside]` — clicks through every screen of the desktop build headlessly (needs `DISPLAY`, `xdotool`, ImageMagick) and saves screenshots. The `gate` scenario covers the firmware screen and developer mode on a simulated 24.0.0; `errors` covers a failed unlock and an unsettable clock; `rescue` the recovery screen, then quitting while still unlocked; `forged` a rescue report the console does not confirm (no action, the lock screen first); `lock` the lock screen of *Ask for the PIN › To open PlayGuard*; `library` a large library in Activity (its first 50 games, then *Show every game*); `devbuild` signing in to a simulated GitHub and installing a pull request's build in place out of its artifact. `outside` the Overview's notices: changes made outside PlayGuard (a play time that started over, the clock, the limits) and, after 90 s on screen, a console that is not counting play time (it takes about two minutes).
- `tools/visual_check.py` — compares those screenshots with the references in `tests/visual/`. A screen that changed fails CI; see [`tests/visual/README.md`](tests/visual/README.md) to update them.
- `tools/perf_frames.sh <name> [VAR=value …]` — the per-frame cost of one screen of the desktop build, headlessly (same needs as the smoke test, plus `dbus-run-session`): the median and p95 frame time over the steady state, and nanovg's draw calls and triangles once `tools/perf_nanovg_stats.patch` is applied in `extern/borealis` (`git -C extern/borealis apply ../../tools/perf_nanovg_stats.patch`; a local patch, never committed). `PERF_KEYS="Down Right"` presses keys first to reach another screen (xdotool names; Return is A, Escape B), `PERF_THEME=light` runs the light variant. Software GL rasterises on the CPU, so the figures weigh pixels far more than the console's GPU does: compare screens with each other, not with the console.
- `python3 tools/check_resources.py .` — checks the XML layouts and translation catalogs.

CI (`.github/workflows/build.yml`) runs all of the above plus the Switch build, the smoke scenarios in parallel shards, on every pull request, every push to main and once a week. The visual check compares the sets in `tests/visual/` and fails on a changed screen (the simulator keeps the console time and the focus highlight still, so the screens come out the same in every run). The desktop and Switch builds turn PlayGuard's compiler warnings into errors (`make desktop CMAKE_ARGS=-DPLAYGUARD_WERROR=ON`, `make dist CMAKE_ARGS=-DPLAYGUARD_WERROR=ON`), and a compiler message is annotated on its line of the pull request; the Switch job also fails on a libnx older than 4.12.0 or an `.nro` without libnx's `LNY2` marker (`tools/check_nro.py`), and checks that a second build gives byte-identical zips; `check_resources.py` warns about sentences a catalog still has in English.

Each job writes a summary on the run page (`tools/ci_summary.py`): what is being built, the suites, the coverage per file, each smoke scenario with its visual check and its time, and for the Switch build the `.nro` size against main's last build, what makes it big (from the linker map: `python3 tools/ci_summary.py map build/borealis.map --nro playguard.nro` shows the same table for a local build), the toolchain and the downloads. A run's artifacts are named after its build, `playguard-<version>-<ref>-<commit>-<component>` (`1.0.0-pr92-1f0a059`, `1.0.0-main-2bd4996`): `-nro` (the app, what the developer tools download), `-rescue` (the recovery sysmodule, as the zip and the raw `.nsp`), `-linker-map`, `-screenshots-<shard>` (with a contact sheet per scenario), and on a release build `-sdcard` (the store zip). The runner image is pinned (`ubuntu-26.04`): the references of the visual check were drawn by its mesa, so moving to the next image is a pull request of its own. The devkitPro image is pinned by digest in `build.yml`; `toolchain-watch.yml` opens an issue once a month when Docker Hub's `latest` moved.

## Code layout

| Path | Content |
|---|---|
| `source/core/` | C, libnx: `pctl_ops`, `time_ops`, `sysinfo`, `playstats`, `platform`, `rescue`, `pure`, `calendar`, `write_guard` (these last four need no console and are shared with the simulator), `startup_hook` (times borealis' `userAppInit`, which libnx calls before `main`) |
| `source/tab/` | One class per tab |
| `source/action/` | Flows, one per thing a tab does, with their UI-free logic in `*_logic.cpp` where there is one: the play-timer write and its recorder, the clock (and a console no time server answers), the PIN prompt and the console lock, settings backups and restores, the change history and undo, changes seen outside PlayGuard and timer health, the firmware screen, the recovery screen, reports and sending them online, updates, GitHub sign-in and development builds, the SD-card write queue, clearing the cache and the log files |
| `source/activity/` | Screens: per-day editor, profiles, a game, first steps, change history, firmware, recovery, the lock screen, the diagnostic report, what's new and the full changelog, the start-up error, the development builds, the loading screen (the start-up runs one step per frame under it; the intro keeps it 2 s) |
| `source/view/` | Widgets: the week chart, the gauge, the play-timer state line, the day bars, the game cell, the QR codes and funding links, release notes, the scrolling frame, the loading screen's icon and progress bar |
| `source/ui/` | Dialogs, formatting, theme colours, the console's state as text, read-only greying and visibility |
| `source/util/` | NTP, HTTP, config, profiles, settings backups, change history, play-log folding, the play-data cache, the Activity summary, table export, diagnostics, sending reports online, the play-timer recorder and block reference, update check, store launcher, GitHub sign-in and development builds (zip, SHA-256), game patches, the bundled changelog, the start-up reminders and times, the background threads (joined, never detached: the console cannot) and which files the cache and the logs are |
| `source/sim/` | The simulated console for the desktop build |
| `resources/` | XML layouts and `i18n/<language>/playguard.json` |
| `sysmodule/` | The optional recovery boot sysmodule; shares `source/core/rescue.c` with the app. See [`sysmodule/README.md`](sysmodule/README.md) |
| `packaging/` | Store and sphaira entries. See [`packaging/README.md`](packaging/README.md) |
| `branding/` | SVG sources of the icon and banners |
| `docs/` | [`parental-controls.md`](docs/parental-controls.md): what is known of the parental-control service, the play-timer block and the clocks, and how sure each fact is; [`companion-app.md`](docs/companion-app.md): what PlayGuard covers of Nintendo's phone app, and the gaps still to close |

**Branding:** `branding/*.svg` are rendered to `icon.jpg`, `images/store/*.png`, `images/social-preview.png` (the repository's social preview) and `resources/img/wordmark.png` (the loading screen's) by `node tools/render_branding.mjs` (Node + Playwright).

## Pull requests and releases

Releases are automated with [release-please](https://github.com/googleapis/release-please), which reads [conventional commits](https://www.conventionalcommits.org/):

- Give each pull request a conventional title (`feat: …`, `fix: …`, `docs: …`, `feat!: …` for a breaking change); CI checks it.
- **Squash-merge** pull requests, so each one lands as a single commit carrying that title. A plain merge commit makes release-please apply a PR's `BEGIN_COMMIT_OVERRIDE` block to every commit of the PR.
- release-please keeps a `chore(main): release X.Y.Z` PR open; merging it tags the release and attaches the `.nro` / `.zip`.
- release-please runs with a token of the *PlayGuard Release* GitHub App (variable `RELEASE_APP_ID`, secret `RELEASE_APP_PRIVATE_KEY`; permissions Contents, Pull requests and Issues: write), not `GITHUB_TOKEN`: the app is on the tag ruleset's bypass list, so it can create the `vX.Y.Z` tag, and its pushes to the release PR run the required checks.
- **The release page** is laid out by the release job: a summary (`.github/release-notes/X.Y.Z.md`, merged before the release PR; else, with an `OPENROUTER_API_KEY` secret, one written by a language model), the figures, the commit list folded (users, then contributors), the screens that changed, the install section, the build report, and a discussion. [`.github/release-notes/README.md`](.github/release-notes/README.md) says how; *release-page* in Actions does it again for an existing release.

**Development builds:** nothing is published for them. The developer tools install the `playguard-<version>-<ref>-<commit>-nro` artifact of a build run (a zip holding `playguard.nro`; the artifacts API only filters by an exact name, so the app lists a few pages of a run's artifacts and keeps that one): main's last commits and the newest run of each open pull request from a branch of this repository; pull requests from forks are left out, as their code is not reviewed yet (`source/util/dev_builds.hpp`, [SECURITY.md](SECURITY.md)). GitHub hands artifacts to signed-in users only, hence the GitHub sign-in (device flow, GitHub App client ID in `source/util/github_auth.hpp`; with the Device Flow enabled and token expiry off; the app's only permissions are Actions: read and Gists: write). For a pull request, `PLAYGUARD_COMMIT` makes the app report its head commit, as the artifacts API names the run. Every build but a release one (`PLAYGUARD_RELEASE=1`, set for the `vX.Y.Z` tag's build) shows itself as a development build in the header, at the right: an amber **DEV** tag, the version and the commit, where a release shows a plain grey `v1.2.0` (`source/view/version_badge.hpp`).

Each release also publishes `compat.json` (`tools/gen_compat.py`: the version and the newest checked firmware), which the app's update check reads, plus `build-info.txt` and `SHA256SUMS.txt`. Details in [`packaging/README.md`](packaging/README.md). The `.nro`, both `.zip` and `compat.json` also carry a [build provenance attestation](https://docs.github.com/actions/security-for-github-actions/using-artifact-attestations): `gh attestation verify playguard.nro -R JigSawFr/PlayGuard` checks that a download was built by this repository's CI.

**Reproducible zips:** `make dist` and `make dist-rescue` give every file in the zip the date `SOURCE_DATE_EPOCH` (POSIX seconds; default: the last commit's time, `git log -1 --format=%ct`) and add the files in a fixed order without extra attributes, so the same commit and `.nro` give a byte-identical zip. The deko3d shaders are not compiled at build time (uam does not always give the same bytes): `shaders/` keeps them, with the checksums of the borealis GLSL they come from, and configure stops when that GLSL changes; then rebuild them with `-DPLAYGUARD_UAM=ON`, copy `build/dksh/*.dksh` to `shaders/` and update `shaders/glsl.sha256`. Set `SOURCE_DATE_EPOCH=<seconds>` to rebuild one from another checkout. CI builds the zips twice and compares them; when they differ, `tools/repro_diff.py` says where (a zip entry, a segment of the `.nro`, a RomFS file, a shader) and the run keeps both builds as an artifact.

When a change is visible to users, update **both** [README.md](README.md) and [README.fr.md](README.fr.md).

## Translating PlayGuard

PlayGuard is translated into every language the console offers: English, French (France and Canada), German, Spanish (Spain and Latin America), Italian, Dutch, Portuguese (Portugal and Brazil), Russian, Japanese, Korean and Chinese (simplified and traditional). It follows the console's language unless *Preferences › Language* picks another one. British English consoles get the English strings, already in British spelling (`CMakeLists.txt` copies them under `en-GB`). Fixes from native speakers are welcome.

To add a language:

1. Copy `resources/i18n/en-US/playguard.json` to `resources/i18n/<code>/playguard.json` (`<code>` as the console names its language, see `brls::LOCALE_*`) and translate the values. Keep every key, and as many `{}` placeholders as the English text, in the same order.
2. Add `<code>` to `config::LANGUAGES` in `source/util/config.hpp` (the language picker and the saved preference use that list).
3. Add the language's own name under `"tools" › "languages"` in **every** `playguard.json` (`"ru": "Русский"`). A regional variant (`fr-CA`, `es-419`, `pt-BR`) is a full catalog of its own: borealis falls back to English, not to the base language.
4. Optionally add `resources/i18n/<code>/hints.json` for borealis' button hints when borealis has none for that language (see `resources/i18n/fr/hints.json`); missing strings fall back to English.
5. Run `python3 tools/check_resources.py .`: it reports missing or extra keys, placeholder mismatches and a language missing from the picker. CI runs the same check.
