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

    std::string tmp = path + ".tmp";
    // A previous write stopped between removing the file and renaming the
    // ".tmp": that ".tmp" is the only good copy. Promote it before reopening
    // the ".tmp" for writing truncates it.
    struct stat st;
    if (stat(path.c_str(), &st) != 0 && stat(tmp.c_str(), &st) == 0)
        std::rename(tmp.c_str(), path.c_str());
    // Created 0644 (as util/pt_log.cpp), not fopen's 0666.
    const int fd = ::open(tmp.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
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
        std::remove(tmp.c_str());
        return fail("Could not write the file", err);
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

// The first `head` bytes, then the last `tail` (all of it when the file is
// no longer): the bytes in between are skipped, not read.
static bool read_ends_of(const std::string& path, size_t head, size_t tail, std::string& out)
{
    FILE* f = std::fopen(path.c_str(), "rb");
    if (!f) return false;
    out.clear();
    const auto take = [f, &out](size_t count) {
        const size_t at = out.size();
        out.resize(at + count);
        out.resize(at + std::fread(&out[at], 1, count, f));
        return !std::ferror(f);
    };
    bool ok = std::fseek(f, 0, SEEK_END) == 0;
    const long size = ok ? std::ftell(f) : -1;
    ok = size >= 0 && std::fseek(f, 0, SEEK_SET) == 0;
    if (ok && (size_t)size <= head + tail) ok = take((size_t)size);
    else if (ok) ok = take(head) && std::fseek(f, size - (long)tail, SEEK_SET) == 0 && take(tail);
    std::fclose(f);
    return ok;
}

// `read` on `path`, else on its ".tmp": atomic_write stopped between removing
// the old file and renaming the new one (crash, power loss, rename error),
// and the ".tmp" is the complete new content. It is only used when the file
// itself is missing.
template <typename Read> static bool read_or_tmp(const std::string& path, Read read)
{
    if (read(path)) return true;
    struct stat st;
    if (stat(path.c_str(), &st) == 0) return false;
    return read(path + ".tmp");
}

bool read_file(const std::string& path, std::string& out)
{
    return read_or_tmp(path, [&out](const std::string& p) { return read_whole(p, out); });
}

bool read_ends(const std::string& path, size_t head, size_t tail, std::string& out)
{
    return read_or_tmp(path, [&](const std::string& p) { return read_ends_of(p, head, tail, out); });
}

bool exists(const std::string& path)
{
    struct stat st;
    return stat(path.c_str(), &st) == 0 || stat((path + ".tmp").c_str(), &st) == 0;
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
