# Contributing to PlayGuard

Thanks for helping! This page is the map; each subject has a page of its own under [`docs/contributing/`](docs/contributing/). For what the app does, see the [README](README.md); for what to work on, the [roadmap](ROADMAP.md). Everyone taking part follows the [code of conduct](CODE_OF_CONDUCT.md); for help rather than a change, see [SUPPORT.md](.github/SUPPORT.md).

## No code needed

Two minutes, if your console is still linked to the phone app: *Security › Before unlinking: help decode…* saves the play-timer block, you change one setting in the phone app ("alarm only", or a bedtime for one day), and PlayGuard lists what changed. Attach that file to an issue: it is the data the [roadmap](ROADMAP.md#waiting-on-data-from-a-console) waits for most. Translation fixes from native speakers are just as welcome (see [Translating PlayGuard](#translating-playguard)).

## In short

```sh
git clone --recursive https://github.com/JigSawFr/PlayGuard && cd PlayGuard
make test        # the host tests: gcc, g++, zlib headers, no devkitPro
make desktop     # the real UI against a simulated console: X11, Wayland and D-Bus dev packages
make dist        # playguard.zip for the SD card: devkitPro's switch-dev
```

Then a pull request with a conventional title and a scope (`fix(play-timer): …`), squash-merged; the [template](.github/pull_request_template.md) has the checklist. Every page below says more.

## The pages

| Page | What it covers |
|---|---|
| [Build from source](docs/contributing/build.md) | Every `make` target and what it needs; the Aurore submodule and how a change to it lands |
| [The desktop simulator](docs/contributing/simulator.md) | `make desktop` against a simulated console, every `PLAYGUARD_SIM_*` variable, the same simulator in a browser |
| [Tests and CI](docs/contributing/tests.md) | The host tests, the smoke scenarios and their reference screenshots, the text and resource checks, what CI runs for a pull request and what it leaves out |
| [Code layout](docs/contributing/layout.md) | What each folder holds, and the Cadran look: colours, tokens, dialogs, the controller's keys |
| [Pull requests and releases](docs/contributing/releases.md) | Conventional titles, squash merges, stacked pull requests, release-please, the release page, development builds, reproducible zips |
| [The user guide and the website](docs/contributing/docs.md) | Which page a change goes to, the French version, the stamps of the other translations, the links the app keeps |
| [Translating PlayGuard](docs/contributing/translating.md) | The 15 catalogs, how to add a language, the checks |
| [File and message formats](docs/contributing/formats.md) | The `schema` rule every file, document and IPC structure follows, and each format's version, readers and tests |
| [Testing on a console](docs/hardware-test.md) | What no simulator can show: the forwarders, the PIN gate and the homebrew menu's lock, to go through on a console |
| [Host tests](tests/README.md) | Each suite and what it covers, the fuzz targets, the fakes they share |

The references nearby: [`config.json`](docs/config.md), [themes](docs/themes.md), [how parental controls work](docs/parental-controls.md), [the companion app](docs/companion-app.md), the [optional sysmodules](sysmodule/README.md) (recovery and the remote link's agent), the remote link's [design](docs/sync-design.md) and [protocol](docs/sync-protocol.md) and its [Home Assistant guide](docs/home-assistant.md), the [web simulator](web/README.md), the [website](site/README.md) and the [store entries](packaging/README.md).

## Translating PlayGuard

PlayGuard is translated into every language the console offers, and fixes from native speakers are welcome: [Translating PlayGuard](docs/contributing/translating.md) says where the catalogs are and how to add a language.
