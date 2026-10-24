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
  xvfb-run -s "-screen 0 1280x720x24" python3 tools/readme_screenshots.py [screen... | gate | clips | clip:<name>]
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
    # Console behind on everything (UPDATES, below): the release notes of the
    # firmware (its next version), Atmosphère, hekate and sys-patch.
    "notes_firmware": ("en-US", "console", ["Right", "Up", "Return"], "updates"),
    "notes_atmosphere": ("en-US", "console", ["Right", "Up", "Right", "Return"], "updates"),
    "notes_hekate": ("en-US", "console", ["Right", "Up", "Right", "Right", "Return"], "updates"),
    "notes_syspatch": ("en-US", "console", ["Right", "Up", "Right", "Right", "Right", "Return"], "updates"),
    # Console's Installed games, checked at start-up (one of each problem).
    "installed_games": ("en-US", "console", ["Right", "Right", "Return"]),
    # Console's Forwarders (PLAYGUARD_SIM_FORWARDERS): the list, sphaira's
    # sheet at its Protection, and Check the homebrew menu.
    "forwarders": ("en-US", "console", ["Right", "Right", "Right", "Return"], "forwarders"),
    "forwarder_protection": ("en-US", "console", ["Right", "Right", "Right", "Return", "Down", "Return"] + ["Down"] * 8, "forwarders"),
    "homebrew_menu": ("en-US", "console", ["Right", "Right", "Right", "Return", "Up", "Return", "Down", "Down", "Down", "Return"], "forwarders"),
}
# The gate's screens (`make gate-preview`): a protected forwarder opened from
# the HOME menu, gate_<state>.png.
GATE = ["asking", "opening", "refused", "closed", "limit"]
IM = ["magick"] if shutil.which("magick") else []

# A console behind on everything (tools/desktop_smoke.py's sysnotes scenario):
# firmware 22.5.0, Atmosphère 1.12.0, hekate 6.5.3 and sys-patch 1.6.2.3 on the
# SD card, the daily check on, web/samples/sys_notes as GitHub and switchbrew.
UPDATES = {"FW": "22.5.0", "SYS_LATEST": "1.12.1:6.5.4:23.0.1:1.6.3.0",
           "SYS_NOTES": os.path.join(ROOT, "web", "samples", "sys_notes"), "LATEST": "offline"}


def behind(data, config):
    config["update_daily"] = True
    sd = os.path.join(data, "sd")
    os.makedirs(os.path.join(sd, "bootloader"))
    open(os.path.join(sd, "bootloader", "update.bin"), "wb").write(b"\0" * 0x40 + b"ICTC" + b"653\0")
    flags = os.path.join(sd, "atmosphere", "contents", "420000000000000B", "flags")
    os.makedirs(flags)
    open(os.path.join(flags, "..", "exefs.nsp"), "wb").write(b"PFS0")
    open(os.path.join(flags, "boot2.flag"), "wb").close()
    os.makedirs(os.path.join(sd, "config", "sys-patch"))
    open(os.path.join(sd, "config", "sys-patch", "log.ini"), "w").write(
        "[stats]\nversion=v1.6.2.3\nfw_version=22.5.0\nis_emummc=1\n")


def take(name, lang, tab, keys, sim=None, setup=None, clip=None):
    run = tempfile.mkdtemp(prefix="readme-shot-")
    data = os.path.join(run, "playguard_data")
    os.makedirs(data)
    config = {"schema": 1, "language": lang, "theme": "dark"}
    if tab:
        config["start_tab"] = tab
    if setup:
        setup(data, config)
    json.dump(config, open(os.path.join(data, "config.json"), "w"))
    env = dict(os.environ, LIBGL_ALWAYS_SOFTWARE="1", TZ="UTC", PLAYGUARD_SIM_NOW="1791475200",
               PLAYGUARD_SIM_VERSION=f"{VERSION}:", PLAYGUARD_SIM_FOCUS_LOG="1",
               FAKETIME_DONT_FAKE_MONOTONIC="1")
    if not clip:
        env["PLAYGUARD_SIM_STILL_FOCUS"] = "1"   # a clip keeps the look's movements
    # The screen's own console (PLAYGUARD_SIM_*, docs/contributing/simulator.md).
    env.update({f"PLAYGUARD_SIM_{k}": v.replace("{root}", ROOT) for k, v in (sim or {}).items()})
    log_path = os.path.join(run, "app.log")
    log = open(log_path, "w")
    cmd = ["dbus-run-session", "--", "faketime", "-f", "@2026-10-08 16:00:00", "stdbuf", "-oL", "-eL", APP]
    proc = subprocess.Popen(cmd, cwd=run, env=env, stdout=log, stderr=subprocess.STDOUT, start_new_session=True)
    try:
        def logged(text):
            log.flush()
            return any(text in l for l in open(log_path, errors="replace"))

        entrance = clip and clip.get("entrance")
        for wanted in ("start-up: first screen ready",) + (() if entrance else ("games: start-up check done",)):
            for _ in range(240):
                if logged(wanted):
                    break
                if proc.poll() is not None:
                    sys.exit(f"{name}: PlayGuard exited ({log_path})")
                time.sleep(0.25)
            else:
                sys.exit(f"{name}: no {wanted!r} in 60 s ({log_path})")
        def press(k, pause=1.2):
            subprocess.run(["xdotool", "keydown", k, "sleep", "0.02", "keyup", k])
            time.sleep(pause)

        if not entrance:
            time.sleep(2)
        for k in keys:
            press(k)
        if clip:
            # The screen as it moves (x11grab), its keys pressed meanwhile.
            # Recorded fast (the screen is drawn on the CPU too), then made a WebM: the
            # Chromium tools/render_tour.mjs draws with plays VP9, not H.264.
            raw = os.path.join(run, "clip.mp4")
            path = os.path.join(OUT, "clips", name + ".webm")
            os.makedirs(os.path.dirname(path), exist_ok=True)
            rec = subprocess.Popen(["ffmpeg", "-y", "-v", "error", "-f", "x11grab", "-draw_mouse", "0", "-framerate", "30",
                                    "-video_size", "1280x720", "-i", os.environ["DISPLAY"], "-an", "-c:v", "libx264",
                                    "-preset", "ultrafast", "-crf", "12", "-pix_fmt", "yuv420p", raw],
                                   stdin=subprocess.PIPE)
            time.sleep(clip.get("lead", 0.6))
            for k in clip.get("keys", []):
                press(k, clip.get("pause", 1.0))
            time.sleep(clip.get("tail", 1.5))
            rec.communicate(b"q")
            subprocess.run(["ffmpeg", "-y", "-v", "error", "-i", raw, "-an", "-c:v", "libvpx-vp9", "-b:v", "0", "-crf", "30",
                            "-row-mt", "1", "-deadline", "good", path], check=True)
            print(name, path)
            return
        time.sleep(1.5)   # every animation over, no label scrolling yet
        path = os.path.join(OUT, name + ".png")
        subprocess.run(IM + ["import", "-window", "root", "-crop", "1280x720+0+0", "+repage", path], check=True)
        # Without the metadata ImageMagick dates: the same screen, the same bytes.
        subprocess.run(IM + ["convert", path, "-strip", "-define", "png:exclude-chunks=date,time", path], check=True)
        # Where the focus ended, to check the keys (PLAYGUARD_SIM_FOCUS_LOG).
        focus = [l.split("focus:", 1)[1].strip() for l in open(log_path, errors="replace") if "focus:" in l]
        print(name, path, f"(focus: {focus[-1]})" if focus else "")
    finally:
        os.killpg(proc.pid, 15)
        proc.wait()
        shutil.rmtree(run, ignore_errors=True)


def gate():
    subprocess.run(["make", "-C", ROOT, "gate-preview"], check=True, stdout=subprocess.DEVNULL)
    for state in GATE:
        path = os.path.join(OUT, f"gate_{state}.png")
        shutil.copyfile(os.path.join(ROOT, "build", "gate-preview", f"{state}.png"), path)
        print(f"gate_{state}", path)


CONSOLES = {"updates": (UPDATES, behind), "forwarders": ({"FORWARDERS": "1"}, None)}

# The tour's clips (images/screenshots/clips/<name>.webm, branding/tour.html):
# the screen as it moves, recorded while its keys are pressed. name: (language,
# tab, keys before, the clip: its keys and timing, its console).
CLIPS = {
    # Today opening: the ring drawing itself, then each extra time moving the line.
    "today": ("en-US", "", [], {"entrance": True, "lead": 1.4, "keys": ["Right", "Right", "Right"], "pause": 1.1, "tail": 0.8}),
    # The child view in its last minutes: the end in amber, the ring beating.
    "child": ("en-US", "", ["Right", "Down", "Right", "Return"], {"tail": 4.0}, {"LEFT_MIN": "4"}),
    # A day raised with the stick, 15 minutes a press.
    "day": ("en-US", "play_timer", ["Right", "Down", "Return"], {"keys": ["Up", "Up", "Up"], "pause": 0.8, "tail": 1.0}),
}

# A screen not in SCREENS: <name>=<language>:<tab>:<key>,<key>…[:<SIM>=<value>;…]
# (to try keys out; SIM a PLAYGUARD_SIM_ variable without its prefix).
for name in sys.argv[1:] or [*SCREENS, "gate"]:
    if name == "gate":
        gate()
    elif name == "clips" or name.startswith("clip:"):
        for clip in CLIPS if name == "clips" else [name[5:]]:
            lang, tab, keys, spec, *sim = CLIPS[clip]
            take(clip, lang, tab, keys, sim[0] if sim else None, clip=spec)
    elif "=" in name:
        name, spec = name.split("=", 1)
        lang, tab, keys, *sim = spec.split(":", 3)
        sim = dict(kv.split("=", 1) for kv in sim[0].split(";") if kv) if sim else {}
        # UPDATES=1: the console behind on everything (above).
        setup = behind if sim.pop("UPDATES", None) else None
        take(name, lang, tab, [k for k in keys.split(",") if k], {**(UPDATES if setup else {}), **sim}, setup)
    else:
        lang, tab, keys, *console = SCREENS[name]
        take(name, lang, tab, keys, *(CONSOLES[console[0]] if console else ()))
