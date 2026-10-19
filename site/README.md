# The website

PlayGuard's website, on GitHub Pages: <https://jigsawfr.github.io/PlayGuard/>.

- **A home page and the documentation in the console's 15 languages**:
  English at the root, the others in `/fr/`, `/fr-ca/`, `/de/`, `/es/`,
  `/es-419/`, `/it/`, `/nl/`, `/pt/`, `/pt-br/`, `/ru/`, `/ja/`, `/ko/`,
  `/zh-hans/`, `/zh-hant/`. A first visit to the root from elsewhere goes to
  the browser's language when the site has it; the header's list changes it
  (and is remembered). The theme is *Auto* (the browser's), *Light* or *Dark*,
  remembered as the web simulator remembers it (both read `pg-theme`).
- **The screenshots in each language**: the console's screens as the
  simulator shows them in that language (below), English where there are
  none (a local build).
- **For search engines and AI assistants**: absolute links (the canonical
  page, each language's, the preview image), `sitemap.xml`, structured data
  (schema.org JSON-LD: the application and the questions on the home page,
  each page of the documentation as an article), each page of the
  documentation in Markdown next to it (`docs/<page>.md`, in English and in
  `/fr/`), listed in [`llms.txt`](https://llmstxt.org) (`llms-full.txt`: the
  whole documentation in one file). A pull request's preview says `noindex`.
  A `robots.txt` would have to be at the root of `jigsawfr.github.io`, not of
  this site, so there is none.
- **The web simulator** in `/simulator/` ([`web/README.md`](../web/README.md)).
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
  build (without its "Back to the README", GitHub's way back): French and Canadian French read `docs/fr/` where it has the page,
  every other language the English page, with a note saying it is not
  translated yet. GitHub's `> [!WARNING]` blocks become callouts, headings
  keep GitHub's anchors, links between the docs and to the README become the
  site's pages, and any other file of the repository links to GitHub. A
  change to `docs/` is a change to the site.
- `site/assets/` — `site.css` (the branding's navy, teal and amber, light and
  dark), `site.js` (the language and theme menus), `theme.js` (the theme
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
  documentation. A new screen in the gallery is a name in both files: its
  tab, and the keys that lead to it from there.
- **The site's address**: `SITE_URL` (`https://jigsawfr.github.io/PlayGuard/`
  by default; a preview's in CI), `SITE_NOINDEX=1` for a preview.
- **A Content-Security-Policy** on every page (a `<meta>`: GitHub Pages sends
  no headers): only the site's own files, no inline script or style. The
  JSON-LD blocks are data, not scripts: the policy does not apply to them. A page
  needing something else (a script inline, a file from another site) is
  blocked: put it in `site/assets/`. The pages also use
  `branding/` (the mark, the icon), `images/screenshots/` and the 30-second
  tour (`images/video/playguard-tour.mp4`).

To look at it locally: `python3 tools/build_site.py /tmp/site` then
`python3 web/serve.py /tmp/site`, with `make web` and `cp -r build-web/simulator /tmp/site/simulator`
for the simulator. With the screenshots in each language: `node tools/site_screenshots.mjs build-web/simulator /tmp/shots`
(after `npm install --no-save playwright`; a few languages: `… /tmp/shots 1.2.0 fr ja`),
then `SITE_SCREENSHOTS=/tmp/shots python3 tools/build_site.py /tmp/site`.

## Publishing

`.github/workflows/web.yml` builds the simulator, takes the screenshots with
it, then builds the site with the simulator in it for main and for each pull
request (the `site` artifact, kept 30 days), which checks that it builds. `.github/workflows/pages.yml` publishes them, with GitHub
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
