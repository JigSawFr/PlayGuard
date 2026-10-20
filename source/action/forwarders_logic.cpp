// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "action/forwarders_logic.hpp"

#include <algorithm>
#include <cerrno>
#include <cstdio>

#include "util/text.hpp"

namespace forwarders_logic
{

std::vector<Row> merge(const std::vector<forwarders::Made>& made, const std::vector<Installed>& installed,
                       const std::function<bool(const std::string&)>& nro_exists, const std::string& self_nro)
{
    std::vector<Row> rows;
    const auto made_of = [&made](uint64_t id) -> const forwarders::Made* {
        for (const auto& m : made)
            if (m.id == id) return &m;
        return nullptr;
    };
    for (const Installed& i : installed) {
        bool seen = false;
        for (const Row& r : rows) seen = seen || r.id == i.id;
        if (seen) continue;
        Row r;
        r.id = i.id;
        r.name = i.name;
        r.author = i.author;
        r.version = i.version;
        if (const forwarders::Made* m = made_of(i.id)) {
            r.nro = m->nro;
            if (r.name.empty()) r.name = m->name;
            if (r.author.empty()) r.author = m->author;
            r.pin = m->pin;
            r.verified = m->pin && m->verified;
            r.free_days = m->free_days;
            r.free_from = m->free_from;
            r.free_to = m->free_to;
            r.state = !nro_exists(m->nro)            ? State::NroMissing
                      : m->pin && !nro_exists(m->gate) ? State::GateMissing
                                                       : State::Ready;
        } else {
            r.state = State::Elsewhere;
        }
        rows.push_back(r);
    }
    for (const forwarders::Made& m : made) {
        bool seen = false;
        for (const Row& r : rows) seen = seen || r.id == m.id;
        if (seen) continue;
        Row r;
        r.id = m.id;
        r.name = m.name;
        r.author = m.author;
        r.nro = m.nro;
        r.pin = m.pin;
        r.gone_seen = m.gone;
        r.verified = m.pin && m.verified;
        r.free_days = m.free_days;
        r.free_from = m.free_from;
        r.free_to = m.free_to;
        r.state = State::NotInstalled;
        rows.push_back(r);
    }
    for (Row& r : rows) {
        if (r.name.empty()) r.name = r.nro.empty() ? forwarders::hex_id(r.id) : file_title(r.nro);
        r.self = !self_nro.empty() && r.nro == self_nro;
    }
    std::sort(rows.begin(), rows.end(), [](const Row& a, const Row& b) {
        const std::string la = text::lower_ascii(a.name), lb = text::lower_ascii(b.name);
        return la != lb ? la < lb : a.id < b.id;
    });
    return rows;
}

const Row* self_row(const std::vector<Row>& rows)
{
    for (const Row& r : rows)
        if (r.self) return &r;
    return nullptr;
}

std::string file_title(const std::string& path)
{
    const size_t slash = path.find_last_of('/');
    std::string name = slash == std::string::npos ? path : path.substr(slash + 1);
    if (name.size() > 4 && text::lower_ascii(name.substr(name.size() - 4)) == ".nro") name.resize(name.size() - 4);
    return name;
}

std::string default_name(const std::string& nacp_name, const std::string& path)
{
    const std::string name = forwarders::clean_text(nacp_name, forwarders::MAX_NAME);
    return name.empty() ? forwarders::clean_text(file_title(path), forwarders::MAX_NAME) : name;
}

Patches patches_state(const patches::Report& r)
{
    switch (patches::status(r)) {
        case patches::Status::SysPatch: return Patches::Ready;
        case patches::Status::SysPatchIncomplete:
            if (patches::issue(r) == patches::Issue::NotPatched)
                return patches::patched(r.fs) && patches::patched(r.ldr) ? Patches::Ready : Patches::Missing;
            if (patches::issue(r) == patches::Issue::Skipped || patches::issue(r) == patches::Issue::NotAtBoot)
                return Patches::Missing;
            return Patches::Unknown;
        case patches::Status::FilesOnly: {
            const auto has = [&r](const char* set) { return std::find(r.files.begin(), r.files.end(), set) != r.files.end(); };
            // hekate's set patches both when it boots Atmosphère (fss0).
            return (has("fs") && has("loader")) || has("hekate") ? Patches::Ready : Patches::Missing;
        }
        default: return Patches::Missing;
    }
}

bool is_menu(const std::string& nro)
{
    if (nro.empty()) return false;
    const std::string name = text::lower_ascii(file_title(nro));
    return name == "hbmenu" || name == "sphaira";
}

bool writes_sd(const std::string& nro)
{
    static const char* const NAMES[] = { "nx-shell", "nxshell", "goldleaf", "dbi", "tinfoil", "awoo", "cyberfoil",
                                         "appstore", "ftpd", "jksv", "edizon", "nxthemesinstaller", "nxmtp" };
    const std::string name = text::lower_ascii(file_title(nro));
    for (const char* n : NAMES)
        if (name == n) return true;
    return false;
}

std::string hours_text(int days, int from, int to, const std::vector<std::string>& day_names)
{
    const auto hm = [](int m) {
        char b[16];
        std::snprintf(b, sizeof(b), "%02d:%02d", m / 60, m % 60);
        return std::string(b);
    };
    std::string d;
    if (day_names.size() == 7) {
        if ((days & 0x7F) == 0x7F) d = day_names[1] + "–" + day_names[0];   // Monday to Sunday
        else if ((days & 0x7F) == 0x3E) d = day_names[1] + "–" + day_names[5];
        else if ((days & 0x7F) == 0x41) d = day_names[6] + "–" + day_names[0];
        else
            for (int i = 1; i <= 7; i++)
                if (days >> (i % 7) & 1) d += (d.empty() ? "" : ", ") + day_names[i % 7];
    }
    return d + " " + hm(from) + "–" + hm(to);
}

LockCheck lock_check(const std::vector<Row>& rows, bool pin_set, Patches patches)
{
    LockCheck c;
    c.pin = pin_set;
    c.patches = patches != Patches::Missing;
    for (const Row& r : rows) {
        if (r.state == State::NotInstalled) continue;
        if (r.self && r.state == State::Ready) c.self = true;
        if (r.state == State::Elsewhere) c.open_doors.push_back(r.name);
        if (!is_menu(r.nro)) {
            if (!r.pin && !r.self && r.state != State::Elsewhere) {
                c.unprotected.push_back(r.name);
                if (writes_sd(r.nro)) c.sd_writers.push_back(r.name);
            }
            continue;
        }
        if (!r.pin) c.open_doors.push_back(r.name);
        else if (r.state == State::Ready) {
            c.menus.push_back(r.name);
            if (r.verified) c.verified.push_back(r.name);
        }
    }
    return c;
}

namespace
{
bool way_back_menu(const Row& r)
{
    return r.pin && r.state == State::Ready && is_menu(r.nro);
}
}   // namespace

std::vector<std::string> other_menus(const std::vector<Row>& rows, const Row& row)
{
    std::vector<std::string> out;
    for (const Row& r : rows)
        if (r.id != row.id && way_back_menu(r)) out.push_back(r.name);
    return out;
}

Guard way_back_guard(const std::vector<Row>& rows, const Row& row, bool locked)
{
    if (!locked || row.state == State::NotInstalled) return Guard::Allowed;
    if (row.self) return Guard::Refused;
    if (!way_back_menu(row)) return Guard::Allowed;
    return other_menus(rows, row).empty() ? Guard::Refused : Guard::Confirm;
}

std::vector<Refusals> repeated_refusals(const std::vector<std::pair<std::string, std::string>>& refused, int at_least)
{
    // Per name and day, in the order first seen.
    std::vector<Refusals> days;
    for (const auto& [name, when] : refused) {
        const std::string day = when.substr(0, 10);
        bool seen = false;
        for (Refusals& d : days)
            if (d.name == name && d.day == day) {
                d.count++;
                seen = true;
            }
        if (!seen) days.push_back({ name, day, 1 });
    }
    std::vector<Refusals> out;
    for (const Refusals& d : days) {
        if (d.count < at_least) continue;
        bool merged = false;
        for (Refusals& o : out)
            if (o.name == d.name) {
                if (d.count > o.count) o = d;
                merged = true;
            }
        if (!merged) out.push_back(d);
    }
    return out;
}

WriteHint write_hint(int err, uint64_t free_bytes)
{
    if (err == ENOSPC || free_bytes < 64 * 1024) return WriteHint::Full;
    if (err == EROFS || err == EACCES || err == EPERM) return WriteHint::ReadOnly;
    return WriteHint::None;
}

}   // namespace forwarders_logic
