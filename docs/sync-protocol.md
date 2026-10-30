# Remote link protocol

**Status: schema 1, implemented: PlayGuard's side, the agent sysmodule and
its IPC service.** The contract between the console (PlayGuard and its
optional agent), the MQTT broker, Home Assistant and the `playguard` HA
integration. The design and its reasons are in
[`sync-design.md`](sync-design.md); the user guide is
[`home-assistant.md`](home-assistant.md). The documents below are what
`source/sync/` writes (its host tests save examples that
`tools/check_sync_json.py` validates); a change that breaks a consumer bumps
`schema`.

Conventions: `<id>` is the console id (8 lowercase hex characters, random,
generated once, never the serial number). `sync.conf` is shared by every
system on the SD card, and a sysMMC and an emuMMC each have their own parental
controls: on an emuMMC, `<id>` is the console id with its first digit's top
bit flipped (`5a0c3e11` becomes `da0c3e11`), so the two are two devices, each
with its own orders, and its name gets ` (emuMMC)`. Times are POSIX seconds from the
console's user clock. Minutes are integers. A value the console could not read
is `null`, never a default. JSON consumers ignore unknown fields.

## Topics

Every Boréale homebrew shares one tree per device: `boreale/<id>/` holds
what belongs to the device (`availability`, `capabilities`, `system`,
`icon/…`, `integration`), and each homebrew publishes under its own module
name, `playguard/` here. `boreale` is `topic_prefix`'s default; the same
tree goes under another prefix.

| Topic | Retained | Direction | Content |
|---|---|---|---|
| `boreale/<id>/availability` | yes | console → | `online` or `offline`. Last will (`offline`), and `offline` published before a clean disconnect. |
| `boreale/<id>/playguard/state` | yes | console → | The state snapshot (below). |
| `boreale/<id>/system` | yes | console → | The console itself: versions, model, name, storage, power, network (below). With the state, every `poll_s`. |
| `boreale/<id>/playguard/games` | yes | console (app) → | The games installed and played, each with its play time in all, and the homebrews as counts only (below), published by PlayGuard after a read of the play log. |
| `boreale/<id>/icon/<APP_ID>` | yes | console (app; agent: the game played) → | A game's icon: the raw JPEG of its control data, for every game installed or played (below). An empty retained message clears it. |
| `boreale/<id>/playguard/activity` | yes | console → | Today's play time from the play log: each game's, the homebrews' as one total. |
| `boreale/<id>/playguard/activity/<YYYY-MM-DD>` | yes | console → | A finished day's totals, published at the first cycle after local midnight. The console keeps the last 14 and clears older ones (empty retained publish). |
| `boreale/<id>/playguard/names` | yes | console (app) → | Game and account names (games only), published by PlayGuard when it runs. |
| `boreale/<id>/playguard/week` | yes | console (app) → | The 7-day table per game, for the homebrews together and per account, published by PlayGuard when it runs. |
| `boreale/<id>/playguard/<entity>/set` | yes (by the publisher) | → console | An order. Cleared by the console once handled. |
| `boreale/<id>/playguard/event` | **no** | console → | One event per order handled: `command_applied`, `command_rejected`, `command_waiting` (an order waiting for PlayGuard or the parent); and what the console does on its own (the agent): `play_started`, `play_stopped`, `time_warning`, `play_blocked` |
| `boreale/<id>/playguard/report` | yes | console → | The last diagnostic report, plain text, at most 128 KiB. Only when `publish_report` is on. |
| `homeassistant/device/boreale_<id>/config` | yes | console → | HA native discovery. Only when `ha_discovery` is on; cleared when it is turned off. |
| `homeassistant/status` | — | ← HA | Subscribed: on `online`, the console republishes discovery, state and activity. |
| `boreale/<id>/integration` | yes | ← the HA integration | Which version of the integration (and of Home Assistant) runs (below), at each set-up of the integration's entry; cleared when the entry is removed. Never cleared by the console. |

MQTT version: **5.0, or 3.1.1** (`mqtt_version` in `sync.conf`: `auto`, the
default, connects in 5.0 and, when the broker refuses it, answering in 3.1.1
or closing the connection, in 3.1.1 at once, then keeps the version that
worked until the settings change; `5` or `3.1.1` force one). In 5.0 the
console sends no property but its maximum packet size (4096 bytes, its
receive buffer: the broker drops a larger message rather than sending it),
and follows what the broker's CONNACK says: its keep-alive, its maximum
packet size (a document larger than it is not published, and counted), and
retained messages unavailable (the link needs them: it does not connect).
A broker's reason codes and reason strings, in a refused CONNECT or
SUBSCRIBE or a DISCONNECT it sends, are what *Remote access › Status* shows.
Orders arrive the same way in both versions; topic aliases are never used.

Client ids: `pg-<id>-agent` for the agent, `pg-<id>-app` for PlayGuard
running without the agent (23 characters at most, as MQTT 3.1.1 promises). Only one of them connects at a time.
Beside it, PlayGuard publishes the game icons over a short connection of its
own, `pg-<id>-icons`, whichever of the two holds the link (`icon/<APP_ID>`
below): no last will, nothing subscribed, closed once its icons are sent.
Outbound publishes use QoS 0 (retained state heals itself at the next cycle);
the subscription to `boreale/<id>/playguard/+/set` uses QoS 1, the one to
`homeassistant/status` (Home Assistant's birth topic, whatever the discovery
prefix) QoS 0, the one to `boreale/<id>/integration` QoS 1. That last one
is optional: a broker whose access list refuses it keeps the link up (the
integration's version stays unknown, and the log says so). Keep-alive is 30 s, so a console that falls asleep is marked offline
within 45 s. The console sends a PINGREQ whenever the broker has said nothing
for 22 s, and connects again at once after a sleep longer than 45 s.

## `state`

```json
{
  "schema": 1,
  "source": "app",
  "ts": 1791000000,
  "local_date": "2026-10-05",
  "weekday": 1,
  "clock_accurate": true,
  "console": {
    "id": "a1b2c3d4", "name": "Salon",
    "firmware": "23.0.1", "atmosphere": "1.12.0",
    "app_version": "1.1.0", "agent_version": null,
    "emummc": true, "read_only": false
  },
  "controls": {
    "enabled": true, "temp_unlocked": false, "pin_set": true,
    "level": "child", "rating_age": 12,
    "sns_post_restricted": true, "free_communication_restricted": false,
    "vr_restricted": false, "rating_org": 6,
    "companion_linked": false
  },
  "timer": {
    "supported": true, "enabled": true, "limit_reached": false, "blocked_by": null,
    "limits": { "sun": 180, "mon": 120, "tue": 120, "wed": 120, "thu": 120, "fri": 120, "sat": 1440 },
    "limits_min": [180, 120, 120, 120, 120, 120, 1440],
    "uniform_min": null,
    "limit_today_min": 120, "remaining_min": 45, "used_min": 75,
    "spent_raw_min": 76,
    "alarm_on": true,
    "bedtime": { "enabled": true, "start": "21:30:00", "end": "06:00:00" },
    "extended_today_min": 30, "console_locked": false, "profile": null
  },
  "activity_today": {
    "used_min": 74,
    "now": "game",
    "now_playing": { "app_id": "0100000000010000", "name": "Super Mario Odyssey", "since": 1790990000,
                     "kind": "game" }
  },
  "link": {
    "policy": "ask", "remote_timer_writes": false, "ha_discovery": true,
    "agent": false, "last_result": "limit_mon: applied",
    "publishes": { "activity": true, "games": true, "icons": true, "console": true, "network": true,
                   "hardware": true, "serial": false, "report": false }
  },
  "profiles": ["School week", "Holidays"]
}
```

Every object is always there; a value the console could not read is `null`
(so a Home Assistant template never fails on a missing level).

- `source`: `agent` or `app` (which process produced the snapshot).
- `console.name`: `console_name` in `sync.conf`, else the console's nickname
  (System Settings, only with `publish_hardware`), with ` (emuMMC)` on an
  emuMMC; `null` when neither says one (Home Assistant then shows *Nintendo
  Switch*).
- `weekday`: 0 = Sunday, the console's local day; `local_date` the same day.
- `clock_accurate`: the console reports its network clock as accurate (`time`
  command 200). `ts` is only trustworthy when it is true.
- `limits` and `limits_min` run Sunday to Saturday. **1440 means no limit**,
  on every topic and order, so that a plain number entity can express it.
  `uniform_min` is the limit when all seven days share it, else `null`.
- `used_min` is the console timer's own figure (`limit_today_min −
  remaining_min`); `remaining_min` is the whole limit until a game has been
  counted today (command 1454 reads 0 until then), 0 once the limit is
  reached. `spent_raw_min` is command 1952, unverified. `activity_today.used_min` comes from the play
  log and exists even with no limit. See `parental-controls.md` for why they
  differ.
- `limit_reached`: the play timer blocks games (1455), which it also does
  during the bedtime alarm's hours; `blocked_by` says why: `bedtime` from the
  bedtime alarm to the time play is allowed again (the console's local time),
  else `limit` (the day's limit, *No more play today*); `null` while games are
  not blocked.
- `alarm_on`: the "time's up" alarm sounds (command 1953 reads it off).
- `bedtime.start` / `end`: `HH:MM:SS`, as Home Assistant's `time` entity
  takes them. `start` is `null` when bedtime is off; `end` is the console's
  own answer (20.0.0+), else what today's block holds.
- `extended_today_min`: extra time granted today by PlayGuard or by an order
  and still on today's limit.
- `activity_today.now_playing`: the title in front, game or homebrew,
  named whatever it is (*Games and homebrews* below): `app_id`, `name` (the
  game's, the homebrew's, `PlayGuard` for its own forwarder; `null` when not
  known), `since` and `kind` (`game`, `homebrew`, or `system`, which the play
  log never gives); every field `null` while none is (PlayGuard publishing:
  it is in front itself). Filled by the agent.
- `activity_today.now`: what the console is doing (below, *What the console
  is doing*); `null` while play time is not published.
- `console.read_only`: PlayGuard's runtime read-only mode (an untested
  firmware); every order is then refused.
- `link.publishes`: what is published (`activity`, `games`, `icons`,
  `console`, `network`, `hardware`, `serial`, `report`: true or false; *What
  is published* below).
- `timer.profile`: the profile in effect, the saved profile whose seven
  limits are the timer's; `null` when none is.
- `link.last_result`: the last order handled, `<entity>: applied` or
  `<entity>: <reason>`.
- `profiles`: the saved profiles' names, the choices of the `profile` order
  (at most 16; `[]` when there are none).

## `capabilities`

What the device is and which homebrews publish on it, retained, at each
connection (and with each *Sync now*), so one integration serves every
platform:

```json
{"schema": 1, "platform": "switch", "modules": {"playguard": {"schema": 1, "version": "1.4.0"}}}
```

`platform` (`switch`) is also in the `system` document. Each module gives
the `schema` of its documents (`state`'s) and its version.

## `system`

What the console is and how it is doing, beside the parental controls:
built by whichever of PlayGuard and the agent holds the link (the agent
reads it all itself, so it stays fresh while PlayGuard is closed), published
with the state. Every unknown value is `null`.

```json
{"schema": 1, "ts": 1790990000,
 "versions": {"firmware": "23.0.1", "atmosphere": "1.12.0", "hekate": "6.5.4", "app": "1.2.0", "agent": "1.2.0"},
 "hardware": {"model": "oled", "nickname": "Switch de Léa", "region": "eur", "system": "emummc",
              "prodinfo_blanked": true, "serial": null},
 "storage": {"sd": {"free_mb": 21344, "total_mb": 59640}, "nand": {"free_mb": 18470, "total_mb": 26592}},
 "power": {"battery_pct": 87, "charging": true, "charger": "official", "docked": false, "temperature_c": 41},
 "network": {"type": "wifi", "ip": "192.168.1.20", "wifi_bars": 3, "ssid": "Maison"},
 "boot_ts": 1790971980}
```

- `versions.hekate`: the SD card's `bootloader/update.bin`. `app`: PlayGuard's
  (from the agent: the one its last session said). `agent`: `null` without it.
- `hardware.model`: `erista` (the first Switch), `mariko` (its 2019
  revision), `lite`, `oled`, `other` (a development unit). `nickname`: System
  Settings' console nickname. `region`: `jpn`, `usa`, `eur`, `aus`, `htk`
  (Hong Kong, Taiwan, Korea), `chn`. `system`: `sysmmc` or `emummc`.
- `hardware.serial`: only when `sync.conf`'s `publish_serial` is on
  (*Remote access › What is published › Publish the serial number*, off by
  default; with `publish_hardware`); `null`
  otherwise. It is the console's own one, the number *Console › Serial
  number* shows: the system's when PRODINFO is not blanked, else the one in
  Atmosphère's backup of this console's PRODINFO
  (`sd:/atmosphere/automatic_backups/*PRODINFO*.bin`, the console told by its
  DeviceId: `source/util/prodinfo`). The agent publishes the one PlayGuard
  read (`sync/nro_state.txt`). It is never in a report or a log.
- `storage`: MiB, read every 5 minutes; `nand` is the user partition of the
  system in use (the emuMMC's on an emuMMC).
- `power.charging`: a charger that gives power (`charger`: `official` — the
  charger or the dock —, `low_power`, `not_supported`, `none`). `docked`:
  the operation mode `omm` keeps (the dock's, the console asleep in it too),
else the boost performance mode (docked and awake only). `temperature_c`:
the internal sensor.
- `network.type`: `wifi`, `ethernet` or `none`; `wifi_bars` 0–3. The
  network's passphrase, which the same call returns, is wiped at once.
- `boot_ts`: when the console started (POSIX, to the minute), not when it
  last woke up.

## `games`

PlayGuard's (the agent relays the last one): every game installed (the last
check of them, *Console › Check the installed games*) or played, most played
first, as many as fit in 64 KiB (`complete` says whether all did). Games only:
the homebrews are counts (*Games and homebrews* below).

```json
{"schema": 1, "ts": 1790990000, "installed": 42, "accounts": 3, "total_min": 12345,
 "homebrews": 23, "forwarders": 4, "homebrew_min": 640,
 "games": {"0100000000010000": {"name": "Super Mario Odyssey", "version": "1.3.0", "installed": true,
                                "total_min": 1234, "launches": 56, "first_played": 1700000000,
                                "last_played": 1790000000}},
 "complete": true}
```

`total_min` is a game's play time since it was first played, every account
(the play log's statistics); `version` the installed one, `null` for a game
no longer installed. At the top, `installed` counts the games installed (the
forwarders left out), `total_min` every title played, every account, the
homebrews included: the games' `total_min` and `homebrew_min` (the homebrews'
time since first played) add up to it. `homebrews` is the number of `.nro`
files in the SD card's `/switch` folder and its sub-folders one level down
(what *Console › Forwarders* offers), `forwarders` the forwarders installed
(PlayGuard's own included); `null` when not known (the installed titles never
checked). Home Assistant's native discovery has the totals; the
integration lists every game on *Games installed* (an attribute) and keeps
each game's history in a long-term statistic, with no sensor per game.

## `icon/<APP_ID>`

A game's icon, as the HOME menu shows it: the JPEG of its control data
(256 × 256), raw bytes, retained. `<APP_ID>` is 16 uppercase hex characters,
as in `names`. Every game has one, and only games (a homebrew's icon sent by
an earlier version is cleared): those installed (the last check of them,
*Console › Check the installed games*, which PlayGuard also runs at
start-up) and those played, installed or not, when PlayGuard can still read
an icon for it (its icon cache on the SD card, or the console's own cache),
512 at most. An icon is 128 KiB at most; a game's is detailed art, most
likely 20 to 60 KiB (an estimate: no console's icons were measured here;
PlayGuard's own, a flat drawing, is 12 KiB), so a library of 200 games is
some 4 to 12 MiB on the broker, a large one up to 64 MiB in the worst case.

**PlayGuard publishes them**, while it runs and the link is online (its own
session's or the agent's), after a read of the play log. They go over a
short connection of their own (`pg-<id>-icons`, no last will, nothing
subscribed), 32 at most per connection (a large library takes a few, one
after the other), one icon read at a time, so they never hold the session
nor go through `pg:agent`; a connection that fails is tried again 5 minutes
later. Each icon is sent once: what was sent, and to which broker, host,
port and `<prefix>/<id>` (a hash), is kept in `nand/<system>/sync/icons.txt`,
so a new run sends only the new games, and a new broker or console id gets
them all again. A game no longer installed nor in the play log has its icon
cleared (an empty retained message), and so does every one when
`publish_icons` is turned off; nothing is cleared before the installed games
were checked once. *Sync now* on the console sends them all again (an icon
the broker lost, a game whose update changed its icon). A broker whose
maximum packet size (5.0) is smaller than an icon does not get it; that one
is not tried again.

**The agent publishes one icon itself:** the game played, when the broker
does not have it yet (neither `icons.txt` nor its own `agent_icons.txt`
lists it for this broker and console) and PlayGuard's icon cache on the SD
card (`config/playguard/cache/icons/<APP_ID>.jpg`) has it, looked for once
per game while it runs. It streams the file in pieces (the header says the
length, then the bytes follow through the engine's 32 KiB scratch buffer:
no buffer of its own), on its own session, then adds the game to
`agent_icons.txt`, which PlayGuard takes into `icons.txt` when it next reads
the play log. That covers a game PlayGuard read the icon of but could not
send (its link was offline, `publish_icons` was off). A game installed or
first played since PlayGuard last ran is not in that cache: the agent has
no `ns` session to read its control data (and an icon is more than its heap
can spare), so that icon waits for PlayGuard to be opened.

## Games and homebrews

Only games are listed on the broker: `names`, `games`, `icon/<APP_ID>`,
`week`'s and the activity documents' per-game entries. A homebrew (an NRO
forwarder on the HOME menu: PlayGuard's own, those PlayGuard made, those
another app made, sphaira's for instance) is told as totals (`homebrew_s`,
`homebrew_min`) and counts (`homebrews`, `forwarders`); the one in front is
named, game or homebrew (`now_playing`, the events, `now`). What a title is
(`source/sync/sync_titles.h`, shared by PlayGuard and the agent):

| Signal | Says | How sure |
|---|---|---|
| The title ID below `0100000000010000` | The console's own program | Certain: Nintendo numbers its applications from `0100000000010000` (switchbrew's title list) |
| The title ID's top byte other than `01` | A homebrew | Certain the other way round: no Nintendo application has one. sphaira and PlayGuard give their forwarders `05` (`core/forwarder_pack.h`); a homebrew under a `01…` ID is not caught by this |
| *Console › Forwarders*' record (`forwarders.json`) | A homebrew, PlayGuard's own told apart | Certain for those PlayGuard made, whatever their ID |
| PlayGuard's record, `nand/<system>/titles.txt` | What was seen before | As sure as when it was written: a forwarder keeps its kind and name after it is deleted |

What a control.nacp says was looked at and not used: a forwarder's is its
homebrew's own (name, author, version), which nothing tells from a game's.
PlayGuard writes the record after each read of the play log (the forwarders'
names from their control data, the installed titles of its last check, the
forwarders it made) and when it makes a forwarder, and gives the agent its
copy without names, `nand/<system>/sync/titles.txt`. The agent, which reads
no control data, takes a title neither file lists (one installed since
PlayGuard last ran) by its ID: a forwarder of sphaira's or PlayGuard's kind
is a homebrew at once, a homebrew under a `01…` ID a game until PlayGuard
has seen it.

An older Home Assistant integration sees fewer entries (the homebrews gone
from `names`, `games`, `per_app` and `week`), a `total_s` / `total_min` that
no longer equals `per_app`'s sum (the homebrews' time is in it, not listed),
and fields it ignores (`homebrew_s`, `homebrew_min`, `homebrews`,
`forwarders`, `kind`); nothing changes meaning, so `schema` stays 1.

## What the console is doing

The state's `activity_today.now`, for a sensor of its own:

| Value | When | `now_playing` |
|---|---|---|
| `game` | A game has the focus | the game |
| `homebrew` | A homebrew has the focus (a forwarder of another app's or PlayGuard made) | the homebrew, by its name |
| `playguard` | PlayGuard is in front: it publishes the state itself (its own session, or pushed to the agent), or the agent sees PlayGuard's own forwarder in focus | `null` (PlayGuard publishing), or PlayGuard's forwarder |
| `home` | No application has the focus and none waits: the HOME menu, or a system applet over it (the Album, System Settings, the eShop…; the play log does not say which) | `null` |
| `suspended` | No application has the focus, one waits in the background (HOME pressed in a game, not closed) | `null` |
| `null` | Play time not published (`publish_activity`), or the play log could not be read | `null` |

Asleep or off is not a value: the console publishes nothing then, and the
broker's last will says `offline` on `availability` within 45 s.

Where it comes from: PlayGuard says `playguard` whenever it publishes (it
builds its state only in front). The agent reads the play log's newest
events with each state it builds, every `poll_s` (30 s; 5 s for a minute
after an order) and after a change PlayGuard pushes: the newest focus change
names the title in front; with none, the newest application event says
whether one waits (out of focus, not exited) or not (exited, or none since a
launch). So a value is at most `poll_s` late, plus the time `pdm` takes to
write the event (at once, as far as seen). Limits: a game that crashed (no
event of its own before the HOME menu took the focus) reads `home`; a
system applet over a game reads `suspended`, as the HOME menu does; PlayGuard
opened over a game (an applet) is `playguard` only while PlayGuard publishes,
which it does in front only. `play_started` / `play_stopped` keep their
meaning (the title in focus changing); they now come within `poll_s` too.

## `activity`, `activity/<date>`, `names`, `week`

```json
{ "schema": 1, "source": "agent", "ts": 1791000000, "local_date": "2026-10-05", "final": false,
  "total_s": 5040, "total_min": 84, "homebrew_s": 600, "homebrew_min": 10,
  "per_app": [ { "app_id": "0100000000010000", "s": 3600, "min": 60 },
               { "app_id": "01000A10041EA000", "s": 840, "min": 14 } ],
  "per_account": null,
  "now_playing": { "app_id": "0100000000010000", "since": 1790990000, "kind": "game" } }
```

`per_app` lists games only; `homebrew_s` / `homebrew_min` is the homebrews'
time together, in `total_s` with the games' (the console's play timer counts
it): `total_s` is `per_app`'s seconds plus `homebrew_s`. `now_playing` is the
title in front, as the state's (`kind`; `app_id` a homebrew's too).
`activity/<date>` is the same document with `"final": true` and no
`now_playing`. PlayGuard publishes the six finished days its play-log window
holds, once per run, and clears the days 15 to 30 days old. `per_account`
(`[{ "uid": "<32 hex>", "s": …, "min": … }]`) is filled by PlayGuard only when
it read every account's data in the last 15 minutes (the Activity tab's
account filter), else `null`. The agent fills it from the play log: the
accounts opened in a game over the last 7 days, each with the time a game had
the focus while it was open (local play: each account the whole time); an
account that did not play then is left out, and the agent does not open the
account service. Application ids are 16 uppercase hex characters, as the
console prints them. PlayGuard's own time over a game is excluded, as in the
Activity tab. The HOME menu and the system applets (the Album, the eShop, the
Mii editor, the web browser…) are never counted: the play log keeps only
applications, as the console's own statistics do, and the HOME menu taking
the focus ends a game's session; so none is ever `now_playing` either. Were
one of them to show, it is no game: none of the lists has it.

`names` has the games' names only. `week`'s `games` lists games only, and
`homebrew_min` the homebrews' minutes per day, in `days`' order.

```json
{ "schema": 1, "ts": 1760000000,
  "games": { "0100000000010000": "Super Mario Odyssey" },
  "accounts": { "00112233445566778899AABBCCDDEEFF": "Léa" } }
```

```json
{ "schema": 1, "ts": 1760000000, "days": ["2026-10-03", "2026-10-04", "2026-10-05", "2026-10-06", "2026-10-07", "2026-10-08", "2026-10-09"],
  "games": [ { "app_id": "0100000000010000", "min": [0, 45, 60, 30, 0, 90, 60], "total_min": 5400, "launches": 123 } ],
  "homebrew_min": [0, 0, 20, 10, 0, 0, 0],
  "accounts": [ { "uid": "00112233445566778899AABBCCDDEEFF", "min": [0, 45, 60, 30, 0, 90, 60] } ] }
```

## Orders: `<entity>/set`

The publisher (HA, the integration, anything) publishes a **retained scalar**
payload (text, no JSON). The console applies it through the same guarded path
as a press on the console, publishes an `event`, **clears the retained
message** (an empty retained publish on the same topic) and republishes
`state`. An order the broker replays at reconnection (delivered with the RETAIN
flag) is applied only if it differs from the current state. The broker also
sends an order again while it is carried out (it stays retained until then)
at each new subscription: that copy, RETAIN set and the same payload, is not
taken a second time; a press that comes live meanwhile is. Orders received
while PlayGuard is open are applied by PlayGuard, with its policy (*ask* shows
the usual confirmation on the console).

Orders carry no `schema` of their own: they follow the `schema` of the
`state` the console publishes. That is how the console says which orders it
accepts: a publisher sends the entities and payloads of that schema (the
table below for schema 1) and nothing newer, and a console of an older
schema refuses an entity it does not know (`command_rejected`, reason
`unknown_entity`) and clears the order.

| Entity | Payload | Bounds | Needs `remote_timer_writes` | Effect |
|---|---|---|---|---|
| `limit_sun` … `limit_sat` | integer | 0–1440, 1440 = no limit | yes | That weekday's limit. Several within 1.5 s are merged into one write. |
| `limit_uniform` | integer | 0–1440 | yes | The same limit every day. |
| `limits_week` | `n,n,n,n,n,n,n` | Sunday first, each 0–1440 | yes | All seven days in one order. |
| `max_screentime_today` | integer | 0–1440 | yes | Today's weekday limit only. |
| `remove_limit` | `PRESS` | | yes | No limit on any day. |
| `console_lock` | `ON` / `OFF` | | yes | 0 minutes every day (`ON`), saving the limits; `OFF` puts them back (or removes the limit when none were saved). |
| `add_bonus_time` | integer | 5–180 | yes | Extra minutes today, 24 h at most; the usual limit comes back the next day, as in PlayGuard. Refused without a limit today or under the console lock. |
| `add_bonus_time_15`, `_30`, `_60` | `PRESS` | | yes | The same, 15 / 30 / 60 minutes. |
| `stop_today` | `PRESS` | | yes | 0 minutes today; the next day PlayGuard offers to put the usual limit back (or does it, with *Play timer › Put the limit back by itself after "No more play today"*). |
| `parental_controls` | `LOCK` / `UNLOCK` | | `UNLOCK` only | `UNLOCK` unlocks parental controls temporarily with the stored PIN; `LOCK` locks again (1007), always allowed, never asked. While PlayGuard's own pause is on (*Pause parental controls while I'm in PlayGuard*), the state shows the console as for the child; `UNLOCK` keeps the pause's unlock once PlayGuard is left, `LOCK` ends the pause. |
| `unlocked`, `lock_now` | `ON` / `OFF`, `PRESS` | | `ON` only | What the lock replaced, still taken (an integration that sends them): `ON` is `UNLOCK`, `OFF` and `PRESS` are `LOCK`. |
| `play_timer_alarm` | `ON` / `OFF` | | yes | `OFF` = the "time's up" alarm disabled (1953). |
| `bedtime_enabled` | `ON` / `OFF` | | yes | The bedtime alarm on (at the time the days share, else 21:00) or off. |
| `bedtime_alarm` | `HH:MM` (`HH:MM:SS` accepted) | 16:00–23:59 | yes | The bedtime alarm, every day. |
| `bedtime_end_time` | `HH:MM` (`HH:MM:SS` accepted) | 05:00–09:00 | yes | When play is allowed again, on the days with a bedtime. |
| `profile` | a profile name | must exist on the SD card | yes | Applies the saved profile's limits. |
| `restriction_level` | `none` / `young_child` / `child` / `teen` / `custom` (`unrestricted` accepted for `none`) | | no | 1033. The discovery's select shows `none` as `unrestricted` (Home Assistant reads a select's `None` as unknown) and sends `none` back (`cmd_tpl`). |
| `vr_restricted` | `ON` / `OFF` | | no | `ON` = VR mode restricted (1063). |
| `sns_post_restricted`, `free_communication_restricted` | `ON` / `OFF` | custom level only | no | 1036; refused at another level. |
| `sync_now` | `PRESS` | | no | Republish everything now. |
| `export_report` | `PRESS` | | no | Save a diagnostic report on the SD card (and publish it on `report` if `publish_report`). |
| `discovery` | `ON` / `OFF` | | no | Native HA discovery, saved in `sync.conf`. `OFF` clears the discovery payload; used by the HA integration when it takes over. |

`ON` / `OFF` are also accepted as `on` / `off`; `PRESS` as `press`, `1` or
`ON`. A retained order the broker replays at reconnection is
applied again only through the same checks, so an order already in effect
changes nothing and is reported as applied.

There is **no** order that deletes parental controls, unlinks the companion
app, or reads, sets or changes the PIN.

Rejection reasons (`event.reason`): `unknown_entity`, `invalid`,
`out_of_range`, `policy_off` (policy is *off*), `timer_writes_disabled`,
`read_only`, `not_confirmed` (declined on the console under *ask*, or the PIN
was not entered), `gated` (the timer counts down and the unlock did not
happen), `unlock_failed`, `not_custom`, `no_such_profile`, `no_limit_today`,
`console_locked`, `unsupported` (firmware, or a bedtime the block does not
hold as the console reports it), `bedtime_off`, `pctl_error` (with `rc`),
`busy`.

## `event`

```json
{ "schema": 1, "source": "app", "ts": 1760000000,
  "event_type": "command_rejected", "entity": "limit_mon", "payload": "5000",
  "reason": "out_of_range", "rc": null }
```

`event_type`: `command_applied`, `command_rejected`, `command_waiting` (the
agent keeps an order for PlayGuard to confirm under *ask*). `rc` is
`"0x…"` when the console answered, else `null`. Not retained: a consumer that
was away misses them on purpose; the state carries `link.last_result`.

What the console does on its own, found by comparing each state snapshot with
the last one (`source/sync/sync_watch.c`), is published the same way, right
after the state that shows it:

```json
{ "schema": 1, "source": "agent", "ts": 1791000000,
  "event_type": "time_warning", "app_id": "0100000000010000", "kind": "game",
  "minutes_left": 10, "blocked_by": null }
```

| `event_type` | When | `app_id` | `minutes_left` | `blocked_by` |
|---|---|---|---|---|
| `play_started` | A game (or a homebrew) is being played that was not | the game | `null` | `null` |
| `play_stopped` | The game played is no longer (closed, suspended, another one started: `play_stopped` comes first) | the game | `null` | `null` |
| `time_warning` | 10 minutes or fewer left of today's limit, while a game is played; once, and again only after extra time takes the time left back above 10 minutes (or on another day). Play starting with fewer left gives it at once. | the game | the minutes left | `null` |
| `play_blocked` | Games are blocked that were not: the day's limit (or *No more play today*) or bedtime | the game played then, else `null` | `null` | `limit` or `bedtime`, as `timer.blocked_by` |

`app_id` is the title's, a homebrew's too (it is in front: named, as
`now_playing`), and `kind` says which (`game`, `homebrew`; `null` with no
`app_id`). A homebrew's session is an event as a game's: the play timer counts
it, and the time warning matters as much.

Only the agent sends them: it builds the state while a game is played, every
`poll_s` (30 s) and when the play log changes, so an event comes within that.
PlayGuard does not (nobody plays while it is in front), nor the agent while
PlayGuard is (it relays PlayGuard's state then). The first snapshot after the
agent starts says nothing: no event for what was already so.

## Home Assistant native discovery

One retained, device-based payload on
`homeassistant/device/boreale_<id>/config` (about 13 KiB with every
entity), which needs **Home Assistant 2026.5 or later** (the MQTT `time`
platform; an older one refuses the whole payload): `dev` (identifiers
`boreale_<id>`, the console's name as `console.name` says it, else `Nintendo Switch`, manufacturer
`Nintendo`, the model as Nintendo names it (`Nintendo Switch`, `Nintendo
Switch Lite`, `Nintendo Switch – OLED Model`) and `mdl_id` its model number
(`HAC-001`, `HAC-001(-01)`, `HDH-001`, `HEG-001`), `sw` the system
(`23.0.1 · Atmosphère 1.10.1`; read again with each system document, the
discovery following a change)), `o` (origin `PlayGuard`,
version, support URL), `avty_t`, and `cmps` keyed by module and object id
(`playguard_limit_mon`; `uniq_id` `boreale_<id>_playguard_limit_mon`). Every
component but the buttons and the event reads the shared state
(`stat_t` `boreale/<id>/playguard/state`) through a `val_tpl`; writable ones have
`cmd_t` `boreale/<id>/playguard/<entity>/set` and `ret: true`. The entities that need
`remote_timer_writes` are not offered while it is off, nor any writable one in
read-only mode, nor the profile select without profiles: each is then sent as
its platform alone (`"limit_mon": {"p": "number"}`), which removes it from
Home Assistant when an earlier discovery had it. The discovery is published
again when Home Assistant announces `online` on `homeassistant/status`, when
the profiles change and when read-only mode changes.

| Platform | Object ids |
|---|---|
| binary_sensor | `parental_controls_enabled`, `pin_set`, `companion_linked`, `timer_enabled`, `limit_reached` (attribute `reason`: `blocked_by`), `clock_accurate` |
| lock | `parental_controls` (locked while parental controls are set up and not temporarily unlocked) |
| sensor | `used_screen_time`, `used_screen_time_log`, `screen_time_remaining`, `extended_screen_time`, `now_playing`, `rating_age`, `firmware`, `atmosphere`, `spent_raw`, `last_order` |
| number | `limit_sun` … `limit_sat`, `limit_uniform`, `max_screentime_today` (0–1440, step 5, minutes) |
| time | `bedtime_alarm`, `bedtime_end_time` |
| select | `restriction_level` (`unrestricted`, `young_child`, `child`, `teen`, `custom`), `profile` (the saved profiles; only when there are some) |
| switch | `bedtime_enabled`, `console_lock`, `play_timer_alarm`, `vr_restricted`, `sns_post_restricted`, `free_communication_restricted` |
| button | `sync_now`, `export_report`, `add_bonus_time_15`, `add_bonus_time_30`, `add_bonus_time_60`, `stop_today`, `remove_limit` |
| event | `events` (event types `command_applied`, `command_rejected`, `command_waiting`, `play_started`, `play_stopped`, `time_warning`, `play_blocked`) |

`used_screen_time_log` (*Play time today*) is the play log's figure,
`used_screen_time` the timer's (off: it only counts while a limit is set);
both are `total_increasing` (a day's total, back to 0 at midnight),
`screen_time_remaining` is `measurement`, all in minutes, device class
`duration`.
Diagnostic entities (`pin_set`, `clock_accurate`, `rating_age`, `firmware`,
`atmosphere`, `spent_raw`, `last_order`, `export_report`) carry
`ent_cat: diagnostic`; the limits, bedtime, alarm and restrictions `config`.
Those few dashboards need carry `en: false`, off until turned on in Home
Assistant: `clock_accurate`, `used_screen_time`, `limit_sun` …
`limit_sat`, `rating_age`, `spent_raw`, `last_order`, `export_report`,
`accounts`, and those of the
`system` topic but `sd_free`, `nand_free`, `battery`, `charging` and `docked`.
Ids and names follow HA's `nintendo_parental_controls` integration where the
meaning matches. The table is `source/sync/sync_entities.c`.

The HA integration, when installed, publishes `discovery/set = OFF`, and
creates its own entities with the same object ids.

What an earlier version offered and the lock replaced is sent as its platform alone, which removes it from Home
Assistant: `temp_unlocked`, `unlocked` and `lock_now` (the lock
`parental_controls` does all three).

## `integration`

What the HA integration (JigSawFr/boreale-ha) publishes about itself,
retained, at each set-up of a console's entry (so at each start of Home
Assistant), and clears (empty retained publish) when the entry is removed:

```json
{"schema": 1, "version": "0.2.0", "homeassistant": "2026.10.1"}
```

The console keeps it as it came (at most 512 bytes; a larger one is
ignored) in `nand/<system>/sync/integration.json`, written by whichever of
PlayGuard and the agent holds the link. One of a newer `schema` is kept but
not read (no version known). PlayGuard shows the version in
*Remote access › Home Assistant* and in the report, and checks the
integration's latest release with its other updates (*Check for updates
every day*, *Check for updates*), only once the integration is known: a
newer one is in the notification bell, with what changed since (the
release's `CHANGELOG.md`) and the QR code that opens it in HACS.

## Files on the SD card

| File | Written by | Content |
|---|---|---|
| `config/boreale/link.conf` | PlayGuard (*Remote access*), or another Boréale homebrew's screen; hand-editable | The broker's settings every Boréale homebrew of the console shares (boreale-link's `bl_link_conf`): `schema=1`, `enabled`, `host`, `port`, `tls`, `ca_file`, `mqtt_version`, `username`, `password`, `allow_anonymous`, `console_id`, `console_name`, `topic_prefix`, `discovery_prefix`, as in `sync.conf` (below). Made from `sync.conf` the first time PlayGuard or the agent reads its settings, which is then written again without them (schema 3); the one read for those keys afterwards. Unknown keys kept, a newer `schema` never written over. |
| `config/playguard/sync.conf` | PlayGuard (*Remote access*); hand-editable | The link's settings, `key=value`, one per line (below). Shared by every system on the SD card. PlayGuard's own keys only: the broker's are in `link.conf`. |
| `config/playguard/sync/ca.pem` | the user | Optional CA certificate for a private broker. |
| `config/playguard/sync/profiles.txt` | PlayGuard, when the link starts and when the profiles change | `60,90,120,120,120,180,180=School week`: minutes Sunday first (65535 no limit), then the name. Shared, as the profiles are. |
| `config/playguard/nand/<system>/sync/nro_state.txt` | PlayGuard, after every settings save (`config.json`, this system's `state.json`) once `sync.conf` exists, when read-only mode changes and when the serial number it publishes does | The extra-time record, the console lock and its saved limits, `relock_pending`, `extra_auto_restore`, the firmware choice (`fw_gate_*`), `pin_lock`, `read_only` and `firmware` (the one PlayGuard last ran on): what the agent must know to act on PlayGuard's behalf; and `serial`, the serial number PlayGuard read, only with `publish_serial`. The agent changes nothing when `read_only=1`, when the key is missing, or when the console's firmware is no longer `firmware` (a system update PlayGuard has not seen). A line break in a value is written as a space. |
| `config/playguard/nand/<system>/sync/names.txt` | PlayGuard, after a read of the play log | `0100000000010000=Super Mario Odyssey`: the agent's "now playing". |
| `config/playguard/nand/<system>/titles.txt` | PlayGuard, after each read of the play log and when it makes a forwarder | The titles that are not games (*Games and homebrews*), each with its name: `050537E2542E2000=homebrew:sphaira` (`homebrew`, `playguard` for PlayGuard's own forwarder, `system`, `game` for one the ID would misplace). Never deleted: a forwarder deleted since keeps its name. |
| `config/playguard/nand/<system>/sync/titles.txt` | PlayGuard, with the record | The same without the names (`050537E2542E2000=homebrew`): what the agent reads before it publishes play time, `now_playing`, an event or an icon. A kind it does not know (a newer PlayGuard's) is read as `homebrew`: never published by its ID. |
| `config/playguard/nand/<system>/sync/icons.txt` | PlayGuard, after it published or cleared game icons | `target=<8 hex>` (a hash of the broker's host, port and `<prefix>/<id>`), then `0100000000010000=40123`: each icon on the broker and its size in bytes (0: the game has none to publish). Read when PlayGuard starts. |
| `config/playguard/nand/<system>/sync/agent_icons.txt` | the agent, after it published the icon of the game played | The same form, for what the agent sent itself. PlayGuard takes it (renamed, read, removed) after each read of the play log and adds its games to `icons.txt`; the agent reads both before publishing an icon. |
| `config/playguard/cache/icons/<APP_ID>.jpg` | PlayGuard, each icon it reads (its screens, its names, the icons it publishes) | Its icon cache, shared by every system on the SD card (16 MiB, 512 icons at most, emptied when the console language changes or it is full); the agent reads it, never writes it. |
| `config/playguard/nand/<system>/sync/agent_state.txt` | the agent | The same records, for what the agent changed itself. At start the agent takes the newer of this file and `nro_state.txt`; PlayGuard adopts the agent's records (`GetRecords`) when it opens. |
| `config/playguard/nand/<system>/sync/integration.json` | the link that got it (PlayGuard's or the agent's) | The HA integration's document (`integration`, above), as it came; removed when the integration clears it. PlayGuard reads it at start, and every 30 s while the agent holds the link. |
| `config/playguard/nand/<system>/sync/agent_events.log` | the agent | One line per order the agent carried out itself, tab-separated: time (POSIX), entity, payload, applied (0/1), reason, change kind (`SyncChange`), number of values, source (`remote`, `remote_extra`…), values before (comma-separated), values after, the console lock afterwards (-1 unchanged, 0, 1). PlayGuard imports them into `history.json`, with the agent's time, when it opens a session, and removes the file. A line with more fields than PlayGuard knows is read for the ones it knows; one it cannot read is kept in `agent_events.log.rejected` (for a later version), and the file is not removed until it is. |
| `config/playguard/nand/<system>/sync/agent_watchdog.log` | the agent | One line each time its loop has not turned for a minute, and when it turns again: the local date and time, how long, and where it stopped (and whether its socket was shut down to free it). Started again past 16 KiB. PlayGuard's report shows the last 6 lines. |
| `config/playguard/logs/reports/report_<date>_<time>.txt` | the agent | A report asked for by `export_report` while PlayGuard is closed (clocks and parental controls, as PlayGuard's report has them). Named as PlayGuard's reports, so *About › Help & diagnostics* lists, sends and prunes it with them. |
| `atmosphere/contents/<tid>/{exefs.nsp, flags/boot2.flag, toolbox.json, version.txt}` | *Console › Optional modules*, or the user | The installed module (`4200000000505247` recovery, `4200000000504741` agent). |

`<system>` is `sysmmc`, `emummc` or `unknown`, as for the rest of PlayGuard's
per-system files (`util/nand.hpp`; the agent names it the same way,
`core/nand_key.h`). Each file has one writer — but for `sync.conf`, which the agent rewrites when
Home Assistant switches the native discovery (`discovery/set`) while
PlayGuard is closed. The IPC service is the live channel, the files the cold
one. `sync.conf` and `link.conf` are never included in a diagnostic report or
an online upload; the report's *Remote link* section has the switches and the
session's counters, never the broker's address, the user name or the
password.

`link.conf`'s keys: `schema=1`, `enabled`, `host`, `port` (1883; 8883 turns
`tls` on by default), `tls`, `ca_file`, `mqtt_version` (`auto` / `5` /
`3.1.1`), `username`, `password`, `allow_anonymous`, `console_id`,
`console_name`, `topic_prefix` (`boreale`), `discovery_prefix`
(`homeassistant`). `sync.conf`'s: `schema=3`, `policy` (`ask` / `auto` /
`off`; the agent alone only applies `auto`), `remote_timer_writes`,
`publish_report`, `publish_activity`, `publish_games`, `publish_icons`, `publish_console`,
`publish_network`, `publish_hardware`, `publish_serial` (0: the serial number
stays off the broker; it needs `publish_hardware` too) (see *What is
published* below), `ha_discovery`, `poll_s` (30, 10–300),
`legacy_prefix` (below),
`log_level`. A value out of range keeps its default; unknown keys are kept
when PlayGuard writes the file again. A file with a higher `schema` (a newer
version's) is read for the keys this one knows and never written again, by
PlayGuard or the agent; nor is one with more unknown lines than fit (1 KB of
them). Anonymous brokers need
`allow_anonymous=1`; without a user name the link otherwise stays off.
A `sync.conf` of schema 1 or 2 still holds the broker's keys: the first
read moves them to `link.conf` (when it has none yet; else `link.conf`'s
win) and writes `sync.conf` again without them, so nothing of them is left
there.

A `schema=1` file (an earlier version's) is read the same way: its
`topic_prefix`, when it is the old default `playguard`, becomes `boreale`,
and the prefix it had is kept as `legacy_prefix`. At its next connection,
the console clears (empty retained publish) what it left on the old tree:
`<legacy_prefix>/<id>/{state, activity, week, names, games, report}`, each
`<entity>/set`, the finished days around today (`activity/<date>`),
`availability` and `system` when the prefix changed, and
`homeassistant/device/playguard_<id>/config`; then it writes the file again
as `schema=2`, without `legacy_prefix`. The icons are PlayGuard's: when
`icons.txt` says they went to `playguard/<id>` on the same broker, its next
batch clears them there and publishes them again under the new tree.

### What is published

The parental controls, the play timer and the link are always published.
The rest is on by default, the serial number off, each with its key
(*Remote access › What is published*, behind the PIN like every setting):

| Key | What | Where |
|---|---|---|
| `publish_activity` | Play time | `activity`, `week`, `names`, `activity/<date>`, the state's `activity_today` |
| `publish_games` | The games | `games` |
| `publish_icons` | The games' icons | `icon/<APP_ID>`, cleared by PlayGuard (it knows which were sent) |
| `publish_console` | The console's state | the system document's `power`, `storage`, `boot_ts` |
| `publish_network` | The network | the system document's `network` |
| `publish_hardware` | Versions and hardware | the system document's `versions` (but PlayGuard's and the agent's) and `hardware`, the state's `console.firmware` and `console.atmosphere`, the discovery device's model and system |
| `publish_serial` | The serial number (with `publish_hardware`) | `hardware.serial` |
| `publish_report` | Diagnostic reports | `report` |

What is off reads `null` in the documents that stay (every key still
there); a document of its own is cleared (an empty retained publish) when
it is turned off and at each connection, the finished days by PlayGuard;
its entities are sent as their platform alone in the discovery, which
removes them from Home Assistant. The state's `link.publishes` says what is
published, for the integration to follow.

## IPC service `pg:agent` (PlayGuard ↔ agent)

Hosted by the agent (`sysmodule/agent/source/agent_ipc.c`); PlayGuard's
client is `source/util/agent_client_nx.cpp` (`source/action/sync_flow.cpp`
drives it). Command ids, structures and result codes are in
`source/sync/agent_ipc.h` (protocol 3). The IPC message holds 256 bytes, so
documents, the log, an order and the status travel in mapped buffers.

PlayGuard looks for the service when the link is on, at start-up and every
5 s after (Atmosphère's `sm` answers whether a service is registered without
waiting for it), so the agent can be started or stopped while PlayGuard runs
(*Console › Optional modules* does it, and hands the link over at once). With
the agent there, PlayGuard opens no broker session of its own; when the
agent goes away, PlayGuard's own session takes over. Once its session opens,
PlayGuard adopts the agent's records (`GetRecords`) when `agent_state.txt` is
not older than `nro_state.txt` (else it asks the agent to read PlayGuard's
newer ones: `ReloadConfig`), imports
`agent_events.log` into its history, and asks for `SyncNow` (which also says
`online` again over any `offline` its own session left behind).

**An open session means PlayGuard is running.** While it is in the foreground
it is the only process that reads pctl: the agent publishes what it pushes.
In the background, or once the session closes (exit or crash), the agent
reads the console again by itself. Orders received while a session is open
are handed to PlayGuard, which carries them out with its own policy, history
and records; those still out when the session closes are answered `waiting`
and come back from the broker for the agent.

| Id | Command | In | Out | Effect |
|---|---|---|---|---|
| 0 | `Hello` | `AgentHello`: protocol, PlayGuard's version | `AgentHelloReply`: protocol, the agent's version, console id, online | Opens PlayGuard's session. Another protocol gets the reply (PlayGuard offers to update the agent) and no session. |
| 1 | `SetForeground` | `u8` | | In the foreground PlayGuard reads and pushes; in the background the agent reads. |
| 2 | `PushState` | buffer: JSON | | Publish `state` now. PlayGuard pushes it when it changes, and every `poll_s` for a fresh time stamp. |
| 3 | `PushActivity` | buffer: JSON | | Publish `activity` now. |
| 4 | `PushNames` | buffer: JSON | | Publish `names`. |
| 5 | `PushWeek` | buffer: JSON | | Publish `week`. |
| 6 | `PushFinal` | buffer: `YYYY-MM-DD` + newline + JSON | | Publish `activity/<date>` (an empty document clears it). Seven wait at most: `AGENT_RC_FULL`, and PlayGuard sends it again a second later. |
| 7 | `PopOrder` | | buffer: `AgentOrder` (id 0: none) | The next order for PlayGuard, each handed out once. PlayGuard asks every second, in the foreground, one order at a time. |
| 8 | `OrderResult` | `AgentResult`: id, rc, applied, changed, reason | | The agent publishes the event, clears the retained order, republishes the state. |
| 9 | `SyncNow` | | | Republish everything. |
| 10 | `GetStatus` | | buffer: `AgentStatus` | The link's status, pending orders, read-only, version, uptime. |
| 12 | `GetRecords` | | `AgentRecords` | What the agent changed (extra time, console lock, relock), for PlayGuard to adopt. |
| 13 | `ReloadConfig` | | | Read `sync.conf` and `nro_state.txt` again (also done when their date changes). |
| 14 | `GetLog` | | buffer: text | The agent's last 200 lines, oldest first. |
| 15 | `PrepareShutdown` | | | Publish `offline`, disconnect, stay idle: first step of an update or a stop from PlayGuard. A later `Hello` of the same protocol brings the link back (the stop did not follow). |
| 16 | `PushGames` | buffer: JSON | | Publish `games`. |

Everything but `Hello`, `GetStatus` and `GetLog` needs the session. The
command dispatch is host-tested (`tests/agent/`); the HIPC framing runs only
on a console.

`AgentHello` and `AgentHelloReply` never change: a `Hello` of any size gets
its reply, so any PlayGuard learns the agent's protocol. The size of every
structure exchanged (and of `SyncIntent`, `SyncStatus` and `SyncRecords`,
which they embed) is checked at build time against the values next to
`AGENT_PROTOCOL`: a change to one fails the build until those values are
updated and `AGENT_PROTOCOL` goes up.

## Schema and compatibility

- `schema` is on every JSON document and in `sync.conf`. Adding a field does
  not bump it; removing or renaming one, or changing an order's payload, does.
- Orders (`<entity>/set`) follow `state`'s `schema`: the console announces
  there what it accepts (see *Orders* above).
- The HA integration pins the schema it understands and raises a repair issue
  on a newer one.
- The entity table (`source/sync/sync_entities.h`) is exported by a host test
  as `entities.json`; the integration carries a copy and its tests fail when
  the two diverge.
