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

A PIN asked for by a request is the same PIN screen as anywhere else: a wrong
PIN counts towards the same lockout, so an app that opens the PIN screen again
and again gains no extra attempts.

### What the parent sees

*Preferences › Other homebrews* lists the last 20 calls: when, the `caller`,
the route, and the status (`200`, `403 permission_denied` …). **Turn it all
off** sets every group back to *None* in one press. Calls that changed
something (`POST`) are also in the change history, as every change is.

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
| `lang` | no | The language of the errors' `title`, as the console names it (`fr`, `en-US`, `ja` …). Default: the console's language. |
| `dry_run` | no | `true`: check the request and its permission, answer `200` (or the error it would get) with `"dry_run": true`, and do nothing. No PIN asked, no history, not kept in `handled.json`. |

The caller writes the request file **in one go**: to a temporary name, then
renamed to the name it passes. A file PlayGuard reads half written is not
JSON, gets `400 bad_request`, and is gone.

The request file is **deleted as soon as it is read**, before anything is
done, so a request left on the card is never replayed at the next start (if
it cannot be deleted, it is not done: `500 request_not_consumed`). A file over
16 KiB, not JSON, or without the required keys gets `400 bad_request`.

### Several requests in one launch

Each launch is a round trip, so a request file may hold up to 8 requests,
done in order:

```json
{
  "api": "v1",
  "id": "c4ca4238",
  "return_to": "sdmc:/switch/myapp/myapp.nro",
  "caller": "MyApp 1.2",
  "requests": [
    { "method": "POST", "path": "/clock/sync" },
    { "method": "GET", "path": "/status" }
  ]
}
```

- `requests` replaces `mode`, `method`, `path`, `body` and `dry_run` at the top;
  each entry takes those keys. `id`, `return_to`, `response` and `caller` stay
  at the top and apply to the whole file.
- Each entry needs its own permission; one refused does not stop the next,
  unless the file says `"stop_on_error": true` (the rest then get
  `424 not_done`).
- Only the last entry may be `open`: the screen is where the launch ends.
- The response has `"status": 200` (the file was read) and `responses`, one per
  entry in the same order, each with its own `status` and `body`.
- The file's `id` is the one that [acts once](#each-request-acts-once): a
  relaunch with it does none of the entries again.

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

A response that carries the child's play (the **activity** routes) is a file
anything can read, so it does not stay on the card:

- the caller deletes it once read (`playguard_api.h` does);
- at its next start, PlayGuard deletes any such response still there (it
  remembers where it wrote them, in `handled.json`), and logs that it did.

Other responses (the clock, the status) may stay; the next one replaces them.

PlayGuard launches `return_to` with two arguments, so the caller knows it is
coming back from PlayGuard and where to look:

```
sdmc:/switch/myapp/myapp.nro --playguard-response sdmc:/switch/myapp/pg_response.json --playguard-status 200
```

`--playguard-status` is the response's `status` (for several requests, the
first status that is not `200`, or `200`). The file is still the answer; the
arguments only point to it.

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
| `404` | `no_route`, `no_game`, `no_measure` | Unknown or planned path, no such game installed, or no clock measure to give from cache. |
| `422` | `bad_body` | A parameter is missing or out of range. |
| `423` | `read_only` | The firmware is newer than PlayGuard supports: it [opens read only](compatibility.md#newer-firmware) and changes nothing. |
| `424` | `not_done` | Several requests with `stop_on_error`: an earlier one failed. |
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
lock setting. **Since** is the API version a route came with: the first
release (v1.0) answers `GET /v1` and the two clock routes, and *planned* routes get
`404 no_route` until the changelog says otherwise (`GET /v1` lists the routes
a build answers). Everything else on this page (the request file, several
requests, `dry_run`, the response and the relaunch arguments, the
permissions) comes with the first release.

### Discovery

| Method | Path | | Answer | Since |
|---|---|---|---|---|
| `GET` | `/v1` | always answers | PlayGuard's version, the API versions, whether the API is on, each group's permission, the routes it opens, and whether the firmware is read only. | v1.0 |

`GET /v1` answers even while the API is off (with `"enabled": false` and no
routes), so a caller can tell the parent what to turn on.

### Clock [clock]

| Method | Path | | Answer | Since |
|---|---|---|---|---|
| `GET` | `/v1/clock` | | The offset of the network clock, measured with NTP, without changing it: `offset_ms`, `server`, `measured_at`, `"cached": false`. With `?cached=1`, no network: the last offset PlayGuard measured, with its `measured_at` and `"cached": true` (`404 no_measure` if it never measured one). `?timeout_ms=` as for the sync. | v1.0 |
| `POST` | `/v1/clock/sync` | | Sets the network clock from NTP, as *Console › Network clock*: `offset_ms_before`, `offset_ms_after`, `server`. Recorded in the history. The body may set `timeout_ms` (1000 to 30000, default 10000): the longest the NTP servers are waited for in all. | v1.0 |
| `OPEN` | `/v1/clock` | | The *Network clock* screen. | planned |

### Status [status]

| Method | Path | | Answer | Since |
|---|---|---|---|---|
| `GET` | `/v1/status` | | Whether controls are set, the limit and time played today, the time left, whether play is suspended, a temporary unlock; `next_reset_at`, when the day's count starts again; `bedtime`, when play stops and starts again tonight, if the alarm is set. | planned |

### Activity [activity]

| Method | Path | | Answer | Since |
|---|---|---|---|---|
| `GET` | `/v1/activity?day=YYYY-MM-DD` | PIN | Time per game and per account that day (today by default). | planned |
| `GET` | `/v1/games/{program_id}/activity?from=…&to=…` | PIN | One game's time per day and per account. | planned |
| `OPEN` | `/v1/games/{program_id}` | | The game's page: its activity and what PlayGuard knows of it. | planned |

What a child plays is the family's business, so reading it takes the PIN even
when the group is set to *Read*. Opening the game's page does not: it shows what
the app shows, behind its own lock (`pin_lock`).

### Games [games]

| Method | Path | | Answer | Since |
|---|---|---|---|---|
| `OPEN` | `/v1/games/{program_id}/check` | | Checks that one game is installed whole, as *Console › Check the installed games*, and shows the result with its repairs. | planned |
| `GET` | `/v1/games/{program_id}/check` | | The same check without the screen: `ok`, and the problems found (`missing_content`, `ticket`, …). Repairs stay on the screen. | planned |

`{program_id}` is the 16 hex digits of the game's program ID.

### Play time (later)

`POST /v1/playtime/extra` (`{"minutes": 30}`) and `POST /v1/playtime/stop` are
planned once the routes above have shipped. Both need *Read and write* on **play time**, and always take the PIN.

## Reading without launching PlayGuard

*Planned, with `GET /v1/status`.* When the **status** group is set to *Read*, PlayGuard writes
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

## Versions

- **Within v1, only additions**: new routes, new keys in a body, new `code`
  values. A caller ignores keys it does not know; PlayGuard ignores keys it
  does not know in a request.
- **Anything that would break a caller** (a key removed or renamed, a meaning
  changed, a stricter check) is `v2`, under `/v2/…` and `"api": "v2"`.
  PlayGuard keeps answering `v1` for at least one year after `v2` ships, and
  `GET /v1` lists the versions it answers.
- The changelog says which PlayGuard version added each route.

## Trying it

*Both planned, soon after the first routes.*

- **In the browser.** The [simulator](https://playguard.link/simulator/)
  takes the short form as a parameter, against its simulated console, and
  shows the response it would write:
  `https://playguard.link/simulator/?api=POST%20/v1/clock/sync` (add
  `&api_permissions=clock:read_write` to set the permissions, which are
  *None* there too by default). The site's smoke test plays every route this
  way, so the simulator and this page stay in step.
- **On a console.** `examples/api-caller/` is a small homebrew, built by CI,
  that calls each route with `playguard_api.h` and shows the response. Use
  `dry_run` first: it says whether the permission is there without doing
  anything.

## Out of scope for v1

- **Starting the game afterwards.** `OPEN /v1/games/{program_id}` returns to
  `return_to`, not to the game; the caller can start it.
- **A Tesla overlay** calling PlayGuard without leaving the game: that needs a
  separate program, not a launch.
- **A resident service** (a sysmodule answering IPC): it would run all the
  time, which PlayGuard avoids.
- **Permissions per app.** What names the caller (`caller`, the path of
  `return_to`) is written by the caller, so anything can claim to be anything:
  a permission tied to it would protect nothing. The groups are the barrier.
- **An events file** (the limit changed, the clock was set) for apps to
  follow: handy, but it would leave on the card what happens to the child's
  play, readable by anything. Not without a real need.

## In the code (when built)

- `source/action/api_logic.cpp`: the router, pure (a request in, a response
  and an action out), tested on the host (`tests/api`).
- `source/action/api_flow.cpp`: reading the request, running the existing
  flows (`clock_flow`, `game_verify_flow`, …), writing the response,
  `envSetNextLoad()` for `return_to`.
- `docs/api.schema.json`: the request and response schemas the tests check
  the router against.
- `playguard_api.h`: a single-header C helper for callers (write a request,
  launch PlayGuard, read the response, read `--playguard-response`).
- `examples/api-caller/`: the example homebrew, built by CI.
- `web/`: the simulator's `?api=` parameter, and the routes in its smoke test.
