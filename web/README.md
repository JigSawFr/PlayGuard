# The web simulator

The desktop simulator (`make desktop`, `source/sim/`), compiled with
[Emscripten](https://emscripten.org) for a browser: the real Aurore UI
against a simulated console, to click through a change, try a state or show
a screen without a build on the console. Its games are the desktop
simulator's made-up ones, with their key art for icons and in the album
([docs/contributing/simulator.md](../docs/contributing/simulator.md)).
In the website's look, over boreale-site's `boreale.css` and `boreale.js`
(copied by CMake with its `fonts/`, the sites' marks into `marks/`, and
`branding/mark.svg`): its colours, its header in Boréale's night whatever the
theme (the name opening the menu of Boréale's sites, the links to the
website, the website's language menu, the theme's menu, a short face of them
on a phone) and Sora for the name. The two menus and where they open are
boreale-site's; `simulator.css` keeps the page's own: its light and dark
themes on `:root`, its header, the language button as tall as the theme's
select beside it, and the privacy and legal notice under the sites
(`.sites-legal`). The language menu is the website's (boreale-site's
`Site.languages()`): a button with the page's flag and language (its flag
and code on a phone, its flag alone under 380px), and under it the 15
languages in a popover, each with its flag, the page's marked (Esc or a click
elsewhere closes it, the focus back on the button; ↓ on the button opens it
at the language in use, the arrows, Home and End move through it). Its links
are the simulator's own (`simulator.js`' `setUpLanguages()`: this page with
the same settings in the language, `lang=`), not boreale-site's `lang.js`,
which the page does not load. The menu of Boréale's sites is a copy of
`Site.switcher()` in `index.html`, its texts (`sites_*`) in `web/i18n/`:
the page has no loader for boreale-site's catalogs, so a site added to
`sites.json` is added here too. The flags are boreale-site's
`fonts/twemoji-flags.woff2` (Twemoji's 15, CC-BY 4.0,
`fonts/LICENSE-twemoji-flags.txt`), so Windows shows them too.

- **main**: [guard.boreale.dev/simulator/](https://guard.boreale.dev/simulator/?utm_source=github&utm_medium=docs), on PlayGuard's
  website ([`site/README.md`](../site/README.md)).
- **a pull request**: `https://guard.boreale.dev/pr-<number>/simulator/`,
  linked in a comment on the pull request and updated at each push. It goes
  when the pull request is closed. Pull requests from forks are built but not
  published (their pages would be served from this site).

## The page

- **The settings are in the URL**, each `PLAYGUARD_SIM_*` variable by its name
  in lower case without the prefix: `?unlocked=1&fw=20.5.0` is
  `PLAYGUARD_SIM_UNLOCKED=1 PLAYGUARD_SIM_FW=20.5.0`
  ([docs/contributing/simulator.md](../docs/contributing/simulator.md) says what
  each one simulates). `tab` is not a variable but the first tab (`play_timer`,
  `console`, …), written into the simulated `config.json` before PlayGuard
  starts. A variable the panel does not know (one a newer build reads) still
  reaches PlayGuard.
- **The language** is the browser's, if the console has it (`fr-CA` for a
  Canadian browser, `es-419` for a Mexican one, `zh-Hant` for Taiwan…; else
  English), unless the URL says `lang=` (`fr`, `de`, `zh-Hans`, …) or the
  header's language menu picks another (each language a link to the page
  with the same settings in it, which reloads it). The page is in the 15 languages of the
  console, and PlayGuard always starts in the page's.
- **The theme** is *Dark* by default, as PlayGuard on a console. The
  header chooses *Auto* (the browser's) or *Light* instead (kept for the next
  visits, and put in the URL so a copied link shows it), and `theme=auto` /
  `theme=light` / `theme=dark` in the URL wins over both. PlayGuard starts with the page's theme. It reads it
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
  else: the page hands them to PlayGuard (`playguard_web_setenv()`,
  `source/sim/web_page.cpp`, kept by `core/sim_env.h` under a lock, the
  process environment left alone) and the URL follows. PlayGuard sees the
  new value at its next read (a screen's next refresh).
- **The time**: the buttons under the screen move the console's time on by
  5 min, 15 min or 1 h (`skip=`, in seconds; **Now** goes back), live, with a
  game played all along: the time left goes down by as much, to the limit and
  past it, and past 21:00 the simulated bedtime begins.
- **Keep the state** (`keep=1`): PlayGuard's data folder (`playguard_data`:
  its settings, history, caches, and the simulated console's settings in
  `sim_console.bin`, `PLAYGUARD_SIM_KEEP`) is kept in the browser's IndexedDB,
  written every 3 s and when the page is left. The console settings of the
  panel then count for a first start only; **Forget the kept state** starts
  over. The console then lives on: its time played today counts in real time
  while the page is open (not while it is closed), stays from one visit to
  the next, starts over with a new day or a clock change
  ([docs/contributing/simulator.md](../docs/contributing/simulator.md)), and
  **Restart the console** ends a temporary unlock, as a restart does on a
  console, then reloads the page.
- **The console's line**, under the time: what the simulated console does
  now, read once a second from the first screen on
  (`playguard_web_console()`, `source/sim/web_page.cpp`; its words from
  `SimLogic.consoleLine`): the game running, the time played today, the time
  left, or the limit reached and the game suspended, a warning only and the
  time over it, parental controls unlocked; a notice above the screen when
  the time runs out. Without *Keep the state*, the time played moves only
  with the buttons above, the same at every start.
- **Controls**: the arrows, Enter (Ⓐ) and Esc (Ⓑ) reach SDL directly; the
  page presses the other buttons by what the key says, on any layout: X, Y,
  L, R, Shift+L (ZL), Shift+R (ZR), + or P, − or M, Backspace (Ⓑ). A click is
  a touch. **Pad** shows an on-screen controller (shown by default on a touch
  screen): right under the screen on a phone in portrait, on either side of it
  in landscape, as on a Switch. On a phone, the tools under it (Screenshot, Steps
  link, Tour…) are behind **Tools**. With the browser's data saver on, the page
  waits for a touch before downloading PlayGuard (a few megabytes), unless its
  service worker already keeps it. On a phone, **Copy link** and **Steps link** open the
  system's share sheet (Messages, mail…) where there is one. A gamepad works through the browser. On a
  touch screen the fields are 16px, so that iOS does not zoom into them, and
  a pad button pressed with a finger vibrates briefly (Android). A long
  press on the screen or the pad opens no loupe, selection or menu (iOS).
  **Full screen** (where the browser allows it: Android, a computer; not an
  iPhone's Safari) shows only the screen and the pad, turned to landscape on
  a phone, ✕ or Back to leave it. Added to a phone's home screen, it is
  "PlayGuard" in a window of its own, under the status bar and around the
  notch (`viewport-fit=cover`, the safe areas kept clear); held in
  landscape there, only the screen and the pad too, everything back when
  turned upright. An iPhone takes `apple-touch-icon.png`, Android
  `icon-192.png` and `icon-512.png` (maskable: the logo stays inside the
  part Android may crop), both from `tools/render_branding.mjs`, which also
  renders what an iPhone shows while it starts (`images/splash/`: each
  iPhone since iOS 16 in both orientations, linked with its media query,
  checked by `tests/web_page/splash.test.js`; Android makes its own from
  the manifest's icon and background); a long
  press on Android's icon offers four states (limit reached, unlocked, no
  PIN yet, the tour). The browser's bar takes the header's colour in the
  page's theme. While the tour or a steps link plays, nobody touches the
  phone: its screen is kept on (Wake Lock).
- **Version** (Settings, at the top, when a release has its own): main's
  simulator (`simulator/dev/`) or a release's (`simulator/<tag>/`;
  `simulator/` itself is the latest release's, its `build.json` naming it),
  opened with the same settings (`releases.json`, laid out by `pages.yml`);
  from `dev/` or a release's folder, the site is two folders up.
- **Screenshot** saves the screen as it is (1280×720); **Record** saves a
  video of it (WebM) until **Stop**; **Log** shows what PlayGuard logs.
- **Steps**: from the first screen on, the page notes each button pressed
  (keys, the pad), each touch and each live change, with the time between.
  **Steps link** gives a link with the settings PlayGuard started with and
  those steps (`rec=`); a page opening it replays them once the first screen
  is up, a notice above the screen saying how far it is (**Stop** ends it),
  then drops `rec=` from its URL. A step is a button (`up`, `down`, `left`,
  `right`, `a`, `b`, `x`, `y`, `l`, `r`, `zl`, `zr`, `plus`, `minus`; `down*3`
  three times), `wN` (wait N tenths of a second), `tX,Y` (a touch, in
  1280×720), `s:name=value` (a live setting) or `say:key` (a line of the
  tour); `runSteps()` plays them for the smoke test. With *Keep the state*,
  a link replays from the kept state of whoever opens it.
- **Tour** (`tour=1`): steps through the tabs, then an hour of play, a line
  above the screen for each, in the page's language.
- **Report a problem** opens GitHub's bug form with its *How to reproduce*
  filled in: the steps link, the build, the browser and the end of the log
  (as much as a URL takes).
- The network is a console without a connection: GitHub and switchbrew send
  no CORS headers, so a page cannot reach them (`source/sim/web_http.cpp`).
  The time servers answer, though, with the simulated world's time
  (`source/util/ntp_client.cpp`: a page sends no UDP), so *Console › Network
  clock* measures and sets the clock as on a console with Internet;
  `ntp=offline` is one without.
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
  Aurore's SDL3 platform with OpenGL ES 3, which Emscripten maps to WebGL 2;
  SDL3 and zlib from Emscripten's ports; no libcurl; the resources packaged at
  `/app/resources` and `web/samples/` at `/app/samples` (`playguard.data`); a
  pool of 16 threads started with the page (a thread asked for later waits
  for the browser, which a busy main thread never lets it have); memory
  growth without `GROWABLE_ARRAYBUFFERS`, which the link suggests for
  threads: WebGL refuses views of a resizable buffer; IDBFS for *Keep the
  state*; `source/sim/web_page.cpp`, what the page calls while
  PlayGuard runs (`ccall`).
- `source/main.cpp`: in a browser a frame must hand the thread back to the
  page, so the main loop is `emscripten_set_main_loop`, one `run_frame()` per
  display refresh.
- Aurore, our borealis fork (patch 0090): no SDL haptic in a browser;
  a window fixed at the console's 1280×720 (the page scales the canvas);
  `aurore_web_button()`, which the page calls to press a button; the CJK font
  below as Aurore's fallback when it is there at start-up.
- **Japanese, Korean and Chinese**: a browser has no system font for Aurore
  to fall back on, and the Latin font (Inter, Aurore's off the Switch) has no
  han, kana or hangul. The
  web build cuts the characters those four languages' catalogs use
  (PlayGuard's and Aurore's, for the theme names) out of Noto Sans CJK
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
- The page: `index.html`, `simulator.js`, `simulator.css`; `sim_logic.js`,
  what of it touches neither the page nor PlayGuard (the languages, the URL,
  the variables, the steps' format), tested with Node (`node --test
  tests/web_page/*.test.js`, CI); `params.json`, the panel's entries;
  `i18n/<language>.json`, its texts, one catalog per language of the console
  (`en-US.json` the reference). `tools/check_web_params.py` (CI)
  fails when a `PLAYGUARD_SIM_*` variable read in `source/` has no entry, an
  entry no longer matches a variable, a language of the console has no
  catalog, or English lacks a text; it warns about a text another catalog has
  not translated yet (the page shows the English one meanwhile).
- **Threads need a cross-origin isolated page** (the COOP / COEP headers).
  GitHub Pages cannot send them, so `sw.js` adds them from a service worker
  (as coi-serviceworker, MIT, did before it), which reloads the page once on
  the first visit (the page waits up to 30 s for it: a first visit on a slow
  connection takes a while). `serve.py` sends them itself.
- **The next visits, and offline**: `sw.js` keeps PlayGuard's files
  (`playguard.js`, `.wasm`, `.data`, `cjk.otf`, asked for with the commit in
  their URL) once fetched, a commit's copy replacing the one before, and
  takes them from its cache from then on: the page starts without
  downloading them again. The page's own files come from the network, from
  the cache without one, so the simulator opens offline once visited. Each
  copy of the simulator on the site (main's, a release's, a pull request's
  preview) has its own cache, named after its folder, and keeps only its own
  files.
  `manifest.webmanifest` (the icon from `branding/icon.svg`) makes it
  installable as an app.
- **A Content-Security-Policy** (`index.html`'s `<meta>`): only the page's
  own files, no inline script or style; `'wasm-unsafe-eval'` compiles
  PlayGuard and `blob:` workers are Emscripten's threads. The theme is set
  before the first paint by `theme.js`, a file rather than an inline script
  for that reason. The page loads nothing from elsewhere (but the
  published page's audience, below).
- **The audience**: the published page (on the site, not a pull request's
  preview) gets Umami's tracker, the website's own copy
  (`../assets/umami.js`, `tools/simulator_audience.py` in CI's `web`
  workflow), which counts each visit on the site's address alone, without
  cookie; its policy then lets the page send to `gateway.umami.is`. The
  sites menu links to the privacy and legal notice (boreale.dev's).
  Its links to our other sites carry `utm_source=guard&utm_medium=nav`
  (`SITE_TAGS`), for Umami's *UTM* report; a visit's own tags stay out of
  the links the page makes (`toQuery()`), so a shared link does not carry them.
- CI writes `build.json` (the repository, commit, branch, pull request and
  its title) for the page's header, and the page asks for PlayGuard's files
  with the commit in their URL: GitHub Pages lets browsers keep them 10
  minutes, and an old `.wasm` with a new `.js` would not start.
- **The smoke test** (`tools/web_smoke.py`, CI's *The simulator in a
  browser*): the build in headless Chromium (Playwright, WebGL 2 from
  SwiftShader) must reach its first screen in each kind of start (Today,
  a tab of its own, limit reached, start-up error, Japanese);
  play desktop smoke scenarios as steps (every tab, its screens the comment's
  Play timer, Activity and Console; *alarm*, the first half of *errors*); move the time on and take a live setting without a restart;
  note keys and replay their steps link, play the tour, fill a report link;
  add the CJK font after the start; keep the data folder across a reload
  with `keep=1`; show the console's line, the game suspended at the limit,
  and a restart that locks it again; and, served without the headers as GitHub Pages does, be
  isolated by `sw.js`, keep PlayGuard's files and start offline. A page
  error fails it. Its scenarios run a few at once, each in a process and
  browser of its own (`WEB_SMOKE_JOBS`: the processors by default), and one
  still running after 5 minutes is stopped and fails: about a minute in all
  on a CI runner. `WEB_SMOKE_ONLY=tour,keep` runs some of them. Its screens
  (`shots/`) and the time each start took (`shots/timings.json`, in the
  run's summary) go with the site; each is taken once Aurore lets go of
  the input (`playguard_web_busy()`, `source/sim/web_page.cpp`), not while a
  screen is still opening.
- **The screens against main's** (CI, pull requests only): each screen is
  compared with main's, published with its site, by `tools/visual_check.py`
  (the footer clock masked); `shots/compare.json` lists the ones that
  changed and `shots/visual_diff/` shows them side by side. The pull
  request's comment says which: a report, not a gate.
- Locally, after `make web`: `python3 tools/web_smoke.py build-web/simulator
  build-web/shots` (with `pip install playwright` and `playwright install
  chromium`).

## GitHub Pages

The simulator is published with the website: `.github/workflows/web.yml`
builds both into one artifact, the simulator in `simulator/`, and
`.github/workflows/pages.yml` publishes main's and each open pull request's
([`site/README.md`](../site/README.md#publishing)).

Downloaded, a build is about 4 MB compressed (the `.wasm` 1.5 MB, the
resources 1.8 MB and the CJK font 0.7 MB, fetched after the start in all but
four languages; the run page's summary gives the sizes of each build).
