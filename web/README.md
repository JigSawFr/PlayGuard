# The web simulator

The desktop simulator (`make desktop`, `source/sim/`), compiled with
[Emscripten](https://emscripten.org) for a browser: the real borealis UI
against a simulated console, to click through a change, try a state or show
a screen without a build on the console.

- **main**: <https://jigsawfr.github.io/PlayGuard/simulator/>, on PlayGuard's
  website ([`site/README.md`](../site/README.md)).
- **a pull request**: `https://jigsawfr.github.io/PlayGuard/pr-<number>/simulator/`,
  linked in a comment on the pull request and updated at each push. It goes
  when the pull request is closed. Pull requests from forks are built but not
  published (their pages would be served from this site).

## The page

- **The settings are in the URL**, each `PLAYGUARD_SIM_*` variable by its name
  in lower case without the prefix: `?unlocked=1&fw=20.5.0` is
  `PLAYGUARD_SIM_UNLOCKED=1 PLAYGUARD_SIM_FW=20.5.0`
  ([CONTRIBUTING.md](../CONTRIBUTING.md#the-desktop-simulator) says what each
  one simulates). `tab` is not a variable but the first tab (`play_timer`,
  `console`, …), written into the simulated `config.json` before PlayGuard
  starts. A variable the panel does not know (one a newer build reads) still
  reaches PlayGuard.
- **The language** is the browser's, if the console has it (`fr-CA` for a
  Canadian browser, `es-419` for a Mexican one, `zh-Hant` for Taiwan…; else
  English), unless the URL says `lang=` (`fr`, `de`, `zh-Hans`, …) or the
  header's list picks another. The page is in the 15 languages of the
  console, and PlayGuard always starts in the page's.
- **The theme** is *Auto* by default: the browser's (light or dark). The
  header chooses *Light* or *Dark* instead (kept for the next visits, and put
  in the URL so a copied link shows it), and `theme=light` / `theme=dark` in
  the URL wins over both. PlayGuard starts with the page's theme. It reads it
  only at start-up: when the system's changes while it runs, under *Auto*, the
  page follows at once and offers to restart PlayGuard with it.
- **The side panel** shows the variables and changes them; **Apply** reloads
  the page with the new URL, so PlayGuard starts again from a fresh state, and
  **Copy link** gives a link that opens that state. **Presets** set common
  states in one go (keeping the language, theme, first tab and *Keep the
  state*); `@21:30` in a preset's time is today at 21:30 in the browser's
  time zone. What is changed inside PlayGuard (a limit, the language in
  Preferences) lives in memory only and goes with the next reload, unless
  *Keep the state* is on (below).
- **Live settings**: the variables PlayGuard reads at each use (`live` in
  `params.json`: the failures, the network's answers, `idle`, `now`, `skip`,
  `accounts`, …) change without a restart when **Apply** changes nothing
  else: the page sets them in PlayGuard's environment
  (`playguard_web_setenv()`, `source/sim/web_page.cpp`) and the URL follows.
  PlayGuard sees the new value at its next read (a screen's next refresh).
- **The time**: the buttons under the screen move the console's time on by
  5 min, 15 min or 1 h (`skip=`, in seconds; **Now** goes back), live, with a
  game played all along: the time left goes down by as much, to the limit and
  past it, and past 21:00 the simulated bedtime begins.
- **Keep the state** (`keep=1`): PlayGuard's data folder (`playguard_data`:
  its settings, history, caches, and the simulated console's settings in
  `sim_console.bin`, `PLAYGUARD_SIM_KEEP`) is kept in the browser's IndexedDB,
  written every 3 s and when the page is left. The console settings of the
  panel then count for a first start only; **Forget the kept state** starts
  over.
- **Controls**: the arrows, Enter (Ⓐ) and Esc (Ⓑ) reach SDL directly; the
  page presses the other buttons by what the key says, on any layout: X, Y,
  L, R, Shift+L (ZL), Shift+R (ZR), + or P, − or M, Backspace (Ⓑ). A click is
  a touch. **Pad** shows an on-screen controller (shown by default on a touch
  screen). A gamepad works through the browser.
- **Screenshot** saves the screen as it is (1280×720); **Record** saves a
  video of it (WebM) until **Stop**; **Log** shows what PlayGuard logs.
- The network is a console without a connection: GitHub and switchbrew send
  no CORS headers, so a page cannot reach them (`source/sim/web_http.cpp`).
  The `PLAYGUARD_SIM_*` folders the desktop reads its answers from are
  packaged instead (`web/samples/`, at `/app/samples`): `sys_notes=sample`
  gives the release notes of Atmosphère 1.12.1, hekate 6.5.4 and firmware
  23.0.0 and 23.0.1, `dev_builds=sample` two releases, main's builds and a
  pull request's, installable (the *Updates to install* preset sets them with
  the rest). Their variables also take `offline` or nothing.
- `<body data-playguard>` says where PlayGuard is: `loading`, `running`,
  `ready` (its first screen is up, `start-up: first screen ready` in its
  log), `error`.

## How it is built

`make web` (needs Emscripten's `emcmake` on the `PATH`: `source <emsdk>/emsdk_env.sh`)
builds `build-web/simulator/`; `make web-serve` serves it at <http://localhost:8000>.

- `CMakeLists.txt`, under `emcmake` (`PLAYGUARD_WEB`): the desktop build on
  borealis' SDL2 platform with OpenGL ES 3, which Emscripten maps to WebGL 2;
  SDL2 and zlib from Emscripten's ports; no libcurl; the resources packaged at
  `/app/resources` and `web/samples/` at `/app/samples` (`playguard.data`); a
  pool of 16 threads started with the page (a thread asked for later waits
  for the browser, which a busy main thread never lets it have); IDBFS for
  *Keep the state*; `source/sim/web_page.cpp`, what the page calls while
  PlayGuard runs (`ccall`).
- `source/main.cpp`: in a browser a frame must hand the thread back to the
  page, so the main loop is `emscripten_set_main_loop`, one `run_frame()` per
  display refresh.
- `patches/borealis/0090-sdl-in-a-browser.patch`: no SDL haptic in a browser;
  a window fixed at the console's 1280×720 (the page scales the canvas);
  `brls_web_button()`, which the page calls to press a button; the CJK font
  below as borealis' fallback when it is there at start-up.
- **Japanese, Korean and Chinese**: a browser has no system font for borealis
  to fall back on, and PlayGuard's Latin font has no han, kana or hangul. The
  web build cuts the characters the four catalogs use out of Noto Sans CJK
  (SIL Open Font License; about 660 KiB, `tools/web_cjk_font.py`, which needs
  fontTools) into `cjk.otf`, served next to the page rather than packaged:
  in those four languages the page fetches it before PlayGuard starts, into
  `/app/resources/font/cjk.otf`; in the others, once PlayGuard runs, and adds
  it behind the Latin font (`playguard_web_cjk_font()`,
  `source/sim/web_page.cpp`), so the first download is that much smaller and
  a language chosen in Preferences still has its characters. It takes
  `/usr/share/fonts/opentype/noto/NotoSansCJK-Regular.ttc` (Debian and
  Ubuntu's `fonts-noto-cjk`), or `-DPLAYGUARD_WEB_CJK_FONT=<file>`; without
  one, the build goes on with a warning and those languages show boxes.
- The page: `index.html`, `simulator.js`, `simulator.css`; `params.json`, the
  panel's entries; `i18n/<language>.json`, its texts, one catalog per language
  of the console (`en-US.json` the reference). `tools/check_web_params.py` (CI)
  fails when a `PLAYGUARD_SIM_*` variable read in `source/` has no entry, an
  entry no longer matches a variable, a language of the console has no
  catalog, or English lacks a text; it warns about a text another catalog has
  not translated yet (the page shows the English one meanwhile).
- **Threads need a cross-origin isolated page** (the COOP / COEP headers).
  GitHub Pages cannot send them, so `coi-serviceworker.js` (vendored, MIT)
  adds them from a service worker, which reloads the page once on the first
  visit (the page waits up to 30 s for it: a first visit on a slow connection
  takes a while). `serve.py` sends them itself.
- **A Content-Security-Policy** (`index.html`'s `<meta>`): only the page's
  own files, no inline script or style; `'wasm-unsafe-eval'` compiles
  PlayGuard and `blob:` workers are Emscripten's threads. The theme is set
  before the first paint by `theme.js`, a file rather than an inline script
  for that reason. The page loads nothing from elsewhere.
- CI writes `build.json` (the repository, commit, branch, pull request and
  its title) for the page's header, and the page asks for PlayGuard's files
  with the commit in their URL: GitHub Pages lets browsers keep them 10
  minutes, and an old `.wasm` with a new `.js` would not start.
- **The smoke test** (`tools/web_smoke.py`, CI's *The simulator in a
  browser*): the build in headless Chromium (Playwright, WebGL 2 from
  SwiftShader) must reach its first screen in each kind of start (Overview,
  Play timer, Activity, Console, limit reached, start-up error, Japanese),
  move the time on and take a live setting without a restart, add the CJK
  font after the start, and keep `sim_console.bin` across a reload with
  `keep=1`; a page error fails it. Its screens (`shots/`) go with the site,
  and the pull request's comment shows them. Locally, after `make web`:
  `python3 tools/web_smoke.py build-web/simulator build-web/shots` (with
  `pip install playwright` and `playwright install chromium`).

## GitHub Pages

The simulator is published with the website: `.github/workflows/web.yml`
builds both into one artifact, the simulator in `simulator/`, and
`.github/workflows/pages.yml` publishes main's and each open pull request's
([`site/README.md`](../site/README.md#publishing)).

Downloaded, a build is about 4 MB compressed (the `.wasm` 1.5 MB, the
resources 1.8 MB and the CJK font 0.7 MB, fetched after the start in all but
four languages; the run page's summary gives the sizes of each build).
