# PlayGuard

![PlayGuard](images/store/banner.png)

[![build](https://github.com/JigSawFr/PlayGuard/actions/workflows/build.yml/badge.svg)](https://github.com/JigSawFr/PlayGuard/actions/workflows/build.yml)
[![latest release](https://img.shields.io/github/v/release/JigSawFr/PlayGuard)](https://github.com/JigSawFr/PlayGuard/releases/latest)
[![downloads](https://img.shields.io/github/downloads/JigSawFr/PlayGuard/total)](https://github.com/JigSawFr/PlayGuard/releases)
[![license: GPLv3](https://img.shields.io/badge/license-GPLv3-blue.svg)](LICENSE)

**English** · [Français](README.fr.md)

**Nintendo Switch parental controls, right on the console — no phone app, no Nintendo account, no internet.**

PlayGuard brings the settings of Nintendo's [Parental Controls](https://apps.apple.com/app/id1190074407) phone app onto a modded console. It drives the console's own controls (the limit, the PIN, the warnings, the suspension), so nothing runs in the background.

![Overview](images/screenshots/dashboard.png)

> [!WARNING]
> **Requires custom firmware (Atmosphère).** PlayGuard uses the restricted `pctl` service, some of it through `*ForDebug` commands. **Use at your own risk.**

## Features

- ⏱️ **Play-time limit** — the same every day or one per day, profiles, extra time or no more play today in one press.
- 📊 **Activity** — time per game and per account, charts, export to CSV, JSON, XLSX or PDF.
- 🔒 **Restrictions and PIN** — restriction level, age rating, PIN, temporary unlock, a one-switch console lock.
- 🕒 **Network clock** — set from public NTP servers, so the timer counts right on a console that never reaches Nintendo.
- 📱 **Companion app** — unlink it, even on a second-hand console.
- ↩️ **History and backups** — every change is logged and can be undone.
- 🌍 Every console language, light and dark themes.

## Compatibility

- **Firmware:** 21.0.0 → 22.5.0
- **Atmosphère:** 1.11.x → 1.12.0
- **Launchers:** hbmenu, sphaira, Homebrew App Store

A newer firmware opens read-only until a release supports it. No Switch 2 support (there is no Atmosphère for it). Details: [compatibility](docs/compatibility.md).

## Install

Search for *PlayGuard* in the Homebrew App Store or sphaira, or extract `playguard.zip` from the [latest release](https://github.com/JigSawFr/PlayGuard/releases/latest) to the root of the SD card. Launch it over a game, with a parent's user.

## Quick start

1. Open PlayGuard: the *First steps* guide sets the PIN and the daily limit.
2. Still linked to the phone app? *Security & app › Unlink the companion app*, or its next sync overwrites what you set.
3. Network clock inaccurate? *Network clock › Measure*, then set it. Without Internet, read [this first](docs/compatibility.md#supported-versions).

Forgotten PIN or second-hand console: [Locked out?](docs/troubleshooting.md#locked-out)

## Documentation

- **User guide:** [features](docs/features.md) · [install](docs/install.md) · [compatibility](docs/compatibility.md) · [troubleshooting](docs/troubleshooting.md)
- **Reference:** [companion app](docs/companion-app.md) · [`config.json`](docs/config.md) · [how parental controls work](docs/parental-controls.md) · [recovery sysmodule](sysmodule/README.md)
- **Contributing:** [CONTRIBUTING.md](CONTRIBUTING.md) · [roadmap](ROADMAP.md) · [changelog](CHANGELOG.md)

## License and credits

GPLv3, see [`LICENSE`](LICENSE). Maintained by [JigSawFr](https://github.com/JigSawFr). A fork of [Pctl Manager](https://github.com/tailiang2008) by Taylor; fw 22.5 fixes and NTP sync from [anbingxi/NX-Pctl-Manager](https://github.com/anbingxi/NX-Pctl-Manager/tree/diag/fw22-5-readonly). Built with [borealis](https://github.com/xfangfang/borealis) (Apache 2.0), [QR Code generator](https://github.com/nayuki/QR-Code-generator) (MIT) and [Twemoji](https://github.com/jdecked/twemoji) (CC-BY 4.0); command reference from [switchbrew](https://switchbrew.org/wiki/Parental_Control_services).

AI coding assistants help with development; every change is reviewed, unit-tested and checked on a real console before release.

PlayGuard is not affiliated with or endorsed by Nintendo. Nintendo Switch is a trademark of Nintendo.
