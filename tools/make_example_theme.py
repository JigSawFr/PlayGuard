#!/usr/bin/env python3
# Draws the pictures of the example theme folder docs/themes/dusk: a sky
# behind every screen for each theme (night and dawn), a logo for each, the
# loading screen's picture (the image slot playguard/loading/art), a tinted
# icon, a strip of stars along the top of the screen and the preview the
# palette list shows. Made here so that the example ships no third-party
# picture.
#
#     python3 tools/make_example_theme.py docs/themes/dusk
#
# Needs Pillow. The output is the same on every run (a fixed seed).
# Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).

import math
import random
import sys
from pathlib import Path

from PIL import Image, ImageDraw, ImageFilter

SS = 4  # drawn this many times larger, then made smaller: smooth edges

NIGHT = {"top": (14, 18, 46), "bottom": (58, 36, 82), "hills": [(26, 22, 52), (18, 16, 38)], "glow": (255, 150, 110)}
DAWN = {"top": (196, 214, 240), "bottom": (252, 222, 206), "hills": [(214, 196, 214), (190, 176, 204)], "glow": (255, 190, 150)}


def lerp(a, b, t):
    return tuple(int(round(x + (y - x) * t)) for x, y in zip(a, b))


def gradient(width, height, top, bottom):
    image = Image.new("RGB", (width, height))
    draw = ImageDraw.Draw(image)
    for y in range(height):
        draw.line([(0, y), (width, y)], fill=lerp(top, bottom, y / (height - 1)))
    return image


def hills(draw, width, height, base, amplitude, phase, colour, scale=1):
    points = [(0, height)]
    for x in range(0, width + 1, 4 * scale):
        t = x / width
        y = base - amplitude * (0.6 * math.sin(t * 5.1 + phase) + 0.4 * math.sin(t * 11.3 + phase * 2))
        points.append((x, y))
    points.append((width, height))
    draw.polygon(points, fill=colour)


def sky(palette, stars, width=1280, height=720):
    """A sky over two lines of hills: the background picture."""
    image = gradient(width, height, palette["top"], palette["bottom"])
    # The glow of the sun just under the horizon, on the right.
    glow = Image.new("RGBA", (width, height), (0, 0, 0, 0))
    ImageDraw.Draw(glow).ellipse([820, 420, 1240, 840], fill=palette["glow"] + (90,))
    glow = glow.filter(ImageFilter.GaussianBlur(90))
    image.paste(glow, (0, 0), glow)
    draw = ImageDraw.Draw(image)
    if stars:
        rng = random.Random(7)
        for _ in range(160):
            x, y = rng.uniform(0, width), rng.uniform(0, height * 0.6)
            r = rng.choice([0.6, 0.8, 1.0, 1.4])
            a = rng.randint(90, 220)
            colour = lerp(palette["top"], (255, 255, 255), a / 255)
            draw.ellipse([x - r, y - r, x + r, y + r], fill=colour)
    hills(draw, width, height, height * 0.78, 34, 0.4, palette["hills"][0])
    hills(draw, width, height, height * 0.88, 26, 2.1, palette["hills"][1])
    return image


def crescent(image, cx, cy, r, colour):
    """A crescent over `image` (RGBA): a disc with another one taken out of it."""
    layer = Image.new("RGBA", image.size, (0, 0, 0, 0))
    draw = ImageDraw.Draw(layer)
    draw.ellipse([cx - r, cy - r, cx + r, cy + r], fill=colour)
    draw.ellipse([cx - r * 0.55, cy - r * 1.05, cx + r * 1.35, cy + r * 0.75], fill=(0, 0, 0, 0))
    image.alpha_composite(layer)


def star(draw, cx, cy, r, colour):
    points = []
    for i in range(8):
        a = math.pi / 4 * i - math.pi / 2
        d = r if i % 2 == 0 else r * 0.32
        points.append((cx + math.cos(a) * d, cy + math.sin(a) * d))
    draw.polygon(points, fill=colour)


def logo(dark):
    """The mark: a crescent and a star on a rounded tile."""
    size = 256 * SS
    image = Image.new("RGBA", (size, size), (0, 0, 0, 0))
    draw = ImageDraw.Draw(image)
    tile = (30, 28, 66, 255) if dark else (255, 246, 238, 255)
    ink = (255, 196, 150, 255) if dark else (122, 70, 140, 255)
    draw.rounded_rectangle([8 * SS, 8 * SS, 248 * SS, 248 * SS], radius=56 * SS, fill=tile)
    crescent(image, 118 * SS, 134 * SS, 70 * SS, ink)
    star(draw, 178 * SS, 84 * SS, 30 * SS, ink)
    return image.resize((256, 256), Image.LANCZOS)


def loading_art():
    """The loading screen's picture: a round window on the night sky."""
    size = 256
    scene = sky(NIGHT, True, 640, 360).crop((180, 40, 480, 340)).resize((size * SS, size * SS), Image.LANCZOS).convert("RGBA")
    crescent(scene, 120 * SS, 100 * SS, 38 * SS, (255, 214, 176, 255))
    mask = Image.new("L", (size * SS, size * SS), 0)
    ImageDraw.Draw(mask).ellipse([6 * SS, 6 * SS, (size - 6) * SS, (size - 6) * SS], fill=255)
    out = Image.new("RGBA", (size * SS, size * SS), (0, 0, 0, 0))
    out.paste(scene, (0, 0), mask)
    ring = ImageDraw.Draw(out)
    ring.ellipse([6 * SS, 6 * SS, (size - 6) * SS, (size - 6) * SS], outline=(255, 196, 150, 255), width=6 * SS)
    return out.resize((size, size), Image.LANCZOS)


def settings_icon():
    """A gear, white on transparent: a mask the theme tints (icon.settings.tint)."""
    size = 96 * SS
    image = Image.new("RGBA", (size, size), (0, 0, 0, 0))
    draw = ImageDraw.Draw(image)
    c = size / 2
    for i in range(8):
        a = math.pi / 4 * i
        dx, dy = math.cos(a), math.sin(a)
        px, py = -dy, dx
        w, r0, r1 = 9 * SS, 26 * SS, 44 * SS
        draw.polygon([(c + dx * r0 + px * w, c + dy * r0 + py * w), (c + dx * r1 + px * w * 0.8, c + dy * r1 + py * w * 0.8),
                      (c + dx * r1 - px * w * 0.8, c + dy * r1 - py * w * 0.8), (c + dx * r0 - px * w, c + dy * r0 - py * w)],
                     fill=(255, 255, 255, 255))
    draw.ellipse([c - 34 * SS, c - 34 * SS, c + 34 * SS, c + 34 * SS], fill=(255, 255, 255, 255))
    draw.ellipse([c - 14 * SS, c - 14 * SS, c + 14 * SS, c + 14 * SS], fill=(0, 0, 0, 0))
    return image.resize((96, 96), Image.LANCZOS)


def stars_strip():
    """A row of small stars hanging from the top edge, tiled sideways (decor.top)."""
    width, height = 160, 28
    image = Image.new("RGBA", (width * SS, height * SS), (0, 0, 0, 0))
    draw = ImageDraw.Draw(image)
    for x, length, r in [(20, 10, 5), (62, 16, 4), (100, 7, 6), (138, 13, 4)]:
        draw.line([(x * SS, 0), (x * SS, length * SS)], fill=(255, 220, 190, 110), width=SS)
        star(draw, x * SS, (length + r) * SS, r * SS, (255, 214, 176, 230))
    return image.resize((width, height), Image.LANCZOS)


def preview():
    """The palette list's thumbnail: the night sky with the crescent."""
    image = sky(NIGHT, True, 640, 360).crop((140, 0, 500, 360)).resize((128 * SS, 128 * SS), Image.LANCZOS).convert("RGBA")
    crescent(image, 52 * SS, 46 * SS, 18 * SS, (255, 214, 176, 255))
    return image.resize((128, 128), Image.LANCZOS).convert("RGB")


def main():
    if len(sys.argv) != 2:
        sys.exit("usage: make_example_theme.py <out-dir>")
    out = Path(sys.argv[1])
    (out / "icons").mkdir(parents=True, exist_ok=True)
    sky(NIGHT, True).save(out / "night.jpg", quality=80, optimize=True)
    sky(DAWN, False).save(out / "day.jpg", quality=80, optimize=True)
    logo(True).save(out / "logo-dark.png", optimize=True)
    logo(False).save(out / "logo-light.png", optimize=True)
    loading_art().save(out / "loading.png", optimize=True)
    settings_icon().save(out / "icons" / "settings.png", optimize=True)
    stars_strip().save(out / "stars.png", optimize=True)
    preview().save(out / "preview.png", optimize=True)


if __name__ == "__main__":
    main()
