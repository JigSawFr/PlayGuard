#!/usr/bin/env python3
"""What differs between two builds of the same commit (CI builds the zips
twice to check they come out the same, docs/contributing/releases.md "Reproducible zips").

Compares <first> and <second>, two folders holding the outputs of a build:
the zips (entry by entry: name, size, CRC-32), playguard.nro (the bytes that
differ, and where: a code segment, the icon, the NACP, or which RomFS file),
playguard.nacp, the deko3d shaders Aurore compiled into the .nro
(dksh/*.dksh, from build/borealis_library/dksh/) and resources.sha256 (the checksum
of every file the RomFS was built from, `find . -type f | sort | xargs
sha256sum` in build/resources). Prints what differs, and exits 1 when
anything does.

Usage: tools/repro_diff.py <first> <second>
"""
import hashlib
import os
import struct
import sys
import zipfile


def sha(path):
    with open(path, "rb") as f:
        return hashlib.sha256(f.read()).hexdigest()


def zip_entries(path):
    with zipfile.ZipFile(path) as z:
        return {i.filename: (i.file_size, i.CRC, i.date_time) for i in z.infolist()}


def romfs_files(data, base):
    """{(offset, size): name} of the files of the RomFS at `base` in `data`."""
    if base + 0x50 > len(data):
        return {}
    (_hdr_size, _dh_ofs, _dh_size, dt_ofs, dt_size, _fh_ofs, _fh_size, ft_ofs, ft_size,
     part_ofs) = struct.unpack_from("<10Q", data, base)
    # Directories: their entry offset -> (parent's, name); the root is entry 0.
    dirs, pos = {}, 0
    while pos + 0x18 <= dt_size:
        parent, _sibling, _child, _file, _hash, name_size = struct.unpack_from("<6I", data, base + dt_ofs + pos)
        start = base + dt_ofs + pos + 0x18
        dirs[pos] = (parent, data[start:start + name_size].decode("utf-8", "replace"))
        pos += 0x18 + ((name_size + 3) & ~3)

    def path(entry):
        parts = []
        while entry in dirs and entry != 0 and len(parts) < 64:
            entry, name = dirs[entry][0], dirs[entry][1]
            parts.append(name)
        return "/".join(reversed(parts))

    files, pos = {}, 0
    while pos + 0x20 <= ft_size:
        parent, _sibling, off, size, _hash, name_size = struct.unpack_from("<IIQQII", data, base + ft_ofs + pos)
        start = base + ft_ofs + pos + 0x20
        name = data[start:start + name_size].decode("utf-8", "replace")
        folder = path(parent)
        files[(base + part_ofs + off, size)] = f"{folder}/{name}" if folder else name
        pos += 0x20 + ((name_size + 3) & ~3)
    return files


def nro_regions(data):
    """[(start, end, label)] of an NRO: its three segments, then its assets."""
    regions = [(0, 0x80, "NRO header")]
    if len(data) < 0x80 or data[0x10:0x14] != b"NRO0":
        return regions
    size = struct.unpack_from("<I", data, 0x18)[0]
    for i, label in enumerate(("code (.text)", "read-only data (.rodata)", "data (.data)")):
        off, seg = struct.unpack_from("<II", data, 0x20 + 8 * i)
        regions.append((off, off + seg, label))
    if data[size:size + 4] == b"ASET":
        icon, nacp, romfs = (struct.unpack_from("<QQ", data, size + 8 + 16 * i) for i in range(3))
        regions.append((size + icon[0], size + icon[0] + icon[1], "icon"))
        regions.append((size + nacp[0], size + nacp[0] + nacp[1], "NACP"))
        for (off, length), name in sorted(romfs_files(data, size + romfs[0]).items()):
            regions.append((off, off + length, "RomFS " + name))
        regions.append((size + romfs[0], size + romfs[0] + romfs[1], "RomFS (tables, padding)"))
    return regions


def where(regions, offset):
    hits = [r for r in regions if r[0] <= offset < r[1]]
    return min(hits, key=lambda r: r[1] - r[0])[2] if hits else "outside every region"


def compare_nro(a, b):
    with open(a, "rb") as f:
        x = f.read()
    with open(b, "rb") as f:
        y = f.read()
    out = []
    if len(x) != len(y):
        out.append(f"  sizes differ: {len(x)} and {len(y)} bytes")
    diffs = [i for i in range(min(len(x), len(y))) if x[i] != y[i]]
    if diffs:
        regions = nro_regions(x)
        by_region = {}
        for i in diffs:
            by_region.setdefault(where(regions, i), []).append(i)
        out.append(f"  {len(diffs)} byte(s) differ:")
        for label, offsets in by_region.items():
            out.append(f"    {label}: {len(offsets)} byte(s), first at 0x{offsets[0]:x}")
    return out


def main():
    if len(sys.argv) != 3:
        print(__doc__.strip().splitlines()[-1])
        return 2
    first, second = sys.argv[1], sys.argv[2]
    found = False
    for name in sorted(set(os.listdir(first)) & set(os.listdir(second))):
        a, b = os.path.join(first, name), os.path.join(second, name)
        if os.path.isdir(a) or os.path.isdir(b):
            for sub in sorted(set(os.listdir(a)) | set(os.listdir(b))):
                pa, pb = os.path.join(a, sub), os.path.join(b, sub)
                if not (os.path.isfile(pa) and os.path.isfile(pb)):
                    print(f"{name}/{sub}: only in one build")
                    found = True
                elif sha(pa) != sha(pb):
                    print(f"{name}/{sub}: differs")
                    found = True
            continue
        if sha(a) == sha(b):
            continue
        found = True
        print(f"{name}: differs")
        if name.endswith(".zip"):
            ea, eb = zip_entries(a), zip_entries(b)
            for entry in sorted(set(ea) | set(eb)):
                if ea.get(entry) != eb.get(entry):
                    print(f"  {entry}: {ea.get(entry)} / {eb.get(entry)}   (size, CRC-32, date)")
        elif name.endswith(".nro"):
            print("\n".join(compare_nro(a, b)))
        elif name.endswith(".sha256"):
            la = set(open(a, encoding="utf-8").read().splitlines())
            lb = set(open(b, encoding="utf-8").read().splitlines())
            for line in sorted(la ^ lb, key=lambda l: l.split()[-1]):
                print(f"  {'first ' if line in la else 'second'}: {line}")
    if not found:
        print("the two builds are the same")
    return 1 if found else 0


if __name__ == "__main__":
    sys.exit(main())
