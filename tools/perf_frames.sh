#!/bin/bash
# Per-frame cost of one screen of the desktop build, headlessly.
#
#   tools/perf_frames.sh <name> [VAR=value ...]
#
# Starts build-desktop/playguard on an Xvfb display (software GL, the same
# conditions as the smoke test: fixed console time, still focus), moves the
# focus into the content with one Right press, lets the screen settle, takes
# a screenshot and quits. The app records one line per frame while it runs
# (PLAYGUARD_SIM_STATS, main.cpp): the frame time with the FPS limiter off
# and, with tools/perf_nanovg_stats.patch applied to extern/borealis,
# nanovg's own counters (draw calls, triangles). The summary covers the last
# 4 s, the steady state. Everything lands in build-desktop/perf/<name>/.
#
# Extra VAR=value pairs go to the app: PLAYGUARD_SIM_CADRAN=1 for the
# "Cadran" prototype of the Overview, PLAYGUARD_SIM_CADRAN_NOGLOW=1 and
# PLAYGUARD_SIM_CADRAN_NORING=1 to leave out its two custom draws, any
# PLAYGUARD_SIM_* knob of the simulated console.
#
# Software GL rasterises on the CPU, so these figures weigh fills (pixels)
# far more than the console's GPU does; compare screens with each other,
# do not read them as console milliseconds. Needs xvfb-run, xdotool,
# ImageMagick and dbus (dbus-run-session: borealis asks the session bus for
# a screensaver inhibitor and aborts without one).
set -u
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
APP="$ROOT/build-desktop/playguard"
NAME="${1:?usage: tools/perf_frames.sh <name> [VAR=value ...]}"; shift
DIR="$ROOT/build-desktop/perf/$NAME"
[ -x "$APP" ] || { echo "no desktop build: make desktop first"; exit 1; }
rm -rf "$DIR"; mkdir -p "$DIR/run/playguard_data"
# French, dark: the README's screenshots; change here for another look.
echo '{"language":"fr","theme":"dark"}' > "$DIR/run/playguard_data/config.json"

# The steps on the display, as a script: an exported function does not
# survive xvfb-run and dbus-run-session.
cat > "$DIR/inner.sh" <<INNER
cd "$DIR/run" || exit 1
export LIBGL_ALWAYS_SOFTWARE=1 TZ=UTC
export PLAYGUARD_SIM_NOW="\${PLAYGUARD_SIM_NOW:-1791475200}" PLAYGUARD_SIM_STILL_FOCUS="\${PLAYGUARD_SIM_STILL_FOCUS:-1}"
export PLAYGUARD_SIM_STATS="$DIR/stats.txt"
"$APP" > app.log 2>&1 &
pid=\$!
sleep 4
xdotool keydown Right sleep 0.05 keyup Right
sleep 6
import -window root "$DIR/shot.png"
kill -TERM \$pid; sleep 1; kill -KILL \$pid 2>/dev/null; wait \$pid 2>/dev/null
INNER
BUS=""; command -v dbus-run-session > /dev/null && BUS="dbus-run-session --"
env "$@" xvfb-run -a -s "-screen 0 1280x720x24" $BUS bash "$DIR/inner.sh"

python3 - "$DIR/stats.txt" <<'PY'
import statistics, sys
rows = [list(map(int, l.split())) for l in open(sys.argv[1]) if len(l.split()) == 6]
if not rows:
    print("no frames recorded (see app.log)"); sys.exit(1)
end = rows[-1][0]
win = [r for r in rows if end - r[0] <= 4_000_000]
ft = sorted(r[1] for r in win)
p95 = ft[min(len(ft) - 1, int(0.95 * len(ft)))]
print(f"frames: {len(win)} in the last 4 s ({len(win) / 4:.0f} fps, limiter off)")
print(f"frame time: median {statistics.median(ft) / 1000:.2f} ms, p95 {p95 / 1000:.2f} ms, mean {statistics.mean(ft) / 1000:.2f} ms")
if any(r[2] for r in win):
    m = lambda i: statistics.mean(r[i] for r in win)
    print(f"nanovg per frame: {m(2):.0f} draw calls, {m(3):.0f} fill tris, {m(4):.0f} stroke tris, {m(5):.0f} text tris")
else:
    print("nanovg counters: not available (apply tools/perf_nanovg_stats.patch in extern/borealis)")
PY
