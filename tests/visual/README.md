# Reference screens

What `tools/desktop_smoke.py` shows on screens that come out the same in every
run (console time fixed, footer clock redrawn at the console's time, 16:00), one folder per scenario:
`smoke/`, `games/`, `errors/`, `gate/`, `rescue/`, `forged/`, `lock/`, `outside/`, `library/`, `devbuild/`, `alarm/`, `cadran/`, `cadranstates/`, `cadranunlocked/`, `cadranlarge/`, `bedtime/`, `forwarders/`, `hbmenu/`.

Each folder's `SCREENS` names the stable screens compared, one a line: that
list is all git keeps. The references are **main's own screens**, never
files kept by hand (`tools/visual_refs.py`): each smoke shard of a run of
main keeps its scenarios' screens, masked, in `.visual-refs/<scenario>/`,
in the Actions cache for the next runs (`visual-refs-<shard>-…`, as the
coverage and `.nro` size baselines) and as the run's `…-refs-<shard>`
artifact. A pull request restores main's and compares with them
(`tools/visual_check.py`); a scenario without any (a cache gone, a new
scenario) takes the latest release's `playguard-screens.zip`, or compares
nothing yet, said as a warning. Each release attaches that zip, the
reference screens of its build, and its page compares them with the
previous release's (*🖼️ On screen*: one before/after image,
`playguard-screens-changed.png`, attached too).

CI compares each new run with them and uploads the side-by-side images of
any screen that changed. **A changed screen fails the build unless the pull
request announces it** on the `Screens:` line of its description (the
template has it): `Screens: smoke/01_dashboard, cadran/*` (`scenario/name`
patterns), `Screens: any` for every screen, `Screens: none` or no line when
no screen is meant to change. An announced screen that changed is counted
as such in the job summary; one announced but unchanged is said. Once the
pull request is merged, main's run keeps the new screens: nothing to update.
The smoke test draws the focus without its moving glow
(`PLAYGUARD_SIM_STILL_FOCUS`) and the header's version badge on one made-up
development build (`PLAYGUARD_SIM_VERSION`), so a screen nobody changed comes out the same.
The screens are drawn by the runner image's software renderer (mesa's
llvmpipe on `ubuntu-26.04`, the image `.github/workflows/build.yml` pins),
so a new image can move every screen a little: moving to one is a pull
request announcing `Screens: any`. Each shard's screenshots
are the `playguard-<version>-<ref>-<commit>-screenshots-<shard>` artifact of
the run: every screen, a `contact-sheet.png` per scenario (every screen of it
on one image) and `visual_diff/` for the screens that changed.

Adding a screen: its name in the scenario's `SCREENS`; the first run says
*new* (nothing to compare with yet), main's next run keeps it. Keep out
screens with a toast, a held-key scroll that can stop anywhere (one held to
the end of a list stops on its last row: `library/`), the app version
(`devbuild/` keeps the build list, not the install confirmation) or SD-card
content (backups) that piles up between local runs. A name taken out of
the list leaves the references at main's next run.

Locally, the same check against main's screens:

```sh
make desktop
python3 tools/visual_refs.py fetch                  # the latest release's playguard-screens.zip into .visual-refs/
python3 tools/visual_refs.py fetch --run <run id>   # or a run's refs artifacts, with gh signed in
xvfb-run -s "-screen 0 1280x720x24" python3 tools/desktop_smoke.py smoke
python3 tools/visual_check.py smoke .visual-refs/smoke --screens tests/visual/smoke/SCREENS --scenario smoke
```

(`smoke-games … games`, `smoke-errors … errors` and so on the same way);
`--expected "smoke/01_dashboard"` or `--expected any` as the `Screens:` line
would. `python3 tools/visual_refs.py keep smoke smoke` makes this run's
screens the local references instead, to compare the next run with.
Every scenario at once, three at a time (`-j` for more), with a summary:

```sh
make desktop
python3 tools/smoke_all.py smoke-all            # against .visual-refs/
python3 tools/smoke_all.py smoke-all --update   # this run's screens as the local references
```
