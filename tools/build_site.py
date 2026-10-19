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
- The screenshots are the console's in each language when SITE_SCREENSHOTS
  names the folder tools/site_screenshots.mjs wrote them to (CI), else the
  English ones of images/screenshots/.
- For search engines and AI assistants: absolute links (SITE_URL, the site's
  address), a sitemap, structured data (JSON-LD), and each page of the
  documentation in Markdown next to it, listed in llms.txt (llms-full.txt has
  them all in one file). SITE_NOINDEX=1 keeps a pull request's preview out of
  search engines.
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
# The site's address, for the links that must be absolute (the canonical
# page, the other languages, the preview image, the sitemap, llms.txt).
SITE_URL = os.environ.get("SITE_URL", "https://jigsawfr.github.io/PlayGuard/").rstrip("/") + "/"
# A pull request's preview: not for search engines (main's site is the one).
NOINDEX = os.environ.get("SITE_NOINDEX") == "1"
# The screenshots of tools/site_screenshots.mjs: <folder>/<language>/<screen>.webp.
SCREENSHOT_DIR = os.environ.get("SITE_SCREENSHOTS", "")
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
# Open Graph's name of each language.
OG_LOCALES = {"en-US": "en_US", "fr": "fr_FR", "fr-CA": "fr_CA", "de": "de_DE", "es": "es_ES", "es-419": "es_LA",
              "it": "it_IT", "nl": "nl_NL", "pt": "pt_PT", "pt-BR": "pt_BR", "ru": "ru_RU", "ja": "ja_JP",
              "ko": "ko_KR", "zh-Hans": "zh_CN", "zh-Hant": "zh_TW"}
# The documentation, in the sidebar's order.
DOCS = [("features", "guide"), ("install", "guide"), ("compatibility", "guide"), ("troubleshooting", "guide"),
        ("companion-app", "reference"), ("config", "reference"), ("parental-controls", "reference")]
# The languages that read docs/fr/.
FRENCH = {"fr", "fr-CA"}
# The pages' Content-Security-Policy (GitHub Pages sends no headers): only the
# site's own files, no inline script, nothing from elsewhere.
CSP = ("default-src 'self'; script-src 'self'; style-src 'self'; img-src 'self' data:; media-src 'self'; "
       "font-src 'self'; connect-src 'self'; object-src 'none'; base-uri 'none'; form-action 'none'")
SCREENSHOTS = ["dashboard", "play_timer", "per_day", "activity", "restrictions", "security", "console", "clock",
               "preferences"]
FAQ = ["closed", "offline", "ban", "children", "emummc", "switch2", "free"]
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
        if path.startswith("images/screenshots/") and name[:-4] in SCREENSHOTS:
            path = screenshot(lang, name[:-4])
        if path.startswith("images/"):
            assets.add(path)
            return f"{to_root}assets/{path}"
        return f"{REPO_URL}/blob/main/{path}{anchor}"

    body = re.sub(r'(href|src)="([^"]+)"', lambda m: f'{m.group(1)}="{html.escape(target(html.unescape(m.group(2))))}"', body)
    return body


def lang_dir(lang):
    folder = next(d for code, d, _, _ in LANGUAGES if code == lang)
    return f"{folder}/" if folder else ""


def screenshot(lang, name):
    """The screenshot `name` under assets/: the console's in `lang` when
    SITE_SCREENSHOTS has it, else the English one of images/screenshots/."""
    if SCREENSHOT_DIR and os.path.exists(os.path.join(SCREENSHOT_DIR, lang, f"{name}.webp")):
        return f"images/screenshots/{lang}/{name}.webp"
    return f"images/screenshots/{name}.png"


def back_to_readme(text):
    """Without the docs' "Back to the README." (GitHub's way back, not the site's)."""
    return re.sub(r" ?(?:Back to the|Retour au) \[README\]\([^)]*\)\.", "", text)


def plain(html_text):
    """The text of some HTML, on one line."""
    return re.sub(r"\s+", " ", html.unescape(re.sub(r"<[^>]+>", "", html_text))).strip()


def doc_source(page, lang):
    """The Markdown `lang` reads for docs/<page>.md, and whether it is in that language."""
    source = f"docs/{page}.md"
    if lang in FRENCH and os.path.exists(os.path.join(ROOT, "docs", "fr", f"{page}.md")):
        source = f"docs/fr/{page}.md"
    return source, lang == "en-US" or source.startswith("docs/fr/")


def render_doc(page, lang, to_root, assets):
    """(title, html, toc, translated, source path) of docs/<page>.md for `lang`."""
    source, translated = doc_source(page, lang)
    with open(os.path.join(ROOT, source), encoding="utf-8") as f:
        text = back_to_readme(f.read())
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

def json_ld(data):
    """Structured data for search engines and AI assistants (schema.org): a
    data block, not a script, so the Content-Security-Policy lets it be."""
    text = json.dumps(data, ensure_ascii=False, indent=1).replace("</", "<\\/")
    return f'  <script type="application/ld+json">\n{text}\n  </script>\n'


def page(t, lang, to_root, path, title, body, description=None, body_class="", full_title=None, structured=(),
         markdown_url=None, indexed=True):
    """A whole page: `path` is its place under the language's folder ('' or 'docs/x/')."""
    code, folder, _, html_lang = next(l for l in LANGUAGES if l[0] == lang)
    description = description or t("meta_description")
    head = []
    if indexed:
        url = f"{SITE_URL}{lang_dir(lang)}{path}"
        head.append(f'  <link rel="canonical" href="{url}">')
        head += [f'  <link rel="alternate" hreflang="{hl}" href="{SITE_URL}{f"{d}/" if d else ""}{path}">'
                 for _, d, _, hl in LANGUAGES]
        head.append(f'  <link rel="alternate" hreflang="x-default" href="{SITE_URL}{path}">')
        head.append(f'  <meta property="og:url" content="{url}">')
    if markdown_url:
        head.append(f'  <link rel="alternate" type="text/markdown" href="{markdown_url}">')
    if NOINDEX or not indexed:
        head.append('  <meta name="robots" content="noindex">')
    options = "\n".join(
        f'        <option value="{to_root}{f"{d}/" if d else ""}{path}" data-lang="{c}" lang="{hl}"'
        f'{" selected" if c == lang else ""}>{html.escape(n)}</option>'
        for c, d, n, hl in LANGUAGES)
    home = f"{to_root}{lang_dir(lang)}"
    full_title = full_title or ("PlayGuard" if not title else f"{title} · PlayGuard")
    head_extra = "\n".join(head)
    return f"""<!doctype html>
<html lang="{html_lang}" data-lang="{code}">
<head>
  <meta charset="utf-8">
  <meta http-equiv="Content-Security-Policy" content="{CSP}">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>{html.escape(full_title)}</title>
  <meta name="description" content="{description}">
{head_extra}
  <meta property="og:type" content="website">
  <meta property="og:site_name" content="PlayGuard">
  <meta property="og:locale" content="{OG_LOCALES[code]}">
  <meta property="og:title" content="{html.escape(full_title)}">
  <meta property="og:description" content="{description}">
  <meta property="og:image" content="{SITE_URL}assets/images/social-preview.png">
  <meta property="og:image:width" content="1280">
  <meta property="og:image:height" content="640">
  <meta property="og:image:alt" content="PlayGuard">
  <meta name="twitter:card" content="summary_large_image">
  <meta name="theme-color" content="#0b1424">
  <link rel="icon" href="{to_root}assets/icon.svg" type="image/svg+xml">
  <link rel="sitemap" type="application/xml" href="{SITE_URL}sitemap.xml">
{"".join(json_ld(d) for d in structured)}  <script src="{to_root}assets/theme.js"></script>
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
      <a href="{REPO_URL}" class="nav-gh">GitHub</a>
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
        · <a href="{REPO_URL}">GitHub</a> · <a href="{REPO_URL}/blob/main/CHANGELOG.md">{t('footer_changelog')}</a></p>
      <p>{t('support')} — <a href="https://github.com/sponsors/JigSawFr">GitHub Sponsors</a> · <a href="https://ko-fi.com/jigsawfr">Ko-fi</a></p>
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
        f'      <figure><a href="{to_root}assets/{screenshot(lang, s)}"><img src="{to_root}assets/{screenshot(lang, s)}" '
        f'alt="{t(f"shot_{s}")}" width="1280" height="720" loading="lazy"></a><figcaption>{t(f"shot_{s}")}</figcaption></figure>'
        for s in SCREENSHOTS)
    faq = "\n".join(
        f'      <details><summary>{t(f"faq_{q}_q")}</summary><p>{t(f"faq_{q}_a")}</p></details>' for q in FAQ)
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
        </div>
        <p class="source"><a href="{REPO_URL}">{t('source')}</a></p>
      </div>
      <div class="hero-video">
        <video src="{to_root}assets/images/video/playguard-tour.mp4" muted loop playsinline preload="none"
               poster="{to_root}assets/{screenshot(lang, 'dashboard')}" aria-label="{t('tour_title')}"></video>
        <button class="video-toggle" type="button" hidden data-play="{t('video_play')}" data-pause="{t('video_pause')}"></button>
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

    <section id="faq" class="section">
      <h2>{t('faq_title')}</h2>
      <div class="faq">
{faq}
      </div>
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
    # The page's description: its first paragraph (the one under the title).
    first = re.search(r"<p>(.*?)</p>", body, re.S)
    description = plain(first.group(1)) if first else ""
    if len(description) > 180:
        description = description[:177].rsplit(" ", 1)[0] + "…"
    return title, translated, description, f"""    <div class="doc">
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


# ---- For search engines and AI assistants ------------------------------

def markdown_lang(page, lang):
    """The language of the Markdown `lang`'s page of docs/<page>.md is published in."""
    return "fr" if doc_source(page, lang)[0].startswith("docs/fr/") else "en-US"


def markdown_doc(page, lang, assets):
    """docs/<page>.md as published next to its page, for AI assistants: its
    links absolute, to the site's Markdown, images and pages, else to GitHub."""
    source, _ = doc_source(page, lang)
    with open(os.path.join(ROOT, source), encoding="utf-8") as f:
        text = back_to_readme(f.read())
    folder = os.path.dirname(source)

    def target(m):
        url = m.group(2)
        parts = urlsplit(url)
        if parts.scheme or url.startswith(("#", "mailto:")):
            return m.group(0)
        path = os.path.normpath(os.path.join(folder, parts.path)).replace(os.sep, "/")
        anchor = f"#{parts.fragment}" if parts.fragment else ""
        name = os.path.basename(path)
        if path.startswith("docs/") and name.endswith(".md") and name[:-3] in dict(DOCS):
            url = f"{SITE_URL}{lang_dir(lang)}docs/{name[:-3]}.md{anchor}"
        elif path in ("README.md", "README.fr.md"):
            url = f"{SITE_URL}{lang_dir(lang)}{anchor}"
        elif path.startswith("images/"):
            assets.add(path)
            url = f"{SITE_URL}assets/{path}"
        else:
            url = f"{REPO_URL}/blob/main/{path}{anchor}"
        return f"{m.group(1)}{url}{m.group(3)}"

    return re.sub(r"(\]\()([^)\s]+)(\))", target, text)


def software(t, lang, version, url):
    """PlayGuard as schema.org describes an application."""
    return {
        "@context": "https://schema.org",
        "@type": "SoftwareApplication",
        "name": "PlayGuard",
        "url": url,
        "description": t("meta_description", escape=False),
        "applicationCategory": "UtilitiesApplication",
        "applicationSubCategory": "Parental controls",
        "operatingSystem": "Nintendo Switch (Atmosphère custom firmware)",
        "softwareVersion": version.lstrip("v"),
        "downloadUrl": f"{REPO_URL}/releases/latest",
        "releaseNotes": f"{REPO_URL}/blob/main/CHANGELOG.md",
        "license": "https://www.gnu.org/licenses/gpl-3.0.html",
        "isAccessibleForFree": True,
        "offers": {"@type": "Offer", "price": "0", "priceCurrency": "USD"},
        "image": f"{SITE_URL}assets/images/social-preview.png",
        "screenshot": [f"{SITE_URL}assets/{screenshot(lang, s)}" for s in SCREENSHOTS],
        "featureList": [t(f"f_{key}_title", escape=False) for key, _ in FEATURES],
        "inLanguage": [hl for _, _, _, hl in LANGUAGES],
        "author": {"@type": "Person", "name": "JigSawFr", "url": "https://github.com/JigSawFr"},
        "sameAs": [REPO_URL],
    }


def home_data(t, lang, version):
    url = f"{SITE_URL}{lang_dir(lang)}"
    html_lang = next(l[3] for l in LANGUAGES if l[0] == lang)
    return [
        {"@context": "https://schema.org", "@type": "WebSite", "name": "PlayGuard", "url": url,
         "inLanguage": html_lang},
        software(t, lang, version, url),
        {"@context": "https://schema.org", "@type": "FAQPage", "inLanguage": html_lang,
         "mainEntity": [{"@type": "Question", "name": t(f"faq_{q}_q", escape=False),
                         "acceptedAnswer": {"@type": "Answer", "text": t(f"faq_{q}_a", escape=False)}}
                        for q in FAQ]},
    ]


def doc_data(t, lang, page_name, title, description, translated):
    home = f"{SITE_URL}{lang_dir(lang)}"
    url = f"{home}docs/{page_name}/"
    content_lang = next(l[3] for l in LANGUAGES if l[0] == (lang if translated else "en-US"))
    return [
        {"@context": "https://schema.org", "@type": "TechArticle", "headline": title, "description": description,
         "url": url, "inLanguage": content_lang, "about": {"@type": "SoftwareApplication", "name": "PlayGuard"},
         "isPartOf": {"@type": "WebSite", "name": "PlayGuard", "url": home}},
        {"@context": "https://schema.org", "@type": "BreadcrumbList", "itemListElement": [
            {"@type": "ListItem", "position": 1, "name": "PlayGuard", "item": home},
            {"@type": "ListItem", "position": 2, "name": t("docs_title", escape=False), "item": f"{home}docs/features/"},
            {"@type": "ListItem", "position": 3, "name": title, "item": url}]},
    ]


def llms_txt(t, version, descriptions, full_docs=None):
    """llms.txt (llmstxt.org): what PlayGuard is and where its documentation
    is, in Markdown, for AI assistants; with `full_docs`, llms-full.txt, every
    page of the documentation in it."""
    p = lambda key: t(key, escape=False)
    lines = [
        "# PlayGuard", "",
        f"> {p('meta_description')}", "",
        p("hero_text"), "",
        f"- {p('cfw_title')} {p('cfw_text')}",
        f"- {p('compat_text')}",
        f"- Latest release: {version} — {REPO_URL}/releases/latest (`playguard.zip`, to extract at the root of the SD card;"
        " also in the Homebrew App Store and sphaira).",
        f"- Free software under the GNU GPL v3: {REPO_URL}",
        f"- Try it in a browser, against a simulated console: {SITE_URL}simulator/",
        f"- The website in the console's 15 languages: {SITE_URL} (French documentation: {SITE_URL}fr/llms.txt)"
        if t.lang == "en-US" else f"- English: {SITE_URL}llms.txt",
        "",
        "## Documentation", "",
    ]
    lines += [f"- [{p(f'doc_{name}')}]({SITE_URL}{lang_dir(t.lang)}docs/{name}.md): {descriptions[name]}"
              for name, _ in DOCS]
    lines += ["", f"## {p('faq_title')}", ""]
    for q in FAQ:
        lines += [f"### {p(f'faq_{q}_q')}", "", p(f"faq_{q}_a"), ""]
    lines += ["## Optional", "",
              f"- [Changelog]({REPO_URL}/blob/main/CHANGELOG.md)",
              f"- [Roadmap]({REPO_URL}/blob/main/ROADMAP.md)",
              f"- [Recovery sysmodule]({REPO_URL}/blob/main/sysmodule/README.md)",
              f"- [Contributing]({REPO_URL}/blob/main/CONTRIBUTING.md)"]
    if not full_docs:
        lines += [f"- [Every page of the documentation in one file]({SITE_URL}{lang_dir(t.lang)}llms-full.txt)"]
        return "\n".join(lines) + "\n"
    return "\n".join(lines) + "\n\n" + "\n\n".join(full_docs) + "\n"


def sitemap(paths):
    """Every page in every language, each with its other languages."""
    entries = []
    for path in paths:
        alternates = "".join(
            f'\n    <xhtml:link rel="alternate" hreflang="{hl}" href="{SITE_URL}{f"{d}/" if d else ""}{path}"/>'
            for _, d, _, hl in LANGUAGES)
        alternates += f'\n    <xhtml:link rel="alternate" hreflang="x-default" href="{SITE_URL}{path}"/>'
        for _, d, _, _ in LANGUAGES:
            entries.append(f'  <url>\n    <loc>{SITE_URL}{f"{d}/" if d else ""}{path}</loc>{alternates}\n  </url>')
    return ('<?xml version="1.0" encoding="UTF-8"?>\n'
            '<urlset xmlns="http://www.sitemaps.org/schemas/sitemap/0.9" xmlns:xhtml="http://www.w3.org/1999/xhtml">\n'
            + "\n".join(entries) + "\n</urlset>\n")


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
    pages = 0
    for lang, folder, _, _ in LANGUAGES:
        t = Texts(lang, version)
        base = f"{folder}/" if folder else ""
        to_root = "../" if folder else ""
        assets.update(screenshot(lang, s) for s in SCREENSHOTS)
        write(out, f"{base}index.html", page(t, lang, to_root, "", "", home_body(t, lang, to_root), body_class="home",
                                             full_title=f"PlayGuard — {t('hero_title', escape=False)}",
                                             structured=home_data(t, lang, version)))
        descriptions, markdown = {}, []
        for name, _ in DOCS:
            doc_root = to_root + "../../"
            title, translated, description, body = doc_body(t, lang, doc_root, name, assets)
            descriptions[name] = description
            md_lang = markdown_lang(name, lang)
            # A page not translated yet is named in the reader's language (its sidebar's name).
            shown = title if translated else t(f"doc_{name}", escape=False)
            write(out, f"{base}docs/{name}/index.html",
                  page(t, lang, doc_root, f"docs/{name}/", shown, body, description=html.escape(description),
                       structured=doc_data(t, lang, name, shown, description, translated),
                       markdown_url=f"{SITE_URL}{lang_dir(md_lang)}docs/{name}.md"))
            # The Markdown of each page, in the languages it is written in.
            if lang in ("en-US", "fr"):
                text = markdown_doc(name, lang, assets)
                write(out, f"{base}docs/{name}.md", text)
                markdown.append(text)
            pages += 1
        if lang in ("en-US", "fr"):
            write(out, f"{base}llms.txt", llms_txt(t, version, descriptions))
            write(out, f"{base}llms-full.txt", llms_txt(t, version, descriptions, markdown))
        pages += 1
    write(out, "sitemap.xml", sitemap([""] + [f"docs/{name}/" for name, _ in DOCS]))
    # GitHub Pages serves 404.html at any path: its links start from the site's root.
    t = Texts("en-US", version)
    base_url = os.environ.get("SITE_BASE", "/PlayGuard/")
    write(out, "404.html", page(t, "en-US", base_url, "", t("not_found_title"),
                                f'    <section class="section narrow"><h1>{t("not_found_title")}</h1><p>{t("not_found_text")}</p>'
                                f'<p><a class="button primary" href="{base_url}">{t("home")}</a></p></section>',
                                indexed=False))
    # The site's own files, then what the pages use from the repository and
    # the screenshots of tools/site_screenshots.mjs.
    os.makedirs(os.path.join(out, "assets"), exist_ok=True)
    shutil.copytree(os.path.join(ROOT, "site", "assets"), os.path.join(out, "assets"), dirs_exist_ok=True)
    for name in ("icon.svg", "mark.svg"):
        shutil.copy(os.path.join(ROOT, "branding", name), os.path.join(out, "assets", name))
    for path in sorted(assets):
        os.makedirs(os.path.dirname(os.path.join(out, "assets", path)), exist_ok=True)
        if path.endswith(".webp"):
            source = os.path.join(SCREENSHOT_DIR, *path.split("/")[2:])
        else:
            source = os.path.join(ROOT, path)
        shutil.copy(source, os.path.join(out, "assets", path))
    open(os.path.join(out, ".nojekyll"), "w").close()
    print(f"site: {pages} pages in {len(LANGUAGES)} languages, {len(assets)} images and videos, in {out}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
