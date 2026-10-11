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
config/playguard/logs/uploads/upload_<date>.txt   one per report sent online: its link (and bpa.st's removal link)
                                         (all three pruned as Tools › Diagnostics says: number, age, size)
config/playguard/logs/play_timer/log.csv          Developer tools › record the play timer (log.old.csv: the previous one)
config/playguard/logs/play_timer/block.json       Developer tools › play-timer block reference
config/playguard/config.json.bad         a config.json that could not be read, kept aside rather than overwritten (.bad.1: the
                                         previous one); history.json.bad and state.json.bad the same
config/playguard/cache/                  the game icons (icons/, 16 MB at most), the development build list (dev_builds.json)
config/playguard/nand/<system>/          what belongs to one system (util/nand.hpp): sysmmc/, or emummc-<path in
                                         emuMMC/emummc.ini>/ for each emuMMC (emummc-sd00)
config/playguard/nand/<system>/state.json     the console lock, the extra time, a pending relock, the firmware choice
config/playguard/nand/<system>/history.json   the change history (the newest 200 changes)
config/playguard/nand/<system>/watch.json     what PlayGuard last saw of the console (today's play time, the clock, the
                                              limits, the user accounts), to tell what changed outside it (docs/config.md)
config/playguard/nand/<system>/backups/*.json settings backups (never contain the PIN)
config/playguard/nand/<system>/cache/         play data per account (activity_*.bin)
config/playguard/github_token            GitHub sign-in of the developer tools: the token in plain text,
                                         treat it as a password (Disconnect deletes it)
config/playguard/rescue_report.txt       left by the recovery sysmodule, read at start-up
```

The optional recovery sysmodule (`playguard-rescue.zip`, a separate download) is also extracted at the root of the SD card:

```
atmosphere/contents/4200000000505247/exefs.nsp          the sysmodule (see sysmodule/README.md)
atmosphere/contents/4200000000505247/flags/boot2.flag   started by Atmosphère at boot
atmosphere/contents/4200000000505247/LICENSE.txt        the GPLv3 text
```

## Channels

| Channel | How it finds updates |
|---|---|
| Homebrew App Store (4TU) and sphaira's App Store | The listing follows the GitHub releases of this repository and picks up the `.zip` asset (see [hb-appstore/README.md](hb-appstore/README.md)). |
| sphaira › GitHub | Reads [`sphaira/playguard.json`](sphaira/playguard.json): downloads the `playguard.nro` asset of the latest release into `/switch/playguard/`. |
| Manual | Extract the zip at the SD card root. |

Release assets are produced by `.github/workflows/build.yml`, called by `release-please.yml` when a release PR is merged: `playguard.nro`, `playguard.zip`, `playguard-rescue.zip` (the optional recovery sysmodule), `compat.json` (the version and the newest checked firmware, read by the app's update check from `releases/latest/download/compat.json`), `build-info.txt` (the commit, the submodules and the devkitPro image and packages used), `SHA256SUMS.txt`. The Switch job runs in the `devkitpro/devkita64` image pinned by digest in `build.yml` (`run.sh` reads it there for local builds, and `toolchain-watch.yml` opens an issue once a month when Docker Hub's `latest` moved on), so a release can be rebuilt with the same toolchain; it fails on a libnx older than 4.12.0 or an `.nro` without libnx's `LNY2` marker (`tools/check_nro.py`). The job refuses to publish when the tag differs from the version in `CMakeLists.txt` (which is also the NACP version the stores display).
