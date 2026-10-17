#!/usr/bin/env python3
"""Headless UI smoke test of the desktop build (simulated backend).

Starts build-desktop/playguard on an X display (Xvfb), opens every tab, the
extra-time picker, the per-day editor (the week chart's day picker and the
number pad), a bedtime alarm change, the settings backup, a game's screen in the Activity tab and its
PDF export, and fails if the app dies on the way (borealis throws on unknown
XML attributes, missing views, …) or the export is missing. Screenshots of
each screen are written to the output folder.

The "gate" scenario starts on a firmware newer than the checked one, with a
simulated release that supports it: the firmware screen, read-only mode,
seven presses on About › Version for the developer mode, its report, a play-timer
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
down (PLAYGUARD_SIM_NOT_COUNTING): the Overview must say what changed
outside PlayGuard, record it in the change history, then, after 90 s on
screen, that the console is not counting; the notice must go when
dismissed, and the Activity tab flag the new account.

The "alarm" scenario starts with "When the time is up" on a warning only and
today's limit passed (PLAYGUARD_SIM_ALARM_ONLY, PLAYGUARD_SIM_RESTRICTED):
the Overview and the Play timer tab must show the time over the limit.

The "sysnotes" scenario checks for updates at start-up against a simulated
GitHub and switchbrew (PLAYGUARD_SIM_SYS_LATEST, PLAYGUARD_SIM_SYS_NOTES),
hekate 6.5.3 on the SD card: Console must say how far behind each
one is and that the newest firmware needs a newer hekate, then show the
release notes of the firmware (the closest newer version, then the
installed one), of Atmosphère and of hekate, and keep them in the cache.

The "loading" scenario stops the start-up on the loading screen
(PLAYGUARD_SIM_HOLD_LOADING), at its last step, for its screenshot. Every
other scenario waits for the first screen after it, at least 2 s later (the
intro), but "lock", which turns the intro off in config.json.

Usage: tools/desktop_smoke.py <out-dir> [gate|errors|rescue|forged|lock|devbuild|update|library|rollback|outside|loading|alarm|sysnotes]   (needs DISPLAY, xdotool, ImageMagick)
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
# 8 October 2026, 16:00 UTC. Only the footer clock follows the host.
env.setdefault("PLAYGUARD_SIM_NOW", "1791475200")
env.setdefault("TZ", "UTC")
# And the focus highlight without its moving glow (ui/theme.cpp).
env.setdefault("PLAYGUARD_SIM_STILL_FOCUS", "1")
# And one made-up development build, not this build's version and commit
# (app.cpp): the header's version badge, and the update offers (1.1.0 below)
# stay the same once this build is 1.1.0 or later.
env.setdefault("PLAYGUARD_SIM_VERSION", "1.0.0:1a2b3c4")
# The installed games are checked at start-up for the notification bell:
# at once in every scenario but the default one, which watches a check go
# game by game (150 ms each) and waits for the start-up one first.
if SCENARIO:
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
    env.setdefault("PLAYGUARD_SIM_NUMPAD", "1:30")   # what the system number pad returns
    env.setdefault("PLAYGUARD_SIM_PASTE", "https://gist.github.com/smoke/ab12")   # what GitHub answers
    github_token = os.path.join(run_dir, "playguard_data", "github_token")
    if os.path.exists(github_token):
        os.remove(github_token)   # signed out: Send a report online offers to sign in first
if ALARM:
    # "When the time is up" on a warning only, today's limit passed: the
    # console does not restrict, and its time left reads below 0.
    env.setdefault("PLAYGUARD_SIM_ALARM_ONLY", "1")
    env.setdefault("PLAYGUARD_SIM_RESTRICTED", "1")
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
if LIBRARY:
    # A large library: the Activity list builds its first rows, then the
    # rest on "Show every game".
    env.setdefault("PLAYGUARD_SIM_GAMES", "120")
if OUTSIDE:
    # What PlayGuard saw last time (outside_change_logic): 100 min played
    # today (the console says 80), a clock 15 min off the steady one (the
    # simulated steady clock's source is 16 bytes of 0x53), 1 h 30 on
    # Saturday (the console has 3 h), and only Alice of the two accounts
    # (Léo is new).
    env.setdefault("PLAYGUARD_SIM_NOT_COUNTING", "1")
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
    # it. Then L + R, with a PIN and without (see the end of the run).
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


def key(name, n=1, hold=0.02):
    """Presses `name` n times, each exactly once. A held Down / Up repeats
    (ScrollView scrolls a long text, then moves the focus again); with frames
    as slow as under software GL, even an 80 ms press could. So each press waits for the screen to settle (the app then waits
    for input and sees the press at once) and is released `hold` seconds later
    (20 ms by default), both
    sent by one xdotool process so a busy runner cannot delay the release."""
    for _ in range(n):
        time.sleep(0.6)
        subprocess.run(["xdotool", "keydown", name, "sleep", str(hold), "keyup", name], env=env)
        time.sleep(0.2)


def button_x():
    """Ⓧ: the desktop build reads it from the middle mouse button (the
    keyboard has none), clicked over the title, where nothing takes the focus."""
    time.sleep(0.6)
    subprocess.run(["xdotool", "mousemove", "640", "40", "mousedown", "2", "sleep", "0.05", "mouseup", "2"], env=env)
    time.sleep(0.2)


def shot(name):
    time.sleep(0.8)
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

    def again(name, extra, until):
        """Starts PlayGuard once more with L + R held (and A answered); its
        log in <name>.log. Ended once `until` is logged (it goes on to its
        window), or after 20 s."""
        run_env = dict(env, PLAYGUARD_SIM_HELD_LR="1", **extra)
        run_log_file = os.path.join(OUT, name + ".log")
        with open(run_log_file, "w") as run_log:
            p = subprocess.Popen(cmd, cwd=run_dir, env=run_env, stdout=run_log, stderr=subprocess.STDOUT)
            for _ in range(40):
                if p.poll() is not None or until in open(run_log_file, errors="replace").read():
                    break
                time.sleep(0.5)
            if p.poll() is None:
                time.sleep(1)   # the lines logged right after it
                p.kill()
            p.wait()
        return open(run_log_file, errors="replace").read()

    # L + R with a PIN set (the simulated console has one): anyone can hold
    # them, so the start-up screen only says where to go back, after the
    # PIN; the two builds stay as they are.
    text = again("app-lr-pin", {}, "rollback: L + R held, not offered")
    if "but a PIN is set" not in text or "Go back to the previous build (the PIN is asked)" not in text:
        fail("L + R with a PIN set did not say where to go back from")
    if "rollback: back to" in text or "next load: sdmc:" in text:
        fail("L + R went back to the previous build without the PIN")
    if open(sd_nro).read() != "the build before" or open(sd_nro + ".previous").read() != "the build that crashes":
        fail("L + R with a PIN set swapped the two builds")
    # L + R without a PIN: offered, and A swaps them back.
    text = again("app-lr-no-pin", {"PLAYGUARD_SIM_NOT_SET_UP": "1"}, "next load: sdmc:")
    if "L + R were held at start-up." not in text or "rollback: back to" not in text:
        fail("L + R without a PIN did not offer the previous build")
    if open(sd_nro).read() != "the build that crashes" or open(sd_nro + ".previous").read() != "the build before":
        fail("L + R without a PIN did not swap the two builds back")
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
if not SCENARIO:
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

tabs = ["dashboard", "play_timer", "activity", "restrictions", "security", "console", "history", "preferences", "about"]


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
    shot("02_read_only")       # title "PlayGuard · read-only"
    key("Down")                # Play timer: the read-only note, no limit editors
    key("Right")
    shot("03_play_timer_read_only")
    key("Left")
    # About: seven presses on Version (its first cell) turn the developer mode on.
    key("Down", steps("play_timer", "about"))
    key("Right")
    key("Return", 7)
    shot("04_dev_enabled")
    # The developer tools are at the end of About › Help & diagnostics:
    # press like a hand.
    key("Left")
    key("Right")
    key("Down", 50, hold=0.15) # its last line: the funding QR codes
    key("Up", 2)               # past Source: Help & diagnostics
    key("Return")
    key("Down", 50, hold=0.15) # the Developer section, down to its last cell
    shot("05_dev_tools")
    # Turning a developer switch off asks first; B keeps it on (the end of
    # the scenario checks developer mode was saved on).
    key("Up", 8)               # past the cleanup cells and the recorder: Read-only mode
    key("Return")
    shot("06a_read_only_off")  # untested firmware: the "own risk" warning
    key("Escape")
    key("Up")                  # Developer mode
    key("Return")
    shot("06b_dev_mode_off")
    key("Escape")
    key("Down", 5)             # Read-only, the recorder, the two cleanup cells: Show the diagnostic report
    key("Return")
    shot("06_dev_report")
    key("Escape")
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
    key("Up", 2)               # Delete the log files
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
    key("Down", 5)             # the report, the block, the firmware screen: Forget the firmware choice
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
    # About: seven presses on Version turn the developer mode on.
    key("Down", steps("dashboard", "about"))
    key("Right")
    key("Return", 7)
    key("Left")
    key("Right")
    key("Down", 50, hold=0.15) # its last line: the funding QR codes
    key("Up", 2)               # past Source: Help & diagnostics
    key("Return")
    key("Down", 50, hold=0.15) # the Developer section, down to its last cell
    key("Return")              # Install another build: signed out, the releases and "Sign in"
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
    shot("01_limit_reached")   # Overview: today's limit reached
    key("Down")                # Play timer: the state line says the limit is reached
    key("Right")               # the week chart (today)
    key("Down")                # Same limit every day
    key("Return")
    shot("02_picker")          # the days differ: "Custom…" is selected
    key("Up")                  # No play (0 min)
    key("Return")
    shot("03_gate")            # the change needs the temporary unlock
    key("Right")               # Unlock and apply
    key("Return")
    shot("04_unlock_failed")   # a dialog, not a toast
    key("Return")              # OK
    key("Left")                # back to the sidebar
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
    key("Down")                # Set the network clock to the console clock
    shot("06_clock_offline")   # "No server answered", then the console clock offered
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


if LIBRARY:
    def listed():
        log.flush()
        return [l.split("activity list: ", 1)[1].strip()
                for l in open(os.path.join(OUT, "app.log"), errors="replace") if "activity list: " in l]
    key("Down", steps("dashboard", "activity"))
    key("Right")
    shot("01_activity")
    if not listed() or not listed()[-1].startswith("50 of "):
        fail("the Activity list did not start with its first 50 games: " + repr(listed()))
    key("Down", 70, hold=0.15)  # past the chart and the 50 games, to "Show every game"
    shot("02_show_every_game")
    key("Return")
    shot("03_every_game")
    total = listed()[-1].split(" of ")[1].split()[0]
    if listed()[-1] != f"{total} of {total} games":
        fail("\"Show every game\" did not list them all: " + repr(listed()))
    # Another tab and back (the sidebar makes the tab again): still every game.
    built = len(listed())
    key("Left")
    key("Up")
    key("Down")
    if len(listed()) == built or listed()[-1] != f"{total} of {total} games":
        fail("\"Show every game\" was not kept across tabs: " + repr(listed()))
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
    # The time left stays the same: after 90 s on screen, the Overview says
    # the console is not counting.
    for _ in range(150):
        if "timer health: not counting" in log_text():
            break
        if not alive():
            fail("app exited while waiting for the timer health check")
        time.sleep(1)
    else:
        fail("the Overview never said the console is not counting")
    time.sleep(6)              # the next refresh shows it
    shot("02_not_counting")
    key("y")                   # the notification bell's list (the keyboard's Y): both, the
    # simulated network clock (not accurate) and PIN (1234) to do; the broken
    # games under Console, on one line.
    if not logged("notices: 4 to do") or not logged("1 about the console to do"):
        fail("the notification bell does not hold four notices to do and one about the console")
    shot("03_notices")
    key("Escape")
    key("Down")                # Play timer: its state line says it too
    key("Right")
    shot("04_play_timer_not_counting")
    key("Down")                # off the week chart (←/→ pick a day there)
    key("Left")
    key("Up")                  # back to the Overview, on the notice's Dismiss line
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
if ALARM:
    shot("01_overview_over")   # Overview: the time over the limit, not a time left
    key("Down")                # Play timer: the state line and the banner say so
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
    key("Down", 4)             # past the installed games, Firmware, Atmosphère: hekate, the warning under it
    shot("01_tools_console")   # 2 versions behind, and 23.0.1 needs hekate 6.5.4 first
    key("Up", 2)               # Firmware
    key("Return")
    wait_logged("release notes: firmware fetched", "firmware notes")
    shot("02_firmware_next")   # the closest newer one, 23.0.0: security fixes, parental controls changed
    key("Left")                # the installed one's tab
    key("Return")
    shot("03_firmware_installed")
    key("Escape")
    key("Down")                # Atmosphère
    key("Return")
    # Its list came with the start-up check, today: shown, not fetched again.
    wait_logged("release notes: atmosphere opened", "Atmosphère notes")
    shot("04_atmosphere")
    key("Escape")
    key("Down")                # hekate
    key("Return")
    wait_logged("release notes: hekate opened", "hekate notes")
    shot("05_hekate")

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
    finish(check_cache)
if LOADING:
    shot("01_loading")         # the icon, the wordmark, the bar at its last step
    finish()
if LOCK:
    shot("01_unlocked")        # the PIN screen answered at once: the Overview
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
    shot("02_opened")          # the Overview: the app opened after the rescue
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

# The per-day editor, opened from the tab's week chart on the day picked
# (its picker already open), then the extra-time picker and the
# Weekdays and weekend row (the editor, Monday–Friday picker first).
key("Up", steps("play_timer", tabs[-1]))   # from the last tab back to Play timer
key("Right")               # the week chart, on today
key("Right")               # tomorrow's day
key("Return")              # the per-day editor on that day, its picker open
shot("19_day_picker")
key("Escape")
shot("21_per_day")         # the editor, tomorrow focused
key("Return")              # tomorrow's limit again
shot("22_dropdown")
key("Down", 15)            # to the end of the list (No limit) …
key("Up")                  # … then Enter minutes…: the number pad types "1:30"
key("Return")
shot("22_numpad")          # tomorrow: 1 h 30, drawn as unsaved; "+ Save (1)" in the footer
if not any("numpad: " in l and l.rstrip().endswith("-> 1:30") for l in open(os.path.join(OUT, "app.log"), errors="replace")):
    fail("the number pad was not asked for the minutes")
key("Escape")              # unsaved: asks before leaving
shot("23_discard")
key("Right")               # Discard
key("Return")
shot("23_back")            # the tab, on the chart again
key("Down")                # Same limit every day
key("Down")                # Extra time today…
key("Return")
shot("20_extra_time")
key("Escape")
key("Down", 2)             # past No more play today: Weekdays and weekend
key("Return")
shot("21_split")           # the editor, Monday–Friday picker open
key("Escape")              # no value: no weekend picker either
key("Escape")              # nothing staged: leaves without asking

# When the time is up: its two choices (on "Suspend the software"), and the
# confirmation of the other one, cancelled.
key("Down", 3)             # past Remove and Profiles…: When the time is up
key("Return")
shot("23_time_up_picker")
key("Down")                # Warning only: a long confirmation, with the unlock
key("Return")
shot("23_time_up_confirm") # the text wraps inside the dialog (ui::dialog measures it)
key("Escape")              # cancelled: nothing written

# Bedtime: the alarm picker (on 21:00), 21:30, confirmed with the unlock the
# running timer needs; the tab must then show it.
key("Down")                # Bedtime alarm
key("Return")
shot("23_bedtime_picker")
key("Down", 2)             # 21:30
key("Return")
shot("23_bedtime_confirm")
key("Right")               # Unlock and apply
key("Return")
shot("23_bedtime_done")
if not any(m.startswith("Bedtime alarm changed.") for m in messages()):
    fail("the bedtime alarm was not changed: " + repr(messages()[-3:]))

# Play timer › Advanced: its own screen, the switch next to the actions it
# shows; turned on, then off again (backups restore the alarm only with it on).
key("Down", 4)             # past Allowed again at, the extra-time amounts and switch: Advanced
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
key("Down", steps("play_timer", "history"))
key("Right")               # Changes made with PlayGuard
key("Down")                # Back up the settings
backups = os.path.join(nand_dir, "backups")
before = len(os.listdir(backups)) if os.path.isdir(backups) else 0
key("Return")              # Back up the settings
shot("24_backup_saved")
if (len(os.listdir(backups)) if os.path.isdir(backups) else 0) != before + 1:
    fail("Return on 'Back up the settings' saved no backup (wrong cell focused?)")
key("Down")                # Restore a backup…
key("Return")
shot("25_backup_list")
key("Return")
shot("26_backup_restore")
key("Escape")
shot("27_back")

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
key("Left")                # back to the sidebar
key("Down", steps("history", "about"))
key("Right")
key("Down", 50, hold=0.15) # its last line: the funding QR codes
key("Up", 2)               # past Source: Help & diagnostics
key("Return")
key("Down", 2)             # past First steps, Export: Send a report online
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
# Robot Factory (the focus, the first game): only its update needs a newer
# firmware; removing the update (the fix, the first button) makes it work again.
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
key("Up", 6)               # Several games at once, above the sections
key("Down")                # the first game
key("Return")
shot("27_games_launch")
key("Return")              # Accept the installed version again
time.sleep(1.5)            # the toast follows the games read again
if not any(m.endswith("works again.") and m.startswith("Moon Racer") for m in messages()):
    fail("the launch version reset did not fix the game: " + repr(messages()[-3:]))
# Forest Tales: the console found corrupt data; its check runs in a dialog
# with its progress and Cancel, here cancelled.
key("Up", 6)
key("Down", 2)             # past Check these 2 games, above them: the first game of To check
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
key("Up", 6)               # Several games at once
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
key("Up", 6)               # 2 games selected
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
key("Up", 6)               # Several games at once
key("Down", 3)             # past Reinstall's buttons, Hedgehog Rush
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

# Activity: one game's screen, then a PDF export to the (simulated) SD card.
exports = os.path.join(run_dir, "playguard_data", "exports")
def pdfs():
    return {f for f in (os.listdir(exports) if os.path.isdir(exports) else []) if f.endswith(".pdf")}
pdfs_before = pdfs()   # the run folder is kept between local runs
key("Left")                # back to the sidebar
key("Up", steps("activity", "console"))
key("Right")               # Account (the simulated console has two; the totals are not focusable)
key("Down", 5)             # a scroll, then past Period, the game to rediscover and Export: the first game
key("Return")
shot("28_activity_game")   # its own screen: icon, seven days, figures, accounts
key("Down", 8)             # scrolled to the accounts: Alice, Léo, and the time on no account
shot("28b_activity_game_accounts")
key("Escape")
key("Up")                  # Export to the SD card…
key("Return")
key("Down", 3)             # PDF
key("Return")
shot("29_activity_export")
if not pdfs() - pdfs_before:
    fail("no new PDF export in " + exports)

finish()
