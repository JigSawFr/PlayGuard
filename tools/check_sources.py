#!/usr/bin/env python3
"""Static checks on the layout of source/ (run in CI and by `make check`).

Errors:
- an include going up the layers: source/ui, source/view and source/util
  never include activity/ or tab/ (they ask the screens through ui::app_hooks,
  ui/app_hooks.hpp, or Aurore's ui::hooks); source/action includes only the screens a flow opens
  itself, listed below with why;
- a C file of source/core that needs no console service (every one but the
  CONSOLE_ONLY ones below) missing from the desktop build's list
  (CMakeLists.txt, which leaves source/core/*.c out and adds those back by
  hand: the simulator shares them) unless NOT_ON_DESKTOP says why, or from
  the host tests' sources (Makefile); a name below that is no file any more.

Usage: tools/check_sources.py [repo-root]
"""
import glob
import os
import re
import sys

ROOT = sys.argv[1] if len(sys.argv) > 1 else "."
errors = []

# --------------------------------------------------------------- includes
# Below the screens: never activity/ nor tab/.
LOWER = ["source/ui", "source/view", "source/util"]
UPWARD = re.compile(r'^\s*#\s*include\s+"((?:activity|tab)/[^"]+)"', re.M)

# A flow that opens its own screen includes it (the flow is the screen's way
# in); nothing else of source/action includes a screen.
ACTION_SCREENS = {
    "source/action/clock_check.cpp": {"tab/clock_tab.hpp"},                     # opens the network clock's screen
    "source/action/dev_build_flow.cpp": {"activity/dev_build_activity.hpp",     # the development builds' screens
                                         "activity/dev_builds_activity.hpp"},
    "source/action/fw_gate.cpp": {"activity/firmware_gate_activity.hpp"},       # the firmware screen
    "source/action/game_notice.cpp": {"activity/game_check_activity.hpp"},      # the installed games check
    "source/action/notices.cpp": {"activity/onboarding_activity.hpp",           # what the bell's entries open:
                                  "tab/clock_tab.hpp"},                         # First steps, the network clock
    "source/action/profile_flow.cpp": {"activity/profiles_activity.hpp"},       # the profiles' screen
    "source/action/support_flow.cpp": {"activity/whats_new_activity.hpp"},      # What's new
    "source/action/sys_notes_flow.cpp": {"activity/sys_notes_activity.hpp"},    # the release notes' screen
    "source/action/update_flow.cpp": {"activity/update_activity.hpp"},          # the update screen
}


def sources(folder):
    for path in sorted(glob.glob(f"{ROOT}/{folder}/**/*", recursive=True)):
        if path.endswith((".c", ".cpp", ".h", ".hpp")):
            yield os.path.relpath(path, ROOT).replace(os.sep, "/"), open(path, encoding="utf-8").read()


for folder in LOWER:
    for rel, text in sources(folder):
        for inc in UPWARD.findall(text):
            errors.append(f'{rel} includes "{inc}": {folder}/ is below the screens (ask them through ui/app_hooks.hpp)')
seen = set()
for rel, text in sources("source/action"):
    allowed = ACTION_SCREENS.get(rel, set())
    for inc in UPWARD.findall(text):
        seen.add((rel, inc))
        if inc not in allowed:
            errors.append(f'{rel} includes "{inc}": a flow includes only a screen it opens '
                          "(else through ui/app_hooks.hpp; or list it in tools/check_sources.py, ACTION_SCREENS)")
for rel, allowed in sorted(ACTION_SCREENS.items()):
    for inc in sorted(allowed - {i for r, i in seen if r == rel}):
        errors.append(f'tools/check_sources.py: ACTION_SCREENS lists "{inc}" for {rel}, which does not include it')

# ------------------------------------------------------------ source/core
# The console's own: the simulator stands in for them on the desktop
# (source/sim/), or the desktop has no use for them.
CONSOLE_ONLY = {
    "album.c": "sim_album.cpp",
    "console_info.c": "sim_console.c",
    "crash_handler.c": "the console's exception handler",
    "forwarder.c": "sim_forwarder.cpp",
    "gamecheck.c": "sim_gamecheck.c",
    "gate_nx.c": "the PIN gate's .nro",
    "nacp_read.c": "sim_games.c",
    "pctl_dump.c": "sim_pctl.c",
    "pctl_ops.c": "sim_pctl.c",
    "platform.c": "sim_platform.cpp",
    "playstats.c": "sim_playstats.c",
    "startup_hook.c": "libnx's userAppInit",
    "sysinfo.c": "sim_pctl.c",
    "time_ops.c": "sim_time.c",
}
# Console-free, host-tested, but not in the desktop build.
NOT_ON_DESKTOP = {
    "gate_screen.c": "the PIN gate's screen (stb, built for the console only)",
    "gate_text.c": "the PIN gate's texts",
    "nacp.c": "a game's control data: the made-up games have their names",
    "playstats_logic.c": "the raw play log: sim_playstats.c makes the figures themselves",
}


def read(path):
    try:
        return open(f"{ROOT}/{path}", encoding="utf-8").read()
    except OSError:
        errors.append(f"{path} not found")
        return ""


cmake = set(re.findall(r"\$\{CMAKE_CURRENT_SOURCE_DIR\}/source/core/(\w+\.c)\b", read("CMakeLists.txt")))
make = set(re.findall(r"\bsource/core/(\w+\.c)\b", read("Makefile")))
core = {os.path.basename(p) for p in glob.glob(f"{ROOT}/source/core/*.c")}
if not cmake or not make or not core:
    errors.append("no source/core/*.c found in CMakeLists.txt, the Makefile or source/core")
for name in sorted(set(CONSOLE_ONLY) | set(NOT_ON_DESKTOP)):
    if name not in core:
        errors.append(f"tools/check_sources.py names source/core/{name}, which is not there any more")
for name in sorted(core - set(CONSOLE_ONLY)):
    if name not in NOT_ON_DESKTOP and name not in cmake:
        errors.append(f"source/core/{name} needs no console service but the desktop build leaves it out: add it "
                      "to CMakeLists.txt's console-free list (or, if the console needs it, to CONSOLE_ONLY in "
                      "tools/check_sources.py)")
    if name not in make:
        errors.append(f"source/core/{name} needs no console service but no host test builds it (Makefile)")
for name in sorted(cmake & set(CONSOLE_ONLY)):
    errors.append(f"CMakeLists.txt adds source/core/{name} to the desktop build, which "
                  f"tools/check_sources.py says is the console's ({CONSOLE_ONLY[name]})")

for e in errors:
    print("ERROR:", e)
print(f"sources check: {len(errors)} error(s)")
sys.exit(1 if errors else 0)
