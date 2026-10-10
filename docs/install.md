# Install

How to install PlayGuard, and what it writes on the SD card. Back to the [README](../README.md).

## Where to get it

Pick one:

- **Homebrew App Store** / **sphaira's App Store** (same catalogue) — Search for *PlayGuard* once the listing is approved. New releases are picked up automatically.
- **sphaira › GitHub** — The release zip already contains the entry (`/config/sphaira/github/playguard.json`): after a first install, update from *GitHub* in sphaira.
- **Manual** — Download `playguard.zip` from the [latest release](https://github.com/JigSawFr/PlayGuard/releases/latest) and extract it to the **root** of the SD card. The app lands in `sd:/switch/playguard/`, its files in `sd:/config/playguard/`.

The optional **recovery sysmodule** is installed from PlayGuard (*Tools › Optional modules*, or *Security & app › Locked out?*), or by hand from its own download (`playguard-rescue.zip`) — see [Locked out?](troubleshooting.md#locked-out).

## Files on the SD card

All in `sd:/config/playguard/` (the app itself stays in `sd:/switch/playguard/`: removing or reinstalling it keeps them). Versions that kept them next to the app had them moved there at their first start.

- **`config.json`** — Preferences (language, theme, NTP server, *Ask for the PIN* …); every key in [config.md](config.md). One that cannot be read is kept as `config.json.bad` (the previous one as `.bad.1`) and the settings start again from their defaults; PlayGuard says so once at start-up
- **`profiles/`** — Saved play-time limit profiles
- **`exports/`** — Activity exports
- **`nand/<system>/`** — What belongs to one system, each with its own parental controls: `sysmmc/` for the sysMMC, `emummc/` for the emuMMC. Inside: `history.json` the change history (newest 500, a change the console refused included; one that cannot be read is kept as `history.json.bad`, the previous one as `.bad.1`, rather than overwritten, and PlayGuard says so once); `state.json` what the console lock, the extra time, a pending relock and the firmware choice need remembered ([config.md](config.md#state-kept-between-launches)); `watch.json` what PlayGuard last saw of the console, to tell what changed outside it ([config.md](config.md#watchjson); deleting it is harmless); `backups/` the settings backups (never contain the PIN); `cache/` the last play activity read (every account, and each account viewed), shown at once on the next start while the log is read again. The files of a version before these folders go to the first system PlayGuard starts on
- **`cache/`** — The game icons (`icons/`, 16 MB at most, emptied when the console language changes), shared by every system, so names and icons are not read from each game again at every start; the release notes of Atmosphère, hekate and the firmware (`sys_notes.json`, for *Tools › Console*); in developer mode, the list of *Install another build* (`dev_builds.json`)
- **`github_token`** — Developer mode only: the GitHub sign-in of *Install another build* (deleted by signing out)
- **`previous_build.json`**, **`boot_check.json`** — After a build is installed in place: the name of the build replaced (kept next to the `.nro` as `playguard.nro.previous`), and the check of the next start (deleted once its first screen is up)
- **`rescue_report.txt`** — Left by the recovery sysmodule after it acted, until PlayGuard shows it at start-up (one it cannot read is removed, and PlayGuard says so)
- **`sync.conf`** — Only once *Remote access* was set up: the link's settings, the broker's password included (never in a report or an upload)
- **`sync/`** — Only with *Remote access*: what a future background module will need (records, profiles, game names)
- **`logs/`** — `reports/` the saved diagnostic reports (never contain the PIN or the serial number); `crashes/` one file per crash (what made PlayGuard stop); `uploads/` one Internet shortcut (`.url`) per report sent online; `play_timer/` the developer tools' play-timer files. How many reports, crash files and links are kept: *Tools › Diagnostics* (by number, age or size; the newest always stays)

More in [packaging/README.md](../packaging/README.md).
