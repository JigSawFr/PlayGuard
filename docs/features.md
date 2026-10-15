# Features

What each tab does, how PlayGuard writes safely, and what the console counts as play time. Back to the [README](../README.md).

| Play timer | Per-day limits |
|---|---|
| ![Play timer](../images/screenshots/play_timer.png) | ![Per-day limits](../images/screenshots/per_day.png) |
| **Activity** | **A game** |
| ![Activity](../images/screenshots/activity.png) | ![A game](../images/screenshots/activity_game.png) |
| **Restrictions** | **Network clock** |
| ![Restrictions](../images/screenshots/restrictions.png) | ![Network clock](../images/screenshots/clock.png) |
| **Security & app** | **Preferences** |
| ![Security & app](../images/screenshots/security.png) | ![Preferences](../images/screenshots/preferences.png) |

## Controls

↑/↓ move · Ⓐ confirm · Ⓑ back or cancel (twice on the sidebar to exit) · Ⓧ refresh · Ⓨ notifications · **+** saves per-day limits.

What failed is said in a dialog, what worked in a toast. The title says when the app is read-only or while parental controls are temporarily unlocked; in read-only mode, actions stay in place, greyed, and say why when pressed.

The app is organised in tabs, like System Settings; each has its section below.

## Notifications — what needs you, from any tab

A bell in the header, left of the version, gathers what needs the parent; Ⓨ opens the list from any tab (the footer says *Notifications (2)* when two things are to do).

- **To do**, counted on the bell in amber, stays until it is fixed: no PIN yet, parental controls temporarily unlocked (*Lock now*), extra time from an earlier day still to put back, the "time's up" alarm off, changes made outside PlayGuard, the console not counting play time, an inaccurate network clock, game patches that need a look, installed games that cannot start (*Check the installed games*), one of PlayGuard's own files that could not be put aside or written to the SD card, a build that did not finish starting and was gone back from (*Send a report*).
- **For information**, a dot on the bell, can be marked as read: a new version installed (*What's new*; after an update in place, the previous build kept), a build gone back from, a report sent online (*Show the link*), a new user account, PlayGuard closed unexpectedly last time (*Send a report*), the companion app linked, a firmware not tested with this version, the serial number not blanked on emuMMC, a newer version of PlayGuard, the network clock found off at start-up, one of PlayGuard's own files found damaged and put aside (a toast at start-up points to it).
- Ⓐ on an entry does the fix (*Turn it on*, *Set it*, *Update*…) or opens the tab where it is made. An entry goes by itself once what it says is fixed.
- The state behind it is read when PlayGuard starts, every minute on the main screen, and every 5 s while the Overview is shown.

## Overview — today at a glance

- **Today first:** a gauge of today's play time (while no game is running, the time from the activity log, marked ≈), today's limit, time left, bedtime alarm. The "time's up" alarm shows in amber while it is off — Ⓐ turns it back on. Under the gauge, a reminder: a game left open on the HOME menu keeps counting (see [What counts as play time](#what-counts-as-play-time)).
- **Is the console counting?** PlayGuard's own time counts while it runs as an application, so with a limit today the time left should go down while it is open. When it has not moved for about 90 s on screen (timer on, a limit today, not unlocked, not opened from the album), an amber line says *The console is not counting play time right now*, with the likely causes: the clock was set back (it counts again later), the network clock was never set (*Network clock* tab), or a limit was just written (start a game for a minute, then check). It goes as soon as the time left moves again. The Play timer's state line says it too.
- **Changed outside PlayGuard:** today's play time starting over on the same day (a clock change does that), the console clock moved by more than 5 minutes, limits that differ from the ones PlayGuard last saw (the phone app, another tool). An amber notice until dismissed or the next day, and an entry in the change history. PlayGuard does not run in the background: it only sees what changed while it was open, or since it last was, when it reads the console again.
- **At the limit** the console shows "Time's up", then suspends the game (no "continue") or only warns: *When the time is up* shows which and changes it, once a limit or a bedtime is set ([details](parental-controls.md#when-the-time-runs-out)).
- **Then the state:** parental controls, PIN (*Set* or *Not set*, never its length), restriction level.
- **Then what needs attention:** network-clock accuracy, companion-app link (amber while linked), firmware / compatibility only when there is a problem, serial blanking and game patches warnings.
- Lines that open another tab end with a chevron (›). Ⓐ on *Today's limit* changes it in place; Ⓐ on *Network clock › Inaccurate* measures and sets the clock on the spot.
- **Extra time today** (+15 min, +30 min, +1 h by default) and **No more play today** (limit 0 for today only, *now* or *in 5 minutes* when the time played is known: the limit then becomes the time played + 5 min, time to save; at the lock the console's "Time's up" comes up, and unsaved progress may be lost — the confirmation says so). The next day — at start-up, or at midnight if the app is open — PlayGuard offers to put the usual limit back, or does it by itself (*Preferences › Put the usual limit back the next day by itself*).
- A **Lock now** banner while parental controls are temporarily unlocked.
- While no PIN is set, a **First steps** line opens the three-step guide, which also comes up at start-up.
- Refreshes every 5 s, with the time of the last refresh (Ⓧ refreshes now).

## Play timer — the week chart is the editor

- A state line: active, time left, today's limit, the matching profile (red while the limit is reached).
- **The week chart:** ←/→ pick a day, Ⓐ changes its limit.
- **Same limit every day:** a quick list or any value, typed in minutes (`90`, `90 min`) or hours (`1:30`, `1h30`).
- **A different limit for each day:** unsaved days in amber and marked \*, quick values, Monday–Friday / weekend presets, "no limit" per day; **+** saves from anywhere.
- **Remove the limit**, **extra time today**, **no more play today** (as on the Overview).
- A note under the daily limit says what the console does at the limit (as on the Overview). While parental controls are temporarily unlocked, play time *may* not be counted: that is still open ([docs](parental-controls.md#still-open)).
- **Profiles** saved on the SD card: apply, edit, rename or delete one; save the current limits or make a new one. Any name, in any script (*École*, *周末*, *Выходные* …) — a name without Latin letters gets a file name of its own, and two names that would map to the same file are caught.
- Every confirmation draws the week as it will be, with the days that change in amber.
- **Bedtime alarm**: the alarm time (16:00 to 23:45, or off) and when play is allowed again (05:00 to 09:00), the same every day. Its place in the play-timer settings, worked out from the companion app's settings, is verified on a console (22.0.0); PlayGuard still changes it only once the console reports what PlayGuard reads there, checks the console's answer after the change and puts the previous settings back if it differs. Advanced, opt-in: "time's up" alarm on/off, pause / resume the countdown.
- `0` minutes means *no play that day*; *Remove the play-time limit* turns the timer off.

## Activity — who played what, and how long

- Time per game **today**, in the **last 7 days** and **in all**, from the console's own activity log — for every account or **one user account** (*Account*, when the console has several).
- A chart of the last seven days (today on the right, a legend under it), with this week's limit for each day as a line and the time over it in amber, marked !; today's, the week's and all-time totals.
- A **summary** over the chosen period, for the account shown: average per day, the most played game, the days played and the busiest one (last 7 days), the average session (all time).
- **To rediscover**: installed games under 3 h in all and not played for a month or more, the least played first (up to three).
- Sort by period; the first games show their icon (not in applet mode, to spare memory). A large library lists its first 50 games, then *Show every game*, kept until another period or account (100 games at most in applet mode); the export always holds them all. PlayGuard reads 256 games at most (this week's first): beyond that, the list and the export (not CSV) say that some are missing, and the all-time total leaves them out.
- Ⓐ on a game: its last seven days as bars, launches, first and last play, time per user account. Deleted games keep their all-time figures, even one deleted before PlayGuard was installed as long as the console's play log still names it, and their name once PlayGuard has seen it (else *Deleted game* and its title ID).
- **Time on no account and new accounts:** a game's time that no user account holds (some games never ask for one, or the account was deleted since) shows as *No account*, from a minute up; a user account PlayGuard never saw before is flagged in amber under *Account* for 30 days (*New account since …*, a fresh account is a known way to start over) and recorded in the change history; the accounts there when PlayGuard first lists them are not flagged.
- **Export to the SD card** as CSV, JSON, XLSX (Excel) or PDF, one column per day.
- Opens at once on the last figures read (kept on the SD card between runs), refreshed in the background when over a minute old; Ⓧ reads them again now, with a spinner.
- Times are approximate if the console clock was changed.

## Restrictions — level, age rating, communication

- Restriction level: None, Young child, Child, Teen, Custom. What a preset restricts (age limit, posting, communication) is shown read-only, and said before you pick it.
- In Custom: age rating, social-media posting, communication with others.
- VR mode, **rating organisation** (PEGI, ESRB, USK, CERO …).
- The number of games allowed to communicate (the list itself is in the diagnostic report, raw, until its layout is known).

## Network clock — the clock the play timer relies on

- Console and network clocks, time zone, accuracy.
- Pick a public NTP server (≈ 50 built in, by region, or your own).
- **Measure** against 3 servers (median; an amber warning when they disagree), then **set the network clock** — a measurement stays usable for 2 minutes, with a countdown. The confirmation says it: changing the clock starts today's play time over, so the whole limit is available again.
- A console that never reaches Nintendo's servers keeps this clock inaccurate, which skews the play timer.
- **No server answers** (a console without Internet): *Set the network clock to the console clock* appears under the result (and on the Overview's *Inaccurate* line, after its measurement fails), behind the same confirmation. PlayGuard has nothing else to compare the console clock with: the confirmation shows its time and asks you to check it against a phone or a watch first, and a console clock before 2026 (never set) is refused. With *Synchronize Clock via Internet* off, the console clock stays where it is, and turning the setting on afterwards no longer makes it jump; with it on, the console clock already follows the network clock, so the confirmation says to turn it off and set the time by hand if it is wrong. It writes the clock only, not the parental controls, so it needs no unlock (the PIN of *Ask for the PIN* still applies); like any clock change, it starts today's play time over. Not tried on a console yet.

## Security & app — PIN, locks, companion app

- **Set / change the PIN** (system PIN screen), **show the PIN** (after a warning and the PIN itself, asked every time; recorded in the history, then a new PIN is offered), **unlock temporarily**, **lock now**.
- **Ask for the PIN** in PlayGuard itself: *Never*, *Before a change* (the default: anyone can look, only the parent changes something; asked again after 5 min) or *To open PlayGuard* (a lock screen at start, and again when PlayGuard comes back after 5 min or more out of focus). Checked in the service layer, so no change skips it; locking again never asks. *Show the PIN* asks for it every time, in every mode and even within the 5 minutes, and a missing or damaged `config.json` counts as *Before a change*. Quitting while parental controls are still unlocked asks whether to lock them again (*Lock and quit* also closes the game PlayGuard was opened over, back to the HOME menu rather than hbmenu); closed from the HOME menu instead, PlayGuard locks them again the next time it starts.
- **Console lock:** one switch that sets every day's limit to 0, so a PIN is needed to start a game — a light lock without age ratings or communication limits. It blocks starting games, not the HOME menu, and needs a PIN. The previous limits come back when it is turned off. While it is on, *extra time* and *no more play today* are refused, and limits set another way (a profile, a backup, the history…) replace it.
- **Companion app:** whether the Nintendo Switch Parental Controls app is linked, its last sync, and **unlink** (otherwise its next sync overwrites the limits set here: while it is linked, every change PlayGuard asks to confirm says so). While it is linked, *Before unlinking: help decode…* opens the block comparison (see [CONTRIBUTING.md](../CONTRIBUTING.md#no-code-needed)).
- **Delete all parental controls:** two confirmations, irreversible; a backup of the settings is saved first.

## Preferences

- Language (every console language, or the console's own) and theme (light / dark, with an offer to restart).
- The tab to start on.
- **Lock again automatically after a change** (on by default).
- Extra-time amounts (+15/+30/+1 h, +10/+20/+30 min …) and putting the usual limit back the next day by itself.
- **The intro** at start-up: the loading screen for 2 seconds (on by default; Ⓐ or Ⓑ skips it; off, PlayGuard opens as soon as it is ready).
- A network-clock check at start-up (a toast when it is more than a minute off; it never sets the clock).
- A **monthly reminder to support PlayGuard** (on by default, never in the first month nor right after an update; *Don't show again* on the reminder or this switch turns it off for good, updates included).
- Advanced actions.

## Tools and About — history, backups, console info; version, updates, what's new, credits

- **Change history:** what PlayGuard changed (limits, restriction level, PIN, unlocks, unlinking, the clock, restores …), when (*Today*, *Yesterday* or the date) and from where; a limit changed on one day names that day. Ⓐ on a change shows it and, for a value, **puts the previous one back** — through the same unlock and PIN as any change, saying if it changed since. A change recorded by a newer version of PlayGuard shows as *Other change*.
- **Back up / restore the settings** on the SD card: restriction level, custom settings, VR mode, rating organisation, daily limits, the "time's up" alarm (with the advanced actions on), and the raw play-timer block for the record — never the PIN. A restore lists only what would change (nothing to change: it just says so, with no unlock and nothing written), and says when the backup had a bedtime alarm on (it is not written back). Choose how many backups to keep.
- **First steps** opens the guide again (with an *Unlink the companion app* step while linked, a *Turn the "Time's up" alarm back on* step while it is off, a *Protect PlayGuard* step once a PIN is set — the *Ask for the PIN* choice —, and a switch to stop it coming up at start-up). Below *Close*, *Support PlayGuard* shows the funding QR codes.
- **Export a diagnostic report**, or **send one online** (see [Reporting a bug](troubleshooting.md#reporting-a-bug)); how many saved reports, crash files and links of reports sent online are kept, by number, age or size.
- **Installed games:** the games the HOME menu cannot start or draw — an icon that keeps loading, a "!", a dotted frame — with what is wrong (name and icon unreadable, the game itself missing behind its update, nothing installed any more, files missing, corrupt data the console found, a newer firmware needed by the game or only by its update, keys too new for the firmware, a missing license (ticket), the console asking for an update that was removed, the SD card unreadable, a launch the console refuses) and what to do, under the game's icon, with how many installed games have a problem. The games are in three tabs by way out (LB/RB): **Fix here** (what PlayGuard repairs or checks), **Reinstall** (files gone or unusable: remove, then install again) and **Firmware or SD card** (the game is fine, the console is not), each with its count, the games with the same problem together. When one action is the fix, it is the dialog's first button; the **other actions** follow: accept the installed version again (when the console asks for a removed update), remove only the update or an add-on, check the data for corruption (the console's own check, with its progress and Cancel), or remove the game. Versions are shown as the game shows them (*version 1.4.0*, *update #4*); after a repair, a toast says whether the game works again. Save data always stays; every change is in the change history. The last check shows at once; then only the games whose entry changed are checked again (every game once a day, after a firmware update, or with Ⓧ *Check again*), each shown as soon as it is checked. Several broken games at once points to the SD card or to a game installed from the other storage (sysMMC / emuMMC).
- **Console:** firmware, Atmosphère, compatibility, storage (emuMMC or sysMMC, and a note when the other system has its own parental controls), whether Atmosphère **blanks the serial number** (partly hidden until Ⓐ; a warning on emuMMC when it is not), **game patches** (sys-patch or sigpatch files, recommending sys-patch when only files are used).
- **About** (its own tab): version, launch mode and data folder; **updates** (check now or once a day at start-up; *Update with*: in PlayGuard, which downloads the release, checks it, replaces itself and restarts, or sphaira, Homebrew App Store or by hand; asked on the first update when none is chosen). While a newer release is known, PlayGuard opens on **PlayGuard X.Y.Z is available** (its date and notes): *Update*, or *Later* for 1 day, 1 week or 30 days at most; **what's new** in the running version (its highlights, in English, and *Full list of changes* for its entry of the bundled changelog); the credits, how to **support PlayGuard** ([GitHub Sponsors](https://github.com/sponsors/JigSawFr), [Ko-fi](https://ko-fi.com/jigsawfr), shown as QR codes to scan with a phone), and a small *Made in France* 🇫🇷. After an update, PlayGuard opens once on **What's new in X.Y.Z** (the same highlights, then the QR codes).

## Safety by design

**Writing the play-time limit safely.** Overwriting the timer's configuration while it counts down destabilises Atmosphère. So PlayGuard checks the state first; if the timer is active, the confirmation says parental controls will be unlocked temporarily — with the stored PIN, **you don't need to remember it**. One press then unlocks, checks that the system really reports the unlock, writes, and **locks again right away** (or offers to, if *Lock again automatically after a change* is off). The service layer re-checks the state just before writing, so no screen can skip the safeguard.

> [!CAUTION]
> A limit below the time already played today suspends the game as soon as parental controls are locked again. The confirmation says so when that would happen.

**No more 22.5 crashes.** `pctl:a`, the privileged parental-control service, accepts a **single session**. Older builds of the original app kept it open, so the HOME-menu PIN prompt (or the PIN applet) could not get it and Atmosphère could crash. PlayGuard opens the session for each action and releases it immediately; periodic refreshes pause while the app is in the background. *(Diagnosis by [anbingxi's fork](https://github.com/anbingxi/NX-Pctl-Manager/tree/diag/fw22-5-readonly).)*

**Nothing runs in the background.** The limit, the PIN, the warnings and the suspension are the console's own; PlayGuard only changes their settings.

## What counts as play time

The play timer is the console's: PlayGuard reads it, it does not count anything itself. What is known, and how sure it is ([parental-controls.md](parental-controls.md#time-spent-and-time-left) has the measurements):

- A game running — Yes *(measured (22.0.0))*
- **A game left open on the HOME menu** (suspended in the background) — **Yes**: close it, or put the console in sleep mode *(reported by parents ([Arqade](https://gaming.stackexchange.com/questions/398954/)); fits the measurements (the console counts while an application is open))*
- Sleep mode — No *(reported ([Arqade](https://gaming.stackexchange.com/questions/398954/)); not measured by PlayGuard)*
- **PlayGuard opened over a game** (as an application) — **Yes**, as that game *(measured (22.0.0): open it with a parent's user, or from the album for a quick look)*
- PlayGuard opened from the album (applet mode) — No *(inferred: it is not an application, and time outside applications did not seem to count)*
- While parental controls are temporarily unlocked — Maybe not *(not measured: still open)*
- After the clock was changed — The day starts over; after a jump back the count stops until the clock passes the old time *(observed (22.0.0); the Overview says when the console is not counting)*
