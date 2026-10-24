#!/usr/bin/env python3
"""The library of forwarder icons (Console › Forwarders › Icon), drawn again.

branding/forwarder-icons/<id>.svg is each icon's source, in PlayGuard's look
(the amber ground, the navy disc, the play-time ring), and badge.svg the
shield drawn over the icon of a forwarder protected with the PIN. This draws
them as the 256 x 256 PNGs the app reads (resources/img/forwarder/): the
icons opaque, the badge transparent but for itself. The PNGs are in the
repository; run this after changing an SVG, and add an icon's id to
source/util/forwarder_library.cpp's IDS (and its name to every catalog).

Usage: tools/forwarder_icons.py [repo-dir]   (needs Playwright for Python
with Chromium)
"""
import glob
import os
import sys

from playwright.sync_api import sync_playwright

ROOT = sys.argv[1] if len(sys.argv) > 1 else "."
SRC = f"{ROOT}/branding/forwarder-icons"
OUT = f"{ROOT}/resources/img/forwarder"
PX = 256


def main():
    os.makedirs(OUT, exist_ok=True)
    sources = sorted(glob.glob(f"{SRC}/*.svg"))
    names = {os.path.basename(s)[:-4] for s in sources}
    for old in glob.glob(f"{OUT}/*.png"):
        if os.path.basename(old)[:-4] not in names:
            os.remove(old)
    with sync_playwright() as p:
        browser = p.chromium.launch()
        page = browser.new_page(viewport={"width": PX, "height": PX})
        for src in sources:
            with open(src) as f:
                svg = f.read()
            page.set_content(f'<body style="margin:0;background:transparent">{svg}</body>')
            out = f"{OUT}/{os.path.basename(src)[:-4]}.png"
            page.locator("svg").screenshot(path=out, omit_background=True)
            print(out)
        browser.close()


if __name__ == "__main__":
    main()
