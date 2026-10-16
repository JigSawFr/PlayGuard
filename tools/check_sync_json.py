#!/usr/bin/env python3
"""Checks the JSON documents the remote link's host tests wrote (make check):
each one parses, the discovery is the device-based form Home Assistant takes
(every component with a platform, a unique id, a state topic and template
or a command topic), and every value template only reads keys the document
of its state topic has (the state, system or games document), so a Home
Assistant template never fails on a missing key."""
import json
import re
import sys
from pathlib import Path


def fail(msg):
    print(f"check_sync_json: {msg}")
    sys.exit(1)


def keys_of(doc, prefix=""):
    """Every dotted path in the state document ("timer.limits.mon")."""
    out = set()
    if isinstance(doc, dict):
        for k, v in doc.items():
            path = f"{prefix}.{k}" if prefix else k
            out.add(path)
            out |= keys_of(v, path)
    return out


def main():
    folder = Path(sys.argv[1] if len(sys.argv) > 1 else "build/host-tests/sync")
    docs = {}
    for name in ("discovery.json", "state.json", "state_empty.json", "activity.json", "system.json",
                 "system_empty.json", "games.json"):
        path = folder / name
        if not path.exists():
            fail(f"{path} is missing: run make test first")
        try:
            docs[name] = json.loads(path.read_text(encoding="utf-8"))
        except json.JSONDecodeError as e:
            fail(f"{name}: {e}")

    disc = docs["discovery.json"]
    for key in ("dev", "o", "avty_t", "cmps"):
        if key not in disc:
            fail(f"discovery: no {key}")
    state_keys = keys_of(docs["state.json"])
    if keys_of(docs["state_empty.json"]) != state_keys:
        fail("the state read with nothing available has other keys than a full one")
    system_keys = keys_of(docs["system.json"])
    if keys_of(docs["system_empty.json"]) != system_keys:
        fail("the system document read with nothing available has other keys than a full one")
    # The keys of the document each state topic carries.
    topic_keys = {"state": state_keys, "system": system_keys, "games": keys_of(docs["games.json"])}
    uniq = set()
    for oid, c in disc["cmps"].items():
        if not re.fullmatch(r"[a-z0-9_]+", oid):
            fail(f"discovery: bad object id {oid!r}")
        if set(c) == {"p"}:
            continue   # not offered now: its platform alone, which removes it
        for key in ("p", "name", "uniq_id"):
            if key not in c:
                fail(f"discovery: {oid} has no {key}")
        if c["uniq_id"] in uniq:
            fail(f"discovery: {oid} repeats a unique id")
        uniq.add(c["uniq_id"])
        if c["p"] in ("button",):
            if "cmd_t" not in c:
                fail(f"discovery: button {oid} has no command topic")
            continue
        if "stat_t" not in c:
            fail(f"discovery: {oid} has no state topic")
        tpl = c.get("val_tpl")
        if tpl is None:
            continue
        topic = c["stat_t"].rsplit("/", 1)[-1]
        if topic not in topic_keys:
            fail(f"discovery: {oid} reads {c['stat_t']}, which carries no document")
        for path in re.findall(r"value_json\.([a-z0-9_.]+)", tpl):
            if path not in topic_keys[topic]:
                fail(f"discovery: {oid} reads value_json.{path}, which the {topic} document does not have")
    # entities.json (the Home Assistant integration's copy of the table): every
    # entity, and nothing else, of the discovery with every option.
    path = folder / "entities.json"
    if not path.exists():
        fail(f"{path} is missing: run make test first")
    try:
        table = json.loads(path.read_text(encoding="utf-8"))["entities"]
    except (json.JSONDecodeError, KeyError) as e:
        fail(f"entities.json: {e}")
    ids = [e["id"] for e in table]
    if len(set(ids)) != len(ids):
        fail("entities.json repeats an id")
    for e in table:
        if e["write"] not in ("none", "free", "timer") or not isinstance(e["extra"], dict) \
                or e.get("topic") not in topic_keys:
            fail(f"entities.json: {e['id']} is malformed")
        for p in re.findall(r"value_json\.([a-z0-9_.]+)", e["value_template"] or ""):
            if p not in topic_keys[e["topic"]]:
                fail(f"entities.json: {e['id']} reads value_json.{p}, which the {e['topic']} document does not have")
    missing = set(disc["cmps"]) - set(ids)
    if missing:
        fail(f"entities.json lacks {sorted(missing)}")
    print(f"check_sync_json: {len(disc['cmps'])} components, {len(state_keys)} state keys, "
          f"{len(system_keys)} system keys, {len(table)} entities: OK")


if __name__ == "__main__":
    main()
