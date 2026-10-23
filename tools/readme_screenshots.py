#!/usr/bin/env python3
"""The README's screenshots (images/screenshots/*.png), from the desktop build.

The same console as the desktop smoke test's (tools/desktop_smoke.py):
Thursday 8 October 2026, 16:00 UTC, the footer's clock too (libfaketime),
the focus without its glow, the latest release's version badge
(.release-please-manifest.json), the dark theme, 1280 x 720. The README, the
website's English pages without SITE_SCREENSHOTS and the Homebrew App Store's
listing (packaging/hb-appstore/pkgbuild.json) show them.

One start of PlayGuard per screen, on the tab it opens on (config.json's
start_tab), then the keys to it from there. Needs `make desktop`, Xvfb,
xdotool, ImageMagick, dbus and faketime.
  xvfb-run -s "-screen 0 1280x720x24" python3 tools/readme_screenshots.py [screen...]
"""
import json
import os
import shutil
import subprocess
import sys
import tempfile
import time

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
APP = os.path.join(ROOT, "build-desktop", "playguard")
# README_SHOTS_OUT: another folder; README_SHOTS_VERSION: another badge (the
# release about to be made, 1.3.0, while its pull request is open).
OUT = os.environ.get("README_SHOTS_OUT") or os.path.join(ROOT, "images", "screenshots")
VERSION = os.environ.get("README_SHOTS_VERSION") or json.load(open(os.path.join(ROOT, ".release-please-manifest.json")))["."]

# name: (language, tab, keys from the tab's first screen)
SCREENS = {
    "dashboard": ("en-US", "", []),
    "play_timer": ("en-US", "play_timer", []),
    # The profiles' chip, Monday, Monday edited and raised by 15 minutes.
    "per_day": ("en-US", "play_timer", ["Right", "Down", "Return", "Up"]),
    "activity": ("en-US", "activity", []),
    # Activity's first game card, its own screen.
    "activity_game": ("en-US", "activity", ["Right", "Down", "Return"]),
    "restrictions": ("en-US", "restrictions", []),
    "security": ("en-US", "security", []),
    "console": ("en-US", "console", []),
    # Console's Network clock tile.
    "clock": ("en-US", "console", ["Right", "Return"]),
    "preferences": ("en-US", "preferences", []),
    "dashboard_fr_dark": ("fr", "", []),
    # For the tour (branding/tour.html): Today's Extra time "…", its Child
    # view, and the notifications (Y).
    "extra_time": ("en-US", "", ["Right", "Right", "Right", "Right", "Return"]),
    "child_view": ("en-US", "", ["Right", "Down", "Right", "Return"]),
    "notifications": ("en-US", "", ["y"]),
}
IM = ["magick"] if shutil.which("magick") else []


def take(name, lang, tab, keys):
    run = tempfile.mkdtemp(prefix="readme-shot-")
    data = os.path.join(run, "playguard_data")
    os.makedirs(data)
    config = {"schema": 1, "language": lang, "theme": "dark"}
    if tab:
        config["start_tab"] = tab
    json.dump(config, open(os.path.join(data, "config.json"), "w"))
    env = dict(os.environ, LIBGL_ALWAYS_SOFTWARE="1", TZ="UTC",
               PLAYGUARD_SIM_NOW="1791475200", PLAYGUARD_SIM_STILL_FOCUS="1",
               PLAYGUARD_SIM_VERSION=f"{VERSION}:", PLAYGUARD_SIM_FOCUS_LOG="1",
               FAKETIME_DONT_FAKE_MONOTONIC="1")
    log_path = os.path.join(run, "app.log")
    log = open(log_path, "w")
    cmd = ["dbus-run-session", "--", "faketime", "-f", "@2026-10-08 16:00:00", "stdbuf", "-oL", "-eL", APP]
    proc = subprocess.Popen(cmd, cwd=run, env=env, stdout=log, stderr=subprocess.STDOUT, start_new_session=True)
    try:
        def logged(text):
            log.flush()
            return any(text in l for l in open(log_path, errors="replace"))

        for wanted in ("start-up: first screen ready", "games: start-up check done"):
            for _ in range(240):
                if logged(wanted):
                    break
                if proc.poll() is not None:
                    sys.exit(f"{name}: PlayGuard exited ({log_path})")
                time.sleep(0.25)
            else:
                sys.exit(f"{name}: no {wanted!r} in 60 s ({log_path})")
        time.sleep(2)
        for k in keys:
            subprocess.run(["xdotool", "keydown", k, "sleep", "0.02", "keyup", k])
            time.sleep(1.2)
        time.sleep(1.5)   # every animation over, no label scrolling yet
        path = os.path.join(OUT, name + ".png")
        subprocess.run(IM + ["import", "-window", "root", "-crop", "1280x720+0+0", "+repage", path], check=True)
        # Without the metadata ImageMagick dates: the same screen, the same bytes.
        subprocess.run(IM + ["convert", path, "-strip", "-define", "png:exclude-chunks=date,time", path], check=True)
        print(name, path)
    finally:
        os.killpg(proc.pid, 15)
        proc.wait()
        shutil.rmtree(run, ignore_errors=True)


# A screen not in SCREENS: <name>=<language>:<tab>:<key>,<key>… (to try keys out).
for name in sys.argv[1:] or SCREENS:
    if "=" in name:
        name, spec = name.split("=", 1)
        lang, tab, keys = spec.split(":", 2)
        take(name, lang, tab, [k for k in keys.split(",") if k])
    else:
        take(name, *SCREENS[name])
