# PlayGuard

![PlayGuard](images/store/banner.png)

[![build](https://github.com/JigSawFr/PlayGuard/actions/workflows/build.yml/badge.svg)](https://github.com/JigSawFr/PlayGuard/actions/workflows/build.yml)
[![latest release](https://img.shields.io/badge/dynamic/json?url=https%3A%2F%2Fraw.githubusercontent.com%2FJigSawFr%2FPlayGuard%2Fmain%2F.release-please-manifest.json&query=%24%5B%27.%27%5D&prefix=v&label=release&color=blue)](https://github.com/JigSawFr/PlayGuard/releases/latest)
[![downloads](https://img.shields.io/github/downloads/JigSawFr/PlayGuard/total)](https://github.com/JigSawFr/PlayGuard/releases)
[![license: GPLv3](https://img.shields.io/badge/license-GPLv3-blue.svg)](LICENSE)

**Nintendo Switch parental controls, right on the console — no phone app, no Nintendo account, no internet.**

PlayGuard brings the settings of Nintendo's [Parental Controls](https://apps.apple.com/app/id1190074407) phone app onto a modded console. It drives the console's own controls (the limit, the PIN, the warnings, the suspension), so nothing runs in the background.

[![PlayGuard in 30 seconds](images/video/playguard-tour.gif)](images/video/playguard-tour.gif)

▶️ **[Try it in your browser](https://playguard.link/simulator/)**, the real app against a simulated console: [unlocked for now](https://playguard.link/simulator/?unlocked=1) · [time's up](https://playguard.link/simulator/?restricted=1) · [first start, no PIN](https://playguard.link/simulator/?not_set_up=1) · [on the TV, larger text](https://playguard.link/simulator/?docked=1).

> [!WARNING]
> **Requires custom firmware (Atmosphère).** PlayGuard uses the restricted `pctl` service, some of it through `*ForDebug` commands. **Use at your own risk.**

## Features

- ⏱️ **Play-time limit** — every day or one per day, extra time or no more play today in one press.
- 📊 **Activity** — time per game and per account, charts, exports.
- 🔒 **Restrictions and PIN** — age rating, temporary unlock, *Block all games* in one switch.
- 🕒 **Console** — a network clock that counts right offline, homebrew on the HOME menu behind the PIN.
- ↩️ **History, backups and notifications** — every change logged and undoable.

Every console language, light and dark themes. Everything else: [features](https://playguard.link/docs/features/).

## Install

> [!WARNING]
> PlayGuard is **not in the Homebrew App Store (nor sphaira's) yet**: its listing is still awaiting approval. Install it by hand for now.

Extract `playguard.zip` from the [latest release](https://github.com/JigSawFr/PlayGuard/releases/latest) to the root of the SD card. Launch it over a game, with a parent's user, and follow *First steps*. Still linked to Nintendo's phone app? *PIN › Unlink the companion app*, or its next sync overwrites what you set.

Firmware 22.0.0 → 23.0.1 tested (Atmosphère 1.11–1.12); older ones and the details: [compatibility](https://playguard.link/docs/compatibility/). No Switch 2.

## Documentation

**[playguard.link](https://playguard.link/)**, in the console's 15 languages: [getting started](https://playguard.link/docs/getting-started/) · [install](https://playguard.link/docs/install/) · [troubleshooting](https://playguard.link/docs/troubleshooting/) · [Locked out?](https://playguard.link/docs/troubleshooting/#locked-out)

The Markdown sources are in [`docs/`](docs/), with the reference pages: [`config.json`](docs/config.md) · [how parental controls work](docs/parental-controls.md) · [companion app](docs/companion-app.md) · [recovery sysmodule](sysmodule/README.md).

Contributing: [CONTRIBUTING.md](CONTRIBUTING.md) · [roadmap](ROADMAP.md) · [changelog](CHANGELOG.md)

## License and credits

GPLv3, see [`LICENSE`](LICENSE). Maintained by [JigSawFr](https://github.com/JigSawFr). A fork of [Pctl Manager](https://github.com/tailiang2008) by Taylor; fw 22.5 fixes and NTP sync from [anbingxi/NX-Pctl-Manager](https://github.com/anbingxi/NX-Pctl-Manager/tree/diag/fw22-5-readonly). Built with [borealis](https://github.com/xfangfang/borealis) (Apache 2.0), [QR Code generator](https://github.com/nayuki/QR-Code-generator) (MIT), [Twemoji](https://github.com/jdecked/twemoji) (CC-BY 4.0) and the [Sora](https://github.com/sora-xor/sora-font) typeface (OFL 1.1); command reference from [switchbrew](https://switchbrew.org/wiki/Parental_Control_services). The installed games check and its repairs follow [Goldleaf](https://github.com/XorTroll/Goldleaf) and [sphaira](https://github.com/ITotalJustice/sphaira) (GPLv3), with the ns and es command numbers of [libnx-ext](https://github.com/XorTroll/libnx-ext) (MIT). The NRO forwarders are built as sphaira builds them, their program sphaira's hbl, after [nx-hbloader](https://github.com/switchbrew/nx-hbloader) (ISC).

AI coding assistants help with development; every change is reviewed, unit-tested and checked on a real console before release.

PlayGuard is not affiliated with or endorsed by Nintendo. Nintendo Switch is a trademark of Nintendo.
