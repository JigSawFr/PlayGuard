#!/usr/bin/env python3
"""Checks the links of the built website (site/README.md): every link and
image of its pages that stays on the site must reach a file of it, and every
#anchor an id of the page it points to. Links to the simulator (built apart,
web/README.md) and to other sites are not followed.

Then boreale-site's checks, the same for each of Boréale's sites
(boreale_site.check.pages(), library/boreale-site/README.md): links out by
the rule "liens sortants anonymes" (no referrer sent; rel="noreferrer" to a
third party; referrerpolicy="strict-origin" to *.boreale.dev and
github.com/JigSawFr), utm_source=guard and a utm_medium on a link to another
of our sites and on no other, the Content-Security-Policy and what it
blocks, Umami's tracker, no {{key}} left, no e-mail address, the pages' data;
and the catalogs (check_catalogs(): site/i18n/ and boreale-site's i18n/).

Usage: tools/check_site_links.py <site-dir> [site-url]
(site-url: the site's own address, whose absolute links are checked too)
"""
import os
import sys
from html.parser import HTMLParser
from pathlib import Path
from urllib.parse import unquote, urlsplit

# What Boréale's sites share (library/boreale-site, a submodule): the checks
# every built site passes, and the common catalogs.
ROOT = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(ROOT / "library" / "boreale-site"))
import boreale_site as boreale  # noqa: E402
from boreale_site import check as boreale_check  # noqa: E402

# boreale-site's page checks this site does otherwise: where links lead (the
# walk below knows the simulator is built apart and where 404.html is
# served, SITE_BASE), and the policy and the tracker of docs/'s pages that
# send their visitor on at once (build_site.py's redirect_page(): no script,
# counted at the page they send to). A preview (SITE_NOINDEX=1) has no
# tracker. A boreale-site option for paths built apart, the 404's base and
# such pages would replace this list (see site/README.md).
NOT_HERE = (" leads nowhere", " is not on the page")
SENDS_ON = ("no Content-Security-Policy", "no Umami tracker (assets/umami.js)")


def shared_problems(root: str, site: str) -> list:
    """boreale-site's checks (boreale_site.check.pages(), its catalogs'
    check_catalogs()) on the built site, but for what this one does itself."""
    errors, _ = boreale_check.pages(Path(root), boreale.Site("playguard", site))
    kept = []
    for error in errors:
        name, _, problem = error.partition(": ")
        if problem.endswith(NOT_HERE):
            continue
        if problem in SENDS_ON and 'http-equiv="refresh"' in (Path(root) / name).read_text(encoding="utf-8"):
            continue
        if problem == SENDS_ON[1] and os.environ.get("SITE_NOINDEX") == "1":
            continue
        kept.append(error)
    return kept + boreale.check_catalogs(ROOT / "site" / "i18n")


class Page(HTMLParser):
    def __init__(self):
        super().__init__()
        self.links, self.ids = [], set()

    def handle_starttag(self, tag, attrs):
        a = dict(attrs)
        if "id" in a:
            self.ids.add(a["id"])
        for key in ("href", "src", "poster"):
            # <link rel="alternate|canonical|sitemap"> name pages by address: checked as links too.
            if a.get(key):
                self.links.append(a[key])


def main():
    if len(sys.argv) < 2:
        print(__doc__)
        return 2
    root = os.path.abspath(sys.argv[1])
    site = sys.argv[2].rstrip("/") + "/" if len(sys.argv) > 2 else None
    pages = {}
    for folder, _, files in os.walk(root):
        for name in files:
            if name.endswith(".html"):
                path = os.path.join(folder, name)
                page = Page()
                with open(path, encoding="utf-8") as f:
                    html = f.read()
                page.feed(html)
                pages[path] = page

    def target(path):
        if os.path.isdir(path):
            path = os.path.join(path, "index.html")
        return path

    errors = 0
    for problem in shared_problems(root, site or os.environ.get("SITE_URL")):
        print(problem)
        errors += 1
    for path, page in sorted(pages.items()):
        if os.path.basename(path) == "404.html":
            continue   # served at any path: its links start from the site's root, elsewhere
        for link in page.links:
            if site and link.startswith(site):
                link = os.path.relpath(os.path.join(root, link[len(site):]), os.path.dirname(path)) + \
                       ("/" if link.endswith("/") else "")
            parts = urlsplit(link)
            if parts.scheme or link.startswith(("mailto:", "data:", "//")):
                continue
            file = target(os.path.normpath(os.path.join(os.path.dirname(path), unquote(parts.path)))) \
                if parts.path else path
            rel = os.path.relpath(file, root)
            if rel.split(os.sep)[0] == "simulator" or rel.startswith(".."):
                continue
            if not os.path.exists(file):
                print(f"{os.path.relpath(path, root)}: {link}: no {rel}")
                errors += 1
            elif parts.fragment and file.endswith(".html"):
                ids = pages[file].ids if file in pages else set()
                if unquote(parts.fragment) not in ids:
                    print(f"{os.path.relpath(path, root)}: {link}: no #{unquote(parts.fragment)} in {rel}")
                    errors += 1
    print(f"site links: {len(pages)} pages, {errors} problem(s)")
    return 1 if errors else 0


if __name__ == "__main__":
    sys.exit(main())
