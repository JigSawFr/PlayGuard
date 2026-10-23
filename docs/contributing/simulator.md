# The desktop simulator

Part of the [contributor's guide](../../CONTRIBUTING.md).

`make desktop` runs the real borealis UI against `source/sim/`, a simulated console. Environment knobs (a flag is on with any value but empty, `0`, `false`, `off` or `no`; a number out of range is brought within it):

| Variable | Simulates |
|---|---|
| `PLAYGUARD_SIM_FW=20.5.0` | Another firmware (default 23.0.1, the newest verified on a console; above it, the firmware screen) |
| `PLAYGUARD_SIM_NO_CFW=1` | No Atmosphère |
| `PLAYGUARD_SIM_NOT_SET_UP=1` | Parental controls never set up (no PIN) |
| `PLAYGUARD_SIM_TIMER_OFF=1` | No play-time limit |
| `PLAYGUARD_SIM_RESTRICTED=1` | Today's limit reached |
| `PLAYGUARD_SIM_ALARM_ONLY=1` | *When the time is up* on a warning only: past the limit, the console does not restrict and the time left reads below 0 |
| `PLAYGUARD_SIM_NOT_COUNTING=1` | The time left not going down (a clock set back): *Today* says the console is not counting after 90 s on screen |
| `PLAYGUARD_SIM_STALL_S=20` | With `PLAYGUARD_SIM_NOT_COUNTING`: *Today* says so after that many seconds on screen instead of 90 (the smoke test's `outside` scenario) |
| `PLAYGUARD_SIM_TAB_RACE=1` | A read of the play data ending as each tab opens, the tab it replaces not freed yet (the smoke test's `alarm` scenario): the header's pill must still be the new tab's |
| `PLAYGUARD_SIM_UNLOCKED=1` | Parental controls temporarily unlocked |
| `PLAYGUARD_SIM_UNPAIRED=1` | No companion app linked |
| `PLAYGUARD_SIM_ALARM_OFF=1` | The "time's up" alarm off |
| `PLAYGUARD_SIM_ACCURATE=1` | An accurate network clock |
| `PLAYGUARD_SIM_AUTOSYNC_OFF=1` | *Synchronise Clock via Internet* off |
| `PLAYGUARD_SIM_NETWORK_OFFSET=1411` | A network clock that many seconds ahead of the console clock until it is set (the console clock follows it while *Synchronise* is on) |
| `PLAYGUARD_SIM_NTP=offline` | No time server answers (a console without Internet). In a browser, which sends no UDP, they answer otherwise with the simulated world's time (the console's before any clock change) |
| `PLAYGUARD_SIM_FAIL=timer,clock,unverified,bedtime,remove,launch,verify,forwarder` | Failures: the play-timer write, setting the clock, an unlock the system does not confirm, a bedtime the console does not take, removing a game or a content, resetting a launch version, the integrity check, installing or removing a forwarder (*Console › Forwarders*) |
| `PLAYGUARD_SIM_CHECK_MS=150` | How long checking one installed game takes (*Console › Installed games*) |
| `PLAYGUARD_SIM_GAMES_OK=1` | Every installed game fine (*Console › Installed games*; by default one of each problem) |
| `PLAYGUARD_SIM_SD_FREE_MB=300` | The SD card's free space, in MB (unset: unknown, as the desktop's own disk says nothing of a console; under 500 MB, *The SD card is almost full* on the Console tab) |
| `PLAYGUARD_SIM_EMUMMC=1` | Running on emuMMC |
| `PLAYGUARD_SIM_BLANK=1` | Atmosphère blanking the serial number |
| `PLAYGUARD_SIM_APPLET=1` | Applet (album) mode |
| `PLAYGUARD_SIM_FROM_FORWARDER=1` | Started from a forwarder's HOME menu icon, not over a game (Today's pill and Help say so) |
| `PLAYGUARD_SIM_NO_GAME=1` | No game open (applet mode with nothing behind it): no "game left open keeps counting" line, no "the game in progress is suspended" warning |
| `PLAYGUARD_SIM_DOCKED=1` | The console on its dock: *Text size › Automatic* draws the larger text |
| `PLAYGUARD_SIM_BATTERY=1` | The console in hand, off its dock (unset: the simulator draws the effects as on the dock, whatever `PLAYGUARD_SIM_DOCKED` says): *Visual effects › Automatic*, the default, leaves out the glow and every movement of the look (`ui/effects`, `ui/motion`) |
| `PLAYGUARD_SIM_LEFT_MIN=4` | That many minutes left today while the timer counts (the child screen's last minutes) |
| `PLAYGUARD_SIM_NO_PDM=1` | No activity log |
| `PLAYGUARD_SIM_NO_PLAY=1` | Nothing played for weeks (the all-time totals stay): the Activity tab's empty state |
| `PLAYGUARD_SIM_GAMES=200` | That many more games played this week (a large library) |
| `PLAYGUARD_SIM_IDLE=1` | No game running |
| `PLAYGUARD_SIM_SKIP=900` | The console's time that many seconds later, a game played all along (unless `IDLE`): the time left goes down by as much, to the limit and past it, and the Activity tab counts it on the first game (read at each call: the web page's time buttons move it while PlayGuard runs) |
| `PLAYGUARD_SIM_ACCOUNTS=5` | That many user accounts, 0 to 8 (default 2: Alice and Léo), each playing twice as much as the next |
| `PLAYGUARD_SIM_KEEP=1` | The simulated console's settings (PIN, limits, bedtime, clocks) and what *Console › Installed games* changed (removed games, launch versions reset) kept in `playguard_data/sim_console.bin` from one run to the next; the variables that set its starting state then count for the first run only. The console then lives on (`source/sim/sim_games.c`): its time spent today counts in real time while a game runs (and with `PLAYGUARD_SIM_SKIP`'s steps), to the limit when it suspends the game, from 0 on a new day, kept too; the user clock moved by the network clock starts it over, set back it counts nothing until it passes the old time again (as observed, `docs/parental-controls.md`). A run counts from its own start: the time between two runs, or a step of more than 10 minutes, is not play |
| `PLAYGUARD_SIM_BACKGROUND=1` | PlayGuard out of focus |
| `PLAYGUARD_SIM_REGION=2` | The console region |
| `PLAYGUARD_SIM_LATEST=1.1.0:24.0.0` | The latest release and the newest firmware it supports, for the update check (`offline` for no network) |
| `PLAYGUARD_SIM_STORE=1.1.0` | The version of PlayGuard the Homebrew App Store's catalogue has (`none`: not listed, `offline` for no network): with its record on the simulated SD card (`playguard_data/sd/switch/appstore/.get/packages/PlayGuard/manifest.install`), an update through a store first checks the catalogue has the new version |
| `PLAYGUARD_SIM_SYS_LATEST=1.12.1:6.5.4:23.0.1:1.6.3.0` | The latest Atmosphère, hekate, firmware and sys-patch the daily update check finds (Atmosphère : hekate : firmware : sys-patch, the last optional; `offline` for no network) |
| `PLAYGUARD_SIM_SYS_NOTES=<folder>` | What GitHub and switchbrew return for the release notes in *Console*: `atmosphere.json` and `hekate.json` (GitHub's release lists), `System_Versions.wiki` and `<version>.wiki` (switchbrew's pages, as wikitext); `offline` for no network |
| `PLAYGUARD_SIM_PASTE=https://gist.github.com/u/ab12` | What GitHub answers to *Send a report…* (`offline` for no network) |
| `PLAYGUARD_SIM_DEV_BUILDS=<folder>` | What the GitHub API returns for *Install another build*: `releases.json` (the releases tab), `latest.json` (the latest release, for *Update in place*), `artifacts.json`, `pulls.json`; `runs.json`, and for *Find a build* `pull_<n>.json`, `commit.json`, `run_artifacts.json`; `https://<local path>` download URLs are copied from disk (`offline` for no network); `PLAYGUARD_SIM_DEV_BUILDS_DELAY=<ms>` slows each answer |
| `PLAYGUARD_SIM_BOOT_CHOICE=a` | The answer of the start-up screen that offers the previous build after a start that did not finish (`a`: A, else B) |
| `PLAYGUARD_SIM_GITHUB_LOGIN=ok` | The GitHub device flow of *GitHub account*: `ok` approves at once, `denied` refuses, `offline` fails |
| `PLAYGUARD_SIM_HBLOADER=1` | A homebrew loader that can hand an update over to a store |
| `PLAYGUARD_SIM_FORWARDERS=1` | Console › Forwarders in use: PlayGuard's own, sphaira protected and seen working, Star Atlas without the PIN, the signature patches, four screenshots in the album dated from the console's day (two in Star Kart Racers the same minute today, one yesterday, one on the HOME menu nine days ago) |
| `PLAYGUARD_SIM_NOW=<POSIX seconds>` | A frozen console time (with `TZ=` for its time zone) |
| `PLAYGUARD_SIM_STILL_FOCUS=1` | Not the console: the focus highlight without its moving glow, the loading screen's ring still, every movement of the look drawn at its end (`effects::motion()`: no entrance, no pulse, no slide), and the footer's hints never greyed while a screen or a dialog opens or closes (borealis patch `0014`), for screenshots that compare |
| `PLAYGUARD_SIM_VERSION=1.0.0:1a2b3c4` | Not the console: the version and commit the header's badge shows (`1.0.0:` for a release), not the build's own, for screenshots that compare |
| `PLAYGUARD_SIM_HOLD_LOADING=1` | Not the console: the start-up stops on the loading screen, at its last step (the `loading` smoke scenario's screenshot) |
| `PLAYGUARD_SIM_CADRAN_NOGLOW=1` | Not the console: no background glow (`PLAYGUARD_SIM_CADRAN_NORING=1`: no ring on Today), the two custom draws of the look after the design boards (`source/view/cadran_*.cpp`: the rail's icons, today's ring, the figures in Sora from `resources/font/`, OFL), to weigh them |
| `PLAYGUARD_SIM_CUT_TEXT=1` | Not the console: once a second, every label the screen on top cuts short ("…") goes to `cut_text.txt` in the working directory with the screen's name (`dashboard › dialog`, `… › picker` for one opened over it), once each (`tools/cut_text_check.py`) |
| `PLAYGUARD_SIM_FOCUS_LOG=1` | Not the console: each line or button that takes the focus logs its words (`focus: Hours without the PIN`), so `tools/desktop_smoke.py` reaches a line by its words (`go_to()`), not by counting presses |
| `PLAYGUARD_SIM_STATS=<file>` | Not the console: one line per frame (frame time with the FPS limiter off; nanovg's draw calls and triangles with `tools/perf_nanovg_stats.patch`), see `tools/perf_frames.sh`. On the console (and the desktop), *About › Help & diagnostics › Developer › Frame statistics* writes one line per second to `logs/frame_stats.txt` instead: the mode (docked, handheld), the UI thread's own time per frame and the late frames (`source/action/frame_stats_flow.cpp`) |

Game patches are read from `./playguard_data/sd/`, the simulated SD card root; *Console › Forwarders* writes a few made-up homebrew there (`switch/*.nro`, `source/sim/sim_forwarder.cpp`) to make forwarders for.

## In a browser

The same simulator runs in a browser, with these variables in the URL (`?unlocked=1&fw=20.5.0`) and a panel to change them: [main's](https://playguard.link/simulator/), on PlayGuard's website, and each pull request's, linked in a comment on it. See [`web/README.md`](../../web/README.md). The page is in the console's 15 languages, the browser's unless the URL says `lang=`, with an *Auto* / *Light* / *Dark* theme, and PlayGuard starts in both. A new `PLAYGUARD_SIM_*` variable needs its entry in `web/params.json` and its label in `web/i18n/en-US.json` (`tools/check_web_params.py` fails CI without them); the other catalogs show the English one until they have theirs. Read it with `sim_env_flag()`, `sim_env_int()` or, for a text, `sim_env()` (`source/core/sim_env.h`), never `getenv()`: the web page's live changes go there, and it is unset on the console. Describe it in the table above too (the same check fails without it). Mark it `"live": true` when PlayGuard reads it at each use (not once at start-up): the page then changes it without a restart. The check fails on `live` for a variable read once only: into a static, or into the simulated console's first state (`sim_defaults()`, `source/sim/sim_state.c`). CI opens the build in headless Chromium (`tools/web_smoke.py`), and the pull request's comment shows its screens.
