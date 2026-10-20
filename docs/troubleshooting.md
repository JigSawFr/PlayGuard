# Troubleshooting

Getting back into a locked console, reporting a bug, and the developer mode. Back to the [README](../README.md).

## Locked out?

PlayGuard only sees the parental controls of the system it runs on: **emuMMC and sysMMC each have their own** (a PIN removed on one is still there on the other). Run it on each one that needs fixing.

### Second-hand console

You know the PIN, but the previous owner's phone app is still linked (unlinking fails, or a factory reset asks for their account).

*PIN › Unlink the companion app*, then, if you want no parental controls at all, *Delete all parental controls*. Both work offline, on emuMMC as on sysMMC.

### PIN forgotten

*Show the PIN* asks for the PIN itself, so it cannot help here. With the recovery sysmodule installed (below), drop a `RESCUE` file and restart: the recovery screen then sets a new one (it never shows the old one). Without it, *Delete all parental controls* starts again (a settings backup is saved first; it never contains the PIN): with *Protect PlayGuard with the PIN* on (the default, *Before a change*), put the SD card in a computer and set `"pin_lock"` to `"off"` in `sd:/config/playguard/config.json` first.

### The play timer blocks everything (0-minute limit) and the PIN is forgotten

PlayGuard itself cannot start then. Install the optional recovery sysmodule (`playguard-rescue.zip`) **beforehand**; when locked out, drop an empty `config/playguard/RESCUE` file on the SD card and boot — it unlocks the console so PlayGuard can open. See [`sysmodule/README.md`](../sysmodule/README.md).

The module must follow PlayGuard's version: with each PlayGuard update, extract that release's `playguard-rescue.zip` again (or *Update* on the notification bell's *The recovery sysmodule is out of date*), then restart the console. An older module leaves a report PlayGuard cannot check: the recovery screen then offers nothing, says the module is older, and doing `RESCUE` again gives the same until it is updated.

### Console not modded

PlayGuard cannot help: it needs Atmosphère. Nintendo support's master-key procedure is the official way.

## Reporting a bug

An error shows the system's code (such as `0x0001088E`) with what is known about it: what to do for the codes whose meaning is documented, otherwise a report helps.

1. *About › Help & diagnostics › Export a diagnostic report* saves a text file in `sd:/config/playguard/logs/reports/`: firmware, Atmosphère version, how long the start-up took and how long the slow screens took (the change history, *All games*, the installed games check, the tabs that refresh themselves), clocks, storage, serial-blanking and game-patch status, the raw result of every parental-control query, and the logs: how the last run ended, the memory and SD-card space left, the newest crash files and the last 80 lines logged. **It never contains the PIN or the serial number.**
2. [Open an issue](https://github.com/JigSawFr/PlayGuard/issues/new) and attach it.

Or, with the console online, *About › Help & diagnostics › Send a report online* (Ⓧ on the diagnostic report in developer mode) sends it, with the latest crash reports (PlayGuard's last 3 in `logs/crashes/`, and Atmosphère's last 2 of a homebrew from the past week, in `sd:/atmosphere/crash_reports/`: another homebrew's crash can be one of them, the list shown before sending says which), the logs of this run and the one before (`logs/sessions/`; a long one: its start and its last 64 KiB) and the debug files (the play-timer block reference, the end of the play-timer recording while *Record the play timer* is on, the last 20 changes of the change history, without names (of games, accounts or profiles) and with their date only, and PlayGuard's settings), to a secret gist in your GitHub account (the report and each file as a file of its own; signed out, PlayGuard offers to sign in first; *GitHub account*, just below once signed in, signs out; when GitHub refuses the gist, PlayGuard offers to sign in again), after saying exactly what goes. It shows the link and two QR codes: the report, and the bug-report form with the link, your versions, how PlayGuard was started (over a game or from the Album; add the menu) and the system (sysMMC or emuMMC) already filled in. Anyone with the link can read the report, until you delete the gist on GitHub, and a bug report opened with it is public: the confirmation says so. Each link is kept in `logs/uploads/` as an Internet shortcut (`.url`) that a computer opens in its browser. A report saved earlier in `logs/reports/` can be sent the same way.

### What PlayGuard keeps on the SD card

Everything is in `sd:/config/playguard/logs/`, plain text a computer opens:

- `sessions/session_<date>.txt` — **one file per run**: everything PlayGuard logged (what used to reach a PC over nxlink only), each line with its time and level (`E` error, `W` warning, `I` information), and a last line saying how the run ended: `=== end: clean exit ===`, or `=== end: crash, see logs/crashes/… ===`. A file stops growing at 1 MiB.
- `crashes/crash_<date>.txt` — **one file per crash**, its first line saying what happened:
  - a **CPU exception** (a bad pointer, an undefined instruction, a stack overflow): the kind of fault and the address it touched, the backtrace and the registers. Addresses are offsets into `playguard.nro` (`playguard+0x1a2b3c`): the linker map of the build (the `-linker-map` artifact of its CI run) gives the function. PlayGuard then ends as before: the console's error screen, and Atmosphère's own report in `sd:/atmosphere/crash_reports/`;
  - an **uncaught exception**: its type and message, and the backtrace;
  - a **run that did not end**: found at the next start, when the last run's session file has no end line (a crash PlayGuard could not catch, closed from the HOME menu while it hung, or the console turned off), with that run's last 60 lines.

  The crash files caught while PlayGuard runs end with the last 80 lines logged before the crash.
- `reports/`, `uploads/`, `play_timer/` — the saved reports, the links of the reports sent online, the play-timer recording.

*About › Help & diagnostics* sets how many reports, crash files, session logs and links are kept (each folder on its own; the newest always stays).

## Developer mode (investigating a new firmware)

Press Ⓨ seven times on *About › Version*; the developer tools appear at the end of *About › Help & diagnostics*. It adds:

- a **read-only** switch — with it on, the app cannot change anything: the safe way to investigate a new firmware (the firmware screen offers it directly);
- the diagnostic report on screen (Ⓨ saves it, Ⓧ sends it online), and a shortcut to export it from the Play timer tab;
- **Install another build** in place, to test a fix before it is released: one of the last 20 releases (pre-releases included, tagged), one of the last 20 commits of `main`, or the newest build of an open pull request from a branch of this repository (never from a fork: its code has not been reviewed yet), each kind in a tab of its own (Ⓛ / Ⓡ, shown on either side of the tabs, go from one to the next; the screen opens on the installed build's, and each tab counts the builds newer than the installed one). Each build shows its title (a commit's or a pull request's subject; a commit's whole message is its description) over tags: the kind of change (*Feature*, *Fix*, *Docs*…) and its scope, *Breaking*, the pull request a commit came from, how its checks ended (*CI OK*, *CI failed*, *CI running*), pre-release, *installed* or *new*; how old it is and its commit at the right. A on the pull requests' tab, when it is shown, sorts them the other way (newest build first, or by number). A build opens its page first: tags (its kind, its checks, installed or new), a card of facts (built, size, commit, checksum, when GitHub deletes it), **Install**, **See on GitHub** (its page as a QR code for a phone), then a release's notes or a pull request's description, drawn from GitHub's Markdown (headings, lists, task boxes, code, quotes and alerts, tables). Everything that comes from GitHub is shown as plain text: no link is opened, no HTML or script is run, and invisible characters that could disguise a title (right-to-left overrides, zero-width characters, control characters, invalid UTF-8) are removed. Signed in, Ⓨ (*Search*) takes a pull request's number (open or closed) or a commit (7 to 40 hex digits) and finds its build, from a branch of this repository only. The releases need nothing; the others are the build workflow's artifacts, which GitHub hands to signed-in users only: **GitHub account** signs in with a code and a QR code to scan with a phone (the only permissions asked are to read the build workflow's files (Actions) and to create gists, for *Send a report online*; otherwise the token can only read what is public; it is kept in `github_token`, never sent with a report, and *GitHub account* signs out). PlayGuard downloads the build, checks it (size, the SHA-256 GitHub records, the NRO header), puts it in place of its own `.nro` and restarts on it (the PIN is asked first whenever one is set). The list is kept for 10 minutes and shown at once, with how old it is (Ⓧ fetches it again); while it is fetched, a bar shows how far along it is. A download cut short (network lost) goes on from where it stopped the next time the same build is installed. The build replaced is kept (`playguard.nro.previous`): **Go back to the previous build** in the same list swaps them again. If a build installed from here (or by *Update in place*) crashes before its first screen, the next start of PlayGuard offers to go back to the previous build on a plain text screen, before anything else is loaded (A goes back, B starts it anyway), without the PIN, since the PIN screen itself may be what crashes. At any other time, *Go back to the previous build* (above) does it, after the PIN. The same list goes back to any release at any time; *About › Version* shows the commit in developer mode. Every screen's header shows the running version next to the title: a release's in a grey tag (`1.2.0`), any other build's in an amber tag followed by its commit, so a test build is never taken for the release. Artifacts expire after 90 days;
- **Compare the play-timer block** (only while the phone app is linked and can sync — not with a hosts file blocking Nintendo; also in *PIN*), to decode settings PlayGuard does not show yet: save the raw block as a reference, change one setting in the phone app, come back — PlayGuard lists the values that changed (saved in `logs/reports/` on request). Different bedtimes per day, set from the phone app, could be confirmed this way. Attach that file to an issue, or send it with the report (*Send a report online*).
- **Record the play timer**: every 30 s while PlayGuard is open, one line of what the play timer reports (time left, time spent, the raw settings block…) in `logs/play_timer/log.csv`, a spreadsheet-ready file, plus a line right after each change PlayGuard makes, saying which (its last column, `event`), and a note when the clock moved more than the time that went by. Left open over midnight, or until the time is up, it shows what one report cannot: when the time spent resets, what the console says near the end. The switch is remembered; it only records in developer mode. What is known so far is in [parental-controls.md](parental-controls.md).
- **Clear the cache** and **Delete the log files**, each after a confirmation saying how many files and how much space: the cache (`cache/` and each system's `nand/<system>/cache/`: game names and icons, the Activity's last figures, the build list) is read again from the console when needed; the log files are the crash files, the session logs (but this run's, still being written), the saved reports and the play-timer recording, while `uploads/` (the removal links of the reports sent online) and the block reference stay. Settings, history, backups, profiles and exports are never touched.
- **Run the migrations again**, after a confirmation: the moves of the earlier layouts (the data folder next to the `.nro`, the logs before their subfolders, what every system shared) run again now, whatever `migrations.json` records (each one otherwise runs once, at the first start of a build that needs it); a file already in place is never replaced. A dialog says how many entries moved; each step is in the session log.
- **Frame statistics**: one line per second in `logs/frame_stats.txt`, the time the screen took to draw and the frames that came late, to weigh one screen against another on the console. **Background glow** turns off the soft light behind the screens (the line's mode then ends in `-noglow`), to measure what it costs.
