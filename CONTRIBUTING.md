# Contributing to PlayGuard

Thanks for helping! This file covers building, testing, the code layout, releases and translations. For what the app does, see the [README](README.md); for what to work on, the [roadmap](ROADMAP.md). Everyone taking part follows the [code of conduct](CODE_OF_CONDUCT.md); for help rather than a change, see [SUPPORT.md](.github/SUPPORT.md).

## No code needed

Two minutes, if your console is still linked to the phone app: *Security & app › Before unlinking: help decode…* saves the play-timer block, you change one setting in the phone app ("alarm only", or a bedtime for one day), and PlayGuard lists what changed. Attach that file to an issue: it is the data the [roadmap](ROADMAP.md#waiting-on-data-from-a-console) waits for most. Translation fixes from native speakers are just as welcome (see [Translating PlayGuard](#translating-playguard)).

## Build from source

| Command | What it does | Needs |
|---|---|---|
| `make test` | Host unit tests of the C service layer and of the UI-free C++ helpers and flow logic, with ASan + UBSan (`SAN=` to turn them off); the suites run in parallel, all of them even when one fails | gcc and g++ with the sanitizers, zlib headers, the borealis submodule — no devkitPro |
| `make desktop` | The real UI on Linux, against a simulated console | GLFW / X11 / D-Bus dev packages |
| `make` | `./playguard.nro` — drawn with deko3d (`GL=1` for OpenGL) | devkitPro `switch-dev`, `DEVKITPRO` set |
| `make dist` | `./playguard.zip` (SD-card layout) | as above |
| `make dist-rescue` | `./playguard-rescue.zip`, the optional recovery sysmodule (`make` also builds it and bundles it in the romfs, for *Tools › Optional modules*) | as above |
| `make dist-agent` | `./playguard-agent.zip`, the optional remote-link agent (bundled the same way) | as above |
| `./run.sh [ip]` | Builds in the `devkitpro/devkita64` Docker image, at the digest CI uses (read from `build.yml`), optionally `nxlink`s to a console | Docker |

Clone with submodules (`git clone --recursive`, or `git submodule update --init`): borealis is pinned at `extern/borealis/`.

## The desktop simulator

`make desktop` runs the real borealis UI against `source/sim/`, a simulated console. Environment knobs:

| Variable | Simulates |
|---|---|
| `PLAYGUARD_SIM_FW=20.5.0` | Another firmware (default 23.0.1, the newest verified on a console; above it, the firmware screen) |
| `PLAYGUARD_SIM_NO_CFW=1` | No Atmosphère |
| `PLAYGUARD_SIM_NOT_SET_UP=1` | Parental controls never set up (no PIN) |
| `PLAYGUARD_SIM_TIMER_OFF=1` | No play-time limit |
| `PLAYGUARD_SIM_RESTRICTED=1` | Today's limit reached |
| `PLAYGUARD_SIM_ALARM_ONLY=1` | *When the time is up* on a warning only: past the limit, the console does not restrict and the time left reads below 0 |
| `PLAYGUARD_SIM_NOT_COUNTING=1` | The time left not going down (a clock set back): the Overview says the console is not counting after 90 s on screen |
| `PLAYGUARD_SIM_UNLOCKED=1` | Parental controls temporarily unlocked |
| `PLAYGUARD_SIM_UNPAIRED=1` | No companion app linked |
| `PLAYGUARD_SIM_ALARM_OFF=1` | The "time's up" alarm off |
| `PLAYGUARD_SIM_ACCURATE=1` | An accurate network clock |
| `PLAYGUARD_SIM_AUTOSYNC_OFF=1` | *Synchronise Clock via Internet* off |
| `PLAYGUARD_SIM_NETWORK_OFFSET=1411` | A network clock that many seconds ahead of the console clock until it is set (the console clock follows it while *Synchronise* is on) |
| `PLAYGUARD_SIM_NTP=offline` | No time server answers (a console without Internet) |
| `PLAYGUARD_SIM_FAIL=timer,clock,unverified,bedtime,remove,launch,verify` | Failures: the play-timer write, setting the clock, an unlock the system does not confirm, a bedtime the console does not take, removing a game or a content, resetting a launch version, the integrity check |
| `PLAYGUARD_SIM_CHECK_MS=150` | How long checking one installed game takes (*Tools › Check the installed games*) |
| `PLAYGUARD_SIM_GAMES_OK=1` | Every installed game fine (*Tools › Check the installed games*; by default one of each problem) |

| `PLAYGUARD_SIM_EMUMMC=1` | Running on emuMMC |
| `PLAYGUARD_SIM_BLANK=1` | Atmosphère blanking the serial number |
| `PLAYGUARD_SIM_APPLET=1` | Applet (album) mode |
| `PLAYGUARD_SIM_NO_PDM=1` | No activity log |
| `PLAYGUARD_SIM_GAMES=200` | That many more games played this week (a large library) |
| `PLAYGUARD_SIM_IDLE=1` | No game running |
| `PLAYGUARD_SIM_BACKGROUND=1` | PlayGuard out of focus |
| `PLAYGUARD_SIM_REGION=2` | The console region |
| `PLAYGUARD_SIM_LATEST=1.1.0:24.0.0` | The latest release and the newest firmware it supports, for the update check (`offline` for no network) |
| `PLAYGUARD_SIM_SYS_LATEST=1.12.1:6.5.4:23.0.1` | The latest Atmosphère, hekate and firmware the daily update check finds (Atmosphère : hekate : firmware; `offline` for no network) |
| `PLAYGUARD_SIM_SYS_NOTES=<folder>` | What GitHub and switchbrew return for the release notes of *Tools › Console*: `atmosphere.json` and `hekate.json` (GitHub's release lists), `System_Versions.wiki` and `<version>.wiki` (switchbrew's pages, as wikitext); `offline` for no network |
| `PLAYGUARD_SIM_PASTE=https://gist.github.com/u/ab12` | What GitHub answers to *Send a report online* (`offline` for no network) |
| `PLAYGUARD_SIM_DEV_BUILDS=<folder>` | What the GitHub API returns for *Install another build*: `releases.json` (the releases tab), `latest.json` (the latest release, for *Update in place*), `artifacts.json`, `pulls.json`; `runs.json`, and for *Find a build* `pull_<n>.json`, `commit.json`, `run_artifacts.json`; `https://<local path>` download URLs are copied from disk (`offline` for no network); `PLAYGUARD_SIM_DEV_BUILDS_DELAY=<ms>` slows each answer |
| `PLAYGUARD_SIM_HELD_LR=1`, `PLAYGUARD_SIM_BOOT_CHOICE=a` | L + R held at start-up, and the answer of the start-up screen that offers the previous build (`a`: A, else B) |
| `PLAYGUARD_SIM_GITHUB_LOGIN=ok` | The GitHub device flow of *GitHub account*: `ok` approves at once, `denied` refuses, `offline` fails |
| `PLAYGUARD_SIM_HBLOADER=1` | A homebrew loader that can hand an update over to a store |
| `PLAYGUARD_SIM_NUMPAD=1:30` | What the system number pad returns |
| `PLAYGUARD_SIM_NOW=<POSIX seconds>` | A frozen console time (with `TZ=` for its time zone) |
| `PLAYGUARD_SIM_STILL_FOCUS=1` | Not the console: the focus highlight without its moving glow, and the loading screen's ring still, for screenshots that compare |
| `PLAYGUARD_SIM_VERSION=1.0.0:1a2b3c4` | Not the console: the version and commit the header's badge shows (`1.0.0:` for a release), not the build's own, for screenshots that compare |
| `PLAYGUARD_SIM_HOLD_LOADING=1` | Not the console: the start-up stops on the loading screen, at its last step (the `loading` smoke scenario's screenshot) |
| `PLAYGUARD_SIM_MODULES=agent:running` | Optional modules running at start (`name:running`, comma-separated); a module starts only when its `exefs.nsp` is on the simulated SD card |
| `PLAYGUARD_SIM_AGENT=running` | The agent sysmodule runs (an agent simulated in the process, with its own command dispatch, `sysmodule/agent/source/agent_core.c`): PlayGuard hands the link to it; what it pushes and answers is logged as `sim agent: …` |
| `PLAYGUARD_SIM_AGENT_ORDER=limit_uniform=90` | An order the simulated agent hands PlayGuard once, after `Hello` |
| `PLAYGUARD_SIM_BUNDLED=<folder>` | The optional modules PlayGuard carries (`<name>/exefs.nsp` and `version.txt`, as `cmake/bundle_sysmodules.cmake` writes them in the romfs) |

Game patches are read from `./playguard_data/sd/`, the simulated SD card root.

## Tests and CI

- `make test` — host unit tests (`tests/`, see [`tests/README.md`](tests/README.md)), including the recovery sysmodule logic (`tests/rescue/`), the remote link (`tests/sync_*`) and the agent (`tests/agent/`); every suite runs, in parallel, and leaves its output in `build/host-tests/<suite>.log`.
- `make check` — `make test`, then the resource check, the remote link's JSON documents (`tools/check_sync_json.py`) and `compat.json`.
- `tools/desktop_smoke.py <out-dir> [gate|errors|rescue|forged|lock|devbuild|update|library|outside|alarm|loading|sysnotes|sync|agent|modules]` — clicks through every screen of the desktop build headlessly (needs `DISPLAY`, `xdotool`, ImageMagick) and saves screenshots. The `gate` scenario covers the firmware screen and developer mode on a simulated 24.0.0; `errors` covers a failed unlock and an unsettable clock; `rescue` the recovery screen, then quitting while still unlocked; `forged` a rescue report the console does not confirm (no action, the lock screen first); `lock` the lock screen of *Ask for the PIN › To open PlayGuard*; `library` a large library in Activity (its first 50 games, then *Show every game*); `devbuild` signing in to a simulated GitHub and installing a pull request's build in place out of its artifact; `alarm` the Overview and the Play timer tab past the limit while the console only warns; `sysnotes` the release notes of the firmware, Atmosphère and hekate from *Tools › Console*, against a simulated GitHub and switchbrew. `outside` the Overview's notices: changes made outside PlayGuard (a play time that started over, the clock, the limits) and, after 90 s on screen, a console that is not counting play time (it takes about two minutes). `sync` the remote link against a local Mosquitto (needs `mosquitto` and `mosquitto-clients`): what it publishes, an order applied and one refused, Home Assistant restarting, and `offline` at exit; `agent` the link handed to a simulated agent sysmodule; `modules` installing, turning off at boot and removing the recovery module.
- `tools/visual_check.py` — compares those screenshots with the references in `tests/visual/`. A screen that changed fails CI; see [`tests/visual/README.md`](tests/visual/README.md) to update them.
- `python3 tools/check_resources.py .` — checks the XML layouts and translation catalogs.

CI (`.github/workflows/build.yml`) runs all of the above plus the Switch build, the smoke scenarios in parallel shards, on every pull request, every push to main and once a week. The visual check compares the sets in `tests/visual/` and fails on a changed screen (the simulator keeps the console time and the focus highlight still, so the screens come out the same in every run). The desktop and Switch builds turn PlayGuard's compiler warnings into errors (`make desktop CMAKE_ARGS=-DPLAYGUARD_WERROR=ON`, `make dist CMAKE_ARGS=-DPLAYGUARD_WERROR=ON`), and a compiler message is annotated on its line of the pull request; the Switch job also fails on a libnx older than 4.12.0 or an `.nro` without libnx's `LNY2` marker (`tools/check_nro.py`), and checks that a second build gives byte-identical zips; `check_resources.py` warns about sentences a catalog still has in English.

Each job writes a summary on the run page (`tools/ci_summary.py`): what is being built, the suites, the coverage per file, each smoke scenario with its visual check and its time, and for the Switch build the `.nro` size against main's last build, what makes it big (from the linker map: `python3 tools/ci_summary.py map build/borealis.map --nro playguard.nro` shows the same table for a local build), the toolchain and the downloads. A run's artifacts are named after its build, `playguard-<version>-<ref>-<commit>-<component>` (`1.0.0-pr92-1f0a059`, `1.0.0-main-2bd4996`): `-nro` (the app, what the developer tools download), `-rescue` (the recovery sysmodule, as the zip and the raw `.nsp`), `-linker-map`, `-screenshots-<shard>` (with a contact sheet per scenario), and on a release build `-sdcard` (the store zip). The runner image is pinned (`ubuntu-26.04`): the references of the visual check were drawn by its mesa, so moving to the next image is a pull request of its own. The devkitPro image is pinned by digest in `build.yml`; `toolchain-watch.yml` opens an issue once a month when Docker Hub's `latest` moved.

## Code layout

| Path | Content |
|---|---|
| `source/core/` | C, libnx: `pctl_ops`, `time_ops`, `sysinfo`, `playstats`, `gamecheck` (its rules in `gamecheck_rules`, shared with the simulator), `platform`, `rescue`, `pure`, `calendar`, `write_guard` (these last four need no console and are shared with the simulator), `modules_nx` (start / stop a sysmodule), `startup_hook` (times borealis' `userAppInit`, which libnx calls before `main`), `crash_handler` (PlayGuard's CPU exceptions to a crash file, before Atmosphère's report) |
| `source/tab/` | One class per tab |
| `source/action/` | Flows, one per thing a tab does, with their UI-free logic in `*_logic.cpp` where there is one: the play-timer write and its recorder, the clock (and a console no time server answers), the PIN prompt and the console lock, settings backups and restores, the change history and undo, changes seen outside PlayGuard and timer health, the notification bell, the firmware screen, the recovery screen, reports and sending them online, updates, GitHub sign-in and development builds, the SD-card write queue, clearing the cache and the log files, the remote link (`sync_flow`, `sync_orders`) |
| `source/activity/` | Screens: per-day editor, profiles, a game, first steps, change history, the notifications, firmware, recovery, the lock screen, the diagnostic report, what's new and the full changelog, the start-up error, the development builds, *Remote access* and *Optional modules*, the loading screen (the start-up runs one step per frame under it; the intro keeps it 2 s) |
| `source/view/` | Widgets: the week chart, the gauge, the play-timer state line, the day bars, the game cell, the QR codes and funding links, release notes, the scrolling frame, the loading screen's icon and progress bar, the notification bell |
| `source/ui/` | Dialogs, formatting, theme colours, the console's state as text, read-only greying and visibility |
| `source/util/` | NTP, HTTP, config, profiles, the optional modules on the SD card and the remote link's files, settings backups, change history, play-log folding, the play-data cache, the Activity summary, table export, diagnostics, the session logs and crash files (and the CPU exception dump), sending reports online, the play-timer recorder and block reference, update check, newer Atmosphère / hekate / firmware releases and their release notes (GitHub, switchbrew), store launcher, GitHub sign-in and development builds (zip, SHA-256), game patches, the bundled changelog, the start-up reminders and times, the background threads (joined, never detached: the console cannot) and which files the cache and the logs are |
| `source/sync/` | C, no allocation: the optional remote link (MQTT client, Home Assistant discovery, orders and their execution), shared with the agent sysmodule. See [`docs/sync-design.md`](docs/sync-design.md) |
| `source/sim/` | The simulated console for the desktop build |
| `resources/` | XML layouts and `i18n/<language>/playguard.json` |
| `sysmodule/` | The optional sysmodules, built with `sysmodule/common.mk`: `rescue/` (recovery at boot; shares `source/core/rescue.c` with the app) and `agent/` (the remote link in the background; links `source/core/` and `source/sync/`). See [`sysmodule/README.md`](sysmodule/README.md) |
| `packaging/` | Store and sphaira entries. See [`packaging/README.md`](packaging/README.md) |
| `branding/` | SVG sources of the icon and banners |
| `docs/` | [`parental-controls.md`](docs/parental-controls.md): what is known of the parental-control service, the play-timer block and the clocks, and how sure each fact is; [`companion-app.md`](docs/companion-app.md): what PlayGuard covers of Nintendo's phone app, and the gaps still to close; [`sync-design.md`](docs/sync-design.md) and [`sync-protocol.md`](docs/sync-protocol.md): the design and the wire contract of the optional remote link (MQTT, Home Assistant), PlayGuard's side and the agent sysmodule; [`home-assistant.md`](docs/home-assistant.md) (and `.fr.md`): the user guide |

**Branding:** `branding/*.svg` are rendered to `icon.jpg`, `images/store/*.png`, `images/social-preview.png` (the repository's social preview) and `resources/img/wordmark.png` (the loading screen's) by `node tools/render_branding.mjs` (Node + Playwright).

## Pull requests and releases

Releases are automated with [release-please](https://github.com/googleapis/release-please), which reads [conventional commits](https://www.conventionalcommits.org/):

- Give each pull request a conventional title with a scope, the part of the app it changes (`feat(history): …`, `fix(play-timer): …`, `docs(install): …`, `feat(notices)!: …` for a breaking change); CI checks it. release-please sorts each section of the changelog by scope, so a title without one lands apart from the rest.
- **Squash-merge** pull requests, so each one lands as a single commit carrying that title. A plain merge commit makes release-please apply a PR's `BEGIN_COMMIT_OVERRIDE` block to every commit of the PR.
- release-please keeps a `chore(main): release X.Y.Z` PR open; merging it tags the release and attaches the `.nro` / `.zip`.
- release-please runs with a token of the *PlayGuard Release* GitHub App (variable `RELEASE_APP_ID`, secret `RELEASE_APP_PRIVATE_KEY`; permissions Contents, Pull requests and Issues: write), not `GITHUB_TOKEN`: the app is on the tag ruleset's bypass list, so it can create the `vX.Y.Z` tag, and its pushes to the release PR run the required checks.
- **The release page** is laid out by the release job: a summary (`.github/release-notes/X.Y.Z.md`, merged before the release PR; else, with an `OPENROUTER_API_KEY` secret, one written by a language model), the figures, the commit list folded (users, then contributors), the screens that changed, the install section, the build report, and a discussion. [`.github/release-notes/README.md`](.github/release-notes/README.md) says how; *release-page* in Actions does it again for an existing release.

**Development builds:** nothing is published for them. The developer tools install the `playguard-<version>-<ref>-<commit>-nro` artifact of a build run (a zip holding `playguard.nro`; the artifacts API only filters by an exact name, so the app lists a few pages of a run's artifacts and keeps that one): main's last commits and the newest run of each open pull request from a branch of this repository; pull requests from forks are left out, as their code is not reviewed yet (`source/util/dev_builds.hpp`, [SECURITY.md](SECURITY.md)). GitHub hands artifacts to signed-in users only, hence the GitHub sign-in (device flow, GitHub App client ID in `source/util/github_auth.hpp`; with the Device Flow enabled and token expiry off; the app's only permissions are Actions: read and Gists: write). For a pull request, `PLAYGUARD_COMMIT` makes the app report its head commit, as the artifacts API names the run. Every build but a release one (`PLAYGUARD_RELEASE=1`, set for the `vX.Y.Z` tag's build) shows itself as a development build in the header, at the right: an amber **DEV** tag, the version and the commit, where a release shows a plain grey `v1.2.0` (`source/view/version_badge.hpp`).

Each release also publishes `compat.json` (`tools/gen_compat.py`: the version and the newest checked firmware), which the app's update check reads, plus `build-info.txt` and `SHA256SUMS.txt`. Details in [`packaging/README.md`](packaging/README.md). The `.nro`, both `.zip` and `compat.json` also carry a [build provenance attestation](https://docs.github.com/actions/security-for-github-actions/using-artifact-attestations): `gh attestation verify playguard.nro -R JigSawFr/PlayGuard` checks that a download was built by this repository's CI.

**Reproducible zips:** `make dist` and `make dist-rescue` give every file in the zip the date `SOURCE_DATE_EPOCH` (POSIX seconds; default: the last commit's time, `git log -1 --format=%ct`) and add the files in a fixed order without extra attributes, so the same commit and `.nro` give a byte-identical zip. The deko3d shaders are not compiled at build time (uam does not always give the same bytes): `shaders/` keeps them, with the checksums of the borealis GLSL they come from, and configure stops when that GLSL changes; then rebuild them with `-DPLAYGUARD_UAM=ON`, copy `build/dksh/*.dksh` to `shaders/` and update `shaders/glsl.sha256`. Set `SOURCE_DATE_EPOCH=<seconds>` to rebuild one from another checkout. CI builds the zips twice and compares them; when they differ, `tools/repro_diff.py` says where (a zip entry, a segment of the `.nro`, a RomFS file, a shader) and the run keeps both builds as an artifact.

When a change is visible to users, update the English and French pages **both**. The READMEs ([README.md](README.md), [README.fr.md](README.fr.md)) stay short (about one screen): the pitch, features, compatibility, install, quick start and links; the details go to the user guide in [`docs/`](docs/) — `features.md`, `install.md`, `compatibility.md`, `troubleshooting.md` — with its French version in [`docs/fr/`](docs/fr/). The reference pages (`config.md`, `parental-controls.md`, `companion-app.md`) are in English only.

## Translating PlayGuard

PlayGuard is translated into every language the console offers: English, French (France and Canada), German, Spanish (Spain and Latin America), Italian, Dutch, Portuguese (Portugal and Brazil), Russian, Japanese, Korean and Chinese (simplified and traditional). It follows the console's language unless *Preferences › Language* picks another one. British English consoles get the English strings, already in British spelling (`CMakeLists.txt` copies them under `en-GB`). Fixes from native speakers are welcome.

To add a language:

1. Copy `resources/i18n/en-US/playguard.json` to `resources/i18n/<code>/playguard.json` (`<code>` as the console names its language, see `brls::LOCALE_*`) and translate the values. Keep every key, and as many `{}` placeholders as the English text, in the same order.
2. Add `<code>` to `config::LANGUAGES` in `source/util/config.hpp` (the language picker and the saved preference use that list).
3. Add the language's own name under `"tools" › "languages"` in **every** `playguard.json` (`"ru": "Русский"`). A regional variant (`fr-CA`, `es-419`, `pt-BR`) is a full catalog of its own: borealis falls back to English, not to the base language.
4. Optionally add `resources/i18n/<code>/hints.json` for borealis' button hints when borealis has none for that language (see `resources/i18n/fr/hints.json`); missing strings fall back to English.
5. Run `python3 tools/check_resources.py .`: it reports missing or extra keys, placeholder mismatches and a language missing from the picker. CI runs the same check.
