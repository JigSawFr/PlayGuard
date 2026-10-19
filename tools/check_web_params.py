#!/usr/bin/env python3
"""Checks web/params.json, the web simulator's settings panel, and its texts
in web/i18n/ against the desktop simulator and the app (web/README.md).

Errors:
- a PLAYGUARD_SIM_* variable read anywhere in source/ without its entry in
  params.json (its name in lower case without the prefix), or an entry no
  variable matches any more;
- an entry of an unknown group or type, a select or a list without options,
  a preset naming an unknown entry;
- a language of the console (config::LANGUAGES, but "system") without its
  catalog web/i18n/<language>.json, or not in the page's `lang` entry;
- a text English (web/i18n/en-US.json) lacks: a group, a panel entry's label,
  an option of a select or a list, a preset; or a text in any catalog that
  English does not have (a typo, or a leftover).
Warnings: a text another catalog has not translated yet (the page shows the
English one).

Usage: tools/check_web_params.py [repo-root]
"""
import json
import os
import re
import sys

TYPES = {"bool", "number", "text", "select", "multi", "datetime"}
I18N_PARTS = ("ui", "groups", "params", "presets")


def flat(d, prefix=""):
    """{"params": {"fw": {"label": ...}}} -> {"params/fw/label": ...}"""
    out = {}
    for k, v in d.items():
        if isinstance(v, dict):
            out.update(flat(v, f"{prefix}{k}/"))
        else:
            out[f"{prefix}{k}"] = v
    return out


def main():
    root = sys.argv[1] if len(sys.argv) > 1 else "."
    errors, warnings = [], []

    read = set()
    for folder, _, files in os.walk(os.path.join(root, "source")):
        for name in files:
            if name.endswith((".c", ".cpp", ".h", ".hpp")):
                with open(os.path.join(folder, name), encoding="utf-8", errors="replace") as f:
                    read |= set(re.findall(r"PLAYGUARD_SIM_([A-Z0-9_]+)", f.read()))

    with open(os.path.join(root, "web", "params.json"), encoding="utf-8") as f:
        manifest = json.load(f)
    groups = [g["id"] for g in manifest["groups"]]

    names, variables = set(), set()
    needed = {f"groups/{g}" for g in groups}   # the texts English must have
    for p in manifest["params"]:
        name = p.get("name", "")
        where = f"params.json: {name or '(no name)'}"
        if not re.fullmatch(r"[a-z0-9_]+", name):
            errors.append(f"{where}: a name in lower case, digits and _")
        if name in names:
            errors.append(f"{where}: twice")
        names.add(name)
        if not p.get("config"):
            variables.add(name.upper())
        if p.get("group") not in groups:
            errors.append(f"{where}: no group {p.get('group')!r}")
        if p.get("type") not in TYPES:
            errors.append(f"{where}: type {p.get('type')!r} is not one of {sorted(TYPES)}")
        if p.get("type") in ("select", "multi"):
            options = p.get("options") or []
            if not options or not all(isinstance(o, str) for o in options):
                errors.append(f"{where}: its options, a list of values")
        if p.get("panel") is False:
            continue
        needed.add(f"params/{name}/label")
        for o in p.get("options") or []:
            needed.add(f"params/{name}/options/{o}")

    for v in sorted(read - variables):
        errors.append(f"PLAYGUARD_SIM_{v} is read in source/ but has no entry in web/params.json ({v.lower()})")
    for v in sorted(variables - read):
        errors.append(f"web/params.json: {v.lower()} is no PLAYGUARD_SIM_* variable read in source/ any more")

    for preset in manifest.get("presets", []):
        needed.add(f"presets/{preset.get('id')}")
        for name in preset.get("params", {}):
            if name not in names:
                errors.append(f"preset {preset.get('id')}: no entry {name!r}")

    # The console's languages: one catalog each, all in the page's `lang` entry.
    with open(os.path.join(root, "source", "util", "config.hpp"), encoding="utf-8") as f:
        m = re.search(r"LANGUAGES\[\]\s*=\s*\{([^}]*)\}", f.read())
    languages = [l for l in re.findall(r'"([^"]+)"', m.group(1)) if l != "system"] if m else []
    if not languages:
        errors.append("source/util/config.hpp: no LANGUAGES[] found")
    lang = next((p for p in manifest["params"] if p.get("name") == "lang"), {})
    for l in languages:
        if l not in (lang.get("options") or []):
            errors.append(f"params.json: lang does not offer {l}, a language of the console")
        if not os.path.exists(os.path.join(root, "web", "i18n", f"{l}.json")):
            errors.append(f"web/i18n/{l}.json is missing ({l} is a language of the console)")

    catalogs = {}
    folder = os.path.join(root, "web", "i18n")
    for name in sorted(os.listdir(folder)):
        if name.endswith(".json"):
            with open(os.path.join(folder, name), encoding="utf-8") as f:
                catalogs[name[:-5]] = json.load(f)
    english = flat(catalogs.get("en-US", {}))
    for key in sorted(needed - english.keys()):
        errors.append(f"web/i18n/en-US.json: no text for {key}")
    for code, catalog in catalogs.items():
        for part in catalog:
            if part not in I18N_PARTS:
                errors.append(f"web/i18n/{code}.json: {part!r} is not one of {I18N_PARTS}")
        texts = flat(catalog)
        for key in sorted(texts.keys() - english.keys()):
            errors.append(f"web/i18n/{code}.json: {key} is not in English (a typo, or a leftover)")
        if code != "en-US":
            missing = sorted(english.keys() - texts.keys())
            if missing:
                warnings.append(f"web/i18n/{code}.json: {len(missing)} text(s) in English for now, e.g. {missing[0]}")

    for w in warnings:
        print(f"warning: {w}")
    for e in errors:
        print(f"error: {e}")
    if errors:
        print(f"{len(errors)} error(s)")
        return 1
    print(f"{len(variables)} variables, {len(names) - len(variables)} settings, "
          f"{len(manifest.get('presets', []))} presets, {len(catalogs)} languages")
    return 0


if __name__ == "__main__":
    sys.exit(main())
