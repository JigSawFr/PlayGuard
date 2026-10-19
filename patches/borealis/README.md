# PlayGuard's fixes to borealis

`extern/borealis` is the upstream submodule (xfangfang/borealis, wiliwili
branch), left at an upstream commit. The fixes PlayGuard needs before
upstream has them are kept here as patches, applied in name order to the
submodule's files when CMake configures (`CMakeLists.txt`). A patch already
in place is left as it is; one that no longer applies stops the configure,
so a borealis bump cannot silently drop a fix. `git status` shows the
submodule as modified once they are applied: that is expected.

To change one: edit the file in `extern/borealis`, then
`git -C extern/borealis diff > patches/borealis/NNNN-name.patch` (one fix
per patch) and `git -C extern/borealis checkout .` before configuring again.
Once upstream has a fix, the bump that brings it removes its patch.

| Patch | Why |
|---|---|
| `0001-i18n-look-strings-up-without-exceptions.patch` | `getRawStr` looked each string up in the current locale first, with a lookup that throws when it misses. In English (the default locale) the current one is empty, so every `getStr` / `_i18n` threw and caught an exception, and `operator[]` added a null to that locale each time. The empty locale is now skipped and the lookup is `at()` on a const catalogue. Desktop build, 20,000 strings: 45 ms → 7.5 ms in English, French unchanged (8 ms). |
| `0002-label-same-text-not-laid-out-again.patch` | `Label::setText` laid the whole screen out again (`invalidate()`) even when the text did not change, and the tabs that refresh themselves every 1 to 5 s set dozens of texts, most of them the same each time. The same text now returns at once. Desktop build, steady refreshes: the Overview 1.0 → 0.25 ms, the Play timer 2.1 → 0.13 ms, PIN 0.27 → 0.1 ms (with 0003). |
| `0003-view-same-size-not-laid-out-again.patch` | `View::setWidth` / `setHeight` did the same with an unchanged size (the Play timer's week chart sets its seven bars' heights on every refresh). The same size, as set before (in points, or automatic), now returns at once. |
| `0004-view-xml-attributes-registered-when-needed.patch` | Every `View` constructor registered 76 XML attribute handlers (`std::function` in string-keyed maps), even for views built in code that never read one: All games builds four views per game. They are registered the first time an XML attribute is applied or checked, without replacing a subclass's own handler for the same name. Desktop build, 200 views: a `Label` 3–6 → 0.6–0.9 ms, a `Box` 4–5 → 0.7 ms, an All games cover 25 → 10 ms. |
| `0090-sdl-in-a-browser.patch` | The web simulator (`web/README.md`) runs on borealis' SDL platform, which asked SDL for haptic support (none in a browser) and stopped without it; it no longer asks there. The window is not resizable there either: the page scales the canvas, and borealis keeps the console's 1280×720 rather than the page's size. And `brls_web_button()`, exported to the page, presses a controller button: the page maps the keys by their meaning (X, Y, L, R… on any keyboard layout) and draws an on-screen pad, where SDL's keyboard only gives A, B and the arrows. And, a browser having no system font, borealis takes the CJK font the web build packages (`font/cjk.otf`, `tools/web_cjk_font.py`) as its fallback. All only under `__EMSCRIPTEN__`. |
