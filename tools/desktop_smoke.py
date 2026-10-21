#!/usr/bin/env python3
"""Headless UI smoke test of the desktop build (simulated backend).

Starts build-desktop/playguard on an X display (Xvfb), opens every tab, the
extra-time picker, the per-day editor (a day's picker and the stepper of an
exact duration), the time-up choice, a bedtime alarm change, the settings
backup and the change history (filtered, then not), and fails if the app
dies on the way (borealis throws on unknown XML attributes, missing views,
…). Screenshots of each screen are written to the output folder.

The "games" scenario is the rest of that tour, in a run of its own so CI can
run both halves side by side: a report sent online (GitHub sign-in first),
Console › Check the installed games (a check game by game, then every fix,
check and removal), then Activity › All games (covers, then the list), a
game's screen from it and the PDF export, which must be there.

The "gate" scenario starts on a firmware newer than the checked one, with a
simulated release that supports it: the firmware screen, read-only mode,
seven presses of Y on About › Version for the developer mode, its report, a play-timer
block reference, deleting the log files and clearing the cache (what goes and
what stays checked on disk), the firmware screen again and the hand-over of
the update to sphaira (simulated hbloader).

The "devbuild" scenario turns the developer mode on, signs in to a simulated
GitHub (PLAYGUARD_SIM_GITHUB_LOGIN) from the build list, then installs a pull
request's build out of its artifact zip (PLAYGUARD_SIM_DEV_BUILDS): the file
must replace the simulated playguard.nro and be handed to hbloader. On the
way, the list must be kept (opened again at once) and X fetch it again.

The "update" scenario starts with Check for updates every day on and a
simulated release 1.1.0 (PLAYGUARD_SIM_LATEST, its notes and playguard.nro
in PLAYGUARD_SIM_DEV_BUILDS): its screen must come up at start-up; Later ›
1 week must postpone it; About › Check for updates must show it again;
Update must ask how to update (none chosen yet), then, in PlayGuard, put
the release in place of the simulated playguard.nro and hand it to hbloader.

The "errors" scenario starts with today's limit reached, the temporary unlock
failing and "Synchronise clock via Internet" off (PLAYGUARD_SIM_FAIL & co.):
a limit change must end in the "could not unlock" dialog, with the app alive,
and the network clock screen must say why the network clock cannot be set. With no time
server answering (PLAYGUARD_SIM_NTP) and a network clock 1411 s ahead, the
tab must then offer the console clock instead, and set it once confirmed.

The "outside" scenario starts with a watch.json from an earlier run that the
console no longer matches (more time played, another clock offset, other
limits, one of the two accounts unknown) and a time left that does not go
down (PLAYGUARD_SIM_NOT_COUNTING): Today must say what changed
outside PlayGuard, record it in the change history, then, after 30 s on
screen (PLAYGUARD_SIM_STALL_S), that the console is not counting; the
notice must go when dismissed, and the Activity tab flag the new account.

The "alarm" scenario starts with "When the time is up" on a warning only and
today's limit passed (PLAYGUARD_SIM_ALARM_ONLY, PLAYGUARD_SIM_RESTRICTED):
Today and the Play timer tab must show the time over the limit. A read of the
play data ends as each tab opens, Today not freed yet (PLAYGUARD_SIM_TAB_RACE):
the header's pill must still be the Play timer's.

The "sysnotes" scenario checks for updates at start-up against a simulated
GitHub and switchbrew (PLAYGUARD_SIM_SYS_LATEST, PLAYGUARD_SIM_SYS_NOTES),
hekate 6.5.3 on the SD card: Console must say how far behind each
one is and that the newest firmware needs a newer hekate, then show the
release notes of the firmware (the closest newer version, then the
installed one), of Atmosphère, of hekate and, from About, of PlayGuard
itself, and keep them in the cache.

The "cadran" scenario tours the boards on top of the tabs: every tab, then
the child screen and back with the PIN, a day of the week edited with the
stick and saved, a restriction level's confirmation, the text size, the
network clock's screen, a game's screen from its card and the notification
bell's list. The "cadranstates" one shows Cadran's Today and
Play timer in the states "alarm" and "outside" set up, together: the time
over the limit (a warning only) and the changes made outside PlayGuard. The
"cadranunlocked" one shows them while parental controls are temporarily
unlocked: "Turn back on" first on Today, the header's pill saying so. The
"cadranlarge" one shows Today, the Play timer, Activity, Console,
Preferences, About, Help and the child screen with Preferences › Text size ›
Larger (as on a TV): Today on one screen, the lists' tiles and cards laid out
again, the child screen keeping its own sizes.

The "bedtime" scenario starts at 20:00 with 90 minutes left
(PLAYGUARD_SIM_NOW, PLAYGUARD_SIM_LEFT_MIN): the bedtime, at 21:00, comes
before the time is up, so the child screen's ring counts down the hour to
the bedtime ("until bedtime"), not the time left.

The "forwarders" scenario opens Console › Forwarders: the forwarder another
app made (its details), PlayGuard itself put on the HOME menu in one press,
a homebrew of the simulated /switch folder picked from the list and protected
with the PIN, then both deleted; forwarders.json and the change history must
follow each step.

The "hbmenu" scenario locks the homebrew menu (with signature patches on the
simulated SD card), the rescue sysmodule's report of a lift at boot taken at
start-up: refused first (no way back yet), then PlayGuard's own
forwarder and sphaira's, protected with the PIN, made; refused again until
sphaira is seen working through its gate (its log); locked with A held on
the checklist, the rescue sheet after it; deleting sphaira refused while
locked; then unlocked: override_config.ini, the state file and the change
history must follow.

The "homebrew" scenario starts with Console › Forwarders in use
(PLAYGUARD_SIM_FORWARDERS) and sphaira paused by its gate: the pause lifted,
hours sphaira never opens, the pause's rule, Star Atlas's icon from the
album, the rescue sheet's page to print; the gate's files, forwarders.json
and the change history must follow.

The "loading" scenario stops the start-up on the loading screen
(PLAYGUARD_SIM_HOLD_LOADING), at its last step, for its screenshot. Every
other scenario waits for the first screen after it, at least 2 s later (the
intro), but "lock", which turns the intro off in config.json.

Usage: tools/desktop_smoke.py <out-dir> [games|gate|errors|rescue|forged|lock|devbuild|update|library|rollback|outside|loading|alarm|sysnotes|cadran|cadranstates|cadranunlocked|cadranlarge|bedtime|forwarders|hbmenu|homebrew]   (needs DISPLAY, xdotool, ImageMagick)
Environment knobs of the simulated backend (PLAYGUARD_SIM_*) are passed through;
the console time is fixed (PLAYGUARD_SIM_NOW, TZ), the focus glow still
(PLAYGUARD_SIM_STILL_FOCUS) and the header's version badge on one development
build (PLAYGUARD_SIM_VERSION) unless set.
"""
import glob
import json
import os
import re
import shutil
import subprocess
import sys
import time

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
APP = os.path.join(ROOT, "build-desktop", "playguard")
OUT = os.path.abspath(sys.argv[1] if len(sys.argv) > 1 else "smoke")
SCENARIO = sys.argv[2] if len(sys.argv) > 2 else ""
GATE = SCENARIO == "gate"
ERRORS = SCENARIO == "errors"
RESCUE = SCENARIO == "rescue"
FORGED = SCENARIO == "forged"
LOCK = SCENARIO == "lock"
DEVBUILD = SCENARIO == "devbuild"
UPDATE = SCENARIO == "update"
LIBRARY = SCENARIO == "library"
OUTSIDE = SCENARIO == "outside"
LOADING = SCENARIO == "loading"
ALARM = SCENARIO == "alarm"
SYSNOTES = SCENARIO == "sysnotes"
ROLLBACK = SCENARIO == "rollback"
GAMES = SCENARIO == "games"
CADRAN = SCENARIO == "cadran"
CADRANSTATES = SCENARIO == "cadranstates"
CADRANUNLOCKED = SCENARIO == "cadranunlocked"
CADRANLARGE = SCENARIO == "cadranlarge"
BEDTIME = SCENARIO == "bedtime"
FORWARDERS = SCENARIO == "forwarders"
HBMENU = SCENARIO == "hbmenu"
HOMEBREW = SCENARIO == "homebrew"
os.makedirs(OUT, exist_ok=True)
run_dir = os.path.join(OUT, "run")
os.makedirs(run_dir, exist_ok=True)
config_file = os.path.join(run_dir, "playguard_data", "config.json")
# What belongs to the simulated system (a sysMMC: util/nand.hpp): its history,
# watch.json, backups and play-data cache.
nand_dir = os.path.join(run_dir, "playguard_data", "nand", "sysmmc")
games_cache = os.path.join(nand_dir, "cache", "games.bin")
if os.path.exists(games_cache):
    os.remove(games_cache)   # Installed games: the first check is this run's
if (GATE or ERRORS or DEVBUILD or UPDATE) and os.path.exists(config_file):
    os.remove(config_file)   # no remembered choice, developer mode off
if DEVBUILD:
    # The pull requests by number (Y on the builds' screen; the keyboard has no Y).
    os.makedirs(os.path.dirname(config_file), exist_ok=True)
    json.dump({"schema": 1, "dev_builds_order": "number"}, open(config_file, "w"))
block_ref = os.path.join(run_dir, "playguard_data", "logs", "play_timer", "block.json")
if GATE and os.path.exists(block_ref):
    os.remove(block_ref)     # the developer tool must write a new one
# What the developer tools' cleanup must remove, and what it must keep.
logs_dir = os.path.join(run_dir, "playguard_data", "logs")
cache_dir = os.path.join(run_dir, "playguard_data", "cache")
old_logs = [os.path.join(logs_dir, n) for n in ("crashes/crash_20261008_120000.txt", "reports/report_20261008_120000.txt")]
uploads = os.path.join(logs_dir, "uploads", "upload_20261008_120000.url")
stale_cache = os.path.join(cache_dir, "stale.bin")
if GATE:
    for path in old_logs + [uploads, stale_cache]:
        os.makedirs(os.path.dirname(path), exist_ok=True)
        with open(path, "w") as f:
            f.write("smoke test\n")

env = dict(os.environ, LIBGL_ALWAYS_SOFTWARE="1")
# ImageMagick 7 keeps the version 6 commands (import, convert, montage) as
# aliases of `magick` that warn on every call; version 6 has no `magick`.
IM = ["magick"] if shutil.which("magick") else []
# One console time and time zone for every run, so the screenshots can be
# compared with the reference ones (tools/visual_check.py): Thursday
# 8 October 2026, 16:00 UTC (20:00 for "bedtime"). Only the footer clock follows the host.
env.setdefault("PLAYGUARD_SIM_NOW", "1791489600" if BEDTIME else "1791475200")
env.setdefault("TZ", "UTC")
# And the focus highlight without its moving glow (ui/cadran_frame.cpp).
env.setdefault("PLAYGUARD_SIM_STILL_FOCUS", "1")
env["PLAYGUARD_SIM_FOCUS_LOG"] = "1"     # each line focused, by its words (go_to())
# And one made-up development build, not this build's version and commit
# (app.cpp): the header's version badge, and the update offers (1.1.0 below)
# stay the same once this build is 1.1.0 or later.
env.setdefault("PLAYGUARD_SIM_VERSION", "1.0.0:1a2b3c4")
# The installed games are checked at start-up for the notification bell:
# at once in every scenario but the default one and "games", which watches a
# check go game by game (150 ms each) and waits for the start-up one first.
if SCENARIO and not GAMES:
    env.setdefault("PLAYGUARD_SIM_CHECK_MS", "0")
if GATE:
    env.setdefault("PLAYGUARD_SIM_FW", "24.0.0")
    env.setdefault("PLAYGUARD_SIM_LATEST", "1.1.0:24.0.0")
    # Started through hbloader, with sphaira on the SD card: the update can be
    # handed over to it.
    env.setdefault("PLAYGUARD_SIM_HBLOADER", "1")
    sphaira = os.path.join(run_dir, "playguard_data", "sd", "switch", "sphaira", "sphaira.nro")
    os.makedirs(os.path.dirname(sphaira), exist_ok=True)
    open(sphaira, "w").write("NRO0")
if DEVBUILD:
    # Help & diagnostics › Developer › Install another build, against a simulated GitHub: the
    # releases (one a pre-release), then (once signed in, simulated device flow) two commits
    # of main and an open pull request whose artifact is a local zip holding
    # playguard.nro.
    import hashlib
    import zipfile
    env.setdefault("PLAYGUARD_SIM_HBLOADER", "1")
    env.setdefault("PLAYGUARD_SIM_GITHUB_LOGIN", "ok")
    # Each simulated answer takes a moment, so the list's bar can be seen.
    env.setdefault("PLAYGUARD_SIM_DEV_BUILDS_DELAY", "600")
    sim = os.path.join(run_dir, "sim_github")
    os.makedirs(sim, exist_ok=True)
    token_file = os.path.join(run_dir, "playguard_data", "github_token")
    if os.path.exists(token_file):
        os.remove(token_file)   # starts signed out
    kept_list = os.path.join(run_dir, "playguard_data", "cache", "dev_builds.json")
    if os.path.exists(kept_list):
        os.remove(kept_list)    # and with no list kept from an earlier run
    pr_content = b"\0" * 16 + b"NRO0" + b"pull request build" * 100
    pr_zip = os.path.join(sim, "pr.zip")
    with zipfile.ZipFile(pr_zip, "w", zipfile.ZIP_DEFLATED) as z:
        # Dated, so the zip and its SHA-256 (on the build's page) are the
        # same in every run, as its reference screenshot needs.
        def entry(name):
            info = zipfile.ZipInfo(name, date_time=(2026, 10, 8, 9, 0, 0))
            info.compress_type = zipfile.ZIP_DEFLATED
            return info
        z.writestr(entry("playguard.nro"), pr_content)
        z.writestr(entry("build-info.txt"), "commit: ccccccc\n")
    zip_bytes = open(pr_zip, "rb").read()
    def artifact(branch, sha, date, path, size, digest=None, head_repo=1, run=1):
        # Named as the build workflow names the artifact holding playguard.nro.
        a = {"name": f"playguard-1.0.0-{'main' if branch == 'main' else 'pr38'}-{sha[:7]}-nro",
             "size_in_bytes": size, "expired": False, "created_at": date,
             "expires_at": "2027-01-07T12:00:00Z",
             "archive_download_url": "https://" + path,
             "workflow_run": {"id": run, "repository_id": 1, "head_repository_id": head_repo,
                              "head_branch": branch, "head_sha": sha}}
        if digest:
            a["digest"] = "sha256:" + digest
        return a
    def release(tag, date, prerelease=False):
        return {"tag_name": tag, "prerelease": prerelease, "draft": False,
                "body": f"## [{tag[1:]}](https://github.com/o/r/compare) ({date[:10]})\n\n"
                        "### Features\n\n* **list:** the releases of the repository\n",
                "assets": [{"name": "playguard.nro", "size": 10, "updated_at": date,
                            "browser_download_url": "https:///missing"}]}
    json.dump(release("v1.0.0", "2026-10-01T00:00:00Z"), open(os.path.join(sim, "latest.json"), "w"))
    json.dump([release("v1.1.0-rc.1", "2026-10-05T00:00:00Z", True), release("v1.0.0", "2026-10-01T00:00:00Z")],
              open(os.path.join(sim, "releases.json"), "w"))
    json.dump({"artifacts": [
        artifact("main", "a" * 40, "2026-10-08T12:00:00Z", "/missing", 10, run=1),
        artifact("main", "b" * 40, "2026-10-07T12:00:00Z", "/missing", 10, run=2),
        artifact("feat/report", "c" * 40, "2026-10-08T09:00:00Z", pr_zip, len(zip_bytes),
                 hashlib.sha256(zip_bytes).hexdigest(), run=3),
    ]}, open(os.path.join(sim, "artifacts.json"), "w"))
    json.dump([{"number": 38, "title": "feat: send a report online\u202e\u0000",
                # GitHub's Markdown, and text that must stay text: HTML, a
                # right-to-left override in the title, control characters.
                "body": "Stacked on #37: merge **that one** first.\n\n"
                        "## What this changes, and why\n\n"
                        "- a report can be sent online, as a [gist](https://gist.github.com)\n"
                        "  - from *Tools \u203a Diagnostics*\n"
                        "1. sign in\n2. send\n\n"
                        "> [!WARNING]\n> Not run on a console.\n\n"
                        "```sh\nmake check\n```\n\n"
                        "| Check | Result |\n|---|---|\n| host tests | pass |\n| smoke | pass |\n\n"
                        "## Checklist\n\n- [x] `make test` passes\n- [ ] tried on a console\n\n"
                        "<img src=x onerror=alert(1)> <script>alert(1)</script>\u0007\u202e\n",
                "head": {"ref": "feat/report", "sha": "c" * 40, "repo": {"id": 1}},
                "base": {"ref": "main", "repo": {"id": 1}}}],
              open(os.path.join(sim, "pulls.json"), "w"))
    # How the runs ended: main's newest failed, the pull request's passed.
    json.dump({"workflow_runs": [
        {"id": 1, "head_sha": "a" * 40, "status": "completed", "conclusion": "failure",
         "display_title": "feat(play-timer)!: show the time played over the limit (#140)",
         "head_commit": {"message": "feat(play-timer)!: show the time played over the limit (#140)\n\n"
                                    "* the gauge goes past its end, in red"}},
        {"id": 2, "head_sha": "b" * 40, "status": "completed", "conclusion": "success",
         "display_title": "docs: a 30-second video tour, at the top of the README (#142)"},
        {"id": 3, "head_sha": "c" * 40, "status": "completed", "conclusion": "success"},
    ]}, open(os.path.join(sim, "runs.json"), "w"))
    env.setdefault("PLAYGUARD_SIM_DEV_BUILDS", sim)
    installed_nro = os.path.join(run_dir, "playguard_data", "sd", "switch", "playguard", "playguard.nro")
    os.makedirs(os.path.dirname(installed_nro), exist_ok=True)
    open(installed_nro, "wb").write(b"the build before")
if UPDATE:
    # A newer release than the simulated 1.0.0, found by the daily check at
    # start-up; its file is a local one (PLAYGUARD_SIM_DEV_BUILDS).
    import hashlib
    env.setdefault("PLAYGUARD_SIM_LATEST", "1.1.0:24.0.0")
    env.setdefault("PLAYGUARD_SIM_HBLOADER", "1")
    sim = os.path.join(run_dir, "sim_github")
    os.makedirs(sim, exist_ok=True)
    release_content = b"\0" * 16 + b"NRO0" + b"release 1.1.0" * 100
    release_nro = os.path.join(sim, "playguard.nro")
    open(release_nro, "wb").write(release_content)
    json.dump({"tag_name": "v1.1.0", "prerelease": False, "draft": False,
               "body": "## [1.1.0](https://github.com/o/r/compare/v1.0.0...v1.1.0) (2026-10-07)\n\n"
                       "### Features\n\n* **update:** update in place from the update screen\n"
                       "* **about:** a screen at start-up for a new version\n\n"
                       "### Bug Fixes\n\n* **ui:** a long link no longer crashes the app\n",
               "assets": [{"name": "playguard.nro", "size": len(release_content),
                           "updated_at": "2026-10-07T10:00:00Z", "browser_download_url": "https://" + release_nro,
                           "digest": "sha256:" + hashlib.sha256(release_content).hexdigest()}]},
              open(os.path.join(sim, "latest.json"), "w"))
    env.setdefault("PLAYGUARD_SIM_DEV_BUILDS", sim)
    installed_nro = os.path.join(run_dir, "playguard_data", "sd", "switch", "playguard", "playguard.nro")
    os.makedirs(os.path.dirname(installed_nro), exist_ok=True)
    open(installed_nro, "wb").write(b"the version before")
    os.makedirs(os.path.dirname(config_file), exist_ok=True)
    json.dump({"schema": 1, "update_daily": True}, open(config_file, "w"))
if not GATE and not ERRORS and not DEVBUILD and not UPDATE:
    env.setdefault("PLAYGUARD_SIM_PASTE", "https://gist.github.com/smoke/ab12")   # what GitHub answers
    github_token = os.path.join(run_dir, "playguard_data", "github_token")
    if os.path.exists(github_token):
        os.remove(github_token)   # signed out: Send a report online offers to sign in first
if ALARM or CADRANSTATES:
    # "When the time is up" on a warning only, today's limit passed: the
    # console does not restrict, and its time left reads below 0.
    env.setdefault("PLAYGUARD_SIM_ALARM_ONLY", "1")
    env.setdefault("PLAYGUARD_SIM_RESTRICTED", "1")
if ALARM:
    env.setdefault("PLAYGUARD_SIM_TAB_RACE", "1")
if ERRORS:
    env.setdefault("PLAYGUARD_SIM_FAIL", "unlock")
    env.setdefault("PLAYGUARD_SIM_RESTRICTED", "1")
    env.setdefault("PLAYGUARD_SIM_AUTOSYNC_OFF", "1")
    env.setdefault("PLAYGUARD_SIM_NTP", "offline")
    env.setdefault("PLAYGUARD_SIM_NETWORK_OFFSET", "1411")
if RESCUE:
    # The playguard-rescue sysmodule left a report: PlayGuard must show the
    # recovery screen at start-up and open the app when it is dismissed.
    # The console confirms it: the sysmodule is installed, its unlock is
    # still on, and the report carries its proof (core/rescue.h): the
    # challenge PlayGuard left, and an HMAC-SHA256 keyed with the simulated
    # PIN (1234). A report the console does not confirm only shows, without
    # the actions.
    import hashlib
    import hmac
    env.setdefault("PLAYGUARD_SIM_UNLOCKED", "1")
    sysmodule = os.path.join(run_dir, "playguard_data", "sd", "atmosphere", "contents", "4200000000505247", "exefs.nsp")
    os.makedirs(os.path.dirname(sysmodule), exist_ok=True)
    open(sysmodule, "w").write("NSP0")
    report = os.path.join(run_dir, "playguard_data", "rescue_report.txt")
    os.makedirs(os.path.dirname(report), exist_ok=True)
    nonce = os.urandom(16).hex()
    open(os.path.join(run_dir, "playguard_data", "rescue_nonce"), "w").write(nonce + "\n")
    body = "mode=unlock\nresult=ok\nrc=0x00000000\nunlocks=1\nrequest=removed\n"
    mac = hmac.new(b"1234", f"playguard-rescue/1\nnonce={nonce}\n{body}".encode(), hashlib.sha256).hexdigest()
    open(report, "w").write(f"{body}nonce={nonce}\nmac={mac}\n")
    history = os.path.join(nand_dir, "history.json")
    if os.path.exists(history):
        os.remove(history)   # a clean history: the rescue entry must be the only one
if FORGED:
    # A rescue report written by hand while PlayGuard asks for the PIN to
    # open, on a console a parent left temporarily unlocked but without the
    # sysmodule: the console's state agrees with it, but it has no proof
    # (core/rescue.h). The recovery screen shows it with no action, and
    # "Open PlayGuard" goes through the lock screen.
    env.setdefault("PLAYGUARD_SIM_UNLOCKED", "1")
    sysmodule = os.path.join(run_dir, "playguard_data", "sd", "atmosphere", "contents", "4200000000505247")
    if os.path.exists(sysmodule):
        import shutil
        shutil.rmtree(sysmodule)
    report = os.path.join(run_dir, "playguard_data", "rescue_report.txt")
    os.makedirs(os.path.dirname(report), exist_ok=True)
    open(report, "w").write("mode=unlock\nresult=ok\nrc=0x00000000\nunlocks=1\n")
    json.dump({"schema": 1, "pin_lock": "open"}, open(config_file, "w"))
if CADRANUNLOCKED:
    # A parent unlocked parental controls for now (1453 answers false). In
    # the light theme: every other scenario draws the default, dark one.
    env.setdefault("PLAYGUARD_SIM_UNLOCKED", "1")
    os.makedirs(os.path.dirname(config_file), exist_ok=True)
    json.dump({"schema": 1, "pin_lock": "open", "theme": "light"}, open(config_file, "w"))
if CADRANLARGE:
    # The text 25 % bigger, as the console docked draws it by default.
    os.makedirs(os.path.dirname(config_file), exist_ok=True)
    json.dump({"schema": 1, "text_size": "large"}, open(config_file, "w"))
if BEDTIME:
    # 20:00 (above), an hour before the simulated bedtime (21:00 every day),
    # with 90 minutes left: the bedtime stops play first.
    env.setdefault("PLAYGUARD_SIM_LEFT_MIN", "90")
if FORWARDERS or HBMENU or HOMEBREW:
    # Nothing made in an earlier local run: the simulated HOME menu starts
    # with the one another app made (PLAYGUARD_SIM_KEEP is off), PlayGuard's
    # file with none, the made-up .nro files written again.
    for f in (os.path.join(nand_dir, "forwarders.json"), os.path.join(nand_dir, "history.json")):
        if os.path.exists(f):
            os.remove(f)
    shutil.rmtree(os.path.join(run_dir, "playguard_data", "sd", "switch"), ignore_errors=True)
sd_override = os.path.join(run_dir, "playguard_data", "sd", "atmosphere", "config", "override_config.ini")
hbmenu_state = os.path.join(run_dir, "playguard_data", "hbmenu_lock.txt")
if HBMENU:
    # The homebrew menu never locked in an earlier run; the signature patches
    # forwarders need (sigpatch files for fs and the loader).
    for f in (sd_override, hbmenu_state, os.path.join(run_dir, "playguard_data", "gate_rules.txt")):
        if os.path.exists(f):
            os.remove(f)
    # What the rescue sysmodule leaves after lifting it at boot (Atmosphère
    # updated since): told and recorded at start-up, then removed.
    os.makedirs(os.path.join(run_dir, "playguard_data"), exist_ok=True)
    open(os.path.join(run_dir, "playguard_data", "hbmenu_lock_report.txt"), "w").write(
        "reason=atmosphere\nresult=ok\nfrom=1.9.4\nto=1.9.5\n")
    # The gate's log of three tries the same evening (refused: the bell
    # tells it), and the lock PlayGuard held taken away from the file
    # outside it: both told and recorded at start-up.
    open(os.path.join(run_dir, "playguard_data", "gate_log.txt"), "w").write(
        "".join("2026-10-07 21:0%d\trefused\tsdmc:/switch/sphaira/sphaira.nro\n" % m for m in (3, 4, 5)))
    open(hbmenu_state, "w").write("ams=1.9.4\n")
    for kind in ("fs_patches", "loader_patches"):
        d = os.path.join(run_dir, "playguard_data", "sd", "atmosphere", "kip_patches", kind)
        os.makedirs(d, exist_ok=True)
        open(os.path.join(d, "0000000000000000.ips"), "wb").write(b"PATCHEOF")
data_dir = os.path.join(run_dir, "playguard_data")
if HOMEBREW:
    # Console › Forwarders in use (PLAYGUARD_SIM_FORWARDERS: PlayGuard's,
    # sphaira protected, Star Atlas without the PIN, two screenshots), the
    # gate's files from no earlier run, sphaira paused for an hour by its gate.
    env["PLAYGUARD_SIM_FORWARDERS"] = "1"
    for f in ("gate_rules.txt", "gate_closed.txt", "rescue.html", "hbmenu_lock.txt"):
        if os.path.exists(os.path.join(data_dir, f)):
            os.remove(os.path.join(data_dir, f))
    if os.path.exists(sd_override):
        os.remove(sd_override)
    os.makedirs(data_dir, exist_ok=True)
    open(os.path.join(data_dir, "gate_pause.txt"), "w").write(
        "0\t%d\tsdmc:/switch/sphaira/sphaira.nro\n" % (int(time.time()) + 3600))
if LIBRARY:
    # A large library: All games builds its first covers, then the rest on
    # "Show every game".
    env.setdefault("PLAYGUARD_SIM_GAMES", "120")
if OUTSIDE or CADRANSTATES:
    # What PlayGuard saw last time (outside_change_logic): 100 min played
    # today (the console says 80), a clock 15 min off the steady one (the
    # simulated steady clock's source is 16 bytes of 0x53), 1 h 30 on
    # Saturday (the console has 3 h), and only Alice of the two accounts
    # (Léo is new).
    env.setdefault("PLAYGUARD_SIM_NOT_COUNTING", "1")
    if OUTSIDE:
        # "Not counting" after 30 s on screen rather than 90: the same
        # screens, a minute sooner (01_outside is taken well before).
        env.setdefault("PLAYGUARD_SIM_STALL_S", "30")
    watch_file = os.path.join(nand_dir, "watch.json")
    os.makedirs(os.path.dirname(watch_file), exist_ok=True)
    json.dump({"schema": 1, "date": "2026-10-08", "spent_s": 6000, "offset_s": 900, "steady_id": "53" * 16,
               "limits": [180, 120, 120, 120, 120, 120, 90],
               "accounts": [{"id": "%016X%016X" % (1, 1), "since": "2026-09-01"}]}, open(watch_file, "w"))
    history_file = os.path.join(nand_dir, "history.json")
    if os.path.exists(history_file):
        os.remove(history_file)   # the outside_* entries must be this run's
if SYSNOTES:
    # A simulated GitHub and switchbrew (util/sys_notes.hpp), shaped like
    # theirs: Atmosphère 1.12.0 runs (1.12.1 is out), hekate 6.5.3 is on the
    # SD card (6.5.4 is out, the first to support 23.0.1), the console runs
    # 22.5.0 (23.0.0 and 23.0.1 are out).
    sim_notes = os.path.join(run_dir, "sys_notes_sim")
    os.makedirs(sim_notes, exist_ok=True)
    def release(repo, tag, date, body):
        return {"tag_name": tag, "html_url": f"https://github.com/{repo}/releases/tag/{tag}", "draft": False,
                "prerelease": False, "published_at": date, "body": body.replace("\n", "\r\n")}
    ams = "Atmosphere-NX/Atmosphere"
    json.dump([release(ams, "1.12.1", "2026-10-06T00:05:51Z",
                       "**1.12.1** is Atmosphère's ninety-fourth official release.\n\n"
                       "The following was changed since the last release:\n"
                       "* Support was added for the new \"BatchRead\" storage in `fs.mitm`.\n"
                       "  * Firmware 23.0.0 introduced a new type of filesystem storage.\n"
                       "* General system stability improvements to enhance the user's experience.\n\n"
                       "And the following was changed in 1.12.0:\n* Support was added for 23.0.0."),
               release(ams, "1.12.0", "2026-09-27T23:21:53Z",
                       "**1.12.0** is Atmosphère's ninety-third official release.\n\n"
                       "The following was changed since the last release:\n\n* Support was added for 23.0.0.\n"
                       "    * `mesosphère` was updated to reflect the latest official kernel behavior.\n"
                       "* General system stability improvements to enhance the user's experience."),
               release(ams, "1.11.2", "2026-06-16T22:01:02Z", "* Basic support was added for 22.5.0.")],
              open(os.path.join(sim_notes, "atmosphere.json"), "w"))
    def hekate_notes(v, fw):
        return ("![hekate-nyx-banner](https://example.invalid/banner.png)\n\n"
                "**Supports booting ALL current CFWs, Android, Linux and payload tools.**\n\n"
                f"### Latest HOS supported: {fw}\n\n\nhekate v{v} changes\n-------------\n- **HOS {fw} support**\n\n\n"
                "FAQ\n-------------\n\n- **Which Linux/Android distributions are supported?**\n Many.\n\n"
                "# How to update:\n- Drag n drop `bootloader` folder into sd card root.\n\n\n# Download:")
    json.dump([release("CTCaer/hekate", "v6.5.4", "2026-09-28T00:06:10Z", hekate_notes("6.5.4", "23.0.1")),
               release("CTCaer/hekate", "v6.5.3", "2026-06-16T22:14:44Z", hekate_notes("6.5.3", "22.5.0"))],
              open(os.path.join(sim_notes, "hekate.json"), "w"))
    # PlayGuard's own releases, as its release workflow writes their pages:
    # 1.0.0 runs (the simulated version), 1.1.0 is out.
    def playguard_notes(v, highlight, change):
        return (f"## [{v}](https://github.com/JigSawFr/PlayGuard/compare/v1.0.0...v{v}) \u00b7 2026-10-14\n\n"
                f"<p>\u2b07\ufe0f <a href=\"https://github.com/JigSawFr/PlayGuard/releases/download/v{v}/playguard.zip\">"
                "<b>Download playguard.zip</b></a> (2.4 MB)</p>\n\n<hr>\n\n### \U0001f31f Highlights\n\n"
                f"* \u23f0 **{highlight}**:  \n  Choose what the console does when the play-time limit is reached.\n\n"
                "<!-- console:hide -->\n> [!IMPORTANT]\n> Summary written automatically.\n<!-- /console -->\n"
                "<!-- console: Summary written automatically. -->\n\n<hr>\n\n"
                f"<details>\n<summary><b>Every change in {v}</b> (1 change)</summary>\n\n### \U0001f680 Features\n\n"
                f"* **play-timer:** {change} ([#134](https://github.com/JigSawFr/PlayGuard/issues/134)) "
                "([cc4b958](https://github.com/JigSawFr/PlayGuard/commit/cc4b958acba7017bb8b178c192d99a1448f66063))\n\n"
                "</details>\n\n## \U0001f4e6 Install\n\n1. **New install**: download `playguard.zip`.\n")
    pg = "JigSawFr/PlayGuard"
    json.dump([release(pg, "v1.1.0", "2026-10-14T22:31:37Z",
                       playguard_notes("1.1.0", "When time is up", "choose what the console does when the time is up")),
               release(pg, "v1.0.0", "2026-10-12T20:17:06Z",
                       playguard_notes("1.0.0", "Play-time limit", "the same every day or one per day"))],
              open(os.path.join(sim_notes, "playguard.json"), "w"))
    rows = [("22.0.0", "March 3, 2026"), ("22.1.0", "May 5, 2026"), ("22.5.0", "June 16, 2026"),
            ("23.0.0", "September 10, 2026"), ("23.0.1", "October 1, 2026")]
    open(os.path.join(sim_notes, "System_Versions.wiki"), "w").write(
        "{| class=\"wikitable\"\n|-\n! System version\n! Release date\n" +
        "".join(f"|-\n| [[{v}]]\n| {d} (UTC)\n" for v, d in rows) + "|}\n")
    pages = {
        "22.5.0": ("June 16, 2026", "no", "* General system stability improvements to enhance the user's experience.", "sdb"),
        "23.0.0": ("September 10, 2026", "yes",
                   "* Added \u201cSettings\u201d to Virtual Game Cards from the HOME Menu.\n"
                   "** You can review and edit which of your systems can use virtual game cards.\n"
                   "* General system stability improvements to enhance the user's experience.", "usb, pctl, ns"),
        "23.0.1": ("October 1, 2026", "<fill this in manually>",
                   "* Fixed an issue from system version 23.0.0 where software occasionally took a long time to start.", "sdb"),
    }
    for v, (d, flaws, log_text, sysmodules) in pages.items():
        open(os.path.join(sim_notes, v + ".wiki"), "w").write(
            f"The Switch {v} system update was released on {d} (UTC).\n\nSecurity flaws fixed: {flaws}.\n\n"
            "==Change-log==\n[https://en-americas-support.nintendo.com/ Official] ALL change-log:\n"
            f"{log_text}\n\n==System Titles==\n* The following titles were updated:\n** Sysmodules: {sysmodules}.\n")
    env.setdefault("PLAYGUARD_SIM_SYS_NOTES", sim_notes)
    env.setdefault("PLAYGUARD_SIM_SYS_LATEST", "1.12.1:6.5.4:23.0.1")
    # No PlayGuard update offered over the Console tab by the same check.
    env.setdefault("PLAYGUARD_SIM_LATEST", "offline")
    env.setdefault("PLAYGUARD_SIM_FW", "22.5.0")   # behind 23.0.0 and 23.0.1
    update_bin = os.path.join(run_dir, "playguard_data", "sd", "bootloader", "update.bin")
    os.makedirs(os.path.dirname(update_bin), exist_ok=True)
    open(update_bin, "wb").write(b"\0" * 0x40 + b"ICTC" + bytes([ord("0") + 6, ord("0") + 5, ord("0") + 3, 0]))
    notes_cache = os.path.join(run_dir, "playguard_data", "cache", "sys_notes.json")
    if os.path.exists(notes_cache):
        os.remove(notes_cache)   # fetched in this run
    # Checking for updates at start-up (the daily check), not done yet today.
    os.makedirs(os.path.dirname(config_file), exist_ok=True)
    json.dump({"schema": 1, "update_daily": True}, open(config_file, "w"))
if LOCK:
    # PIN › Protect PlayGuard with the PIN › To open PlayGuard: the lock screen comes
    # first, and the right PIN (the simulated PIN screen accepts) opens the
    # app in its place. And without the intro (Preferences): no 2 s hold.
    os.makedirs(os.path.dirname(config_file), exist_ok=True)
    json.dump({"schema": 1, "pin_lock": "open", "intro": False}, open(config_file, "w"))
if LOADING:
    env["PLAYGUARD_SIM_HOLD_LOADING"] = "1"
if ROLLBACK:
    # A build installed in place whose last start never reached its first
    # screen, the build before it kept: the start-up screen offers it before
    # anything else (the PIN set or not), A takes it back and hbloader starts
    # it. Then a start that is not after a crash: nothing offered (see the
    # end of the run).
    env.setdefault("PLAYGUARD_SIM_HBLOADER", "1")
    env["PLAYGUARD_SIM_BOOT_CHOICE"] = "a"
    sd_nro = os.path.join(run_dir, "playguard_data", "sd", "switch", "playguard", "playguard.nro")
    os.makedirs(os.path.dirname(sd_nro), exist_ok=True)
    open(sd_nro, "w").write("the build that crashes")
    open(sd_nro + ".previous", "w").write("the build before")
    data = os.path.join(run_dir, "playguard_data")
    json.dump({"label": "1.0.0 (0abc123)"}, open(os.path.join(data, "previous_build.json"), "w"))
    json.dump({"label": "PR #38", "starts": 1}, open(os.path.join(data, "boot_check.json"), "w"))
log = open(os.path.join(OUT, "app.log"), "w")
def have(tool):
    return subprocess.run(["which", tool], capture_output=True).returncode == 0


# Line-buffered output, so app.log is complete at any moment (messages()).
cmd = (["stdbuf", "-oL", "-eL"] if have("stdbuf") else []) + [APP]
if have("dbus-run-session"):
    cmd = ["dbus-run-session", "--"] + cmd
proc = subprocess.Popen(cmd, cwd=run_dir, env=env, stdout=log, stderr=subprocess.STDOUT)


def alive():
    return proc.poll() is None


# The pauses around a press and before a screenshot. borealis' longest
# animation (a toast coming in) takes 300 ms, and a label starts scrolling
# after 1.5 s focused: a screenshot comes 0.9 s after the last press, every
# animation over, no label moving yet.
KEY_BEFORE = 0.35
KEY_AFTER = 0.1
SHOT_WAIT = 0.45


def key(name, n=1, hold=0.02):
    """Presses `name` n times, each exactly once. A held Down / Up repeats
    (ScrollView scrolls a long text, then moves the focus again); with frames
    as slow as under software GL, even an 80 ms press could. So each press
    waits for the screen to settle (KEY_BEFORE: borealis' animations last
    300 ms at most, timed by the clock, not by the frames) and is released
    `hold` seconds later (20 ms by default), both sent by one xdotool process
    so a busy runner cannot delay the release."""
    for _ in range(n):
        time.sleep(KEY_BEFORE)
        subprocess.run(["xdotool", "keydown", name, "sleep", str(hold), "keyup", name], env=env)
        time.sleep(KEY_AFTER)


def focused():
    """The words of the line that has the focus now (PLAYGUARD_SIM_FOCUS_LOG:
    source/ui/focus_log.cpp), or None."""
    log.flush()
    last = None
    for l in open(os.path.join(OUT, "app.log"), errors="replace"):
        at = l.find("focus: ")
        if at >= 0:
            last = l[at + len("focus: "):].rstrip("\n")
    return last


def go_to(label, direction="Down", most=25):
    """Presses `direction` until the line `label` has the focus (its words, in
    English), `most` times at most: a list or a sheet whose lines move keeps
    its tests right."""
    time.sleep(KEY_BEFORE)     # the sheet just opened has its focus
    for _ in range(most):
        if focused() == label:
            return
        key(direction)
    if focused() != label:
        fail(f"no line {label!r} reached with {direction} (the focus on {focused()!r})")


def tap(x, y):
    """A touch at (x, y): the desktop build reads the left mouse button as one."""
    time.sleep(KEY_BEFORE)
    subprocess.run(["xdotool", "mousemove", str(x), str(y), "mousedown", "1", "sleep", "0.05", "mouseup", "1"], env=env)
    time.sleep(KEY_AFTER)


def screen(name):
    """A screenshot of the screen without the footer (its clock follows the
    host), as <out-dir>/name.png."""
    path = os.path.join(OUT, name + ".png")
    subprocess.run(IM + ["import", "-window", "root", "-crop", "1280x650+0+0", "+repage", path], env=env)
    return path


def key_to_end(name, most=50, hold=0.02):
    """Presses `name` until the screen stops changing (the end of the list:
    the focus and the scroll stay put), `most` times at most. A pixel counts
    as changed past 10 % (the focused tile's shade wavers by a few), as in
    tools/visual_check.py; a screen that never settles gets all `most`
    presses, as key() would."""
    before = screen("_before")
    for _ in range(most):
        key(name, hold=hold)
        now = screen("_now")
        changed = subprocess.run(IM + ["compare", "-fuzz", "10%", "-metric", "AE", before, now, "null:"],
                                 capture_output=True, text=True).stderr.split()
        os.replace(now, before)
        if changed[:1] == ["0"]:   # ImageMagick 7 adds " (0)"
            break
    os.remove(before)


def button_x():
    """Ⓧ: the desktop build reads it from the middle mouse button (the
    keyboard has none), clicked over the title, where nothing takes the focus."""
    time.sleep(KEY_BEFORE)
    subprocess.run(["xdotool", "mousemove", "640", "40", "mousedown", "2", "sleep", "0.05", "mouseup", "2"], env=env)
    time.sleep(KEY_AFTER)


def shot(name):
    time.sleep(SHOT_WAIT)
    subprocess.run(IM + ["import", "-window", "root", os.path.join(OUT, name + ".png")], env=env)
    if not alive():
        fail(f"app exited while showing {name}")


def messages():
    """What the app told the user so far: every toast (ui::notify) and every
    error dialog (ui::error), both logged."""
    log.flush()
    out = []
    for l in open(os.path.join(OUT, "app.log"), errors="replace"):
        for tag in ("toast: ", "error: "):
            if tag in l:
                out.append(l.split(tag, 1)[1].strip())
    return out


def fail(msg):
    log.flush()
    print("SMOKE FAILED:", msg)
    print(open(os.path.join(OUT, "app.log")).read()[-3000:])
    sys.exit(1)


if ROLLBACK:
    # No window: PlayGuard ends at once, the previous build in place.
    try:
        proc.wait(20)
    except subprocess.TimeoutExpired:
        proc.kill()
        fail("still running: the previous build was not offered at start-up")
    log.flush()
    text = open(os.path.join(OUT, "app.log"), errors="replace").read()
    if "Its last start did not finish" not in text or "Go back to 1.0.0 (0abc123)" not in text:
        fail("the start-up screen did not offer the previous build")
    if "next load: sdmc:/switch/playguard/playguard.nro" not in text:
        fail("the previous build was not set as the next homebrew")
    if open(sd_nro).read() != "the build before" or open(sd_nro + ".previous").read() != "the build that crashes":
        fail("the two builds were not swapped")
    if os.path.exists(os.path.join(data, "boot_check.json")):
        fail("the start check was left behind")
    if json.load(open(os.path.join(data, "previous_build.json"))).get("label") != "1.0.0 (1a2b3c4)":
        fail("the build gone back from is not named as the previous one")

    # The next start (no start check left): nothing offered, it goes on to
    # its window, the two builds as they are.
    run_log_file = os.path.join(OUT, "app-again.log")
    with open(run_log_file, "w") as run_log:
        p = subprocess.Popen(cmd, cwd=run_dir, env=env, stdout=run_log, stderr=subprocess.STDOUT)
        for _ in range(40):
            if p.poll() is not None or "start-up: loading screen drawn" in open(run_log_file, errors="replace").read():
                break
            time.sleep(0.5)
        if p.poll() is None:
            p.kill()
        p.wait()
    text = open(run_log_file, errors="replace").read()
    if "start-up: loading screen drawn" not in text:
        fail("the start after going back did not reach its loading screen")
    if "rollback: offered" in text or "next load: sdmc:" in text:
        fail("the previous build was offered at a start that was not after a crash")
    if open(sd_nro).read() != "the build before" or open(sd_nro + ".previous").read() != "the build that crashes":
        fail("a start that was not after a crash swapped the two builds")
    print("desktop smoke test passed:", OUT)
    sys.exit(0)


for _ in range(60):
    if subprocess.run(["xdotool", "search", "--name", "PlayGuard"], env=env, capture_output=True).returncode == 0:
        break
    if not alive():
        fail("app exited during start-up")
    time.sleep(0.5)
else:
    fail("no window after 30 s")
# The window exists before its first frame: wait until the screen is drawn
# (not all black), else the first screenshot can catch an empty window on a
# slow runner. Then a moment more for the first screen to settle.
probe = os.path.join(OUT, "_first_frame.png")
for _ in range(40):
    subprocess.run(IM + ["import", "-window", "root", probe], env=env)
    mean = subprocess.run(IM + ["convert", probe, "-format", "%[fx:mean]", "info:"], capture_output=True, text=True).stdout
    if mean and float(mean) > 0.05:
        break
    if not alive():
        fail("app exited during start-up")
    time.sleep(0.5)
else:
    fail("the window stayed black for 20 s")
os.remove(probe)


def logged(text):
    log.flush()
    return any(text in l for l in open(os.path.join(OUT, "app.log"), errors="replace"))


# What is drawn first is the loading screen: wait for the screen after it
# (or, holding it, for its last step).
started = "start-up: held on the loading screen" if LOADING else "start-up: first screen ready"
for _ in range(60):
    if logged(started):
        break
    if not alive():
        fail("app exited during start-up")
    time.sleep(0.5)
else:
    fail("no first screen after the loading screen in 30 s")
time.sleep(2)
if not SCENARIO or GAMES:
    # The start-up check of the installed games: the bell counts them.
    for _ in range(40):
        if logged("games: start-up check done"):
            break
        time.sleep(0.25)
    else:
        fail("the installed games were not checked at start-up")
# The intro (Preferences, on by default): the loading screen stays 2 s.
intro = logged("start-up: intro, 2000 ms at least")
if LOCK and intro:
    fail("the intro was shown with \"intro\": false in config.json")
if not LOCK and not intro:
    fail("no intro at start-up")
if not LOADING and not LOCK:
    ready = [l for l in open(os.path.join(OUT, "app.log"), errors="replace") if "start-up: first screen ready after" in l]
    m = re.search(r"after (\d+) ms", ready[0]) if ready else None
    if not m or int(m.group(1)) < 2000:
        fail("the first screen came before the end of the intro: " + (ready[0].strip() if ready else "no line"))

tabs = ["dashboard", "play_timer", "activity", "restrictions", "security", "console", "preferences", "about"]


def steps(a, b):
    """Sidebar presses from tab `a` down to tab `b` (negative: up)."""
    return tabs.index(b) - tabs.index(a)


def finish(check=None):
    """Stops the app, then runs `check` (on its complete log)."""
    proc.terminate()
    try:
        proc.wait(5)
    except subprocess.TimeoutExpired:
        proc.kill()
    if check:
        check()
    contact_sheet()
    print("desktop smoke test passed:", OUT)
    sys.exit(0)


def contact_sheet():
    """Every screen of the run on one image, contact-sheet.png, each under its
    name: a whole scenario can be looked over at a glance (CI keeps it with
    the screenshots). Not a screen: tools/visual_check.py only reads the
    names of the references."""
    shots = sorted(p for p in glob.glob(os.path.join(OUT, "*.png")) if not p.endswith("contact-sheet.png"))
    if not shots:
        return
    subprocess.run(IM + ["montage", "-label", "%t", "-background", "#1f1f1f", "-fill", "white",
                         "-tile", "4x", "-geometry", "320x180+6+6"] + shots + [os.path.join(OUT, "contact-sheet.png")],
                   env=env, capture_output=True)


if GATE:
    shot("01_gate")            # a release supports the firmware: its cell has the focus
    key("Down")                # Continue in read-only mode
    key("Return")
    shot("02_read_only")       # title "PlayGuard" and the read-only icon
    key("Down")                # Play timer: the read-only note, no limit editors
    key("Right")
    shot("03_play_timer_read_only")
    key("Left")
    # About: seven presses of Y on Version (its first cell) turn the developer mode on.
    key("Down", steps("play_timer", "about"))
    key("Right")
    key("y", 7)
    shot("04_dev_enabled")
    # The developer tools are at the end of About › Help & diagnostics:
    # press like a hand.
    key("Left")
    key("Right")
    key_to_end("Down", hold=0.15) # its last line: the funding QR codes
    key("Up", 2)               # past Source: Help & diagnostics
    key("Return")
    key_to_end("Down", hold=0.15) # the Developer section, down to its last cell
    shot("05_dev_tools")
    # Run the migrations again (its last cell), two of the three chosen: a
    # diagnostic report saved in the logs/ layout before its subfolders, left
    # there while PlayGuard runs, goes to logs/reports/ (then away: the
    # cleanup below counts its files).
    old_report = os.path.join(logs_dir, "20261009_141203.txt")
    with open(old_report, "w") as f:
        f.write("smoke test\n")
    key("Return")
    shot("05b_migrations")     # a line per migration, all checked, then "Run the 3 chosen"
    key("Return")              # the data folder unchecked: the list again, on it
    shot("05c_migrations_two") # "Run the 2 chosen"
    key("Down", 2)             # the system's folder unchecked too
    key("Return")
    key("Up", 2)               # the data folder checked again
    key("Return")
    key("Down", 3)             # Run the 2 chosen (the data folder and the logs' subfolders)
    key("Return")
    shot("05d_migrations_confirm")   # the migrations chosen, Run
    key("Right")               # Run
    key("Return")
    shot("05e_migrations_done")
    key("Return")              # OK
    moved_report = os.path.join(logs_dir, "reports", "report_20261009_141203.txt")
    if os.path.exists(old_report) or not os.path.exists(moved_report):
        fail("Run the migrations again did not move " + old_report)
    os.remove(moved_report)
    log.flush()
    if "dev tools: migrations run again: 2 step(s), 1 entries moved" not in open(os.path.join(OUT, "app.log"), errors="replace").read():
        fail("no migrations line in the log; messages: " + repr(messages()))
    # Turning a developer switch off asks first; B keeps it on (the end of
    # the scenario checks developer mode was saved on).
    key("Up", 12)              # past Install another build, the self-test, the cleanup cells, the glow and statistics switches and the recorder: Read-only mode
    key("Return")
    shot("06a_read_only_off")  # untested firmware: the "own risk" warning
    key("Escape")
    key("Up")                  # Developer mode
    key("Return")
    shot("06b_dev_mode_off")
    key("Escape")
    key("Down", 7)             # Read-only, the recorder, the statistics and glow switches, the two cleanup cells: Show the diagnostic report
    key("Return")
    shot("06_dev_report")
    key("Escape")
    key("Down")                # Console self-test
    key("Return")
    time.sleep(1)
    shot("06_self_test")       # the console, the forwarders, the homebrew menu: each checked; the report written
    self_test = os.path.join(run_dir, "playguard_data", "self-test.md")
    if not os.path.exists(self_test) or not open(self_test).read().startswith("## PlayGuard console self-test"):
        fail("the self-test's report was not written in " + self_test)
    key("Return")              # OK
    key("Down")                # Compare the play-timer block: none saved yet
    key("Return")
    shot("06_dev_block")
    key("Right")               # Save the reference (read-only mode only writes to the SD card)
    key("Return")
    if not os.path.exists(block_ref):
        fail("no play-timer block reference in " + block_ref)
    time.sleep(4.5)            # its toast gone: the next screens are references
    # The cleanup tools, just above the report (read-only mode allows them:
    # nothing on the console changes): a confirmation, then a result screen.
    # The log files go but the upload links and the block reference just saved;
    # the cache's files go, its folder stays.
    key("Up", 3)               # the self-test, the report: Delete the log files
    key("Return")
    shot("06c_clear_logs")     # how many files, what goes, what stays; Delete in red
    key("Right")               # Delete
    key("Return")
    shot("06d_clear_logs_done")
    key("Return")              # OK
    if any(os.path.exists(p) for p in old_logs):
        fail("Delete the log files left " + repr([p for p in old_logs if os.path.exists(p)]))
    if not os.path.exists(uploads) or not os.path.exists(block_ref):
        fail("Delete the log files removed an upload link or the block reference")
    key("Up")                  # Clear the cache
    key("Return")
    shot("06e_clear_cache")
    key("Right")               # Delete
    key("Return")
    shot("06f_clear_cache_done")
    key("Return")              # OK
    if os.path.exists(stale_cache) or not os.path.isdir(cache_dir):
        fail("Clear the cache left " + stale_cache + " or removed the folder")
    log.flush()
    app_log = open(os.path.join(OUT, "app.log"), errors="replace").read()
    if "cleanup (clear cache)" not in app_log or "cleanup (clear logs)" not in app_log:
        fail("no cleanup line in the log; messages: " + repr(messages()))
    key("Down", 6)             # the report, the self-test, the block, the firmware screen: Forget the firmware choice
    key("Return")
    shot("06g_forget")
    key("Right")               # Forget
    key("Return")
    if not any("firmware choice will be asked again" in m for m in messages()):
        fail("Forget the firmware choice did not say so; messages: " + repr(messages()))
    key("Up")                  # Show the firmware screen again
    key("Return")
    shot("07_gate_again")
    key("Return")              # Update (it has the focus): how? (none chosen yet)
    shot("07b_update_method")  # In PlayGuard first, recommended
    key("Down")                # Automatic (sphaira, else the App Store)
    key("Return")
    shot("08_open_store")      # open sphaira?
    key("Right")               # Open sphaira: PlayGuard hands over to hbloader and quits
    key("Return")
    for _ in range(20):
        if not alive():
            break
        time.sleep(0.5)
    else:
        fail("still running after handing over to sphaira")
    log.flush()
    if "next load: sdmc:/switch/sphaira/sphaira.nro" not in open(os.path.join(OUT, "app.log"), errors="replace").read():
        fail("sphaira was not set as the next homebrew")
    try:
        cfg = json.load(open(config_file))
    except (OSError, ValueError) as e:
        fail(f"no config saved: {e}")
    if cfg.get("dev_mode") is not True:
        fail("developer mode was not saved")
    if cfg.get("fw_gate_choice"):
        fail("a firmware choice was remembered without the box ticked")
    finish()
if DEVBUILD:
    # About: seven presses of Y on Version turn the developer mode on.
    key("Down", steps("dashboard", "about"))
    key("Right")
    key("y", 7)
    key("Left")
    key("Right")
    key_to_end("Down", hold=0.15) # its last line: the funding QR codes
    key("Up", 2)               # past Source: Help & diagnostics
    key("Return")
    key_to_end("Down", hold=0.15) # the Developer section, down to its last cell (Run the migrations again)
    key("Up")                  # Install another build
    key("Return")              # signed out, the releases and "Sign in"
    time.sleep(1)              # once fetched (the simulated GitHub takes 0.6 s an answer)
    shot("01_dev_builds_signed_out")
    key("Down", 2)             # past the two releases, to Sign in to GitHub
    key("Return")
    shot("02_github_code")     # the code and its QR code (the simulated GitHub approves at once)
    time.sleep(4)
    if not os.path.exists(token_file):
        fail("no GitHub token saved in " + token_file)
    shot("03_dev_builds")      # the same screen filled: the tabs, the release, how old the list is
    key("Up")                  # the tabs
    key("Right")               # main
    key("Return")
    shot("03a_dev_builds_main")   # main's commits: their subjects, kind of change, scope, pull request, checks
    key("Up")
    key("Left")                # back to the releases' tab, as the rest of the run expects
    key("Return")
    cache_file = os.path.join(run_dir, "playguard_data", "cache", "dev_builds.json")
    if not os.path.exists(cache_file) or len(json.load(open(cache_file))["builds"]) != 5:
        fail("the list was not kept in " + cache_file)
    def fetches():
        log.flush()
        return open(os.path.join(OUT, "app.log"), errors="replace").read().count("dev builds: fetching the list")
    fetched = fetches()
    key("Escape")              # closed, then opened again: the list kept is shown at once
    key("Return")              # on the release's tab, its build focused
    key("Up")                  # the tabs
    key("Right", 2)            # Pull requests (the keyboard has no L / R)
    key("Return")
    shot("03b_dev_builds_kept")  # the pull requests' tab, by number
    key("Return")              # A on that tab, shown: the other order
    key("Return")              # and back
    if messages()[-2:] != ["newest build first", "by number"]:
        fail("A on the pull requests' tab did not sort them: %s" % messages()[-2:])
    if fetches() != fetched:
        fail("a list fetched a moment ago was fetched again")
    key("Down")                # the pull request
    button_x()                 # Refresh
    shot("03c_dev_builds_loading")   # the bar over the list kept, while it is fetched again (2.4 s)
    time.sleep(3)
    if fetches() != fetched + 1:
        fail("X did not fetch the list again")
    key("Down")                # back to the controller after the mouse (nothing below the pull request)
    key("Return")              # the pull request, still focused (the screen updated in place)
    shot("04_dev_build_details")   # its date, checks, size, description, and Install
    key("Down", 3)             # below the buttons: the description, half a screen a press
    shot("04b_dev_build_description")   # GitHub's Markdown: headings, lists, tasks, an alert, code, a table
    key("Up", 3)               # back up to the buttons
    key("Left")                # Install
    key("Return")              # Install (focused)
    time.sleep(0.7)
    shot("05_dev_build_downloading")   # the progress bar and Cancel (the simulated download takes 4 s)
    key("Escape")              # B: cancelled; the download stops, nothing is replaced
    time.sleep(2.5)
    if not alive():
        fail("cancelling the download ended the app")
    if "Download cancelled." not in messages():
        fail("no word that the download was cancelled")
    if open(installed_nro, "rb").read() != b"the build before":
        fail("a cancelled download replaced " + installed_nro)
    key("Return")              # Install again (still focused on the build's page)
    for _ in range(20):
        if not alive():
            break
        time.sleep(0.5)
    else:
        fail("still running after installing the build")
    log.flush()
    if "next load: sdmc:" not in open(os.path.join(OUT, "app.log"), errors="replace").read():
        fail("the new build was not set as the next homebrew")
    if open(installed_nro, "rb").read() != pr_content:
        fail("the pull request's build did not replace " + installed_nro)
    # The build replaced is kept, its start checked (action/rollback_flow).
    if not os.path.exists(installed_nro + ".previous") or open(installed_nro + ".previous", "rb").read() != b"the build before":
        fail("the build replaced was not kept as " + installed_nro + ".previous")
    check = os.path.join(run_dir, "playguard_data", "boot_check.json")
    if not os.path.exists(check) or json.load(open(check)).get("starts") != 0:
        fail("the next start of the build installed is not checked")
    for leftover in (".old", ".new", ".new.zip", ".new.zip.part", ".new.zip.part.json"):
        if os.path.exists(installed_nro + leftover):
            fail("a %s file was left next to %s" % (leftover, installed_nro))
    finish()
if UPDATE:
    for _ in range(20):
        if logged("update: offering 1.1.0"):
            break
        time.sleep(0.5)
    else:
        fail("no update screen at start-up")
    shot("01_update_offer")    # 1.1.0, its date and notes; Update focused
    key("Down", 2)             # Later
    key("Return")
    shot("02_update_later")    # 1 day, 1 week, 30 days
    key("Down")                # 1 week
    key("Return")
    time.sleep(1)
    cfg = json.load(open(config_file))
    # The simulated console date is 2026-10-08.
    if cfg.get("update_snooze_version") != "1.1.0" or cfg.get("update_snooze_until") != "2026-10-15":
        fail("Later › 1 week was not saved: " + repr({k: v for k, v in cfg.items() if k.startswith("update")}))
    # About › Check for updates shows it again, postponed or not.
    key("Down", steps("dashboard", "about"))
    key("Right")
    key("Down")                # Check for updates
    key("Return")
    time.sleep(2)
    if sum("update: offering 1.1.0" in l for l in open(os.path.join(OUT, "app.log"), errors="replace")) < 2:
        fail("Check for updates did not show the update screen")
    key("Return")              # Update: how? (none chosen yet)
    shot("03_update_method")   # In PlayGuard (recommended) first
    key("Return")              # In PlayGuard
    shot("04_update_confirm")  # downloads, checks, replaces, restarts
    key("Right")               # Update
    key("Return")
    for _ in range(20):
        if not alive():
            break
        time.sleep(0.5)
    else:
        fail("still running after updating in place")
    log.flush()
    if "next load: sdmc:/switch/playguard/playguard.nro" not in open(os.path.join(OUT, "app.log"), errors="replace").read():
        fail("the new version was not set as the next homebrew")
    if open(installed_nro, "rb").read() != release_content:
        fail("the release did not replace " + installed_nro)
    if json.load(open(config_file)).get("update_via") != "inplace":
        fail("the way to update was not saved")
    finish()
if ERRORS:
    shot("01_limit_reached")   # Today: today's limit reached
    key("Down")                # Play timer: the state line says the limit is reached
    key("Right")               # the profile chip
    key("Right")               # Same limit every day
    key("Return")
    shot("02_picker")          # the days differ: "Custom…" is selected
    key("Up")                  # No play (0 min)
    key("Return")
    shot("03_gate")            # the change needs the temporary unlock
    key("Right")               # Unlock and apply
    key("Return")
    shot("04_unlock_failed")   # a dialog, not a toast
    key("Return")              # OK
    key("Left", 2)             # the profile chip, then the sidebar
    key("Down", steps("play_timer", "console"))
    key("Right")               # Network clock, its own screen
    key("Return")
    shot("05_clock_autosync_off")
    # No time server answers: measuring offers the console clock instead,
    # behind the same confirmation as a measured time.
    history_file = os.path.join(nand_dir, "history.json")
    def clock_entries():
        try:
            return [e for e in json.load(open(history_file)).get("entries", []) if e.get("kind") == "clock"]
        except (OSError, ValueError):
            return []
    clock_before = len(clock_entries())
    key("Down", 4)             # past Region, Server, Enter a server name: Measure
    key("Return")
    time.sleep(1)
    shot("06_clock_offline")   # the result's dialog: "No server answered", the console clock offered
    key("Right")               # Use the console clock
    key("Return")
    shot("07_clock_copy_confirm")
    key("Right")               # Set the clock
    key("Return")
    shot("08_clock_copied")
    key("Return")              # OK

    def told_unlock_failed():
        if not any(t.startswith("Could not unlock parental controls") for t in messages()):
            fail("no 'could not unlock' dialog; messages: " + repr(messages()))
        if len(clock_entries()) != clock_before + 1:
            fail("the network clock was not set to the console clock (no history entry)")
    finish(told_unlock_failed)
def main_opened():
    log.flush()
    return any("main screen opened" in l for l in open(os.path.join(OUT, "app.log"), errors="replace"))


if FORWARDERS:
    # Console › Forwarders: the one another app made (its details), PlayGuard
    # itself put on the HOME menu in one press, a homebrew of /switch from the
    # list (named as its .nro names itself) and protected with the PIN, then
    # both deleted; each change in
    # forwarders.json (PlayGuard's own) and in the change history.
    made_file = os.path.join(nand_dir, "forwarders.json")
    history_file = os.path.join(nand_dir, "history.json")
    def made():
        try:
            return [(e.get("id"), e.get("name")) for e in json.load(open(made_file)).get("forwarders", [])]
        except (OSError, ValueError):
            return []
    def protected():
        try:
            return [(e.get("name"), e.get("gate")) for e in json.load(open(made_file)).get("forwarders", []) if e.get("pin")]
        except (OSError, ValueError):
            return []
    def recorded(kind):
        try:
            return [e.get("detail", "") for e in json.load(open(history_file)).get("entries", []) if e.get("kind") == kind]
        except (OSError, ValueError):
            return []
    self_id = "050537e2542e2000"   # forwarder_id() of sdmc:/switch/playguard/playguard.nro
    key("Down", steps("dashboard", "console"))
    key("Right")               # Network clock
    key("Down", 2)             # Forwarders, under Homebrew
    shot("00_console")
    key("Return")
    shot("01_forwarders")      # Save Keeper (another app's), PlayGuard to add, no signature patches
    key("Return")              # Save Keeper: Details or Delete
    shot("02_actions_elsewhere")
    key("Return")              # Details
    shot("03_details_elsewhere")
    tap(640, 160)              # above the sheet: a touch outside it closes it
    key("Right")               # after a touch, a first press only shows the focus again
    if focused() != "Save Keeper":
        fail("a touch outside the details did not close them: the focus on " + repr(focused()))
    key("Down")                # PlayGuard, to add
    key("Return")
    shot("04_add_self")        # the confirmation, with its .nro
    key("Right")               # Add
    key("Return")
    time.sleep(3)              # the toast gone
    if made() != [(self_id, "PlayGuard")]:
        fail("PlayGuard's forwarder was not remembered: " + repr(made()))
    if recorded("add_forwarder") != ["PlayGuard (" + self_id + ")"]:
        fail("the forwarder added is not in the change history: " + repr(recorded("add_forwarder")))
    shot("05_self_added")      # PlayGuard on the HOME menu with its icon, the focus on it
    key("Down", 3)             # Save Keeper, PlayGuard's line, Another homebrew
    key("Return")
    shot("06_pick")            # the made-up homebrew of /switch, PlayGuard's own among them
    key("Down", 5)             # Star Atlas, after sphaira
    key("Return")
    shot("07_name")            # its name from its .nro, to change or keep
    key("Return")              # kept
    shot("08_confirm")         # what the HOME menu will show: its icon, name and publisher
    key("Right")               # Add
    key("Return")
    time.sleep(3)
    if [n for _, n in made()] != ["PlayGuard", "Star Atlas"]:
        fail("Star Atlas's forwarder was not remembered: " + repr(made()))
    shot("09_two")             # by group: PlayGuard, Star Atlas (without the PIN), Save Keeper (another app's)
    key("Down")                # Star Atlas
    key("Return")
    shot("10_actions")         # the sheet: its icon; Details, Rename, Change the publisher, Change the icon; Ask for the PIN (off); Install again, Delete
    go_to("Ask for the PIN")   # its switch, off
    key("Return")
    shot("11_pin_confirm")     # what it asks, and the homebrew menu still starting it
    key("Right")               # Protect with the PIN
    key("Return")
    time.sleep(3)
    if protected() != [("Star Atlas", "sdmc:/switch/playguard/playguard.nro")]:
        fail("Star Atlas was not protected with the PIN: " + repr(protected()))
    if len(recorded("protect_forwarder")) != 1 or [n for _, n in made()] != ["PlayGuard", "Star Atlas"]:
        fail("the protection is not in the change history: " + repr(recorded("protect_forwarder")))
    shot("12_protected")       # Star Atlas: On the HOME menu · PIN, the focus on it
    key("Return")
    go_to("Delete")
    key("Return")
    key("Right")               # Delete, in the confirmation
    key("Return")
    time.sleep(3)
    if [n for _, n in made()] != ["PlayGuard"] or len(recorded("remove_forwarder")) != 1:
        fail("Star Atlas's forwarder was not deleted: " + repr(made()) + " " + repr(recorded("remove_forwarder")))
    key("Down")                # Save Keeper
    key("Return")
    go_to("Delete")
    key("Return")
    key("Right")
    key("Return")
    time.sleep(3)
    if len(recorded("remove_forwarder")) != 2 or made() != [(self_id, "PlayGuard")]:
        fail("the other app's forwarder was not deleted: " + repr(recorded("remove_forwarder")))
    shot("13_left")            # PlayGuard alone
    finish()
if HBMENU:
    # Console › Forwarders › Lock the homebrew menu: refused with no way back,
    # then PlayGuard's forwarder and sphaira's (protected with the PIN, free
    # hours on the weekend) made, Star Atlas left unprotected and protected
    # on the way to the lock, the check, unlocked.
    made_file = os.path.join(nand_dir, "forwarders.json")
    history_file = os.path.join(nand_dir, "history.json")
    rules_file = os.path.join(run_dir, "playguard_data", "gate_rules.txt")
    def recorded(kind):
        try:
            return [e for e in json.load(open(history_file)).get("entries", []) if e.get("kind") == kind]
        except (OSError, ValueError):
            return []
    def override():
        try:
            return open(sd_override).read()
        except OSError:
            return ""
    def pinned(flag="pin"):
        return [e.get("name") for e in json.load(open(made_file)).get("forwarders", []) if e.get(flag)]
    restored = recorded("hbmenu_restored")
    if len(restored) != 1 or restored[0].get("detail") != "Atmosphère 1.9.4 → 1.9.5" or \
            os.path.exists(os.path.join(run_dir, "playguard_data", "hbmenu_lock_report.txt")):
        fail("the sysmodule's report was not taken: " + repr(restored))
    refused = recorded("gate_refused")   # the evening's three in one entry
    if len(refused) != 1 or refused[0].get("detail") != "sphaira · 3 times (21:03, 21:04, 21:05)" or \
            refused[0].get("when") != "2026-10-07 21:03":
        fail("the gate's log was not taken: " + repr(refused))
    if len(recorded("hbmenu_lock_removed")) != 1 or os.path.exists(hbmenu_state):
        fail("the lock taken away outside PlayGuard was not told")
    key("Down", steps("dashboard", "console"))
    key("Right")               # Network clock
    key("Down", 2)             # Forwarders, under Homebrew
    key("Return")
    key("Down", 4)             # PlayGuard's line, Another homebrew, Install them all again, Lock the homebrew menu
    key("Return")
    shot("01_not_ready")       # the steps: PlayGuard's forwarder next, then a protected menu, then started once
    if override():
        fail("the homebrew menu was locked without a way back: " + repr(override()))
    key("Return")              # Next step: PlayGuard on the HOME menu
    key("Right")               # Add
    key("Return")
    time.sleep(3)              # the steps again, a protected menu next
    key("Return")              # Next step: sphaira, found in /switch, made protected
    key("Return")              # its name kept
    shot("01_preview")         # sphaira as the HOME menu will show it, the PIN it asks
    key("Right")               # Add
    key("Return")
    time.sleep(3)
    if pinned() != ["sphaira"]:
        fail("sphaira was not made protected with the PIN: " + repr(pinned()))
    shot("01_steps")           # the steps again: sphaira to start once from the HOME menu
    key("Escape")              # later
    key("Up", 5)               # Install them all again, Another homebrew, PlayGuard's line, Save Keeper, sphaira
    key("Return")              # sphaira's actions
    go_to("Hours without the PIN")
    key("Return")
    key("Down", 3)             # Saturday and Sunday
    key("Return")
    key("Return")              # from 09:00
    key("Return")              # until 20:00
    time.sleep(3)
    # In the forwarder, made again (its gate's argv): the SD card's rules file is gone.
    sphaira = [e for e in json.load(open(made_file)).get("forwarders", []) if e.get("name") == "sphaira"]
    if not sphaira or sphaira[0].get("free") != {"days": 65, "from": 540, "to": 1200} or \
            not sphaira[0].get("carried") or os.path.exists(rules_file) or len(recorded("gate_hours")) != 1:
        fail("sphaira's free hours were not set: " + repr(sphaira))
    shot("02_hours")           # sphaira: On the HOME menu · PIN, not asked Sat–Sun 09:00–20:00
    key("Down", 5)             # Save Keeper, PlayGuard's line, Another homebrew, Install them all again, Lock
    key("Return")              # the steps: sphaira never started from the HOME menu
    if override():
        fail("the homebrew menu was locked before sphaira was seen working: " + repr(override()))
    key("Return")              # Next step: how
    key("Return")              # OK
    # sphaira started from the HOME menu, the PIN entered at its gate: the
    # gate's log, read on the way to the lock.
    open(os.path.join(run_dir, "playguard_data", "gate_log.txt"), "w").write(
        "2026-10-07 18:00\tpin\tsdmc:/switch/sphaira/sphaira.nro\n")
    key("Up", 2)               # Install them all again, Another homebrew
    key("Return")
    key("Down", 5)             # Star Atlas, after sphaira
    key("Return")
    key("Return")
    key("Right")
    key("Return")
    time.sleep(3)
    key("Down", 7)             # sphaira, Star Atlas, Save Keeper, PlayGuard's line, Another homebrew, Install them all again, Lock
    key("Return")
    shot("03_unprotected")     # Star Atlas not protected: protect them all first, or lock as it is
    key("Return")              # Protect them all, then lock
    key("Right")               # Protect, in its confirmation
    key("Return")
    time.sleep(3)
    time.sleep(3)              # the toast of the protection gone
    shot("04_confirm")         # the checks, Save Keeper still open, the way back from a computer
    key("Right")               # Lock
    key("Return")              # a press is not enough
    time.sleep(1)
    if override():
        fail("the homebrew menu was locked without A held: " + repr(override()))
    key("Return", hold=2.6)    # held 2 s
    time.sleep(3)
    shot("04_rescue")          # the rescue sheet: a computer, the rescue module, the help's QR code
    key("Return")              # OK
    if "<svg" not in open(os.path.join(run_dir, "playguard_data", "rescue.html")).read():
        fail("the rescue sheet's page to print was not written")
    if sorted(pinned()) != ["Star Atlas", "sphaira"]:
        fail("Star Atlas was not protected on the way to the lock: " + repr(pinned()))
    if pinned("verified") != ["sphaira"]:
        fail("sphaira was not seen working through its gate: " + repr(pinned("verified")))
    text = override()
    if "; >>> PlayGuard: the homebrew menu is locked" not in text or "override_any_app=false" not in text:
        fail("override_config.ini is not locked: " + repr(text))
    if not os.path.exists(hbmenu_state) or len(recorded("lock_hbmenu")) != 1:
        fail("the lock's state file or its history entry is missing")
    key("Down", 4)             # Protect every forwarder, Pause after refused PINs, Tries this week, Check the homebrew menu
    key("Return")
    shot("05_check")           # the album and R closed, the ways back, the module
    key("Return")              # OK
    key("Up")                  # Tries this week
    key("Return")
    shot("08_week")            # sphaira: refused 3 times on the 7th, opened once
    key("Return")              # OK
    key("Up", 9)               # Pause after refused PINs, Protect every forwarder, Lock, Install them all again, Another homebrew, PlayGuard's line, Save Keeper, Star Atlas, sphaira
    key("Return")
    go_to("Delete")
    key("Return")              # refused: the last way back through a menu
    shot("06_guard")
    key("Return")              # OK
    if "sphaira" not in pinned():
        fail("sphaira, the last way back, was deleted while locked: " + repr(pinned()))
    key("Down", 6)             # Star Atlas, Save Keeper, PlayGuard's line, Another homebrew, Install them all again, Lock
    key("Return")              # unlocked at once (the PIN asked)
    time.sleep(3)
    text = override()
    if "the homebrew menu is locked" in text or "override_any_app=true" not in text or "010000000000100D" not in text:
        fail("override_config.ini is not unlocked: " + repr(text))
    if os.path.exists(hbmenu_state) or len(recorded("unlock_hbmenu")) != 1:
        fail("the state file stayed, or the unlock is not in the history")
    shot("07_unlocked")
    finish()
if HOMEBREW:
    # A pause lifted, hours it never opens, the pause's rule, an icon from the
    # album, the rescue sheet's page to print: each in the gate's files,
    # forwarders.json and the change history.
    made_file = os.path.join(nand_dir, "forwarders.json")
    history_file = os.path.join(nand_dir, "history.json")
    def recorded(kind):
        try:
            return [e for e in json.load(open(history_file)).get("entries", []) if e.get("kind") == kind]
        except (OSError, ValueError):
            return []
    def data(name):
        try:
            return open(os.path.join(data_dir, name)).read()
        except OSError:
            return ""
    key("Down", steps("dashboard", "console"))
    key("Right")               # Network clock
    key("Down", 2)             # Forwarders, under Homebrew
    key("Return")
    shot("01_in_use")          # the card, the groups, the badges, sphaira paused
    key("Down")                # sphaira
    key("Return")
    shot("02_actions")         # the sheet: this forwarder, its protection (Lift the pause until …, the hours set), Install and delete
    go_to("Lift the pause")
    key("Return")
    time.sleep(3)
    if "sphaira" in data("gate_pause.txt") or len(recorded("gate_pause_lifted")) != 1:
        fail("sphaira's pause was not lifted: " + repr(data("gate_pause.txt")))
    key("Return")              # sphaira's actions again
    go_to("Hours it never opens")
    key("Return")
    key("Down")                # every day
    key("Return")
    key("Return")              # from 21:00
    key("Return")              # until 07:00
    time.sleep(3)
    def made(name):
        found = [e for e in json.load(open(made_file)).get("forwarders", []) if e.get("name") == name]
        return found[0] if found else {}
    # In the forwarder (it carries its rules): not in the SD card's file.
    if made("sphaira").get("closed") != {"days": 127, "from": 1260, "to": 420} or data("gate_closed.txt") or \
            len(recorded("gate_closed_hours")) != 1:
        fail("sphaira's closed hours were not set: " + repr(made("sphaira")))
    key("Return")              # sphaira's actions again
    go_to("Hours without the PIN")
    key("Return")
    key("Down", 4)             # Monday to Friday, and the weekend apart
    key("Return")
    key("Return")              # Monday to Friday: from 16:00
    key("Return")              # until 19:00
    key("Return")              # the weekend: from 09:00
    key("Return")              # until 20:00
    time.sleep(3)
    if made("sphaira").get("free") != {"days": 62, "from": 960, "to": 1140} or \
            made("sphaira").get("free2") != {"days": 65, "from": 540, "to": 1200}:
        fail("sphaira's two spans were not set: " + repr(made("sphaira")))
    key("Return")              # sphaira's actions again
    go_to("Closed once today's play time is used up")   # its switch, off
    key("Return")
    time.sleep(3)
    if not made("sphaira").get("limit") or len(recorded("gate_limit_set")) != 1:
        fail("sphaira's limit was not set: " + repr(made("sphaira")))
    time.sleep(3)              # the toast gone
    shot("05_rules")           # sphaira: its rules as icons: the PIN (key), hours without it (open lock), closed hours (stop), the day's time (timer)
    key("Down", 8)             # Star Atlas, Save Keeper, PlayGuard's line, Another homebrew, Install them all again, Lock, Protect every forwarder, Pause after refused PINs
    key("Return")
    shot("03_pause")           # never, 3 tries, 5 tries (the default), 5 tries for an hour, 10 tries
    key("Down")                # 5 tries in a row: 1 h
    key("Return")
    time.sleep(3)
    if not data("gate_pause.txt").startswith("rule\t5\t60\n") or len(recorded("gate_pause")) != 1:
        fail("the pause's rule was not written: " + repr(data("gate_pause.txt")))
    key("Up", 7)               # Protect every forwarder, Lock, Install them all again, Another homebrew, PlayGuard's line, Save Keeper, Star Atlas
    key("Return")
    go_to("Change the icon")
    key("Return")
    shot("04_icon")            # its .nro's own, then the album's screenshots, the newest first
    key("Down")                # 2026-10-20 18:30
    key("Return")
    time.sleep(3)
    icons = [e.get("icon") for e in json.load(open(made_file)).get("forwarders", []) if e.get("name") == "Star Atlas"]
    if icons != ["sdmc:/Nintendo/Album/2026/10/20/2026102018300000-57B4628D2267231D57E0FC1078C0596D.jpg"]:
        fail("Star Atlas's icon from the album was not kept: " + repr(icons))
    key("Down", 9)             # Save Keeper, PlayGuard's line, Another homebrew, Install them all again, Lock, Protect every forwarder, Pause after refused PINs, Tries this week, Check
    key("Return")
    key("Left")                # Rescue sheet
    key("Return")
    time.sleep(2)
    shot("05_rescue")          # the sheet, its page to print on the SD card
    page = data("rescue.html")
    if "<svg" not in page or "override_config.ini" not in page:
        fail("the rescue sheet's page was not written: " + repr(page[:200]))
    key("Return")              # OK
    finish()
if LIBRARY:
    def listed():
        log.flush()
        return [l.split("games list: ", 1)[1].strip()
                for l in open(os.path.join(OUT, "app.log"), errors="replace") if "games list: " in l]
    key("Down", steps("dashboard", "activity"))
    key("Right")               # Every account, the first chip
    key_to_end("Down", hold=0.15)   # the end of the tab, the focus on its last line (the QR code)
    key("Up", 3)               # scrolled back to that line, then All games
    key("Return")
    shot("01_games")           # the covers, the first one's figures on top
    if not listed() or not listed()[-1].startswith("50 of ") or not listed()[-1].endswith("(grid)"):
        fail("All games did not open on its first 50 games as covers: " + repr(listed()))
    key("Down", 8)             # the ninth row: the 49th and 50th covers, then "Show every game"
    key("Right", 2)
    shot("02_show_every_game")
    key("Return")
    shot("03_every_game")      # the first cover added, selected
    total = listed()[-1].split(" of ")[1].split()[0]
    if listed()[-1] != f"{total} of {total} games (grid)":
        fail("\"Show every game\" did not list them all: " + repr(listed()))
    key("y")                   # the list instead, on the same game
    shot("04_list")
    if listed()[-1] != f"{total} of {total} games (list)":
        fail("Y did not switch to the list: " + repr(listed()))
    finish()
if OUTSIDE:
    def log_text():
        log.flush()
        return open(os.path.join(OUT, "app.log"), errors="replace").read()
    shot("01_outside")         # the amber notice: play time started over, clock, limits
    try:
        kinds = [e.get("kind") for e in json.load(open(history_file)).get("entries", [])]
    except (OSError, ValueError) as e:
        fail(f"no history written: {e}")
    for kind in ("outside_reset", "outside_clock", "outside_limits", "new_account"):
        if kinds.count(kind) != 1:
            fail(f"{kind} recorded {kinds.count(kind)} times in the history: {kinds!r}")
    # The time left stays the same: after 30 s on screen
    # (PLAYGUARD_SIM_STALL_S, above), Today says the console is not counting.
    for _ in range(150):
        if "timer health: not counting" in log_text():
            break
        if not alive():
            fail("app exited while waiting for the timer health check")
        time.sleep(1)
    else:
        fail("Today never said the console is not counting")
    time.sleep(6)              # the next refresh shows it
    shot("02_not_counting")
    key("y")                   # the notification bell's list (the keyboard's Y): both, the
    # simulated network clock (not accurate) and PIN (1234) to do; the broken
    # games under Console, on one line.
    if not logged("notices: 4 to do") or not logged("1 about the console to do"):
        fail("the notification bell does not hold four notices to do and one about the console")
    shot("03_notices")
    key("Down", 3)             # the weak PIN, the last to do: X ignores it, once confirmed
    button_x()
    # Right twice: after the mouse's Ⓧ, borealis takes the first key press
    # only to show the focus again.
    key("Right", 2)            # Ignore
    key("Return")
    if not logged("notices: pin_weak ignored"):
        fail("the weak PIN was not ignored from the notification bell's list")
    time.sleep(1)              # written in the background
    if "pin_weak:4:" not in open(config_file).read():
        fail("the weak PIN ignored is not in config.json (notices_acked)")
    # The rebuilt list has the focus on its first entry, but after the mouse's
    # Ⓧ borealis does not scroll to it: the list stays where it was, clamped
    # to its end on whichever frame the new layout landed, a fraction of a
    # pixel apart from one run to the next. Down to the end (past the last
    # entry, a press does nothing) scrolls it from the settled layout.
    key("Down", 10)
    shot("03_notices_ignored")  # under "Ignored", last
    key("Escape")
    key("Down")                # Play timer: its state line says it too
    key("Right")
    shot("04_play_timer_not_counting")
    key("Down")                # off the week chart (←/→ pick a day there)
    key("Left")
    key("Up")                  # back to Today, on the notice's Dismiss line
    key("Right")
    key("Return")
    shot("05_dismissed")
    try:
        watch = json.load(open(watch_file))
    except (OSError, ValueError) as e:
        fail(f"watch.json unreadable: {e}")
    if watch.get("notice"):
        fail("the notice was not dismissed in " + watch_file)
    if [a.get("new", False) for a in watch.get("accounts", [])] != [False, True]:
        fail("Léo was not kept as a new account in " + watch_file)
    key("Left")                # the sidebar, then Activity: Léo flagged under the account filter
    key("Down", steps("dashboard", "activity"))
    key("Right")
    shot("06_new_account")
    finish()
if CADRAN:
    shot("01_today")
    for i, tab in enumerate(tabs[1:], start=2):
        key("Down")
        key("Right")
        shot(f"{i:02d}_{tab}")
        key("Down", 12)        # scroll through the whole tab
        shot(f"{i:02d}_{tab}_end")
        key("Left")            # back to the sidebar

    # The child screen, then back to the parent's with the PIN (the simulated
    # PIN screen accepts at once).
    key("Up", steps("dashboard", tabs[-1]))
    key("Right")               # Extra time, the hero's first action
    key("Down")                # Stop for today
    key("Right")               # Show the child
    key("Return")
    time.sleep(1)              # the screen's own entrance over
    shot("11_child")
    key("Return")              # A: back to the parent, the PIN first
    time.sleep(1)
    if not alive() or not logged("pctl_ask_pin returned"):
        fail("the child screen did not ask for the PIN to go back")
    shot("12_today_again")

    # The week edited with the stick: Monday +30 min, saved with the unlock the
    # running timer needs.
    key("Left", 3)             # the sidebar (Escape there would ask to quit)
    key("Down", steps("dashboard", "play_timer"))
    key("Right")               # the profile chip
    key("Down")                # the week, on Monday
    key("Return")              # A: edit the day
    key("Up", 2)               # +30 min
    shot("13_week_edit")
    key("Return")              # A: done
    key("Down")                # Discard, then Save
    key("Right")
    key("Return")
    shot("14_week_confirm")
    key("Right")               # Unlock and apply
    key("Return")
    time.sleep(1)
    written = json.load(open(os.path.join(ROOT, "resources", "i18n", "en-US", "playguard.json")))["play_timer"]["written_days"]
    if not any(m.startswith(written) for m in messages()):
        fail("the week's change was not saved: " + repr(messages()[-3:]))
    time.sleep(3.5)            # the toast gone: the next screen is a reference
    shot("15_week_saved")

    # A restriction level from its tile: the confirmation, cancelled.
    key("Left", 2)
    key("Down", steps("play_timer", "restrictions"))
    key("Right")               # Young child, the first tile
    key("Right", 2)            # Teen
    key("Return")
    shot("16_level_confirm")
    key("Escape")

    # Preferences › Text size: its three choices.
    key("Left", 3)
    key("Down", steps("restrictions", "preferences"))
    key("Right")               # Language, the first tile
    key("Right", 2)            # Text size
    key("Return")
    shot("17_text_size")
    key("Escape")

    # Console › Network clock: both clocks large.
    key("Left", 3)
    key("Up", steps("console", "preferences"))
    key("Right")               # Network clock, the first line
    key("Return")
    time.sleep(1)              # the screen's own entrance over
    shot("18_clock")
    key("Escape")

    # Activity: a game's own screen from its card.
    key("Left")
    key("Up", steps("activity", "console"))
    key("Right")               # Every account, the first chip
    key("Down")                # the first game's card
    key("Return")
    time.sleep(1)              # the screen's own entrance over
    shot("19_game")
    key("Escape")

    # The notification bell's list (Y, the keyboard's y), its entries in cards.
    key("y")
    time.sleep(1)              # the screen's own entrance over
    shot("20_notices")
    key("Escape")

    # Preferences › Theme and Language, applied at once: every screen made
    # again in the light theme, then in French, on Preferences, the focus on
    # the line it was on, no restart.
    key("Left")
    key("Down", steps("activity", "preferences"))
    key("Right")               # Language, the first tile
    key("Right")               # Theme
    key("Return")
    key("Up")                  # Light, above Dark
    key("Return")
    time.sleep(1)
    shot("21_live_theme")      # the focus back on Theme
    key("Left")                # Language
    key("Return")
    key("Down", 2)             # Français, after the console's and English
    key("Return")
    time.sleep(1)
    shot("22_live_language")
    # To Chinese, with its own font on the console, still without a restart
    # (no screenshot: the desktop's font has no simplified han).
    key("Return")
    key("Down", 12)            # 简体中文, after Français
    key("Return")
    time.sleep(1)
    if json.load(open(config_file)).get("language") != "zh-Hans":
        fail("the language was not set to Chinese")
    finish()
if CADRANSTATES:
    # Today and the Play timer in the states "alarm" and "outside" set up:
    # the time over the limit (a warning only) and the changes made outside
    # PlayGuard.
    shot("01_today")
    key("Down")
    key("Right")
    shot("02_play_timer")
    finish()
if CADRANUNLOCKED:
    # Parental controls temporarily unlocked: Today offers to turn them back
    # on first, the pill says so on every tab.
    key("Right")               # Today's first action: Turn back on
    shot("01_today")
    key("Left")
    key("Down")
    key("Right")
    shot("02_play_timer")
    finish()
if CADRANLARGE:
    # Larger text: Today still on one screen, the other tabs' tops laid out
    # again, the child screen at its own sizes.
    shot("01_today")
    key("Down")                # Play timer
    key("Right")
    shot("02_play_timer")
    key("Left")
    key("Down")                # Activity
    key("Right")
    shot("03_activity")
    key("Left")
    # The list screens' tiles, cards and notes, and a screen of its own.
    key("Down", steps("activity", "console"))
    key("Right")
    shot("05_console")
    key("Left")
    key("Down", steps("console", "preferences"))
    key("Right")
    shot("06_preferences")
    key("Left")
    key("Down", steps("preferences", "about"))
    key("Right")
    shot("07_about")
    key_to_end("Down", hold=0.15) # its last line: the funding QR codes
    key("Up", 2)               # past Source: Help & diagnostics
    key("Return")
    time.sleep(0.6)
    shot("08_help")            # its title after where it opened from
    key("Escape")
    key("Left")
    key("Up", steps("dashboard", "about"))   # Today
    key("Right")               # Extra time, the hero's first action
    key("Down")                # Stop for today
    key("Right")               # Show the child
    key("Return")
    time.sleep(1)              # the screen's own entrance over
    shot("04_child")
    finish()
if BEDTIME:
    shot("01_today")
    key("Right")               # Extra time, the hero's first action
    key("Down")                # Stop for today
    key("Right")               # Show the child
    key("Return")
    time.sleep(1)              # the screen's own entrance over
    shot("02_child")           # the ring: 1 h until bedtime
    finish()
if ALARM:
    shot("01_overview_over")   # Today: the time over the limit, not a time left
    key("Down")                # Play timer: the state line and the banner say so; its pill, not Today's
    shot("02_play_timer_over")
    finish()
if SYSNOTES:
    def wait_logged(text, what):
        for _ in range(60):
            if logged(text):
                return
            if not alive():
                fail(f"app exited while waiting for {what}")
            time.sleep(0.5)
        fail(f"no {what} in 30 s")
    wait_logged("release notes: lists kept", "release lists from the start-up check")
    key("Down", steps("dashboard", "console"))
    key("Right")               # Network clock
    shot("01_tools_console")   # the tiles: 2 versions behind, and 23.0.1 needs hekate 6.5.4 first
    key("Up")                  # Firmware, the first tile
    key("Return")
    wait_logged("release notes: firmware fetched", "firmware notes")
    shot("02_firmware_next")   # the closest newer one, 23.0.0: security fixes, parental controls changed
    key("Left")                # the installed one's tab
    key("Return")
    shot("03_firmware_installed")
    key("Escape")
    key("Right")               # Atmosphère
    key("Return")
    # Its list came with the start-up check, today: shown, not fetched again.
    wait_logged("release notes: atmosphere opened", "Atmosphère notes")
    shot("04_atmosphere")
    key("Escape")
    key("Right")               # hekate
    key("Return")
    wait_logged("release notes: hekate opened", "hekate notes")
    shot("05_hekate")
    # PlayGuard's own, from About › Version (its first line): the closest
    # newer one first.
    key("Escape")
    key("Down")                # off the tiles' row
    key("Left")
    key("Down", steps("console", "about"))
    key("Right")
    key("Return")
    wait_logged("release notes: playguard fetched", "PlayGuard notes")
    shot("06_playguard_next")  # 1.1.0, newer: its highlights and changes, not its download or install lines
    key("Left")                # the installed one's tab
    key("Return")
    shot("07_playguard_installed")

    def check_cache():
        if logged("release notes: atmosphere fetched") or logged("release notes: hekate fetched"):
            fail("Atmosphère's or hekate's notes fetched again on the day of the start-up check")
        try:
            kept = json.load(open(notes_cache))
        except (OSError, ValueError) as e:
            fail(f"the release notes were not kept: {e}")
        ams_kept = {r["version"]: r for r in kept["atmosphere"]["releases"]}
        if ams_kept.get("1.12.1", {}).get("supports") != "23.0.0" or "And the following" in ams_kept["1.12.1"]["notes"]:
            fail("Atmosphère 1.12.1 kept wrong: " + repr(ams_kept.get("1.12.1")))
        hek = {r["version"]: r for r in kept["hekate"]["releases"]}
        if hek.get("6.5.4", {}).get("supports") != "23.0.1" or "FAQ" in hek["6.5.4"]["notes"]:
            fail("hekate 6.5.4 kept wrong: " + repr(hek.get("6.5.4")))
        fw = {r["version"]: r for r in kept["firmware"]["releases"]}
        for v in ("22.5.0", "23.0.0", "23.0.1"):
            if not fw.get(v, {}).get("notes"):
                fail(f"no notes kept for firmware {v}")
        if not fw["23.0.0"].get("security") or not fw["23.0.0"].get("pctl") or fw["23.0.1"].get("security"):
            fail("firmware 23.0.0 / 23.0.1 flags kept wrong")
        pg_kept = {r["version"]: r for r in kept.get("playguard", {}).get("releases", [])}
        pg_notes = pg_kept.get("1.1.0", {}).get("notes", "")
        if "When time is up" not in pg_notes or "Download" in pg_notes or "Install" in pg_notes or "#134" in pg_notes:
            fail("PlayGuard 1.1.0 kept wrong: " + repr(pg_kept.get("1.1.0")))
    finish(check_cache)
if LOADING:
    shot("01_loading")         # the icon, the wordmark, the bar at its last step
    finish()
if LOCK:
    shot("01_unlocked")        # the PIN screen answered at once: Today
    if not main_opened():
        fail("the right PIN left the lock screen up; messages: " + repr(messages()))
    finish()
if FORGED:
    shot("01_recovery_unconfirmed")   # what the file says, no action offered
    def log_text():
        log.flush()
        return open(os.path.join(OUT, "app.log"), errors="replace").read()
    if "rescue report not confirmed" not in log_text():
        fail("a hand-written rescue report was trusted")
    if "pctl_ask_pin" in log_text():
        fail("the PIN was asked before leaving the recovery screen")
    key("Return")              # Open PlayGuard, the only thing left
    shot("02_opened")
    if "pctl_ask_pin returned" not in log_text():
        fail("an unconfirmed rescue report skipped the lock screen")
    if not main_opened():
        fail("the app did not open after the lock screen")
    finish()
if RESCUE:
    shot("01_recovery")        # the recovery screen, in place of the usual first screen
    key("Down", 2)             # past Set a new PIN / Delete: Open PlayGuard
    key("Return")              # proceed to the app
    shot("02_opened")          # Today: the app opened after the rescue
    if not alive():
        fail("app exited instead of opening after recovery")
    if not main_opened():
        fail("the app did not open after recovery")
    report_path = os.path.join(run_dir, "playguard_data", "rescue_report.txt")
    if os.path.exists(report_path):
        fail("the rescue report was not removed after it was read")
    if "rescue report confirmed by the console (proof valid" not in open(os.path.join(OUT, "app.log"), errors="replace").read():
        fail("the sysmodule's proof was not checked")
    try:
        if open(os.path.join(run_dir, "playguard_data", "rescue_nonce")).read().strip() == nonce:
            fail("the challenge was not replaced once the report was read")
    except OSError:
        fail("no new challenge after the report was read")

    def recorded():
        history = os.path.join(nand_dir, "history.json")
        try:
            entries = json.load(open(history)).get("entries", [])
        except (OSError, ValueError) as e:
            fail(f"no history written: {e}")
        if not any(e.get("kind") == "rescue" for e in entries):
            fail("recovery was not recorded in the change history")
        if not any(e.get("kind") == "relock" for e in entries):
            fail("\"Turn back on and quit\" did not lock again")

    # Still unlocked (the sysmodule's unlock): quitting asks to lock again.
    def asked_to_relock():
        log.flush()
        return "asking to lock again" in open(os.path.join(OUT, "app.log"), errors="replace").read()
    for _ in range(4):         # back to the sidebar first, then B twice
        if asked_to_relock():
            break
        key("Escape")
        time.sleep(0.3)
    time.sleep(4.5)            # the "Press B again" toast gone: the dialog is a reference
    shot("03_quit_unlocked")
    if not alive() or not asked_to_relock():
        fail("quitting while unlocked did not ask to lock again")
    key("Return")              # Turn back on and quit, the first button: focused
    for _ in range(50):
        if not alive():
            break
        time.sleep(0.1)
    if alive():
        fail("\"Turn back on and quit\" did not quit")
    if "exit to HOME" not in open(os.path.join(OUT, "app.log"), errors="replace").read():
        fail("\"Turn back on and quit\" went back to hbmenu, not to the HOME menu")
    finish(recorded)

if not GAMES:
    shot("01_dashboard")
    # Left (not Escape) goes back to the sidebar: Escape there asks to quit, so a
    # step that went wrong could close the app.
    for i, tab in enumerate(tabs[1:], start=2):
        key("Down")
        key("Right")
        shot(f"{i:02d}_{tab}")
        key("Down", 12)   # scroll through the whole tab
        shot(f"{i:02d}_{tab}_end")
        key("Left")       # back to the sidebar

    # Play timer: the extra-time picker, then the per-day editor from Weekdays
    # and weekend (its Monday–Friday picker first), a day's picker there and
    # an exact duration with the stepper, discarded.
    key("Up", steps("play_timer", tabs[-1]))   # from the last tab back to Play timer
    key("Right")               # the profile chip
    key("Down", 3)             # the week, the bedtime chip: Extra time today...
    key("Return")
    shot("20_extra_time")
    key("Escape")
    key("Down", 2)             # past No more play today: Weekdays and weekend
    key("Return")
    shot("21_split")           # the editor, Monday–Friday picker open
    key("Escape")              # no value: no weekend picker either
    shot("21_per_day")         # the editor, on Monday
    button_x()                 # Ⓧ: Monday's picker
    shot("19_day_picker")
    key("Down", 15)            # to the end of the list (No limit) …
    key("Up")                  # … then Enter a duration...: the stepper
    key("Return")
    shot("22_stepper")
    key("Up", 2)               # 2 h → 2 h 30
    key("Return")              # OK
    shot("22_staged")          # Monday: 2 h 30, drawn as unsaved; "+ Save (1)" in the footer
    key("Escape")              # unsaved: asks before leaving
    shot("23_discard")
    key("Right")               # Discard
    key("Return")
    shot("23_back")            # the tab, on Weekdays and weekend again

    # When the time is up: its two choices (on "Suspend the software"), and the
    # confirmation of the other one, cancelled.
    key("Up", 3)               # No more play today, Extra time today: the bedtime chip
    key("Right")               # When the time is up
    key("Return")
    shot("23_time_up_picker")
    key("Down")                # Warning only: a long confirmation, with the unlock
    key("Return")
    shot("23_time_up_confirm") # the text wraps inside the dialog (ui::dialog measures it)
    key("Escape")              # cancelled: nothing written

    # Bedtime: the alarm set with the stick (from 21:00), 21:30, confirmed with
    # the unlock the running timer needs; the tab must then show it.
    key("Left")                # the bedtime chip
    key("Return")
    shot("23_bedtime_picker")
    key("Up", 2)               # 21:30
    key("Return")              # OK
    shot("23_bedtime_confirm")
    key("Right")               # Unlock and apply
    key("Return")
    shot("23_bedtime_done")
    if not any(m.startswith("Bedtime alarm changed.") for m in messages()):
        fail("the bedtime alarm was not changed: " + repr(messages()[-3:]))

    # By day (Y in the bedtime dialog): the weekend alone at 22:30; the
    # weekdays keep 21:30, and the tab says the days differ.
    key("Return")
    key("y")                   # By day
    shot("23_bedtime_by_day")  # each set of days with its time
    key("Down")                # Saturday and Sunday
    key("Return")
    key("Up", 4)               # 21:30 → 22:30
    key("Return")              # OK
    key("Right")               # Unlock and apply
    key("Return")
    # The write and its toast come after the unlock, off the UI thread: a
    # slow runner may not have logged it yet.
    for _ in range(20):
        if sum(m.startswith("Bedtime alarm changed.") for m in messages()) >= 2:
            break
        time.sleep(0.5)
    else:
        fail("the weekend's bedtime alarm was not changed: " + repr(messages()[-3:]))
    time.sleep(4.5)            # its toast gone: the next screens are references

    # Play timer › Advanced: its own screen, the switch next to the actions it
    # shows; turned on, then off again (backups restore the alarm only with it on).
    key("Down", 7)             # past the day's lines, Allowed again at, the extra-time amounts and switch: Advanced
    key("Return")
    shot("23_advanced")
    def advanced_saved(on):
        # The config is written off the UI thread (ui::save_config).
        for _ in range(25):
            try:
                if bool(json.load(open(config_file)).get("advanced")) == on:
                    return True
            except (OSError, ValueError):
                pass
            time.sleep(0.2)
        return False
    key("Return")              # Show advanced play-timer actions: on
    shot("23_advanced_on")     # the "Time's up" alarm, pause and resume below it
    if not advanced_saved(True):
        fail("the advanced actions switch was not saved on")
    key("Return")              # and off again
    if not advanced_saved(False):
        fail("the advanced actions switch was not saved off")
    key("Escape")              # back to the Play timer tab

    # Settings backup: save one, open the list and the restore summary (cancelled).
    key("Left")                # back to the sidebar
    key("Down", steps("play_timer", "preferences"))
    key("Right")               # Language, the first tile
    key("Down", 9)             # past the effects, the intro and the notifications: Changes made with PlayGuard, then Back up the settings, the first tile
    backups = os.path.join(nand_dir, "backups")
    before = len(os.listdir(backups)) if os.path.isdir(backups) else 0
    key("Return")              # Back up the settings
    shot("24_backup_saved")
    if (len(os.listdir(backups)) if os.path.isdir(backups) else 0) != before + 1:
        fail("Return on 'Back up the settings' saved no backup (wrong cell focused?)")
    key("Right")               # Restore a backup...
    key("Return")
    shot("25_backup_list")
    key("Return")
    shot("26_backup_restore")
    key("Escape")
    shot("27_back")

    # The change history (the bedtime alarm changed twice above): every change,
    # then only the refused ones (none: the line hidden), then every change
    # again (the same line shown again).
    def history_listed(text, times=1):
        """Whether the list was built `times` times showing `text` (logged)."""
        for _ in range(20):
            log.flush()
            if sum(("history list: " + text) in l for l in open(os.path.join(OUT, "app.log"), errors="replace")) >= times:
                return True
            time.sleep(0.25)
        return False
    key("Up")                  # Changes made with PlayGuard
    key("Return")
    shot("27_history")
    if not history_listed("2 of 2 changes"):
        fail("the change history did not list the bedtime alarm changes")
    key("Return")              # Show: the kinds of change
    key("Down", 5)             # Refused by the console
    key("Return")
    shot("27_history_refused")
    if not history_listed("0 of 2 changes"):
        fail("the change history did not filter the refused changes")
    key("Return")
    key("Up", 5)               # Everything
    key("Return")
    if not history_listed("2 of 2 changes", times=2):
        fail("the change history did not list every change again")
    shot("27_history_again")
    key("Escape")
    finish()

# The "games" scenario, from the sidebar on Today.
# Send a report online: signing in to GitHub first, the confirmation, then the link and its QR codes.
uploads = os.path.join(run_dir, "playguard_data", "logs", "uploads")
def upload_count():
    # One file per link (logs/uploads/upload_<date>.url, an Internet shortcut); a run pruned by the
    # default limit (the 20 newest) counts the same.
    if not os.path.isdir(uploads):
        return 0
    return sum(1 for f in os.listdir(uploads) if f.startswith("upload_") and f.endswith(".url")
               and "https://" in open(os.path.join(uploads, f)).read())
uploads_before = upload_count()
reports = os.path.join(run_dir, "playguard_data", "logs", "reports")
def saved_reports():
    return [f for f in (os.listdir(reports) if os.path.isdir(reports) else []) if f.startswith("report_") and f.endswith(".txt")]
saved = saved_reports()
key("Down", steps("dashboard", "about"))
key("Right")
key_to_end("Down", hold=0.15) # its last line: the funding QR codes
key("Up", 2)               # past Source: Help & diagnostics
key("Return")
key("Right")               # past First steps: Send a report online, the tile beside it
key("Return")
if saved:                  # reports saved by an earlier local run: this report is the first choice
    key("Return")
shot("27_upload_sign_in")  # signed out of GitHub: signing in comes first
key("Escape")              # not now
if upload_count() != uploads_before:
    fail("a report was sent while signed out of GitHub")
# Signed in: what goes to the secret gist, then its link and QR codes.
open(github_token, "w").write("ghu_smoke")
key("Return")              # Send a report online (still focused)
if saved_reports():
    key("Return")          # this report, the first choice
shot("27_upload_confirm")
key("Right")               # Send
key("Return")
shot("27_upload_link")
if upload_count() != uploads_before + 1:
    fail("the gist link was not recorded in " + uploads)
key("Return")              # OK
os.remove(github_token)

# Installed games: the simulated console's broken games in four sections
# (fixed here, to check, to reinstall, firmware or SD card), the fixes of the
# first, the data of the second checked one game and then both at once (the
# section picked, the corrupt game kept as such), a removal of the games
# picked and one of a single game (recorded in the change history), the
# third all at once, the fourth.
key("Escape")              # back to About
key("Left")                # back to the sidebar
key("Up", steps("console", "about"))
key("Right")               # Network clock
key("Down")                # Check the installed games
os.remove(games_cache)     # made by the start-up check: this screen's first check, game by game
key("Return")
time.sleep(0.8)
shot("27_games_checking")  # the games shown as they are checked, with the count
time.sleep(3)              # 15 games, 150 ms each
shot("27_games")           # Fix here: Robot Factory, Moon Racer; To check: Forest Tales, an unnamed game; Reinstall; …
if not os.path.exists(games_cache):
    fail("the installed games check was not cached in " + games_cache)
# Opened again: the cache at once, nothing to check again.
key("Escape")
key("Return")
time.sleep(0.6)
shot("27_games_cached")
# The same games as covers, then back to the list.
key("Up", 7)               # Display, above the sections
key("Return")
time.sleep(0.6)
shot("27_games_grid")      # covers with their problem, by section
key("Return")
time.sleep(0.6)
# Robot Factory (the first game): only its update needs a newer firmware;
# removing the update (the fix, the first button) makes it work again.
key("Down", 2)             # past Several games at once
key("Return")
shot("27_games_update")    # Remove the update (version 2.1.0)
key("Return")              # the fix
key("Right")               # Remove
key("Return")
time.sleep(1.5)            # the toast follows the games read again
shot("27_games_update_removed")
if not any(m.endswith("works again.") and m.startswith("Robot Factory") for m in messages()):
    fail("the update removal did not fix the game: " + repr(messages()[-3:]))
# Moon Racer: the console asks for a removed update; accepting the installed
# version again (the fix, no question) makes it work again.
key("Up", 7)               # Display, above the sections
key("Down", 2)             # past Several games at once: the first game
key("Return")
shot("27_games_launch")
key("Return")              # Accept the installed version again
time.sleep(1.5)            # the toast follows the games read again
if not any(m.endswith("works again.") and m.startswith("Moon Racer") for m in messages()):
    fail("the launch version reset did not fix the game: " + repr(messages()[-3:]))
# Forest Tales: the console found corrupt data; its check runs in a dialog
# with its progress and Cancel, here cancelled.
key("Up", 7)
key("Down", 3)             # past Several games at once, Check these 2 games: the first game of To check
key("Return")              # icon, why, what to do
shot("27_games_details")   # Check the data (the fix, focused), Other actions
key("Return")              # Check the data (the fix)
key("Escape")              # B: cancel, well before its 4 s end
time.sleep(1)
if not any(m == "Check cancelled." for m in messages()):
    fail("the check was not cancelled: " + repr(messages()[-3:]))
# Both games of To check picked at once (the keyboard has no Y), then their
# data checked one after the other: Forest Tales corrupt, the unnamed game
# intact; both kept as such, out of To check.
key("Up", 7)               # Display, above the sections
key("Down")                # Several games at once
key("Return")
shot("27_games_select_menu")   # Select all of: To check (2), Reinstall (4), Firmware or SD card (1)
key("Return")
shot("27_games_selected")  # 2 games selected, their check marks
key("Return")
shot("27_games_selection_menu")   # Check their data (2), Remove these games (2), …
key("Return")              # Check their data
time.sleep(2.6)            # past the first seconds: the time left shows
shot("27_games_verifying_batch")  # game 1 of 2
time.sleep(8)
shot("27_games_verified_batch")   # 2 of 2 checked: 1 corrupt (Forest Tales), 1 intact
key("Return")              # Cancel: the corrupt game stays, as such
time.sleep(1.5)            # both read again
shot("27_games_after_verify")     # Forest Tales "Corrupt (checked)" in Reinstall, still picked
with open(games_cache, "rb") as f:
    if f.read(8)[4:8] != (2).to_bytes(4, "little"):
        fail("the games cache is not the version that keeps what the check found")
# The two games picked, removed at once (their list saved first).
key("Up", 7)
key("Down")                # 2 games selected
key("Return")
key("Down")                # Remove these games (2)
key("Return")
shot("27_games_remove_selected")
key("Right")               # Remove
key("Return")
time.sleep(2)
if not any(m.startswith("2 games removed.") for m in messages()):
    fail("the games picked were not removed: " + repr(messages()[-3:]))
# To reinstall: Garden Defenders (an update without its game), removed.
key("Up", 7)               # Display, above the sections
key("Down", 4)             # past Several games at once, Reinstall's buttons: Hedgehog Rush
shot("27_games_reinstall")
key("Return")
shot("27_games_actions")   # Remove the game (the fix), Other actions
key("Return")              # Remove the game from the console
key("Right")               # Remove
key("Return")
time.sleep(1.5)
shot("27_games_removed")
if not any(m.endswith("removed") for m in messages()):
    fail("the broken game was not removed: " + repr(messages()[-3:]))
# The rest of Reinstall: their list saved, then all of them removed at once
# (the list saved again first), each in the change history.
exports_dir = os.path.join(run_dir, "playguard_data", "exports")
key("Up", 2)               # past Hedgehog Rush: Save the list of these games, above the games
shot("27_games_reinstall_actions")
key("Return")
time.sleep(0.5)
# The newest: the games picked and removed above left one of their own.
lists = sorted(glob.glob(os.path.join(exports_dir, "games_to_reinstall_*.json")), key=os.path.getmtime)
if not lists or not os.path.exists(lists[-1][:-4] + "txt"):
    fail("the list of the games to reinstall was not saved in " + exports_dir)
with open(lists[-1], encoding="utf-8") as f:
    if "Ocean Explorer" not in f.read():
        fail("the saved list does not name Ocean Explorer: " + lists[-1])
# The same list to a secret gist (signed in): its link and QR code.
open(github_token, "w").write("ghu_smoke")
uploads_before = upload_count()
key("Right")               # Send the list online, next to it
key("Return")
shot("27_games_send_confirm")
key("Right")               # Send
key("Return")
shot("27_games_send_link")
if upload_count() != uploads_before + 1:
    fail("the gist link of the list was not recorded in " + uploads)
key("Return")              # OK
os.remove(github_token)
key("Right")               # Remove these 3 games, last
key("Return")
shot("27_games_remove_all")
key("Right")               # Remove
key("Return")
time.sleep(2)
shot("27_games_removed_all")
if not any(m.startswith("3 games removed.") for m in messages()):
    fail("the games to reinstall were not all removed: " + repr(messages()[-3:]))
# Firmware or SD card: Star Pilots, the last game.
key("Down", 4)
shot("27_games_system")
key("Escape")

# Activity: All games, one game's screen, then a PDF export to the (simulated) SD card.
exports = os.path.join(run_dir, "playguard_data", "exports")
def pdfs():
    return {f for f in (os.listdir(exports) if os.path.isdir(exports) else []) if f.endswith(".pdf")}
pdfs_before = pdfs()   # the run folder is kept between local runs
key("Left")                # back to the sidebar
key("Up", steps("activity", "console"))
key("Right")               # Every account, the first chip
key_to_end("Down", hold=0.15)   # the end of the tab, the focus on its last line (the QR code)
key("Up", 3)               # scrolled back to that line, then All games
key("Return")              # the covers, on the first game
shot("28_activity_games")
key("Return")
shot("28_activity_game")   # its own screen: icon, seven days, figures, accounts
key("Down", 8)             # scrolled to the accounts: Alice, Léo, and the time on no account
shot("28b_activity_game_accounts")
key("Escape")
key("y")                   # All games as a list
shot("28c_activity_games_list")
key("y")                   # covers again (the config keeps the choice)
key("Escape")
key_to_end("Up", hold=0.15)   # the top: the accounts' chips
key_to_end("Right")        # the row's last chip: Export
key("Return")
key("Down", 3)             # PDF
key("Return")
shot("29_activity_export")
if not pdfs() - pdfs_before:
    fail("no new PDF export in " + exports)

finish()
