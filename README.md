# PlayGuard

![PlayGuard](images/store/banner.png)

[![build](https://github.com/JigSawFr/PlayGuard/actions/workflows/build.yml/badge.svg)](https://github.com/JigSawFr/PlayGuard/actions/workflows/build.yml)
[![latest release](https://img.shields.io/github/v/release/JigSawFr/PlayGuard)](https://github.com/JigSawFr/PlayGuard/releases/latest)
[![downloads](https://img.shields.io/github/downloads/JigSawFr/PlayGuard/total)](https://github.com/JigSawFr/PlayGuard/releases)
[![license: GPLv3](https://img.shields.io/badge/license-GPLv3-blue.svg)](LICENSE)

**English** · [Français](README.fr.md)

**Nintendo Switch parental controls, right on the console — no phone app, no Nintendo account, no internet.**

PlayGuard is a homebrew app that brings the settings of the [Nintendo Switch Parental Controls](https://apps.apple.com/app/id1190074407) phone app onto the console itself, for offline use: daily play-time limits, restrictions, PIN, network clock, play activity — and a way back in when you are locked out. What it covers of the phone app, and what is still missing: [docs/companion-app.md](docs/companion-app.md).

![Overview](images/screenshots/dashboard.png)

> [!WARNING]
> **Requires custom firmware (Atmosphère).** PlayGuard talks to the restricted system `pctl` service, so it only runs on a modded console. It does not bypass any account or online check, and some of the commands it uses are `*ForDebug` ones. **Use at your own risk.**

## Contents

- [Why PlayGuard](#why-playguard) · [Highlights](#highlights) · [Compatibility](#compatibility) · [Install](#install) · [Quick start](#quick-start)
- [Locked out?](#locked-out-second-hand-console-forgotten-pin) · [Documentation](#documentation) · [Contributing](#contributing) · [License and credits](#license-and-credits)

## Why PlayGuard

Children love video games, and video games mean screens: a limit is part of looking after them. On an unmodified Switch, Nintendo's phone app took care of it. Once the console runs custom firmware, a whole new world opens up — and parental control goes out of the window, because the phone app no longer reaches the console. Knowing how long they played, setting a limit, getting the game stopped without a fight or one more "five more minutes" became hard.

The few tools that existed did not do the job: activity reports that were barely maintained and not very detailed, parental controls that were rough and unfinished. And I did not want a custom replacement either, or a sysmodule always running in the background. The idea was to **reuse the console's own parental controls as much as possible**, with everything they already do — the limit, the PIN, the warnings, the suspension — and to bring their settings back onto the console. That is PlayGuard.

Driving the console's own controls also opens the door to much more: reporting play time to a home server, taking orders from it, Home Assistant, automations. Where that could go is in the [roadmap's horizon](ROADMAP.md#horizon).

## Highlights

- ⏱️ **Daily play-time limit** — the same every day or one per day, edited on a week chart; saved **profiles** (*School week*, *Summer holidays* …); **extra time today** and **no more play today** in one press.
- 📊 **Activity** — time per game today, over 7 days and in all, per user account, with charts, a summary per period, games to rediscover and **export** to CSV, JSON, XLSX or PDF.
- 🔒 **Restrictions and PIN** — restriction level, age rating and rating organisation, set / show the PIN, unlock temporarily, a one-switch **console lock**, and an optional PIN to open PlayGuard itself.
- 🕒 **Network clock** — measure against public NTP servers and set it, so the play timer counts correctly on a console that never reaches Nintendo.
- 📱 **Companion app** — see whether the phone app is linked and **unlink** it, even for a second-hand console.
- ↩️ **Change history, backups and undo** — every change PlayGuard makes is logged and can be reverted; settings can be backed up to the SD card.
- 🌍 **Every console language** — 15 catalogs, light and dark themes.
- 🛡️ **Safe writes** — the play timer is never written while it counts down, and no background process runs while a child plays.

| Play timer | Per-day limits |
|---|---|
| ![Play timer](images/screenshots/play_timer.png) | ![Per-day limits](images/screenshots/per_day.png) |
| **Activity** | **A game** |
| ![Activity](images/screenshots/activity.png) | ![A game](images/screenshots/activity_game.png) |
| **Restrictions** | **Network clock** |
| ![Restrictions](images/screenshots/restrictions.png) | ![Network clock](images/screenshots/clock.png) |
| **Security & app** | **Preferences** |
| ![Security & app](images/screenshots/security.png) | ![Preferences](images/screenshots/preferences.png) |

## Compatibility

| | Supported | Notes |
|---|---|---|
| **Firmware** | **21.0.0 → 22.5.0** | The play-time limit layout (0x44 bytes) exists since 21.0.0; below that, every tab works except the play timer. 23.x goes through the [newer firmware](docs/compatibility.md#newer-firmware) screen until it is tested on a console. |
| **Atmosphère** | **1.11.x → 1.12.0** | 1.12.0 adds 23.0.0 support. The app shows the detected version. |
| **Launchers** | hbmenu, **sphaira**, **Homebrew App Store** | Launch it over a game (title override), with a parent's user: the console counts PlayGuard's time as that game's ([details](docs/compatibility.md#supported-versions)). |
| **Tested on hardware** | 22.0.0, 22.1.0, 22.5.0 | 23.0.1 / 1.12.0 has the same commands but was not run on a console yet: PlayGuard asks first (read-only until you choose) — reports are welcome. |
| **Nintendo Switch 2** | Not supported | There is no Atmosphère for it. |

Newer firmware, emuMMC and sysMMC, ban risk, consoles without Internet, and how PlayGuard compares with a replacement sysmodule: **[docs/compatibility.md](docs/compatibility.md)**.

## Install

Pick one:

| Channel | How |
|---|---|
| **Homebrew App Store** / **sphaira's App Store** (same catalogue) | Search for *PlayGuard* once the listing is approved. New releases are picked up automatically. |
| **sphaira › GitHub** | The release zip already contains the entry (`/config/sphaira/github/playguard.json`): after a first install, update from *GitHub* in sphaira. |
| **Manual** | Download `playguard.zip` from the [latest release](https://github.com/JigSawFr/PlayGuard/releases/latest) and extract it to the **root** of the SD card. The app lands in `sd:/switch/playguard/`, its files in `sd:/config/playguard/`. |

The optional **recovery sysmodule** (`playguard-rescue.zip`) is a separate download — see [Locked out?](#locked-out-second-hand-console-forgotten-pin).

The files PlayGuard writes on the SD card: [docs/install.md](docs/install.md#files-on-the-sd-card).

## Quick start

1. **Parental controls not set up yet?** Open PlayGuard: the *First steps* guide sets the PIN (system PIN screen), the daily limit, and checks the network clock.
2. **Paired with the phone app?** *Security & app › Unlink the companion app* — otherwise its next sync overwrites what you set here.
3. Set the limit: *Play timer › Same limit every day* (or *A different limit for each day…*).
4. **Network clock inaccurate** (shown on the Overview)? Turn on *Synchronise Clock via Internet* in System Settings, then *Network clock › Measure* and *Set the network clock*. **No Internet?** Leave that setting off and check the console clock first: when no server answers, *Measure* offers *Set the network clock to the console clock*; turn the setting on only afterwards (before, the console clock jumps to the wrong network clock).

**Controls:** ↑/↓ move · Ⓐ confirm · Ⓑ back or cancel (twice on the sidebar to exit) · Ⓧ refresh · **+** saves per-day limits.

Every tab in detail, the safeguards around writing the limit, and what the console counts as play time: **[docs/features.md](docs/features.md)**.

## Locked out? (second-hand console, forgotten PIN)

PlayGuard only sees the parental controls of the system it runs on: **emuMMC and sysMMC each have their own** (a PIN removed on one is still there on the other). Run it on each one that needs fixing.

| Situation | What to do |
|---|---|
| **Second-hand console:** you know the PIN, but the previous owner's phone app is still linked (unlinking fails, or a factory reset asks for their account) | *Security & app › Unlink the companion app*, then, if you want no parental controls at all, *Delete all parental controls*. Both work offline, on emuMMC as on sysMMC. |
| **PIN forgotten** | *Show the PIN* asks for the PIN itself, so it cannot help here. With the recovery sysmodule installed (below), drop a `RESCUE` file and restart: the recovery screen then sets a new one (it never shows the old one). Without it, *Delete all parental controls* starts again (a settings backup is saved first; it never contains the PIN): with *Ask for the PIN* on (the default, *Before a change*), put the SD card in a computer and set `"pin_lock"` to `"off"` in `sd:/config/playguard/config.json` first. |
| **The play timer blocks everything (0-minute limit) and the PIN is forgotten** | PlayGuard itself cannot start then. Install the optional recovery sysmodule (`playguard-rescue.zip`) **beforehand**; when locked out, drop an empty `config/playguard/RESCUE` file on the SD card and boot — it unlocks the console so PlayGuard can open. See [`sysmodule/README.md`](sysmodule/README.md). |
| **Console not modded** | PlayGuard cannot help: it needs Atmosphère. Nintendo support's master-key procedure is the official way. |

Reporting a bug (diagnostic report, sending one online) and the developer mode: [docs/troubleshooting.md](docs/troubleshooting.md).

## Documentation

| Page | What is in it |
|---|---|
| [Features](docs/features.md) | Every tab, the controls, safety by design, what counts as play time |
| [Install](docs/install.md) | Install channels, the files written on the SD card |
| [Compatibility](docs/compatibility.md) | Firmware, Atmosphère, launchers, emuMMC / sysMMC, ban risk, PlayGuard vs a replacement sysmodule |
| [Troubleshooting](docs/troubleshooting.md) | Reporting a bug, the diagnostic report, developer mode |
| [Companion app](docs/companion-app.md) | What PlayGuard covers of Nintendo's phone app, and what is missing |
| [`config.json`](docs/config.md) | Every preference key and the state kept between launches |
| [How parental controls work](docs/parental-controls.md) | The `pctl` service, the play timer and the clocks, as measured on hardware |
| [Recovery sysmodule](sysmodule/README.md) | The optional `RESCUE` boot sysmodule |

## Contributing

Build instructions, the desktop simulator, the code layout, the release process and how to translate PlayGuard are in **[CONTRIBUTING.md](CONTRIBUTING.md)**. What comes next, and the ideas waiting for a decision: **[ROADMAP.md](ROADMAP.md)**.

**No code needed, two minutes:** if your console is still linked to the phone app, *Security & app › Before unlinking: help decode…* saves the play-timer block, you change one setting in the phone app ("alarm only", or a bedtime for one day), and PlayGuard lists what changed. Attach that file to an issue: it is the data the [roadmap](ROADMAP.md#waiting-on-data-from-a-console) waits for most.

Quick taste:

```sh
make test      # unit tests of the C service layer (no devkitPro needed)
make desktop   # the real UI on Linux, against a simulated console
make dist      # playguard.zip (devkitPro switch-dev)
```

Translations from native speakers are especially welcome — PlayGuard ships English, French (France, Canada), German, Spanish (Spain, Latin America), Italian, Dutch, Portuguese (Portugal, Brazil), Russian, Japanese, Korean and Chinese (simplified, traditional).

## License and credits

GPLv3 — see [`LICENSE`](LICENSE). Maintained by **[JigSawFr](https://github.com/JigSawFr)**.

- A fork of **Pctl Manager** by **Taylor** ([tailiang2008](https://github.com/tailiang2008)) (v2–v3): the original pctl service layer, play-timer write gate and borealis UI. Its history is in [CHANGELOG.md](CHANGELOG.md).
- UI: **[borealis](https://github.com/xfangfang/borealis)** (Apache 2.0), pinned at `extern/borealis/`.
- QR codes: **[QR Code generator](https://github.com/nayuki/QR-Code-generator)** by Project Nayuki (MIT), in `extern/qrcodegen/`.
- The release notes' icons: **[Twemoji](https://github.com/jdecked/twemoji)** graphics (CC-BY 4.0), in `resources/img/emoji/`.
- fw 22.5 diagnosis, session release and NTP synchronisation adapted from **[anbingxi/NX-Pctl-Manager](https://github.com/anbingxi/NX-Pctl-Manager/tree/diag/fw22-5-readonly)**.
- Command reference: [switchbrew — Parental Control services](https://switchbrew.org/wiki/Parental_Control_services).

> [!NOTE]
> **How PlayGuard is built.** AI coding assistants were used alongside development: writing and reviewing code, translations and documentation. PlayGuard's changes are driven, reviewed and validated by a professional developer, covered by the C unit tests and the desktop simulator, and tested on a real console (22.0.0, 22.1.0 and 22.5.0 / Atmosphère 1.11.x) before release.

PlayGuard is not affiliated with or endorsed by Nintendo. Nintendo Switch is a trademark of Nintendo.
