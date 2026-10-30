# Build from source

Part of the [contributor's guide](../../CONTRIBUTING.md).

| Command | What it does | Needs |
|---|---|---|
| `make test` | Host unit tests of the C service layer and of the UI-free C++ helpers and flow logic, with ASan + UBSan (`SAN=` to turn them off) and libstdc++'s assertions (`_GLIBCXX_ASSERTIONS`); the suites run in parallel, all of them even when one fails | gcc and g++ with the sanitizers, zlib headers, the Aurore submodule — no devkitPro |
| `make desktop` | The real UI on Linux, against a simulated console, with Aurore's debugging aids read from the environment (`AURORE_INSPECTOR=1`, `AURORE_HOT_RELOAD=<resources folder>`) | X11, Wayland and D-Bus dev packages (Aurore builds SDL3 with them) |
| `make web` | The same in a browser: `build-web/simulator/` (`make web-serve` serves it at localhost:8000) | Emscripten (`source <emsdk>/emsdk_env.sh`) |
| `make` | `./playguard.nro` — drawn with deko3d (`GL=1` for OpenGL) | devkitPro `switch-dev`, `DEVKITPRO` set |
| `make dist` | `./playguard.zip` (SD-card layout) | as above |
| `make dist-rescue` | `./playguard-rescue.zip`, the optional recovery sysmodule (`make` also builds it and bundles it in the romfs, for *Console › Optional modules*) | as above |
| `make dist-agent` | `./playguard-agent.zip`, the optional remote-link agent (bundled the same way) | as above |
| `./run.sh [ip]` | Builds in the `devkitpro/devkita64` Docker image, at the digest CI uses (read from `build.yml`), optionally `nxlink`s to a console | Docker |

Clone with submodules (`git clone --recursive`, or `git submodule update --init`): Aurore is pinned at `library/aurore/`: [Aurore](https://github.com/JigSawFr/Aurore) (`main`), our fork of xfangfang/borealis, with the fixes PlayGuard needs in it (one commit each, its message says why; those that were patches here name theirs, `patch 0007`) and a few taken from borealis' other forks. A change to it is a pull request there, then a submodule bump here. A clone from before the move to `library/aurore/` (it was `extern/borealis/`): `git submodule sync && git submodule update --init`.
