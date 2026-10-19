#!/usr/bin/env python3
"""Smoke test of the web simulator in a browser (web/README.md).

Serves the built simulator (build-web/simulator/) with the headers threads
need, opens it in headless Chromium (Playwright) and fails when PlayGuard does
not reach its first screen ("start-up: first screen ready" in its log, which
the page says in <body data-playguard="ready">), when the page throws, or
when what the page does while PlayGuard runs does not work:

- a page in each kind of start: the Overview, the Play timer, Activity,
  Console, today's limit reached, the start-up error (no Atmosphère);
- desktop smoke scenarios (tools/desktop_smoke.py) played as steps
  (runSteps(), web/simulator.js): every tab in turn, "alarm" (the time over
  the limit), "errors" (an unlock that fails ends in its dialog);
- the time moved on from the buttons under the screen (skip=), without a
  restart: the URL follows, the page is not reloaded;
- a `live` setting changed from the panel without a restart;
- the steps: keys pressed, then the steps link replayed; the tour; a report
  link to the bug form;
- the CJK font: waited for in Japanese, added after the start in English;
- keep=1: the simulated console's settings file is still there after a
  reload;
- the service worker (web/sw.js), served without the headers as GitHub Pages
  does: it makes the page cross-origin isolated, keeps PlayGuard's files, and
  the simulator starts offline.

The screens are written to <out-dir>/<name>.png (1280x720, the canvas), for
the pull request's preview comment (.github/workflows/pages.yml), and the
time each start took to <out-dir>/timings.json. The screens show one frozen
moment (PLAYGUARD_SIM_NOW) with the focus glow still.

Usage: tools/web_smoke.py <simulator-dir> <out-dir>   (needs Playwright for
Python with Chromium; PLAYWRIGHT_CHROMIUM=<path> for a browser of its own)
"""
import base64
import functools
import http.server
import json
import os
import sys
import threading
import time

from playwright.sync_api import sync_playwright

SITE, OUT = sys.argv[1], sys.argv[2]
os.makedirs(OUT, exist_ok=True)

# The same moment for every screen (Thursday 8 October 2026, 15:00 in Paris),
# the focus still, the header's badge on one build, the page in English.
FIXED = "lang=en-US&theme=light&now=1791471600&still_focus=1&version=1.0.0:1a2b3c4"
# What a start may take on a CI runner (software WebGL): the download, the
# intro (2 s) and the start-up itself.
START_S = 120
# build.json for the service worker's server: a commit in the files' URLs.
BUILD = {"repo": "JigSawFr/PlayGuard", "sha": "0000000smoke", "ref": "smoke", "title": "", "pr": None}


class Handler(http.server.SimpleHTTPRequestHandler):
    """The site, with the COOP / COEP headers (`isolate`), else as GitHub
    Pages serves it, with a build.json."""
    isolate = True

    def do_GET(self):
        if not self.isolate and self.path.split("?")[0] == "/build.json":
            body = json.dumps(BUILD).encode()
            self.send_response(200)
            self.send_header("Content-Type", "application/json")
            self.send_header("Content-Length", str(len(body)))
            self.end_headers()
            self.wfile.write(body)
            return
        super().do_GET()

    def end_headers(self):
        if self.isolate:
            self.send_header("Cross-Origin-Opener-Policy", "same-origin")
            self.send_header("Cross-Origin-Embedder-Policy", "require-corp")
        self.send_header("Cache-Control", "no-cache")
        super().end_headers()

    def log_message(self, *args):
        pass


class PagesHandler(Handler):
    isolate = False


class Server(http.server.ThreadingHTTPServer):
    def handle_error(self, request, client_address):
        if not isinstance(sys.exc_info()[1], (ConnectionResetError, BrokenPipeError)):
            super().handle_error(request, client_address)   # a page left mid-download is no error


def serve(handler, host):
    server = Server((host, 0), functools.partial(handler, directory=SITE))
    threading.Thread(target=server.serve_forever, daemon=True).start()
    return server, f"http://{host}:{server.server_address[1]}/"


server, BASE = serve(Handler, "127.0.0.1")
# Another origin, so its service worker starts from nothing.
pages_server, PAGES = serve(PagesHandler, "localhost")

failures = []
timings = {}


def fail(msg):
    print("WEB SMOKE FAILED:", msg)
    failures.append(msg)


def log_text(page):
    """What PlayGuard and the page logged (simulator.js' log lines)."""
    return page.evaluate("logLines.map(([line]) => line).join('\\n')")


def wait_logged(page, text, seconds):
    end = time.time() + seconds
    while time.time() < end:
        if text in log_text(page):
            return True
        time.sleep(0.25)
    return False


def open_page(page, query, what, base=None):
    """Opens the simulator with `query`; True once its first screen is up."""
    t0 = time.time()
    page.goto(f"{base or BASE}?{query}")
    # The service worker reloads the page once (web/sw.js): waiting goes on
    # in the new one.
    while True:
        left = START_S - (time.time() - t0)
        try:
            page.wait_for_function("document.body && ['ready', 'error'].includes(document.body.dataset.playguard)",
                                   timeout=max(left, 1) * 1000)
            break
        except Exception as e:
            if left > 0 and ("navigat" in str(e) or "destroyed" in str(e)):
                continue
            fail(f"{what}: no first screen in {START_S} s ({str(e).splitlines()[0]})")
            try:
                print(log_text(page)[-3000:])
            except Exception:
                pass
            return False
    if page.evaluate("document.body.dataset.playguard") == "error":
        fail(f"{what}: {page.inner_text('#overlay')}")
        print(log_text(page)[-3000:])
        return False
    timings[what] = round(time.time() - t0, 2)
    return True


def steps(page, text):
    """Plays steps (web/simulator.js' runSteps()) and waits for their end."""
    page.evaluate("(text) => runSteps(text)", text)


def shot(page, name):
    # A moment for the screen to settle (borealis' animations last 300 ms),
    # then the canvas as PlayGuard drew it.
    time.sleep(1.5)
    data = page.evaluate("canvas.toDataURL('image/png')")
    with open(os.path.join(OUT, name + ".png"), "wb") as f:
        f.write(base64.b64decode(data.split(",", 1)[1]))


with sync_playwright() as p:
    browser = p.chromium.launch(
        executable_path=os.environ.get("PLAYWRIGHT_CHROMIUM") or None,
        # WebGL 2 without a GPU: SwiftShader.
        args=["--use-angle=swiftshader", "--enable-unsafe-swiftshader", "--ignore-gpu-blocklist"])
    # The page allows no eval (its Content-Security-Policy); the test's own
    # wait_for_function needs one.
    context = browser.new_context(viewport={"width": 1600, "height": 1000}, locale="en-US", bypass_csp=True)
    page = context.new_page()
    errors = []
    page.on("pageerror", lambda e: errors.append(str(e)))

    # The screens of the preview comment.
    screens = [
        ("overview", ""),
        ("play_timer", "&tab=play_timer"),
        ("activity", "&tab=activity"),
        ("console", "&tab=console"),
        ("limit_reached", "&restricted=1"),
        ("init_error", "&no_cfw=1"),
    ]
    for name, extra in screens:
        if open_page(page, FIXED + extra, name):
            shot(page, name)
            print(f"ok   {name}")

    # desktop_smoke's tour of the tabs: each one in turn from the sidebar.
    if open_page(page, FIXED, "tabs"):
        for name in ["play_timer", "activity", "restrictions", "security", "console", "history", "preferences", "about"]:
            steps(page, "down")
            shot(page, f"tabs_{name}")
        if page.evaluate("document.body.dataset.playguard") != "ready":
            fail("tabs: PlayGuard stopped on the way")
        else:
            print("ok   every tab")

    # desktop_smoke's "alarm": a warning only, the limit passed.
    if open_page(page, FIXED + "&alarm_only=1&restricted=1", "alarm"):
        shot(page, "alarm_overview_over")
        steps(page, "down")
        shot(page, "alarm_play_timer_over")
        print("ok   alarm")

    # desktop_smoke's "errors", its first half: a limit change while the
    # unlock fails ends in the "could not unlock" dialog, not a toast. In the
    # classic appearance, as desktop_smoke runs it: the steps follow its lines.
    if open_page(page, FIXED + "&appearance=classic&fail=unlock&restricted=1&autosync_off=1&ntp=offline&network_offset=1411", "errors"):
        steps(page, "down.right.down.a")   # Play timer › the week chart › Same limit every day
        shot(page, "errors_picker")
        steps(page, "up.a")                # No play (0 min): the change needs the unlock
        shot(page, "errors_gate")
        steps(page, "right.a")             # Unlock and apply
        if not wait_logged(page, "Could not unlock parental controls", 10):
            fail("errors: no 'could not unlock' dialog")
        else:
            shot(page, "errors_unlock_failed")
            steps(page, "a")
            print("ok   errors")

    # The time moved on, live: no reload, the URL and the log follow.
    if open_page(page, FIXED, "time"):
        page.evaluate("window.notReloaded = true")
        page.click("#time-buttons button:last-child")   # +1 h
        if not wait_logged(page, "live: skip=3600", 5):
            fail("time: +1 h did not reach PlayGuard")
        elif not page.evaluate("window.notReloaded === true"):
            fail("time: the page reloaded")
        elif "skip=3600" not in page.url:
            fail(f"time: the URL does not say skip=3600 ({page.url})")
        else:
            print("ok   time moved on without a restart")
        # A live setting from the panel (its group folded until then).
        page.evaluate("document.querySelector('[data-group=games]').open = true")
        page.check("[data-name=idle]")
        page.click("#apply")
        if not wait_logged(page, "live: idle=1", 5) or not page.evaluate("window.notReloaded === true"):
            fail("live: idle from the panel did not reach PlayGuard without a restart")
        else:
            print("ok   live setting from the panel")
        # The CJK font, after the start in English.
        if not wait_logged(page, "Using zh-Hans font", 30):
            fail("cjk: the font was not added after the start")
        else:
            print("ok   CJK font added after the start")

    # The steps: two presses noted, their link replays them.
    if open_page(page, FIXED, "steps"):
        page.focus("#canvas")
        for key in ("ArrowDown", "ArrowDown"):
            time.sleep(0.5)
            page.keyboard.press(key)
        link = page.evaluate("stepsLink()")
        rec = link.split("rec=", 1)[1].split("&")[0] if "rec=" in link else ""
        if rec.split(".").count("down") != 2:
            fail(f"steps: the link does not hold the presses ({link})")
        elif open_page(page, link.split("?", 1)[1], "steps (replayed)") and not wait_logged(page, "steps: done", 30):
            fail("steps: the link's steps were not replayed")
        elif "rec=" in page.url:
            fail(f"steps: rec= stayed in the URL ({page.url})")
        else:
            print("ok   steps link replayed")
        # A report: GitHub's bug form, the steps link in it.
        url = page.evaluate("() => { let u = ''; window.open = (x) => { u = x; }; report(); return u; }")
        if "template=1-bug.yml" not in url or "steps=" not in url or len(url) > 8000:
            fail(f"report: not the bug form with the steps ({len(url)} characters)")
        else:
            print("ok   report link")

    # The tour.
    if open_page(page, FIXED + "&tour=1", "tour"):
        if not wait_logged(page, "steps: done", 90) or "live: skip=3600" not in log_text(page):
            fail("tour: it did not play to its end")
        else:
            print("ok   tour")

    # The CJK font waited for in Japanese.
    if open_page(page, "lang=ja&theme=light&now=1791471600&still_focus=1&version=1.0.0:1a2b3c4", "japanese"):
        log = log_text(page)
        if "Using zh-Hans font" not in log or log.index("Using zh-Hans font") > log.index("start-up: first screen ready"):
            fail("japanese: the CJK font was not there at start-up")
        else:
            shot(page, "japanese")
            print("ok   japanese")

    # keep=1: the simulated console's settings survive a reload.
    if open_page(page, FIXED + "&keep=1", "keep"):
        time.sleep(4)   # the page writes IndexedDB every 3 s
        page.evaluate("saveData()")
        time.sleep(1)
        if open_page(page, FIXED + "&keep=1", "keep (again)"):
            kept = page.evaluate("Module.FS.readdir('/playguard_data').includes('sim_console.bin')")
            if not kept:
                fail("keep: sim_console.bin is not there after a reload")
            else:
                print("ok   keep")

    # The service worker, as on GitHub Pages: no headers from the server.
    sw_context = browser.new_context(viewport={"width": 1600, "height": 1000}, locale="en-US", bypass_csp=True)
    sw_page = sw_context.new_page()
    sw_page.on("pageerror", lambda e: errors.append(str(e)))
    if open_page(sw_page, FIXED, "service worker", PAGES):
        if not sw_page.evaluate("self.crossOriginIsolated && !!navigator.serviceWorker.controller"):
            fail("service worker: the page is not isolated by it")
        else:
            print("ok   service worker: isolated")
        cached = sw_page.evaluate("caches.open('playguard-simulator').then((c) => c.keys()).then((k) => k.map((r) => r.url))")
        if not any("playguard.wasm?v=" in u for u in cached) or not any("playguard.data?v=" in u for u in cached):
            fail(f"service worker: PlayGuard's files are not kept ({cached})")
        else:
            print("ok   service worker: PlayGuard's files kept")
        sw_context.set_offline(True)
        if open_page(sw_page, FIXED, "offline", PAGES):
            print("ok   offline")
        sw_context.set_offline(False)
    sw_context.close()

    for e in errors:
        fail(f"page error: {e}")
    browser.close()

server.shutdown()
pages_server.shutdown()
with open(os.path.join(OUT, "timings.json"), "w") as f:
    json.dump(timings, f, indent=1)
print("starts (s): " + ", ".join(f"{k} {v}" for k, v in timings.items()))
if failures:
    print(f"{len(failures)} failure(s)")
    sys.exit(1)
print("web smoke test: ok")
