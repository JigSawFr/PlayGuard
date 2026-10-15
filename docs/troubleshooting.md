# Troubleshooting

Getting back into a locked console, reporting a bug, and the developer mode. Back to the [README](../README.md).

## Locked out?

PlayGuard only sees the parental controls of the system it runs on: **emuMMC and sysMMC each have their own** (a PIN removed on one is still there on the other). Run it on each one that needs fixing.

### Second-hand console

You know the PIN, but the previous owner's phone app is still linked (unlinking fails, or a factory reset asks for their account).

*Security & app › Unlink the companion app*, then, if you want no parental controls at all, *Delete all parental controls*. Both work offline, on emuMMC as on sysMMC.

### PIN forgotten

*Show the PIN* asks for the PIN itself, so it cannot help here. With the recovery sysmodule installed (below), drop a `RESCUE` file and restart: the recovery screen then sets a new one (it never shows the old one). Without it, *Delete all parental controls* starts again (a settings backup is saved first; it never contains the PIN): with *Ask for the PIN* on (the default, *Before a change*), put the SD card in a computer and set `"pin_lock"` to `"off"` in `sd:/config/playguard/config.json` first.

### The play timer blocks everything (0-minute limit) and the PIN is forgotten

PlayGuard itself cannot start then. Install the optional recovery sysmodule (`playguard-rescue.zip`) **beforehand**; when locked out, drop an empty `config/playguard/RESCUE` file on the SD card and boot — it unlocks the console so PlayGuard can open. See [`sysmodule/README.md`](../sysmodule/README.md).

### Console not modded

PlayGuard cannot help: it needs Atmosphère. Nintendo support's master-key procedure is the official way.

## Reporting a bug

An error shows the system's code (such as `0x0001088E`) with what is known about it: what to do for the codes whose meaning is documented, otherwise a report helps.

1. *Tools › Export a diagnostic report* saves a text file in `sd:/config/playguard/logs/reports/`: firmware, Atmosphère version, how long the start-up took, clocks, storage, serial-blanking and game-patch status, and the raw result of every parental-control query. **It never contains the PIN or the serial number.**
2. [Open an issue](https://github.com/JigSawFr/PlayGuard/issues/new) and attach it.

Or, with the console online, *Tools › Send a report online* (Ⓧ on the diagnostic report in developer mode) sends it, with the latest crash reports (PlayGuard's last 3 in `logs/crashes/`, and Atmosphère's last 2 of a homebrew from the past week, in `sd:/atmosphere/crash_reports/`: another homebrew's crash can be one of them, the list shown before sending says which) and the debug files (the play-timer block reference, the end of the play-timer recording, the change history and PlayGuard's settings), to a secret gist in your GitHub account (the report and each file as a file of its own; signed out, PlayGuard offers to sign in first, as *GitHub account* in the developer tools does), after saying exactly what goes. It shows the link and two QR codes: the report, and the bug-report form with the link, your versions, how PlayGuard was started (over a game or from the Album; add the menu) and the system (sysMMC or emuMMC) already filled in. Anyone with the link can read the report, until you delete the gist on GitHub. Each link is kept in `logs/uploads/` as an Internet shortcut (`.url`) that a computer opens in its browser. A report saved earlier in `logs/reports/` can be sent the same way.

## Developer mode (investigating a new firmware)

Press *About › Version* seven times; the developer tools appear at the end of *Tools*. It adds:

- a **read-only** switch — with it on, the app cannot change anything: the safe way to investigate a new firmware (the firmware screen offers it directly);
- the diagnostic report on screen (Ⓨ saves it, Ⓧ sends it online), and a shortcut to export it from the Play timer tab;
- **Install another build** in place, to test a fix before it is released: the latest release, one of the last 20 commits of `main`, or the newest build of an open pull request from a branch of this repository (never from a fork: its code has not been reviewed yet), each kind in a tab of its own (Ⓛ / Ⓡ go from one to the next; the screen opens on the installed build's), the installed one marked; Ⓨ sorts the pull requests (newest build first, or by number). The release needs nothing; the others are the build workflow's artifacts, which GitHub hands to signed-in users only: **GitHub account** signs in with a code and a QR code to scan with a phone (the only permissions asked are to read the build workflow's files (Actions) and to create gists, for *Send a report online*; otherwise the token can only read what is public; it is kept in `github_token`, never sent with a report, and *GitHub account* signs out). PlayGuard downloads the build, checks it (size, the SHA-256 GitHub records, the NRO header), puts it in place of its own `.nro` and restarts on it (the PIN is asked first whenever one is set). The list is kept for 10 minutes and shown at once, with how old it is (Ⓧ fetches it again). The same list goes back to the release at any time; *About › Version* shows the commit in developer mode. Every screen's header shows the running version at the right: a release as a plain grey `v1.2.0`, any other build with an amber **DEV** tag, its version and its commit, so a test build is never taken for the release. Artifacts expire after 90 days;
- **Compare the play-timer block** (also in *Security & app* while the phone app is linked), to decode settings PlayGuard does not show yet: save the raw block as a reference, change one setting in the phone app, come back — PlayGuard lists the values that changed (saved in `logs/reports/` on request). Different bedtimes per day, set from the phone app, could be confirmed this way. Attach that file to an issue, or send it with the report (*Send a report online*).
- **Record the play timer**: every 30 s while PlayGuard is open, one line of what the play timer reports (time left, time spent, the raw settings block…) in `logs/play_timer/log.csv`, a spreadsheet-ready file, plus a line right after each change PlayGuard makes, saying which (its last column, `event`), and a note when the clock moved more than the time that went by. Left open over midnight, or until the time is up, it shows what one report cannot: when the time spent resets, what the console says near the end. The switch is remembered; it only records in developer mode. What is known so far is in [parental-controls.md](parental-controls.md).
- **Clear the cache** and **Delete the log files**, each after a confirmation saying how many files and how much space: the cache (`cache/` and each system's `nand/<system>/cache/`: game names and icons, the Activity's last figures, the build list) is read again from the console when needed; the log files are the crash files, the saved reports and the play-timer recording, while `uploads/` (the removal links of the reports sent online) and the block reference stay. Settings, history, backups, profiles and exports are never touched.
