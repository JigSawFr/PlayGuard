#!/usr/bin/env python3
"""Smoke test of the web simulator in a browser (web/README.md).

Serves the built simulator (build-web/simulator/) with the headers threads
need, opens it in headless Chromium (Playwright) and fails when PlayGuard does
not reach its first screen ("start-up: first screen ready" in its log, which
the page says in <body data-playguard="ready">), when the page throws, or
when what the page does while PlayGuard runs does not work:

- a page in each kind of start: the Overview, the Play timer, Activity,
  Console, today's limit reached, the start-up error (no Atmosphère);
- the time moved on from the buttons under the screen (skip=), without a
  restart: the URL follows, the page is not reloaded;
- a `live` setting changed from the panel without a restart;
- the CJK font: waited for in Japanese, added after the start in English;
- keep=1: the simulated console's settings file is still there after a
  reload.

The screens are written to <out-dir>/<name>.png (1280x720, the canvas), for
the pull request's preview comment (.github/workflows/pages.yml). They show
one frozen moment (PLAYGUARD_SIM_NOW) with the focus glow still.

Usage: tools/web_smoke.py <simulator-dir> <out-dir>   (needs Playwright for
Python with Chromium; PLAYWRIGHT_CHROMIUM=<path> for a browser of its own)
"""
import base64
import functools
import http.server
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


class Handler(http.server.SimpleHTTPRequestHandler):
    def end_headers(self):
        self.send_header("Cross-Origin-Opener-Policy", "same-origin")
        self.send_header("Cross-Origin-Embedder-Policy", "require-corp")
        self.send_header("Cache-Control", "no-cache")
        super().end_headers()

    def log_message(self, *args):
        pass


class Server(http.server.ThreadingHTTPServer):
    def handle_error(self, request, client_address):
        if not isinstance(sys.exc_info()[1], (ConnectionResetError, BrokenPipeError)):
            super().handle_error(request, client_address)   # a page left mid-download is no error


server = Server(("127.0.0.1", 0), functools.partial(Handler, directory=SITE))
threading.Thread(target=server.serve_forever, daemon=True).start()
BASE = f"http://127.0.0.1:{server.server_address[1]}/"

failures = []


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


def open_page(page, query, what):
    """Opens the simulator with `query`; True once its first screen is up."""
    page.goto(f"{BASE}?{query}")
    try:
        page.wait_for_function("['ready', 'error'].includes(document.body.dataset.playguard)", timeout=START_S * 1000)
    except Exception:
        fail(f"{what}: no first screen in {START_S} s (page: {page.evaluate('document.body.dataset.playguard')})")
        print(log_text(page)[-3000:])
        return False
    if page.evaluate("document.body.dataset.playguard") == "error":
        fail(f"{what}: {page.inner_text('#overlay')}")
        print(log_text(page)[-3000:])
        return False
    return True


def shot(page, name):
    # A moment for the first screen to settle (borealis' animations last
    # 300 ms), then the canvas as PlayGuard drew it.
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

    # The CJK font waited for in Japanese.
    if open_page(page, "lang=ja&now=1791471600&still_focus=1", "japanese"):
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

    for e in errors:
        fail(f"page error: {e}")
    browser.close()

server.shutdown()
if failures:
    print(f"{len(failures)} failure(s)")
    sys.exit(1)
print("web smoke test: ok")
