# Installing and updating

The release zip (`playguard.zip`) is meant to be extracted at the root of the SD card:

```
switch/playguard/playguard.nro     the app (hbmenu, sphaira, hb-appstore)
switch/playguard/LICENSE.txt       the GPLv3 text
config/sphaira/github/playguard.json     sphaira "GitHub" updater entry
```

At runtime the app writes its files in `config/playguard/` (not next to itself: removing or reinstalling the
app keeps them; a version that kept them in `switch/playguard/` has them moved at its first start,
`source/util/data_move.hpp`):

```
config/playguard/config.json             preferences (language, theme, NTP server, …)
config/playguard/profiles/*.json         saved play-time limit profiles
config/playguard/exports/activity_*      Activity exports (CSV, JSON, XLSX, PDF)
config/playguard/logs/reports/report_<date>.txt   diagnostic reports (never contain the PIN)
config/playguard/logs/crashes/crash_<date>.txt    one file per crash
config/playguard/logs/sessions/session_<date>.txt everything logged during one run
config/playguard/logs/uploads/upload_<date>.url   one per report sent online: an Internet shortcut to its gist
                                         (all four pruned as About › Help & diagnostics says: number, age, size)
config/playguard/logs/play_timer/log.csv          Help & diagnostics › Developer › Record the play timer (log.old.csv: the previous one)
config/playguard/logs/play_timer/block.json       Help & diagnostics › Developer › Compare the play-timer block (reference)
config/playguard/config.json.bad         a config.json that could not be read, kept aside rather than overwritten (.bad.1: the
                                         previous one); history.json.bad and state.json.bad the same
config/playguard/cache/                  the game icons (icons/, 16 MB at most), the development build list (dev_builds.json)
config/playguard/nand/<system>/          what belongs to one system (util/nand.hpp): sysmmc/ or emummc/
config/playguard/nand/<system>/state.json     Block all games, the extra time, a pending relock, the firmware choice
config/playguard/nand/<system>/history.json   the change history (the newest 200 changes)
config/playguard/nand/<system>/watch.json     what PlayGuard last saw of the console (today's play time, the clock, the
                                              limits, the user accounts), to tell what changed outside it (docs/config.md)
config/playguard/nand/<system>/backups/*.json settings backups (never contain the PIN)
config/playguard/nand/<system>/cache/         play data per account (activity_*.bin)
config/playguard/nand/<system>/sync/          with Remote access: the records the optional agent needs (nro_state.txt),
                                              the game names (names.txt), and what the agent wrote (agent_state.txt,
                                              agent_events.log)
config/playguard/sync.conf               Remote access: the link's settings, the broker's password included (plain text;
                                         never in a report or an upload); shared by every system
config/playguard/sync/profiles.txt       Remote access: the saved profiles, for the optional agent
config/playguard/github_token            GitHub sign-in of the developer tools: the token in plain text,
                                         treat it as a password (Disconnect deletes it)
config/playguard/rescue_report.txt       left by the recovery sysmodule, read at start-up
config/playguard/rescue_nonce            the challenge the recovery sysmodule answers in its report
```

The optional sysmodules (`playguard-rescue.zip` and `playguard-agent.zip`, separate downloads; PlayGuard also
installs them from *Console › Optional modules*) are extracted at the root of the SD card too:

```
atmosphere/contents/4200000000505247/exefs.nsp          the recovery sysmodule (see sysmodule/rescue/README.md)
atmosphere/contents/4200000000505247/flags/boot2.flag   started by Atmosphère at boot
atmosphere/contents/4200000000505247/toolbox.json       its name for sysmodule managers
atmosphere/contents/4200000000505247/LICENSE.txt        the GPLv3 text
atmosphere/contents/4200000000504741/…                  the remote link's agent, the same files (see sysmodule/agent/README.md)
```

Installed from PlayGuard, each folder also has a `version.txt` (the build it is), and an update keeps the previous
`exefs.nsp` as `exefs.nsp.bak` until the new one answers.

## Channels

| Channel | How it finds updates |
|---|---|
| Homebrew App Store (4TU) and sphaira's App Store | The listing follows the GitHub releases of this repository and picks up the `.zip` asset (see [hb-appstore/README.md](hb-appstore/README.md)). |
| sphaira › GitHub | Reads [`sphaira/playguard.json`](sphaira/playguard.json): downloads the `playguard.nro` asset of the latest release into `/switch/playguard/`. |
| Manual | Extract the zip at the SD card root. |

Release assets are produced by `.github/workflows/build.yml`, called by `release-please.yml` when a release PR is merged: `playguard.nro`, `playguard.zip`, `playguard-rescue.zip` and `playguard-agent.zip` (the optional sysmodules, which the `.nro` also carries and installs from *Console › Optional modules*), `compat.json` and `compat-1.json` (the version and the newest checked firmware, read by the app's update check from `releases/latest/download/compat-1.json`, or `compat.json` (schema 1 forever, for the versions before) when it is missing), `build-info.txt` (the commit, the submodules and the devkitPro image and packages used), `SHA256SUMS.txt`. The Switch job runs in the `devkitpro/devkita64` image pinned by digest in `build.yml` (`run.sh` reads it there for local builds, and `toolchain-watch.yml` opens an issue once a month when Docker Hub's `latest` moved on), so a release can be rebuilt with the same toolchain; it fails on a libnx older than 4.12.0 or an `.nro` without libnx's `LNY2` marker (`tools/check_nro.py`). The job refuses to publish when the tag differs from the version in `CMakeLists.txt` (which is also the NACP version the stores display).
