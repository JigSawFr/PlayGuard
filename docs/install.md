# Install

How to install, update and remove PlayGuard, and what it writes on the SD card. Once installed: [getting started](getting-started.md). Back to the [README](../README.md).

## Where to get it

Pick one:

- **Homebrew App Store** / **sphaira's App Store** (same catalogue) — Search for *PlayGuard* and install it. New releases are picked up automatically, after a review: often a few days after GitHub. PlayGuard knows it was installed from there and recommends updating through the store.
- **Manual** — Download `playguard.zip` from the [latest release](https://github.com/JigSawFr/PlayGuard/releases/latest) and extract it to the **root** of the SD card. The app lands in `sd:/switch/playguard/`, its files in `sd:/config/playguard/`.
- **sphaira › GitHub** — The release zip already contains the entry (`/config/sphaira/github/playguard.json`): after a first install, update from *GitHub* in sphaira.

The optional **recovery sysmodule** is installed from PlayGuard (*Console › Optional modules*, or *Security › Locked out?*), or by hand from its own download (`playguard-rescue.zip`): extract it to the root of the SD card too, then restart the console. It lands in `atmosphere/contents/4200000000505247/` and only acts when a `RESCUE` file asks it to — see [Locked out?](troubleshooting.md#locked-out).

Then open PlayGuard **over a game**, with a parent's user ([why](compatibility.md#launchers)).

## Updating

- **In PlayGuard:** *Preferences › Update with* picks how — in PlayGuard itself (it downloads the release, checks it, replaces itself and restarts), sphaira, the Homebrew App Store or by hand. With *Check for updates every day* on, PlayGuard opens on the new release when there is one.
- **The recovery sysmodule** follows PlayGuard's version: after each update, extract that release's `playguard-rescue.zip` again (or *Update* on the bell's *The recovery sysmodule is out of date*, or in *Console › Optional modules*), then restart the console.
- **The remote link's agent**, when installed, is offered at the next start when PlayGuard carries a newer one ([Home Assistant](home-assistant.md#keep-the-link-up-when-playguard-is-closed)).
- Settings, history, profiles and backups stay: they are in `sd:/config/playguard/`, not next to the app.

## Removing it

Delete `sd:/switch/playguard/` (the app) and, to remove its files too, `sd:/config/playguard/`; with the optional sysmodules, also `sd:/atmosphere/contents/4200000000505247/` (recovery) and `sd:/atmosphere/contents/4200000000504741/` (the agent), or remove them first in *Console › Optional modules*. The limits, the PIN and the restrictions are the console's own: they **stay on the console**. To remove them as well, use *Security › Delete all parental controls* first.

## Files on the SD card

All in `sd:/config/playguard/` (the app itself stays in `sd:/switch/playguard/`: removing or reinstalling it keeps them). Versions that kept them next to the app had them moved there at their first start.

- **`config.json`** — Preferences (language, theme, NTP server, *Protect PlayGuard with the PIN* …); every key in [config.md](config.md). One that cannot be read is kept as `config.json.bad` (the previous one as `.bad.1`) and the settings start again from their defaults; PlayGuard says so once at start-up
- **`profiles/`** — Saved play-time limit profiles
- **`exports/`** — Activity exports
- **`nand/<system>/`** — What belongs to one system, each with its own parental controls: `sysmmc/` for the sysMMC, `emummc/` for the emuMMC. Inside: `history.json` the change history (newest 500, a change the console refused included; one that cannot be read is kept as `history.json.bad`, the previous one as `.bad.1`, rather than overwritten, and PlayGuard says so once); `state.json` what *Block all games*, the extra time, a pending relock and the firmware choice need remembered ([config.md](config.md#state-kept-between-launches)); `forwarders.json` the forwarders PlayGuard made; `watch.json` what PlayGuard last saw of the console, to tell what changed outside it ([config.md](config.md#watchjson); deleting it is harmless); `backups/` the settings backups (never contain the PIN); `cache/` the last play activity read (every account, and each account viewed), shown at once on the next start while the log is read again. The files of a version before these folders go to the first system PlayGuard starts on
- **`cache/`** — The game icons (`icons/`, 16 MB at most, emptied when the console language changes), shared by every system, so names and icons are not read from each game again at every start; the release notes of Atmosphère, hekate, sys-patch and the firmware (`sys_notes.json`, for *Console*) and PlayGuard's own (for *About*); in developer mode, the list of *Install another build* (`dev_builds.json`)
- **`github_token`** — The GitHub sign-in of *Send a report…* and, in developer mode, of *Install another build*; deleted by signing out (*About › Help & diagnostics › GitHub account*, shown once signed in)
- **`gate.nro`**, **`gate-<hash>.nro`**, **`gate_*.txt`** — The protected forwarders' gate (*Console › Forwarders*): its program, its free hours, closed hours and pause after refusals, its tries until PlayGuard reads them, and how long its opening screen lasts (`gate_wait.txt`, read at each start)
- **`previous_build.json`**, **`boot_check.json`** — After a build is installed in place: the name of the build replaced (kept next to the `.nro` as `playguard.nro.previous`), and the check of the next start (deleted once its first screen is up)
- **`rescue_report.txt`** — Left by the recovery sysmodule after it acted, until PlayGuard shows it at start-up (one it cannot read is removed, and PlayGuard says so)
- **`rescue_nonce`** — A random challenge the recovery sysmodule answers in its report, so PlayGuard can tell a report it wrote from one written by hand ([sysmodule/rescue/README.md](../sysmodule/rescue/README.md)); a new one at every start
- **`sync.conf`** — Only once *Remote access* was set up: the link's settings, the broker's password included (never in a report or an upload)
- **`sync/`** — Only with *Remote access*: the saved profiles, for the optional agent (`profiles.txt`); what it reads and writes for one system is in that system's `nand/<system>/sync/` (records, game names, what it changed, its events). A sysMMC and an emuMMC share `sync.conf` but are two devices in Home Assistant
- **`logs/`** — `reports/` the saved diagnostic reports (never contain the PIN or the serial number); `crashes/` one file per crash (what made PlayGuard stop); `uploads/` one Internet shortcut (`.url`) per report sent online; `play_timer/` the developer tools' play-timer files. How many reports, crash files and links are kept: *About › Help & diagnostics* (by number, age or size; the newest always stays)

More in [packaging/README.md](../packaging/README.md).
