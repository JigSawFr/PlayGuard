# The website

PlayGuard's website, on GitHub Pages: <https://playguard.link/>.

- **A home page and the documentation in the console's 15 languages**:
  English at the root, the others in `/fr/`, `/fr-ca/`, `/de/`, `/es/`,
  `/es-419/`, `/it/`, `/nl/`, `/pt/`, `/pt-br/`, `/ru/`, `/ja/`, `/ko/`,
  `/zh-hans/`, `/zh-hant/`. A first visit to the root from elsewhere goes to
  the browser's language when the site has it; the header's list changes it
  (and is remembered). The theme is *Auto* (the browser's), *Light* or *Dark*,
  remembered as the web simulator remembers it (both read `pg-theme`).
- **The screenshots in each language**: the console's screens as the
  simulator shows them in that language (below), English where there are
  none (a local build); under the site's dark theme, the console's dark one.
- **On a phone** (an iPhone in Safari or Edge, the same WebKit): no field
  under 16px (iOS zooms into one), 44px to touch, the page beside the notch
  in landscape (`viewport-fit=cover`, `env(safe-area-inset-*)`), each page's
  sections folded under its first paragraph, and nothing wider than the
  screen (a long `code` breaks). The header goes on two rows up to 1080px,
  where its links no longer fit beside the menus in every language.
- **The home page**: install first (the download under it, with the
  release's date), the three main features then the others, PlayGuard
  beside Nintendo's app on what each needs, the questions each with its own
  address (`#faq-<question>`, opened by a link to it). The screenshots open
  larger over the page (`site.js`: swipe or the arrows, down or Esc to
  close), the documentation's too: its tables of screenshots under their
  names become captioned figures (`screenshot_table()`), one per row on a
  phone. Each section of the documentation has a link to share it, each
  page the previous and next ones; the changelog folds the older releases.
- **For a parent in a hurry**: *Locked out?* under the home page's
  buttons; a check on the compatibility page (a firmware typed, what
  PlayGuard does on it, from `source/core/sysinfo.h`'s limits as
  `tools/gen_compat.py` reads them); a copy button on each block of code; a
  page that prints as its text alone (its folded parts open); an icon for an
  iPhone's home screen (`images/apple-touch-icon.png`,
  `tools/render_branding.mjs`).
- **The home page's install**: two tabs (`site.js`, both under their names
  without it), the app store or by hand; the release's date links to its notes.
  The buttons to the simulator say what it downloads (`SITE_SIMULATOR`, the
  built simulator: its files compressed as GitHub Pages sends them; nothing for
  a local build without it).
- **The documentation's search**: a box above its pages (`site.js`), over
  `docs/search.json`, each section of the language's pages, the home page's
  questions and each release of the changelog; every word typed
  must be in the section, accents or not, and is marked in the results; `/` to search, the
  arrows through them.
- **The changelog** (`/changelog/`, `CHANGELOG.md` in each language's page,
  its notes in English) and `releases.atom`, a feed of the releases.
- **The page not found** (`404.html`, served at any address) in every
  language: `site.js` shows the one of the address' folder (`/fr/…`), else
  the visitor's.
- **For search engines and AI assistants**: absolute links (the canonical
  page, each language's, the preview image), `sitemap.xml`, structured data
  (schema.org JSON-LD: the application and the questions on the home page,
  each page of the documentation as an article), a preview image in each
  language (the site's title over the console's dashboard), each page of the
  documentation in Markdown next to it (`docs/<page>.md`, in English and in
  `/fr/`), listed in [`llms.txt`](https://llmstxt.org) (`llms-full.txt`: the
  whole documentation in one file), and `robots.txt`, `favicon.ico` and
  `apple-touch-icon.png` when the site is at its domain's root (a custom
  domain: they count nowhere else; browsers, bookmarks and link previews ask
  for the icons there without reading the page). Added to a phone's home
  screen, the site is named "PlayGuard" (`assets/site.webmanifest`,
  `apple-mobile-web-app-title`), not by its page's title; a long press on
  Android's icon offers the simulator. `docs/` itself, in each
  language, sends its visitor on to the guide's first page. A pull request's
  preview says `noindex`.
- **The documentation in two versions**: the latest release's, made from
  the Markdown and the screenshots of its tag (read with `git show`: no
  file kept for it in the repository, the next release's taken the same
  way), and main's. `docs/<page>/` (the site's, indexed) is the release's
  where it has the page in that language (what a console runs), main's
  otherwise; each is also at its own address, `docs/<tag>/<page>/` (in a
  language it was not written in, the English page, which a note says) and
  `docs/dev/<page>/` (`noindex`). A switch above each page's list goes from
  one to the other. Main's pages that are not the release's (new, or their
  English changed) say so, link to the release's page and to the
  changelog's **In development** (main's commits since the tag, under the
  sections the next release's notes will have, `release-please-config.json`,
  with a link to install a development build), and mark **New** the
  sections the release had not. The changelog shows what changed since a
  version typed (a console's): the releases after it, open, the others
  hidden. Without git's history (a tarball), the site has main's
  documentation alone; CI clones the history without old files' contents
  (`fetch-depth: 0`, `filter: blob:none`).
- **The web simulator** in `/simulator/` ([`web/README.md`](../web/README.md)).
- **The simulator, versioned as the documentation**: `/simulator/` is the
  latest release's (main's while no release has one: from 1.3.0 on),
  `/simulator/dev/` main's, `/simulator/<tag>/` each release's. `pages.yml`
  attaches the simulator web.yml built for a release's commit to the release
  (`playguard-simulator.zip`: an artifact lasts 30 days, a release's asset
  stays), then lays them out so, with their list in
  `simulator/releases.json` for the simulator's **Version**; main's
  screenshots stay in `/simulator/shots/`, which pull requests compare with.
  The rule for both: `<section>/` what the latest release does,
  `<section>/dev/` main, `<section>/<version>/` that version.
- **Each open pull request's preview** in `/pr-<number>/`: its website and
  its simulator, linked in a comment on the pull request.

## How it is made

`tools/build_site.py <out-dir>` (Python 3 and Python-Markdown) writes the
pages; no framework, nothing to install but that.

- `site/i18n/<language>.json` — the home page's and the header's texts, one
  catalog per language (`en-US.json` the reference: a text missing from
  another catalog is shown in English). In a text, `{{app:<key>}}` is the
  app's own text for that key in that language
  (`resources/i18n/<language>/playguard.json`), so a menu the site names is
  named as on the console, and follows the app when it changes;
  `{{version}}` is the latest release (`.release-please-manifest.json`).
- **The documentation is `docs/`**, converted from its Markdown at each
  build (without its "Back to the README", GitHub's way back): each language
  reads `docs/<language>/` where it has the page (Canadian French reads
  `docs/fr/`), else the English page, with a note saying it is not
  translated yet. A translation's first line,
  `<!-- translated from docs/<page>.md, sha256 <12 hex> -->`
  (`tools/doc_stamp.py docs/<language>/<page>.md`), is the English page it
  matches: once that page changes, the site says under the translation that
  the English one may be more recent, and links to it. GitHub's `> [!WARNING]` blocks become callouts, headings
  keep GitHub's anchors, a paragraph after `<!-- github-only -->` is left out
  (what the site shows otherwise: its screenshots are the page's language's,
  their dark ones under the dark theme), links between the docs and to the README become the
  site's pages, and any other file of the repository links to GitHub. A
  change to `docs/` is a change to the site.
- `site/assets/` — `site.css` (the branding's navy, teal and amber, light and
  dark), `site.js` (the language and theme menus; on a phone, the header
  out of the way while scrolling down, the gallery's dots, the way back to
  the top), `theme.js` (the theme, and the browser's bar in its colour,
  before the first paint) and `fonts/sora.woff2`, the Sora headings' font
  (SIL Open Font License, `fonts/OFL.txt`; its Latin letters, cut out of
  google/fonts' `ofl/sora/Sora[wght].ttf`), served with the site so that no
  visitor's browser asks Google Fonts or anyone else for anything.
- **The screenshots**: `tools/site_screenshots.mjs <simulator> <out-dir>
  [version]` (Node and Playwright) runs the web simulator in a headless
  Chromium and saves each screen of the home page's gallery in each language,
  `<out-dir>/<language>/<screen>.webp`, on the desktop smoke test's console
  (8 October 2026, 16:00 UTC; the focus without its glow). `SITE_SCREENSHOTS=<out-dir>`
  tells `build_site.py` to use them, on the home page and in the
  documentation. Taking them takes about an hour on a CI runner (both
  themes), so CI takes them for a release (main, its version changed: the
  header's badge) and by hand (*Actions › web › Run workflow*, `all`); any
  other build takes main's (its `site` artifact). A change to the screenshots'
  script, or to a screen between releases, shows once taken by hand. A new screen in the gallery is a name in both files: its
  tab, and the keys that lead to it from there.
- **The preview images**: `tools/site_social.mjs <out-dir>` (Node and
  Playwright) writes `<out-dir>/<language>/social.png` (1280×640) from the
  catalog's title and that language's dashboard; `build_site.py` makes it the
  language's `og:image`, else the repository's `images/social-preview.png`.
- **The tour in each language**: `tools/render_tour.mjs <out-dir>` (Node,
  Playwright and ffmpeg) renders `branding/tour.html` with the language's
  texts (its catalog) over its screenshots, `<out-dir>/<language>/tour.mp4`
  (1280×720), the home page's video in that language; English keeps
  `images/video/playguard-tour.mp4`. A few minutes each: rendered with the
  screenshots (a release, or by hand), else main's.
- **The site's address**: `SITE_URL` (`https://playguard.link/`
  by default). CI asks GitHub Pages for it (its custom domain, when it has
  one) and adds a preview's folder; `SITE_BASE` is its path, for 404.html;
  `SITE_NOINDEX=1` for a preview.
- **A Content-Security-Policy** on every page (a `<meta>`: GitHub Pages sends
  no headers): only the site's own files, no inline script or style. The
  JSON-LD blocks are data, not scripts: the policy does not apply to them. A page
  needing something else (a script inline, a file from another site) is
  blocked: put it in `site/assets/`. The pages also use
  `branding/` (the mark, the icon), `images/screenshots/` and the 90-second
  tour (`images/video/playguard-tour.mp4`).

To look at it locally: `python3 tools/build_site.py /tmp/site` then
`python3 web/serve.py /tmp/site`, with `make web` and `cp -r build-web/simulator /tmp/site/simulator`
for the simulator. With the screenshots in each language: `node tools/site_screenshots.mjs build-web/simulator /tmp/shots`
(after `npm install --no-save playwright`; a few languages: `… /tmp/shots 1.2.0 fr ja`),
then `SITE_SCREENSHOTS=/tmp/shots python3 tools/build_site.py /tmp/site`.

## Publishing

`.github/workflows/web.yml` builds the site with the simulator in it for main
and for each pull request (the `site` artifact, kept 30 days), which checks
that it builds. It starts from main's last site and makes only what changed
since the commit that one was built from: the simulator (and its smoke test,
about ten minutes) when the app, its resources, the simulator or the workflow
changed; the screenshots and tours for a release or by hand (above); the
website itself, every time, in seconds. A change to the docs or the site
takes a few minutes. A pull request that changes the site, its documentation or its texts
also has its layout checked (`tools/site_layout.py`, under a minute): no
page wider than the screen on a phone, a tablet or a desktop, no field iOS
zooms into, no script that throws, and nothing axe-core finds against WCAG 2.2
AA (contrast, names, labels, keyboard; `SITE_AXE=<axe.min.js>` locally). main's builds run one at a time, each from the last. `.github/workflows/pages.yml` publishes them, with GitHub
Actions as the source of GitHub Pages: main's at the root, each open pull
request's in `pr-<number>/` (not a fork's: its pages would be served from this
site), then edits the pull request's comment with the links. It runs where
main's workflows run (after each `web` run, when a pull request is closed, or
by hand), as the `github-pages` environment requires, and only puts in place
the files `web.yml` built. Each run publishes everything again, so a closed
pull request's preview goes at the next one.

GitHub lists a workflow's runs by branch *name*, whatever repository the
branch is in: a fork's pull request from a branch named as one of ours
(`main`, or an open pull request's) would come up with them. `pages.yml` takes
only runs of this repository (`head_repository`), so a fork can never have its
files served from the site, and comments only on this repository's pull
requests.

Once: *Settings › Pages › Build and deployment › Source: GitHub Actions*.
