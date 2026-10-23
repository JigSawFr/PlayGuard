# Build from source

Part of the [contributor's guide](../../CONTRIBUTING.md).

| Command | What it does | Needs |
|---|---|---|
| `make test` | Host unit tests of the C service layer and of the UI-free C++ helpers and flow logic, with ASan + UBSan (`SAN=` to turn them off); the suites run in parallel, all of them even when one fails | gcc and g++ with the sanitizers, zlib headers, the borealis submodule — no devkitPro |
| `make desktop` | The real UI on Linux, against a simulated console | GLFW / X11 / D-Bus dev packages |
| `make web` | The same in a browser: `build-web/simulator/` (`make web-serve` serves it at localhost:8000) | Emscripten (`source <emsdk>/emsdk_env.sh`) |
| `make` | `./playguard.nro` — drawn with deko3d (`GL=1` for OpenGL) | devkitPro `switch-dev`, `DEVKITPRO` set |
| `make dist` | `./playguard.zip` (SD-card layout) | as above |
| `make dist-rescue` | `./playguard-rescue.zip`, the optional recovery sysmodule | as above |
| `./run.sh [ip]` | Builds in the `devkitpro/devkita64` Docker image, at the digest CI uses (read from `build.yml`), optionally `nxlink`s to a console | Docker |

Clone with submodules (`git clone --recursive`, or `git submodule update --init`): borealis is pinned at `extern/borealis/`. The fixes PlayGuard needs in it before upstream has them are patches in `patches/borealis/` (its README says why each one), applied to the submodule when CMake configures. `.gitmodules` says `ignore = dirty`, so `git status` leaves those changes out (a new borealis commit still shows); `git -C extern/borealis diff` shows them.
