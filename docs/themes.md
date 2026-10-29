# Themes and palettes

PlayGuard draws every screen in two choices of *Preferences*:

- **Theme**: light, dark (the default) or the console's.
- **Palette**: the colours. *Default* is PlayGuard's own (navy, or cool white
  in the light theme, with teal), then the palettes that come with it, then
  the theme files you add. The palette is also the last line of the theme
  list (**−**, or the theme button in the header).

Both apply at once, on every screen, without starting PlayGuard again. A
palette made for one theme only (the high-contrast ones, *Ocean*, *Sand*)
sets *Theme* to it; choosing the other theme afterwards changes the palette
too, to its other half when it has one (*High contrast light* and *High
contrast dark*), else back to *Default* (the theme list says which
beforehand). The other palettes follow *Theme*.

Back to the [README](../README.md).

## The palettes that come with PlayGuard

| Palette | Theme | What |
| --- | --- | --- |
| *High contrast dark* | dark | White on black, a yellow focus ring, thicker |
| *High contrast light* | light | Black on white, a deep blue focus ring, thicker |
| *Ocean* | dark | Deep blue, a sky-blue accent |
| *Amethyst* | light and dark | Violet |
| *Sand* | light | Warm paper, a terracotta accent |
| *Winter*, *Valentine's Day*, *Lunar New Year*, *Spring*, *Summer*, *Autumn*, *Halloween*, *Christmas*, *New Year's Eve* | light and dark | One for each season and feast: frost and silver, rose, red lacquer and gold, cherry blossoms, sea and sunset, maple, pumpkin on a violet night, fir and gold, midnight and champagne |
| *Seasonal* | light and dark | The one of the day, by the calendar: Halloween from 17 October, Christmas in December, and so on |

They come from Aurore, the library PlayGuard is built on, and their names are
in every language of the console.

*Seasonal* changes by itself: on the day a season or a feast begins, the
screens take its palette at midnight, or as soon as the console wakes up. The
seasons are the northern hemisphere's; Aurore's [theme
guide](https://github.com/JigSawFr/Aurore/blob/main/docs/themes.md#the-seasons)
has the exact days.

## Adding your own

1. Write a theme file (below) and give it a name ending in `.theme`. A theme
   with pictures is a folder instead: its file is named `theme.theme`, its
   pictures are next to it ([Pictures](#pictures)).
2. Copy the file, or the whole folder, to the SD card, in
   `config/playguard/themes/` (make the folder if it is not there). In the
   simulator on a computer, the folder is `playguard_data/themes/`, next to
   the data PlayGuard keeps there.
3. *Preferences › Palette* lists it under its `name`, after the palettes that
   come with PlayGuard. Choose it: every screen takes it at once.

A file with a mistake PlayGuard cannot read past (a missing included file or
picture, a file that is not a theme) is left out of the list; a line it cannot
read is skipped, the rest of the file used. Both are written to the session log
(`config/playguard/logs/sessions/`), with the file and the line. Should the
chosen file be removed from the card, PlayGuard's own colours come back.

## Writing a theme file

A theme file is text: `key = value` lines, `[sections]`, and `#` comments. Three
lines are enough:

```ini
name = Mint
inherit = dark
accent = #3ddc97
```

- **`name`**, and optionally `author`, `version` and `description`: what the
  list shows. **`icon`**: the motif drawn on the palette's row, in its accent
  over a swatch of its colours: `snowflake`, `heart`, `lantern`, `blossom`,
  `sun`, `wave`, `leaf`, `pumpkin`, `tree`, `firework`, `gem`, `dune`,
  `contrast`, `moon`, `bolt` or `sparkle` (the default), or any other of
  Cadran's icons.
- **`inherit`**: what the file starts from, `dark` (when it says nothing),
  `light`, one of the palettes above (`builtin:ocean`, `builtin:amethyst`,
  `builtin:sand`, `builtin:high-contrast-dark`, `builtin:high-contrast-light`)
  or another file of the folder. Everything it does not say comes from there.
- **The palette**: `background`, `surface` (the cards), `text`, `accent`,
  `warning` and `danger`. Each one sets every colour that follows from it:
  `text` the quieter inks of the notes and captions, `accent` the focus ring,
  the active tab, the switches and the ring of the play timer, and so on.
  The inks are darkened or lightened until they read on the background and
  the cards.
- **`[light]` and `[dark]`**: a palette or keys for one theme only. A file
  with both sections styles both themes, and PlayGuard's *Theme* chooses
  which one shows.
- **`counterpart`**, for a file that styles one theme only: the file (or
  preset, `builtin:…`) with the other one, which PlayGuard switches to when
  the other theme is chosen (`counterpart = mint-day.theme`).
- **`[colors]`**: any single colour by its key (the lists below), over what
  the palette gave.
- **Colours**: `#rgb`, `#rrggbb`, `#rrggbbaa`, `rgb(r, g, b)` or
  `rgba(r, g, b, a)` (a from 0 to 1).
- **`@include other.inc`**: the lines of another file of the folder, at that
  place (name it `.inc`, so that it is not listed as a palette of its own).

Every key PlayGuard draws with is a theme key: Aurore's (`aurore/…`), Cadran's,
the look PlayGuard shares with PlayShelf (`cadran/…`), and PlayGuard's own
(`playguard/…`). Aurore's [theme guide](https://github.com/JigSawFr/Aurore/blob/main/docs/themes.md)
lists the first two with what each one colours, and what each palette key
sets.

### PlayGuard's own keys

| Key | What |
| --- | --- |
| `playguard/loading/glow` | The loading screen's dial: the glow behind it |
| `playguard/loading/arc` | The dial: the day's arc |
| `playguard/loading/track` | The dial: the arc's track |
| `playguard/loading/line` | The dial: its rings and minutes, at full strength (they are drawn faint) |
| `playguard/ring/tip` | The point of light at the end of the play timer's ring |
| `playguard/badge/ink` | The "!" on the coral disc of a game that stays (*Console › Check the installed games*, removals) |

The loading screen's background and text are the theme's (`aurore/background`,
`cadran/text2`), as on every screen, and so is its background picture.

### Readable text

After reading a file, PlayGuard checks that what must stay readable still
is: the text and the inks on the background and on the cards (4.5:1), the
focus ring (3:1), and its own `playguard/badge/ink` on `cadran/coral` (3:1).
A pair under its minimum is written to the session log, for example
`cadran/text3 on cadran/surf reads badly: 3.9:1, under 4.5:1`; the theme is
used all the same. The palettes that come with PlayGuard pass every check.

### Two examples

Both files are in [`docs/themes/`](themes/), ready to copy (a theme folder
with pictures is [below](#a-theme-folder-dusk)).

[`mint.theme`](themes/mint.theme): PlayGuard's dark look with another
accent.

```ini
name = Mint
author = PlayGuard
version = 1
description = The dark look, a mint accent.

inherit = dark
accent = #3ddc97
```

[`paper.theme`](themes/paper.theme): both themes, and one of PlayGuard's
own keys.

```ini
name = Paper and ink
author = PlayGuard
version = 1
description = Warm paper by day, warm ink by night, a terracotta accent.

[light]
background = #f4efe6
surface = #fffaf2
text = #2b2622
accent = #a4502a
warning = #8a5a00
danger = #b3261e

[dark]
background = #17130f
surface = #221c17
text = #f0e7da
accent = #e39a6b
warning = #f2b84b
danger = #ff8a80

[colors]
playguard/ring/tip = #fff1dc
```

## Pictures

A theme folder can bring pictures: a background behind every screen, a logo,
some of the icons, PlayGuard's loading picture, a thumbnail for the palette
list, and decorations. PNG or JPEG; each key names a file of the folder (or
of one of its subfolders): a path that goes out of the folder (`/…`,
`sdmc:/…`, `../…`) is refused, the line skipped and written to the session
log. A key at the top of the file is for both themes; in `[light]` or
`[dark]`, for that one only. Without these keys nothing changes.

```
config/playguard/themes/
├── mint.theme          # a single file
└── dusk/               # a theme folder
    ├── theme.theme
    ├── night.jpg
    └── icons/settings.png
```

### The background

| Key | Values | What |
| --- | --- | --- |
| `background_image` | a file, or `none` | A picture behind every screen (the cards, the sidebar and the dialogs stay over it) |
| `background_fit` | `cover` (the default), `contain`, `tile`, `center` | How it fills the screen: whole screen cut at the edges, whole picture, repeated, or as it is in the middle |
| `background_dim` | 0 to 1, 0.35 by default | A veil of the background colour over the picture, so that text reads on it |
| `surface_opacity` | 0 to 1, 1 by default | How opaque the cards, the sidebar, the focus and the buttons are over the picture |

The text is checked on the picture under its veil (its average colour): when
it reads badly, the session log says so, and a darker or lighter `background`
(or a stronger `background_dim`) brings it back. A picture of 1920 × 1080 or
less is best; a larger one is made smaller when loaded.

### The logo, the icons and PlayGuard's pictures

| Key | What |
| --- | --- |
| `logo` | PlayGuard's logo: the header's mark, *About*, and the loading screen (unless the theme gives the loading picture below). Fitted in the logo's square, drawn as it is |
| `icon.<name>` | One of Cadran's icons (`settings`, `info`, `today`, `timer`…), on the sidebar, the rows and the sheets |
| `icon.<name>.tint` | `true`: the picture is a shape drawn in the icon's colour (draw it white on transparent); `false`, the default: in its own colours |
| `image.<slot>` | One of PlayGuard's pictures (below) |
| `preview` | The thumbnail the palette list shows on the theme's row, instead of the swatch of its colours (square, 72 × 72 or more) |

PlayGuard's pictures a theme may replace (its image slots):

| Slot | What | PlayGuard's own |
| --- | --- | --- |
| `playguard/loading/art` | The picture in the middle of the loading screen, over its breathing glow, 168 units square (keep it 256 × 256 to 512 × 512, a transparent edge for a round one) | The app's icon |
| `playguard/loading/wordmark` | The name under it, 360 × 88 units (a transparent PNG about 720 × 176) | The *PlayGuard* wordmark, in the theme's inks |

```ini
image.playguard/loading/art = loading.png
[light]
image.playguard/loading/wordmark = name-day.png
[dark]
image.playguard/loading/wordmark = name-night.png
```

### Decorations

| Key | What |
| --- | --- |
| `backdrop.header`, `backdrop.sidebar`, `backdrop.dialog` | A picture over the header, the sidebar, or the panel of the dialogs and sheets, under their text: keep it quiet |
| `decor.top`, `decor.bottom` | A strip along the top or the bottom edge of the screen, behind the screens (a garland, icicles): about 30 units high at the top, 20 at the bottom, so that the header and the footer read over it |
| `<key>_fit`, `<key>_opacity`, `decor.top_height`, `decor.bottom_height` | How each one fills its place (as `background_fit`), how opaque it is, a strip's height |
| `particles` | `snow`, `leaves`, `petals`, `embers`, `confetti`, `sparkles`, `hearts`, `bubbles`, or `none`: what drifts behind the screens |
| `particles_density` | `low` (the default), `medium`, `high` |
| `particles_color` | Up to five colours, comma-separated; from the palette without it |
| `particles_speed` | 0.25 to 4, 1 by default |
| `particles_layer` | `back` (the default: behind the screens, between the cards) or `front` (over them, fainter) |

The seasonal palettes that come with PlayGuard have gentle particles of their
own: snow in winter and at Christmas, petals in spring, leaves in autumn,
embers at Halloween, confetti for the New Year, hearts on Valentine's Day,
gold sparkles for the Lunar New Year (`particles = none` in a file built on
one takes them away). **Particles move only when the look may move**: with
*Preferences › Visual effects* at *Reduced motion*, or at *Automatic* with the
console in hand, there are none at all, only the palette. After a minute
without a button pressed they fade out and stop, until the next press.

### A theme folder: Dusk

[`dusk/`](themes/dusk/) has a starry night sky behind the dark theme and the
dawn behind the light one, a logo for each, its own loading picture, a tinted
settings icon, stars hanging from the top of the screen at night, sparkles
drifting behind the screens, and a preview for the palette list. To install
it, copy the whole `dusk` folder into `config/playguard/themes/` (on a
computer, `playguard_data/themes/`), so that the file is
`config/playguard/themes/dusk/theme.theme`, then choose *Dusk* in
*Preferences › Palette*. Its pictures are drawn by
[`tools/make_example_theme.py`](../tools/make_example_theme.py), and weigh
about 100 KB together.

```ini
# A theme folder: this file is theme.theme, its pictures are next to it.
# Copy the whole dusk/ folder into config/playguard/themes/ (docs/themes.md).
# The pictures are drawn by tools/make_example_theme.py.
name = Dusk
author = PlayGuard
version = 1
description = A starry sky at night, the dawn by day, with a logo and a loading picture of its own.
icon = moon

# Behind every screen: the sky, the cards slightly see-through over it
background_fit = cover
surface_opacity = 0.88

# The palette list shows this picture instead of a swatch of the colours
preview = preview.png

# The loading screen's picture (an image slot of PlayGuard's)
image.playguard/loading/art = loading.png

# Cadran's settings icon (the rail, Preferences), drawn in the icon's colour
icon.settings = icons/settings.png
icon.settings.tint = true

# A few stars twinkling behind the screens (not with reduced motion)
particles = sparkles
particles_density = low

[light]
background = #f1edf4
surface = #fdfbff
text = #2a2238
accent = #6a3a7c
warning = #8a5a00
danger = #b3261e
background_image = day.jpg
background_dim = 0.6
logo = logo-light.png
# Over the picture, a little darker than the palette makes them
cadran/text3 = #574d66
aurore/header/subtitle = #574d66

[dark]
background = #1b1838
surface = #26224a
text = #f4eefb
accent = #ffc496
warning = #f2b84b
danger = #ff8a80
background_image = night.jpg
background_dim = 0.3
logo = logo-dark.png
# Stars hanging from the top of the screen, repeated sideways
decor.top = stars.png
decor.top_fit = tile
```

## What keeps its colours

- **The PIN gate** of a forwarder protected with the PIN (`config/playguard/gate.nro`)
  is a small program of its own, without PlayGuard's library: it keeps
  PlayGuard's own colours, in the light or dark theme.
- **PlayGuard's logo** (the header's mark, the loading screen's icon) and the
  wordmark, unless the theme gives pictures for them (`logo`, `image.…`,
  above), and the French flag in *About*.
- **The QR codes** stay dark on white, as phones read them best.
- **The games' covers and icons**.

The choice is kept in `config.json` (`theme`, `theme_file`: see
[config.md](config.md)).
