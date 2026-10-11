# `config.json` and `state.json`

PlayGuard's settings, in `sd:/config/playguard/config.json`, shared by every
system on the SD card; and what one system's parental controls need
remembered (the console lock, the extra time, a pending relock, the firmware
choice), in its own `sd:/config/playguard/nand/<system>/state.json`
([below](#state-kept-between-launches)). Everything here is
set from the app; the file is only worth editing by hand to get back in (see
[`pin_lock`](#pin_lock)) or to check what PlayGuard remembered.

The code is the reference: `source/util/config.hpp` (fields and defaults) and
`source/util/config.cpp` (reading, checks, writing).

## How the file is read

- **A missing file gives the defaults**, silently (a first run). PlayGuard
  writes the whole file at the next change.
- **A file that is there but cannot be read, or is not a JSON object, is kept
  as `config.json.bad`** before anything is saved (PlayGuard saves by itself
  at start-up), and the defaults are used. A `config.json.bad` already there
  becomes `config.json.bad.1` (an older `.bad.1` is deleted): the two most
  recent damaged files are kept. PlayGuard says so once, at start-up (*the
  settings could not be read and were reset*), and the change history
  records *Settings reset*. What the old file held (the console lock's saved
  limits, a pending relock or extra time, the firmware choice …) is not
  carried over: check those settings, or copy values back from the `.bad`
  file by hand. Should the file not be movable either, nothing is saved
  until the next start, so it is never overwritten.
- **Each key is read on its own.** A missing key keeps its default; a key of
  the wrong type keeps its default too, and the others are still read.
  Types are strict: `true`, not `1`; `90`, not `90.5` or `"90"`.
- **A value outside the list below goes back to its default**, without a
  message.
- Keys PlayGuard does not know are ignored, and dropped at the next save.
- Some groups of keys are all-or-nothing: the [firmware choice](#firmware-not-supported-yet)
  and the [extra time of the day](#extra-time-today). One key refused, and the
  whole group is cleared.
- The file is written atomically (a temporary file, then a rename), indented
  with two spaces.

## Preferences

| Key | Type | Values | Default | Set in |
|---|---|---|---|---|
| `schema` | integer | `1` | `1` | written by PlayGuard, not read |
| `language` | string | `"system"` (the console's language), `"en-US"`, `"fr"`, `"fr-CA"`, `"de"`, `"es"`, `"es-419"`, `"it"`, `"nl"`, `"pt"`, `"pt-BR"`, `"ru"`, `"ja"`, `"ko"`, `"zh-Hans"`, `"zh-Hant"` | `"system"` | Preferences |
| `theme` | string | `"system"`, `"light"`, `"dark"` | `"system"` | Preferences |
| `start_tab` | string | the tab shown at start-up: `"dashboard"` (Overview), `"play_timer"`, `"activity"`, `"restrictions"`, `"clock"` (Network clock), `"security"`, `"preferences"`, `"tools"`, `"about"` | `"dashboard"` | Preferences |
| `advanced` | boolean | shows the play timer's debug-class actions | `false` | Preferences |
| `auto_relock` | boolean | locks parental controls again right after a change that needed the temporary unlock | `true` | Preferences |
| `onboarding_at_start` | boolean | *First steps* opens by itself at start-up while no PIN is set | `true` | First steps (*Show at start-up*) |
| `clock_check_at_start` | boolean | measures the network clock at start-up and says when it is off | `false` | Preferences |
| `intro` | boolean | the loading screen stays 2 seconds at start-up (Ⓐ or Ⓑ skips it); off, the first screen comes as soon as it is ready | `true` | Preferences (*Show the intro for 2 seconds*) |
| `activity_period` | integer | Activity's period: `0` today, `1` last 7 days, `2` all time | `1` | Activity |
| `export_format` | integer | the last export format: `0` CSV, `1` JSON, `2` XLSX, `3` PDF | `0` | Activity › Export |
| `backup_keep` | integer | backups kept after a new one: `0` (all of them), `5`, `10`, `20` | `0` | Tools |
| `logs_keep` | integer | saved reports kept, crash files kept and links of reports sent online kept (`logs/reports/`, `logs/crashes/`, `logs/uploads/`, each on its own): the newest `5`, `10`, `20` or `50`; `0` all of them. A file goes when any of the three `logs_keep*` says so, at start-up and when one changes; the newest file of each folder always stays | `20` | Tools › Diagnostics |
| `logs_keep_days` | integer | the same, by age: those of the last `7`, `30`, `90` or `365` days (from the date in the file name); `0` no limit | `0` | Tools › Diagnostics |
| `logs_keep_mb` | integer | the same, by size: the newest that fit in `1`, `5` or `20` MB together; `0` no limit | `0` | Tools › Diagnostics |
| `extra_amounts` | array of 3 integers | the *Extra time today* choices, in minutes: `[15, 30, 60]`, `[10, 20, 30]`, `[30, 60, 90]` or `[5, 10, 15]` | `[15, 30, 60]` | Preferences |

## `pin_lock`

*Security › Ask for the PIN*. A string:

| Value | In the app | What it does |
|---|---|---|
| `"off"` | Never | Nothing is asked, except before *Show the PIN* (the PIN opens everything else). |
| `"changes"` | Before a change | Anyone can look; the first change asks for the PIN, then nothing is asked for 5 minutes. The default. |
| `"open"` | To open PlayGuard | A lock screen first; the right PIN opens the app, B quits. It comes back when PlayGuard returns after 5 minutes or more out of focus. |

- With no PIN on the console, nothing is asked whatever the value.
- *Show the PIN* asks for it every time, whatever the value, even within the
  5 minutes.
- A missing file, a missing key or a value not in the list gives `"changes"`:
  a damaged file never turns the prompt off. Only `"off"` written on purpose
  does.
- In the app, a lower setting asks for the PIN first. The file itself is not
  protected: it keeps a child out of PlayGuard, not someone who edits the SD
  card.
- **PIN forgotten while this is `"changes"` or `"open"`:** put the SD card in a
  computer and set `"pin_lock": "off"`. *Show the PIN* still asks for it, and
  the recovery sysmodule's screen sets a new one rather than showing it.

## Network clock

| Key | Type | Values | Default |
|---|---|---|---|
| `ntp_server` | string | the NTP server used; empty picks one from the console's region. At most 253 characters, or it is cleared | `""` |
| `custom_servers` | array of strings | servers added by hand: at most 10, each 1 to 253 characters (empty or longer ones are dropped) | `[]` |

## Updates

Set in *About*.

| Key | Type | Values | Default |
|---|---|---|---|
| `update_via` | string | how an update is installed: `"auto"` (Sphaira if installed, else the Homebrew App Store), `"sphaira"`, `"appstore"`, `"manual"` (only say an update exists) | `"auto"` |
| `update_daily` | boolean | checks for an update at start-up, once a day | `false` |
| `update_checked` | string | `"YYYY-MM-DD"` of the last check; anything else is cleared | `""` |

## Reminders

| Key | Type | Values | Default |
|---|---|---|---|
| `support_reminder` | boolean | *Support PlayGuard* once a month at start-up (Preferences) | `true` |
| `support_reminded` | string | `"YYYY-MM-DD"` of the last reminder; anything else is cleared | `""` |
| `seen_version` | string | the version whose *What's new* was shown; at most 32 characters | `""` |

## Developer tools

| Key | Type | Values | Default |
|---|---|---|---|
| `dev_mode` | boolean | the developer tools (seven presses on *About › Version*) | `false` |
| `pt_log` | boolean | *Developer tools › Record the play timer* (`logs/play_timer/log.csv`) | `false` |
| `dev_builds_order` | string | how *Developer tools › Install another build* orders the pull requests (Ⓨ there): `"newest"` (the newest build first) or `"number"` (the highest number first); main's commits are always newest first | `"newest"` |

## State kept between launches

Written by PlayGuard to finish what it started. Better left alone: a wrong
value here can make PlayGuard put back a limit nobody asked for.

These keys are in `state.json`, one per system: `nand/sysmmc/` for the
sysMMC, `nand/emummc/` for the emuMMC (every emuMMC of the SD card shares
it), `nand/unknown/` when the system could not be told
(`source/util/nand.hpp`). Each system has its own parental
controls: the console lock turned on on the sysMMC must not put its limits
back on an emuMMC. `state.json` is read like `config.json` (a damaged one is
kept as `state.json.bad`, with the same notice); when a system has none yet,
these keys are read from `config.json`, where the versions before it kept
them, and leave it at the next save. `extra_auto_restore`, a preference,
stays in `config.json`.

### Console lock

*Security › Console lock*: every day's limit set to 0, so the PIN is needed to
play.

| Key | Type | Values | Default |
|---|---|---|---|
| `console_lock` | boolean | the console lock is on | `false` |
| `console_lock_prev` | array of 7 integers | the limits it replaced, Sunday to Saturday, in minutes: `0` to `1440`, or `65535` for no limit. Anything but seven valid values empties it; turning the lock off then clears the limits instead of putting them back | `[]` |

### Extra time today

*Extra time today* or *No more play today*: one day's limit changed for that
day only. All four keys go together.

| Key | Type | Values | Default |
|---|---|---|---|
| `extra_weekday` | integer | the day changed, `0` (Sunday) to `6`; `-1`: nothing pending | `-1` |
| `extra_date` | string | `"YYYY-MM-DD"` of the change | `""` |
| `extra_base` | integer | the limit before, in minutes: `0` to `1440`, or `65535` for no limit | `0` |
| `extra_value` | integer | the limit for that day, `0` to `1440` minutes | `0` |
| `extra_auto_restore` | boolean | the next day, put the limit back by itself instead of asking (the temporary unlock is still asked when needed); set in Preferences | `false` |

### Firmware not supported yet

The choice made on that screen, remembered for one firmware with one app
version. The three keys go together.

| Key | Type | Values | Default |
|---|---|---|---|
| `fw_gate_fw` | string | the firmware it applies to | `""` |
| `fw_gate_app` | string | the PlayGuard version it applies to | `""` |
| `fw_gate_choice` | string | `""` (none), `"read_only"`, `"probe"`, `"risk"` | `""` |

### Locking again

| Key | Type | Values | Default |
|---|---|---|---|
| `relock_pending` | boolean | set right before PlayGuard unlocks parental controls (for a change, or *Security › Unlock temporarily*), cleared once they are locked again (or *Quit, still unlocked* is chosen on quitting). Still `true` at start-up: the app stopped in between (or was closed from the HOME menu), so it locks again then | `false` |

## Example

Every key with *Ask for the PIN* set to *To open PlayGuard*, in the order of
this page: `config.json`, then a system's `state.json`. PlayGuard itself
writes the keys in alphabetical order, with each array element on its own
line; the order does not matter when it reads them:

```json
{
  "schema": 1,
  "language": "fr",
  "theme": "system",
  "start_tab": "dashboard",
  "advanced": false,
  "auto_relock": true,
  "onboarding_at_start": true,
  "clock_check_at_start": false,
  "intro": true,
  "activity_period": 1,
  "export_format": 0,
  "backup_keep": 0,
  "logs_keep": 20,
  "logs_keep_days": 0,
  "logs_keep_mb": 0,
  "extra_amounts": [15, 30, 60],
  "pin_lock": "open",
  "ntp_server": "",
  "custom_servers": [],
  "update_via": "auto",
  "update_daily": false,
  "update_checked": "",
  "support_reminder": true,
  "support_reminded": "",
  "seen_version": "",
  "dev_mode": false,
  "pt_log": false,
  "dev_builds_order": "newest",
  "extra_auto_restore": false
}
```

```json
{
  "schema": 1,
  "console_lock": false,
  "console_lock_prev": [],
  "extra_weekday": -1,
  "extra_date": "",
  "extra_base": 0,
  "extra_value": 0,
  "fw_gate_fw": "",
  "fw_gate_app": "",
  "fw_gate_choice": "",
  "relock_pending": false
}
```

A file with only `{"pin_lock": "off"}` is valid too: every other key keeps its
default.

## `watch.json`

One per system, next to its `state.json`, in
`sd:/config/playguard/nand/<system>/watch.json`: what PlayGuard
last saw of the console, to tell the parent what changed outside it (the
Overview's amber notice, the `outside_*` and `new_account` entries of the
change history, the Activity tab's new-account flag).
The code: `source/action/outside_change_logic.cpp` (the rules),
`source/action/outside_watch.cpp` (the readings).

**Only what PlayGuard sees.** Nothing runs in the background: the file is
compared with the console at each refresh of the Overview or the Play timer
(every 5 s while one is on screen) and at the next start. A change made and
undone while PlayGuard was closed, or a clock changed and the day over before
PlayGuard opens again, goes unseen.

| Key | Type | Values |
|---|---|---|
| `schema` | integer | `1` |
| `date` | string | `"YYYY-MM-DD"`, the day `spent_s` belongs to |
| `spent_s` | integer | today's time spent, last seen (today's limit minus the time left), `0` to `86400` seconds. Written at most every 5 minutes of play, so it can be a little behind |
| `offset_s` | integer | the user clock minus the steady clock (which neither a clock change nor sleep mode moves), in seconds |
| `steady_id` | string | the steady clock's source; another one is not compared |
| `limits` | array of 7 integers | the limits PlayGuard last saw, Sunday to Saturday, in minutes (`0` to `1440`, or `65535` for no limit) |
| `notice` | integer | what the Overview says: `1` the play time started over, `2` the clock was changed, `4` the limits were changed outside PlayGuard (added up) |
| `notice_date` | string | `"YYYY-MM-DD"` the notice belongs to: it goes the next day, or when dismissed |
| `reset_at` | string | `"HH:MM"` the play time was seen starting over |
| `accounts` | array of objects | the user accounts PlayGuard has seen, oldest first (32 at most): `id` (the account's 32 hex digits), `since` (`"YYYY-MM-DD"` first seen), `new` (`true`: it appeared after the first listing) |

- **The play time started over:** `spent_s` went down by more than a minute
  on the same `date`. The console's own reset comes with a new day; when it
  happens exactly (midnight or the "allowed again" time) is not confirmed
  yet ([parental-controls.md](parental-controls.md#time-spent-and-time-left)),
  so a reset at the "allowed again" time could show here.
- **The clock was changed:** `offset_s` moved by more than 5 minutes. The
  automatic correction moves it by seconds only.
- **The limits were changed outside PlayGuard:** they differ from `limits`.
- **A new user account:** the console lists one that is not in `accounts`
  (read once per start). It goes in the change history (`new_account`) and is
  flagged under the Activity tab's account filter for 30 days after `since`,
  not on the Overview. Without `accounts` (a first run, or a file from an
  older version) the accounts listed are only recorded.
- Every change PlayGuard makes itself (anything it records in the change
  history) makes it take what it reads next as is, so its own limit and clock
  changes are not reported.
- A missing or damaged file, or a key out of range, is a first reading:
  nothing is reported, the values are only recorded. Deleting the file is
  harmless.
