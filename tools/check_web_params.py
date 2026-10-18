#!/usr/bin/env python3
"""Checks web/params.json, the web simulator's settings panel, against the
desktop simulator (web/README.md).

Every PLAYGUARD_SIM_* variable read anywhere in source/ must have its entry
(its name in lower case without the prefix), and every entry must still be
read: a variable added to the simulator without its line in the panel, or
one removed from it, fails. Also checks the file itself: each entry's group
exists, a select or a list has options, the labels have an English and a
French text, the presets name only known entries.

Usage: tools/check_web_params.py [repo-root]
"""
import json
import os
import re
import sys

TYPES = {"bool", "number", "text", "select", "multi", "datetime"}


def labelled(value):
    return isinstance(value, str) or (isinstance(value, dict) and value.get("en") and value.get("fr"))


def main():
    root = sys.argv[1] if len(sys.argv) > 1 else "."
    errors = []

    read = set()
    for folder, _, files in os.walk(os.path.join(root, "source")):
        for name in files:
            if name.endswith((".c", ".cpp", ".h", ".hpp")):
                with open(os.path.join(folder, name), encoding="utf-8", errors="replace") as f:
                    read |= set(re.findall(r"PLAYGUARD_SIM_([A-Z0-9_]+)", f.read()))

    with open(os.path.join(root, "web", "params.json"), encoding="utf-8") as f:
        manifest = json.load(f)
    groups = {g["id"] for g in manifest["groups"]}
    for g in manifest["groups"]:
        if not labelled(g.get("label")):
            errors.append(f"group {g['id']}: a label in English and French")

    names = set()
    variables = set()
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
        if not labelled(p.get("label")):
            errors.append(f"{where}: a label in English and French")
        if "help" in p and not labelled(p["help"]):
            errors.append(f"{where}: its help in English and French")
        if p.get("type") in ("select", "multi"):
            options = p.get("options") or []
            if not options:
                errors.append(f"{where}: no options")
            for o in options:
                if "value" not in o or not labelled(o.get("label")):
                    errors.append(f"{where}: an option without a value or a label")

    for v in sorted(read - variables):
        errors.append(f"PLAYGUARD_SIM_{v} is read in source/ but has no entry in web/params.json ({v.lower()})")
    for v in sorted(variables - read):
        errors.append(f"web/params.json: {v.lower()} is no PLAYGUARD_SIM_* variable read in source/ any more")

    for preset in manifest.get("presets", []):
        if not labelled(preset.get("label")):
            errors.append(f"preset {preset.get('label')}: a label in English and French")
        for name in preset.get("params", {}):
            if name not in names:
                errors.append(f"preset {preset['label'].get('en')}: no entry {name!r}")

    for e in errors:
        print(f"error: {e}")
    if errors:
        print(f"{len(errors)} error(s)")
        return 1
    print(f"{len(variables)} variables, {len(names) - len(variables)} settings, {len(manifest.get('presets', []))} presets")
    return 0


if __name__ == "__main__":
    sys.exit(main())
