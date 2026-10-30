// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "action/shared_bell_logic.hpp"

#include <cstdio>
#include <cstring>
#include <memory>

#include "bl_notices.h"
#include "sync/sync_discovery.h"

namespace shared_bell_logic
{

namespace
{
// `s` into `out`, cut before a character that does not fit whole.
void copy(char* out, size_t cap, const std::string& s)
{
    size_t n = s.size() < cap ? s.size() : cap - 1;
    while (n > 0 && n < s.size() && ((unsigned char)s[n] & 0xC0) == 0x80) n--;
    std::memcpy(out, s.data(), n);
    out[n] = '\0';
}
}   // namespace

std::string file(const std::string& sd_root)
{
    char path[BL_APP_PATH_MAX];
    bl_notices_path(SYNC_MODULE, path, sizeof(path));
    return sd_root + (sd_root.empty() || sd_root.back() != '/' ? "/" : "") + path;
}

std::string text(const std::vector<Entry>& entries, const std::string& lang, const std::string& nro,
                 const std::string& previous, int64_t now)
{
    // Some 12 KB each, and as many tokens: on the heap, not the stack.
    auto before = std::make_unique<BlNotices>();
    auto after = std::make_unique<BlNotices>();
    auto toks = std::make_unique<BlJTok[]>(BL_NOTICES_TOKENS);
    std::string self = !nro.empty() && nro[0] == '/' ? nro.substr(1) : nro;
    bl_notices_init(before.get(), SYNC_MODULE, "");
    if (!previous.empty() && bl_notices_parse(before.get(), previous.data(), previous.size(), toks.get(), BL_NOTICES_TOKENS)) {
        if (!bl_notices_may_write(before.get())) return "";
        if (self.empty()) self = before->nro;
    }
    bl_notices_init(after.get(), SYNC_MODULE, self.c_str());
    std::memcpy(after->kept, before->kept, sizeof(after->kept));
    for (const Entry& e : entries) {
        if (after->count >= BL_NOTICES_MAX) break;
        BlNotice n = {};
        copy(n.id, sizeof(n.id), e.id);
        if (!n.id[0] || bl_notices_find(after.get(), n.id)) continue;
        n.level = e.todo ? BlNotice_Todo : BlNotice_Info;
        copy(n.title, sizeof(n.title), e.title);
        copy(n.detail, sizeof(n.detail), e.detail);
        copy(n.lang, sizeof(n.lang), lang);
        n.progress = -1;
        if (bl_route_valid(e.route.c_str())) copy(n.route, sizeof(n.route), e.route);
        n.updated = now;
        // Unchanged: the same `updated`, and what it did not know kept.
        if (const BlNotice* old = bl_notices_find(before.get(), n.id)) {
            std::memcpy(n.kept, old->kept, sizeof(n.kept));
            if (old->level == n.level && !std::strcmp(old->title, n.title) && !std::strcmp(old->detail, n.detail) &&
                !std::strcmp(old->lang, n.lang) && !std::strcmp(old->route, n.route) && old->progress == n.progress &&
                old->expires == n.expires)
                n.updated = old->updated;
        }
        bl_notices_put(after.get(), &n);
    }
    std::string out(64 * 1024, '\0');
    const size_t len = bl_notices_write(after.get(), &out[0], out.size());
    if (len == 0) return "";
    out.resize(len);
    return out == previous ? "" : out;
}

}   // namespace shared_bell_logic
