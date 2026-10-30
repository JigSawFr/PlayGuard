// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "util/data_move.hpp"

#include <algorithm>
#include <cctype>
#include <cstdio>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <utility>
#include <vector>

#include "util/log_files.hpp"
#include "util/paths.hpp"

namespace data_move
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

// "2026-10-11 14:25[:30]" at the start of `line` as a stamp ("20261011_142500",
// the seconds when there are some); false without one.
bool line_stamp(const std::string& line, std::string* stamp)
{
    if (line.size() < 16 || !digits(line, 0, 4) || line[4] != '-' || !digits(line, 5, 2) || line[7] != '-' ||
        !digits(line, 8, 2) || line[10] != ' ' || !digits(line, 11, 2) || line[13] != ':' || !digits(line, 14, 2))
        return false;
    const bool seconds = line.size() >= 19 && line[16] == ':' && digits(line, 17, 2);
    *stamp = line.substr(0, 4) + line.substr(5, 2) + line.substr(8, 2) + "_" + line.substr(11, 2) +
             line.substr(14, 2) + (seconds ? line.substr(17, 2) : std::string("00"));
    return true;
}

// The lines of an upload file before the shortcuts ("2026-10-11 14:25  <link>
// (remove: <link>)") as log_files::shortcut() files, named after their date or `stamp`
// without one, in `dir`. False when one could not be written.
bool save_shortcuts(const std::string& text, const std::string& stamp, const std::string& dir)
{
    size_t from = 0;
    while (from < text.size()) {
        size_t nl = text.find('\n', from);
        if (nl == std::string::npos) nl = text.size();
        const std::string line = text.substr(from, nl - from);
        from = nl + 1;
        const size_t at = line.find("https://");
        if (at == std::string::npos) continue;   // nothing to open
        const std::string url = line.substr(at, line.find_first_of(" \t\r)", at) - at);
        std::string removal;
        const size_t r = line.find("(remove: ");
        if (r != std::string::npos) removal = line.substr(r + 9, line.find_first_of(" \t\r)", r + 9) - r - 9);
        std::string st = stamp;
        const bool dated = line_stamp(line, &st);
        unsigned n = 0;
        while (n < 1000 && paths::file_exists(dir + "/" + log_files::name(log_files::UPLOAD, st, n))) n++;
        if (!paths::atomic_write(dir + "/" + log_files::name(log_files::UPLOAD, st, n), log_files::shortcut(url, removal, dated ? line.substr(0, 16) : "")))
            return false;
    }
    return true;
}

// `from` to `to` when `to` is free (FAT's rename would not replace it anyway).
bool move_free(const std::string& from, const std::string& to)
{
    if (!paths::file_exists(from) || paths::file_exists(to)) return false;
    return std::rename(from.c_str(), to.c_str()) == 0;
}

bool stat_of(const std::string& path, struct stat* st)
{
    return stat(path.c_str(), st) == 0;
}

// `from` to `to`. A folder whose name is taken in `to` is merged into it,
// then removed when nothing of it is left behind.
size_t move(const std::string& from, const std::string& to)
{
    struct stat src, dst;
    if (!stat_of(from, &src)) return 0;
    if (!stat_of(to, &dst)) return std::rename(from.c_str(), to.c_str()) == 0 ? 1 : 0;
    if (!S_ISDIR(src.st_mode) || !S_ISDIR(dst.st_mode)) return 0;   // a file already there wins
    std::vector<std::string> names;
    if (DIR* d = opendir(from.c_str())) {
        while (dirent* e = readdir(d)) {
            const std::string n = e->d_name;
            if (n != "." && n != "..") names.push_back(n);
        }
        closedir(d);
    }
    size_t moved = 0;
    for (const std::string& n : names) moved += move(from + "/" + n, to + "/" + n);
    rmdir(from.c_str());   // fails, and stays, when something was not moved
    return moved;
}
}   // namespace

std::string old_dir()
{
    return "/switch/playguard";
}

size_t run(const std::string& from, const std::string& to)
{
    static const char* const FILES[] = { "config.json", "history.json", "watch.json", "github_token",
                                         "rescue_report.txt", "sync.conf" };
    static const char* const SUFFIXES[] = { "", ".tmp", ".bad", ".bad.1" };
    static const char* const OTHERS[] = { "RESCUE", "RESCUE.txt", "RESCUE.done",
                                          "profiles", "backups", "exports", "cache", "logs", "sync" };
    if (from == to) return 0;
    struct stat st;
    if (!stat_of(from, &st) || !S_ISDIR(st.st_mode)) return 0;
    std::vector<std::string> names;
    for (const char* f : FILES)
        for (const char* s : SUFFIXES) names.push_back(std::string(f) + s);
    for (const char* o : OTHERS) names.push_back(o);
    size_t moved = 0;
    for (const std::string& n : names) {
        if (!stat_of(from + "/" + n, &st)) continue;
        if (!paths::ensure_dir(to)) return moved;
        moved += move(from + "/" + n, to + "/" + n);
    }
    return moved;
}

size_t to_nand(const std::string& data_dir, const std::string& nand_dir)
{
    static const char* const FILES[] = { "history.json", "watch.json" };
    static const char* const SUFFIXES[] = { "", ".tmp", ".bad", ".bad.1" };
    // The folder read once: on the SD card each look for a file costs about
    // a millisecond, and once moved the names are missing at every start.
    const std::vector<std::string> names = paths::list_files(data_dir, "");
    const auto has = [&names](const std::string& n) { return std::binary_search(names.begin(), names.end(), n); };
    std::vector<std::pair<std::string, std::string>> moves;   // from, to (relative)
    for (const char* f : FILES)
        for (const char* s : SUFFIXES)
            if (has(std::string(f) + s)) moves.push_back({ std::string(f) + s, std::string(f) + s });
    if (has("backups")) moves.push_back({ "backups", "backups" });
    // The remote link's records and what the agent sysmodule wrote for this
    // system (util/sync_files.hpp); profiles.txt stays shared.
    if (has("sync"))
        for (const char* f : { "nro_state.txt", "agent_state.txt", "agent_events.log", "agent_events.log.read", "agent_events.log.rejected", "names.txt" })
            moves.push_back({ std::string("sync/") + f, std::string("sync/") + f });
    if (has("cache"))
        for (const std::string& n : paths::list_files(data_dir + "/cache", ".bin"))
            if (n.starts_with("activity_")) moves.push_back({ "cache/" + n, "cache/" + n });
    size_t moved = 0;
    struct stat st;
    for (const auto& m : moves) {
        if (!stat_of(data_dir + "/" + m.first, &st)) continue;
        const std::string to = nand_dir + "/" + m.second;
        if (!paths::ensure_dir(to.substr(0, to.find_last_of('/')))) return moved;
        moved += move(data_dir + "/" + m.first, to);
    }
    return moved;
}

size_t logs(const std::string& logs_dir)
{
    const std::string reports = logs_dir + "/reports", crashes = logs_dir + "/crashes",
                      play_timer = logs_dir + "/play_timer";
    size_t moved = 0;
    // logs/ read once: on the SD card each look for a file costs about a
    // millisecond, and the old names are missing at nearly every start.
    const std::vector<std::string> names = paths::list_files(logs_dir, "");
    const auto has = [&names](const std::string& n) { return std::binary_search(names.begin(), names.end(), n); };
    for (const std::string& n : names) {
        if (n.size() <= 4 || !n.ends_with(".txt")) continue;
        // diagnostic::save's names before the folders: "20261009_141203[_n].txt".
        if (!is_stamp(n, 0)) continue;
        const std::string rest = n.substr(15, n.size() - 15 - 4);
        if (!rest.empty() && (rest[0] != '_' || !digits(rest, 1, rest.size() - 1) || rest.size() < 2)) continue;
        if (paths::ensure_dir(reports) && move_free(logs_dir + "/" + n, reports + "/" + std::string(log_files::REPORT) + "_" + n)) moved++;
    }

    // crash.txt held every crash, one line each, starting "2026-10-11 14:25:30".
    const std::string crash = logs_dir + "/crash.txt";
    std::string text;
    if ((has("crash.txt") || has("crash.txt.tmp")) && paths::read_file(crash, text)) {
        std::string st = "19700101_000000";
        line_stamp(text, &st);
        if (paths::ensure_dir(crashes)) {
            for (unsigned i = 0; i < 100; i++)
                if (!paths::file_exists(crashes + "/" + log_files::name(log_files::CRASH, st, i))) {
                    if (move_free(crash, crashes + "/" + log_files::name(log_files::CRASH, st, i))) moved++;
                    break;
                }
        }
    }

    // uploads.txt held every link, one line each: "2026-10-11 14:25  https://…";
    // then uploads/upload_<date>.txt, a file per link.
    const std::string uploads = logs_dir + "/uploads.txt", uploads_dir = logs_dir + "/uploads";
    if ((has("uploads.txt") || has("uploads.txt.tmp")) && paths::read_file(uploads, text) && paths::ensure_dir(uploads_dir) &&
        save_shortcuts(text, "19700101_000000", uploads_dir)) {
        // Only once every line is in a file of its own.
        std::remove(uploads.c_str());
        std::remove((uploads + ".tmp").c_str());
        moved++;
    }
    for (const std::string& n : has("uploads") ? paths::list_files(uploads_dir, ".txt") : std::vector<std::string>{}) {
        const std::string path = uploads_dir + "/" + n;
        const std::string as_url = n.substr(0, n.size() - 4) + ".url";
        std::string st;
        if (!log_files::parse(as_url, log_files::UPLOAD, &st) || !paths::read_file(path, text) || !save_shortcuts(text, st, uploads_dir))
            continue;
        std::remove(path.c_str());
        moved++;
    }

    static const char* const PT[][2] = { { "play_timer_log.csv", "log.csv" },
                                         { "play_timer_log.old.csv", "log.old.csv" },
                                         { "play_timer_block.json", "block.json" } };
    for (const auto& f : PT) {
        const std::string from = logs_dir + "/" + f[0];
        if (has(f[0]) && paths::file_exists(from) && paths::ensure_dir(play_timer) && move_free(from, play_timer + "/" + f[1])) moved++;
    }
    return moved;
}

}   // namespace data_move
