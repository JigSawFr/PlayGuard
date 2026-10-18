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

| Topic | Retained | Direction | Content |
|---|---|---|---|
| `playguard/<id>/availability` | yes | console → | `online` or `offline`. Last will (`offline`), and `offline` published before a clean disconnect. |
| `playguard/<id>/state` | yes | console → | The state snapshot (below). |
| `playguard/<id>/system` | yes | console → | The console itself: versions, model, name, storage, power, network (below). With the state, every `poll_s`. |
| `playguard/<id>/games` | yes | console (app) → | The games installed and played, each with its play time in all (below), published by PlayGuard after a read of the play log. |
| `playguard/<id>/activity` | yes | console → | Today's play time from the play log. |
| `playguard/<id>/activity/<YYYY-MM-DD>` | yes | console → | A finished day's totals, published at the first cycle after local midnight. The console keeps the last 14 and clears older ones (empty retained publish). |
| `playguard/<id>/names` | yes | console (app) → | Game and account names, published by PlayGuard when it runs. |
| `playguard/<id>/week` | yes | console (app) → | The 7-day table per game and per account, published by PlayGuard when it runs. |
| `playguard/<id>/<entity>/set` | yes (by the publisher) | → console | An order. Cleared by the console once handled. |
| `playguard/<id>/event` | **no** | console → | One event per order handled: `command_applied`, `command_rejected`, `command_waiting` (an order waiting for PlayGuard or the parent) |
| `playguard/<id>/report` | yes | console → | The last diagnostic report, plain text, at most 128 KiB. Only when `publish_report` is on. |
| `homeassistant/device/playguard_<id>/config` | yes | console → | HA native discovery. Only when `ha_discovery` is on; cleared when it is turned off. |
| `homeassistant/status` | — | ← HA | Subscribed: on `online`, the console republishes discovery, state and activity. |
| `playguard/<id>/integration` | yes | ← the HA integration | Which version of the integration (and of Home Assistant) runs (below), at each set-up of the integration's entry; cleared when the entry is removed. Never cleared by the console. |

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
Outbound publishes use QoS 0 (retained state heals itself at the next cycle);
the subscription to `playguard/<id>/+/set` uses QoS 1, the one to
`homeassistant/status` (Home Assistant's birth topic, whatever the discovery
prefix) QoS 0, the one to `playguard/<id>/integration` QoS 1. That last one
is optional: a broker whose access list refuses it keeps the link up (the
integration's version stays unknown, and the log says so). Keep-alive is 30 s, so a console that falls asleep is marked offline
within 45 s.

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
    "supported": true, "enabled": true, "limit_reached": false,
    "limits": { "sun": 180, "mon": 120, "tue": 120, "wed": 120, "thu": 120, "fri": 120, "sat": 1440 },
    "limits_min": [180, 120, 120, 120, 120, 120, 1440],
    "uniform_min": null,
    "limit_today_min": 120, "remaining_min": 45, "used_min": 75,
    "spent_raw_min": 76,
    "alarm_on": true,
    "bedtime": { "enabled": true, "start": "21:30:00", "end": "06:00:00" },
    "extended_today_min": 30, "console_locked": false
  },
  "activity_today": {
    "used_min": 74,
    "now_playing": { "app_id": "0100000000010000", "name": "Super Mario Odyssey", "since": 1790990000 }
  },
  "link": {
    "policy": "ask", "remote_timer_writes": false, "ha_discovery": true,
    "agent": false, "last_result": "limit_mon: applied"
  },
  "profiles": ["School week", "Holidays"]
}
```

Every object is always there; a value the console could not read is `null`
(so a Home Assistant template never fails on a missing level).

- `source`: `agent` or `app` (which process produced the snapshot).
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
- `alarm_on`: the "time's up" alarm sounds (command 1953 reads it off).
- `bedtime.start` / `end`: `HH:MM:SS`, as Home Assistant's `time` entity
  takes them. `start` is `null` when bedtime is off; `end` is the console's
  own answer (20.0.0+), else what today's block holds.
- `extended_today_min`: extra time granted today by PlayGuard or by an order
  and still on today's limit.
- `activity_today.now_playing`: `null` while PlayGuard publishes (it is in
  front, so no game is being played); filled by the agent.
- `console.read_only`: PlayGuard's runtime read-only mode (an untested
  firmware); every order is then refused.
- `link.last_result`: the last order handled, `<entity>: applied` or
  `<entity>: <reason>`.
- `profiles`: the saved profiles' names, the choices of the `profile` order
  (at most 16; `[]` when there are none).

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
  (*Remote access › Publish the serial number*, off by default); `null`
  otherwise. It is the console's own one: the system's when PRODINFO is not
  blanked, else the one in Atmosphère's backup of the real PRODINFO
  (`sd:/atmosphere/automatic_backups/*PRODINFO*.bin`). It is never in a
  report or a log.
- `storage`: MiB, read every 5 minutes; `nand` is the user partition of the
  system in use (the emuMMC's on an emuMMC).
- `power.charging`: a charger that gives power (`charger`: `official` — the
  charger or the dock —, `low_power`, `not_supported`, `none`). `docked`:
  the boost performance mode. `temperature_c`: the internal sensor.
- `network.type`: `wifi`, `ethernet` or `none`; `wifi_bars` 0–3. The
  network's passphrase, which the same call returns, is wiped at once.
- `boot_ts`: when the console started (POSIX, to the minute), not when it
  last woke up.

## `games`

PlayGuard's (the agent relays the last one): every game installed (the last
check of them, *Console › Check the installed games*) or played, most played
first, as many as fit in 64 KiB (`complete` says whether all did).

```json
{"schema": 1, "ts": 1790990000, "installed": 42, "accounts": 3, "total_min": 12345,
 "games": {"0100000000010000": {"name": "Super Mario Odyssey", "version": "1.3.0", "installed": true,
                                "total_min": 1234, "launches": 56, "first_played": 1700000000,
                                "last_played": 1790000000}},
 "complete": true}
```

`total_min` is a game's play time since it was first played, every account
(the play log's statistics); `version` the installed one, `null` for a game
no longer installed. Home Assistant's native discovery has the totals; the
integration adds one sensor per game.

## `activity`, `activity/<date>`, `names`, `week`

```json
{ "schema": 1, "source": "agent", "ts": 1791000000, "local_date": "2026-10-05", "final": false,
  "total_s": 4440, "total_min": 74,
  "per_app": [ { "app_id": "0100000000010000", "s": 3600, "min": 60 },
               { "app_id": "01000A10041EA000", "s": 840, "min": 14 } ],
  "per_account": null,
  "now_playing": { "app_id": "0100000000010000", "since": 1790990000 } }
```

`activity/<date>` is the same document with `"final": true` and no
`now_playing`. PlayGuard publishes the six finished days its play-log window
holds, once per run, and clears the days 15 to 30 days old. `per_account`
(`[{ "uid": "<32 hex>", "s": …, "min": … }]`) is filled by PlayGuard only when
it read every account's data in the last 15 minutes (the Activity tab's
account filter), else `null`; the agent does not open the account service. Application ids are 16 uppercase hex characters, as the
console prints them. PlayGuard's own time over a game is excluded, as in the
Activity tab.

```json
{ "schema": 1, "ts": 1760000000,
  "games": { "0100000000010000": "Super Mario Odyssey" },
  "accounts": { "00112233445566778899AABBCCDDEEFF": "Léa" } }
```

```json
{ "schema": 1, "ts": 1760000000, "days": ["2026-10-03", "2026-10-04", "2026-10-05", "2026-10-06", "2026-10-07", "2026-10-08", "2026-10-09"],
  "games": [ { "app_id": "0100000000010000", "min": [0, 45, 60, 30, 0, 90, 60], "total_min": 5400, "launches": 123 } ],
  "accounts": [ { "uid": "00112233445566778899AABBCCDDEEFF", "min": [0, 45, 60, 30, 0, 90, 60] } ] }
```

## Orders: `<entity>/set`

The publisher (HA, the integration, anything) publishes a **retained scalar**
payload (text, no JSON). The console applies it through the same guarded path
as a press on the console, publishes an `event`, **clears the retained
message** (an empty retained publish on the same topic) and republishes
`state`. An order the broker replays at reconnection (delivered with the RETAIN
flag) is applied only if it differs from the current state. Orders received
while PlayGuard is open are applied by PlayGuard, with its policy (*ask* shows
the usual confirmation on the console).

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
| `unlocked` | `ON` / `OFF` | | `ON` only | `ON` unlocks parental controls temporarily with the stored PIN; `OFF` locks again. |
| `lock_now` | `PRESS` | | no | Locks again (1007), always allowed, never asked. |
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

## Home Assistant native discovery

One retained, device-based payload on
`homeassistant/device/playguard_<id>/config` (about 13 KiB with every
entity), which needs **Home Assistant 2026.5 or later** (the MQTT `time`
platform; an older one refuses the whole payload): `dev` (identifiers
`playguard_<id>`, the console's name or `Nintendo Switch`, manufacturer
`PlayGuard`, model `Nintendo Switch`, `sw`), `o` (origin `PlayGuard`,
version, support URL), `avty_t`, and `cmps` keyed by object id. Every
component but the buttons and the event reads the shared state
(`stat_t` `playguard/<id>/state`) through a `val_tpl`; writable ones have
`cmd_t` `playguard/<id>/<entity>/set` and `ret: true`. The entities that need
`remote_timer_writes` are not offered while it is off, nor any writable one in
read-only mode, nor the profile select without profiles: each is then sent as
its platform alone (`"limit_mon": {"p": "number"}`), which removes it from
Home Assistant when an earlier discovery had it. The discovery is published
again when Home Assistant announces `online` on `homeassistant/status`, when
the profiles change and when read-only mode changes.

| Platform | Object ids |
|---|---|
| binary_sensor | `parental_controls_enabled`, `temp_unlocked`, `pin_set`, `companion_linked`, `timer_enabled`, `limit_reached`, `clock_accurate` |
| sensor | `used_screen_time`, `used_screen_time_log`, `screen_time_remaining`, `extended_screen_time`, `now_playing`, `rating_age`, `firmware`, `atmosphere`, `spent_raw` (disabled by default), `last_order` |
| number | `limit_sun` … `limit_sat`, `limit_uniform`, `max_screentime_today` (0–1440, step 5, minutes) |
| time | `bedtime_alarm`, `bedtime_end_time` |
| select | `restriction_level` (`unrestricted`, `young_child`, `child`, `teen`, `custom`), `profile` (the saved profiles; only when there are some) |
| switch | `bedtime_enabled`, `console_lock`, `play_timer_alarm`, `unlocked`, `vr_restricted`, `sns_post_restricted`, `free_communication_restricted` |
| button | `sync_now`, `lock_now`, `export_report`, `add_bonus_time_15`, `add_bonus_time_30`, `add_bonus_time_60`, `stop_today`, `remove_limit` |
| event | `events` (event types `command_applied`, `command_rejected`, `command_waiting`) |

`used_screen_time` is the timer's figure, `used_screen_time_log` the play
log's; both are `total_increasing` (a day's total, back to 0 at midnight),
`screen_time_remaining` is `measurement`, all in minutes, device class
`duration`.
Diagnostic entities (`pin_set`, `clock_accurate`, `rating_age`, `firmware`,
`atmosphere`, `spent_raw`, `last_order`, `export_report`) carry
`ent_cat: diagnostic`; the limits, bedtime, alarm and restrictions `config`.
Ids and names follow HA's `nintendo_parental_controls` integration where the
meaning matches. The table is `source/sync/sync_entities.c`.

The HA integration, when installed, publishes `discovery/set = OFF`, and
creates its own entities with the same object ids.

## `integration`

What the HA integration (JigSawFr/playguard-ha) publishes about itself,
retained, at each set-up of a console's entry (so at each start of Home
Assistant), and clears (empty retained publish) when the entry is removed:

```json
{"schema": 1, "version": "0.2.0", "homeassistant": "2026.10.1"}
```

The console keeps it as it came (at most 512 bytes; a larger one is
ignored) in `nand/<system>/sync/integration.json`, written by whichever of
PlayGuard and the agent holds the link. PlayGuard shows the version in
*Remote access › Home Assistant* and in the report, and checks the
integration's latest release with its other updates (*Check for updates
every day*, *Check for updates*), only once the integration is known: a
newer one is in the notification bell, with what changed since (the
release's `CHANGELOG.md`) and the QR code that opens it in HACS.

## Files on the SD card

| File | Written by | Content |
|---|---|---|
| `config/playguard/sync.conf` | PlayGuard (*Remote access*); hand-editable | The link's settings, `key=value`, one per line (below). Shared by every system on the SD card. |
| `config/playguard/sync/ca.pem` | the user | Optional CA certificate for a private broker. |
| `config/playguard/sync/profiles.txt` | PlayGuard, when the link starts and when the profiles change | `60,90,120,120,120,180,180=School week`: minutes Sunday first (65535 no limit), then the name. Shared, as the profiles are. |
| `config/playguard/nand/<system>/sync/nro_state.txt` | PlayGuard, after every settings save (`config.json`, this system's `state.json`) once `sync.conf` exists, and when read-only mode changes | The extra-time record, the console lock and its saved limits, `relock_pending`, `extra_auto_restore`, the firmware choice (`fw_gate_*`), `pin_lock`, `read_only` and `firmware` (the one PlayGuard last ran on): what the agent must know to act on PlayGuard's behalf. The agent changes nothing when `read_only=1`, when the key is missing, or when the console's firmware is no longer `firmware` (a system update PlayGuard has not seen). A line break in a value is written as a space. |
| `config/playguard/nand/<system>/sync/names.txt` | PlayGuard, after a read of the play log | `0100000000010000=Super Mario Odyssey`: the agent's "now playing". |
| `config/playguard/nand/<system>/sync/agent_state.txt` | the agent | The same records, for what the agent changed itself. At start the agent takes the newer of this file and `nro_state.txt`; PlayGuard adopts the agent's records (`GetRecords`) when it opens. |
| `config/playguard/nand/<system>/sync/integration.json` | the link that got it (PlayGuard's or the agent's) | The HA integration's document (`integration`, above), as it came; removed when the integration clears it. PlayGuard reads it at start, and every 30 s while the agent holds the link. |
| `config/playguard/nand/<system>/sync/agent_events.log` | the agent | One line per order the agent carried out itself, tab-separated: time (POSIX), entity, payload, applied (0/1), reason, change kind (`SyncChange`), number of values, source (`remote`, `remote_extra`…), values before (comma-separated), values after, the console lock afterwards (-1 unchanged, 0, 1). PlayGuard imports them into `history.json`, with the agent's time, when it opens a session, and removes the file. |
| `config/playguard/logs/reports/report_<date>_<time>.txt` | the agent | A report asked for by `export_report` while PlayGuard is closed (clocks and parental controls, as PlayGuard's report has them). Named as PlayGuard's reports, so *About › Help & diagnostics* lists, sends and prunes it with them. |
| `atmosphere/contents/<tid>/{exefs.nsp, flags/boot2.flag, toolbox.json, version.txt}` | *Console › Optional modules*, or the user | The installed module (`4200000000505247` recovery, `4200000000504741` agent). |

`<system>` is `sysmmc`, `emummc` or `unknown`, as for the rest of PlayGuard's
per-system files (`util/nand.hpp`; the agent names it the same way,
`core/nand_key.h`). Each file has one writer — but for `sync.conf`, which the agent rewrites when
Home Assistant switches the native discovery (`discovery/set`) while
PlayGuard is closed. The IPC service is the live channel, the files the cold
one. `sync.conf` is never included in a diagnostic report or
an online upload; the report's *Remote link* section has the switches and the
session's counters, never the broker's address, the user name or the
password.

`sync.conf` keys: `schema=1`, `enabled`, `host`, `port` (1883; 8883 turns
`tls` on by default), `tls`, `ca_file`, `mqtt_version` (`auto` / `5` /
`3.1.1`), `username`, `password`,
`allow_anonymous`, `console_id`, `console_name`, `policy` (`ask` / `auto` /
`off`; the agent alone only applies `auto`), `remote_timer_writes`,
`publish_report`, `publish_activity`, `publish_serial` (0: the serial number
stays off the broker), `ha_discovery`, `poll_s` (30, 10–300),
`topic_prefix` (`playguard`), `discovery_prefix` (`homeassistant`),
`log_level`. A value out of range keeps its default; unknown keys are kept
when PlayGuard writes the file again. Anonymous brokers need
`allow_anonymous=1`; without a user name the link otherwise stays off.

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

## Schema and compatibility

- `schema` is on every JSON document and in `sync.conf`. Adding a field does
  not bump it; removing or renaming one, or changing an order's payload, does.
- The HA integration pins the schema it understands and raises a repair issue
  on a newer one.
- The entity table (`source/sync/sync_entities.h`) is exported by a host test
  as `entities.json`; the integration carries a copy and its tests fail when
  the two diverge.
