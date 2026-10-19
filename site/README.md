# The website

PlayGuard's website, on GitHub Pages: <https://jigsawfr.github.io/PlayGuard/>.

- **A home page and the documentation in the console's 15 languages**:
  English at the root, the others in `/fr/`, `/fr-ca/`, `/de/`, `/es/`,
  `/es-419/`, `/it/`, `/nl/`, `/pt/`, `/pt-br/`, `/ru/`, `/ja/`, `/ko/`,
  `/zh-hans/`, `/zh-hant/`. A first visit to the root from elsewhere goes to
  the browser's language when the site has it; the header's list changes it
  (and is remembered). The theme is *Auto* (the browser's), *Light* or *Dark*,
  remembered as the web simulator remembers it (both read `pg-theme`).
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
  build: French and Canadian French read `docs/fr/` where it has the page,
  every other language the English page, with a note saying it is not
  translated yet. GitHub's `> [!WARNING]` blocks become callouts, headings
  keep GitHub's anchors, links between the docs and to the README become the
  site's pages, and any other file of the repository links to GitHub. A
  change to `docs/` is a change to the site.
- `site/assets/` — `site.css` (the branding's navy, teal and amber, light and
  dark) and `site.js` (the language and theme menus). The pages also use
  `branding/` (the mark, the icon), `images/screenshots/` and the 30-second
  tour (`images/video/playguard-tour.mp4`).

To look at it locally: `python3 tools/build_site.py /tmp/site` then
`python3 web/serve.py /tmp/site`, with `make web` and `cp -r build-web/simulator /tmp/site/simulator`
for the simulator.

## Publishing

`.github/workflows/web.yml` builds the site with the simulator in it for main
and for each pull request (the `site` artifact, kept 30 days), which checks
that it builds. `.github/workflows/pages.yml` publishes them, with GitHub
Actions as the source of GitHub Pages: main's at the root, each open pull
request's in `pr-<number>/` (not a fork's: its pages would be served from this
site), then edits the pull request's comment with the links. It runs where
main's workflows run (after each `web` run, when a pull request is closed, or
by hand), as the `github-pages` environment requires, and only puts in place
the files `web.yml` built. Each run publishes everything again, so a closed
pull request's preview goes at the next one.

Once: *Settings › Pages › Build and deployment › Source: GitHub Actions*.
