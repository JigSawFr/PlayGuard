<div align="center">

<img src="images/store/banner.png" alt="PlayGuard" width="100%">

### Nintendo Switch parental controls, right on the console

**No&nbsp;phone&nbsp;app · No&nbsp;Nintendo&nbsp;account · No&nbsp;internet**

[![build](https://github.com/JigSawFr/PlayGuard/actions/workflows/build.yml/badge.svg)](https://github.com/JigSawFr/PlayGuard/actions/workflows/build.yml)
[![latest release](https://img.shields.io/badge/dynamic/json?url=https%3A%2F%2Fraw.githubusercontent.com%2FJigSawFr%2FPlayGuard%2Fmain%2F.release-please-manifest.json&query=%24%5B%27.%27%5D&prefix=v&label=release&color=blue)](https://github.com/JigSawFr/PlayGuard/releases/latest)
[![downloads](https://img.shields.io/github/downloads/JigSawFr/PlayGuard/total)](https://github.com/JigSawFr/PlayGuard/releases)
[![license: GPLv3](https://img.shields.io/badge/license-GPLv3-blue.svg)](LICENSE)

[**Download**](https://github.com/JigSawFr/PlayGuard/releases/latest) ·
[**Try&nbsp;it&nbsp;in&nbsp;your&nbsp;browser**](https://playguard.link/simulator/) ·
[**Documentation**](https://playguard.link/) ·
[**Locked&nbsp;out?**](https://playguard.link/docs/troubleshooting/#locked-out)

<img src="images/video/playguard-tour.gif" alt="PlayGuard in 90 seconds" width="100%">

</div>

---

PlayGuard brings the settings of Nintendo's [Parental Controls](https://apps.apple.com/app/id1190074407) phone app (the companion app) onto a modded console. It drives the console's **own** controls — the limit, the PIN, the warnings, the suspension — so **nothing runs in the background**.

> [!WARNING]
> **Requires custom firmware (Atmosphère).** PlayGuard uses the restricted `pctl` service, some of it through `*ForDebug` commands. **Use at your own risk.**

## ✨ Features

- ⏱️ **Play-time limit** — every day or one per day, extra time or *no more play today* in one press, *Block all games* in one switch.
- 📊 **Activity** — time per game and per account, charts, exports.
- 🔒 **Restrictions and PIN** — age rating, temporary unlock, PlayGuard itself behind the PIN.
- 🕒 **Console** — a network clock that counts right offline, the installed games checked and repaired, homebrew on the HOME menu behind the PIN.
- ↩️ **History, backups, notifications** — every change logged and undoable.
- 🌍 **Everywhere** — every console language, light and dark themes.

→ Everything else: [**features**](https://playguard.link/docs/features/).

<details>
<summary><b>📸 Screenshots</b></summary>
<br>

<p align="center">
  <img src="images/screenshots/dashboard.png" alt="Today" width="49%">
  <img src="images/screenshots/play_timer.png" alt="Play timer" width="49%">
  <br><sub><b>Today</b> · <b>Play timer</b></sub>
</p>

<p align="center">
  <img src="images/screenshots/activity.png" alt="Activity" width="49%">
  <img src="images/screenshots/restrictions.png" alt="Restrictions" width="49%">
  <br><sub><b>Activity</b> · <b>Restrictions</b></sub>
</p>

<p align="center">
  <img src="images/screenshots/security.png" alt="Security" width="49%">
  <img src="images/screenshots/console.png" alt="Console" width="49%">
  <br><sub><b>Security</b> · <b>Console</b></sub>
</p>

</details>

## 🎮 Try it first

The real app, against a simulated console, in your browser:

[Unlocked for now](https://playguard.link/simulator/?unlocked=1) · [Time's up](https://playguard.link/simulator/?restricted=1) · [First start, no PIN](https://playguard.link/simulator/?not_set_up=1) · [On the TV, larger text](https://playguard.link/simulator/?docked=1)

## 📦 Install

1. On the console, open the **Homebrew App Store** or **sphaira**'s App Store, search for *PlayGuard* and install it.
   By hand instead: download `playguard.zip` from the [**latest release**](https://github.com/JigSawFr/PlayGuard/releases/latest) and extract it to the **root of the SD card**.
2. Launch PlayGuard **over a game**, with a **parent's user**, and follow *First steps*.

> [!TIP]
> Still linked to the companion app? Use *Security › Unlink the companion app*, or its next sync overwrites what you set.

**Compatibility:** tested on firmware 22.0.0, 22.1.0, 22.5.0 and 23.0.1 (Atmosphère 1.11–1.12); 23.0.0 and 21.x should work, untested; older firmware, everything but the play timer. Details: [compatibility](https://playguard.link/docs/compatibility/). **No Switch 2.**

## 📖 Documentation

**[playguard.link](https://playguard.link/)**, in the console's 15 languages:

- 🚀 [Getting started](https://playguard.link/docs/getting-started/)
- 📦 [Install, update, remove](https://playguard.link/docs/install/)
- 🛠️ [Troubleshooting](https://playguard.link/docs/troubleshooting/) · [Locked out?](https://playguard.link/docs/troubleshooting/#locked-out)

The Markdown sources live in [`docs/`](docs/), with the reference pages: [`config.json`](docs/config.md) · [how parental controls work](docs/parental-controls.md) · [companion app](docs/companion-app.md) · [recovery sysmodule](sysmodule/README.md).

## 🤝 Contributing

[CONTRIBUTING.md](CONTRIBUTING.md) · [Roadmap](ROADMAP.md) · [Changelog](CHANGELOG.md) · [Code of conduct](CODE_OF_CONDUCT.md) · [Security](SECURITY.md)

AI coding assistants help with development; every change is reviewed, unit-tested and checked on a real console before release.

## 📜 License and credits

**GPLv3**, see [`LICENSE`](LICENSE). Maintained by [JigSawFr](https://github.com/JigSawFr).

<details>
<summary>Credits</summary>
<br>

- A fork of [Pctl Manager](https://github.com/tailiang2008) by Taylor; fw 22.5 fixes and NTP sync from [anbingxi/NX-Pctl-Manager](https://github.com/anbingxi/NX-Pctl-Manager/tree/diag/fw22-5-readonly).
- Built with [borealis](https://github.com/xfangfang/borealis) (Apache 2.0), [QR Code generator](https://github.com/nayuki/QR-Code-generator) (MIT), [Twemoji](https://github.com/jdecked/twemoji) (CC-BY 4.0) and the [Sora](https://github.com/sora-xor/sora-font) typeface (OFL 1.1).
- Command reference from [switchbrew](https://switchbrew.org/wiki/Parental_Control_services).
- The installed games check and its repairs follow [Goldleaf](https://github.com/XorTroll/Goldleaf) and [sphaira](https://github.com/ITotalJustice/sphaira) (GPLv3), with the ns and es command numbers of [libnx-ext](https://github.com/XorTroll/libnx-ext) (MIT).
- The NRO forwarders are built as sphaira builds them, their program sphaira's hbl, after [nx-hbloader](https://github.com/switchbrew/nx-hbloader) (ISC).

</details>

<sub>PlayGuard is not affiliated with or endorsed by Nintendo. Nintendo Switch is a trademark of Nintendo.</sub>
