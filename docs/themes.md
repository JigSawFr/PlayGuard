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

1. Write a theme file (below) and give it a name ending in `.theme`.
2. Copy it to the SD card, in `config/playguard/themes/` (make the folder if it
   is not there). In the simulator on a computer, the folder is
   `playguard_data/themes/`, next to the data PlayGuard keeps there.
3. *Preferences › Palette* lists it under its `name`, after the palettes that
   come with PlayGuard. Choose it: every screen takes it at once.

A file with a mistake PlayGuard cannot read past (a missing included file, a
file that is not a theme) is left out of the list; a line it cannot read is
skipped, the rest of the file used. Both are written to the session log
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
  list shows.
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

Every key PlayGuard draws with is a theme key: Aurore's (`brls/…`), Cadran's,
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

The loading screen's background and text are the theme's (`brls/background`,
`cadran/text2`), as on every screen.

### Readable text

After reading a file, PlayGuard checks that what must stay readable still
is: the text and the inks on the background and on the cards (4.5:1), the
focus ring (3:1), and its own `playguard/badge/ink` on `cadran/coral` (3:1).
A pair under its minimum is written to the session log, for example
`cadran/text3 on cadran/surf reads badly: 3.9:1, under 4.5:1`; the theme is
used all the same. The palettes that come with PlayGuard pass every check.

### Two examples

Both files are in [`docs/themes/`](themes/), ready to copy.

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

## What keeps its colours

- **The PIN gate** of a forwarder protected with the PIN (`config/playguard/gate.nro`)
  is a small program of its own, without PlayGuard's library: it keeps
  PlayGuard's own colours, in the light or dark theme.
- **PlayGuard's logo** (the header's mark, the loading screen's icon) and the
  wordmark, and the French flag in *About*.
- **The QR codes** stay dark on white, as phones read them best.
- **The games' covers and icons**.

The choice is kept in `config.json` (`theme`, `theme_file`: see
[config.md](config.md)).
