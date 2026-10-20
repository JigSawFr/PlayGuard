#!/usr/bin/env python3
"""Writes the Homebrew App Store (hb-appstore, 4TU) listing of a release, and checks it.

packaging/hb-appstore/pkgbuild.json is the listing's source: a pkgbuild as
switch-hbas-repo takes it, with these filled in so it never disagrees with
the build:
  {version}            CMakeLists.txt  VERSION_MAJOR / MINOR / ALTER
  {fw_min_play_timer}  source/core/sysinfo.h  PCTL_FW_MIN_PLAYTIMER
  {fw_tested_max}      source/core/sysinfo.h  PCTL_FW_TESTED_MAX
  {changelog}          .github/release-notes/<version>.md, else the version's
                       section of CHANGELOG.md, then a link to the release
and two keys of its own, taken out of what it writes: an asset's "from" (the
image in this repository it is copied from) and a zip's "leaves_out" (files
of playguard.zip the store does not install). "details" may be a list of
paragraphs. The store shows "\\n" (a backslash and an n) as a line break, as
every pkgbuild of switch-hbas-repo writes it.

Usage: tools/hbas_package.py [--root .] [--zip playguard.zip] [--spinarak spinarak.py] <out-dir>

Writes <out-dir>/PlayGuard/: pkgbuild.json and its images, the folder a
submission adds to switch-hbas-repo's packages/. Then checks
  - the images' sizes (icon 256 x 150, banner 848 x 208, screenshots 1280 x 720,
    6 at most: no listing of the store has more);
  - with --zip, the release zip: each path of the listing finds files in it,
    the binary is one of them, and every file it holds is installed or left
    out on purpose, so a file added to the zip cannot miss the store unseen;
  - with --spinarak, the listing built by the store's own tool (spinarak, the
    script switch-hbas-repo's CI runs), from the local zip: the package must
    build, with a manifest that installs the binary. An empty manifest is
    what failed the first submission.
"""
import argparse
import fnmatch
import json
import os
import re
import shutil
import struct
import subprocess
import sys
import tempfile
import unicodedata
import zipfile

PACKAGE = "PlayGuard"
SIZES = {"icon": (256, 150), "banner": (848, 208), "screenshot": (1280, 720)}
MAX_SCREENSHOTS = 6
# switch-hbas-repo's categories (the ones its listings use).
CATEGORIES = {"tool", "game", "emu", "advanced", "theme", "concept", "_misc"}
MAX_CHANGELOG = 1500


def fail(msg):
    print("hbas_package:", msg)
    sys.exit(1)


def read(path):
    with open(path, encoding="utf-8") as f:
        return f.read()


def version(root):
    cmake = read(f"{root}/CMakeLists.txt")
    parts = []
    for name in ("MAJOR", "MINOR", "ALTER"):
        m = re.search(rf'set\(VERSION_{name} "(\d+)"\)', cmake)
        if not m:
            fail(f"VERSION_{name} not found in CMakeLists.txt")
        parts.append(m.group(1))
    return ".".join(parts)


def firmware(root, macro):
    m = re.search(rf"#define {macro}\s+MAKEHOSVERSION\((\d+),\s*(\d+),\s*(\d+)\)",
                  read(f"{root}/source/core/sysinfo.h"))
    if not m:
        fail(f"{macro} not found in source/core/sysinfo.h")
    return ".".join(m.groups())


def plain(text):
    """Markdown and emoji out of a line of release notes: the store draws neither."""
    text = re.sub(r"\[([^\]]*)\]\([^)]*\)", r"\1", text)   # [text](link)
    text = re.sub(r"\*\*([^*]*)\*\*", r"\1", text)
    text = re.sub(r"`([^`]*)`", r"\1", text)
    text = "".join(c for c in text if unicodedata.category(c) != "So" and c not in "️‍")
    return re.sub(r"\s+", " ", text).strip()


def changelog(root, v):
    lines = []
    notes = f"{root}/.github/release-notes/{v}.md"
    if os.path.exists(notes):
        for line in read(notes).splitlines():
            if not line.strip() or line.startswith("#"):
                continue
            bullet = re.match(r"\s*[*-]\s+(.*)", line)
            lines.append("• " + plain(bullet.group(1)) if bullet else plain(line))
    else:
        # No summary written for it: CHANGELOG.md's lists are commit titles,
        # contributors' ones among them, so only how many, and the link.
        m = re.search(rf"^## \[?{re.escape(v)}\]?.*?$(.*?)(?=^## |\Z)", read(f"{root}/CHANGELOG.md"), re.M | re.S)
        counts = {}
        for section in re.split(r"^### ", m.group(1) if m else "", flags=re.M)[1:]:
            name = plain(section.splitlines()[0]).lower()
            counts[name] = counts.get(name, 0) + len(re.findall(r"^\* ", section, re.M))
        said = [f"{n} {name}" for name, n in counts.items() if n and name in ("features", "bug fixes", "polish")]
        if said:
            lines.append(", ".join(said) + ".")
    link = f"Full notes: https://github.com/JigSawFr/PlayGuard/releases/tag/v{v}"
    kept, size = [], len(link)
    for i, line in enumerate(lines):
        if size + len(line) > MAX_CHANGELOG:
            kept.append(f"…and {len(lines) - i} more.")
            break
        kept.append(line)
        size += len(line) + 1
    return "\n".join([f"PlayGuard {v}"] + kept + ["", link])


def png_size(path):
    with open(path, "rb") as f:
        head = f.read(24)
    if head[:8] != b"\x89PNG\r\n\x1a\n":
        fail(f"{path}: not a PNG")
    return struct.unpack(">II", head[16:24])


def store_text(text):
    """A real line break to the "\\n" the store reads."""
    return text.replace("\n", "\\n")


def write(root, out):
    v = version(root)
    fields = {"version": v, "fw_min_play_timer": firmware(root, "PCTL_FW_MIN_PLAYTIMER"),
              "fw_tested_max": firmware(root, "PCTL_FW_TESTED_MAX")}
    pkg = json.loads(read(f"{root}/packaging/hb-appstore/pkgbuild.json"))
    info = pkg["info"]
    for key in ("title", "author", "category", "version", "url", "license", "description", "details", "binary"):
        if not info.get(key):
            fail(f"info.{key} missing from packaging/hb-appstore/pkgbuild.json")
    if info["category"] not in CATEGORIES:
        fail(f"info.category {info['category']!r} is not one of the store's: {', '.join(sorted(CATEGORIES))}")
    if isinstance(info["details"], list):
        info["details"] = "\n\n".join(info["details"])
    for key in ("version", "details", "description"):
        info[key] = store_text(info[key].format(**fields))
    pkg["changelog"] = store_text(changelog(root, v))

    folder = os.path.join(out, PACKAGE)
    shutil.rmtree(folder, ignore_errors=True)
    os.makedirs(folder)
    screenshots = 0
    for asset in pkg["assets"]:
        if "url" in asset:
            asset["url"] = asset["url"].format(**fields)
        asset.pop("leaves_out", None)
        source = asset.pop("from", None)
        kind = asset["type"]
        if kind in SIZES:
            if not source:
                fail(f"the {kind} {asset['url']} has no \"from\"")
            size = png_size(f"{root}/{source}")
            if size != SIZES[kind]:
                fail(f"{source}: {size[0]} x {size[1]}, the store's {kind} is {SIZES[kind][0]} x {SIZES[kind][1]}")
            shutil.copyfile(f"{root}/{source}", os.path.join(folder, asset["url"]))
            screenshots += kind == "screenshot"
    if screenshots > MAX_SCREENSHOTS:
        fail(f"{screenshots} screenshots, {MAX_SCREENSHOTS} at most")
    with open(os.path.join(folder, "pkgbuild.json"), "w", encoding="utf-8") as f:
        json.dump(pkg, f, indent=4, ensure_ascii=False)
        f.write("\n")
    print(f"{folder}/: PlayGuard {v}, {screenshots} screenshots")
    return pkg


def installs(root, archive):
    """What the store installs from the zip, "dest" for each file: as spinarak reads the listing."""
    source = json.loads(read(f"{root}/packaging/hb-appstore/pkgbuild.json"))
    asset = next((a for a in source["assets"] if a["type"] == "zip"), None)
    if asset is None:
        fail("the listing has no zip asset: the store would install nothing")
    with zipfile.ZipFile(archive) as z:
        files = [n for n in z.namelist() if not n.endswith("/")]
    dests, used = {}, set()
    for sub in asset["zip"]:
        pattern = sub["path"].lstrip("/")
        matches = [n for n in files if fnmatch.fnmatchcase(n, pattern)]
        if not matches:
            fail(f"{archive}: nothing matches the listing's path {sub['path']}")
        # spinarak's handleAsset: the path's prefix taken off each match, put after "dest";
        # what is left starting with "/" means one file (dest is its name) or a folder's files.
        base, dest = pattern.rstrip(".*/"), "/" + sub["dest"].lstrip("/")
        for name in matches:
            rel = name[len(base):] if name.startswith(base) else name
            if rel.startswith("/"):
                rel = "" if len(matches) == 1 else "/" + os.path.basename(name)
            dests[name] = dest + rel
            if "//" in dests[name]:
                fail(f"{name} would install to {dests[name]}: drop the \"/\" that ends \"dest\" ({sub['dest']})")
            used.add(name)
    left_out = set(asset.get("leaves_out", []))
    unknown = sorted(set(files) - used - left_out)
    if unknown:
        fail(f"{archive} holds files the store would not install: {', '.join(unknown)} "
             "(add them to the listing, or to its \"leaves_out\")")
    for name in sorted(left_out - set(files)):
        print(f"hbas_package: note: {name} (in \"leaves_out\") is not in {archive}")
    binary = source["info"]["binary"]
    if binary not in dests.values():
        fail(f"the binary {binary} is not among what the store installs: {', '.join(sorted(dests.values()))}")
    for name, dest in sorted(dests.items()):
        print(f"  {name} -> {dest}")
    return dests


def spinarak(script, folder, archive, binary, expected):
    """Builds the listing with the store's own tool, from the local zip."""
    with tempfile.TemporaryDirectory() as work:
        pkg_dir = os.path.join(work, PACKAGE)
        shutil.copytree(folder, pkg_dir)
        shutil.copyfile(archive, os.path.join(pkg_dir, "playguard.zip"))
        path = os.path.join(pkg_dir, "pkgbuild.json")
        pkg = json.loads(read(path))
        for asset in pkg["assets"]:
            if asset["type"] == "zip":
                asset["url"] = "playguard.zip"   # spinarak takes a file of the package's folder as is
        with open(path, "w", encoding="utf-8") as f:
            json.dump(pkg, f, indent=4, ensure_ascii=False)
        run =subprocess.run([sys.executable, os.path.abspath(script)], cwd=work,
                             capture_output=True, text=True)
        log = run.stdout + run.stderr
        print("\n".join(l for l in log.splitlines() if l.strip() and "hasn't changed" not in l))
        if run.returncode != 0:
            fail(f"spinarak exited with {run.returncode}")
        failed = re.search(r"Failed packages: \[(.*)\]", log)
        if failed and PACKAGE in failed.group(1):
            fail("spinarak could not build the package (above)")
        repo = json.loads(read(os.path.join(work, "public", "repo.json")))
        entry = next((p for p in repo["packages"] if p["name"] == PACKAGE), None)
        if entry is None:
            fail("spinarak's repo.json has no PlayGuard")
        if entry["binary"] != binary:
            fail(f"spinarak's binary is {entry['binary']!r}, not {binary!r}")
        manifest = read(os.path.join(work, "public", "packages", PACKAGE, "manifest.install"))
        listed = {"/" + line[3:] for line in manifest.splitlines() if line[1:3] == ": "}
        if not listed or listed != set(expected.values()):
            fail(f"spinarak's manifest installs {sorted(listed)}, not {sorted(set(expected.values()))}")
        print(f"spinarak: PlayGuard {entry['version']} builds; it installs {', '.join(sorted(listed))}, "
              f"{entry['screens']} screenshots")


def main():
    parser = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    parser.add_argument("out")
    parser.add_argument("--root", default=".")
    parser.add_argument("--zip", help="the release zip (make dist's playguard.zip) to check the listing against")
    parser.add_argument("--spinarak", help="switch-hbas-repo's spinarak.py, to build the listing with (needs --zip)")
    args = parser.parse_args()
    if args.spinarak and not args.zip:
        fail("--spinarak needs --zip")
    pkg = write(args.root, args.out)
    if args.zip:
        dests = installs(args.root, args.zip)
        if args.spinarak:
            spinarak(args.spinarak, os.path.join(args.out, PACKAGE), args.zip, pkg["info"]["binary"], dests)
    print(f"hb-appstore listing of PlayGuard {pkg['info']['version']}: OK")


if __name__ == "__main__":
    main()
