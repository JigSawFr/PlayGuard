# Roadmap

Where PlayGuard is going, and the ideas waiting for a decision. Nothing here
is a promise or a date: it is a list to pick from, most useful first in each
section. To propose something, open a
[feature request](https://github.com/JigSawFr/PlayGuard/issues/new?template=4-feature.yml);
to take something on, say so in an issue first.

The goal stays the same: everything the Nintendo Switch Parental Controls
phone app sets, on the console, offline, with nothing running in the
background. What that means feature by feature is in
[docs/companion-app.md](docs/companion-app.md); what is known of the console's
side is in [docs/parental-controls.md](docs/parental-controls.md).

## Waiting on data from a console

These need a finding before any code: most of them a play-timer block
compared before and after a change made from the phone app, on a console
still linked to it: *Security › Before unlinking: help decode…* (shown
while linked, for everyone) or *About › Help & diagnostics ›
Developer › Compare the play-timer block*. A report attached to an issue
is the most useful contribution right now.

- **Exceptions for specific software**: show the games on the
  free-communication list by name, then add and remove them, once the list's
  entries are decoded.
  ([details](docs/companion-app.md#3-exceptions-for-specific-software))

## Ready to build

No unknown left; only code.

- **The time play is allowed again, per day of the week.** The bedtime alarm
  is already set by day (Monday to Friday, the weekend, one day); the time
  play is allowed again is still one for every day. Left: the code for a
  start time per day, and a console check that it rings each day's own.
  ([details](docs/companion-app.md#2-bedtime-and-start-time-per-day-of-the-week))
- **A month in Activity**: per game, per account and per day over a month,
  with the same export, the offline counterpart of the phone app's monthly
  report. ([details](docs/companion-app.md#5-monthly-report))

## To investigate

- **Game categories and suggestions** in Activity: the console's control
  data names no genre, so "the category played most" or suggestions of new
  games would need an outside game database (downloaded, kept up to date).
  Worth it only if it stays optional and offline-friendly.
- **History of PIN entries** made from the HOME menu, as the phone app shows
  since 2.5.0: does the console log them anywhere PlayGuard can read?
  ([details](docs/companion-app.md#4-history-of-pin-entries))
- The open questions on the play timer (when the time spent resets on its
  own, whether it counts while temporarily unlocked, command 1406's error,
  1460's input…): [docs/parental-controls.md](docs/parental-controls.md#still-open).
  Each answer makes PlayGuard's figures and warnings more exact.

## Ongoing

- **Translations by native speakers** of the 15 catalogues (see
  [Translating PlayGuard](docs/contributing/translating.md)).
- **New firmware**: PlayGuard opens read-only on an unknown firmware until a
  release supports it; each new firmware needs a check of the command table.

## Horizon

Further out, and bigger than a feature: PlayGuard drives the console's own
parental controls instead of replacing them, so the console can become one
piece of a home that already automates the rest. The first step is in:
PlayGuard links to an MQTT broker and Home Assistant (*Console › Remote
access*, [guide](docs/home-assistant.md)): the console pushes its state and
statistics and takes orders (extra time, no more play today, limits, a
profile, the lock), the optional agent sysmodule keeps the link up while
PlayGuard is closed, and the
[PlayGuard integration](https://github.com/JigSawFr/playguard-ha) adds the
game names, each account's play time, long-term statistics and blueprints.
The design and its reasons are in [docs/sync-design.md](docs/sync-design.md);
the wire contract is in [docs/sync-protocol.md](docs/sync-protocol.md).

What it already allows, through Home Assistant:

- **Switch profiles on their own** (*School week* / *Summer holidays*) from a
  calendar, school holidays, the weather (more play on a rainy day) or any
  other data at hand: an automation that sets the console's *Profile*.
- **Chores that earn play time**: the integration's *Extra time when…*
  blueprint adds minutes when a helper is turned on, a button pressed or a
  tag scanned; an automation can lower *Max screentime today* as a penalty.
- **Notifications that can be answered**: "10 minutes left" (the
  `time_warning` event), with a button that adds a quarter of an hour.
- **Several consoles at home**, each a device of its own, seen and set from
  one place.

Still to explore:

- **Live figures elsewhere**: time left today on a dashboard, on a
  Busy Bar, on a phone. On an iPhone, a Home Assistant automation can start a
  **Live Activity** when play begins, through an app such as
  [Aivi](https://docs.getaivi.app/) (a Home Assistant blueprint, or a plain
  HTTP call): the time left counts down on the lock screen and in the Dynamic
  Island, on every parent's phone at once, and ends with the session. A tap
  on it opens the console's view in Home Assistant, where a parent adds time
  or ends play for today, and the order goes back to the console: the loop is
  closed. Between sessions, a widget can keep today's play time on the Home
  Screen or the Apple Watch, with buttons for the same orders.
- **A child asking for more time** from the console, the request answered
  from a parent's phone.

What it must keep: the limit, the PIN, the warnings and the suspension stay
the console's own, so they keep working when the link, the network or the
broker is down. And it holds to the rules the console sets: `pctl:a` takes a
single session, so the link opens it for each call and releases it at once,
as PlayGuard does, and an order writes the play timer only through the same
gate as a press on the console (parental controls unlocked for the write,
then locked again). The agent is the one exception to "nothing in the
background", an optional one: it never takes over the controls.

## Ideas without a decision

Asked for by parents (mostly in the phone app's store reviews). Each one runs
into a limit of the console's play timer, which PlayGuard drives but does not
replace; they stay here until someone finds a way that keeps "nothing in the
background".

| Idea | What stands in the way |
|---|---|
| Two play windows a day (morning, afternoon) | The timer has one bedtime and one "allowed again" time per day |
| A compulsory break after a stretch of play | The timer has no such mode; enforcing it needs something running while the game runs |
| A weekly budget instead of daily limits | The timer counts per day; moving unused time between days needs PlayGuard to run every day |
| A message shown on the console to the child | No system call for it outside a running homebrew |

## Not planned

| | Why |
|---|---|
| A limit per user account | The console's timer has one limit for the whole console. A replacement sysmodule does this ([compared](docs/compatibility.md#playguard-or-a-replacement-sysmodule)) |
| Anything running in the background to enforce the rules | PlayGuard only changes the console's own settings, which enforce them. The optional recovery sysmodule acts at boot, and only on request; the optional agent only holds the remote link, and carries out its orders, while PlayGuard is closed |
| Remote control through Nintendo's servers, a second parent's Nintendo Account | PlayGuard runs on the console, offline; remote control goes through your own MQTT broker instead ([Home Assistant](docs/home-assistant.md), the [horizon](#horizon)) |
| GameChat settings | Nintendo Switch 2 only, which Atmosphère does not run on |
| Purchase restrictions | Nintendo Account settings, on Nintendo's servers |
