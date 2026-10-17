#!/usr/bin/env python3
"""Static checks on the app resources (run in CI and by `make check`).

- every i18n JSON file parses, and every language (resources/i18n/<code>/
  playguard.json) defines exactly the keys of en-US, with as many "{}";
- config::LANGUAGES (source/util/config.hpp) lists exactly those languages,
  and each has a name under playguard/tools/languages/;
- every "playguard/..." key referenced from C++ or XML exists in en-US
  (keys built at runtime are checked as prefixes: "playguard/days/{}" etc.);
- every en-US key is referenced (exactly, or under such a prefix): no dead
  strings for translators to keep up to date;
- every XML layout is well formed (borealis' "brls:" prefix is not declared,
  so a non-namespace-aware parser is used);
- brls:Label never carries padding attributes (borealis throws at runtime);
- every BRLS_BIND(type, name, "id") names an id of the XML its class
  inflates (CONTENT_FROM_XML_RES, TabBase("xml/..."), inflateFromXMLRes), or
  of a custom view that XML holds (<PtStateHeader/>): a missing one is a null
  view at runtime, the first use of which crashes.
- every sidebar tab name (playguard/tabs/) fits on one line: at most
  TAB_WIDTH columns, a wide (CJK) character counting as two.

It also warns, without failing:
- about sentence-like values a catalog still has in English (English is a
  fine placeholder for a new string);
- about an XML layout of resources/xml no class inflates, and an image of
  resources/img nothing names (the emoji of img/emoji/ aside: a palette for
  the release notes, kept whole on purpose);
- about an emoji a heading or a bullet of CHANGELOG.md or
  .github/release-notes/ opens on that has no img/emoji/<hex>.png (it shows
  no icon on the console);
- about a key docs/config.md documents that source/util/config.cpp neither
  reads nor writes, and one config.cpp reads or writes that the page leaves
  out.
"""
import glob
import json
import os
import re
import sys
import unicodedata
import xml.parsers.expat

ROOT = sys.argv[1] if len(sys.argv) > 1 else "."
errors = []
warnings = []
# Names that read the same in every language.
SAME_IN_EVERY_LANGUAGE = {"about/made_in_france", "tools/update_via_values/appstore", "update/appstore",
                          "tools/storage_emummc", "tools/storage_sysmmc", "init_error/firmware"}


def flatten(d, prefix=""):
    out = {}
    for k, v in d.items():
        if isinstance(v, dict):
            out.update(flatten(v, prefix + k + "/"))
        else:
            out[prefix + k] = v
    return out


BASE = "en-US"
catalogs = {}
for path in sorted(glob.glob(f"{ROOT}/resources/i18n/*/playguard.json")):
    lang = path.split("/")[-2]
    try:
        catalogs[lang] = flatten(json.load(open(path, encoding="utf-8")))
    except Exception as e:  # noqa: BLE001
        errors.append(f"{path}: {e}")
for path in glob.glob(f"{ROOT}/resources/i18n/*/*.json"):
    if not path.endswith("/playguard.json"):
        try:
            json.load(open(path, encoding="utf-8"))
        except Exception as e:  # noqa: BLE001
            errors.append(f"{path}: {e}")
if BASE not in catalogs:
    errors.append(f"resources/i18n/{BASE}/playguard.json is missing")

base = catalogs.get(BASE, {})
for lang, cat in sorted(catalogs.items()):
    if lang == BASE:
        continue
    for k in sorted(set(base) - set(cat)):
        errors.append(f"{lang} is missing {k}")
    for k in sorted(set(cat) - set(base)):
        errors.append(f"{lang} has extra key {k}")
    # Still in English: a value identical to en-US with three or more words is
    # most likely a placeholder nobody translated (names and brands aside).
    for k in sorted(set(base) & set(cat)):
        v = str(cat[k])
        if v == str(base[k]) and k not in SAME_IN_EVERY_LANGUAGE and not k.startswith("tools/languages/") \
                and len(re.findall(r"[A-Za-z]{2,}", v)) >= 3:
            warnings.append(f"{lang}: {k} is still in English")
    # The same number of "{}" placeholders as en-US.
    for k in sorted(set(base) & set(cat)):
        if str(base[k]).count("{}") != str(cat[k]).count("{}"):
            errors.append(f"{lang}: placeholder count differs for {k}")

# The sidebar's tab names stay on one line in every language.
TAB_WIDTH = 16
for lang, cat in sorted(catalogs.items()):
    for k, v in cat.items():
        if k.startswith("tabs/"):
            width = sum(2 if unicodedata.east_asian_width(c) in "WF" else 1 for c in v)
            if width > TAB_WIDTH:
                errors.append(f"{lang}: tab name {k} \"{v}\" is {width} columns wide, more than {TAB_WIDTH}")

# The language picker's list (config::LANGUAGES) and the catalogs agree.
config_hpp = open(f"{ROOT}/source/util/config.hpp", encoding="utf-8").read()
m = re.search(r"LANGUAGES\[\]\s*=\s*\{([^}]*)\}", config_hpp)
if not m:
    errors.append("source/util/config.hpp: config::LANGUAGES not found")
else:
    listed = re.findall(r'"([^"]+)"', m.group(1))
    if not listed or listed[0] != "system":
        errors.append('config::LANGUAGES must start with "system"')
    for lang in sorted(set(listed) - {"system"} - set(catalogs)):
        errors.append(f"config::LANGUAGES lists {lang}, which has no resources/i18n/{lang}/playguard.json")
    for lang in sorted(set(catalogs) - set(listed)):
        errors.append(f"resources/i18n/{lang} is not in config::LANGUAGES (the language picker)")
    for lang in listed:
        if f"tools/languages/{lang}" not in base:
            errors.append(f"no language name playguard/tools/languages/{lang}")

en_keys = set(base)
prefixes = {k.rsplit("/", 1)[0] + "/" for k in en_keys}


used_exact = set()
used_prefixes = set()


def check_key(key, where):
    if "{" in key or key.endswith("/"):
        prefix = key.split("{")[0]
        used_prefixes.add(prefix.replace("playguard/", "", 1))
        if not any(p.startswith(prefix.replace("playguard/", "", 1)) for p in prefixes):
            errors.append(f"{where}: no key under prefix {key}")
        return
    used_exact.add(key.replace("playguard/", "", 1))
    if key.replace("playguard/", "", 1) not in en_keys:
        errors.append(f"{where}: unknown i18n key {key}")


for path in glob.glob(f"{ROOT}/source/**/*.[ch]pp", recursive=True) + glob.glob(f"{ROOT}/source/**/*.[ch]", recursive=True):
    text = open(path, encoding="utf-8").read()
    for m in re.finditer(r'"(playguard/[A-Za-z0-9_/{}.-]*)"', text):
        check_key(m.group(1), path)

for path in sorted(glob.glob(f"{ROOT}/resources/xml/**/*.xml", recursive=True)):
    data = open(path, "rb").read()
    parser = xml.parsers.expat.ParserCreate()

    def start(name, attrs, path=path):
        if name == "brls:Label" and any(a.startswith("padding") for a in attrs):
            errors.append(f"{path}: brls:Label does not support padding attributes")
        for value in attrs.values():
            if value.startswith("@i18n/playguard/"):
                check_key(value[len("@i18n/"):], path)

    parser.StartElementHandler = start
    try:
        parser.Parse(data, True)
    except xml.parsers.expat.ExpatError as e:
        errors.append(f"{path}: {e}")

for k in sorted(en_keys):
    if k not in used_exact and not any(k.startswith(p) for p in used_prefixes):
        errors.append(f"unused i18n key playguard/{k} (remove it from every language)")


def read(path):
    try:
        return open(path, encoding="utf-8").read()
    except OSError:
        return ""


# BRLS_BIND ids against the XML each class inflates. A class's XML is named in
# its header (an activity's CONTENT_FROM_XML_RES("activity/x.xml"), under
# resources/xml/) or in its .cpp (a tab's TabBase("xml/tab/x.xml"), a view's
# inflateFromXMLRes("xml/view/x.xml"), under resources/). One class per header.
xml_dir = f"{ROOT}/resources/xml"
code_files = glob.glob(f"{ROOT}/source/**/*.[ch]pp", recursive=True)
class_xml = {}      # class name -> XML path, relative to resources/xml
class_binds = {}    # class name -> (header, [ids])
loaded_xml = set()
for path in sorted(glob.glob(f"{ROOT}/source/**/*.hpp", recursive=True)):
    text = read(path)
    own = text + read(path[:-len(".hpp")] + ".cpp")
    names = re.findall(r"^\s*class\s+(\w+)\b[^;]*$", text, re.M)
    res = re.findall(r'CONTENT_FROM_XML_RES\(\s*"([^"]+)"', own) + \
        [p[len("xml/"):] for p in re.findall(r'(?:TabBase|inflateFromXMLRes)\(\s*"(xml/[^"]+)"', own)]
    binds = re.findall(r'BRLS_BIND\([^;]*?,\s*\w+\s*,\s*"([^"]+)"\s*\)', text)
    if names and res:
        class_xml[names[0]] = res[0]
    if names and binds:
        class_binds[names[0]] = (path, binds)
for path in code_files:
    text = read(path)
    loaded_xml.update(re.findall(r'CONTENT_FROM_XML_RES\(\s*"([^"]+)"', text))
    loaded_xml.update(re.findall(r'"xml/([^"]+\.xml)"', text))   # TabBase, inflateFromXMLRes

xml_ids = {}    # XML path -> (ids, element names)
for path in glob.glob(f"{xml_dir}/**/*.xml", recursive=True):
    ids, tags = set(), set()
    parser = xml.parsers.expat.ParserCreate()

    def start(name, attrs, ids=ids, tags=tags):
        tags.add(name)
        if "id" in attrs:
            ids.add(attrs["id"])

    parser.StartElementHandler = start
    try:
        parser.Parse(open(path, "rb").read(), True)
    except xml.parsers.expat.ExpatError:
        pass   # reported above
    xml_ids[path[len(xml_dir) + 1:]] = (ids, tags)


def ids_of(res, seen):
    """The ids of `res`, with those of the custom views it holds."""
    if res in seen or res not in xml_ids:
        return set()
    seen.add(res)
    ids, tags = xml_ids[res]
    out = set(ids)
    for tag in tags:
        if tag in class_xml:
            out |= ids_of(class_xml[tag], seen)
    return out


for name, (path, binds) in sorted(class_binds.items()):
    res = class_xml.get(name)
    if res is None:
        warnings.append(f"{path}: {name} binds views but no XML it inflates was found")
        continue
    if res not in xml_ids:
        errors.append(f"{path}: {name} inflates {res}, which is not in resources/xml")
        continue
    ids = ids_of(res, set())
    for i in binds:
        if i not in ids:
            errors.append(f"{path}: BRLS_BIND id \"{i}\" is not in resources/xml/{res}")

# XML layouts nothing loads.
for res in sorted(set(xml_ids) - loaded_xml):
    warnings.append(f"resources/xml/{res} is never loaded (no CONTENT_FROM_XML_RES, TabBase or inflateFromXMLRes)")

# Images nothing names ("img/x.png" in the code, "@res/img/x.png" in a layout).
# The emoji are a palette for the release notes (img/emoji/README.md): an
# unused one is fine; an emoji the notes open on without its file is not.
named = "\n".join(read(p) for p in code_files + glob.glob(f"{xml_dir}/**/*.xml", recursive=True))
for path in sorted(glob.glob(f"{ROOT}/resources/img/**/*", recursive=True)):
    rel = path[len(f"{ROOT}/resources/"):]
    if os.path.isdir(path) or rel.startswith("img/emoji/"):
        continue
    if rel not in named:
        warnings.append(f"resources/{rel} is never used (no \"{rel}\" in the code or a layout)")


def is_emoji(cp):
    """The pictographs source/util/changelog.cpp draws as images."""
    return cp >= 0x1F000 or 0x2600 <= cp <= 0x27BF or 0x2B00 <= cp <= 0x2BFF or \
        0x231A <= cp <= 0x231B or 0x23E9 <= cp <= 0x23FA


notes = [f"{ROOT}/CHANGELOG.md"] + sorted(p for p in glob.glob(f"{ROOT}/.github/release-notes/*.md")
                                         if not p.endswith("/README.md"))   # how to write them
missing_emoji = {}
for path in notes:
    for line in read(path).splitlines():
        m = re.match(r"\s*(?:#+|[*+-]|\d+\.)\s+(.*)", line)
        if not m:
            continue
        lead = m.group(1).lstrip(" *")
        if lead and is_emoji(ord(lead[0])) and not 0xFE00 <= ord(lead[0]) <= 0xFE0F:
            hexa = f"{ord(lead[0]):x}"
            if not os.path.exists(f"{ROOT}/resources/img/emoji/{hexa}.png"):
                missing_emoji.setdefault(hexa, (lead[0], os.path.relpath(path, ROOT)))
for hexa, (ch, where) in sorted(missing_emoji.items()):
    warnings.append(f"{where}: {ch} has no resources/img/emoji/{hexa}.png (see its README)")

# docs/config.md and the keys config.cpp reads and writes. The page's keys
# are the first cell of its tables and its "## `key`" headings, up to the
# watch.json section (another file).
config_cpp = read(f"{ROOT}/source/util/config.cpp")
config_md = read(f"{ROOT}/docs/config.md").split("## `watch.json`")[0]
code_read = set(re.findall(r'read_\w+\(\s*j\s*,\s*"(\w+)"', config_cpp)) | \
    set(re.findall(r'\bj\.find\(\s*"(\w+)"', config_cpp))
code_written = set(re.findall(r'\b(?:j|st)\["(\w+)"\]', config_cpp))
documented = set(re.findall(r"^\|\s*`(\w+)`\s*\|", config_md, re.M)) | \
    set(re.findall(r"^#+\s*`(\w+)`\s*$", config_md, re.M))
if not config_cpp or not documented:
    errors.append("source/util/config.cpp or docs/config.md not found, or no key found in it")
for k in sorted(documented - code_read - code_written):
    warnings.append(f"docs/config.md documents `{k}`, which source/util/config.cpp neither reads nor writes")
for k in sorted((code_read | code_written) - documented):
    warnings.append(f"source/util/config.cpp keeps `{k}`, which docs/config.md does not document")
for k in sorted(code_written - code_read - {"schema"}):
    warnings.append(f"source/util/config.cpp writes `{k}` but never reads it back")
for k in sorted(code_read - code_written):
    warnings.append(f"source/util/config.cpp reads `{k}` but never writes it (lost at the next save)")

for w in warnings:
    print("WARNING:", w)
for e in errors:
    print("ERROR:", e)
print(f"resources check: {len(errors)} error(s)")
sys.exit(1 if errors else 0)
