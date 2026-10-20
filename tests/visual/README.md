# Reference screenshots

What `tools/desktop_smoke.py` shows on screens that come out the same in every
run (console time fixed, footer clock redrawn at the console's time, 16:00:00), one folder per scenario:
`smoke/`, `games/`, `errors/`, `gate/`, `rescue/`, `forged/`, `lock/`, `outside/`, `library/`, `devbuild/`, `alarm/`, `cadran/`, `cadranstates/`, `cadranunlocked/`, `cadranlarge/`, `bedtime/`, `forwarders/`, `hbmenu/`.
CI compares each new run with them
(`tools/visual_check.py`) and uploads the side-by-side images of any screen
that changed. A changed screen fails the build, with the details in the job
summary; the smoke test draws the focus without its moving glow
(`PLAYGUARD_SIM_STILL_FOCUS`) and the header's version badge on one made-up
development build (`PLAYGUARD_SIM_VERSION`), so a screen nobody changed comes out the same.
The screens are drawn by the runner image's software renderer (mesa's
llvmpipe on `ubuntu-26.04`, the image `.github/workflows/build.yml` pins),
so a new image can move every screen a little: moving to one is a pull
request of its own, with the references regenerated. Each shard's screenshots
are the `playguard-<version>-<ref>-<commit>-screenshots-<shard>` artifact of
the run: every screen, a `contact-sheet.png` per scenario (every screen of it
on one image) and `visual_diff/` for the screens that changed.

A UI change that is meant:

```sh
make desktop
xvfb-run -s "-screen 0 1280x720x24" python3 tools/desktop_smoke.py smoke
python3 tools/visual_check.py smoke tests/visual/smoke --update
```

(`smoke-games … games`, `smoke-errors … errors`, `smoke-gate … gate`, `smoke-rescue … rescue`, `smoke-forged … forged`, `smoke-lock … lock`, `smoke-outside … outside`, `smoke-library … library`, `smoke-devbuild … devbuild`, `smoke-alarm … alarm` `smoke-cadran … cadran`, `smoke-cadranstates … cadranstates` `smoke-cadranunlocked … cadranunlocked`, `smoke-cadranlarge … cadranlarge`, `smoke-bedtime … bedtime` `smoke-forwarders … forwarders` and `smoke-hbmenu … hbmenu` the same way), then look at
the new images before committing them. `--update` rewrites only the references
of the screens that changed (by more than the check allows) and leaves the
other files as they are, byte for byte. `--add NAME …` adds a screen to a set;
keep out screens with a toast, a held-key scroll that can stop anywhere
(one held to the end of a list stops on its last row: `library/`), the app
version (`devbuild/` keeps the build list, not the install confirmation) or
SD-card content (backups) that piles up between local runs.

Every scenario at once instead, three at a time (`-j` for more), with the
references of each set whose screens changed replaced, and a summary of
which ones did:

```sh
make desktop
python3 tools/smoke_all.py smoke-all --update
```
