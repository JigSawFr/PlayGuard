// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "util/log_files.hpp"

#include <algorithm>
#include <cctype>
#include <cstdio>
#include <cstring>
#include <sys/stat.h>

#include "util/paths.hpp"

namespace log_files
{

namespace
{
bool digits(const std::string& s, size_t from, size_t count)
{
    if (from + count > s.size()) return false;
    for (size_t i = from; i < from + count; i++)
        if (!std::isdigit((unsigned char)s[i])) return false;
    return true;
}

// "YYYYMMDD_HHMMSS"
bool is_stamp(const std::string& s, size_t from)
{
    return digits(s, from, 8) && from + 8 < s.size() && s[from + 8] == '_' && digits(s, from + 9, 6);
}

// The n of "_n" after the stamp, 0 without one.
unsigned suffix_n(const std::string& name, size_t stamp_end)
{
    if (name[stamp_end] != '_') return 0;
    return (unsigned)std::strtoul(name.c_str() + stamp_end + 1, nullptr, 10);
}

bool exists(const std::string& path)
{
    struct stat st;
    return stat(path.c_str(), &st) == 0;
}

// `from` to `to` when `to` is free (FAT's rename would not replace it anyway).
bool move(const std::string& from, const std::string& to)
{
    if (!exists(from) || exists(to)) return false;
    return std::rename(from.c_str(), to.c_str()) == 0;
}
}   // namespace

std::string name(const char* prefix, const std::string& stamp, unsigned n)
{
    std::string out = std::string(prefix) + "_" + stamp;
    if (n > 0) out += "_" + std::to_string(n);
    return out + ".txt";
}

std::string stamp(std::time_t t)
{
    std::tm tmv{};
#ifdef _WIN32
    const bool ok = localtime_s(&tmv, &t) == 0;
#else
    const bool ok = localtime_r(&t, &tmv) != nullptr;
#endif
    char buf[32];
    if (!ok || std::strftime(buf, sizeof(buf), "%Y%m%d_%H%M%S", &tmv) == 0) return "19700101_000000";
    return buf;
}

bool parse(const std::string& name, const char* prefix, std::string* stamp)
{
    const size_t p = std::strlen(prefix);
    if (name.compare(0, p, prefix) != 0 || name.size() < p + 1 + 15 + 4 || name[p] != '_') return false;
    const size_t at = p + 1, end = at + 15;
    if (!is_stamp(name, at) || name.compare(name.size() - 4, 4, ".txt") != 0) return false;
    const size_t rest = name.size() - 4 - end;   // "" or "_<n>"
    if (rest == 1 || (rest > 1 && (name[end] != '_' || !digits(name, end + 1, rest - 1)))) return false;
    if (stamp) *stamp = name.substr(at, 15);
    return true;
}

std::vector<std::string> list(const std::string& dir, const char* prefix)
{
    std::vector<std::string> names;
    for (const std::string& n : paths::list_files(dir, ".txt"))
        if (parse(n, prefix)) names.push_back(n);
    const size_t end = std::strlen(prefix) + 1 + 15;
    std::sort(names.begin(), names.end(), [end](const std::string& a, const std::string& b) {
        const int c = a.compare(0, end, b, 0, end);
        if (c != 0) return c > 0;
        return suffix_n(a, end) > suffix_n(b, end);
    });
    return names;
}

std::vector<std::string> to_prune(const std::vector<File>& files, const Limits& limits, const std::string& cutoff)
{
    std::vector<std::string> out;
    const uint64_t max_bytes = (uint64_t)limits.mb * 1024 * 1024;
    uint64_t total = 0;
    for (size_t i = 0; i < files.size(); i++) {
        total += files[i].bytes;
        if (i == 0) continue;   // the newest stays
        const bool too_many = limits.count > 0 && i >= (size_t)limits.count;
        const bool too_old  = limits.days > 0 && files[i].stamp < cutoff;
        const bool too_big  = limits.mb > 0 && total > max_bytes;
        if (too_many || too_old || too_big) out.push_back(files[i].name);
    }
    return out;
}

size_t prune(const std::string& dir, const char* prefix, const Limits& limits, std::time_t now)
{
    if (limits.count <= 0 && limits.days <= 0 && limits.mb <= 0) return 0;
    std::vector<File> files;
    for (const std::string& n : list(dir, prefix)) {
        File f;
        f.name = n;
        parse(n, prefix, &f.stamp);
        struct stat st;
        if (stat((dir + "/" + n).c_str(), &st) == 0) f.bytes = (uint64_t)st.st_size;
        files.push_back(f);
    }
    const std::string cutoff = stamp(now - (std::time_t)limits.days * 86400);
    size_t removed = 0;
    for (const std::string& n : to_prune(files, limits, cutoff))
        if (std::remove((dir + "/" + n).c_str()) == 0) removed++;
    return removed;
}

size_t migrate(const std::string& logs_dir)
{
    const std::string reports = logs_dir + "/reports", crashes = logs_dir + "/crashes",
                      play_timer = logs_dir + "/play_timer";
    size_t moved = 0;
    for (const std::string& n : paths::list_files(logs_dir, ".txt")) {
        // diagnostic::save's names before the folders: "20261009_141203[_n].txt".
        if (!is_stamp(n, 0)) continue;
        const std::string rest = n.substr(15, n.size() - 15 - 4);
        if (!rest.empty() && (rest[0] != '_' || !digits(rest, 1, rest.size() - 1) || rest.size() < 2)) continue;
        if (paths::ensure_dir(reports) && move(logs_dir + "/" + n, reports + "/" + std::string(REPORT) + "_" + n)) moved++;
    }

    // crash.txt held every crash, one line each, starting "2026-10-11 14:25:30".
    const std::string crash = logs_dir + "/crash.txt";
    std::string text;
    if (paths::read_file(crash, text)) {
        std::string st = "19700101_000000";
        if (text.size() >= 19 && digits(text, 0, 4) && text[4] == '-' && digits(text, 5, 2) && text[7] == '-' &&
            digits(text, 8, 2) && text[10] == ' ' && digits(text, 11, 2) && text[13] == ':' && digits(text, 14, 2) &&
            text[16] == ':' && digits(text, 17, 2))
            st = text.substr(0, 4) + text.substr(5, 2) + text.substr(8, 2) + "_" + text.substr(11, 2) +
                 text.substr(14, 2) + text.substr(17, 2);
        if (paths::ensure_dir(crashes)) {
            for (unsigned i = 0; i < 100; i++)
                if (!exists(crashes + "/" + name(CRASH, st, i))) {
                    if (move(crash, crashes + "/" + name(CRASH, st, i))) moved++;
                    break;
                }
        }
    }

    static const char* const PT[][2] = { { "play_timer_log.csv", "log.csv" },
                                         { "play_timer_log.old.csv", "log.old.csv" },
                                         { "play_timer_block.json", "block.json" } };
    for (const auto& f : PT) {
        const std::string from = logs_dir + "/" + f[0];
        if (exists(from) && paths::ensure_dir(play_timer) && move(from, play_timer + "/" + f[1])) moved++;
    }
    return moved;
}

}   // namespace log_files
