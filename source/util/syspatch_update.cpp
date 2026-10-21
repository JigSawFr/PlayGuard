// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "util/syspatch_update.hpp"

#include <cstdio>
#include <vector>

#include "util/dev_builds.hpp"
#include "util/json_util.hpp"
#include "util/paths.hpp"
#include "util/safe_text.hpp"
#include "util/text.hpp"
#include "util/zip_read.hpp"

namespace syspatch_update
{

namespace
{
using json = nlohmann::json;

// What the release's zip puts on the SD card (sys-patch's Makefiles: dist).
struct File
{
    const char* path;   // in the zip and on the SD card, the same
    uint64_t    max;
    bool        required;
};
const File FILES[] = {
    { "atmosphere/contents/420000000000000B/exefs.nsp", 4 * 1024 * 1024, true },
    { "atmosphere/contents/420000000000000B/toolbox.json", 4 * 1024, true },
    { "atmosphere/contents/420000000000000B/flags/boot2.flag", 0, true },
    { "switch/.overlays/sys-patch-overlay.ovl", 4 * 1024 * 1024, false },
};

std::string join(const std::string& root, const std::string& rel)
{
    if (root.empty() || root.back() == '/') return root + rel;
    return root + "/" + rel;
}

std::string parent(const std::string& path)
{
    const size_t slash = path.rfind('/');
    return slash == std::string::npos ? "" : path.substr(0, slash);
}

// The dotted numbers at the start of `text` (after a "v").
std::vector<unsigned long> numbers(const std::string& text)
{
    std::vector<unsigned long> out;
    size_t i = !text.empty() && (text[0] == 'v' || text[0] == 'V') ? 1 : 0;
    while (i < text.size() && text[i] >= '0' && text[i] <= '9') {
        unsigned long n = 0;
        while (i < text.size() && text[i] >= '0' && text[i] <= '9' && n < 100000000ul) n = n * 10 + (unsigned long)(text[i++] - '0');
        out.push_back(n);
        if (i + 1 < text.size() && text[i] == '.' && text[i + 1] >= '0' && text[i + 1] <= '9') ++i;
        else break;
    }
    return out;
}

bool zip_asset(const std::string& name)
{
    return text::starts_with(name, "sys-patch") && text::ends_with(name, ".zip");
}
}   // namespace

bool parse_release(const std::string& text, Release* out)
{
    return json_util::parse_with(text, [out](const json& r) {
        if (!r.is_object() || r.value("draft", true) || r.value("prerelease", true)) return false;
        const json* assets = json_util::array_at(r, "assets");
        if (!assets) return false;
        const std::string tag = json_util::string_at(r, "tag_name");
        Release x;
        x.version = tag.size() > 1 && (tag[0] == 'v' || tag[0] == 'V') ? tag.substr(1) : tag;
        if (!safe_text::is_version(x.version) || numbers(x.version).empty()) return false;
        x.page = json_util::string_at(r, "html_url");
        if (!text::starts_with(x.page, "https://github.com/")) x.page = PAGE;
        for (const auto& a : *assets) {
            if (!a.is_object() || !zip_asset(json_util::string_at(a, "name"))) continue;
            x.url = json_util::string_at(a, "browser_download_url");
            x.sha256 = dev_builds::digest_hex(json_util::string_at(a, "digest"));
            const int64_t size = json_util::integer_or(a, "size");
            if (!text::starts_with(x.url, "https://") || size <= 0 || (uint64_t)size > MAX_ZIP) return false;
            x.size = (uint64_t)size;
            *out = x;
            return true;
        }
        return false;
    });
}

int compare(const std::string& a, const std::string& b)
{
    const std::vector<unsigned long> x = numbers(a), y = numbers(b);
    for (size_t i = 0; i < x.size() || i < y.size(); i++) {
        const unsigned long p = i < x.size() ? x[i] : 0, q = i < y.size() ? y[i] : 0;
        if (p != q) return p < q ? -1 : 1;
    }
    if (x.empty() != y.empty()) return x.empty() ? -1 : 1;
    return 0;
}

bool newer(const Release& r, const std::string& installed)
{
    return numbers(installed).empty() || compare(r.version, installed) > 0;
}

bool install(const std::string& zip_path, const std::string& sd_root, const std::string& backup_dir,
             std::string* error)
{
    // Every file out first, next to its place: nothing changes on a damaged zip.
    std::vector<const File*> taken;
    std::string err;
    for (const File& f : FILES) {
        const std::string dest = join(sd_root, f.path);
        if (!paths::ensure_dir(parent(dest))) {
            err = "cannot create " + parent(f.path);
        } else if (zip_read::extract(zip_path, f.path, dest + ".new", f.max, &err)) {
            taken.push_back(&f);
            continue;
        } else if (!f.required) {
            err.clear();   // not in this release: the one there stays
            continue;
        }
        for (const File* t : taken) std::remove((join(sd_root, t->path) + ".new").c_str());
        if (error) *error = err;
        return false;
    }

    for (const File* f : taken) {
        const std::string dest = join(sd_root, f->path);
        std::string old;
        if (paths::file_exists(dest) && paths::read_file(dest, old)) {
            const std::string kept = join(backup_dir, f->path);
            if (!paths::ensure_dir(parent(kept)) || !paths::atomic_write(kept, old)) {
                err = "cannot keep the installed " + std::string(f->path);
                break;
            }
        }
        if (!paths::replace_file(dest, dest + ".new", "", &err)) {
            err = std::string(f->path) + ": " + err;
            break;
        }
    }
    if (!err.empty()) {
        for (const File* t : taken) std::remove((join(sd_root, t->path) + ".new").c_str());
        if (error) *error = err;
        return false;
    }
    return true;
}

}   // namespace syspatch_update
