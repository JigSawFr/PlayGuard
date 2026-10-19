#!/usr/bin/env python3
"""Builds PlayGuard's website (site/README.md) into <out-dir>: a home page and
the documentation (docs/*.md, docs/fr/*.md) in each of the console's 15
languages, English at the root and the others in /<language>/.

- The home page's texts are site/i18n/<language>.json (en-US.json the
  reference; a text missing elsewhere is shown in English). `{{app:<key>}}`
  in them is the app's own text for that key, in that language
  (resources/i18n/<language>/playguard.json), so a menu named on the site is
  named as on the console; `{{version}}` the latest release
  (.release-please-manifest.json).
- The documentation is the Markdown of docs/: French (and Canadian French)
  read docs/fr/ where it has the page, every other language the English one,
  with a note saying so. Links between the docs and to the README become the
  site's pages; other files of the repository link to GitHub.
- The simulator (web/README.md) is not built here: the deployment
  (.github/workflows/pages.yml) puts it in /simulator/.

Usage: tools/build_site.py <out-dir> [repo-root]          (needs Python-Markdown)
"""
import html
import json
import os
import re
import shutil
import sys
from urllib.parse import urlsplit

import markdown

REPO_URL = "https://github.com/JigSawFr/PlayGuard"
# The console's languages: config.json's code, the site's folder ('' for the
# root), the name in its own words, the html lang.
LANGUAGES = [
    ("en-US", "", "English", "en"),
    ("fr", "fr", "Français", "fr"),
    ("fr-CA", "fr-ca", "Français (Canada)", "fr-CA"),
    ("de", "de", "Deutsch", "de"),
    ("es", "es", "Español", "es"),
    ("es-419", "es-419", "Español (Latinoamérica)", "es-419"),
    ("it", "it", "Italiano", "it"),
    ("nl", "nl", "Nederlands", "nl"),
    ("pt", "pt", "Português", "pt-PT"),
    ("pt-BR", "pt-br", "Português (Brasil)", "pt-BR"),
    ("ru", "ru", "Русский", "ru"),
    ("ja", "ja", "日本語", "ja"),
    ("ko", "ko", "한국어", "ko"),
    ("zh-Hans", "zh-hans", "简体中文", "zh-Hans"),
    ("zh-Hant", "zh-hant", "繁體中文", "zh-Hant"),
]
# The documentation, in the sidebar's order.
DOCS = [("features", "guide"), ("install", "guide"), ("compatibility", "guide"), ("troubleshooting", "guide"),
        ("companion-app", "reference"), ("config", "reference"), ("parental-controls", "reference")]
# The languages that read docs/fr/.
FRENCH = {"fr", "fr-CA"}
SCREENSHOTS = ["dashboard", "play_timer", "per_day", "activity", "restrictions", "security", "console", "clock",
               "preferences"]
FEATURES = [("timer", "⏱️"), ("activity", "📊"), ("restrictions", "🔒"), ("console", "🕒"), ("app", "📱"),
            ("history", "↩️"), ("notices", "🔔"), ("languages", "🌍")]

ROOT = "."
APP_TEXTS = {}


def flat(d, prefix=""):
    out = {}
    for k, v in d.items():
        if isinstance(v, dict):
            out.update(flat(v, f"{prefix}{k}/"))
        else:
            out[f"{prefix}{k}"] = v
    return out


def read_json(*parts):
    with open(os.path.join(ROOT, *parts), encoding="utf-8") as f:
        return json.load(f)


def app_text(lang, key):
    if lang not in APP_TEXTS:
        APP_TEXTS[lang] = flat(read_json("resources", "i18n", lang, "playguard.json"))
    value = APP_TEXTS[lang].get(key) or flat(read_json("resources", "i18n", "en-US", "playguard.json")).get(key)
    if value is None:
        raise SystemExit(f"site: no app text {key!r}")
    return value.rstrip(".…").rstrip()   # "Unlink the companion app..." as a menu's name


class Texts:
    """The home page's texts in one language, English underneath."""

    def __init__(self, lang, version):
        self.lang, self.version = lang, version
        self.english = read_json("site", "i18n", "en-US.json")
        path = os.path.join(ROOT, "site", "i18n", f"{lang}.json")
        self.own = read_json("site", "i18n", f"{lang}.json") if os.path.exists(path) else {}

    def __call__(self, key, escape=True):
        value = self.own.get(key, self.english.get(key))
        if value is None:
            raise SystemExit(f"site: no text {key!r} in site/i18n/en-US.json")
        value = value.replace("{{version}}", self.version)
        value = re.sub(r"\{\{app:([^}]+)\}\}", lambda m: app_text(self.lang, m.group(1)), value)
        return html.escape(value) if escape else value


# ---- Markdown ------------------------------------------------------------

def github_slug(value, separator="-"):
    """The anchor GitHub gives a heading, so links written for GitHub still work."""
    s = html.unescape(re.sub(r"<[^>]+>", "", value)).strip().lower()
    s = re.sub(r"[^\w\- ]", "", s)
    return s.replace(" ", separator)


ALERTS = {"NOTE": "note", "TIP": "tip", "IMPORTANT": "important", "WARNING": "warning", "CAUTION": "caution"}


def alerts(body):
    """GitHub's "> [!WARNING]" blockquotes as callouts."""
    def callout(m):
        kind = ALERTS[m.group(1)]
        return f'<div class="callout {kind}">\n<p>'
    body = re.sub(r"<blockquote>\s*<p>\[!(NOTE|TIP|IMPORTANT|WARNING|CAUTION)\]\s*(?:<br\s*/?>)?\s*", callout, body)
    # The blockquotes opened as callouts close as divs.
    out, stack = [], []
    for token in re.split(r"(<blockquote>|</blockquote>|<div class=\"callout [a-z]+\">)", body):
        if token == "<blockquote>" or token.startswith("<div class=\"callout"):
            stack.append(token)
            out.append(token)
        elif token == "</blockquote>":
            opened = stack.pop() if stack else "<blockquote>"
            out.append("</div>" if opened.startswith("<div") else token)
        else:
            out.append(token)
    return "".join(out)


def rewrite_links(body, source, lang, to_root, assets):
    """Links of a document of the repository (`source`, its path) as the site's."""
    folder = os.path.dirname(source)

    def target(url):
        parts = urlsplit(url)
        if parts.scheme or url.startswith(("#", "mailto:")):
            return url
        path = os.path.normpath(os.path.join(folder, parts.path)).replace(os.sep, "/")
        anchor = f"#{parts.fragment}" if parts.fragment else ""
        name = os.path.basename(path)
        if path.startswith("docs/") and name.endswith(".md") and name[:-3] in dict(DOCS):
            return f"{to_root}{lang_dir(lang)}docs/{name[:-3]}/{anchor}"
        if path in ("README.md", "README.fr.md"):
            return f"{to_root}{lang_dir(lang)}{anchor}"
        if path.startswith("images/"):
            assets.add(path)
            return f"{to_root}assets/{path}"
        return f"{REPO_URL}/blob/main/{path}{anchor}"

    body = re.sub(r'(href|src)="([^"]+)"', lambda m: f'{m.group(1)}="{html.escape(target(html.unescape(m.group(2))))}"', body)
    return body


def lang_dir(lang):
    folder = next(d for code, d, _, _ in LANGUAGES if code == lang)
    return f"{folder}/" if folder else ""


def render_doc(page, lang, to_root, assets):
    """(title, html, translated, source path) of docs/<page>.md for `lang`."""
    source = f"docs/{page}.md"
    if lang in FRENCH and os.path.exists(os.path.join(ROOT, "docs", "fr", f"{page}.md")):
        source = f"docs/fr/{page}.md"
    translated = lang == "en-US" or source.startswith("docs/fr/")
    with open(os.path.join(ROOT, source), encoding="utf-8") as f:
        text = f.read()
    md = markdown.Markdown(extensions=["tables", "fenced_code", "sane_lists", "toc"],
                           extension_configs={"toc": {"slugify": github_slug, "toc_depth": "2-3"}})
    body = md.convert(text)
    title_match = re.search(r"<h1[^>]*>(.*?)</h1>", body, re.S)
    title = re.sub(r"<[^>]+>", "", title_match.group(1)) if title_match else page
    body = re.sub(r"<h1[^>]*>.*?</h1>", "", body, count=1, flags=re.S)
    body = alerts(rewrite_links(body, source, lang, to_root, assets))
    body = re.sub(r"<table>", '<div class="table"><table>', body).replace("</table>", "</table></div>")
    return html.unescape(title), body, md.toc_tokens, translated, source


# ---- Pages ------------------------------------------------------------

def page(t, lang, to_root, path, title, body, description=None, body_class=""):
    """A whole page: `path` is its place under the language's folder ('' or 'docs/x/')."""
    code, folder, _, html_lang = next(l for l in LANGUAGES if l[0] == lang)
    alternates = "\n".join(
        f'  <link rel="alternate" hreflang="{hl}" href="{to_root}{f"{d}/" if d else ""}{path}">'
        for _, d, _, hl in LANGUAGES)
    options = "\n".join(
        f'        <option value="{to_root}{f"{d}/" if d else ""}{path}" data-lang="{c}" lang="{hl}"'
        f'{" selected" if c == lang else ""}>{html.escape(n)}</option>'
        for c, d, n, hl in LANGUAGES)
    home = f"{to_root}{lang_dir(lang)}"
    full_title = "PlayGuard" if not title else f"{title} · PlayGuard"
    return f"""<!doctype html>
<html lang="{html_lang}" data-lang="{code}">
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>{html.escape(full_title)}</title>
  <meta name="description" content="{description or t('meta_description')}">
  <meta property="og:title" content="{html.escape(full_title)}">
  <meta property="og:description" content="{description or t('meta_description')}">
  <meta property="og:image" content="{to_root}assets/images/social-preview.png">
  <meta name="theme-color" content="#0b1424">
  <link rel="icon" href="{to_root}assets/icon.svg" type="image/svg+xml">
{alternates}
  <script>
    (function () {{
      var c = null;
      try {{ c = localStorage.getItem('pg-theme'); }} catch (e) {{}}
      if (c !== 'light' && c !== 'dark') c = matchMedia('(prefers-color-scheme: dark)').matches ? 'dark' : 'light';
      document.documentElement.dataset.theme = c;
    }})();
  </script>
  <link rel="stylesheet" href="{to_root}assets/site.css">
</head>
<body class="{body_class}">
  <a class="skip" href="#main">{t('skip')}</a>
  <header class="top">
    <a class="brand" href="{home}"><img src="{to_root}assets/mark.svg" alt="" width="34" height="34"><span>Play<b>Guard</b></span></a>
    <nav class="nav" aria-label="PlayGuard">
      <a href="{home}#features">{t('nav_features')}</a>
      <a href="{home}#install">{t('nav_install')}</a>
      <a href="{home}docs/features/">{t('nav_docs')}</a>
      <a href="{to_root}simulator/?lang={code}" class="nav-try">{t('nav_simulator')}</a>
      <a href="{REPO_URL}" class="nav-gh" aria-label="GitHub">GitHub</a>
    </nav>
    <div class="settings">
      <select id="site-language" aria-label="{t('language')}">
{options}
      </select>
      <select id="site-theme" aria-label="{t('theme')}">
        <option value="auto">{t('theme_auto')}</option>
        <option value="light">{t('theme_light')}</option>
        <option value="dark">{t('theme_dark')}</option>
      </select>
    </div>
  </header>
  <main id="main">
{body}
  </main>
  <footer class="footer">
    <div class="footer-inner">
      <p><img src="{to_root}assets/mark.svg" alt="" width="20" height="20"> <strong>PlayGuard</strong> · {t('footer_license')}
        · <a href="{REPO_URL}">GitHub</a> · <a href="{REPO_URL}/blob/main/CHANGELOG.md">{t('footer_changelog')}</a>
        · <a href="https://github.com/sponsors/JigSawFr">{t('support')}</a> · <a href="https://ko-fi.com/jigsawfr">Ko-fi</a></p>
      <p class="fine">{t('footer_nintendo')}</p>
    </div>
  </footer>
  <script src="{to_root}assets/site.js"></script>
</body>
</html>
"""


def home_body(t, lang, to_root):
    code = lang
    features = "\n".join(
        f'      <li class="card"><span class="emoji" aria-hidden="true">{emoji}</span>'
        f'<h3>{t(f"f_{key}_title")}</h3><p>{t(f"f_{key}_text")}</p></li>'
        for key, emoji in FEATURES)
    shots = "\n".join(
        f'      <figure><img src="{to_root}assets/images/screenshots/{s}.png" alt="{t(f"shot_{s}")}" '
        f'width="1280" height="720" loading="lazy"><figcaption>{t(f"shot_{s}")}</figcaption></figure>'
        for s in SCREENSHOTS)
    docs = docs_list(t, lang, to_root, None)
    home = f"{to_root}{lang_dir(lang)}"
    return f"""    <section class="hero">
      <div class="hero-text">
        <p class="wordmark" aria-hidden="true">Play<span>Guard</span></p>
        <h1>{t('hero_title')}</h1>
        <p class="sub">{t('hero_sub')}</p>
        <p>{t('hero_text')}</p>
        <div class="actions">
          <a class="button primary" href="{REPO_URL}/releases/latest">{t('download')}</a>
          <a class="button" href="{to_root}simulator/?lang={code}">{t('try')}</a>
          <a class="button ghost" href="{REPO_URL}">{t('source')}</a>
        </div>
      </div>
      <div class="hero-video">
        <video src="{to_root}assets/images/video/playguard-tour.mp4" autoplay muted loop playsinline preload="metadata"
               poster="{to_root}assets/images/screenshots/dashboard.png" aria-label="{t('tour_title')}"></video>
      </div>
    </section>

    <aside class="callout warning wide"><p><strong>{t('cfw_title')}</strong> {t('cfw_text')}</p></aside>

    <section id="features" class="section">
      <h2>{t('features_title')}</h2>
      <ul class="cards">
{features}
      </ul>
    </section>

    <section id="screens" class="section">
      <h2>{t('screens_title')}</h2>
      <div class="gallery">
{shots}
      </div>
    </section>

    <section id="simulator" class="section band">
      <div>
        <h2>{t('simulator_title')}</h2>
        <p>{t('simulator_text')}</p>
      </div>
      <a class="button primary" href="{to_root}simulator/?lang={code}">{t('simulator_button')}</a>
    </section>

    <section id="install" class="section columns">
      <div>
        <h2>{t('install_title')}</h2>
        <ol class="steps">
          <li>{t('install_1')}</li>
          <li>{t('install_2')} <a href="{REPO_URL}/releases/latest">{t('download')}</a></li>
          <li>{t('install_3')}</li>
        </ol>
      </div>
      <div>
        <h2>{t('start_title')}</h2>
        <ol class="steps">
          <li>{t('start_1')}</li>
          <li>{t('start_2')}</li>
          <li>{t('start_3')}</li>
        </ol>
        <p>{t('locked_out')} <a href="{home}docs/troubleshooting/#{locked_out_anchor(lang)}">{t('locked_out_link')}</a></p>
      </div>
    </section>

    <section id="compatibility" class="section">
      <h2>{t('compat_title')}</h2>
      <p>{t('compat_text')} <a href="{home}docs/compatibility/">{t('compat_link')}</a></p>
    </section>

    <section id="docs" class="section">
      <h2>{t('docs_title')}</h2>
{docs}
    </section>
"""


def locked_out_anchor(lang):
    # The anchor of "Locked out?" in the troubleshooting page the language reads.
    source = "docs/fr/troubleshooting.md" if lang in FRENCH else "docs/troubleshooting.md"
    with open(os.path.join(ROOT, source), encoding="utf-8") as f:
        for line in f:
            if line.startswith("## ") and ("Locked out" in line or "bloquée" in line):
                return github_slug(line[3:].strip())
    return ""


def docs_list(t, lang, to_root, current):
    home = f"{to_root}{lang_dir(lang)}"
    out = []
    for section in ("guide", "reference"):
        items = "".join(
            f'<li><a href="{home}docs/{p}/"{" aria-current=\"page\"" if p == current else ""}>{t(f"doc_{p}")}</a></li>'
            for p, s in DOCS if s == section)
        out.append(f'      <div class="doc-group"><h3>{t(f"docs_{section}")}</h3><ul>{items}</ul></div>')
    return f'      <div class="doc-list">\n' + "\n".join(out) + "\n      </div>"


def doc_body(t, lang, to_root, page_name, assets):
    title, body, toc, translated, source = render_doc(page_name, lang, to_root, assets)
    note = "" if translated else f'      <p class="callout note">{t("doc_untranslated")}</p>\n'
    # The page's sections (its h2: toc_depth leaves the h1 out).
    toc_items = "".join(f'<li><a href="#{h["id"]}">{h["name"]}</a></li>' for h in toc)
    return title, f"""    <div class="doc">
      <nav class="doc-nav" aria-label="{t('docs_title')}">
{docs_list(t, lang, to_root, page_name)}
      </nav>
      <article class="prose">
        <h1>{html.escape(title)}</h1>
{note}{body}
        <p class="edit"><a href="{REPO_URL}/blob/main/{source}">{t('doc_on_github')}</a></p>
      </article>
      {f'<nav class="toc" aria-label="{html.escape(title)}"><ul>{toc_items}</ul></nav>' if toc_items else ''}
    </div>
"""


def write(out, rel, text):
    path = os.path.join(out, rel)
    os.makedirs(os.path.dirname(path), exist_ok=True)
    with open(path, "w", encoding="utf-8") as f:
        f.write(text)


def main():
    global ROOT
    if len(sys.argv) < 2:
        print(__doc__)
        return 2
    out = sys.argv[1]
    ROOT = sys.argv[2] if len(sys.argv) > 2 else "."
    version = "v" + read_json(".release-please-manifest.json")["."]
    os.makedirs(out, exist_ok=True)
    assets = {"images/social-preview.png", "images/video/playguard-tour.mp4"}
    assets.update(f"images/screenshots/{s}.png" for s in SCREENSHOTS)
    pages = 0
    for lang, folder, _, _ in LANGUAGES:
        t = Texts(lang, version)
        base = f"{folder}/" if folder else ""
        to_root = "../" if folder else ""
        write(out, f"{base}index.html", page(t, lang, to_root, "", "", home_body(t, lang, to_root), body_class="home"))
        for name, _ in DOCS:
            doc_root = to_root + "../../"
            title, body = doc_body(t, lang, doc_root, name, assets)
            write(out, f"{base}docs/{name}/index.html", page(t, lang, doc_root, f"docs/{name}/", title, body))
            pages += 1
        pages += 1
    # GitHub Pages serves 404.html at any path: its links start from the site's root.
    t = Texts("en-US", version)
    base_url = os.environ.get("SITE_BASE", "/PlayGuard/")
    write(out, "404.html", page(t, "en-US", base_url, "", t("not_found_title"),
                                f'    <section class="section narrow"><h1>{t("not_found_title")}</h1><p>{t("not_found_text")}</p>'
                                f'<p><a class="button primary" href="{base_url}">{t("home")}</a></p></section>'))
    # The site's own files, then what the pages use from the repository.
    os.makedirs(os.path.join(out, "assets"), exist_ok=True)
    for name in os.listdir(os.path.join(ROOT, "site", "assets")):
        shutil.copy(os.path.join(ROOT, "site", "assets", name), os.path.join(out, "assets", name))
    for name in ("icon.svg", "mark.svg"):
        shutil.copy(os.path.join(ROOT, "branding", name), os.path.join(out, "assets", name))
    for path in sorted(assets):
        os.makedirs(os.path.dirname(os.path.join(out, "assets", path)), exist_ok=True)
        shutil.copy(os.path.join(ROOT, path), os.path.join(out, "assets", path))
    open(os.path.join(out, ".nojekyll"), "w").close()
    print(f"site: {pages} pages in {len(LANGUAGES)} languages, {len(assets)} images and videos, in {out}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
