#!/usr/bin/env python3
"""Checks the built website (site/README.md) with boreale-site's checks, the
same for each of Boréale's sites (boreale_site.check.pages(),
library/boreale-site/README.md):

- every link and image of its pages to the site (relative, from its root, or
  its own address: canonical, other languages, sitemap) leads to a file of
  it, and every #anchor to an id of the page it names; the simulator, built
  apart (web/README.md), is neither walked nor followed; 404.html's links
  start from where it is served (SITE_BASE: /PlayGuard/pr-<n>/ for a pull
  request's preview);
- links out by the rule "liens sortants anonymes" (no referrer sent;
  rel="noreferrer" to a third party; referrerpolicy="strict-origin" to
  *.boreale.dev and github.com/JigSawFr), utm_source=guard and a utm_medium
  on a link to another of our sites and on no other;
- the Content-Security-Policy and what it blocks, Umami's tracker on every
  page (not a preview's, SITE_NOINDEX=1, counted by no one); a page that
  sends its visitor on at once (docs/'s, build_site.py's redirect_page())
  needs neither;
- the page chrome (library/boreale-site, "The page chrome (v4)"): one header
  with the name's menu and the nav, the call to action (the simulator) before
  GitHub, the footer linking to every other of Boréale's sites, chrome.css and
  chrome.js loaded;
- no {{key}} left, no e-mail address, the pages' data;

then the catalogs (check_catalogs(): site/i18n/ and boreale-site's i18n/).

Usage: tools/check_site_links.py <site-dir> [site-url]
(site-url: the site's own address, whose absolute links are checked too;
SITE_URL by default)
"""
import os
import sys
from pathlib import Path

# What Boréale's sites share (library/boreale-site, a submodule): the checks
# every built site passes, and the common catalogs.
ROOT = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(ROOT / "library" / "boreale-site"))
import boreale_site as boreale  # noqa: E402
from boreale_site import check  # noqa: E402


def main():
    if len(sys.argv) < 2:
        print(__doc__)
        return 2
    out = Path(sys.argv[1])
    site = boreale.Site("playguard", sys.argv[2] if len(sys.argv) > 2 else os.environ.get("SITE_URL"))
    errors, count = check.pages(out, site, apart=("simulator",), tracker=os.environ.get("SITE_NOINDEX") != "1",
                                base=os.environ.get("SITE_BASE"), chrome=True)
    errors += boreale.check_catalogs(ROOT / "site" / "i18n")
    for error in errors:
        print(error)
    print(f"site links: {count} pages, {len(errors)} problem(s)")
    return 1 if errors else 0


if __name__ == "__main__":
    sys.exit(main())
