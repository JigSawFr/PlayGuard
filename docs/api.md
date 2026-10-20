# The launch API (draft, v1)

> [!NOTE]
> **A proposal, not implemented yet.** This page is the contract to build
> against; nothing below exists in a release until the changelog says so.

Other homebrews can ask PlayGuard to do something (set the network clock),
tell them something (the time left today), or open on a given screen (a
game's activity). There is no server and nothing runs in the background: a
homebrew **launches PlayGuard with a request**, PlayGuard does it, writes a
JSON response and, when asked, **launches the caller again**. The requests and
responses borrow REST's shape (a method, a path, a status code, a JSON body)
because it is familiar, not because there is HTTP underneath.

## Two ways to call

| Mode | What happens | Typical use |
|---|---|---|
| **`call`** | PlayGuard does the request with a minimal screen, writes the response, then launches `return_to` (or exits). | Set the clock, read the status, from another app or a HOME-menu icon. |
| **`open`** | PlayGuard opens on the screen the path names and stays there, as if the user had gone to it. When the user leaves, it launches `return_to` (or exits). | Show a game's activity or check it, from a game launcher. |

So yes: a homebrew that wants an answer launches PlayGuard, which launches it
back. The round trip is the price of nothing running in the background; for
the little that is worth reading without it, see
[the snapshot](#reading-without-launching-playguard).

## Off until a parent turns it on

The API answers nothing until *Preferences › Other homebrews* is turned on,
from a parent's session (behind the PIN, like every setting). While it is
off, every request gets `403 api_disabled` and nothing else is done (the
request file is still consumed).

Once it is on, each group of routes gets its own permission:

| Permission | Allows |
|---|---|
| **None** | Nothing: `403 permission_denied`. |
| **Read** | The routes that change nothing: `GET`, and `OPEN` (a screen, where any change goes through the app as usual). |
| **Read and write** | Those, and the routes that change something (`POST`). |

| Group | Read | Write | Default |
|---|---|---|---|
| **clock** | measure the offset, open *Network clock* | set the clock from NTP | None |
| **status** | the time left today, the snapshot | (none) | None |
| **activity** | time per game and per account (PIN), a game's page | (none) | None |
| **games** | check a game, open its check | (none in v1; repairs stay on the screen) | None |
| **play time** (later) | (none) | extra time, stop for today (PIN) | None |

A group with nothing to write only offers *None* and *Read*. Turning the API
on changes no permission: the parent picks each one. Each route below says
which permission it needs. In `config.json` (`api`):

```json
"api": {
  "enabled": true,
  "permissions": { "clock": "read_write", "status": "read", "activity": "none", "games": "read" }
}
```

A missing or unknown value is `"none"`.

Whatever the request says, it **never authorises anything**: an action that
changes parental controls goes through the PIN screen, the `caller` field is
only shown and recorded, and there is no route that sets the clock to a given
time (one that moved the day forward would hand the child a new daily limit).
`POST /clock/sync` only puts back the time NTP servers give.

## Launching PlayGuard

PlayGuard reads its arguments (`argv`) at start-up. A homebrew passes them
with `envSetNextLoad()`; a launcher or an [NRO forwarder](features.md) passes
them from its settings.

```
# Short form: one route, no body. Fits a HOME-menu forwarder.
sdmc:/switch/playguard/playguard.nro --api "POST /v1/clock/sync"
sdmc:/switch/playguard/playguard.nro --api "OPEN /v1/games/0100000000010000"

# Full form: a request file.
sdmc:/switch/playguard/playguard.nro --api-request sdmc:/switch/myapp/pg_request.json
```

In the short form, `OPEN` is the `open` mode and every other method the `call`
mode; there is no `return_to`, so PlayGuard exits when done.

## The request

```json
{
  "api": "v1",
  "id": "8f14e45f",
  "mode": "call",
  "method": "POST",
  "path": "/clock/sync",
  "body": {},
  "return_to": "sdmc:/switch/myapp/myapp.nro",
  "response": "sdmc:/switch/myapp/pg_response.json",
  "caller": "MyApp 1.2"
}
```

| Key | Required | Meaning |
|---|---|---|
| `api` | yes | `"v1"`. Any other value: `400 unsupported_api`. |
| `id` | yes | 1 to 64 characters `[A-Za-z0-9_-]`, chosen by the caller. Copied into the response, and the key that makes a repeated request [harmless](#each-request-acts-once). |
| `mode` | no | `"call"` (default) or `"open"`. |
| `method`, `path` | yes | The route, below. The path may carry a query (`?day=2026-10-20`). |
| `body` | no | A JSON object, the route's parameters. |
| `return_to` | no | The `.nro` to launch when done. It must exist, end in `.nro` and be under `sdmc:/switch/`; anything else is ignored (and said in the response). |
| `response` | no | Where to write the response: a `.json` path under `sdmc:/switch/` or `sdmc:/config/`. Default `sdmc:/config/playguard/api/response.json`. |
| `caller` | no | A name to show and record in the history, at most 64 characters. Not checked, so not trusted. |

The request file is **deleted as soon as it is read**, before anything is
done, so a request left on the card is never replayed at the next start (if
it cannot be deleted, it is not done: `500 request_not_consumed`). A file over
16 KiB, not JSON, or without the required keys gets `400 bad_request`.

## The response

Written atomically (a temporary file, then a rename) before `return_to` is
launched, so the caller finds it at its next start:

```json
{
  "api": "v1",
  "id": "8f14e45f",
  "status": 200,
  "body": {
    "offset_ms_before": -183000,
    "offset_ms_after": 12,
    "server": "pool.ntp.org"
  },
  "playguard": "1.9.0"
}
```

A caller should check that `id` is the one it sent: a response from an earlier
request may still be on the card.

### Errors

An error's body follows [RFC 9457](https://www.rfc-editor.org/rfc/rfc9457)
(*problem details*), with a stable `code` to test and a `title` already in the
console's language, ready to show:

```json
{
  "api": "v1",
  "id": "8f14e45f",
  "status": 503,
  "body": {
    "type": "https://playguard.link/api/errors/no_network",
    "code": "no_network",
    "title": "Pas de connexion Internet : l'horloge n'a pas été réglée.",
    "detail": "No NTP server answered (3 tried)."
  }
}
```

| Status | `code` | When |
|---|---|---|
| `200` | | Done (or, for `open`, the screen was shown). |
| `400` | `bad_request`, `unsupported_api` | The request cannot be read. |
| `403` | `api_disabled`, `permission_denied` | The API is off, or the route's group lacks the permission (`detail` names the group and the permission needed). |
| `403` | `pin_refused` | The PIN screen was cancelled or the PIN was wrong. |
| `404` | `no_route`, `no_game` | Unknown path, or no such game installed. |
| `422` | `bad_body` | A parameter is missing or out of range. |
| `423` | `read_only` | The firmware is newer than PlayGuard supports: it [opens read only](compatibility.md#newer-firmware) and changes nothing. |
| `500` | `request_not_consumed`, `failed` | The request file could not be deleted, or the system refused (the `detail` says which call and its result). |
| `503` | `no_network`, `applet_mode` | No network, or not enough memory: PlayGuard was started from the Album instead of over a game. |

There is no `202`: PlayGuard is on screen while it works, so by the time it
answers, the parent has typed the PIN or not.

### Each request acts once

PlayGuard keeps the `id` and response of the last 32 requests in
`sdmc:/config/playguard/api/handled.json`. A request with an `id` already
there is not done again; its response is written again. A crash in the middle
of `POST /playtime/extra` and a relaunch with the same request therefore
cannot give the extra time twice.

## Routes

The group in brackets is the one the parent sets (*Preferences › Other
homebrews*); a route needs *Read* for `GET` and `OPEN`, *Read and write* for
`POST`. **PIN** means the route goes through the PIN screen, whatever the
lock setting.

### Discovery

| Method | Path | | Answer |
|---|---|---|---|
| `GET` | `/v1` | always answers | PlayGuard's version, the API versions, whether the API is on, each group's permission, the routes it opens, and whether the firmware is read only. |

`GET /v1` answers even while the API is off (with `"enabled": false` and no
routes), so a caller can tell the parent what to turn on.

### Clock [clock]

| Method | Path | | Answer |
|---|---|---|---|
| `GET` | `/v1/clock` | | The offset of the network clock, measured with NTP, without changing it: `offset_ms`, `server`, `measured_at`. |
| `POST` | `/v1/clock/sync` | | Sets the network clock from NTP, as *Console › Network clock*: `offset_ms_before`, `offset_ms_after`, `server`. Recorded in the history. |
| `OPEN` | `/v1/clock` | | The *Network clock* screen. |

### Status [status]

| Method | Path | | Answer |
|---|---|---|---|
| `GET` | `/v1/status` | | Whether controls are set, the limit and time played today, the time left, whether play is suspended, a temporary unlock. |

### Activity [activity]

| Method | Path | | Answer |
|---|---|---|---|
| `GET` | `/v1/activity?day=YYYY-MM-DD` | PIN | Time per game and per account that day (today by default). |
| `GET` | `/v1/games/{program_id}/activity?from=…&to=…` | PIN | One game's time per day and per account. |
| `OPEN` | `/v1/games/{program_id}` | | The game's page: its activity and what PlayGuard knows of it. |

What a child plays is the family's business, so reading it takes the PIN even
when the group is set to *Read*. Opening the game's page does not: it shows what
the app shows, behind its own lock (`pin_lock`).

### Games [games]

| Method | Path | | Answer |
|---|---|---|---|
| `OPEN` | `/v1/games/{program_id}/check` | | Checks that one game is installed whole, as *Console › Check the installed games*, and shows the result with its repairs. |
| `GET` | `/v1/games/{program_id}/check` | | The same check without the screen: `ok`, and the problems found (`missing_content`, `ticket`, …). Repairs stay on the screen. |

`{program_id}` is the 16 hex digits of the game's program ID.

### Play time (later)

`POST /v1/playtime/extra` (`{"minutes": 30}`) and `POST /v1/playtime/stop` are
planned once the routes above have shipped. Both need *Read and write* on **play time**, and always take the PIN.

## Reading without launching PlayGuard

When the **status** group is set to *Read*, PlayGuard writes
`sdmc:/config/playguard/api/snapshot.json` each time it closes:

```json
{
  "api": "v1",
  "written_at": "2026-10-20T18:42:07+02:00",
  "status": { "limit_min": 90, "played_min": 55, "left_min": 35, "suspended": false },
  "clock": { "offset_ms": 12, "measured_at": "2026-10-20T18:40:51+02:00" }
}
```

It is only as fresh as `written_at`: the timer keeps counting after PlayGuard
closes. No activity goes in it. A homebrew that needs the live time left can
also read it from the system itself (`pctl`), as PlayGuard does.

## Out of scope for v1

- **Starting the game afterwards.** `OPEN /v1/games/{program_id}` returns to
  `return_to`, not to the game; the caller can start it.
- **A Tesla overlay** calling PlayGuard without leaving the game: that needs a
  separate program, not a launch.
- **A resident service** (a sysmodule answering IPC): it would run all the
  time, which PlayGuard avoids.

## In the code (when built)

- `source/action/api_logic.cpp`: the router, pure (a request in, a response
  and an action out), tested on the host (`tests/api`).
- `source/action/api_flow.cpp`: reading the request, running the existing
  flows (`clock_flow`, `game_verify_flow`, …), writing the response,
  `envSetNextLoad()` for `return_to`.
- `docs/api.schema.json`: the request and response schemas the tests check
  the router against.
- `playguard_api.h`: a single-header C helper for callers (write a request,
  launch PlayGuard, read the response).
