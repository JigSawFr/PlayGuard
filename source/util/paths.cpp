// Copyright (C) 2026 JigSawFr, (C) 2026 Taylor.  GPLv3-or-later (see LICENSE).
#include "util/paths.hpp"

#include <algorithm>
#include <cerrno>
#include <cstdio>
#include <cstring>
#include <dirent.h>
#include <fcntl.h>
#include <fstream>
#include <sstream>
#include <sys/stat.h>
#include <unistd.h>

namespace paths
{

std::string data_dir()
{
#ifdef __SWITCH__
    return "/config/playguard";
#else
    return "./playguard_data";
#endif
}

std::string logs_dir()     { return data_dir() + "/logs"; }
std::string reports_dir()  { return logs_dir() + "/reports"; }
std::string crashes_dir()  { return logs_dir() + "/crashes"; }
std::string sessions_dir() { return logs_dir() + "/sessions"; }
std::string uploads_dir()  { return logs_dir() + "/uploads"; }
std::string play_timer_dir() { return logs_dir() + "/play_timer"; }
std::string profiles_dir() { return data_dir() + "/profiles"; }
std::string exports_dir()  { return data_dir() + "/exports"; }
std::string config_file()  { return data_dir() + "/config.json"; }

namespace
{
std::string s_nand = "unknown";
}
void        set_nand(const std::string& key) { s_nand = key.empty() ? "unknown" : key; }
std::string nand_key()     { return s_nand; }
std::string nand_dir()     { return data_dir() + "/nand/" + s_nand; }
std::string backups_dir()  { return nand_dir() + "/backups"; }
std::string history_file() { return nand_dir() + "/history.json"; }
std::string state_file()   { return nand_dir() + "/state.json"; }

std::string sd_root()
{
#ifdef __SWITCH__
    return "/";
#else
    return "./playguard_data/sd";
#endif
}

bool ensure_dir(const std::string& dir)
{
    std::string partial;
    std::stringstream ss(dir);
    std::string part;
    bool absolute = !dir.empty() && dir[0] == '/';
    if (absolute) partial = "/";
    while (std::getline(ss, part, '/')) {
        if (part.empty()) continue;
        partial += part;
        if (mkdir(partial.c_str(), 0777) != 0 && errno != EEXIST) return false;
        partial += "/";
    }
    return true;
}

bool atomic_write(const std::string& path, const std::string& content, std::string* error)
{
    auto fail = [&](const char* what, int err) {
        if (error) *error = std::string(what) + ": " + std::strerror(err);
        return false;
    };
    std::size_t slash = path.find_last_of('/');
    if (slash != std::string::npos && !ensure_dir(path.substr(0, slash)))
        return fail("Could not create the folder", errno);

    // Written as "<path>.part", then renamed "<path>.tmp" once complete: a
    // ".tmp" is always a whole file, and a write cut short leaves only a
    // ".part" (dropped by the next write), never a ".tmp" taken for the file.
    const std::string tmp = path + ".tmp", part = path + ".part";
    // A previous write stopped between removing the file and renaming the
    // ".tmp": that ".tmp" is the only good copy. Promote it before a newer
    // one takes its name.
    struct stat st;
    if (stat(path.c_str(), &st) != 0 && stat(tmp.c_str(), &st) == 0)
        std::rename(tmp.c_str(), path.c_str());
    // Created 0644 (as util/pt_log.cpp), not fopen's 0666.
    const int fd = ::open(part.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
    FILE* f = fd >= 0 ? ::fdopen(fd, "wb") : nullptr;
    if (!f) {
        const int err = errno;
        if (fd >= 0) ::close(fd);
        return fail("Could not open the file", err);
    }
    bool ok = std::fwrite(content.data(), 1, content.size(), f) == content.size();
    int err = ok ? 0 : errno;
    if (ok && std::fflush(f) != 0) { ok = false; err = errno; }
    // On the card before the old file is removed: else a power loss after
    // the rename can leave an empty file in its place.
    if (ok && fsync(fileno(f)) != 0) { ok = false; err = errno; }
    if (std::fclose(f) != 0 && ok) { ok = false; err = errno; }
    if (!ok) {
        std::remove(part.c_str());
        return fail("Could not write the file", err);
    }
    // A ".tmp" next to the file (a write stopped before its last step) is an
    // older copy than this one; FAT rename does not replace it. Without the
    // file (the promotion above failed), that ".tmp" is the only good copy:
    // kept, and on FAT this write fails rather than lose it.
    if (stat(path.c_str(), &st) == 0) std::remove(tmp.c_str());
    if (std::rename(part.c_str(), tmp.c_str()) != 0) {
        const int e = errno;
        std::remove(part.c_str());
        return fail("Could not rename the file", e);
    }
    std::remove(path.c_str());   // FAT (SD card) rename does not replace an existing file
    if (std::rename(tmp.c_str(), path.c_str()) != 0) {
        // The old file is gone and the new one is complete: keep it, read_file
        // falls back to it.
        return fail("Could not rename the file", errno);
    }
    return true;
}

static bool read_whole(const std::string& path, std::string& out)
{
    std::ifstream in(path, std::ios::binary);
    if (!in) return false;
    std::stringstream ss;
    ss << in.rdbuf();
    out = ss.str();
    return true;
}

bool read_file(const std::string& path, std::string& out)
{
    if (read_whole(path, out)) return true;
    // atomic_write stopped between removing the old file and renaming the
    // new one (crash, power loss, rename error): the ".tmp" is the complete
    // new content. It is only used when the file itself is missing.
    struct stat st;
    if (stat(path.c_str(), &st) == 0) return false;
    return read_whole(path + ".tmp", out);
}

bool file_exists(const std::string& path)
{
    struct stat st;
    return stat(path.c_str(), &st) == 0 && S_ISREG(st.st_mode);
}

bool exists_or_pending(const std::string& path)
{
    struct stat st;
    return stat(path.c_str(), &st) == 0 || stat((path + ".tmp").c_str(), &st) == 0;
}

namespace
{
std::string aside_of(const std::string& target) { return target + ".old"; }
}   // namespace

void recover_replace(const std::string& target, const std::string& keep_as)
{
    const std::string aside = aside_of(target);
    if (!file_exists(aside)) return;
    // Stopped after the first step: the target is only aside, put back.
    if (!file_exists(target)) {
        std::rename(aside.c_str(), target.c_str());
        return;
    }
    // Stopped after the second: the new one is in place, the last step done.
    if (!keep_as.empty()) {
        std::remove(keep_as.c_str());   // FAT rename does not replace an existing file
        if (std::rename(aside.c_str(), keep_as.c_str()) == 0) return;
    }
    std::remove(aside.c_str());
}

bool replace_file(const std::string& target, const std::string& fresh, const std::string& keep_as, std::string* error)
{
    auto fail = [error](const std::string& why) {
        if (error) *error = why;
        return false;
    };
    recover_replace(target, keep_as);
    const std::string aside = aside_of(target);
    if (file_exists(aside)) return fail("cannot clear " + aside);
    // FAT has no atomic replace: the old one moves aside first.
    const bool had_target = std::rename(target.c_str(), aside.c_str()) == 0;
    if (!had_target && errno != ENOENT) return fail("cannot move " + target + " aside");
    if (std::rename(fresh.c_str(), target.c_str()) != 0) {
        if (had_target) std::rename(aside.c_str(), target.c_str());
        return fail("cannot put " + fresh + " in place");
    }
    if (had_target && !keep_as.empty()) {
        std::remove(keep_as.c_str());
        if (std::rename(aside.c_str(), keep_as.c_str()) == 0) return true;
    }
    std::remove(aside.c_str());
    return true;
}

bool put_aside(const std::string& path)
{
    struct stat st;
    const std::string bad = path + ".bad", older = bad + ".1";
    if (stat(bad.c_str(), &st) == 0) {
        std::remove(older.c_str());   // FAT rename does not replace an existing file
        if (std::rename(bad.c_str(), older.c_str()) != 0) return false;
    }
    const std::string from = stat(path.c_str(), &st) == 0 ? path : path + ".tmp";
    return std::rename(from.c_str(), bad.c_str()) == 0;
}

std::vector<std::string> list_files(const std::string& dir, const std::string& suffix)
{
    std::vector<std::string> names;
    DIR* d = opendir(dir.c_str());
    if (!d) return names;
    while (dirent* e = readdir(d)) {
        std::string n = e->d_name;
        if (n.size() > suffix.size() && n.compare(n.size() - suffix.size(), suffix.size(), suffix) == 0)
            names.push_back(n);
    }
    closedir(d);
    std::sort(names.begin(), names.end());
    return names;
}

}   // namespace paths
