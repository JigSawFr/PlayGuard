# PlayGuard and the companion app

PlayGuard brings the settings of Nintendo's companion app,
**Nintendo Switch Parental Controls**
([App Store](https://apps.apple.com/app/id1190074407),
[Google Play](https://play.google.com/store/apps/details?id=com.nintendo.znma)),
onto the console itself. A console running Atmosphère is usually kept away
from Nintendo's servers, so the companion app can no longer reach it: every
setting it offers should be reachable from PlayGuard, offline.

This page lists what the companion app does (version 2.6.0, September 2026,
from its store page and release notes), what PlayGuard already covers, and
what is still missing. It is the to-do list for parity: when a gap closes,
move it to the first table.

How the settings are stored on the console is in
[parental-controls.md](parental-controls.md).

## Already covered

| Companion app | PlayGuard | Notes |
|---|---|---|
| Daily play-time limit, the same every day or one per day | *Play timer* | Plus saved profiles, and the week chart as the editor |
| "Extend today's play time" (+5 min … +1 h, added in 2.4.0 for +1 h); undo today's changes | *Extra time today*, *No more play today*; the usual limit comes back the next day | Amounts in *Play timer › Extra time*, including +5/+10/+15 |
| Alarm only, or suspend the software, when the time is up | *Play timer › When the time is up* | Shown once a limit or a bedtime is set (see gap 1) |
| Bedtime alarm and the time play is allowed again (2.1.0) | *Play timer › Bedtime alarm* | Every day, or by day of the week; the time play is allowed again, one for every day (see gap 2) |
| Restriction level, age rating, rating organisation, posting to social media, communicating with other players, VR mode (1.10.0) | *Restrictions* | |
| Play history per game | *Activity* | Plus per user account, all-time totals and export (CSV, JSON, XLSX, PDF) |
| Parental-controls PIN | *PIN* | Plus showing the PIN (asked first) and a PIN to open PlayGuard itself; for a forgotten PIN, the recovery sysmodule |
| Linking the app to a console | *PIN › Unlink the companion app* | PlayGuard can only unlink, which is what a console kept offline needs |

## Missing

Most useful first. Each says what blocks it.

### 1. Alarm only, or suspend the software

**Covered** by *Play timer › When the time is up*; left here so the numbers
of the gaps below stay the same.

- In the app's API it is `restrictionMode`: `ALARM` or `FORCED_TERMINATION`.
- On the console it is byte `01` of the play-timer block's header (`0`
  `ALARM`, `1` `FORCED_TERMINATION`), as reversed from the 22.5.0 `pctl`
  sysmodule (see [the header](parental-controls.md#the-header-is-the-modes-plus-a-daily-rule)).
- Verified on a console (2026-10-14): set to a warning only by PlayGuard, the
  console warned at the limit and the game went on, the time over it
  counted (1454 below 0).

### 2. Bedtime and start time per day of the week

Since 2.4.0 the companion app sets a different start time for each day
(*Detailed end-of-day settings*), and each day's rule has its own bedtime.
PlayGuard sets the bedtime alarm for every day or by day of the week
(Monday to Friday, the weekend, or one day); the time play is allowed
again, one for every day.

- The per-day bedtime bytes are **verified** on 22.0.0 (2026-10-14: a
  bedtime set by PlayGuard, read back from the block and from 1954–1959);
  PlayGuard still checks every bedtime write against what the console
  reports and reverts it otherwise.
- **Needs:** a console check: a block with different bedtimes on two days
  (set by PlayGuard, or by the companion app) would confirm that the console
  rings each day's own; and code for a start time per day.

### 3. Exceptions for specific software

The companion app can let a given game through the restriction level (for
example, allow communication in one game only). PlayGuard shows how many games
are on the free-communication list (command 1039) but not which ones.

- The list's entry layout is only partly known (an application ID, then a
  `01` byte whose meaning is open); the raw list is in the diagnostic report.
- **Needs:** a report with a known list (one or two games added from the
  app), to decode the entries. Then show them by name, then add and remove.

### 4. History of PIN entries

Since 2.5.0 the companion app shows when the PIN was entered on the console,
and can notify the parent.

- **Unknown** whether the console logs this in a place PlayGuard can read: the
  play-event log (`pdm:qry`, already read for *Activity*) or the
  parental-control service itself.
- **Needs:** a play-event log dump from a console where the PIN was entered,
  looking for an event at that time.
- What PlayGuard already has: its own change history (*Preferences › Change
  history*) records its temporary unlocks, not those made from the HOME menu.

### 5. Monthly report

The companion app sends a monthly activity report.

- Nothing blocks it: *Activity* already reads the whole play-event log. It
  needs a month view (per game, per account, per day) and the same export.

## Out of scope

| Companion app | Why not |
|---|---|
| Push notifications, remote control, a second parent, up to eight consoles | PlayGuard runs on the console, offline, and nothing of it runs in the background |
| GameChat settings | Nintendo Switch 2 only, which Atmosphère does not run on |
| Purchase restrictions | Nintendo Account settings, on Nintendo's servers, not on the console |

## Asked for in the store reviews

Not in the companion app either; noted here because parents ask for them.

| Request | Status |
|---|---|
| A limit per user account | Not possible with the console's play timer: one limit for the whole console. A replacement sysmodule does it ([compared](compatibility.md#playguard-or-a-replacement-sysmodule)) |
| A warning a few minutes before the end, on the console | The console's own warnings; PlayGuard does not change them |
| A start time before which the console cannot be used | Covered by the bedtime's "allowed again" time; per day is gap 2 |
| Settings changed while the console is off, applied later | Not applicable: PlayGuard changes the console directly |
