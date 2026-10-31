# The website

PlayGuard's website, on GitHub Pages: <https://guard.boreale.dev/>.

- **A home page and the documentation in the console's 15 languages**:
  English at the root, the others in `/fr/`, `/fr-ca/`, `/de/`, `/es/`,
  `/es-419/`, `/it/`, `/nl/`, `/pt/`, `/pt-br/`, `/ru/`, `/ja/`, `/ko/`,
  `/zh-hans/`, `/zh-hant/`. A first visit to the root goes to the browser's
  language when the site has it; the header's language menu changes it (and
  is remembered). Japanese, Korean and Chinese lines break where their
  readers expect (`site.css`: between phrases, between words, no punctuation
  opening a line). One dark look, Boréale's (the night, the aurora:
  [boreale/branding](https://github.com/JigSawFr/boreale/tree/main/branding)),
  as [Aurore's site](https://aurore.boreale.dev/) and
  [boreale.dev](https://boreale.dev/); the web simulator keeps its own theme.
- **The page chrome** (the skip link, the sky, the header and the footer,
  with the buttons, sections and cards of the home page) is boreale-site's
  v4: `Site.skip()`, `Site.sky()`, `Site.header()` and `Site.footer()`
  write it, `chrome.css` and `chrome.js` style and move it (the aurora, the
  header's row of links on a phone, the cards' glow, the reveal), and
  `check.pages(..., chrome=True)` keeps it. `site.css` and `site.js` keep
  what is PlayGuard's own: the hero and its console, the documentation, the
  gallery, the FAQ, the featured and small cards.
- **The header**: the mark and name open the menu of Boréale's sites
  (boreale-site's `Site.switcher()` and `sites.json`, a popover placed by its
  `boreale.js`, styled by its `boreale.css`; the marks boreale-site's, the
  versions boreale.dev's `versions.json` read at the build; each site in the
  page's language, the sites sharing the 15 folders: the homebrew, the
  integrations, Aurore, Atlas and Lantern), and a button opens the 15
  languages, each with its flag (boreale-site's `Site.languages()`, a
  popover too). The language chosen there is kept by boreale-site's
  `lang.js` (`boreale-lang`: the folder, `en-US` for English), loaded in
  each page's `<head>` after `reading.js`: an English page goes to the
  language chosen, at the same place, the root (`data-first-visit`) to the
  browser's on a first visit, nothing chosen; the menu opens the page at
  the same place (`#…`) in the language chosen; the page not found is one
  `404.html` holding every language (`.not-found[data-lang]`), shown in the
  address' language, else the one chosen, else the browser's. A choice
  kept before as `pg-site-lang` moves to `boreale-lang` once (`reading.js`,
  until 2027-01-31). The footer's Boréale column
  (`Site.footer()`, read from `sites.json`): the other sites and the privacy
  and legal notice in the page's language.
  The page not found
  stands under the aurora, as the home page's hero.
- **Reading the documentation**: once the header is scrolled away, a slim
  bar at the top (the way home, the page's name, the search, the light
  reading); *Light reading* puts the documentation's pages on a light
  ground (kept as `pg-reading`, `reading.js` setting it before the first
  paint); the search's field shows its key, `/`.
- **The home page's showcase** (site.css's and site.js's "showcase"
  blocks, the same on the four sites): the tour on a console of our own
  (`.device`, its sides `DEVICE_GRIPS`), leaning towards the text on a
  wide screen; the night going on down the page (veils of aurora, faint
  stars, a horizon over the footer); the features on tiles, their edge lit
  under the pointer, the three first showing their screen; the sections
  rising into view once; on a wide screen, the screens' gallery as one
  large over a strip of the others; the steps joined by a line; a short cross-fade from one page to
  the next where the browser can. Without script, or with less motion
  asked, everything stays in place.
- **The screenshots in each language**: the console's screens as the
  simulator shows them in that language (below), English where there are
  none (a local build); the console's dark ones, as the site is.
- **On a phone** (an iPhone in Safari or Edge, the same WebKit): no field
  under 16px (iOS zooms into one), 44px to touch, the page beside the notch
  in landscape (`viewport-fit=cover`, `env(safe-area-inset-*)`), each page's
  sections folded under its first paragraph, and nothing wider than the
  screen (a long `code` breaks). The header goes on two rows up to 1080px,
  where its links no longer fit beside the menus in every language.
- **The home page**: install first (the download under it, with the
  release's date), the three main features then the others, the questions
  each with its own address (`#faq-<question>`, opened by a link to it). The
  screenshots open
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
  documentation in Markdown next to it (`docs/<page>.md`, in each language
  `docs/` has a folder for; Canadian French reads French's), listed in [`llms.txt`](https://llmstxt.org) (`llms-full.txt`: the
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
  their dark ones), links between the docs and to the README become the
  site's pages, and any other file of the repository links to GitHub. A
  change to `docs/` is a change to the site.
- `site/assets/` — `site.css` (the site's look, over boreale-site's
  `boreale.css`), `site.js` (the tour, the gallery, the
  documentation's search; on a phone, the gallery's dots, the way back to
  the top; the home page's aurora is boreale-site's `chrome.js`, still when
  less motion is asked) and `reading.js` (the light reading before the first
  paint; the old language key's move).
- **What Boréale's sites share** is
  [boreale-site](https://github.com/JigSawFr/boreale-site), the submodule
  `library/boreale-site` (`git submodule update --init`): the languages, the
  header's two menus and their texts (its `i18n/`, which `site/i18n/` does
  not repeat: the build stops if it does), the links out and their UTM
  tags, the audience, the checks every site passes, and the assets copied
  into `assets/` at the build (`boreale.copy_assets()`): `boreale.css`
  (Boréale's colours and fonts, the two menus), `chrome.css` and `chrome.js`
  (the page chrome, above), `boreale.js` (where the menus open), `lang.js` (the language chosen, the header, above), the
  sites' marks, Umami's tracker and the fonts (Sora for the
  headings, Figtree for the text, JetBrains Mono for code and labels, their
  Latin letters; SIL Open Font License, `fonts/OFL-*.txt`; the language
  menu's 15 flags cut out of Twemoji's, CC-BY 4.0,
  `fonts/LICENSE-twemoji-flags.txt`), served with the site so that no
  visitor's browser loads anything from Google Fonts or anyone else. Its
  checks (`boreale_site.check.pages()`) are `tools/check_site_links.py`'s:
  the simulator built apart (`apart`), 404.html's links from `SITE_BASE`,
  no tracker on a preview.
- **The screenshots**: `tools/site_screenshots.mjs <simulator> <out-dir>
  [version]` (Node and Playwright) runs the web simulator in a headless
  Chromium and saves each screen of the home page's gallery in each language,
  `<out-dir>/<language>/<screen>.webp`, on the desktop smoke test's console
  (8 October 2026, 16:00 UTC; the focus without its glow). `SITE_SCREENSHOTS=<out-dir>`
  tells `build_site.py` to use them, on the home page and in the
  documentation. Taking them takes about three hours on a CI runner (both
  themes), so CI takes them by hand (*Actions › web › Run workflow*, `all`)
  and for a release (main, its version changed: the header's badge), in a run
  of their own: the release's site goes out first, in minutes, with the
  repository's English screenshots (`images/screenshots/`, taken for the
  release) and main's in the other languages, then its `release-screens` job
  starts that `all` run, published once it ends; any other build takes main's
  (its `site` artifact). A change to the screenshots'
  script, or to a screen between releases, shows once taken by hand. A new screen in the gallery is a name in both files: its
  tab, and the keys that lead to it from there.
- **The preview images**: `tools/site_social.mjs <out-dir>` (Node and
  Playwright) writes `<out-dir>/<language>/social.png` (1280×640) from the
  catalog's title and that language's dashboard; `build_site.py` makes it the
  language's `og:image`, else the repository's `images/social-preview.png`.
- **The tour in each language**: `tools/render_tour.mjs <out-dir>` (Node,
  Playwright and ffmpeg) renders `branding/tour.html` with the language's
  texts (its catalog's `tour_*` keys) over its screenshots and the gate's in
  that language (`make gate-preview`), the scenes the language has a
  screenshot for, with the music, `<out-dir>/<language>/tour.mp4`
  (1280×720), the home page's video in that language; English keeps
  `images/video/playguard-tour.mp4`. A few minutes each: rendered with the
  screenshots (a release, or by hand), else main's.
- **The site's address**: `SITE_URL` (`https://guard.boreale.dev/`
  by default). CI asks GitHub Pages for it (its custom domain, when it has
  one) and adds a preview's folder; `SITE_BASE` is its path, for 404.html
  (the address' path by default);
  `SITE_NOINDEX=1` for a preview.
- **The audience**, counted by Umami Cloud (<https://cloud.umami.is>):
  `assets/umami.js` (boreale-site's copy) is its tracker, served with the
  site, that sends each page seen to `gateway.umami.is`, without cookie, and
  only from the site's own address (`SITE_URL`'s, not a local copy); a
  preview has none. Its site in Umami (boreale-site's `UMAMI_WEBSITE`) is
  Boréale's, shared with the other projects' sites: Umami's *Hostname*
  filter tells them apart.
  A link to another of our sites (boreale.dev, its other sub-domains) says
  where the visit comes from, for Umami's *UTM* report: boreale-site's
  `Site.links_out()`, over every page, adds `utm_source=guard` and
  `utm_medium=nav` (header), `footer` or `link` (the page's text); a link to
  guard.boreale.dev itself loses the tags a documentation page was written
  with for GitHub (`utm_source=github`, `Site.untagged()` in the Markdown and
  `llms.txt` too), and `tools/check_site_links.py` checks both, with
  boreale-site's checks (`boreale_site.check.pages()`, `check_catalogs()`)
  beside its own walk of the links (which knows the simulator is built
  apart, and where `404.html` is served). The
  app's QR codes to the documentation carry `utm_source=playguard&utm_medium=qr`
  (`app::doc_url()`), the release notes `utm_source=release`.
- **A Content-Security-Policy** on every page (a `<meta>`: GitHub Pages sends
  no headers): only the site's own files, no inline script or style; the one
  address it may send to is Umami's (`connect-src`). The
  JSON-LD blocks are data, not scripts: the policy does not apply to them. A page
  needing something else (a script inline, a file from another site) is
  blocked: put it in `site/assets/`. The pages also use
  `branding/` (the mark, the icon), `images/screenshots/` and the 90-second
  tour (`images/video/playguard-tour.mp4`), muted until its sound button
  (site.js) turns its music on.

To look at it locally: `git submodule update --init library/boreale-site`
(once), `python3 tools/build_site.py /tmp/site` then
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
changed; the screenshots and tours by hand, or for a release in a run after its site (above); the
website itself, every time, in seconds. A change to the docs or the site
takes a few minutes. A pull request that changes the site, its documentation or its texts
also has its layout checked (boreale-site's `tools/site_layout.py`, a few
minutes): no page wider than the screen on a phone, a tablet or a desktop, no
target under 24px, no field iOS zooms into, no script that throws, and
nothing axe-core finds against WCAG 2.2 AA (contrast, names, labels,
keyboard; `SITE_AXE=<axe.min.js>` locally). A sample, chosen by its options
in `web.yml`: every page on an iPhone but the documentation's versions
(`docs/dev/`, `docs/v<x>/`) outside English and French; the home page, the
documentation's widest and the changelog in each language on the four other
screens; axe on the English pages and each language's home page. main's builds run one at a time, each from the last. `.github/workflows/pages.yml` publishes them, with GitHub
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
