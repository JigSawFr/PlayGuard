# Homebrew App Store (4TU) listing

The listing lives in [switch-hbas-repo](https://github.com/fortheusers/switch-hbas-repo) as `packages/PlayGuard/`: a `pkgbuild.json` and its images (the same catalogue is shown by sphaira's App Store). Its source is [`pkgbuild.json`](pkgbuild.json) in this folder; `tools/hbas_package.py` fills it in for a build and checks it:

```
python3 tools/hbas_package.py <out-dir> [--zip playguard.zip] [--spinarak <switch-hbas-repo>/spinarak/spinarak.py]
```

It writes `<out-dir>/PlayGuard/`, the folder a submission adds to `packages/`:

| In the source | Becomes |
|---|---|
| `{version}` (`info.version`, the zip's URL) | the version of `CMakeLists.txt` |
| `{fw_min_play_timer}`, `{fw_tested_max}` (`info.details`) | `PCTL_FW_MIN_PLAYTIMER` and `PCTL_FW_TESTED_MAX` of `source/core/sysinfo.h` |
| `{changelog}` | `.github/release-notes/<version>.md` without its Markdown and emoji (write one: else only how many features and fixes, and the link to the release) |
| `info.details` as a list | its paragraphs; a line break becomes the `\n` the store reads |
| an image's `"from"` | the image copied as its `url` (`icon.png` 256 × 150, `screen.png` the 848 × 208 banner, `screen1.png`… the screenshots, 1280 × 720, 6 at most) |
| the zip's `"leaves_out"` | removed: the files of `playguard.zip` the store does not install (sphaira's GitHub updater entry) |

With `--zip` it checks the listing against the release zip: each of its paths finds files, `info.binary` is one of them, and every file of the zip is installed or in `"leaves_out"`, so a file added to `make dist` cannot miss the store unseen. With `--spinarak` it then builds the listing with the store's own tool from that zip, as switch-hbas-repo's CI does, and fails when the package would not build or its manifest would not install the binary: what refused the first submission, a listing with no zip (`PlayGuard has an empty manifest`).

CI runs both for every build (the Switch job, spinarak pinned to the commit switch-hbas-repo uses) and keeps the folder as the `playguard-<build>-hbas` artifact.

## What the listing says

- **Short description** (`info.description`): `Offline parental controls & play activity`.
- **Details**: what PlayGuard does, that it carries on Pctl Manager (which is in the store too), the recovery sysmodule as a separate download, and the firmware it needs.
- **Package**: the release's `playguard.zip`, `switch/playguard/*` installed to `/switch/playguard` as `update` (replaced by each update). PlayGuard's own files are in `config/playguard/`, outside the manifest: removing it from the store keeps them, and the limits stay.
- **The recovery sysmodule** (`playguard-rescue.zip`) is not in the listing: the store cannot make a file optional, and it would install a sysmodule started at every boot on every console.

## Submitting or updating

1. Take the `playguard-<version>-hbas` artifact of the release's build (or run the tool on the release's `playguard.zip`).
2. Copy its `PlayGuard/` to `packages/PlayGuard/` of a fork of switch-hbas-repo and open a pull request titled `[new] PlayGuard` (an update: `PlayGuard: update to X.Y.Z`); or use <https://submit.fortheusers.org/>, giving the release's `playguard.zip` as the package.

Once listed, new GitHub releases are detected automatically and published after a manual review. That bot follows the zip by its name and layout: keep `playguard.zip` and its `switch/playguard/` folder, which the CI check above holds to. Releases are automated with [release-please](https://github.com/googleapis/release-please):

1. Merge pull requests with a conventional-commit title (`feat: …`, `fix: …`; checked by `.github/workflows/pr-title.yml`).
2. release-please keeps a `chore(main): release X.Y.Z` PR open with the next version, the `CHANGELOG.md` entry and the bumped `VERSION_*` lines of `CMakeLists.txt`.
3. Merging that PR creates the `vX.Y.Z` tag and the GitHub release; `.github/workflows/release-please.yml` then builds and attaches `playguard.nro`, `playguard.zip`, `playguard-rescue.zip`, `compat.json`, `build-info.txt` and `SHA256SUMS.txt`.
