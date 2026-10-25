# Build from source

Part of the [contributor's guide](../../CONTRIBUTING.md).

| Command | What it does | Needs |
|---|---|---|
| `make test` | Host unit tests of the C service layer and of the UI-free C++ helpers and flow logic, with ASan + UBSan (`SAN=` to turn them off) and libstdc++'s assertions (`_GLIBCXX_ASSERTIONS`); the suites run in parallel, all of them even when one fails | gcc and g++ with the sanitizers, zlib headers, the borealis submodule — no devkitPro |
| `make desktop` | The real UI on Linux, against a simulated console | GLFW / X11 / D-Bus dev packages |
| `make web` | The same in a browser: `build-web/simulator/` (`make web-serve` serves it at localhost:8000) | Emscripten (`source <emsdk>/emsdk_env.sh`) |
| `make` | `./playguard.nro` — drawn with deko3d (`GL=1` for OpenGL) | devkitPro `switch-dev`, `DEVKITPRO` set |
| `make dist` | `./playguard.zip` (SD-card layout) | as above |
| `make dist-rescue` | `./playguard-rescue.zip`, the optional recovery sysmodule | as above |
| `./run.sh [ip]` | Builds in the `devkitpro/devkita64` Docker image, at the digest CI uses (read from `build.yml`), optionally `nxlink`s to a console | Docker |

Clone with submodules (`git clone --recursive`, or `git submodule update --init`): borealis is pinned at `extern/borealis/`, on [JigSawFr/borealis](https://github.com/JigSawFr/borealis) (`wiliwili` branch): xfangfang/borealis with the fixes PlayGuard needs in it (one commit each, its message says why; those that were patches here name theirs, `patch 0007`) and a few taken from borealis' other forks. A change to borealis is a pull request there, then a submodule bump here.
