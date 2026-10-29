#!/usr/bin/env python3
"""The simulator's game pictures, drawn again: each made-up game's icon and
the album's screenshots (source/sim/), from one table
(tools/sim_art_games.json), into the headers the desktop and web builds
compile in:

- source/sim/sim_icons.h: SIM_ICON_<n>, 256 × 256 JPEG, the icon of the
  table's game n (sim_games.c's `icon`), framed on its scene's subject with
  its name in Sora (resources/font, OFL) as a logo;
- source/sim/sim_shots.h: SIM_SHOT_<n>_<moment>, 1280 × 720 JPEG, the game
  as the console's screenshot shows it, with the head-up display of its
  kind of game (sim_album.cpp).

Every picture is a scene of its kind (tools/sim_art_scenes.py: an open sky
of islands, a kart track, a harbour village…) in the game's colours, as SVG
rendered in Chromium, the same at every run. Nothing of a real game: no
name, character, logo or picture. The headers are in the repository; run
this after changing the table or a scene, and give a new game's place to
its `icon` in sim_games.c.

Usage: tools/sim_art.py [repo-dir]   (needs Playwright for Python, with its
Chromium or CHROMIUM=<path> for another, and Pillow)
"""
import base64
import io
import json
import os
import random
import sys

from PIL import Image

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import sim_art_scenes as scenes  # noqa: E402

ROOT = os.path.abspath(sys.argv[1] if len(sys.argv) > 1 else os.path.join(os.path.dirname(__file__), ".."))
FONT = os.path.join(ROOT, "resources", "font", "sora_bold_latin.ttf")
ICON_QUALITY = 82           # JPEG
SHOT_QUALITY = 72


# ------------------------------------------------------------- the pages

PAGE = """<!doctype html><html><head><meta charset="utf-8"><style>
@font-face{{font-family:Sora;src:url(data:font/ttf;base64,{font}) format("truetype");font-weight:700}}
html,body{{margin:0;padding:0;background:#000;overflow:hidden}}
.stage{{position:relative;width:{w}px;height:{h}px;overflow:hidden}}
.stage>svg{{position:absolute;inset:0;width:100%;height:100%}}
.shade{{position:absolute;left:0;right:0;bottom:0;height:62%;background:linear-gradient(180deg,rgba(0,0,0,0) 0%,rgba(8,6,20,.55) 55%,rgba(8,6,20,.82) 100%)}}
.logo{{position:absolute;left:6%;right:6%;bottom:6.5%;text-align:center;font-family:Sora,sans-serif;font-weight:700;color:#fff}}
.kicker{{display:inline-block;font-size:{kick}px;letter-spacing:.18em;text-transform:uppercase;padding:.28em .8em .22em;border-radius:999px;
  background:{accent};color:#fff;text-shadow:0 1px 2px rgba(0,0,0,.35);margin-bottom:.45em;box-shadow:0 4px 14px rgba(0,0,0,.35)}}
.name{{font-size:{size}px;line-height:1.02;letter-spacing:-.01em;
  background:linear-gradient(180deg,#ffffff 0%,#ffffff 48%,{tint} 100%);-webkit-background-clip:text;background-clip:text;color:transparent;
  -webkit-text-stroke:{stroke}px rgba(20,12,40,.85);paint-order:stroke fill;filter:drop-shadow(0 {drop}px {blur}px rgba(0,0,0,.55))}}
{hud_css}
</style></head><body><div class="stage">{svg}{over}</div>
<script>
// The name as large as its box allows: smaller until it fits.
const n = document.querySelector('.name');
if (n) {{
  const box = document.querySelector('.logo');
  let size = parseFloat(getComputedStyle(n).fontSize);
  const wide = () => {{ const r = document.createRange(); r.selectNodeContents(n);
    return Math.max(...[...r.getClientRects()].map(c => c.width)); }};
  while (size > 12 && (wide() > box.clientWidth || n.getBoundingClientRect().height > {max_h})) {{
    size -= 2; n.style.fontSize = size + 'px';
  }}
}}
</script></body></html>"""


def font_data():
    with open(FONT, "rb") as f:
        return base64.b64encode(f.read()).decode()


def split_name(name):
    """A series' game: its series as a kicker over its own name."""
    if ": " in name:
        series, own = name.split(": ", 1)
        return series, own
    return "", name


def page(game, v, kind, w, h, font):
    """The HTML of one of the game's pictures: `kind` icon or screen."""
    p = scenes.Palette(game["hues"])
    rnd = random.Random(f"{game['name']}/{v}")
    picture = scenes.svg(game["art"], p, rnd, v, "square" if kind == "icon" else "wide")
    over, hud_css = "", ""
    size = kick = stroke = drop = blur = max_h = 0
    if kind == "icon":
        series, own = split_name(game.get("logo", game["name"]))
        kicker = f'<div class="kicker">{series}</div><br>' if series else ""
        over = f'<div class="shade"></div><div class="logo">{kicker}<div class="name">{own}</div></div>'
        size, kick, stroke, drop, blur = int(w * 0.135), int(w * 0.042), max(2, w // 120), w // 100, w // 70
        max_h = int(h * (0.36 if series else 0.46))
    else:
        over = scenes.hud(scenes.HUD_OF.get(game["art"], "adventure"), game["name"], sum(map(ord, game["name"])), v)
        hud_css = scenes.HUD_CSS
    return PAGE.format(font=font, w=w, h=h, svg=picture, over=over, hud_css=hud_css, size=size, kick=kick, stroke=stroke,
                       drop=drop, blur=blur, max_h=max_h, accent=p.accent, tint=p.light)


class Renderer:
    """Chromium, through Playwright: a page at a time, as a PNG."""

    def __init__(self):
        from playwright.sync_api import sync_playwright
        self.pw = sync_playwright().start()
        # CHROMIUM: a Chromium of one's own, when Playwright's is not installed.
        self.browser = self.pw.chromium.launch(executable_path=os.environ.get("CHROMIUM") or None)
        self.pages = {}

    def shot(self, html, w, h):
        if (w, h) not in self.pages:
            self.pages[(w, h)] = self.browser.new_page(viewport={"width": w, "height": h}, device_scale_factor=1)
        tab = self.pages[(w, h)]
        tab.set_content(html, wait_until="load")
        tab.evaluate("document.fonts.ready")
        return Image.open(io.BytesIO(tab.screenshot(clip={"x": 0, "y": 0, "width": w, "height": h}))).convert("RGB")

    def close(self):
        self.browser.close()
        self.pw.stop()


# ------------------------------------------------------------- the headers

def jpeg(img, size, quality):
    if img.size != size:
        img = img.resize(size, Image.LANCZOS)
    out = io.BytesIO()
    img.save(out, "JPEG", quality=quality, optimize=True)
    return out.getvalue()


def array(name, data):
    rows = [", ".join(f"0x{b:02X}" for b in data[i:i + 16]) for i in range(0, len(data), 16)]
    return f"static const unsigned char {name}[] = {{\n    " + ",\n    ".join(rows) + "\n};\n"


def header(path, doc, arrays, tail):
    with open(path, "w", encoding="utf-8") as f:
        f.write(doc + "#pragma once\n#include <stddef.h>\n\n")
        for name, data in arrays:
            f.write(array(name, data))
        f.write(tail)


def main():
    with open(os.path.join(ROOT, "tools", "sim_art_games.json"), encoding="utf-8") as f:
        games = json.load(f)["games"]
    font = font_data()
    render = Renderer()
    icons, shots = [], []
    for n, g in enumerate(games):
        icon = render.shot(page(g, 0, "icon", 512, 512, font), 512, 512)
        icons.append((f"SIM_ICON_{n}", jpeg(icon, (256, 256), ICON_QUALITY)))
        for v in g.get("shots", []):
            shot = render.shot(page(g, v, "screen", 1280, 720, font), 1280, 720)
            shots.append((f"SIM_SHOT_{n}_{v}", jpeg(shot, (1280, 720), SHOT_QUALITY)))
    render.close()
    sim = os.path.join(ROOT, "source", "sim")
    names = [n for n, _ in icons]
    header(os.path.join(sim, "sim_icons.h"),
           "// The simulated games' icons for the desktop and web builds: 256 x 256\n"
           "// JPEGs, the size the HOME menu wants, one a game of\n"
           "// tools/sim_art_games.json (sim_games.c's `icon`), its key art with its\n"
           "// name. Made by tools/sim_art.py: run it again rather than edit this.\n"
           "// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).\n",
           icons,
           f"static const unsigned char *const SIM_ICONS[] = {{ {', '.join(names)} }};\n"
           f"static const size_t SIM_ICON_SIZES[] = {{ {', '.join(f'sizeof({n})' for n in names)} }};\n")
    header(os.path.join(sim, "sim_shots.h"),
           "// The album's screenshots of the simulated games (sim_album.cpp): 1280 x 720\n"
           "// JPEGs, SIM_SHOT_<game>_<moment>, the game of tools/sim_art_games.json\n"
           "// with its head-up display. Made by tools/sim_art.py: run it again rather\n"
           "// than edit this.\n"
           "// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).\n",
           shots, "")
    size = sum(len(d) for _, d in icons + shots)
    print(f"sim_art: {len(icons)} icons, {len(shots)} screenshots, {size // 1024} KB of JPEG")


if __name__ == "__main__":
    main()
